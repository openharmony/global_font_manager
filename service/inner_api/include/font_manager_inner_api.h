/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef GLOBAL_FONT_MANAGER_FONT_MANAGER_INNER_API_H
#define GLOBAL_FONT_MANAGER_FONT_MANAGER_INNER_API_H

#include <string>

namespace OHOS {
namespace Global {
namespace FontManager {
class FontManagerInnerApi {
public:
    static int32_t InstallFont(const std::string &fontPath, int32_t userId);
    static int32_t UninstallFont(const std::string &fontName, int32_t userId);

    // Installs a user-level font from a caller-owned fd. The fd is not closed by this API
    // and must stay open with stable content for the duration of the call.
    static int32_t InstallFontByFd(int32_t fd, int32_t userId);

    // Uninstalls a single font by full name. Unlike UninstallFont, this API also handles
    // app-level and session-level records in the user's install_fontconfig.json.
    static int32_t UninstallFontByFullName(const std::string &fontFullName, int32_t userId);
};
} // namespace FontManager
} // namespace Global
} // namespace OHOS
#endif // GLOBAL_FONT_MANAGER_FONT_MANAGER_INNER_API_H
