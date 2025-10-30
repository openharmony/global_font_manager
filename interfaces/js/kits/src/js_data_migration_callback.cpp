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

#include "js_data_migration_callback.h"

#include <cstdlib>
#include "font_define.h"

namespace OHOS {
namespace Global {
namespace FontManager {
using namespace AbilityRuntime;
static const std::unordered_map<uint32_t, std::string> g_DataMigrationErrMsgMap = {
    {ERR_NOT_NEED_DATA_MIGRATION, "The device dont need font data migration."},
    {ERR_SYSTEM_ERROR, "System service exception."}
};

JsDataMigrationCallback::JsDataMigrationCallback(napi_env env, napi_value value)
    : env_(env)
{
    napi_status status = napi_create_reference(env, value, 1, &cbRef_);
    if (status != napi_ok) {
        FONT_LOGE("JsDataMigrationCallback napi_create_reference failed.");
        napi_throw(env, CreateJsError(env, ERR_INVALID_PARAM));
    }

}

JsDataMigrationCallback::~JsDataMigrationCallback()
{
    ReleaseRef();
}

void JsDataMigrationCallback::ReleaseRef()
{
    if (env_ == nullptr) {
        FONT_LOGE("JsDataMigrationCallback ReleaseRef env is null.");
        return;
    }
    if (cbRef_ == nullptr) {
        FONT_LOGE("JsDataMigrationCallback ReleaseRef cbRef is null.");
        return;
    }
    auto cb = cbRef_;
    cbRef_ = nullptr;
    std::unique_ptr<AbilityRuntime::NapiAsyncTask::CompleteCallback> complete =
        std::make_unique<AbilityRuntime::NapiAsyncTask::CompleteCallback>(
            [cb](napi_env env, AbilityRuntime::NapiAsyncTask&, int32_t) {
                if (napi_delete_reference(env, cb) != napi_ok) {
                    FONT_LOGE("JsDataMigrationCallback failed to delete method reference.");
                    return;
                }
                FONT_LOGI("JsDataMigrationCallback delete method reference OK.");
            }
        );
    napi_ref callback = nullptr;
    std::unique_ptr<AbilityRuntime::NapiAsyncTask::ExecuteCallback> execute = nullptr;
    AbilityRuntime::NapiAsyncTask::Schedule("JsDataMigrationCallback::ReleaseRef",
                                            env_,
                                            std::make_unique<AbilityRuntime::NapiAsyncTask>(
                                                callback,
                                                std::move(execute),
                                                std::move(complete)
                                            ));
}

void JsDataMigrationCallback::CallJsMethod(napi_env env, const napi_value* argv, size_t argc)
{
    napi_value method = nullptr;
    if (napi_get_reference_value(env, cbRef_, &method) != napi_ok) {
        FONT_LOGE("JsDataMigrationCallback CallJsMethod failed to get method from reference.");
        return;
    }
    if (method == nullptr) {
        FONT_LOGE("JsDataMigrationCallback CallJsMethod failed to get method.");
        return;
    }
    napi_value callResult = nullptr;
    if (napi_call_function(env, nullptr, method, argc, argv, &callResult) != napi_ok) {
        FONT_LOGE("JsDataMigrationCallback CallJsMethod failed to callback js method.");
        return;
    }
}

napi_value GenerateErrMsg(const napi_env &env, int32_t errCode)
{
    napi_value errMsg;
    auto it = g_DataMigrationErrMsgMap.find(errCode);
    if (it != g_DataMigrationErrMsgMap.end()) {
        errMsg = AbilityRuntime::CreateJsError(env, errCode, g_DataMigrationErrMsgMap.at(errCode));
    } else {
        errMsg = AbilityRuntime::CreateJsError(env, ERR_SYSTEM_ERROR, g_DataMigrationErrMsgMap.at(ERR_SYSTEM_ERROR));
    }
    NAPI_ASSERT(env, errMsg != nullptr, "create error failed.");
    return errMsg;
}

void JsDataMigrationCallback::OnHandle(uint32_t errCode, const EventData& eventData)
{
    if (env_ == nullptr) {
        FONT_LOGE("JsDataMigrationCallback OnHandle env_ is null.");
        return;
    }
    std::unique_ptr<AbilityRuntime::NapiAsyncTask::CompleteCallback> complete =
        std::make_unique<AbilityRuntime::NapiAsyncTask::CompleteCallback>(
            [this, errCode, eventData](napi_env env, AbilityRuntime::NapiAsyncTask& task, int32_t status) {
                napi_value jsErrCode;
                if (errCode == ERR_OK) {
                    napi_create_uint32(env, errCode, &jsErrCode);
                } else {
                    jsErrCode = GenerateErrMsg(env, errCode);
                }
                napi_value jsEventData;
                napi_create_object(env, &jsEventData);
                napi_value value;
                napi_create_int32(env, static_cast<int32_t>(eventData.event), &value);
                napi_set_named_property(env, jsEventData, "event", value);
                napi_create_int32(env, static_cast<int32_t>(eventData.timeRemain), &value);
                napi_set_named_property(env, jsEventData, "timeRemain", value);
                napi_create_int32(env, static_cast<int32_t>(eventData.progressRate), &value);
                napi_set_named_property(env, jsEventData, "progressRate", value);
                napi_create_int32(env, static_cast<int32_t>(eventData.progressResult), &value);
                napi_set_named_property(env, jsEventData, "progressResult", value);
                
                napi_value argv[] = {jsErrCode, jsEventData};
                CallJsMethod(env, argv, AbilityRuntime::ArraySize(argv));
                FONT_LOGI("JsDataMigrationCallback CallJsMethod errCode:%{public}d, currentEventType:%{public}d",
                    errCode, eventData.event);
            }
        );
    napi_ref callback = nullptr;
    std::unique_ptr<AbilityRuntime::NapiAsyncTask::ExecuteCallback> execute = nullptr;
    AbilityRuntime::NapiAsyncTask::Schedule(
        "JsDataMigrationCallback::OnHandle", env_,
        std::make_unique<AbilityRuntime::NapiAsyncTask>(callback, std::move(execute), std::move(complete))
    );
}
} // namespace FontManager
} // namespace Global
} // namespace OHOS