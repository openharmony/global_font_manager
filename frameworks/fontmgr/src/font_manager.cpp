/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "font_hilog.h"
#include "font_event_publish.h"
#include "font_config.h"
#include "file_utils.h"
#include "SkFontMgr.h"

namespace OHOS {
namespace Global {
namespace FontManager {
static const std::string INSTALL_PATH = "/data/service/el1/public/for-all-app/fonts/";
static const std::string FONT_CONFIG_FILE = INSTALL_PATH + "install_fontconfig.json";
static const std::string FONTS_TEMP_PATH = "/data/service/el1/public/for-all-app/fonts/temp/";
static constexpr int32_t MAX_INSTALL_NUM = 200;
FontManager::FontManager()
{
}

FontManager::~FontManager()
{
}

bool FontManager::CheckInstallPath()
{
    if (!FileUtils::CheckPathExist(INSTALL_PATH)) {
        return false;
    }
    if (!FileUtils::CheckPathExist(FONTS_TEMP_PATH)) {
        return FileUtils::CreatDirWithPermission(FONTS_TEMP_PATH);
    }
    return true;
}

bool FontManager::CheckFontConfigPath()
{
    if (FileUtils::CheckPathExist(FONT_CONFIG_FILE)) {
        return true;
    }
    std::string font_list = R"({
        "fontlist": []
    })";
    return FileUtils::CreateFileWithPermission(FONT_CONFIG_FILE, font_list);
}

int32_t FontManager::InstallFont(const int32_t &fd)
{
    if (!(CheckInstallPath() && CheckFontConfigPath())) {
        return ERR_FILE_NOT_EXISTS;
    }
    
    std::vector<std::string> fullNameVector = GetFontFullName(fd);
    if (fullNameVector.size() == 0) {
        FONT_LOGE("get fontFullName failed, font file verified failed");
        return ERR_FILE_VERIFY_FAIL;
    }

    // 判断字体文件是否已安装
    FontConfig fontConfig(FONT_CONFIG_FILE);
    for (const auto &fullName : fullNameVector) {
        std::string path = fontConfig.GetFontFileByName(fullName);
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
    std::string destPath = CopyFile(sourcePath, fd);
    if (destPath.empty()) {
        FONT_LOGE("copy file %{public}s error", sourcePath.c_str());
        return ERR_COPY_FAIL;
    }
    if (fontConfig.InsertFontRecord(destPath, fullNameVector)) {
        FontEventPublish::PublishFontUpdate(FontEventType::INSTALL, GetFormatFullName(fullNameVector));
        return SUCCESS;
    }
    return ERR_INSTALL_FAIL;
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
    std::vector<SkByteArray> fullname;
    sk_sp<SkFontMgr> fontMgr = SkFontMgr::RefDefault();
    if (fontMgr == nullptr) {
        FONT_LOGE("fontMgr is null");
        return fullNameVector;
    }
 
    int ret = fontMgr->GetFontFullName(fd, fullname);
    if (ret != SUCCESS) {
        FONT_LOGE("GetFontFullName failed, err:%{public}d", ret);
        return fullNameVector;
    }
    
    for (const auto &name : fullname) {
        std::string fullnameStr;
        fullnameStr.assign((char *)name.strData.get(), name.strLen);
        fullnameStr.erase(std::remove(fullnameStr.begin(), fullnameStr.end(), '\0'), fullnameStr.end());
        FONT_LOGI("GetFontFullname, fullnameStr:%{public}s", fullnameStr.c_str());
        fullNameVector.emplace_back(std::move(fullnameStr));
    }
    return fullNameVector;
}

std::string FontManager::CopyFile(const std::string &sourcePath, const int32_t &fd)
{
    std::string fileName = FileUtils::GetFileName(sourcePath);
    std::string tempPath = FONTS_TEMP_PATH + fileName;

    if (!FileUtils::CopyFile(fd, tempPath)) {
        FONT_LOGE("copy file %{public}s error", tempPath.c_str());
        return "";
    }

    std::string destPath = INSTALL_PATH + fileName;
    if (FileUtils::CheckPathExist(destPath)) {
        std::string split = "_";
        destPath = INSTALL_PATH + FileUtils::GetFileTime() + split + fileName;
        FONT_LOGI("target file name is exist, store the file with a new name (%{public}s)", destPath.c_str());
    }
    if (!FileUtils::RenameFile(tempPath, destPath)) {
        FONT_LOGE("rename file %{public}s error", sourcePath.c_str());
        FileUtils::RemoveFile(tempPath);
        return "";
    }
    return destPath;
}

int32_t FontManager::UninstallFont(const std::string &fontFullName)
{
    FONT_LOGI("FontManager UninstallFont: %{public}s", fontFullName.c_str());
    if (fontFullName.empty()) {
        FONT_LOGE("FontManager::UninstallFont, fontName is empty");
        return ERR_UNINSTALL_FILE_NOT_EXISTS;
    }
    FontConfig fontConfig(FONT_CONFIG_FILE);
    std::string path = fontConfig.GetFontFileByName(fontFullName);
    if (path.empty()) {
        FONT_LOGE("Can't find fontFullName = %{public}s", fontFullName.c_str());
        return ERR_UNINSTALL_FILE_NOT_EXISTS;
    }
    if (!FileUtils::RemoveFile(path)) {
        return ERR_UNINSTALL_REMOVE_FAIL;
    }
    if (!fontConfig.DeleteFontRecord(path)) {
        FONT_LOGE("update install_fontconfig fail");
        return ERR_UNINSTALL_FAIL;
    }
    FontEventPublish::PublishFontUpdate(FontEventType::UNINSTALL, fontFullName);
    return SUCCESS;
}
} // namespace FontManager
} // namespace Global
} // namespace OHOS
