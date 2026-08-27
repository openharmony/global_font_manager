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

#ifndef GLOBAL_FONT_MANAGER_FONT_MANAGER_H
#define GLOBAL_FONT_MANAGER_FONT_MANAGER_H

#include "singleton.h"
#include "font_config.h"

namespace OHOS {
namespace Global {
namespace FontManager {
struct ScopeFontInstallInfo {
    int32_t fd = -1;
    int32_t scope = -1;
    std::string srcPath;
    std::string bundleName;
    std::string appIdentifier;
    int32_t userId = -1;
};
class FontManager : public DelayedSingleton<FontManager> {
    DECLARE_DELAYED_SINGLETON(FontManager);
public:
    int32_t InstallFont(const int32_t &fd, const int32_t userId);
    int32_t UninstallFont(const std::string &fontFullName, const int32_t userId);

    int32_t InstallScopeFont(const ScopeFontInstallInfo &info);
    int32_t UninstallScopeFont(const std::string &srcPath, const std::string &bundleName,
        int32_t userId);
    int32_t GetFontScope(const std::string &srcPath, const std::string &bundleName, int32_t userId);
    int32_t CleanupAppScopeFonts(const std::string &appIdentifier, int32_t userId);
    int32_t CleanupScopeFontsByUser(int32_t userId);
    int32_t CleanupAppScopeFontsByUser(int32_t userId);

private:
    static int32_t GetMaxInstallNum();
    std::string GetFormatFullName(const std::vector<std::string> &fullNameVector);
    std::string CopyFileForInstall(const std::string &installPath, const std::string &fileName, const int32_t &fd);
    std::string SandBoxPathToRealPath(const std::string &installPath, const std::string &path);
    FontConfig& SafeGetOrCreateConfig(int32_t userId, const std::string& configPath);
    std::string GetAppInstallPath(int32_t userId, const std::string &appIdentifier);
    int32_t ValidateScopeFontForInstall(FontConfig& fontConfig, const std::string &srcPath,
        const int32_t fd, std::vector<std::string>& fullNames);
    int32_t CopyAndInsertScopeFont(const ScopeFontInstallInfo &info,
        FontConfig& fontConfig, const std::vector<std::string>& fullNames);
    std::unordered_map<int32_t, FontConfig> configMap_;
    std::mutex mapLock_;
};
} // namespace FontManager
} // namespace Global
} // namespace OHOS
#endif // GLOBAL_FONT_MANAGER_FONT_MANAGER_H
