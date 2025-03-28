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

#ifndef GLOBAL_FONT_MANAGER_FONT_MANAGER_H
#define GLOBAL_FONT_MANAGER_FONT_MANAGER_H

#include "singleton.h"

namespace OHOS {
namespace Global {
namespace FontManager {
class FontManager : public DelayedSingleton<FontManager> {
    DECLARE_DELAYED_SINGLETON(FontManager);
public:
    int32_t InstallFont(const int32_t &fd);
    int32_t UninstallFont(const std::string &fontFullName);

private:
    bool CheckInstallPath();
    bool CheckFontConfigPath();
    std::vector<std::string> GetFontFullName(const int32_t &fd);
    std::string GetFormatFullName(const std::vector<std::string> &fullNameVector);
    std::string CopyFile(const std::string &fontPath, const int32_t &fd);
};
} // namespace FontManager
} // namespace Global
} // namespace OHOS
#endif // GLOBAL_FONT_MANAGER_FONT_MANAGER_H
