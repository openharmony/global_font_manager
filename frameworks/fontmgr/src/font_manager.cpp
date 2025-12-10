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
#include "font_define.h"

namespace OHOS {
namespace Global {
namespace FontManager {
static constexpr int32_t MAX_INSTALL_NUM = 200;
FontManager::FontManager()
{
}

FontManager::~FontManager()
{
}

int32_t FontManager::InstallFont(const int32_t fd, const int32_t userId)
{
    std::string installPath = INSTALL_PATH_PREFIX + std::to_string(userId) + INSTALL_PATH_SUFFIX;
    if (!(FontManagerUtils::CheckAndInitInstallPath(installPath) &&
        FontManagerUtils::CheckFontConfigPath(installPath))) {
        return ERR_FILE_NOT_EXISTS;
    }
    if (configMap_.find(userId) == configMap_.end()) {
        configMap_.emplace(userId, FontConfig(installPath + FONT_CONFIG_FILE));
    }
    auto& fontConfig = configMap_.at(userId);
    if (!fontConfig.CheckAndUpdateFontRecord()) {
        FONT_LOGE("CheckAndUpdateFontRecord fail");
        return ERR_INSTALL_FAIL;
    }
    std::vector<std::string> fullNameVector = FontManagerUtils::GetFullNamesByFd(fd);
    if (fullNameVector.size() == 0) {
        return ERR_FILE_VERIFY_FAIL;
    }
    for (const auto &fullName : fullNameVector) {
        std::string findPath = fontConfig.GetFontFileByName(fullName);
        if (!findPath.empty() && !FontManagerUtils::CheckPathExist(SandBoxPathToRealPath(installPath, findPath))) {
            if (!fontConfig.DeleteFontRecord(findPath)) {
                FONT_LOGE("fix install_fontconfig fail");
                return ERR_INSTALL_FAIL;
            }
            break;
        }
        if (!findPath.empty()) {
            return ERR_INSTALLED_ALRADY;
        }
    }
    if (fontConfig.GetInstalledFontsNum() >= MAX_INSTALL_NUM) {
        return ERR_MAX_FILE_COUNT;
    }
    std::string sourcePath = FontManagerUtils::GetFilePathByFd(fd);
    std::string fileName = FontManagerUtils::GetFileName(sourcePath);
    std::string destPath = CopyFileForInstall(installPath, fileName, fd);
    if (destPath.empty()) {
        FONT_LOGE("copy file %{public}s error", sourcePath.c_str());
        return ERR_COPY_FAIL;
    }
    std::string realFileName = FontManagerUtils::GetFileName(destPath);
    std::string jsonPath = INSTALL_PATH_APP + realFileName;
    if (!fontConfig.InsertFontRecord(jsonPath, fullNameVector)) {
        FontManagerUtils::DeleteDir(destPath, true);
        FONT_LOGE("update install_fontconfig fail, fileName = %{public}s", realFileName.c_str());
        return ERR_INSTALL_FAIL;
    }
    HisyseventAdapter::GetInstance()->CollectUserDataSize(installPath);
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
    return FormatFullName.substr(0, FormatFullName.size() - split.size());
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
    if (configMap_.find(userId) == configMap_.end()) {
        configMap_.emplace(userId, FontConfig(installPath + FONT_CONFIG_FILE));
    }
    auto& fontConfig = configMap_.at(userId);
    if (!fontConfig.CheckAndUpdateFontRecord()) {
        FONT_LOGE("CheckAndUpdateFontRecord fail");
        return ERR_UNINSTALL_FAIL;
    }
    std::string path = fontConfig.GetFontFileByName(fontFullName);
    if (path.empty()) {
        FONT_LOGE("Can't find fontFullName = %{public}s", fontFullName.c_str());
        return ERR_UNINSTALL_FILE_NOT_EXISTS;
    }
    std::string realPath = SandBoxPathToRealPath(installPath, path);
    if (!FontManagerUtils::RemoveFile(realPath)) {
        return ERR_UNINSTALL_REMOVE_FAIL;
    }
    if (!fontConfig.DeleteFontRecord(path)) {
        FONT_LOGE("update install_fontconfig fail, path = %{public}s", path.c_str());
        return ERR_UNINSTALL_FAIL;
    }
    HisyseventAdapter::GetInstance()->CollectUserDataSize(installPath);
    FontEventPublish::PublishFontUpdate(FontEventType::UNINSTALL, fontFullName, userId);
    FONT_LOGI("Uninstall font success, fontFullName:%{public}s, userId:%{public}d", fontFullName.c_str(), userId);
    return ERR_OK;
}

std::string FontManager::SandBoxPathToRealPath(const std::string &installPath, const std::string &path)
{
    std::string fileName = FontManagerUtils::GetFileName(path);
    return installPath + fileName;
}
} // namespace FontManager
} // namespace Global
} // namespace OHOS
