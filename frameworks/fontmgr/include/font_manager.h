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
class FontManager : public DelayedSingleton<FontManager> {
    DECLARE_DELAYED_SINGLETON(FontManager);
public:
    int32_t InstallFont(const std::string &path, const int32_t userId);
    int32_t UninstallFont(const std::string &fontFullName, const int32_t userId);

private:
    std::string GetFormatFullName(const std::vector<std::string> &fullNameVector);
    std::string CopyFileForInstall(const std::string &installPath, const std::string &fileName,
        const std::string &srcPath);
    std::string GetRealPath(const std::string &installPath, const std::string &path);
    std::unordered_map<int32_t, FontConfig> configMap_;
};
} // namespace FontManager
} // namespace Global
} // namespace OHOS
#endif // GLOBAL_FONT_MANAGER_FONT_MANAGER_H
