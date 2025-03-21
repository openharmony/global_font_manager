/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
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

#include "font_config.h"

#include "font_hilog.h"
#include "securec.h"

namespace OHOS {
namespace Global {
namespace FontManager {
static const char *FONT_PATH = "fontfullpath";
static const char *FONT_FULL_NAME = "fullname";

bool FontConfig::InsertFontRecord(const std::string &fontPath, const std::vector<std::string> &fullNames)
{
    cJSON *jsonValue = cJSON_Parse(CheckConfigFile(ConfigPath_).c_str());
    if (jsonValue == nullptr) {
        FONT_LOGE("heck config file failed");
        return false;
    }
    cJSON *fontList = cJSON_GetObjectItem(jsonValue, "fontlist");
    if (fontList == nullptr) {
        FONT_LOGE("Font Config file's format is incorrect");
        cJSON_Delete(jsonValue);
        return false;
    }
    cJSON *insertValue = ConstructCJSON(fontPath, fullNames);
    if (insertValue == nullptr) {
        cJSON_Delete(jsonValue);
        return false;
    }
    cJSON_AddItemToArray(fontList, insertValue);
    char *fileData = cJSON_Print(jsonValue);
    cJSON_Delete(jsonValue);
    return WriteToFile(fileData);
}

bool FontConfig::DeleteFontRecord(const std::string &fontPath)
{
    std::lock_guard<std::mutex> lock(fontsMapLock_);
    if (fontsMap_.size() == 0) {
        fontsMap_ = GetFontsMap(ConfigPath_);
    }
    if (fontsMap_.find(fontPath) == fontsMap_.end()) {
        return false;
    }
    fontsMap_.erase(fontPath);

    cJSON *rootData = cJSON_CreateArray();
    if (rootData == nullptr) {
        return false;
    }
    for (const auto &info : fontsMap_) {
        cJSON *item = cJSON_CreateObject();
        if (item == nullptr) {
            continue;
        }
        cJSON_AddItemToObject(item, FONT_PATH, cJSON_CreateString(info.first.c_str()));
        cJSON *arrData = cJSON_CreateArray();
        if (arrData == nullptr) {
            cJSON_Delete(item);
            continue;
        }
        for (const auto &name : info.second) {
            cJSON_AddItemToArray(arrData, cJSON_CreateString(name.c_str()));
        }
        cJSON_AddItemToObject(item, FONT_FULL_NAME, arrData);
        cJSON_AddItemToArray(rootData, item);
    }
    cJSON *cJsonData = cJSON_CreateObject();
    if (cJsonData == nullptr) {
        cJSON_Delete(rootData);
        return false;
    }
    cJSON_AddItemToObject(cJsonData, "fontlist", rootData);
    char *fileData = cJSON_Print(cJsonData);
    cJSON_Delete(cJsonData);
    return WriteToFile(fileData);
}

int FontConfig::GetInstalledFontsNum()
{
    std::lock_guard<std::mutex> lock(fontsMapLock_);
    if (fontsMap_.size() == 0) {
        fontsMap_ = GetFontsMap(ConfigPath_);
    }
    return fontsMap_.size();
}
 
std::string FontConfig::GetFontFileByName(const std::string &fullName)
{
    std::lock_guard<std::mutex> lock(fontsMapLock_);
    if (fontsMap_.size() == 0) {
        fontsMap_ = GetFontsMap(ConfigPath_);
    }
    for (const auto &font : fontsMap_) {
        for (const auto &name : font.second) {
            if (name == fullName) {
                return font.first;
            }
        }
    }
    return "";
}

char *FontConfig::GetFileData(const std::string &filePath, int &size)
{
    std::lock_guard<std::mutex> lock(configLock_);
    FILE *fp = std::fopen(filePath.c_str(), "r");
    if (fp == nullptr) {
        FONT_LOGE("failed open the filePath = %{public}s", filePath.c_str());
        return nullptr;
    }
    std::fseek(fp, 0, SEEK_END);
    size = ftell(fp) + 1;
    rewind(fp);
    char *data = static_cast<char *>(malloc(size));
    if (data == nullptr) {
        FONT_LOGE("failed malloc in GetFileData");
        fclose(fp);
        fp = nullptr;
        return nullptr;
    }
    memset_s(data, size, 0, size);
    std::fread(data, size, 1, fp);
    fclose(fp);
    fp = nullptr;
    return data;
}

bool FontConfig::WriteToFile(char *fileData)
{
    std::lock_guard<std::mutex> lock(configLock_);
    if (fileData == nullptr) {
        return false;
    }
    FILE *fp = fopen(ConfigPath_.c_str(), "w");
    if (fp == nullptr) {
        FONT_LOGE("failed open the filePath = %{public}s", ConfigPath_.c_str());
        free(fileData);
        fileData = nullptr;
        return false;
    }

    bool ret = true;
    if (std::fwrite(fileData, sizeof(char), strlen(fileData), fp) != strlen(fileData)) {
        FONT_LOGE("failed to write file");
        ret = false;
    }
    if (fileData != nullptr) {
        free(fileData);
        fileData = nullptr;
    }
    fclose(fp);
    fp = nullptr;
    return ret;
}

std::string FontConfig::CheckConfigFile(const std::string &fontPath)
{
    int size = 0;
    char *data = GetFileData(fontPath, size);
    if (data == nullptr) {
        FONT_LOGE("data is NULL");
        return "";
    }
    std::string pramsString;
    pramsString.assign(data, size);
    free(data);
    data = nullptr;
    return pramsString;
}

cJSON *FontConfig::ConstructCJSON(const std::string &fontFullPath, const std::vector<std::string> &fullName)
{
    cJSON *jsonData = cJSON_CreateObject();
    if (jsonData == nullptr) {
        return nullptr;
    }
    cJSON_AddStringToObject(jsonData, FONT_PATH, fontFullPath.c_str());
    cJSON *fullNameJson = cJSON_CreateArray();
    if (fullNameJson == nullptr) {
        cJSON_Delete(jsonData);
        return nullptr;
    }
    for (const auto &name : fullName) {
        cJSON_AddItemToArray(fullNameJson, cJSON_CreateString(name.c_str()));
    }
    cJSON_AddItemToObject(jsonData, FONT_FULL_NAME, fullNameJson);
    return jsonData;
}

std::unordered_map<std::string, std::vector<std::string>> FontConfig::GetFontsMap(const std::string &fontPath)
{
    cJSON *jsonData = cJSON_Parse(CheckConfigFile(fontPath).c_str());
    if (jsonData == nullptr) {
        FONT_LOGE("heck config file failed");
        return std::unordered_map<std::string, std::vector<std::string>>();
    }
    cJSON *fontList = cJSON_GetObjectItem(jsonData, "fontlist");
    if (fontList == nullptr) {
        cJSON_Delete(jsonData);
        return std::unordered_map<std::string, std::vector<std::string>>();
    }
    std::unordered_map<std::string, std::vector<std::string>> fontMap;
    int fontSize = cJSON_GetArraySize(fontList);
    for (int i = 0; i < fontSize; i++) {
        cJSON *arrItem = cJSON_GetArrayItem(fontList, i);
        cJSON *fullNameValue = cJSON_GetObjectItem(arrItem, FONT_FULL_NAME);
        int fullNameSize = cJSON_GetArraySize(fullNameValue);
        std::vector<std::string> fullNames;
        for (int j = 0; j < fullNameSize; j++) {
            cJSON *fullNameItem = cJSON_GetArrayItem(fullNameValue, j);
            if (fullNameItem != nullptr && fullNameItem->valuestring != nullptr) {
                fullNames.emplace_back(fullNameItem->valuestring);
            }
        }
        cJSON *fontFullPathValue = cJSON_GetObjectItem(arrItem, FONT_PATH);
        if (fontFullPathValue != nullptr && fontFullPathValue->valuestring != nullptr) {
            fontMap.insert(std::make_pair(fontFullPathValue->valuestring, fullNames));
        }
    }
    cJSON_Delete(jsonData);
    return fontMap;
}
}  // namespace FontManager
}  // namespace Global
}  // namespace OHOS