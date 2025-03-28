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

#ifndef GLOBAL_FONT_MANAGER_FONT_ADDON_H
#define GLOBAL_FONT_MANAGER_FONT_ADDON_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "font_napi_callback.h"

namespace OHOS {
namespace Global {
namespace FontManager {
class FontManagerAddon {
public:
    FontManagerAddon();
    ~FontManagerAddon();
    static napi_value Init(napi_env env, napi_value exports);
    static napi_value InstallFont(napi_env env, napi_callback_info info);
    static napi_value UninstallFont(napi_env env, napi_callback_info info);

private:
    static napi_value ProcessFontByValue(
        napi_env env, napi_callback_info info, const std::string &name, napi_async_execute_callback execute);
    static std::string GetResNameOrPath(napi_env env, size_t argc, napi_value *argv);
    static void Complete(napi_env env, napi_status status, void* data);
    static void ProcessPromiseResult(napi_env env, FontNapiCallback *callback, napi_value (&result)[2]);
    static void ProcessCallbackResult(napi_env env, FontNapiCallback *callback, napi_value (&result)[2]);
    static napi_value GetResult(napi_env env, std::unique_ptr<FontNapiCallback> &callback,
        const std::string &name, napi_async_execute_callback execute);
};
} // namespace FontManager
} // namespace Global
} // namespace OHOS
#endif // GLOBAL_FONT_MANAGER_FONT_ADDON_H
