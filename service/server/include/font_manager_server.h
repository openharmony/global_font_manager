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

#ifndef GLOBAL_FONT_MANAGER_FONT_SERVER_H
#define GLOBAL_FONT_MANAGER_FONT_SERVER_H

#include <atomic>
#include "event_handler.h"
#include "font_service_stub.h"
#include "system_ability.h"
#include "system_ability_ondemand_reason.h"
#include "font_manager.h"

namespace OHOS {
namespace Global {
namespace FontManager {
using RemoteCallbackPtr = sptr<IDataMigrationCallback>;
class FontManagerServer : public SystemAbility, public FontServiceStub {
    DECLARE_SYSTEM_ABILITY(FontManagerServer);
public:
    DISALLOW_COPY_AND_MOVE(FontManagerServer);

    FontManagerServer(int32_t saId, bool runOnCreate);

    ~FontManagerServer() override = default;

    int32_t InstallFont(const int32_t fd, int32_t &outValue) override;

    int32_t UninstallFont(const std::string &fontName, int32_t &outValue) override;

    int32_t DataMigration(const RemoteCallbackPtr& callback) override;

protected:
    void OnStart(const SystemAbilityOnDemandReason &startReason) override;

    void OnStop(const SystemAbilityOnDemandReason &startReason) override;

private:
    void InstallFontInner(const int32_t fd, int32_t &outValue);
    void UninstallFontInner(const std::string &fontName, int32_t &outValue);
    int32_t DataMigrationInner(const sptr<IDataMigrationCallback>& callback);
    void AddUnloadFontServiceTask();
    void RemoveUnloadFontServiceTask();
    int32_t CheckPermission();
    // font service unload event handler.
    std::shared_ptr <AppExecFwk::EventHandler> handler_;
    void StartDataMigrationTask(const RemoteCallbackPtr& callback);
    std::atomic_uint callingCount_ {0};
    std::atomic<bool> isDataMigrationing_ {false};
};
} // namespace FontManager
} // namespace Global
} // namespace OHOS
#endif // GLOBAL_FONT_MANAGER_FONT_SERVER_H
