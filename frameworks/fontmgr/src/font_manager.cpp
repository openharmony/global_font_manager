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

#include <fcntl.h>
#include <string_ex.h>
#include "font_hilog.h"
#include "font_event_publish.h"
#include "font_config.h"
#include "file_utils.h"
#include "hisysevent_adapter.h"
#include "text/font_mgr.h"
#include "directory_ex.h"
#ifdef ACCOUNT_ENABLE
#include "os_account_manager.h"
#endif

namespace OHOS {
namespace Global {
namespace FontManager {
using namespace Rosen::Drawing;
static constexpr int32_t MAX_INSTALL_NUM = 200;
static constexpr int32_t NUM_TWO = 2;
static constexpr int32_t COPY_SPEED = 5 * 1024 / 60; // Mb/s
static constexpr int32_t MAX_TRIGGER_COUNT = 100;
static constexpr double EPSILON = 0.5;
FontManager::FontManager()
{
}

FontManager::~FontManager()
{
}

bool FontManager::CheckAndInitInstallPath(const std::string &installPath)
{
    // 若不存在当前用户的字体文件夹，新建对应文件夹
    if (!FileUtils::CheckPathExist(installPath)) {
        if (!FileUtils::CreatDirWithPermission(installPath)) {
            return false;
        }
    }
    std::string installTempPath = installPath + TEMP_FILE;
    if (!FileUtils::CheckPathExist(installTempPath)) {
        if (!FileUtils::CreatDirWithPermission(installTempPath)) {
            return false;
        }
    }
    return true;
}

bool FontManager::CheckFontConfigPath(const std::string &installPath)
{
    if (FileUtils::CheckPathExist(installPath + FONT_CONFIG_FILE)) {
        return true;
    }
    std::string font_list = R"({
        "fontlist": []
    })";
    return FileUtils::CreateFileWithPermission(installPath + FONT_CONFIG_FILE, font_list);
}

int32_t FontManager::InstallFont(const int32_t &fd, const int32_t userId)
{
    std::string installPath = INSTALL_PATH_PREFIX + std::to_string(userId) + "/";
    if (!(CheckAndInitInstallPath(installPath) && CheckFontConfigPath(installPath))) {
        return ERR_FILE_NOT_EXISTS;
    }

    std::vector<std::string> fullNameVector = GetFontFullName(fd);
    if (fullNameVector.size() == 0) {
        FONT_LOGE("get fontFullName failed, font file verified failed");
        return ERR_FILE_VERIFY_FAIL;
    }

    // 判断字体文件是否已安装
    FontConfig fontConfig(installPath + FONT_CONFIG_FILE);
    for (const auto &fullName : fullNameVector) {
        std::string path = fontConfig.GetFontFileByName(fullName);
        if (!path.empty() && !FileUtils::CheckPathExist(GetRealPath(installPath, path))) {
            if (!fontConfig.DeleteFontRecord(path)) {
                FONT_LOGE("update install_fontconfig fail");
                return ERR_INSTALL_FAIL;
            }
            break;
        }
        if (!path.empty()) {
            FONT_LOGI("Font already installed");
            return ERR_INSTALLED_ALRADY;
        }
    }
    // 判断是否超过最大安装数量
    if (fontConfig.GetInstalledFontsNum() >= MAX_INSTALL_NUM) {
        FONT_LOGI("installed files reach 200, not allowed to install more");
        return ERR_MAX_FILE_COUNT;
    }
    // 将字体文件拷贝到目标目录
    std::string sourcePath = FileUtils::GetFilePathByFd(fd);
    std::string fileName = FileUtils::GetFileName(sourcePath);
    std::string destPath = CopyFileForInstall(installPath, fileName, fd);
    if (destPath.empty()) {
        FONT_LOGE("copy file %{public}s error", sourcePath.c_str());
        return ERR_COPY_FAIL;
    }
    // 写入至json内的文件路径为应用沙箱路径
    std::string realFileName = FileUtils::GetFileName(destPath);
    std::string jsonPath = INSTALL_PATH_APP + realFileName;
    HisyseventAdapter::GetInstance()->CollectUserDataSize();
    if (!fontConfig.InsertFontRecord(jsonPath, fullNameVector)) {
        FONT_LOGE("update install_fontconfig fail, fileName = %{public}s", realFileName.c_str());
        return ERR_INSTALL_FAIL;
    }
    FontEventPublish::PublishFontUpdate(FontEventType::INSTALL, GetFormatFullName(fullNameVector), userId);
    return ERR_OK;
}

std::string FontManager::GetFormatFullName(const std::vector<std::string> &fullNameVector)
{
    std::string FormatFullName;
    std::string split = ",";
    for (const auto &name : fullNameVector) {
        FormatFullName += name + split;
    }
    return FormatFullName.substr(0, FormatFullName.size() - split.size());
}

std::vector<std::string> FontManager::GetFontFullName(const int32_t &fd)
{
    // 调用字体引擎接口校验字体格式
    std::vector<std::string> fullNameVector;
    std::vector<FontByteArray> fullNameVec;
    std::shared_ptr<FontMgr> fontMgr = FontMgr::CreateDefaultFontMgr();
    if (fontMgr == nullptr) {
        FONT_LOGE("fontMgr is null");
        return fullNameVector;
    }

    int ret = fontMgr->GetFontFullName(fd, fullNameVec);
    if (ret != ERR_OK) {
        FONT_LOGE("GetFontFullName failed, err:%{public}d", ret);
        return fullNameVector;
    }

    for (const auto &name : fullNameVec) {
        if (name.strData && name.strLen > 0) {
            std::string fullnameStr = Utf16BEToUtf8(name.strData.get(), name.strLen);
            FONT_LOGI("GetFontFullname, fullnameStr:%{public}s", fullnameStr.c_str());
            fullNameVector.emplace_back(std::move(fullnameStr));
        }
    }
    return fullNameVector;
}

std::string FontManager::Utf16BEToUtf8(const uint8_t* data, size_t byteLen)
{
    std::u16string utf16Str;
    for (size_t i = 0; i + 1 < byteLen; i += NUM_TWO) {
        uint16_t ch = (data[i] << 8) | data[i + 1];
        utf16Str.push_back(static_cast<char16_t>(ch));
    }
    // Convert to UTF-8
    return Str16ToStr8(utf16Str);
}

bool FontManager::CopyFileForDataMigration(const std::string &srcPath, const int32_t userId)
{
    std::string fileName = FileUtils::GetFileName(srcPath);
    std::string tempPath = INSTALL_PATH_PREFIX + TEMP_FILE + fileName;
    std::string desPath = INSTALL_PATH_PREFIX + std::to_string(userId) + "/" + fileName;
    if (FileUtils::CheckPathExist(desPath)) {
        FONT_LOGI("CopyFileForDataMigration path is exist(%{public}s) ", fileName.c_str());
        return true;
    }
    int fd = open(srcPath.c_str(), O_RDONLY);
    if (fd < 0) {
        FONT_LOGE("CopyFileForDataMigration pen font file failed, errno: %{public}d", errno);
        return false;
    }

    if (!FileUtils::CopyFile(fd, tempPath)) {
        FONT_LOGE("CopyFileForDataMigration copy file %{public}s error", fileName.c_str());
        close(fd);
        return false;
    }
    if (!FileUtils::RenameFile(tempPath, desPath)) {
        FONT_LOGE("CopyFileForDataMigration rename file %{public}s error", fileName.c_str());
        FileUtils::RemoveFile(tempPath);
        close(fd);
        return false;
    }
    close(fd);
    return true;
}

std::string FontManager::CopyFileForInstall(const std::string &installPath, const std::string &fileName,
    const int32_t &fd)
{
    std::string tempPath = installPath + TEMP_FILE + fileName;
    if (!FileUtils::CopyFile(fd, tempPath)) {
        FONT_LOGE("copy file %{public}s error", tempPath.c_str());
        return "";
    }

    std::string destPath = installPath + fileName;
    if (FileUtils::CheckPathExist(destPath)) {
        std::string split = "_";
        destPath = installPath + FileUtils::GetFileTime() + split + fileName;
        FONT_LOGI("target file name is exist, store the file with a new name (%{public}s)", destPath.c_str());
    }
    if (!FileUtils::RenameFile(tempPath, destPath)) {
        FONT_LOGE("rename file %{public}s error", fileName.c_str());
        FileUtils::RemoveFile(tempPath);
        return "";
    }
    return destPath;
}

int32_t FontManager::UninstallFont(const std::string &fontFullName, const int32_t userId)
{
    FONT_LOGI("FontManager UninstallFont: %{public}s, userId:%{public}d", fontFullName.c_str(), userId);
    std::string installPath = INSTALL_PATH_PREFIX + std::to_string(userId) + "/";
    if (fontFullName.empty()) {
        FONT_LOGE("FontManager::UninstallFont, fontName is empty");
        return ERR_UNINSTALL_FILE_NOT_EXISTS;
    }
    FontConfig fontConfig(installPath + FONT_CONFIG_FILE);
    std::string path = fontConfig.GetFontFileByName(fontFullName);
    if (path.empty()) {
        FONT_LOGE("Can't find fontFullName = %{public}s", fontFullName.c_str());
        return ERR_UNINSTALL_FILE_NOT_EXISTS;
    }
    HisyseventAdapter::GetInstance()->CollectUserDataSize();
    std::string realPath = GetRealPath(installPath, path);
    if (!FileUtils::RemoveFile(realPath)) {
        return ERR_UNINSTALL_REMOVE_FAIL;
    }
    if (!fontConfig.DeleteFontRecord(path)) {
        FONT_LOGE("update install_fontconfig fail, path = %{public}s", path.c_str());
        return ERR_UNINSTALL_FAIL;
    }
    FontEventPublish::PublishFontUpdate(FontEventType::UNINSTALL, fontFullName, userId);
    return ERR_OK;
}

std::vector<int32_t> FontManager::GetAllCreatedUserIds()
{
    std::vector<int32_t> allUserIds;
#ifdef ACCOUNT_ENABLE
    std::vector<AccountSA::OsAccountInfo> osAccountInfos;
    ErrCode ret = AccountSA::OsAccountManager::QueryAllCreatedOsAccounts(osAccountInfos);
    if (ret != ERR_OK || osAccountInfos.empty()) {
        FONT_LOGE("FontManager::GetAllCreatedUserIds failed.err=%{public}d", ret);
    }
    for (const auto &info : osAccountInfos) {
        allUserIds.push_back(info.GetLocalId());
    }
    return allUserIds;
#else
    FONT_LOGI("FontManager osAccount not support.");
    return allUserIds;
#endif
}

int32_t FontManager::DataMigrationInner(const RemoteCallbackPtr& callback)
{
    if (OHOS::IsEmptyFolder(INSTALL_PATH_APP)) {
        FONT_LOGE("FontManager INSTALL_PATH_APP is empty.");
        return ERR_NOT_NEED_DATA_MIGRATION;
    }
    std::vector<int32_t> userIds = GetAllCreatedUserIds();
    if (userIds.empty()) {
        FONT_LOGE("FontManager userIds empty.");
        return ERR_SYSTEM_ERROR;
    }
    if (!InitAllUserDir(userIds) || !InitDataMigrationTempDir()) {
        FONT_LOGE("FontManager DataMigrationInner InitDir err.");
        return ERR_SYSTEM_ERROR;
    }
    std::vector<std::string> paths;
    OHOS::GetDirFiles(INSTALL_PATH_APP, paths);
    EventDataBeginCallback(callback);
    for (size_t i = 0; i < paths.size(); ++i) {
        if (ShouldCallback(i, paths.size())) {
            EventDataProgressCallback(i, paths.size(), userIds.size(), callback);
        }
        int ret = StartOneFileCopyTask(paths[i], userIds);
        if (ret != ERR_OK) {
            return ERR_SYSTEM_ERROR;
        }
        FONT_LOGI("FontManager::FileCopyTask suc.FileName:%{public}s.", FileUtils::GetFileName(paths[i]).c_str());
    }
    FileUtils::DeleteDir(INSTALL_PATH_PREFIX + TEMP_FILE, true);
    return ERR_OK;
}

void FontManager::DataMigration(const RemoteCallbackPtr& callback)
{
    FileUtils::DeleteDir(INSTALL_PATH_APP + TEMP_FILE, true);
    int32_t ret = DataMigrationInner(callback);
    EventDataResultCallback(ret, callback);
}

int32_t FontManager::StartOneFileCopyTask(const std::string& path, const std::vector<int32_t>& userIds)
{
    for (const auto& userId : userIds) {
        if (!CopyFileForDataMigration(path, userId)) {
            FONT_LOGE("StartOneFileCopyTask copy file %{public}s error", FileUtils::GetFileName(path).c_str());
            return ERR_SYSTEM_ERROR;
        }
    }
    if (!FileUtils::RemoveFile(path)) {
        FONT_LOGE("StartOneFileCopyTask RemoveFile file (%{public}s) error", FileUtils::GetFileName(path).c_str());
        return ERR_SYSTEM_ERROR;
    }
    return ERR_OK;
}

bool FontManager::InitAllUserDir(const std::vector<int32_t> userIds)
{
    for (const auto& userId : userIds) {
        std::string path = INSTALL_PATH_PREFIX + std::to_string(userId) + "/";
        if (!FileUtils::CreatDirWithPermission(path)) {
            FONT_LOGE("InitAllUserDir CreatDirWithPermission err. userid = %{public}d.", userId);
            return false;
        }
    }
    return true;
}

bool FontManager::InitDataMigrationTempDir()
{
    std::string tempPath = INSTALL_PATH_PREFIX + TEMP_FILE;
    if (!FileUtils::CheckPathExist(tempPath)) {
        if (!FileUtils::CreatDirWithPermission(tempPath)) {
            return false;
        }
    }
    return true;
}

std::string FontManager::GetRealPath(const std::string &installPath, const std::string &path)
{
    std::string fileName = FileUtils::GetFileName(path);
    return installPath + fileName;
}

void FontManager::EventDataBeginCallback(const RemoteCallbackPtr& callback)
{
    EventData eventData = {.event = ProgressType::START, .timeRemain = 0, .progressRate = 0, .progressResult = 0};
    RefreshEventData(eventData, callback);
}

void FontManager::EventDataProgressCallback(int32_t i, int32_t size, int32_t idsize, const RemoteCallbackPtr& callback)
{

    std::uintmax_t remainSize = OHOS::GetFolderSize(INSTALL_PATH_APP) * idsize >> 20;
    int32_t timeRemain = remainSize / COPY_SPEED;
    if (timeRemain < 1) {
        timeRemain = 1;
    }
    int32_t progressRate =
        i == 0 ? i : static_cast<int32_t>((static_cast<int64_t>(i) * MAX_TRIGGER_COUNT + size / 2) / size);
    EventData eventData = {.event = ProgressType::PROGRESS_DOING,
                            .timeRemain = timeRemain,
                            .progressRate = progressRate,
                            .progressResult = 0};
    RefreshEventData(eventData, callback);
}

void FontManager::EventDataResultCallback(int32_t result, const RemoteCallbackPtr& callback)
{
    EventData eventData = {.event = ProgressType::PROGRESS_RESULT,
                            .timeRemain = 0,
                            .progressRate = 0,
                            .progressResult = result};
    RefreshEventData(eventData, callback);
}

void FontManager::EventDataHeartBeatCallback(const RemoteCallbackPtr& callback)
{
    EventData eventData = {.event = ProgressType::HEART_BEAT,
                            .timeRemain = 0,
                            .progressRate = 0,
                            .progressResult = 0};
    RefreshEventData(eventData, callback);
}

bool FontManager::ShouldCallback(int32_t i, int32_t totalCount)
{
    if (totalCount <= MAX_TRIGGER_COUNT) {
        return true;
    }
    double step = static_cast<double>(totalCount) / static_cast<double>(MAX_TRIGGER_COUNT);
    int32_t expectedTriggerIndex = static_cast<int32_t>(std::round(i / step));
    double triggerPos = expectedTriggerIndex * step;
    return std::fabs(i - triggerPos) < EPSILON;
}

void FontManager::RefreshEventData(const EventData& eventData, const RemoteCallbackPtr& callback)
{
    sptr<IRemoteObject> object = callback->AsObject();
    if (object != nullptr) {
        callback->Handle(ERR_OK, std::move(eventData));
    }
}
} // namespace FontManager
} // namespace Global
} // namespace OHOS
