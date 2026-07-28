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

#ifndef GLOBAL_FONT_MANAGER_FONT_CLIENT_REGISTRY_H
#define GLOBAL_FONT_MANAGER_FONT_CLIENT_REGISTRY_H

#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>
#include <set>

#include "iremote_object.h"
#include "singleton.h"

namespace OHOS {
namespace Global {
namespace FontManager {
class FontClientRegistry : public DelayedSingleton<FontClientRegistry> {
    DECLARE_DELAYED_SINGLETON(FontClientRegistry);
public:
    using ClientDiedCallback = std::function<void()>;
    int32_t RegisterClient(const sptr<IRemoteObject> &observerBinder,
        const std::string &bundleName, int32_t userId, int32_t tokenId);
    int32_t UnregisterClient(int32_t tokenId);
    bool IsClientRegistered(int32_t tokenId);
    int32_t GetClientCount();
    void OnClientDied(int32_t tokenId);
    void SetClientDiedCallback(ClientDiedCallback callback);

private:
    void NotifyClientDied();
    struct ClientInfo {
        sptr<IRemoteObject> binder;
        std::string bundleName;
        int32_t userId = -1;
        int32_t tokenId = 0;
        std::string appIdentifier;
        sptr<IRemoteObject::DeathRecipient> recipient;
    };
    std::mutex mutex_;
    std::unordered_map<int32_t, ClientInfo> clients_;
    std::unordered_map<int32_t, std::set<int32_t>> userClients_;
    ClientDiedCallback clientDiedCallback_;
};

class FontClientDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    explicit FontClientDeathRecipient(int32_t tokenId) : tokenId_(tokenId) {}
    ~FontClientDeathRecipient() override = default;
    void OnRemoteDied(const wptr<IRemoteObject> &remote) override;
private:
    int32_t tokenId_;
};
} // namespace FontManager
} // namespace Global
} // namespace OHOS
#endif // GLOBAL_FONT_MANAGER_FONT_CLIENT_REGISTRY_H
