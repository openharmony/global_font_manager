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

#ifndef FONT_MANAGER_JS_DATA_MIGRATION_LISTENER_H
#define FONT_MANAGER_JS_DATA_MIGRATION_LISTENER_H

#include "idata_migration_listener.h"
#include "js_func_ref_holder.h"

namespace OHOS {
namespace Global {
namespace FontManager {
class JsDataMigrationListener : public std::enable_shared_from_this<JsDataMigrationListener>,
    public IDataMigrationListener {
public:
    explicit JsDataMigrationListener(napi_env env) : env_(env){};
    JsDataMigrationListener(napi_env env, const std::shared_ptr<JsFuncRefHolder> &heartBeatCallback,
        const std::shared_ptr<JsFuncRefHolder> &progressCallback,
        const std::shared_ptr<JsFuncRefHolder> &resultCallback);
    ~JsDataMigrationListener() override;

    void OnHandle(const EventData& eventData) override;

private:
    void CallJsMethod(napi_ref funcRef, const napi_value* argv, size_t argc);
    void DoHeartbeatCallback();
    void DoProgressCallback(const EventData& eventData);
    void DoResultCallback(const EventData& eventData);
    napi_env env_ = nullptr;
    std::shared_ptr<JsFuncRefHolder> heartBeatCallback_ = nullptr;
    std::shared_ptr<JsFuncRefHolder> progressCallback_ = nullptr;
    std::shared_ptr<JsFuncRefHolder> resultCallback_ = nullptr;
};
} // namespace FontManager
} // namespace Global
} // namespace OHOS
#endif // FONT_MANAGER_JS_DATA_MIGRATION_LISTENER_H
