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
#include "font_manager_utils.h"
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
    if (callingCount_ == 0) {
        AddUnloadFontServiceTask();
    }
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
    if (outValue != ERR_OK) {
        FONT_LOGE("FontManagerServer:InstallFont failed.ErrCode:%{public}d", outValue);
    }
}

int32_t FontManagerServer::UninstallFont(const std::string &fontName, int32_t &outValue)
{
    RemoveUnloadFontServiceTask();
    callingCount_++;
    UninstallFontInner(fontName, outValue);
    callingCount_--;
    if (callingCount_ == 0) {
        AddUnloadFontServiceTask();
    }
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
        return ret;
    }
    if (handler_ == nullptr) {
        FONT_LOGE("FontManagerServer handler_ is null.");
        return ERR_SYSTEM_ERROR;
    }
    bool expected = false;
    if (!isDataMigrationing_.compare_exchange_strong(expected, true)) {
        FONT_LOGE("FontManagerServer is DataMigrationing.");
        return ERR_DATA_MIGRATIONING;
    }
    auto task = [this, callback]() {
        StartDataMigrationTask(callback);
    };
    if (!handler_->PostTask(task)) {
        isDataMigrationing_ = false;
        FONT_LOGE("FontManagerServer PostTask ERR.");
        return ERR_SYSTEM_ERROR;
    }
    FONT_LOGI("FontManagerServer DataMigration call success.");
    return ERR_OK;
}

void FontManagerServer::StartDataMigrationTask(const sptr<IDataMigrationCallback>& callback)
{
    RemoveUnloadFontServiceTask();
    DataMigrationManager::GetInstance()->DataMigration(callback);
    isDataMigrationing_ = false;
    FONT_LOGI("FontManagerServer DataMigration finish.");
    AddUnloadFontServiceTask();
}

void FontManagerServer::AddUnloadFontServiceTask()
{
    if (callingCount_ > 0 || isDataMigrationing_) {
        return;
    }
    auto task = [this]() {
        if (callingCount_ == 0 && !isDataMigrationing_) {
            auto fontSaLoadManager = DelayedSingleton<FontServiceLoadManager>::GetInstance();
            if (fontSaLoadManager != nullptr) {
                FONT_LOGI("FontManagerServer start to unload fontManager SA.");
                FontManagerUtils::ClearAllTempFileDir();
                fontSaLoadManager->UnloadFontService(FONT_SA_ID);
            }
        }
    };
    if (handler_ != nullptr) {
        handler_->PostTask(task, UNLOAD_TASK, DELAY_MILLISECONDS_FOR_UNLOAD_SA);
    } else {
        FONT_LOGE("FontManagerServer add task failed, handler is nullptr");
    }
}

void FontManagerServer::RemoveUnloadFontServiceTask()
{
    if (handler_ != nullptr) {
        handler_->RemoveTask(UNLOAD_TASK);
    } else {
        FONT_LOGE("FontManagerServer remove task failed, handler is nullptr");
    }
}

void FontManagerServer::OnStart(const SystemAbilityOnDemandReason &startReason)
{
    std::string reasonName = startReason.GetName();
    FONT_LOGI("FontManagerServer OnStart, startReason name %{public}s", reasonName.c_str());
    handler_ = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::Create(true));
    AddUnloadFontServiceTask();
    bool status = Publish(this);
    if (status) {
        FONT_LOGI("FontManagerServer Publish success.");
    } else {
        FONT_LOGI("FontManagerServer Publish failed.");
    }
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
        FONT_LOGE("FontManagerServer caller process doesn't have permission.");
        return ERR_NO_PERMISSION;
    }
    FONT_LOGI("FontManagerServer CheckPermission success.");
    return ERR_OK;
}
} // namespace FontManager
} // namespace Global
} // namespace OHOS