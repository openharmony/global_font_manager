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

#include <string_ex.h>
#include "font_hilog.h"
#include "font_event_publish.h"
#include "font_manager_utils.h"
#include "hisysevent_adapter.h"
#include "text/font_mgr.h"
namespace OHOS {
namespace Global {
namespace FontManager {
using namespace Rosen::Drawing;
static constexpr int32_t MAX_INSTALL_NUM = 200;
static constexpr int32_t NUM_TWO = 2;
FontManager::FontManager()
{
}

FontManager::~FontManager()
{
}

int32_t FontManager::InstallFont(const int32_t &fd, const int32_t userId)
{
    std::string installPath = INSTALL_PATH_PREFIX + std::to_string(userId) + "/";
    if (!(FontManagerUtils::CheckAndInitInstallPath(installPath) &&
        FontManagerUtils::CheckFontConfigPath(installPath))) {
        return ERR_FILE_NOT_EXISTS;
    }

    std::vector<std::string> fullNameVector = GetFontFullName(fd);
    if (fullNameVector.size() == 0) {
        FONT_LOGE("get fontFullName failed, font file verified failed");
        return ERR_FILE_VERIFY_FAIL;
    }

    // 判断字体文件是否已安装
    if (configMap_.find(userId) == configMap_.end()) {
        configMap_.emplace(userId, FontConfig(installPath + FONT_CONFIG_FILE));
    }
    auto& fontConfig = configMap_.at(userId);
    for (const auto &fullName : fullNameVector) {
        std::string path = fontConfig.GetFontFileByName(fullName);
        if (!path.empty() && !FontManagerUtils::CheckPathExist(GetRealPath(installPath, path))) {
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
    std::string sourcePath = FontManagerUtils::GetFilePathByFd(fd);
    std::string fileName = FontManagerUtils::GetFileName(sourcePath);
    std::string destPath = CopyFileForInstall(installPath, fileName, fd);
    if (destPath.empty()) {
        FONT_LOGE("copy file %{public}s error", sourcePath.c_str());
        return ERR_COPY_FAIL;
    }
    // 写入至json内的文件路径为应用沙箱路径
    std::string realFileName = FontManagerUtils::GetFileName(destPath);
    std::string jsonPath = INSTALL_PATH_APP + realFileName;
    HisyseventAdapter::GetInstance()->CollectUserDataSize();
    if (!fontConfig.InsertFontRecord(jsonPath, fullNameVector)) {
        FontManagerUtils::DeleteDir(destPath, true);
        FONT_LOGE("update install_fontconfig fail, fileName = %{public}s", realFileName.c_str());
        return ERR_INSTALL_FAIL;
    }
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
    std::string installPath = INSTALL_PATH_PREFIX + std::to_string(userId) + "/";
    if (fontFullName.empty()) {
        FONT_LOGE("FontManager::UninstallFont, fontName is empty");
        return ERR_UNINSTALL_FILE_NOT_EXISTS;
    }
    if (configMap_.find(userId) == configMap_.end()) {
        configMap_.emplace(userId, FontConfig(installPath + FONT_CONFIG_FILE));
    }
    auto& fontConfig = configMap_.at(userId);
    std::string path = fontConfig.GetFontFileByName(fontFullName);
    if (path.empty()) {
        FONT_LOGE("Can't find fontFullName = %{public}s", fontFullName.c_str());
        return ERR_UNINSTALL_FILE_NOT_EXISTS;
    }
    HisyseventAdapter::GetInstance()->CollectUserDataSize();
    std::string realPath = GetRealPath(installPath, path);
    if (!FontManagerUtils::RemoveFile(realPath)) {
        return ERR_UNINSTALL_REMOVE_FAIL;
    }
    if (!fontConfig.DeleteFontRecord(path)) {
        FONT_LOGE("update install_fontconfig fail, path = %{public}s", path.c_str());
        return ERR_UNINSTALL_FAIL;
    }
    FontEventPublish::PublishFontUpdate(FontEventType::UNINSTALL, fontFullName, userId);
    FONT_LOGI("Uninstall font success, fontFullName:%{public}s, userId:%{public}d", fontFullName.c_str(), userId);
    return ERR_OK;
}

std::string FontManager::GetRealPath(const std::string &installPath, const std::string &path)
{
    std::string fileName = FontManagerUtils::GetFileName(path);
    return installPath + fileName;
}
} // namespace FontManager
} // namespace Global
} // namespace OHOS
