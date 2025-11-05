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

namespace OHOS {
namespace Global {
namespace FontManager {
class FontManager : public DelayedSingleton<FontManager> {
    DECLARE_DELAYED_SINGLETON(FontManager);
public:
    int32_t InstallFont(const int32_t &fd, const int32_t userId);
    int32_t UninstallFont(const std::string &fontFullName, const int32_t userId);
    bool CheckAndInitInstallPath(const std::string &installPath);

private:
    bool CheckFontConfigPath(const std::string &installPath);
    std::string Utf16BEToUtf8(const uint8_t* data, size_t byteLen);
    std::vector<std::string> GetFontFullName(const int32_t &fd);
    std::string GetFormatFullName(const std::vector<std::string> &fullNameVector);
    std::string CopyFileForInstall(const std::string &installPath, const std::string &fileName, const int32_t &fd);
    std::string GetRealPath(const std::string &installPath, const std::string &path);
};
} // namespace FontManager
} // namespace Global
} // namespace OHOS
#endif // GLOBAL_FONT_MANAGER_FONT_MANAGER_H
