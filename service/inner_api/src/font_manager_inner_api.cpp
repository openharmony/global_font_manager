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
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "font_manager_inner_api.h"
#include "font_define.h"
#include "font_manager_client.h"
#include "singleton.h"

namespace OHOS {
namespace Global {
namespace FontManager {
int32_t FontManagerInnerApi::InstallFont(const std::string &fontPath, int32_t userId)
{
    return DelayedRefSingleton<FontManagerClient>::GetInstance().InstallFontWithUserId(fontPath, userId);
}

int32_t FontManagerInnerApi::UninstallFont(const std::string &fontName, int32_t userId)
{
    return DelayedRefSingleton<FontManagerClient>::GetInstance().UninstallFontWithUserId(fontName, userId);
}

int32_t FontManagerInnerApi::InstallScopeFont(const std::string &fontPath, int32_t scope, int32_t userId)
{
    int32_t outValue = 0;
    int32_t ret = DelayedRefSingleton<FontManagerClient>::GetInstance().InstallScopeFont(fontPath, scope, outValue);
    if (ret != ERR_OK) {
        return ret;
    }
    return outValue;
}

int32_t FontManagerInnerApi::UninstallScopeFont(const std::string &srcPath, int32_t userId)
{
    int32_t outValue = 0;
    int32_t ret = DelayedRefSingleton<FontManagerClient>::GetInstance().UninstallScopeFont(srcPath, outValue);
    if (ret != ERR_OK) {
        return ret;
    }
    return outValue;
}
} // namespace FontManager
} // namespace Global
} // namespace OHOS
