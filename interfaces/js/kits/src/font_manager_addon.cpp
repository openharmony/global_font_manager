/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
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

#include "font_define.h"
#include "font_hilog.h"
#include "font_manager_client.h"

namespace OHOS {
namespace Global {
namespace FontManager {
#define GET_PARAMS(env, info, num)    \
    size_t argc = num;                \
    napi_value argv[num] = {nullptr}; \
    napi_value thisVar = nullptr;     \
    void *data = nullptr;             \
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data)

static constexpr int32_t ARRAY_SUBCRIPTOR_ZERO = 0;

napi_value FontManagerAddon::Init(napi_env env, napi_value exports)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("installFont", InstallFont),
        DECLARE_NAPI_FUNCTION("uninstallFont", UninstallFont)
    };
    napi_define_properties(env, exports, sizeof(properties) / sizeof(properties[0]), properties);
    return exports;
}

napi_value GetCallbackErrorCode(napi_env env, const int32_t errCode, const std::string &errMsg)
{
    napi_value error = nullptr;
    napi_value eCode = nullptr;
    napi_value eMsg = nullptr;
    napi_create_int32(env, errCode, &eCode);
    napi_create_string_utf8(env, errMsg.c_str(), NAPI_AUTO_LENGTH, &eMsg);
    napi_create_object(env, &error);
    napi_set_named_property(env, error, "code", eCode);
    napi_set_named_property(env, error, "message", eMsg);
    return error;
}

void FontManagerAddon::Complete(napi_env env, napi_status status, void* data)
{
    if (data == nullptr) {
        return;
    }
    FontNapiCallback *fontNapiCallback = static_cast<FontNapiCallback*>(data);
    if (fontNapiCallback == nullptr) {
        return;
    }
    napi_value finalResult = nullptr;
    napi_status ret = napi_create_int32(env, fontNapiCallback->errCode_, &finalResult);
    if (ret != napi_ok) {
        FONT_LOGE("InstallFont: create boolean result failed");
        fontNapiCallback->success_ = false;
    }

    napi_value result[] = { nullptr, nullptr };
    if (fontNapiCallback->success_) {
        napi_get_undefined(env, &result[0]);
        result[1] = finalResult;
    } else {
        result[0] = GetCallbackErrorCode(env, fontNapiCallback->errCode_, fontNapiCallback->errMsg_.c_str());
        napi_get_undefined(env, &result[1]);
    }
    if (fontNapiCallback->deferred_) {
        ProcessPromiseResult(env, fontNapiCallback, result);
    } else {
        ProcessCallbackResult(env, fontNapiCallback, result);
    }
    napi_delete_async_work(env, fontNapiCallback->work_);
    delete fontNapiCallback;
};

void FontManagerAddon::ProcessPromiseResult(napi_env env, FontNapiCallback *fontCallback, napi_value (&result)[2])
{
    if (fontCallback == nullptr) {
        return;
    }

    if (fontCallback->success_) {
        if (napi_resolve_deferred(env, fontCallback->deferred_, result[1]) != napi_ok) {
            FONT_LOGE("napi_resolve_deferred failed");
        }
    } else {
        if (napi_reject_deferred(env, fontCallback->deferred_, result[0]) != napi_ok) {
            FONT_LOGE("napi_reject_deferred failed");
        }
    }
}

void FontManagerAddon::ProcessCallbackResult(napi_env env, FontNapiCallback *fontCallback, napi_value (&result)[2])
{
    if (fontCallback == nullptr) {
        return;
    }

    napi_value callback = nullptr;
    napi_status status = napi_get_reference_value(env, fontCallback->callbackRef_, &callback);
    if (status != napi_ok) {
        FONT_LOGE("napi_get_reference_value failed status=%{public}d", status);
        return;
    }
    napi_value userRet;
    status = napi_call_function(env, nullptr, callback, sizeof(result) / sizeof(napi_value), result, &userRet);
    if (status != napi_ok) {
        FONT_LOGE("napi_call_function failed status=%{public}d", status);
        return;
    }
    status = napi_delete_reference(env, fontCallback->callbackRef_);
    if (status != napi_ok) {
        FONT_LOGE("napi_call_function failed status=%{public}d", status);
        return;
    }
}

