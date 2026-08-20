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

#ifndef FONT_MANAGER_FONT_CONFIG_H
#define FONT_MANAGER_FONT_CONFIG_H
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "cJSON.h"
#include "nocopyable.h"

namespace OHOS {
namespace Global {
namespace FontManager {
struct FontRecordInfo {
    std::string fontPath;
    std::vector<std::string> fullNames;
    int32_t scope = -1;           // -1 = user-level (legacy), 0 = app-level, 1 = session-level
    std::string srcPath;
    std::string appIdentifier;
    std::string bundleName;
};

class FontConfig {
public:
    DISALLOW_COPY(FontConfig);
    FontConfig(const std::string &configPath) : ConfigPath_(configPath){};
    FontConfig(FontConfig&& other) noexcept : ConfigPath_(std::move(other.ConfigPath_)) {};
    FontConfig& operator = (FontConfig&& other) noexcept
    {
        if (this != &other) {
            ConfigPath_ = std::move(other.ConfigPath_);
        }
        return *this;
    };
    ~FontConfig() = default;
    bool InsertFontRecord(const std::string &fontPath, const std::vector<std::string> &fullNames);
    bool DeleteFontRecord(const std::string &fontPath);
    int GetInstalledFontsNum();
    std::string GetFontFileByName(const std::string &fullName);

    // Scope font methods
    bool InsertScopeFontRecord(const FontRecordInfo &record);
    bool DeleteScopeFontRecordByUrl(const std::string &srcPath);
    bool DeleteScopeFontRecordByAppId(const std::string &appIdentifier);
    std::optional<FontRecordInfo> GetFontRecordByUrl(const std::string &srcPath);
    std::optional<FontRecordInfo> GetFontRecordByName(const std::string &fullName);
    std::vector<FontRecordInfo> GetFontRecordsByAppId(const std::string &appIdentifier);
    std::vector<FontRecordInfo> GetScopeFontRecords();
    std::vector<FontRecordInfo> GetAppScopeFontRecords();
    bool CheckAndUpdateFontRecord();
    int GetTotalInstalledFontsNum();
private:
    char *GetFileData(const std::string &filePath, long &size);
    std::string CheckConfigFile(const std::string &fontPath);
    cJSON *ConstructCJSON(const std::string &fontFullPath, const std::vector<std::string> &fullName);
    cJSON *ConstructScopeCJSON(const FontRecordInfo &record);
    bool WriteToFile(char *jsonData);
    std::string SandBoxPathToRealPath(const std::string &path);
    void RefreshFullNames(cJSON *jsonValue);
private:
    std::string ConfigPath_;
    std::mutex configLock_;
};
} // namespace FontManager
} // namespace Global
} // namespace OHOS
#endif // FONT_MANAGER_FONT_CONFIG_H
