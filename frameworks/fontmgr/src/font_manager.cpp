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
    auto& fontConfig = SafeGetOrCreateConfig(userId, installPath + FONT_CONFIG_FILE);
    std::vector<std::string> fullNameVector = FontManagerUtils::GetFullNamesByFd(fd);
    if (fullNameVector.size() == 0) {
        return ERR_FILE_VERIFY_FAIL;
    }
    for (const auto &fullName : fullNameVector) {
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
    int32_t maxInstallNum = GetMaxInstallNum();
    if (fontConfig.GetInstalledFontsNum() >= maxInstallNum) {
        FONT_LOGE("FontManager: Max install count %{public}d reached", maxInstallNum);
        return ERR_MAX_FILE_COUNT;
    }
    std::string fileName = FontManagerUtils::GetFileName(FontManagerUtils::GetFilePathByFd(fd));
    std::string destPath = CopyFileForInstall(installPath, fileName, fd);
    if (destPath.empty()) {
        return ERR_COPY_FAIL;
    }
    std::string realFileName = FontManagerUtils::GetFileName(destPath);
    if (!fontConfig.InsertFontRecord(INSTALL_PATH_APP + realFileName, fullNameVector)) {
        FontManagerUtils::DeleteDir(destPath, true);
        FONT_LOGE("update install_fontconfig fail, fileName = %{public}s", realFileName.c_str());
        return ERR_INSTALL_FAIL;
    }
    HisyseventAdapter::GetInstance()->CollectUserDataSize(installPath);
    StorageManagerAdapter::GetInstance()->ReportFontBundleStats(userId, installPath);
    FontEventPublish::PublishFontUpdate(FontEventType::INSTALL, GetFormatFullName(fullNameVector), userId);
    FONT_LOGI("Install font success, fileName:%{public}s, userId:%{public}d", realFileName.c_str(), userId);
    return ERR_OK;
}

std::string FontManager::GetFormatFullName(const std::vector<std::string> &fullNameVector)
{
    std::string FormatFullName;
    std::string split = ",";
    for (const auto &name : fullNameVector) {
        FormatFullName += name + split;
    }
    if (FormatFullName.size() >= split.size()) {
        return FormatFullName.substr(0, FormatFullName.size() - split.size());
    }
    return FormatFullName;
}

