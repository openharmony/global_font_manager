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

#include "font_client_registry.h"

#include "font_define.h"
#include "font_hilog.h"
#include "font_manager.h"

namespace OHOS {
namespace Global {
namespace FontManager {

void FontClientDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    FONT_LOGI("Font client died, tokenId=%{public}d", tokenId_);
    FontClientRegistry::GetInstance()->OnClientDied(tokenId_);
}

FontClientRegistry::FontClientRegistry() {}

FontClientRegistry::~FontClientRegistry() {}

static std::string BuildAppIdentifier(int32_t scope, int32_t tokenId)
{
    if (scope == FONT_SCOPE_APP) {
        return "app_" + std::to_string(tokenId);
    }
    return "session_" + std::to_string(tokenId);
}

int32_t FontClientRegistry::RegisterClient(const sptr<IRemoteObject> &observerBinder,
    const std::string &bundleName, int32_t userId, int32_t tokenId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (clients_.find(tokenId) != clients_.end()) {
        FONT_LOGE("RegisterClient: tokenId=%{public}d already registered", tokenId);
        return ERR_SCOPE_FONT_REPEATED_REGISTER;
    }
    // check per-user limit
    int32_t count = static_cast<int32_t>(userClients_[userId].size());
    if (count >= MAX_SCOPE_FONT_APP_NUM) {
        FONT_LOGE("RegisterClient: exceed limit, userId=%{public}d count=%{public}d", userId, count);
        return ERR_SCOPE_FONT_EXCEED_REGISTER_LIMIT;
    }
    if (observerBinder == nullptr) {
        FONT_LOGE("RegisterClient: observerBinder is null, tokenId=%{public}d", tokenId);
        return ERR_SYSTEM_ERROR;
    }
    auto recipient = sptr<FontClientDeathRecipient>::MakeSptr(tokenId);
    if (recipient == nullptr) {
        FONT_LOGE("RegisterClient: alloc recipient failed");
        return ERR_SYSTEM_ERROR;
    }
    if (observerBinder->AddDeathRecipient(recipient)) {
        ClientInfo info;
        info.binder = observerBinder;
        info.bundleName = bundleName;
        info.userId = userId;
        info.tokenId = tokenId;
        info.appIdentifier = BuildAppIdentifier(FONT_SCOPE_APP, tokenId);
        info.recipient = recipient;
        clients_[tokenId] = info;
        userClients_[userId].insert(tokenId);
        FONT_LOGI("RegisterClient success, tokenId=%{public}d, bundleName=%{public}s",
            tokenId, bundleName.c_str());
        return ERR_OK;
    }
    FONT_LOGE("RegisterClient: AddDeathRecipient failed, tokenId=%{public}d", tokenId);
    return ERR_SYSTEM_ERROR;
}

int32_t FontClientRegistry::UnregisterClient(int32_t tokenId)
{
    std::string appIdentifier;
    int32_t userId = -1;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = clients_.find(tokenId);
        if (it == clients_.end()) {
            FONT_LOGE("UnregisterClient: tokenId=%{public}d not registered", tokenId);
            return ERR_SCOPE_FONT_NOT_REGISTERED;
        }
        if (it->second.binder != nullptr && it->second.recipient != nullptr) {
            it->second.binder->RemoveDeathRecipient(it->second.recipient);
        }
        appIdentifier = it->second.appIdentifier;
        userId = it->second.userId;
        clients_.erase(it);
        userClients_[userId].erase(tokenId);
    }
    // clean up app scope fonts for this client
    if (!appIdentifier.empty() && userId >= 0) {
        FontManager::GetInstance()->CleanupAppScopeFonts(appIdentifier, userId);
    }
    FONT_LOGI("UnregisterClient success, tokenId=%{public}d", tokenId);
    NotifyClientDied();
    return ERR_OK;
}

bool FontClientRegistry::IsClientRegistered(int32_t tokenId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    return clients_.find(tokenId) != clients_.end();
}

int32_t FontClientRegistry::GetClientCount()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return static_cast<int32_t>(clients_.size());
}

void FontClientRegistry::OnClientDied(int32_t tokenId)
{
    std::string appIdentifier;
    int32_t userId = -1;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = clients_.find(tokenId);
        if (it == clients_.end()) {
            return;
        }
        appIdentifier = it->second.appIdentifier;
        userId = it->second.userId;
        clients_.erase(it);
        userClients_[userId].erase(tokenId);
    }
    if (!appIdentifier.empty() && userId >= 0) {
        FontManager::GetInstance()->CleanupAppScopeFonts(appIdentifier, userId);
    }
    FONT_LOGI("OnClientDied cleanup done, tokenId=%{public}d", tokenId);
    NotifyClientDied();
}

void FontClientRegistry::SetClientDiedCallback(ClientDiedCallback callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    clientDiedCallback_ = std::move(callback);
}

void FontClientRegistry::NotifyClientDied()
{
    ClientDiedCallback callback;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        callback = clientDiedCallback_;
    }
    if (callback) {
        callback();
    }
}
} // namespace FontManager
} // namespace Global
} // namespace OHOS
