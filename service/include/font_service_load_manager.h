/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef FONT_SERVICE_LOAD_MANAGER_H
#define FONT_SERVICE_LOAD_MANAGER_H

#include <mutex>
#include <unistd.h>
#include <singleton.h>

#include "ifont_service.h"
#include "iremote_object.h"
#include "iservice_registry.h"

namespace OHOS {
namespace Global {
namespace FontManager {
enum class LoadSaStatus {
    WAIT_RESULT = 0,
    SUCCESS,
    FAIL,
};
 
class FontServiceLoadManager : public DelayedSingleton<FontServiceLoadManager> {
public:
    FontServiceLoadManager();
    ~FontServiceLoadManager();
    sptr<IFontService> GetFontServiceAbility(int32_t systemAbilityId);
    void OnLoadSystemAbilitySuccess();
    void OnLoadSystemAbilityFail();
    bool UnloadFontService(int32_t systemAbilityId);
 
private:
    void InitStatus();
    bool LoadSa(int systemAbilityId);
 
    LoadSaStatus loadSaStatus_ = LoadSaStatus::WAIT_RESULT;
    std::condition_variable proxyConVar_;
    std::mutex serviceLock_;
};
} // namespace FontManager
} // namespace Global
} // namespace OHOS
#endif // FONT_SERVICE_LOAD_MANAGER_H