/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "font_manager.h"

#include <unistd.h>
#include <string_ex.h>
#include <fcntl.h>
#include <filesystem>
#include <functional>
#include <thread>
#include "font_hilog.h"
#include "font_event_publish.h"
#include "font_manager_utils.h"
#include "hisysevent_adapter.h"
#include "storage_manager_adapter.h"
#include "font_define.h"
#include "parameters.h"

namespace OHOS {
namespace Global {
namespace FontManager {
namespace {
// Removes the private temp copy unless it has been renamed into place.
class TempFontGuard {
public:
    TempFontGuard(const std::string &path, int32_t fd) : path_(path), fd_(fd) {}
    ~TempFontGuard()
    {
        if (fd_ >= 0) {
            (void)close(fd_);
        }
        if (!committed_ && !path_.empty()) {
            if (!FontManagerUtils::RemoveFile(path_)) {
                FONT_LOGW("TempFontGuard: failed to remove temp file %{public}s", path_.c_str());
            }
        }
    }
    TempFontGuard(const TempFontGuard &) = delete;
    TempFontGuard &operator=(const TempFontGuard &) = delete;

    void MarkCommitted()
    {
        committed_ = true;
    }
private:
    std::string path_;
    int32_t fd_ = -1;
    bool committed_ = false;
};

std::string GetSafeFileName(const std::string &fdPath)
{
    std::string fileName = FontManagerUtils::GetFileName(fdPath);
    if (fileName.empty() || fileName == "." || fileName == "..") {
        return "font.ttf";
    }
    return fileName;
}
} // namespace

FontManager::FontManager()
{
}

FontManager::~FontManager()
{
}

int32_t FontManager::GetMaxInstallNum()
{
    int32_t maxInstallNum = OHOS::system::GetIntParameter<int32_t>(MAX_INSTALL_NUM_PARAM_KEY,
        DEFAULT_MAX_INSTALL_NUM);
    if (maxInstallNum < DEFAULT_MAX_INSTALL_NUM) {
        FONT_LOGE("Invalid max install num %{public}d, fallback to default %{public}d",
            maxInstallNum, DEFAULT_MAX_INSTALL_NUM);
        return DEFAULT_MAX_INSTALL_NUM;
    }
    return maxInstallNum;
}

int32_t FontManager::InstallFont(const int32_t &fd, const int32_t userId)
{
    std::string installPath = INSTALL_PATH_PREFIX + std::to_string(userId) + INSTALL_PATH_SUFFIX;
    if (!(FontManagerUtils::CheckAndInitInstallPath(installPath))) {
        return ERR_FILE_NOT_EXISTS;
    }
    if (!FontManagerUtils::IsValidFontFd(fd)) {
        return ERR_FILE_VERIFY_FAIL;
    }
    auto& fontConfig = SafeGetOrCreateConfig(userId, installPath + FONT_CONFIG_FILE);
    std::string fileName = GetSafeFileName(FontManagerUtils::GetFilePathByFd(fd));

    // Snapshot the caller-supplied fd first. The parser mmaps the file, so it
    // must never see an inode the caller can still truncate or rewrite.
    std::string tempPath;
    int32_t tempFd = FontManagerUtils::CreatePrivateTempCopy(fd, installPath + TEMP_FILE, tempPath);
    if (tempFd < 0) {
        return ERR_COPY_FAIL;
    }
    TempFontGuard tempGuard(tempPath, tempFd);
    std::vector<std::string> fullNameVector = FontManagerUtils::GetFullNamesByPrivateFd(tempFd);
    if (fullNameVector.empty()) {
        return ERR_FILE_VERIFY_FAIL;
    }

    std::string destPath;
    {
        std::lock_guard<std::mutex> installLock(fontConfig.GetInstallLock());
        int32_t ret = CheckAndRemoveDuplicateFonts(fontConfig, installPath, fullNameVector);
        if (ret != ERR_OK) {
            return ret;
        }
        int32_t maxInstallNum = GetMaxInstallNum();
        if (fontConfig.GetInstalledFontsNum() >= maxInstallNum) {
            FONT_LOGE("FontManager: Max install count %{public}d reached", maxInstallNum);
            return ERR_MAX_FILE_COUNT;
        }
        destPath = CommitTempFile(tempPath, installPath, fileName);
        if (destPath.empty()) {
            return ERR_COPY_FAIL;
        }
        tempGuard.MarkCommitted();
        ret = InsertUserFontRecord(fontConfig, destPath, fullNameVector, maxInstallNum);
        if (ret != ERR_OK) {
            return ret;
        }
    }
    ReportFontInstalled(userId, installPath, FontManagerUtils::GetFileName(destPath), fullNameVector);
    return ERR_OK;
}

int32_t FontManager::CheckAndRemoveDuplicateFonts(FontConfig& fontConfig, const std::string &installPath,
    const std::vector<std::string> &fullNames)
{
    for (const auto &fullName : fullNames) {
        std::string findPath = fontConfig.GetFontFileByName(fullName);
        if (findPath.empty()) {
            continue;
        }
        if (FontManagerUtils::CheckPathExist(SandBoxPathToRealPath(installPath, findPath))) {
            return ERR_INSTALLED_ALRADY;
        }
        if (!fontConfig.DeleteFontRecord(findPath)) {
            FONT_LOGE("fix install_fontconfig fail");
            return ERR_INSTALL_FAIL;
        }
    }
    return ERR_OK;
}

int32_t FontManager::InsertUserFontRecord(FontConfig& fontConfig, const std::string &destPath,
    const std::vector<std::string> &fullNames, int32_t maxInstallNum)
{
    std::string realFileName = FontManagerUtils::GetFileName(destPath);
    int32_t ret = fontConfig.InsertFontRecordIfUnderLimit(INSTALL_PATH_APP + realFileName,
        fullNames, maxInstallNum);
    if (ret != ERR_OK) {
        FontManagerUtils::RemoveFile(destPath);
        FONT_LOGE("install failed: max count reached or config update fail, fileName = %{public}s",
            realFileName.c_str());
    }
    return ret;
}

void FontManager::ReportFontInstalled(int32_t userId, const std::string &installPath,
    const std::string &fileName, const std::vector<std::string> &fullNames)
{
    HisyseventAdapter::GetInstance()->CollectUserDataSize(installPath);
    StorageManagerAdapter::GetInstance()->ReportFontBundleStats(userId, installPath);
    FontEventPublish::PublishFontUpdate(FontEventType::INSTALL, GetFormatFullName(fullNames), userId);
    FONT_LOGI("Install font success, fileName:%{public}s, userId:%{public}d", fileName.c_str(), userId);
}

std::string FontManager::GetFormatFullName(const std::vector<std::string> &fullNameVector)
{
    std::string formatFullName;
    std::string split = ",";
    for (const auto &name : fullNameVector) {
        formatFullName += name + split;
    }
    if (formatFullName.size() >= split.size()) {
        return formatFullName.substr(0, formatFullName.size() - split.size());
    }
    return formatFullName;
}

std::string FontManager::CommitTempFile(const std::string &tempPath, const std::string &installPath,
    const std::string &fileName)
{
    std::string timeTag = FontManagerUtils::GetFileTime();
    std::string threadTag = std::to_string(std::hash<std::thread::id>{}(std::this_thread::get_id()));
    std::string split = "_";
    std::string destPath = installPath + timeTag + split + threadTag + split + fileName;
    if (!FontManagerUtils::RenameFile(tempPath, destPath)) {
        FONT_LOGE("rename file %{public}s error", fileName.c_str());
        return "";
    }
    return destPath;
}

int32_t FontManager::UninstallFont(const std::string &fontFullName, const int32_t userId)
{
    std::string installPath = INSTALL_PATH_PREFIX + std::to_string(userId) + INSTALL_PATH_SUFFIX;
    if (fontFullName.empty()) {
        FONT_LOGE("FontManager::UninstallFont, fontName is empty");
        return ERR_UNINSTALL_FILE_NOT_EXISTS;
    }
    auto& fontConfig = SafeGetOrCreateConfig(userId, installPath + FONT_CONFIG_FILE);
    {
        std::lock_guard<std::mutex> installLock(fontConfig.GetInstallLock());
        auto record = fontConfig.GetFontRecordByName(fontFullName);
        if (!record.has_value() || record->fontPath.empty()) {
            FONT_LOGE("Can't find fontFullName = %{public}s", fontFullName.c_str());
            return ERR_UNINSTALL_FILE_NOT_EXISTS;
        }
        if (record->scope >= 0) {
            FONT_LOGE("UninstallFont: font is scope font, cannot uninstall via user-level API, name=%{public}s",
                fontFullName.c_str());
            return ERR_UNINSTALL_FILE_NOT_EXISTS;
        }
        std::string path = record->fontPath;
        std::string realPath = ValidateAndResolveRealPath(installPath, path);
        if (realPath.empty()) {
            FONT_LOGE("UninstallFont: invalid font path in config, path = %{public}s", path.c_str());
            return ERR_UNINSTALL_FILE_NOT_EXISTS;
        }
        if (FontManagerUtils::CheckPathExist(realPath)) {
            if (!FontManagerUtils::RemoveFile(realPath)) {
                return ERR_UNINSTALL_REMOVE_FAIL;
            }
        }
        if (!fontConfig.DeleteFontRecord(path)) {
            FONT_LOGE("update install_fontconfig fail, path = %{public}s", path.c_str());
            return ERR_UNINSTALL_FAIL;
        }
    }
    HisyseventAdapter::GetInstance()->CollectUserDataSize(installPath);
    StorageManagerAdapter::GetInstance()->ReportFontBundleStats(userId, installPath);
    FontEventPublish::PublishFontUpdate(FontEventType::UNINSTALL, fontFullName, userId);
    FONT_LOGI("Uninstall font success, fontFullName:%{public}s, userId:%{public}d", fontFullName.c_str(), userId);
    return ERR_OK;
}

std::string FontManager::SandBoxPathToRealPath(const std::string &installPath, const std::string &path)
{
    if (path.find(INSTALL_PATH_APP) == 0) {
        return installPath + path.substr(INSTALL_PATH_APP.length());
    }
    std::string fileName = FontManagerUtils::GetFileName(path);
    return installPath + fileName;
}

std::string FontManager::ValidateAndResolveRealPath(const std::string &installPath, const std::string &path)
{
    std::string fileName = FontManagerUtils::GetFileName(path);
    if (fileName.empty() || fileName == "." || fileName == ".." ||
        fileName.find('/') != std::string::npos || fileName.find('\\') != std::string::npos) {
        FONT_LOGE("ValidateAndResolveRealPath: invalid font path in config: %{public}s", path.c_str());
        return "";
    }
    std::string realPath = SandBoxPathToRealPath(installPath, path);
    std::error_code ec;
    std::filesystem::path canonical = std::filesystem::weakly_canonical(realPath, ec);
    if (ec) {
        FONT_LOGE("ValidateAndResolveRealPath: weakly_canonical failed, path=%{public}s, ec=%{public}d",
            realPath.c_str(), ec.value());
        return "";
    }
    std::string canonicalStr = canonical.string();
    std::error_code pathEc;
    std::filesystem::path canonicalInstall = std::filesystem::weakly_canonical(installPath, pathEc);
    std::string installDir = canonicalInstall.string();
    if (!installDir.empty() && installDir.back() != '/') {
        installDir += '/';
    }
    if (pathEc || canonicalStr.rfind(installDir, 0) != 0) {
        FONT_LOGE("ValidateAndResolveRealPath: realPath escapes installPath: %{public}s", realPath.c_str());
        return "";
    }
    return canonicalStr;
}

FontConfig& FontManager::SafeGetOrCreateConfig(int32_t userId, const std::string& configPath)
{
    std::lock_guard<std::mutex> lock(mapLock_);
    auto result = configMap_.try_emplace(userId, FontConfig(configPath));
    return result.first->second;
}

std::string FontManager::GetAppInstallPath(int32_t userId, const std::string &appIdentifier)
{
    return INSTALL_PATH_PREFIX + std::to_string(userId) + INSTALL_PATH_SUFFIX + appIdentifier + "/";
}

int32_t FontManager::InstallScopeFont(const ScopeFontInstallInfo &info)
{
    if (info.scope != FONT_SCOPE_APP && info.scope != FONT_SCOPE_SESSION) {
        FONT_LOGE("InstallScopeFont: invalid scope=%{public}d", info.scope);
        return ERR_INVALID_PARAM;
    }
    std::string installPath = INSTALL_PATH_PREFIX + std::to_string(info.userId) + INSTALL_PATH_SUFFIX;
    if (!FontManagerUtils::CheckAndInitInstallPath(installPath)) {
        return ERR_SYSTEM_ERROR;
    }
    auto& fontConfig = SafeGetOrCreateConfig(info.userId, installPath + FONT_CONFIG_FILE);
    std::string fileName;
    std::string appDir;
    int32_t ret = PrepareScopeFontInstall(info, fontConfig, fileName, appDir);
    if (ret != ERR_OK) {
        return ret;
    }

    // Snapshot the caller-supplied fd before parsing; see InstallFont.
    std::string tempPath;
    int32_t tempFd = FontManagerUtils::CreatePrivateTempCopy(info.fd, appDir + TEMP_FILE, tempPath);
    if (tempFd < 0) {
        return ERR_COPY_FAIL;
    }
    TempFontGuard tempGuard(tempPath, tempFd);
    std::vector<std::string> fullNames = FontManagerUtils::GetFullNamesByPrivateFd(tempFd);
    if (fullNames.empty()) {
        return ERR_FILE_VERIFY_FAIL;
    }

    {
        std::lock_guard<std::mutex> installLock(fontConfig.GetInstallLock());
        // Re-check under the install lock: the pre-check above ran without it.
        ret = RecheckScopeFontUnderLock(fontConfig, info.srcPath, fullNames);
        if (ret != ERR_OK) {
            return ret;
        }
        std::string destPath = CommitTempFile(tempPath, appDir, fileName);
        if (destPath.empty()) {
            return ERR_COPY_FAIL;
        }
        tempGuard.MarkCommitted();
        ret = InsertScopeFontRecord(info, fontConfig, destPath, fullNames);
        if (ret != ERR_OK) {
            return ret;
        }
    }
    ReportScopeFontInstalled(info, installPath, fullNames);
    return ERR_OK;
}

int32_t FontManager::PrepareScopeFontInstall(const ScopeFontInstallInfo &info, FontConfig& fontConfig,
    std::string &fileName, std::string &appDir)
{
    if (!fontConfig.CheckAndUpdateFontRecord()) {
        FONT_LOGE("CheckAndUpdateFontRecord fail");
        return ERR_SYSTEM_ERROR;
    }
    int32_t ret = ValidateScopeFontForInstall(fontConfig, info.srcPath);
    if (ret != ERR_OK) {
        return ret;
    }
    if (!FontManagerUtils::IsValidFontFd(info.fd)) {
        return ERR_FILE_VERIFY_FAIL;
    }
    fileName = GetSafeFileName(FontManagerUtils::GetFilePathByFd(info.fd));
    appDir = GetAppInstallPath(info.userId, info.appIdentifier);
    if (!FontManagerUtils::CheckAndInitScopeFontPath(appDir)) {
        FONT_LOGE("InstallScopeFont: init app dir failed");
        return ERR_SYSTEM_ERROR;
    }
    return ERR_OK;
}

int32_t FontManager::CheckScopeFontNameConflicts(FontConfig& fontConfig,
    const std::vector<std::string> &fullNames)
{
    for (const auto &name : fullNames) {
        if (fontConfig.GetFontRecordByName(name).has_value()) {
            FONT_LOGE("InstallScopeFont: font name already exists");
            return ERR_INSTALLED_ALRADY;
        }
    }
    return ERR_OK;
}

int32_t FontManager::RecheckScopeFontUnderLock(FontConfig& fontConfig, const std::string &srcPath,
    const std::vector<std::string> &fullNames)
{
    int32_t ret = ValidateScopeFontForInstall(fontConfig, srcPath);
    if (ret != ERR_OK) {
        return ret;
    }
    return CheckScopeFontNameConflicts(fontConfig, fullNames);
}

int32_t FontManager::InsertScopeFontRecord(const ScopeFontInstallInfo &info, FontConfig& fontConfig,
    const std::string &destPath, const std::vector<std::string> &fullNames)
{
    FontRecordInfo record;
    record.fontPath = INSTALL_PATH_APP + info.appIdentifier + "/" + FontManagerUtils::GetFileName(destPath);
    record.fullNames = fullNames;
    record.scope = info.scope;
    record.srcPath = info.srcPath;
    record.appIdentifier = info.appIdentifier;
    record.bundleName = info.bundleName;
    if (!fontConfig.InsertScopeFontRecord(record)) {
        FontManagerUtils::RemoveFile(destPath);
        FONT_LOGE("InstallScopeFont: insert record failed");
        return ERR_SYSTEM_ERROR;
    }
    return ERR_OK;
}

void FontManager::ReportScopeFontInstalled(const ScopeFontInstallInfo &info, const std::string &installPath,
    const std::vector<std::string> &fullNames)
{
    HisyseventAdapter::GetInstance()->CollectUserDataSize(installPath);
    StorageManagerAdapter::GetInstance()->ReportFontBundleStats(info.userId, installPath);
    FontEventPublish::PublishFontUpdate(FontEventType::INSTALL, GetFormatFullName(fullNames), info.userId);
    FONT_LOGI("InstallScopeFont success, srcPath:%{public}s, scope:%{public}d", info.srcPath.c_str(), info.scope);
}

int32_t FontManager::ValidateScopeFontForInstall(FontConfig& fontConfig, const std::string &srcPath)
{
    auto record = fontConfig.GetFontRecordByUrl(srcPath);
    if (record.has_value()) {
        FONT_LOGE("InstallScopeFont: srcPath already installed");
        return ERR_INSTALLED_ALRADY;
    }
    int32_t maxInstallNum = GetMaxInstallNum();
    if (fontConfig.GetTotalInstalledFontsNum() >= maxInstallNum) {
        FONT_LOGE("InstallScopeFont: max count reached");
        return ERR_MAX_FILE_COUNT;
    }
    return ERR_OK;
}

int32_t FontManager::UninstallScopeFont(const std::string &srcPath, const std::string &bundleName,
    int32_t tokenId, int32_t userId)
{
    std::string installPath = INSTALL_PATH_PREFIX + std::to_string(userId) + INSTALL_PATH_SUFFIX;
    if (srcPath.empty()) {
        return ERR_SCOPE_FONT_NOT_FOUND;
    }
    auto& fontConfig = SafeGetOrCreateConfig(userId, installPath + FONT_CONFIG_FILE);
    std::vector<std::string> fullNames;
    {
        std::lock_guard<std::mutex> installLock(fontConfig.GetInstallLock());
        auto record = fontConfig.GetFontRecordByUrl(srcPath);
        if (!record.has_value()) {
            FONT_LOGE("UninstallScopeFont: record not found, srcPath=%{public}s", srcPath.c_str());
            return ERR_SCOPE_FONT_NOT_FOUND;
        }
        if (record->bundleName != bundleName) {
            FONT_LOGE("UninstallScopeFont: bundleName mismatch, caller=%{public}s, owner=%{public}s",
                bundleName.c_str(), record->bundleName.c_str());
            return ERR_SCOPE_FONT_NOT_FOUND;
        }
        std::string expectedAppId = FontManagerUtils::MakeAppIdentifier(record->scope, tokenId);
        if (record->appIdentifier != expectedAppId) {
            FONT_LOGE("UninstallScopeFont: appIdentifier mismatch, caller tokenId=%{public}d", tokenId);
            return ERR_SCOPE_FONT_NOT_FOUND;
        }
        fullNames = record->fullNames;
        // scope font fontPath is sandbox path, convert to real path
        std::string realPath = ValidateAndResolveRealPath(installPath, record->fontPath);
        if (!realPath.empty() && FontManagerUtils::CheckPathExist(realPath)) {
            if (!FontManagerUtils::RemoveFile(realPath)) {
                return ERR_UNINSTALL_REMOVE_FAIL;
            }
        }
        if (!fontConfig.DeleteScopeFontRecordByUrl(srcPath)) {
            FONT_LOGE("UninstallScopeFont: delete record failed");
            return ERR_SYSTEM_ERROR;
        }
    }
    HisyseventAdapter::GetInstance()->CollectUserDataSize(installPath);
    StorageManagerAdapter::GetInstance()->ReportFontBundleStats(userId, installPath);
    FontEventPublish::PublishFontUpdate(FontEventType::UNINSTALL, GetFormatFullName(fullNames), userId);
    FONT_LOGI("UninstallScopeFont success, srcPath=%{public}s", srcPath.c_str());
    return ERR_OK;
}

int32_t FontManager::GetFontScope(const std::string &srcPath, const std::string &bundleName,
    int32_t tokenId, int32_t userId)
{
    std::string installPath = INSTALL_PATH_PREFIX + std::to_string(userId) + INSTALL_PATH_SUFFIX;
    auto& fontConfig = SafeGetOrCreateConfig(userId, installPath + FONT_CONFIG_FILE);
    auto record = fontConfig.GetFontRecordByUrl(srcPath);
    if (!record.has_value()) {
        return ERR_SCOPE_FONT_NOT_FOUND;
    }
    if (record->bundleName != bundleName) {
        FONT_LOGI("GetFontScope: bundleName mismatch, caller does not own this font");
        return ERR_SCOPE_FONT_NOT_FOUND;
    }
    std::string expectedAppId = FontManagerUtils::MakeAppIdentifier(record->scope, tokenId);
    if (record->appIdentifier != expectedAppId) {
        FONT_LOGI("GetFontScope: appIdentifier mismatch, caller tokenId=%{public}d", tokenId);
        return ERR_SCOPE_FONT_NOT_FOUND;
    }
    return record->scope;
}

int32_t FontManager::CleanupAppScopeFonts(const std::string &appIdentifier, int32_t userId)
{
    std::string installPath = INSTALL_PATH_PREFIX + std::to_string(userId) + INSTALL_PATH_SUFFIX;
    auto& fontConfig = SafeGetOrCreateConfig(userId, installPath + FONT_CONFIG_FILE);
    std::string removedNames;
    size_t recordCount = 0;
    {
        std::lock_guard<std::mutex> installLock(fontConfig.GetInstallLock());
        auto records = fontConfig.GetFontRecordsByAppId(appIdentifier);
        recordCount = records.size();
        for (const auto &record : records) {
            std::string realPath = ValidateAndResolveRealPath(installPath, record.fontPath);
            if (!realPath.empty() && FontManagerUtils::CheckPathExist(realPath)) {
                if (!FontManagerUtils::RemoveFile(realPath)) {
                    FONT_LOGW("CleanupAppScopeFonts: remove file failed, path=%{public}s", realPath.c_str());
                }
            }
            fontConfig.DeleteScopeFontRecordByUrl(record.srcPath);
            std::string names = GetFormatFullName(record.fullNames);
            if (!names.empty()) {
                removedNames += removedNames.empty() ? names : "," + names;
            }
        }
        std::string appDir = GetAppInstallPath(userId, appIdentifier);
        FontManagerUtils::DeleteDir(appDir, true);
    }
    HisyseventAdapter::GetInstance()->CollectUserDataSize(installPath);
    if (recordCount > 0) {
        StorageManagerAdapter::GetInstance()->ReportFontBundleStats(userId, installPath);
        FontEventPublish::PublishFontUpdate(FontEventType::UNINSTALL, removedNames, userId);
    }
    FONT_LOGI("CleanupAppScopeFonts done, appIdentifier=%{public}s, count=%{public}zu",
        appIdentifier.c_str(), recordCount);
    return ERR_OK;
}

int32_t FontManager::CleanupScopeFontsByUser(int32_t userId)
{
    std::string installPath = INSTALL_PATH_PREFIX + std::to_string(userId) + INSTALL_PATH_SUFFIX;
    auto& fontConfig = SafeGetOrCreateConfig(userId, installPath + FONT_CONFIG_FILE);
    std::string removedNames;
    size_t recordCount = 0;
    {
        std::lock_guard<std::mutex> installLock(fontConfig.GetInstallLock());
        auto scopeRecords = fontConfig.GetScopeFontRecords();
        recordCount = scopeRecords.size();
        for (const auto &record : scopeRecords) {
            std::string realPath = ValidateAndResolveRealPath(installPath, record.fontPath);
            if (!realPath.empty() && FontManagerUtils::CheckPathExist(realPath)) {
                if (!FontManagerUtils::RemoveFile(realPath)) {
                    FONT_LOGW("CleanupScopeFontsByUser: remove file failed, path=%{public}s", realPath.c_str());
                }
            }
            fontConfig.DeleteScopeFontRecordByUrl(record.srcPath);
            std::string names = GetFormatFullName(record.fullNames);
            if (!names.empty()) {
                removedNames += removedNames.empty() ? names : "," + names;
            }
        }
    }
    HisyseventAdapter::GetInstance()->CollectUserDataSize(installPath);
    if (recordCount > 0) {
        StorageManagerAdapter::GetInstance()->ReportFontBundleStats(userId, installPath);
        FontEventPublish::PublishFontUpdate(FontEventType::UNINSTALL, removedNames, userId);
    }
    FONT_LOGI("CleanupScopeFontsByUser done, userId=%{public}d, count=%{public}zu", userId, recordCount);
    return ERR_OK;
}

int32_t FontManager::CleanupAppScopeFontsByUser(int32_t userId)
{
    std::string installPath = INSTALL_PATH_PREFIX + std::to_string(userId) + INSTALL_PATH_SUFFIX;
    auto& fontConfig = SafeGetOrCreateConfig(userId, installPath + FONT_CONFIG_FILE);
    std::string removedNames;
    size_t recordCount = 0;
    {
        std::lock_guard<std::mutex> installLock(fontConfig.GetInstallLock());
        auto appRecords = fontConfig.GetAppScopeFontRecords();
        recordCount = appRecords.size();
        for (const auto &record : appRecords) {
            std::string realPath = ValidateAndResolveRealPath(installPath, record.fontPath);
            if (!realPath.empty() && FontManagerUtils::CheckPathExist(realPath)) {
                if (!FontManagerUtils::RemoveFile(realPath)) {
                    FONT_LOGW("CleanupAppScopeFontsByUser: remove file failed, path=%{public}s", realPath.c_str());
                }
            }
            fontConfig.DeleteScopeFontRecordByUrl(record.srcPath);
            std::string names = GetFormatFullName(record.fullNames);
            if (!names.empty()) {
                removedNames += removedNames.empty() ? names : "," + names;
            }
        }
    }
    HisyseventAdapter::GetInstance()->CollectUserDataSize(installPath);
    if (recordCount > 0) {
        StorageManagerAdapter::GetInstance()->ReportFontBundleStats(userId, installPath);
        FontEventPublish::PublishFontUpdate(FontEventType::UNINSTALL, removedNames, userId);
    }
    FONT_LOGI("CleanupAppScopeFontsByUser done, userId=%{public}d, count=%{public}zu", userId, recordCount);
    return ERR_OK;
}
} // namespace FontManager
} // namespace Global
} // namespace OHOS