std::string FontManager::CopyFileForInstall(const std::string &installPath, const std::string &fileName,
    const int32_t &fd)
{
    std::string tempPath = installPath + TEMP_FILE + fileName;
    if (!FontManagerUtils::CopyFile(fd, tempPath)) {
        FONT_LOGE("copy file %{public}s error", tempPath.c_str());
        return "";
    }

    std::string destPath = installPath + fileName;
    if (FontManagerUtils::CheckPathExist(destPath)) {
        std::string split = "_";
        destPath = installPath + FontManagerUtils::GetFileTime() + split + fileName;
        FONT_LOGI("target file name is exist, store the file with a new name (%{public}s)", destPath.c_str());
    }
    if (!FontManagerUtils::RenameFile(tempPath, destPath)) {
        FONT_LOGE("rename file %{public}s error", fileName.c_str());
        FontManagerUtils::RemoveFile(tempPath);
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
    std::string realPath = SandBoxPathToRealPath(installPath, path);
    if (FontManagerUtils::CheckPathExist(realPath)) {
        if (!FontManagerUtils::RemoveFile(realPath)) {
            return ERR_UNINSTALL_REMOVE_FAIL;
        }
    }
    if (!fontConfig.DeleteFontRecord(path)) {
        FONT_LOGE("update install_fontconfig fail, path = %{public}s", path.c_str());
        return ERR_UNINSTALL_FAIL;
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
    if (!fontConfig.CheckAndUpdateFontRecord()) {
        FONT_LOGE("CheckAndUpdateFontRecord fail");
        return ERR_SYSTEM_ERROR;
    }
    std::vector<std::string> fullNames;
    int32_t ret = ValidateScopeFontForInstall(fontConfig, info.srcPath, info.fd, fullNames);
    if (ret != ERR_OK) {
        return ret;
    }
    ret = CopyAndInsertScopeFont(info, fontConfig, fullNames);
    if (ret != ERR_OK) {
        return ret;
    }
    HisyseventAdapter::GetInstance()->CollectUserDataSize(installPath);
    StorageManagerAdapter::GetInstance()->ReportFontBundleStats(info.userId, installPath);
    FontEventPublish::PublishFontUpdate(FontEventType::INSTALL, GetFormatFullName(fullNames), info.userId);
    FONT_LOGI("InstallScopeFont success, srcPath:%{public}s, scope:%{public}d", info.srcPath.c_str(), info.scope);
    return ERR_OK;
}

int32_t FontManager::ValidateScopeFontForInstall(FontConfig& fontConfig, const std::string &srcPath,
    const int32_t fd, std::vector<std::string>& fullNames)
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
    fullNames = FontManagerUtils::GetFullNamesByFd(fd);
    if (fullNames.empty()) {
        return ERR_FILE_VERIFY_FAIL;
    }
    for (const auto &name : fullNames) {
        auto existing = fontConfig.GetFontRecordByName(name);
        if (existing.has_value()) {
            FONT_LOGE("InstallScopeFont: font name already exists");
            return ERR_INSTALLED_ALRADY;
        }
    }
    return ERR_OK;
}

int32_t FontManager::CopyAndInsertScopeFont(const ScopeFontInstallInfo &info,
    FontConfig& fontConfig, const std::vector<std::string>& fullNames)
{
    std::string appDir = GetAppInstallPath(info.userId, info.appIdentifier);
    if (!FontManagerUtils::CheckAndInitScopeFontPath(appDir)) {
        FONT_LOGE("InstallScopeFont: init app dir failed");
        return ERR_SYSTEM_ERROR;
    }
    std::string fileName = FontManagerUtils::GetFileName(FontManagerUtils::GetFilePathByFd(info.fd));
    std::string destPath = CopyFileForInstall(appDir, fileName, info.fd);
    if (destPath.empty()) {
        return ERR_COPY_FAIL;
    }
    FontRecordInfo record;
    record.fontPath = INSTALL_PATH_APP + info.appIdentifier + "/" + FontManagerUtils::GetFileName(destPath);
    record.fullNames = fullNames;
    record.scope = info.scope;
    record.srcPath = info.srcPath;
    record.appIdentifier = info.appIdentifier;
    record.bundleName = info.bundleName;
    if (!fontConfig.InsertScopeFontRecord(record)) {
        FontManagerUtils::DeleteDir(destPath, true);
        FONT_LOGE("InstallScopeFont: insert record failed");
        return ERR_SYSTEM_ERROR;
    }
    return ERR_OK;
}

int32_t FontManager::UninstallScopeFont(const std::string &srcPath, const std::string &bundleName,
    int32_t userId)
{
    std::string installPath = INSTALL_PATH_PREFIX + std::to_string(userId) + INSTALL_PATH_SUFFIX;
    if (srcPath.empty()) {
        return ERR_SCOPE_FONT_NOT_FOUND;
    }
    auto& fontConfig = SafeGetOrCreateConfig(userId, installPath + FONT_CONFIG_FILE);
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
    // scope font fontPath is sandbox path, convert to real path
    std::string realPath = SandBoxPathToRealPath(installPath, record->fontPath);
    if (FontManagerUtils::CheckPathExist(realPath)) {
        if (!FontManagerUtils::RemoveFile(realPath)) {
            return ERR_UNINSTALL_REMOVE_FAIL;
        }
    }
    if (!fontConfig.DeleteScopeFontRecordByUrl(srcPath)) {
        FONT_LOGE("UninstallScopeFont: delete record failed");
        return ERR_SYSTEM_ERROR;
    }
    HisyseventAdapter::GetInstance()->CollectUserDataSize(installPath);
    StorageManagerAdapter::GetInstance()->ReportFontBundleStats(userId, installPath);
    FontEventPublish::PublishFontUpdate(FontEventType::UNINSTALL,
        GetFormatFullName(record->fullNames), userId);
    FONT_LOGI("UninstallScopeFont success, srcPath=%{public}s", srcPath.c_str());
    return ERR_OK;
}

int32_t FontManager::GetFontScope(const std::string &srcPath, const std::string &bundleName,
    int32_t userId)
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
    return record->scope;
}

