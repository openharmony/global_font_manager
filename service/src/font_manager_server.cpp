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
 
#include "font_manager_server.h"

#include "accesstoken_kit.h"
#include "file_utils.h"
#include "font_define.h"
#include "font_hilog.h"
#include "font_manager.h"
#include "font_service_load_manager.h"
#include "ipc_skeleton.h"
#include "tokenid_kit.h"

namespace OHOS {
namespace Global {
namespace FontManager {
REGISTER_SYSTEM_ABILITY_BY_ID(FontManagerServer, FONT_SA_ID, false);
static const std::string FONTS_TEMP_PATH = "/data/service/el1/public/for-all-app/fonts/temp/";
static const std::string UNLOAD_TASK = "font_service_unload";
static const std::string PERMISSION_UPDATE_FONT = "ohos.permission.UPDATE_FONT";
static const uint32_t DELAY_MILLISECONDS_FOR_UNLOAD_SA = 10000;

FontManagerServer::FontManagerServer(int32_t saId, bool runOnCreate) : SystemAbility(saId, runOnCreate)
{
}

int32_t FontManagerServer::InstallFont(const int32_t fd, int32_t &outValue)
{
    int32_t err = CheckPermission();
    if (err != SUCCESS) {
        outValue = static_cast<int32_t>(ERR_NO_PERMISSION);
        return SUCCESS;
    }
    outValue = FontManager::GetInstance()->InstallFont(fd);
    UnloadFontServiceAbility();
    return SUCCESS;
}

int32_t FontManagerServer::UninstallFont(const std::string &fontName, int32_t &outValue)
{
    int32_t err = CheckPermission();
    if (err != SUCCESS) {
        outValue = static_cast<int32_t>(ERR_NO_PERMISSION);
        return SUCCESS;
    }
    outValue = FontManager::GetInstance()->UninstallFont(fontName);
    UnloadFontServiceAbility();
    return SUCCESS;
}

void FontManagerServer::UnloadFontServiceAbility()
{
    auto task = [this]() {
        auto fontSaLoadManager = DelayedSingleton<FontServiceLoadManager>::GetInstance();
        if (fontSaLoadManager != nullptr) {
            FONT_LOGI("FontManagerServer::UnloadFontServiceAbility start to unload fontManager SA.");
            fontSaLoadManager->UnloadFontService(FONT_SA_ID);
        }
    };
    if (handler_ != nullptr) {
        handler_->RemoveTask(UNLOAD_TASK);
        handler_->PostTask(task, UNLOAD_TASK, DELAY_MILLISECONDS_FOR_UNLOAD_SA);
    }
}

void FontManagerServer::OnStart(const SystemAbilityOnDemandReason &startReason)
{
    FONT_LOGI("FontManagerServer OnStart, startReason name %{public}s", startReason.GetName().c_str());
    bool status = Publish(this);
    if (status) {
        FONT_LOGI("FontManagerServer Publish success.");
    } else {
        FONT_LOGI("FontManagerServer Publish failed.");
    }
    FileUtils::DeleteDir(FONTS_TEMP_PATH, false);
    handler_ = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::Create(true));
    UnloadFontServiceAbility();
}

void FontManagerServer::OnStop(const SystemAbilityOnDemandReason &stopReason)
{
    FONT_LOGI("FontManagerServer OnStop, stopReason name %{public}s", stopReason.GetName().c_str());
}

int32_t FontManagerServer::CheckPermission()
{
    uint64_t accessTokenID = IPCSkeleton::GetCallingFullTokenID();
    uint32_t callerToken = IPCSkeleton::GetCallingTokenID();
    bool isSystemApp = Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(accessTokenID);
    if (!isSystemApp) {
        FONT_LOGE("caller process is not System app.");
        return ERR_NOT_SYSTEM_APP;
    }
    int result = Security::AccessToken::AccessTokenKit::VerifyAccessToken(callerToken, PERMISSION_UPDATE_FONT);
    if (result != Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
        FONT_LOGE("FontManagerServer caller process doesn't have UPDATE_FONT permission.");
        return ERR_NO_PERMISSION;
    }
    return SUCCESS;
}
} // namespace FontManager
} // namespace Global
} // namespace OHOS