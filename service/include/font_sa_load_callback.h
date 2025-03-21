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

#ifndef FONT_MANAGER_FONT_SA_LOAD_CALLBACK_H
#define FONT_MANAGER_FONT_SA_LOAD_CALLBACK_H

#include "system_ability_load_callback_stub.h"

namespace OHOS {
namespace Global {
namespace FontManager {
/**
* @brief Callback implement when loading font service in FontServiceLoadManager.
*/
class FontSALoadCallback : public SystemAbilityLoadCallbackStub {
public:
    /**
     * @brief called when loading font service successful.
     *
     * @param systemAbilityId I18n system ability id which is FONT_SA_ID, introduced from LoadSystemAbility function.
     * @param remoteObject  Introduced from LoadSystemAbility function.
     */
    void OnLoadSystemAbilitySuccess(int32_t systemAbilityId, const sptr <IRemoteObject> &remoteObject) override;

    /**
     * @brief called when loading font service failed.
     *
     * @param systemAbilityId I18n system ability id which is FONT_SA_ID,  introduced from LoadSystemAbility function.
     */
    void OnLoadSystemAbilityFail(int32_t systemAbilityId) override;
};
} // namespace FontManager
} // namespace Global
} // namespace OHOS
#endif // FONT_MANAGER_FONT_SA_LOAD_CALLBACK_H
