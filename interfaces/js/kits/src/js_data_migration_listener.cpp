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

#include "js_data_migration_listener.h"

#include "font_define.h"
#include "font_hilog.h"

namespace OHOS {
namespace Global {
namespace FontManager {
using namespace AbilityRuntime;
namespace {
constexpr int32_t ARGS_ZERO = 0;
constexpr int32_t ARGS_ONE = 1;
}

JsDataMigrationListener::JsDataMigrationListener(napi_env env, const std::shared_ptr<JsFuncRefHolder> &heartBeatCallback,
    const std::shared_ptr<JsFuncRefHolder> &progressCallback, const std::shared_ptr<JsFuncRefHolder> &resultCallback)
    : env_(env), heartBeatCallback_(heartBeatCallback), progressCallback_(progressCallback),
    resultCallback_(resultCallback)
{
}

JsDataMigrationListener::~JsDataMigrationListener()
{
}

void JsDataMigrationListener::CallJsMethod(napi_ref funcRef, const napi_value* argv, size_t argc)
{
    napi_value method = nullptr;
    if (napi_get_reference_value(env_, funcRef, &method) != napi_ok) {
        FONT_LOGE("JsDataMigrationListener CallJsMethod failed to get method from reference.");
        return;
    }
    if (method == nullptr) {
        FONT_LOGE("JsDataMigrationListener CallJsMethod failed to get method.");
        return;
    }
    napi_value callResult = nullptr;
    if (napi_call_function(env_, nullptr, method, argc, argv, &callResult) != napi_ok) {
        FONT_LOGE("JsDataMigrationListener CallJsMethod failed to callback js method.");
        return;
    }
}

void JsDataMigrationListener::OnHandle(const EventData& eventData)
{
    if (env_ == nullptr) {
        FONT_LOGE("JsDataMigrationListener OnHandle env_ is null.");
        return;
    }
    auto self = shared_from_this();
    std::unique_ptr<AbilityRuntime::NapiAsyncTask::CompleteCallback> complete =
        std::make_unique<AbilityRuntime::NapiAsyncTask::CompleteCallback>(
            [self, eventData](napi_env env, AbilityRuntime::NapiAsyncTask& task, int32_t status) {
                FONT_LOGI("JsDataMigrationListener CallJsMethod currentEventType:%{public}d", eventData.event);
                if (eventData.event == EventType::HEART_BEAT) {
                    self->DoHeartbeatCallback();
                } else if (eventData.event == EventType::PROGRESS_DOING) {
                    self->DoProgressCallback(eventData);
                } else if (eventData.event == EventType::PROGRESS_RESULT) {
                    self->DoResultCallback(eventData);
                }
            }
        );
    napi_ref callback = nullptr;
    std::unique_ptr<AbilityRuntime::NapiAsyncTask::ExecuteCallback> execute = nullptr;
    AbilityRuntime::NapiAsyncTask::Schedule(
        "JsDataMigrationListener::OnHandle", env_,
        std::make_unique<AbilityRuntime::NapiAsyncTask>(callback, std::move(execute), std::move(complete))
    );
}

void JsDataMigrationListener::DoHeartbeatCallback()
{
    if (heartBeatCallback_ == nullptr) {
        FONT_LOGE("DoHeartbeatCallback heartBeatCallback_ is nullptr.");
        return;
    }
    napi_value params[ARGS_ZERO];
    return CallJsMethod(heartBeatCallback_->Get(), params, ARGS_ZERO);
}

void JsDataMigrationListener::DoProgressCallback(const EventData& eventData)
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
    return CallJsMethod(progressCallback_->Get(), params, ARGS_ONE);
}

void JsDataMigrationListener::DoResultCallback(const EventData& eventData)
{
    if (resultCallback_ == nullptr) {
        FONT_LOGE("DoProgressCallback progressCallback_ is nullptr.");
        return;
    }
    napi_value value;
    napi_create_int32(env_, static_cast<int32_t>(eventData.progressResult), &value);
    napi_value params[ARGS_ONE];
    params[0] = value;
    return CallJsMethod(resultCallback_->Get(), params, ARGS_ONE);
}
} // namespace FontManager
} // namespace Global
} // namespace OHOS