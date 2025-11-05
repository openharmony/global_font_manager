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

#ifndef FONT_MANAGER_DATA_MIGRATION_CALLBACK_H
#define FONT_MANAGER_DATA_MIGRATION_CALLBACK_H

#include "js_runtime_utils.h"
#include "native_engine/native_engine.h"
#include "native_engine/native_reference.h"
#include "native_engine/native_value.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "font_hilog.h"
#include "data_migration_callback.h"

namespace OHOS {
namespace Global {
namespace FontManager {
class JsRefHolder : public NoCopyable {
public:
    JsRefHolder(napi_env env, napi_value value);
    ~JsRefHolder() override;
    bool IsValid() const;
    napi_ref Get() const;
private:
    napi_env env_ {nullptr};
    napi_ref ref_ {nullptr};
};

class JsDataMigrationCallback : public DataMigrationCallback {
public:
    explicit JsDataMigrationCallback(napi_env env) : env_(env){};
    JsDataMigrationCallback(napi_env env, const std::shared_ptr<JsRefHolder> &heartBeatCallback,
        const std::shared_ptr<JsRefHolder> &progressCallback,
        const std::shared_ptr<JsRefHolder> &resultCallback);
    ~JsDataMigrationCallback() override;

    void OnHandle(uint32_t errCode, const EventData& eventData) override;

private:
    void CallJsMethod(napi_env env, napi_ref funcRef, const napi_value* argv, size_t argc);
    void DoHeartbeatCallback();
    void DoProgressCallback(const EventData& eventData);
    void DoResultCallback(const EventData& eventData);
    napi_env env_ = nullptr;
    std::shared_ptr<JsRefHolder> heartBeatCallback_ = nullptr;
    std::shared_ptr<JsRefHolder> progressCallback_ = nullptr;
    std::shared_ptr<JsRefHolder> resultCallback_ = nullptr;
};
} // namespace FontManager
} // namespace Global
} // namespace OHOS
#endif // FONT_MANAGER_DATA_MIGRATION_CALLBACK_H
