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

#include <chrono>
#include "accesstoken_kit.h"
#include "file_utils.h"
#include "font_define.h"
#include "font_hilog.h"
#include "font_manager.h"
#include "data_migration_manager.h"
#include "font_service_load_manager.h"
#include "ipc_skeleton.h"
#include "tokenid_kit.h"
#include "common_event_support.h"
#ifdef ACCOUNT_ENABLE
#include "os_account_manager.h"
#endif
namespace OHOS {
namespace Global {
namespace FontManager {
REGISTER_SYSTEM_ABILITY_BY_ID(FontManagerServer, FONT_SA_ID, false);
namespace {
static const std::string UNLOAD_TASK = "font_service_unload";
static const std::string PERMISSION_UPDATE_FONT = "ohos.permission.UPDATE_FONT";
static constexpr uint32_t DELAY_MILLISECONDS_FOR_UNLOAD_SA = 10000;
static constexpr uint32_t ONE_CALLING = 1;
static constexpr int32_t INVALID_USERID = -1;
}
FontManagerServer::FontManagerServer(int32_t saId, bool runOnCreate) : SystemAbility(saId, runOnCreate)
{
}

int32_t FontManagerServer::InstallFont(const int32_t fd, int32_t &outValue)
{
    RemoveUnloadFontServiceTask();
    callingCount_++;
    InstallFontInner(fd, outValue);
    callingCount_--;
    AddUnloadFontServiceTask();
    return ERR_OK;
}

void FontManagerServer::InstallFontInner(const int32_t fd, int32_t &outValue)
{
    int32_t userId = INVALID_USERID;
    int32_t ret = CheckPermission();
    if (ret != ERR_OK) {
        outValue = ret;
        return;
    }
#ifdef ACCOUNT_ENABLE
    ret = AccountSA::OsAccountManager::GetOsAccountLocalIdFromUid(IPCSkeleton::GetCallingUid(), userId);
    if (ret != ERR_OK) {
        outValue = ERR_INSTALL_FAIL;
        return;
    }
#else
    FONT_LOGE("FontManagerServer:this device not support os account.");
    outValue = ERR_INSTALL_FAIL;
    return;
#endif
    outValue = FontManager::GetInstance()->InstallFont(fd, userId);
    if (callingCount_ == ONE_CALLING) {
        std::string installPath = INSTALL_PATH_PREFIX + std::to_string(userId) + "/";
        FileUtils::DeleteDir(installPath + TEMP_FILE, true);
    }
}

int32_t FontManagerServer::UninstallFont(const std::string &fontName, int32_t &outValue)
{
    RemoveUnloadFontServiceTask();
    callingCount_++;
    UninstallFontInner(fontName, outValue);
    callingCount_--;
    AddUnloadFontServiceTask();
    return ERR_OK;
}

void FontManagerServer::UninstallFontInner(const std::string &fontName, int32_t &outValue)
{
    int32_t userId = INVALID_USERID;
    int32_t ret = CheckPermission();
    if (ret != ERR_OK) {
        outValue = ret;
        return;
    }
#ifdef ACCOUNT_ENABLE
    ret = AccountSA::OsAccountManager::GetOsAccountLocalIdFromUid(IPCSkeleton::GetCallingUid(), userId);
    if (ret != ERR_OK) {
        outValue = ERR_UNINSTALL_FAIL;
        return;
    }
#else
    FONT_LOGE("FontManagerServer:this device not support os account.");
    outValue = ERR_UNINSTALL_FAIL;
    return;
#endif
    outValue = FontManager::GetInstance()->UninstallFont(fontName, userId);
}

int32_t FontManagerServer::DataMigration(const sptr<IDataMigrationCallback>& callback)
{
    RemoveUnloadFontServiceTask();
    callingCount_++;
    int32_t ret = DataMigrationInner(callback);
    callingCount_--;
    AddUnloadFontServiceTask();
    return ret;
}

int32_t FontManagerServer::DataMigrationInner(const sptr<IDataMigrationCallback>& callback)
{
    int32_t ret = CheckPermission();
    if (ret != ERR_OK) {
        FONT_LOGI("FontManagerServer no permission.");
        return ret;
    }
    if (handler_ == nullptr) {
        FONT_LOGE("FontManagerServer handler_ is null.");
        return ERR_SYSTEM_ERROR;
    }
    if (isDataMigrationing_) {
        FONT_LOGE("FontManagerServer is DataMigrationing.");
        return ERR_DATA_MIGRATIONING;
    }
    auto task = [this, callback]() {
        StartDataMigrationTask(callback);
    };
    handler_->PostTask(task);
    FONT_LOGI("FontManagerServer DataMigration call success.");
    return ERR_OK;
}

void FontManagerServer::StartDataMigrationTask(const sptr<IDataMigrationCallback>& callback)
{
    RemoveUnloadFontServiceTask();
    isDataMigrationing_ = true;
    DataMigrationManager::GetInstance()->DataMigration(callback);
    isDataMigrationing_ = false;
    FONT_LOGI("FontManagerServer DataMigration finish.");
    AddUnloadFontServiceTask();
}

void FontManagerServer::AddUnloadFontServiceTask()
{
    auto task = [this]() {
        if (callingCount_ == 0 && !isDataMigrationing_) {
            auto fontSaLoadManager = DelayedSingleton<FontServiceLoadManager>::GetInstance();
            if (fontSaLoadManager != nullptr) {
                FONT_LOGI("FontManagerServer start to unload fontManager SA.");
                fontSaLoadManager->UnloadFontService(FONT_SA_ID);
            }
        }
    };
    if (handler_ != nullptr) {
        handler_->PostTask(task, UNLOAD_TASK, DELAY_MILLISECONDS_FOR_UNLOAD_SA);
    }
}

void FontManagerServer::RemoveUnloadFontServiceTask()
{
    if (handler_ != nullptr) {
        handler_->RemoveTask(UNLOAD_TASK);
    }
}

void FontManagerServer::OnStart(const SystemAbilityOnDemandReason &startReason)
{
    std::string reasonName = startReason.GetName();
    FONT_LOGI("FontManagerServer OnStart, startReason name %{public}s", reasonName.c_str());
    bool status = Publish(this);
    if (status) {
        FONT_LOGI("FontManagerServer Publish success.");
    } else {
        FONT_LOGI("FontManagerServer Publish failed.");
    }
    if (reasonName == EventFwk::CommonEventSupport::COMMON_EVENT_USER_ADDED) {
        std::string userId = startReason.GetValue();
        std::string installPath = INSTALL_PATH_PREFIX + userId + "/";
        FontManager::GetInstance()->CheckAndInitInstallPath(installPath);
    }
    if (reasonName == EventFwk::CommonEventSupport::COMMON_EVENT_USER_REMOVED) {
        std::string userId = startReason.GetValue();
        std::string installPath = INSTALL_PATH_PREFIX + userId + "/";
        FileUtils::DeleteDir(installPath, true);
        FONT_LOGI("FontManagerServer DeleteUserInstallDir finish.");
    }

    handler_ = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::Create(true));
    AddUnloadFontServiceTask();
}

void FontManagerServer::OnStop(const SystemAbilityOnDemandReason &stopReason)
{
    FONT_LOGI("FontManagerServer OnStop, stopReason name %{public}s", stopReason.GetName().c_str());
}

int32_t FontManagerServer::CheckPermission()
{
    uint32_t callerToken = IPCSkeleton::GetCallingTokenID();
    int result = Security::AccessToken::AccessTokenKit::VerifyAccessToken(callerToken, PERMISSION_UPDATE_FONT);
    if (result != Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
        FONT_LOGE("FontManagerServer caller process doesn't have UPDATE_FONT permission.");
        return ERR_NO_PERMISSION;
    }
    return ERR_OK;
}
} // namespace FontManager
} // namespace Global
} // namespace OHOS