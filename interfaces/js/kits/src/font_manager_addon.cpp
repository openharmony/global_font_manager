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

#include "font_manager_addon.h"

#include "font_define.h"
#include "font_hilog.h"
#include "font_manager_kits.h"
#include "js_data_migration_listener.h"

namespace OHOS {
namespace Global {
namespace FontManager {
using namespace AbilityRuntime;
namespace {
static const std::unordered_map<uint32_t, std::string> g_DataMigrationErrMsgMap = {
    {ERR_NO_PERMISSION, "Permission verification failed. The application does not "
        "have the permission required to call the API."},
    {ERR_NOT_SYSTEM_APP, "Permission verification failed. A non-system application calls a system API."},
    {ERR_DATA_MIGRATIONING, "Data migration is in progress."},
    {ERR_SYSTEM_ERROR, "Call failed due to system error."}
};
static constexpr int32_t ARRAY_SUBCRIPTOR_ZERO = 0;
static constexpr int32_t ARGS_ZERO = 0;
static constexpr int32_t ARGS_SIZE_ONE = 1;
}
FontManagerAddon::FontManagerAddon()
{
}
FontManagerAddon::~FontManagerAddon()
{
}

napi_value FontManagerAddonInit(napi_env env, napi_value exports)
{
    FONT_LOGI("FontManagerAddon::Init");
    if (env == nullptr || exports == nullptr) {
        FONT_LOGI("FontManagerAddon::Init env or exports is null.");
        return CreateJsUndefined(env);
    }
    std::unique_ptr<FontManagerAddon> JsCMManager = std::make_unique<FontManagerAddon>();
    napi_wrap(env, exports, JsCMManager.release(), FontManagerAddon::Finalizer, nullptr, nullptr);
    const char *moduleName = "FontManagerAddon";
    BindNativeFunction(env, exports, "installFont", moduleName, FontManagerAddon::InstallFont);
    BindNativeFunction(env, exports, "uninstallFont", moduleName, FontManagerAddon::UninstallFont);
    BindNativeFunction(env, exports, "dataMigration", moduleName, FontManagerAddon::DataMigration);
    return CreateJsUndefined(env);
}

void FontManagerAddon::Finalizer(napi_env env, void* data, void* hint)
{
    FONT_LOGI("FontManagerAddon::Finalizer");
    std::unique_ptr<FontManagerAddon>(static_cast<FontManagerAddon*>(data));
    (void)hint;
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
        FONT_LOGE("InstallFont: create int result failed");
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
    ProcessPromiseResult(env, fontNapiCallback, result);
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

napi_value FontManagerAddon::GetResult(napi_env env, std::unique_ptr<FontNapiCallback> &callback,
    const std::string &name, napi_async_execute_callback execute)
{
    napi_value result = nullptr;
    if (callback == nullptr) {
        return result;
    }
    napi_create_promise(env, &callback->deferred_, &result);

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
    int ret = FontManagerKits::GetInstance().InstallFont(callback->value_, callback->errCode_);
    if (ret != ERR_OK) {
        callback->SetErrorMsg("The system ability works abnormally.", ERR_INSTALL_FAIL);
    }
    if (callback->errCode_ != ERR_OK) {
        std::string msg = "";
        switch (callback->errCode_) {
            case ERR_NO_PERMISSION :
                msg = "Permission verification failed. The application does not "
                    "have the permission required to call the API.";
                break;
            case ERR_NOT_SYSTEM_APP :
                msg = "Permission verification failed. A non-system application calls a system API.";
                break;
            case ERR_FILE_NOT_EXISTS :
                msg = "The font does not exist.";
                break;
            case ERR_FILE_VERIFY_FAIL :
                msg = "The font is not supported.";
                break;
            case ERR_COPY_FAIL :
                msg = "Failed to copy the font file.";
                break;
            case ERR_INSTALLED_ALRADY :
                msg = "The font file is installed.";
                break;
            case ERR_MAX_FILE_COUNT :
                msg = "Exceeded the maximum number of installed files.";
                break;
            case ERR_INSTALL_FAIL :
                msg = "The system ability works abnormally.";
                break;
            default:
                msg = "Other error.";
        }
        callback->SetErrorMsg(msg, callback->errCode_);
    }
    return;
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
    int ret = FontManagerKits::GetInstance().UninstallFont(callback->value_, callback->errCode_);
    if (ret != ERR_OK) {
        callback->SetErrorMsg("The system ability works abnormally.", ERR_UNINSTALL_FAIL);
    }
    if (callback->errCode_ != ERR_OK) {
        std::string msg = "";
        switch (callback->errCode_) {
            case ERR_NO_PERMISSION :
                msg = "Permission verification failed. The application does not "
                    "have the permission required to call the API.";
                break;
            case ERR_NOT_SYSTEM_APP :
                msg = "Permission verification failed. A non-system application calls a system API.";
                break;
            case ERR_UNINSTALL_FILE_NOT_EXISTS :
                msg = "The font file does not exist.";
                break;
            case ERR_UNINSTALL_REMOVE_FAIL :
                msg = "Failed to delete the font file.";
                break;
            case ERR_UNINSTALL_FAIL :
                msg = "The system ability works abnormally.";
                break;
            default:
                msg = "Other error.";
        }
        callback->SetErrorMsg(msg, callback->errCode_);
    }
    return;
};

napi_value FontManagerAddon::UninstallFont(napi_env env, napi_callback_info info)
{
    return ProcessFontByValue(env, info, "UninstallFont", uninstallFontFunc);
}

napi_value FontManagerAddon::ProcessFontByValue(
    napi_env env, napi_callback_info info, const std::string &name, napi_async_execute_callback execute)
{
    FONT_LOGI("ProcessFontByValue, func:%{public}s", name.c_str());
    size_t argc = ARGS_SIZE_ONE;
    napi_value argv[ARGS_SIZE_ONE] = {nullptr};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    std::unique_ptr<FontNapiCallback> callback = std::make_unique<FontNapiCallback>();
    callback->value_ = GetResNameOrPath(env, argc, argv);
    FONT_LOGD("ProcessFontByValue callback->value_= %{public}s", callback->value_.c_str());
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

napi_value FontManagerAddon::DataMigration(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, FontManagerAddon, DataMigrationInner);
}

napi_value FontManagerAddon::DataMigrationInner(napi_env env, AbilityRuntime::NapiCallbackInfo& info)
{
    FONT_LOGI("FontManagerAddon DataMigrationInner enter.");
    if (info.argc != ARGS_SIZE_ONE) {
        FONT_LOGE("Argc is invalid:%{public}zu", info.argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(ERR_INVALID_PARAM)));
        return CreateJsUndefined(env);
    }
    napi_value value = info.argv[ARGS_ZERO];
    napi_valuetype valuetype;
    napi_status ret = napi_typeof(env, value, &valuetype);
    if (ret != napi_ok || valuetype != napi_object) {
        FONT_LOGE("Callback(info->argv[0]) is not object.");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(ERR_INVALID_PARAM)));
        return CreateJsUndefined(env);
    }
    napi_value onHeartBeatValue;
    NAPI_CALL(env, napi_get_named_property(env, value, "onHeartBeat", &onHeartBeatValue));
    auto heartBeatCallback = std::make_shared<JsFuncRefHolder>(env, onHeartBeatValue);
    napi_value onProgressValue;
    NAPI_CALL(env, napi_get_named_property(env, value, "onProgress", &onProgressValue));
    auto progressCallback = std::make_shared<JsFuncRefHolder>(env, onProgressValue);
    napi_value onResultValue;
    NAPI_CALL(env, napi_get_named_property(env, value, "onResult", &onResultValue));
    auto resultCallback = std::make_shared<JsFuncRefHolder>(env, onResultValue);
    auto listener = std::make_shared<JsDataMigrationListener>(env, heartBeatCallback, progressCallback,
        resultCallback);
    int32_t result = FontManagerKits::GetInstance().DataMigration(std::move(listener));
    if (result != ERR_OK) {
        napi_throw(env, CreateJsError(env, result, GetDataMigrationErrMsg(result)));
        return CreateJsUndefined(env);
    } else {
        napi_value ret = nullptr;
        napi_create_int32(env, result, &ret);
        return ret;
    }
}

std::string FontManagerAddon::GetDataMigrationErrMsg(int32_t errCode)
{
    auto it = g_DataMigrationErrMsgMap.find(errCode);
    if (it != g_DataMigrationErrMsgMap.end()) {
        return g_DataMigrationErrMsgMap.at(errCode);
    }
    return g_DataMigrationErrMsgMap.at(ERR_SYSTEM_ERROR);
}
} // namespace FontManager
} // namespace Global
} // namespace OHOS