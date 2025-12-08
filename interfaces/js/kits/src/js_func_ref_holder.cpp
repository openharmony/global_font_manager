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

#include "js_func_ref_holder.h"

#include "font_define.h"
#include "font_hilog.h"

namespace OHOS {
namespace Global {
namespace FontManager {
using namespace AbilityRuntime;
namespace {
struct DeleteRefHolder {
    napi_env env {nullptr};
    napi_ref ref {nullptr};
};
}

JsFuncRefHolder::JsFuncRefHolder(napi_env env, napi_value value) : env_(nullptr), ref_(nullptr)
{
    if (env == nullptr || value == nullptr) {
        FONT_LOGE("JsFuncRefHolder env or value is null.");
        return;
    }
    napi_valuetype valuetype;
    napi_status result = napi_typeof(env, value, &valuetype);
    if (result != napi_ok || valuetype != napi_function) {
        FONT_LOGE("JsFuncRefHolder value is not function.");
        return;
    }
    result = napi_create_reference(env, value, 1, &ref_);
    if (result != napi_ok) {
        FONT_LOGE("JsFuncRefHolder napi_create_reference fail.");
        ref_ = nullptr;
        return;
    }
    env_ = env;
}

JsFuncRefHolder::~JsFuncRefHolder()
{
    if (!IsValid()) {
        FONT_LOGI("JsFuncRefHolder Invalid.");
        return;
    }
    uv_loop_s *loop = nullptr;
    napi_status napiStatus = napi_get_uv_event_loop(env_, &loop);
    if (napiStatus != napi_ok || loop == nullptr) {
        napi_delete_reference(env_, ref_);
        FONT_LOGE("JsFuncRefHolder napi_get_uv_event_loop fail.");
        return;
    }
    std::shared_ptr<DeleteRefHolder> deleteRefHolder = std::make_unique<DeleteRefHolder>();
    deleteRefHolder->env = env_;
    deleteRefHolder->ref = ref_;
    auto task = [handler = std::move(deleteRefHolder)] () {
        FONT_LOGI("JsFuncRefHolder deleteRefHoldertask start.");
        napi_delete_reference(handler->env, handler->ref);
    };
    if (napi_send_event(env_, task, napi_eprio_immediate) != napi_status::napi_ok) {
        napi_delete_reference(env_, ref_);
    }
}

bool JsFuncRefHolder::IsValid() const
{
    return (env_ != nullptr && ref_ != nullptr);
}

napi_ref JsFuncRefHolder::Get() const
{
    return ref_;
}
} // namespace FontManager
} // namespace Global
} // namespace OHOS