int32_t FontManager::CleanupAppScopeFonts(const std::string &appIdentifier, int32_t userId)
{
    std::string installPath = INSTALL_PATH_PREFIX + std::to_string(userId) + INSTALL_PATH_SUFFIX;
    auto& fontConfig = SafeGetOrCreateConfig(userId, installPath + FONT_CONFIG_FILE);
    auto records = fontConfig.GetFontRecordsByAppId(appIdentifier);
    std::string removedNames;
    for (const auto &record : records) {
        std::string realPath = SandBoxPathToRealPath(installPath, record.fontPath);
        if (FontManagerUtils::CheckPathExist(realPath)) {
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
    HisyseventAdapter::GetInstance()->CollectUserDataSize(installPath);
    if (!records.empty()) {
        StorageManagerAdapter::GetInstance()->ReportFontBundleStats(userId, installPath);
        FontEventPublish::PublishFontUpdate(FontEventType::UNINSTALL, removedNames, userId);
    }
    std::string appDir = GetAppInstallPath(userId, appIdentifier);
    FontManagerUtils::DeleteDir(appDir, true);
    FONT_LOGI("CleanupAppScopeFonts done, appIdentifier=%{public}s, count=%{public}zu",
        appIdentifier.c_str(), records.size());
    return ERR_OK;
}

int32_t FontManager::CleanupScopeFontsByUser(int32_t userId)
{
    std::string installPath = INSTALL_PATH_PREFIX + std::to_string(userId) + INSTALL_PATH_SUFFIX;
    auto& fontConfig = SafeGetOrCreateConfig(userId, installPath + FONT_CONFIG_FILE);
    auto scopeRecords = fontConfig.GetScopeFontRecords();
    std::string removedNames;
    for (const auto &record : scopeRecords) {
        std::string realPath = SandBoxPathToRealPath(installPath, record.fontPath);
        if (FontManagerUtils::CheckPathExist(realPath)) {
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
    HisyseventAdapter::GetInstance()->CollectUserDataSize(installPath);
    if (!scopeRecords.empty()) {
        StorageManagerAdapter::GetInstance()->ReportFontBundleStats(userId, installPath);
        FontEventPublish::PublishFontUpdate(FontEventType::UNINSTALL, removedNames, userId);
    }
    FONT_LOGI("CleanupScopeFontsByUser done, userId=%{public}d, count=%{public}zu",
        userId, scopeRecords.size());
    return ERR_OK;
}

int32_t FontManager::CleanupAppScopeFontsByUser(int32_t userId)
{
    std::string installPath = INSTALL_PATH_PREFIX + std::to_string(userId) + INSTALL_PATH_SUFFIX;
    auto& fontConfig = SafeGetOrCreateConfig(userId, installPath + FONT_CONFIG_FILE);
    auto appRecords = fontConfig.GetAppScopeFontRecords();
    std::string removedNames;
    for (const auto &record : appRecords) {
        std::string realPath = SandBoxPathToRealPath(installPath, record.fontPath);
        if (FontManagerUtils::CheckPathExist(realPath)) {
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
    HisyseventAdapter::GetInstance()->CollectUserDataSize(installPath);
    if (!appRecords.empty()) {
        StorageManagerAdapter::GetInstance()->ReportFontBundleStats(userId, installPath);
        FontEventPublish::PublishFontUpdate(FontEventType::UNINSTALL, removedNames, userId);
    }
    FONT_LOGI("CleanupAppScopeFontsByUser done, userId=%{public}d, count=%{public}zu",
        userId, appRecords.size());
    return ERR_OK;
}
} // namespace FontManager
} // namespace Global
} // namespace OHOS