napi_value FontManagerAddon::GetResult(napi_env env, std::unique_ptr<FontNapiCallback> &callback,
    const std::string &name, napi_async_execute_callback execute)
{
    napi_value result = nullptr;
    if (callback == nullptr) {
        return result;
    }
    if (callback->callbackRef_ == nullptr) {
        napi_create_promise(env, &callback->deferred_, &result);
    } else {
        napi_get_undefined(env, &result);
    }
    napi_value resource = nullptr;
    napi_create_string_utf8(env, name.c_str(), NAPI_AUTO_LENGTH, &resource);
    if (napi_create_async_work(env, nullptr, resource, execute, FontManagerAddon::Complete,
        static_cast<void*>(callback.get()), &callback->work_) != napi_ok) {
        FONT_LOGE("Failed to create async work for %{public}s", name.c_str());
        return result;
    }
    if (napi_queue_async_work_with_qos(env, callback->work_, napi_qos_user_initiated) != napi_ok) {
        FONT_LOGE("Failed to queue async work for %{public}s", name.c_str());
        return result;
    }
    callback.release();
    return result;
}
 
auto installFontFunc = [](napi_env env, void* data) {
    if (data == nullptr) {
        return;
    }

    FontNapiCallback *callback = static_cast<FontNapiCallback*>(data);
    if (callback->value_.empty()) {
        callback->SetErrorMsg("invalid param", ERR_FILE_NOT_EXISTS);
        return;
    }
    int ret = FontManagerClient::InstallFont(callback->value_, callback->errCode_);
    if (ret) {
        callback->SetErrorMsg("Failed to InstallFont", ret);
        return;
    }
};

napi_value FontManagerAddon::InstallFont(napi_env env, napi_callback_info info)
{
    return ProcessFontByValue(env, info, "InstallFont", installFontFunc);
}

auto uninstallFontFunc = [](napi_env env, void* data) {
    if (data == nullptr) {
        return;
    }

    FontNapiCallback *callback = static_cast<FontNapiCallback*>(data);
    if (callback->value_.empty()) {
        callback->SetErrorMsg("invalid param", ERR_UNINSTALL_FILE_NOT_EXISTS);
        return;
    }
    int ret = FontManagerClient::UninstallFont(callback->value_, callback->errCode_);
    if (ret) {
        callback->SetErrorMsg("Failed to UninstallFont", ret);
        return;
    }
};

napi_value FontManagerAddon::UninstallFont(napi_env env, napi_callback_info info)
{
    return ProcessFontByValue(env, info, "UninstallFont", uninstallFontFunc);
}

napi_value FontManagerAddon::ProcessFontByValue(
    napi_env env, napi_callback_info info, const std::string &name, napi_async_execute_callback execute)
{
    FONT_LOGI("ProcessFontByValue, func:%{public}s", name.c_str());
    GET_PARAMS(env, info, 2);
    std::unique_ptr<FontNapiCallback> callback = std::make_unique<FontNapiCallback>();
    for (size_t i = 0; i < argc; i++) {
        napi_valuetype valueType;
        napi_typeof(env, argv[i], &valueType);
        if (i == 0 && valueType == napi_string) {
            callback->value_ = GetResNameOrPath(env, argc, argv);
            FONT_LOGD("ProcessFontByValue callback->value_= %{public}s", callback->value_.c_str());
        } else if (i == 1 && valueType == napi_function) {
            napi_create_reference(env, argv[i], 1, &callback->callbackRef_);
            break;
        }
    }
    return GetResult(env, callback, name, execute);
}

std::string FontManagerAddon::GetResNameOrPath(napi_env env, size_t argc, napi_value *argv)
{
    if (argc == 0 || argv == nullptr) {
        return "";
    }

    napi_valuetype valuetype;
    napi_typeof(env, argv[0], &valuetype);
    if (valuetype != napi_string) {
        FONT_LOGE("Invalid param, not string");
        return "";
    }
    size_t len = 0;
    napi_status status = napi_get_value_string_utf8(
        env, argv[ARRAY_SUBCRIPTOR_ZERO], nullptr, ARRAY_SUBCRIPTOR_ZERO, &len);
    if (status != napi_ok) {
        FONT_LOGE("Failed to get font path length");
        return "";
    }
    std::vector<char> buf(len + 1);
    status = napi_get_value_string_utf8(env, argv[ARRAY_SUBCRIPTOR_ZERO], buf.data(), len + 1, &len);
    if (status != napi_ok) {
        FONT_LOGE("Failed to get font file path");
        return "";
    }
    return buf.data();
}
} // namespace FontManager
} // namespace Global
} // namespace OHOS