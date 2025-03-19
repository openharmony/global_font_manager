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

#include "font_sa_load_callback.h"

#include <singleton.h>
#include "font_hilog.h"
#include "font_service_load_manager.h"
#include "iremote_object.h"

namespace OHOS {
namespace Global {
namespace FontManager {
void FontSALoadCallback::OnLoadSystemAbilitySuccess(int32_t systemAbilityId,
    const sptr<IRemoteObject> &remoteObject)
{
    auto loadMgr = DelayedSingleton<FontServiceLoadManager>::GetInstance();
    if (loadMgr == nullptr) {
        FONT_LOGE("FontSALoadCallback::OnLoadSystemAbilitySuccess get load manager failed.");
        return;
    }
    loadMgr->OnLoadSystemAbilitySuccess();
}

void FontSALoadCallback::OnLoadSystemAbilityFail(int32_t systemAbilityId)
{
    auto loadMgr = DelayedSingleton<FontServiceLoadManager>::GetInstance();
    if (loadMgr == nullptr) {
        FONT_LOGE("FontSALoadCallback::OnLoadSystemAbilityFail get load manager failed.");
        return;
    }
    loadMgr->OnLoadSystemAbilityFail();
}
} // namespace FontManager
} // namespace Global
} // namespace OHOS