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
#ifndef FONT_MANAGER_UNITTEST_PERMISSION_COMMON_H
#define FONT_MANAGER_UNITTEST_PERMISSION_COMMON_H

#include <string>
namespace OHOS {
namespace Global {
namespace FontManager {
class PermissionCommon {
public:
    static void SetFontManagerPermission(const std::string &processName);
    static void ResetTokenAndUid();
    static void SetUid();
    static void ResetUid();
    static bool IsOriginalUTEnv();
    static void SetFontManagerInitEnv();
private:
    static uint64_t selfTokenId_;
};
} // namespace FontManager
} // namespace Global
} // namespace OHOS
#endif // FONT_MANAGER_UNITTEST_PERMISSION_COMMON_H
