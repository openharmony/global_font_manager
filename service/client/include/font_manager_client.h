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

#ifndef GLOBAL_FONT_MANAGER_FONT_SERVICE_CLIENT_H
#define GLOBAL_FONT_MANAGER_FONT_SERVICE_CLIENT_H

#include <string>
#include <unistd.h>
#include <mutex>
#include <singleton.h>

#include "font_manager_kits.h"
#include "data_migration_cb_agent.h"
#include "font_service_load_manager.h"
#include "ifont_client_observer.h"

namespace OHOS {
namespace Global {
namespace FontManager {
class FontServiceDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    explicit FontServiceDeathRecipient(sptr<IFontClientObserver> observer) : observer_(observer) {}
    ~FontServiceDeathRecipient() override = default;
    void OnRemoteDied(const wptr<IRemoteObject> &remote) override;
private:
    sptr<IFontClientObserver> observer_;
};

class FontManagerClient : public FontManagerKits, public DelayedSingleton<FontManagerClient> {
    DECLARE_DELAYED_REF_SINGLETON(FontManagerClient);
using CbAgentPtr = sptr<DataMigrationCbAgent>;
public:
    DISALLOW_COPY_AND_MOVE(FontManagerClient);
    int32_t InstallFont(const std::string &fontPath, int &outValue) override;
    int32_t UninstallFont(const std::string &fontName, int &outValue) override;
    int32_t DataMigration(std::shared_ptr<IDataMigrationListener> listener) override;
    int32_t InstallFontWithUserId(const std::string &fontPath, int32_t userId);
    int32_t UninstallFontWithUserId(const std::string &fontName, int32_t userId);

    int32_t OnFontObserver(const sptr<IFontClientObserver>& observer) override;
    int32_t OffFontObserver(const sptr<IFontClientObserver>& observer) override;
    int32_t InstallScopeFont(const std::string &fontPath, int32_t scope, int32_t &outValue) override;
    int32_t UninstallScopeFont(const std::string &srcPath, int32_t &outValue) override;
    int32_t GetFontScope(const std::string &srcPath, int32_t &outValue) override;

private:
    bool PathToRealPath(const std::string& path, std::string& realPath);
    std::mutex observerLock_;
    sptr<IRemoteObject> saBinder_;
    sptr<FontServiceDeathRecipient> saDeathRecipient_;
};
} // namespace FontManager
} // namespace Global
} // namespace OHOS
#endif // GLOBAL_FONT_MANAGER_FONT_SERVICE_CLIENT_H
