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
#include <uv.h>
#include "font_define.h"

namespace OHOS {
namespace Global {
namespace FontManager {
using namespace AbilityRuntime;
namespace {
struct DeleteRefHolder {
    napi_env env {nullptr};
    napi_ref ref {nullptr};
};
constexpr int32_t ARGS_ZERO = 0;
constexpr int32_t ARGS_ONE = 1;
}

JsRefHolder::JsRefHolder(napi_env env, napi_value value)
{
    if (env == nullptr || value == nullptr) {
        FONT_LOGE("JsRefHolder env or value is null.");
        return;
    }
    napi_valuetype valuetype;
    napi_status result = napi_typeof(env, value, &valuetype);
    if (result != napi_ok || valuetype != napi_function) {
        FONT_LOGE("JsRefHolder value is not function.");
        return;
    }
    result = napi_create_reference(env, value, 1, &ref_);
    if (result != napi_ok) {
        FONT_LOGE("JsRefHolder napi_create_reference fail.");
        ref_ = nullptr;
        return;
    }
    env_ = env;
}

JsRefHolder::~JsRefHolder()
{
    if (!IsValid()) {
        FONT_LOGI("JsRefHolder Invalid.");
        return;
    }
    FONT_LOGI("JsRefHolder delete reference.");
    uv_loop_s *loop = nullptr;
    napi_status napiStatus = napi_get_uv_event_loop(env_, &loop);
    if (napiStatus != napi_ok || loop == nullptr) {
        FONT_LOGE("JsRefHolder napi_get_uv_event_loop fail.");
        return;
    }
    std::shared_ptr<DeleteRefHolder> deleteRefHolder = std::make_shared<DeleteRefHolder>();
    if (deleteRefHolder == nullptr) {
        FONT_LOGE("JsRefHolder deleteRefHolder is nullptr.");
        return;
    }
    deleteRefHolder->env = env_;
    deleteRefHolder->ref = ref_;
    auto task = [deleteRefHolder] () {
        FONT_LOGI("JsRefHolder deleteRefHolder start.");
        if (deleteRefHolder == nullptr) {
            FONT_LOGE("JsRefHolder deleteRefHolder is nullptr.");
            return;
        }
        napi_status ret = napi_delete_reference(deleteRefHolder->env, deleteRefHolder->ref);
        if (ret != napi_ok) {
            FONT_LOGE("JsRefHolder napi_delete_reference fail %{public}d.", ret);
            return;
        }
    };
    if (napi_status::napi_ok != napi_send_event(env_, task, napi_eprio_immediate)) {
        FONT_LOGE("JsRefHolder napi_send_event faid.");
    }
}

bool JsRefHolder::IsValid() const
{
    return (env_ != nullptr && ref_ != nullptr);
}

napi_ref JsRefHolder::Get() const
{
    return ref_;
}

JsDataMigrationCallback::JsDataMigrationCallback(napi_env env, const std::shared_ptr<JsRefHolder> &heartBeatCallback,
    const std::shared_ptr<JsRefHolder> &progressCallback, const std::shared_ptr<JsRefHolder> &resultCallback)
    : env_(env), heartBeatCallback_(heartBeatCallback), progressCallback_(progressCallback),
    resultCallback_(resultCallback)
{
}

JsDataMigrationCallback::~JsDataMigrationCallback()
{
}

void JsDataMigrationCallback::CallJsMethod(napi_env env, napi_ref funcRef, const napi_value* argv, size_t argc)
{
    napi_value method = nullptr;
    if (napi_get_reference_value(env, funcRef, &method) != napi_ok) {
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

void JsDataMigrationCallback::OnHandle(const EventData& eventData)
{
    if (env_ == nullptr) {
        FONT_LOGE("JsDataMigrationCallback OnHandle env_ is null.");
        return;
    }
    std::unique_ptr<AbilityRuntime::NapiAsyncTask::CompleteCallback> complete =
        std::make_unique<AbilityRuntime::NapiAsyncTask::CompleteCallback>(
            [this, eventData](napi_env env, AbilityRuntime::NapiAsyncTask& task, int32_t status) {
                FONT_LOGI("JsDataMigrationCallback CallJsMethod currentEventType:%{public}d", eventData.event);
                if (eventData.event == ProgressType::HEART_BEAT) {
                    DoHeartbeatCallback();
                } else if (eventData.event == ProgressType::PROGRESS_DOING) {
                    DoProgressCallback(eventData);
                } else if (eventData.event == ProgressType::PROGRESS_RESULT) {
                    DoResultCallback(eventData);
                }
            }
        );
    napi_ref callback = nullptr;
    std::unique_ptr<AbilityRuntime::NapiAsyncTask::ExecuteCallback> execute = nullptr;
    AbilityRuntime::NapiAsyncTask::Schedule(
        "JsDataMigrationCallback::OnHandle", env_,
        std::make_unique<AbilityRuntime::NapiAsyncTask>(callback, std::move(execute), std::move(complete))
    );
}

void JsDataMigrationCallback::DoHeartbeatCallback()
{
    if (heartBeatCallback_ == nullptr) {
        FONT_LOGE("DoHeartbeatCallback heartBeatCallback_ is nullptr.");
        return;
    }
    napi_value params[ARGS_ZERO];
    return CallJsMethod(env_, heartBeatCallback_->Get(), params, ARGS_ZERO);
}

void JsDataMigrationCallback::DoProgressCallback(const EventData& eventData)
{
    if (progressCallback_ == nullptr) {
        FONT_LOGE("DoProgressCallback progressCallback_ is nullptr.");
        return;
    }
    napi_value progressData;
    napi_create_object(env_, &progressData);
    napi_value value;
    napi_create_int32(env_, static_cast<int32_t>(eventData.timeRemaining), &value);
    napi_set_named_property(env_, progressData, "timeRemaining", value);
    napi_create_int32(env_, static_cast<int32_t>(eventData.progressPercentage), &value);
    napi_set_named_property(env_, progressData, "progressPercentage", value);
    napi_value params[ARGS_ONE];
    params[0] = progressData;
    return CallJsMethod(env_, progressCallback_->Get(), params, ARGS_ONE);
}

void JsDataMigrationCallback::DoResultCallback(const EventData& eventData)
{
    if (resultCallback_ == nullptr) {
        FONT_LOGE("DoProgressCallback progressCallback_ is nullptr.");
        return;
    }
    napi_value value;
    napi_create_int32(env_, static_cast<int32_t>(eventData.progressResult), &value);
    napi_value params[ARGS_ONE];
    params[0] = value;
    return CallJsMethod(env_, resultCallback_->Get(), params, ARGS_ONE);
}
} // namespace FontManager
} // namespace Global
} // namespace OHOS