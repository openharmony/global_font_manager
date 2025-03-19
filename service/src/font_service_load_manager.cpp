/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
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
 
#include "font_service_load_manager.h"
 
#include <thread>
#include "font_define.h"
#include "font_hilog.h"
#include "font_sa_load_callback.h"
 
namespace OHOS {
namespace Global {
namespace FontManager {
FontServiceLoadManager::FontServiceLoadManager() = default;
 
FontServiceLoadManager::~FontServiceLoadManager() = default;
 
sptr<IFontService> FontServiceLoadManager::GetFontServiceAbility(int32_t systemAbilityId)
{
    sptr<ISystemAbilityManager> manager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (manager == nullptr) {
        FONT_LOGE("manager is nullptr");
        return nullptr;
    }
    {
        std::lock_guard<std::mutex> lock(serviceLock_);
        sptr<IRemoteObject> object = manager->CheckSystemAbility(systemAbilityId);
        if (object != nullptr) {
            return iface_cast<IFontService>(object);
        }
    }

    if (!LoadSa(systemAbilityId)) {
        FONT_LOGE("loadSA failed");
        return nullptr;
    }
    sptr<IRemoteObject> object = manager->GetSystemAbility(systemAbilityId);
    if (object == nullptr) {
        FONT_LOGE("Get remote object from samgr failed");
        return nullptr;
    }
    return iface_cast<IFontService>(object);
}
 
void FontServiceLoadManager::OnLoadSystemAbilitySuccess()
{
    loadSaStatus_ = LoadSaStatus::SUCCESS;
    proxyConVar_.notify_one();
}
 
void FontServiceLoadManager::OnLoadSystemAbilityFail()
{
    FONT_LOGE("SystemAbility Load fail");
    loadSaStatus_ = LoadSaStatus::FAIL;
    proxyConVar_.notify_one();
}

void FontServiceLoadManager::InitStatus()
{
    loadSaStatus_ = LoadSaStatus::WAIT_RESULT;
}
 
bool FontServiceLoadManager::LoadSa(int systemAbilityId)
{
    InitStatus();
    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        FONT_LOGE("samgr object null!");
        return false;
    }
    sptr<FontSALoadCallback> fontSaLoadCallback = new FontSALoadCallback();
    if (fontSaLoadCallback == nullptr) {
        FONT_LOGE("systemAbilityId: %{public}d, create load callback failed", systemAbilityId);
        return false;
    }
    int32_t result = samgr->LoadSystemAbility(systemAbilityId, fontSaLoadCallback);
    if (result != ERR_OK) {
        FONT_LOGE("systemAbilityId: %{public}d, load failed, result code: %{public}d", systemAbilityId, result);
        return false;
    }
    
    {
        std::unique_lock<std::mutex> lock(serviceLock_);
        constexpr int64_t sleepTime = 5000;
        auto waitStatus = proxyConVar_.wait_for(lock, std::chrono::milliseconds(sleepTime),
            [this]() { return loadSaStatus_ == LoadSaStatus::SUCCESS; });
        if (!waitStatus) {
            FONT_LOGE("systemAbilityId: %{public}d, CheckSaLoaded failed", systemAbilityId);
            return false;
        }
    }
    FONT_LOGI("systemAbilityId: %{public}d, load succeed", systemAbilityId);
    return true;
}
 
bool FontServiceLoadManager::UnloadFontService(int32_t systemAbilityId)
{
    std::lock_guard<std::mutex> lock(serviceLock_);
    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        FONT_LOGE("FontServiceLoadManager::UnloadFontService can't get samgr.");
        return false;
    }
    int32_t ret = samgr->UnloadSystemAbility(systemAbilityId);
    if (ret != ERR_OK) {
        FONT_LOGE("FontServiceLoadManager::UnloadFontService sa unload failed.");
        return false;
    }
    return true;
}
} // namespace FontManager
} // namespace Global
} // namespace OHOS