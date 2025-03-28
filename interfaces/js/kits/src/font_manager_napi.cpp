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

#include "font_manager_addon.h"

#include "napi/native_api.h"
#include "napi/native_common.h"

namespace OHOS {
namespace Global {
namespace FontManager {
napi_value Init(napi_env env, napi_value exports)
{
    napi_value val = FontManagerAddon::Init(env, exports);
    return val;
}

static napi_module g_FontResourceModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "fontmanager",
    .nm_priv = nullptr,
    .reserved = { 0 }
};

extern "C" __attribute__((constructor)) void AbilityRegister()
{
    napi_module_register(&g_FontResourceModule);
}
} // namespace FontManager
} // namespace Global
} // namespace OHOS