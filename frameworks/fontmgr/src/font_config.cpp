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

#include "font_config.h"
#include "font_hilog.h"
#include "font_define.h"
#include "securec.h"
#include "font_manager_utils.h"

namespace OHOS {
namespace Global {
namespace FontManager {
static const char *FONT_PATH = "fontfullpath";
static const char *FONT_FULL_NAME = "fullname";
static const char *FONT_SCOPE = "scope";
static const char *FONT_SRC_PATH = "srcPath";
static const char *FONT_APP_IDENTIFIER = "appIdentifier";
static const char *FONT_BUNDLE_NAME = "bundleName";
static const char *FONT_VERSION = "version";
static const int32_t VERSION = 1;


std::string FontConfig::SandBoxPathToRealPath(const std::string &path)
{
    std::string fileName = FontManagerUtils::GetFileName(path);
    std::string directory = FontManagerUtils::GetFileDirectory(ConfigPath_);
    return directory + fileName;
}

bool FontConfig::InsertFontRecord(const std::string &fontPath, const std::vector<std::string> &fullNames)
{
    std::lock_guard<std::mutex> lock(configLock_);
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
    std::lock_guard<std::mutex> lock(configLock_);
    cJSON *jsonValue = cJSON_Parse(CheckConfigFile(ConfigPath_).c_str());
    if (jsonValue == nullptr) {
        FONT_LOGE("DeleteFontRecord parse config failed");
        return false;
    }
    cJSON *fontList = cJSON_GetObjectItem(jsonValue, "fontlist");
    if (!cJSON_IsArray(fontList)) {
        cJSON_Delete(jsonValue);
        return false;
    }
    int sz = cJSON_GetArraySize(fontList);
    for (int i = 0; i < sz; i++) {
        cJSON *item = cJSON_GetArrayItem(fontList, i);
        cJSON *pathVal = cJSON_GetObjectItem(item, FONT_PATH);
        if (pathVal && cJSON_IsString(pathVal) && fontPath == pathVal->valuestring) {
            cJSON_DeleteItemFromArray(fontList, i);
            char *fileData = cJSON_Print(jsonValue);
            cJSON_Delete(jsonValue);
            return WriteToFile(fileData);
        }
    }
    cJSON_Delete(jsonValue);
    return false;
}

int FontConfig::GetInstalledFontsNum()
{
    std::lock_guard<std::mutex> lock(configLock_);
    cJSON *jsonValue = cJSON_Parse(CheckConfigFile(ConfigPath_).c_str());
    if (jsonValue == nullptr) {
        return 0;
    }
    cJSON *fontList = cJSON_GetObjectItem(jsonValue, "fontlist");
    int count = cJSON_IsArray(fontList) ? cJSON_GetArraySize(fontList) : 0;
    cJSON_Delete(jsonValue);
    return count;
}
 
static std::string FindFontPathByName(cJSON *fontList, const std::string &fullName)
{
    int sz = cJSON_GetArraySize(fontList);
    for (int i = 0; i < sz; i++) {
        cJSON *item = cJSON_GetArrayItem(fontList, i);
        cJSON *nameArr = cJSON_GetObjectItem(item, FONT_FULL_NAME);
        if (!cJSON_IsArray(nameArr)) {
            continue;
        }
        int nameSz = cJSON_GetArraySize(nameArr);
        for (int j = 0; j < nameSz; j++) {
            cJSON *nameItem = cJSON_GetArrayItem(nameArr, j);
            if (!nameItem || !cJSON_IsString(nameItem) || fullName != nameItem->valuestring) {
                continue;
            }
            cJSON *pathVal = cJSON_GetObjectItem(item, FONT_PATH);
            if (pathVal && cJSON_IsString(pathVal)) {
                return pathVal->valuestring;
            }
            return "";
        }
    }
    return "";
}

std::string FontConfig::GetFontFileByName(const std::string &fullName)
{
    std::lock_guard<std::mutex> lock(configLock_);
    cJSON *jsonValue = cJSON_Parse(CheckConfigFile(ConfigPath_).c_str());
    if (jsonValue == nullptr) {
        return "";
    }
    cJSON *fontList = cJSON_GetObjectItem(jsonValue, "fontlist");
    if (!cJSON_IsArray(fontList)) {
        cJSON_Delete(jsonValue);
        return "";
    }
    std::string result = FindFontPathByName(fontList, fullName);
    cJSON_Delete(jsonValue);
    return result;
}

char *FontConfig::GetFileData(const std::string &filePath, long &size)
{
    FILE *fp = std::fopen(filePath.c_str(), "r");
    if (fp == nullptr) {
        FONT_LOGE("failed open the filePath = %{public}s", filePath.c_str());
        return nullptr;
    }
    std::fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    if (fileSize < 0) {
        FONT_LOGE("failed to get the file size for filePath = %{public}s", filePath.c_str());
        (void)fclose(fp);
        return nullptr;
    }
    if (fileSize >= LONG_MAX) {
        FONT_LOGE("file size too large for filePath = %{public}s", filePath.c_str());
        (void)fclose(fp);
        return nullptr;
    }
    size = fileSize + 1;

    if (size <= 0) {
        FONT_LOGE("invalid file size = %ld for filePath = %{public}s", size, filePath.c_str());
        (void)fclose(fp);
        return nullptr;
    }
    char *data = static_cast<char *>(malloc(size));
    if (data == nullptr) {
        FONT_LOGE("failed malloc in GetFileData for filePath = %{public}s", filePath.c_str());
        (void)fclose(fp);
        return nullptr;
    }
    memset_s(data, size, 0, size);
    std::fseek(fp, 0, SEEK_SET);
    size_t bytesRead = std::fread(data, 1, fileSize, fp);
    if (bytesRead != static_cast<size_t>(fileSize)) {
        FONT_LOGE("failed to read the full file content for filePath = %{public}s", filePath.c_str());
        free(data);
        (void)fclose(fp);
        return nullptr;
    }

    (void)fclose(fp);
    return data;
}

bool FontConfig::WriteToFile(char *fileData)
{
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
    (void)fclose(fp);
    fp = nullptr;
    return ret;
}

std::string FontConfig::CheckConfigFile(const std::string &fontPath)
{
    if (!FontManagerUtils::CheckPathExist(fontPath)) {
        return "";
    }
    long size = 0;
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

// ===== Scope font methods =====

cJSON *FontConfig::ConstructScopeCJSON(const FontRecordInfo &record)
{
    cJSON *item = ConstructCJSON(record.fontPath, record.fullNames);
    if (item == nullptr) {
        return nullptr;
    }
    if (record.scope >= 0) {
        cJSON_AddNumberToObject(item, FONT_SCOPE, record.scope);
    }
    if (!record.srcPath.empty()) {
        cJSON_AddStringToObject(item, FONT_SRC_PATH, record.srcPath.c_str());
    }
    if (!record.appIdentifier.empty()) {
        cJSON_AddStringToObject(item, FONT_APP_IDENTIFIER, record.appIdentifier.c_str());
    }
    if (!record.bundleName.empty()) {
        cJSON_AddStringToObject(item, FONT_BUNDLE_NAME, record.bundleName.c_str());
    }
    return item;
}

static FontRecordInfo ParseScopeRecord(cJSON *arrItem)
{
    FontRecordInfo info;
    cJSON *pathVal = cJSON_GetObjectItem(arrItem, FONT_PATH);
    if (pathVal && cJSON_IsString(pathVal)) {
        info.fontPath = pathVal->valuestring;
    }
    cJSON *nameArr = cJSON_GetObjectItem(arrItem, FONT_FULL_NAME);
    if (nameArr && cJSON_IsArray(nameArr)) {
        int sz = cJSON_GetArraySize(nameArr);
        for (int j = 0; j < sz; j++) {
            cJSON *n = cJSON_GetArrayItem(nameArr, j);
            if (n && cJSON_IsString(n)) {
                info.fullNames.emplace_back(n->valuestring);
            }
        }
    }
    cJSON *scopeVal = cJSON_GetObjectItem(arrItem, FONT_SCOPE);
    if (scopeVal && cJSON_IsNumber(scopeVal)) {
        info.scope = scopeVal->valueint;
    }
    cJSON *srcVal = cJSON_GetObjectItem(arrItem, FONT_SRC_PATH);
    if (srcVal && cJSON_IsString(srcVal)) {
        info.srcPath = srcVal->valuestring;
    }
    cJSON *appIdVal = cJSON_GetObjectItem(arrItem, FONT_APP_IDENTIFIER);
    if (appIdVal && cJSON_IsString(appIdVal)) {
        info.appIdentifier = appIdVal->valuestring;
    }
    cJSON *bnVal = cJSON_GetObjectItem(arrItem, FONT_BUNDLE_NAME);
    if (bnVal && cJSON_IsString(bnVal)) {
        info.bundleName = bnVal->valuestring;
    }
    return info;
}

bool FontConfig::InsertScopeFontRecord(const FontRecordInfo &record)
{
    std::lock_guard<std::mutex> lock(configLock_);
    cJSON *jsonValue = cJSON_Parse(CheckConfigFile(ConfigPath_).c_str());
    if (jsonValue == nullptr) {
        FONT_LOGE("InsertScopeFontRecord parse config failed");
        return false;
    }
    cJSON *fontList = cJSON_GetObjectItem(jsonValue, "fontlist");
    if (fontList == nullptr) {
        cJSON_Delete(jsonValue);
        return false;
    }
    cJSON *insertValue = ConstructScopeCJSON(record);
    if (insertValue == nullptr) {
        cJSON_Delete(jsonValue);
        return false;
    }
    cJSON_AddItemToArray(fontList, insertValue);
    char *fileData = cJSON_Print(jsonValue);
    cJSON_Delete(jsonValue);
    return WriteToFile(fileData);
}

bool FontConfig::DeleteScopeFontRecordByUrl(const std::string &srcPath)
{
    std::lock_guard<std::mutex> lock(configLock_);
    cJSON *jsonValue = cJSON_Parse(CheckConfigFile(ConfigPath_).c_str());
    if (jsonValue == nullptr) {
        return false;
    }
    cJSON *fontList = cJSON_GetObjectItem(jsonValue, "fontlist");
    if (!cJSON_IsArray(fontList)) {
        cJSON_Delete(jsonValue);
        return false;
    }
    int sz = cJSON_GetArraySize(fontList);
    for (int i = 0; i < sz; i++) {
        cJSON *item = cJSON_GetArrayItem(fontList, i);
        cJSON *srcVal = cJSON_GetObjectItem(item, FONT_SRC_PATH);
        if (srcVal && cJSON_IsString(srcVal) && srcPath == srcVal->valuestring) {
            cJSON_DeleteItemFromArray(fontList, i);
            char *fileData = cJSON_Print(jsonValue);
            cJSON_Delete(jsonValue);
            return WriteToFile(fileData);
        }
    }
    cJSON_Delete(jsonValue);
    return false;
}

bool FontConfig::DeleteScopeFontRecordByAppId(const std::string &appIdentifier)
{
    std::lock_guard<std::mutex> lock(configLock_);
    cJSON *jsonValue = cJSON_Parse(CheckConfigFile(ConfigPath_).c_str());
    if (jsonValue == nullptr) {
        return false;
    }
    cJSON *fontList = cJSON_GetObjectItem(jsonValue, "fontlist");
    if (!cJSON_IsArray(fontList)) {
        cJSON_Delete(jsonValue);
        return false;
    }
    bool changed = false;
    int sz = cJSON_GetArraySize(fontList);
    for (int i = sz - 1; i >= 0; i--) {
        cJSON *item = cJSON_GetArrayItem(fontList, i);
        cJSON *appIdVal = cJSON_GetObjectItem(item, FONT_APP_IDENTIFIER);
        if (appIdVal && cJSON_IsString(appIdVal) && appIdentifier == appIdVal->valuestring) {
            cJSON_DeleteItemFromArray(fontList, i);
            changed = true;
        }
    }
    if (changed) {
        char *fileData = cJSON_Print(jsonValue);
        cJSON_Delete(jsonValue);
        return WriteToFile(fileData);
    }
    cJSON_Delete(jsonValue);
    return false;
}

std::optional<FontRecordInfo> FontConfig::GetFontRecordByUrl(const std::string &srcPath)
{
    std::lock_guard<std::mutex> lock(configLock_);
    cJSON *jsonValue = cJSON_Parse(CheckConfigFile(ConfigPath_).c_str());
    if (jsonValue == nullptr) {
        return std::nullopt;
    }
    cJSON *fontList = cJSON_GetObjectItem(jsonValue, "fontlist");
    if (!cJSON_IsArray(fontList)) {
        cJSON_Delete(jsonValue);
        return std::nullopt;
    }
    int sz = cJSON_GetArraySize(fontList);
    for (int i = 0; i < sz; i++) {
        cJSON *item = cJSON_GetArrayItem(fontList, i);
        cJSON *srcVal = cJSON_GetObjectItem(item, FONT_SRC_PATH);
        if (srcVal && cJSON_IsString(srcVal) && srcPath == srcVal->valuestring) {
            FontRecordInfo info = ParseScopeRecord(item);
            cJSON_Delete(jsonValue);
            return info;
        }
    }
    cJSON_Delete(jsonValue);
    return std::nullopt;
}

std::optional<FontRecordInfo> FontConfig::GetFontRecordByName(const std::string &fullName)
{
    std::lock_guard<std::mutex> lock(configLock_);
    cJSON *jsonValue = cJSON_Parse(CheckConfigFile(ConfigPath_).c_str());
    if (jsonValue == nullptr) {
        return std::nullopt;
    }
    cJSON *fontList = cJSON_GetObjectItem(jsonValue, "fontlist");
    if (!cJSON_IsArray(fontList)) {
        cJSON_Delete(jsonValue);
        return std::nullopt;
    }
    int sz = cJSON_GetArraySize(fontList);
    for (int i = 0; i < sz; i++) {
        cJSON *item = cJSON_GetArrayItem(fontList, i);
        cJSON *nameArr = cJSON_GetObjectItem(item, FONT_FULL_NAME);
        if (!nameArr || !cJSON_IsArray(nameArr)) {
            continue;
        }
        int nameSz = cJSON_GetArraySize(nameArr);
        for (int j = 0; j < nameSz; j++) {
            cJSON *n = cJSON_GetArrayItem(nameArr, j);
            if (n && cJSON_IsString(n) && fullName == n->valuestring) {
                FontRecordInfo info = ParseScopeRecord(item);
                cJSON_Delete(jsonValue);
                return info;
            }
        }
    }
    cJSON_Delete(jsonValue);
    return std::nullopt;
}

std::vector<FontRecordInfo> FontConfig::GetFontRecordsByAppId(const std::string &appIdentifier)
{
    std::vector<FontRecordInfo> result;
    std::lock_guard<std::mutex> lock(configLock_);
    cJSON *jsonValue = cJSON_Parse(CheckConfigFile(ConfigPath_).c_str());
    if (jsonValue == nullptr) {
        return result;
    }
    cJSON *fontList = cJSON_GetObjectItem(jsonValue, "fontlist");
    if (!cJSON_IsArray(fontList)) {
        cJSON_Delete(jsonValue);
        return result;
    }
    int sz = cJSON_GetArraySize(fontList);
    for (int i = 0; i < sz; i++) {
        cJSON *item = cJSON_GetArrayItem(fontList, i);
        cJSON *appIdVal = cJSON_GetObjectItem(item, FONT_APP_IDENTIFIER);
        if (appIdVal && cJSON_IsString(appIdVal) && appIdentifier == appIdVal->valuestring) {
            result.push_back(ParseScopeRecord(item));
        }
    }
    cJSON_Delete(jsonValue);
    return result;
}

std::vector<FontRecordInfo> FontConfig::GetScopeFontRecords()
{
    std::vector<FontRecordInfo> result;
    std::lock_guard<std::mutex> lock(configLock_);
    cJSON *jsonValue = cJSON_Parse(CheckConfigFile(ConfigPath_).c_str());
    if (jsonValue == nullptr) {
        return result;
    }
    cJSON *fontList = cJSON_GetObjectItem(jsonValue, "fontlist");
    if (!cJSON_IsArray(fontList)) {
        cJSON_Delete(jsonValue);
        return result;
    }
    int sz = cJSON_GetArraySize(fontList);
    for (int i = 0; i < sz; i++) {
        cJSON *item = cJSON_GetArrayItem(fontList, i);
        cJSON *scopeVal = cJSON_GetObjectItem(item, FONT_SCOPE);
        if (scopeVal && cJSON_IsNumber(scopeVal) && scopeVal->valueint >= 0) {
            result.push_back(ParseScopeRecord(item));
        }
    }
    cJSON_Delete(jsonValue);
    return result;
}

std::vector<FontRecordInfo> FontConfig::GetAppScopeFontRecords()
{
    std::vector<FontRecordInfo> result;
    std::lock_guard<std::mutex> lock(configLock_);
    cJSON *jsonValue = cJSON_Parse(CheckConfigFile(ConfigPath_).c_str());
    if (jsonValue == nullptr) {
        return result;
    }
    cJSON *fontList = cJSON_GetObjectItem(jsonValue, "fontlist");
    if (!cJSON_IsArray(fontList)) {
        cJSON_Delete(jsonValue);
        return result;
    }
    int sz = cJSON_GetArraySize(fontList);
    for (int i = 0; i < sz; i++) {
        cJSON *item = cJSON_GetArrayItem(fontList, i);
        cJSON *scopeVal = cJSON_GetObjectItem(item, FONT_SCOPE);
        if (scopeVal && cJSON_IsNumber(scopeVal) && scopeVal->valueint == FONT_SCOPE_APP) {
            result.push_back(ParseScopeRecord(item));
        }
    }
    cJSON_Delete(jsonValue);
    return result;
}

bool FontConfig::CheckAndUpdateFontRecord()
{
    std::lock_guard<std::mutex> lock(configLock_);
    cJSON *jsonValue = cJSON_Parse(CheckConfigFile(ConfigPath_).c_str());
    if (jsonValue == nullptr) {
        return false;
    }
    cJSON *versionJson = cJSON_GetObjectItem(jsonValue, FONT_VERSION);
    if (versionJson != nullptr) {
        if (cJSON_IsString(versionJson) && FONT_CONFIG_VERSION_7 == versionJson->valuestring) {
            cJSON_Delete(jsonValue);
            return true;
        }
        if (cJSON_IsNumber(versionJson) && versionJson->valueint == VERSION) {
            // upgrade v1 -> v7: old records get scope=-1 (absent), version -> "7.0"
            cJSON_ReplaceItemInObject(jsonValue, FONT_VERSION, cJSON_CreateString(FONT_CONFIG_VERSION_7.c_str()));
            char *fileData = cJSON_Print(jsonValue);
            cJSON_Delete(jsonValue);
            return WriteToFile(fileData);
        }
    }
    // no version field: set to 7.0
    cJSON_AddStringToObject(jsonValue, FONT_VERSION, FONT_CONFIG_VERSION_7.c_str());
    char *fileData = cJSON_Print(jsonValue);
    cJSON_Delete(jsonValue);
    return WriteToFile(fileData);
}

int FontConfig::GetTotalInstalledFontsNum()
{
    std::lock_guard<std::mutex> lock(configLock_);
    cJSON *jsonValue = cJSON_Parse(CheckConfigFile(ConfigPath_).c_str());
    if (jsonValue == nullptr) {
        return 0;
    }
    cJSON *fontList = cJSON_GetObjectItem(jsonValue, "fontlist");
    int count = cJSON_IsArray(fontList) ? cJSON_GetArraySize(fontList) : 0;
    cJSON_Delete(jsonValue);
    return count;
}
}  // namespace FontManager
}  // namespace Global
}  // namespace OHOS