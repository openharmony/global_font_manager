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
#include <string>
#include <vector>

#include "cJSON.h"
#include "nocopyable.h"

namespace OHOS {
namespace Global {
namespace FontManager {
class FontConfig {
public:
    DISALLOW_COPY(FontConfig);
    // 构造函数
    FontConfig(const std::string &configPath) : ConfigPath_(configPath){};
    // 移动构造函数
    FontConfig(FontConfig&& other) noexcept : fontsMap_(std::move(other.fontsMap_)),
        ConfigPath_(std::move(other.ConfigPath_)) {};
    // 移动赋值操作符
    FontConfig& operator = (FontConfig&& other) noexcept
    {
        if (this != &other) {
            fontsMap_ = std::move(other.fontsMap_);
            ConfigPath_ = std::move(other.ConfigPath_);
        }
        return *this;
    };
    ~FontConfig() = default;
    bool InsertFontRecord(const std::string &fontPath, const std::vector<std::string> &fullNames);
    bool DeleteFontRecord(const std::string &fontPath);
    int GetInstalledFontsNum();
    std::string GetFontFileByName(const std::string &fullName);
    bool CheckAndUpdateFontRecord();
private:
    char *GetFileData(const std::string &filePath, long &size);
    std::string CheckConfigFile(const std::string &fontPath);
    cJSON *ConstructCJSON(const std::string &fontFullPath, const std::vector<std::string> &fullName);
    std::unordered_map<std::string, std::vector<std::string>> GetFontsMap(const std::string &filePath);
    bool WriteToFile(char *jsonData);
    cJSON* CovertFontMapToJsonArray(const std::unordered_map<std::string, std::vector<std::string>>& fontsMap);
private:
    std::unordered_map<std::string, std::vector<std::string>> fontsMap_;
    std::string ConfigPath_;
    std::mutex configLock_;
    std::mutex fontsMapLock_;
};
} // namespace FontManager
} // namespace Global
} // namespace OHOS
#endif // FONT_MANAGER_FONT_CONFIG_H
