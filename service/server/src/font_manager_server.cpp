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

#include <cstdlib>
#include <cerrno>
#include <climits>
#include <chrono>
#include "accesstoken_kit.h"
#include "font_manager_utils.h"
#include "font_define.h"
#include "font_hilog.h"
#include "font_manager.h"
#include "font_client_registry.h"
#include "data_migration_manager.h"
#include "font_service_load_manager.h"
#include "ipc_skeleton.h"
#include "tokenid_kit.h"
#include "common_event_support.h"
#include "common_event_manager.h"
#include "common_event_subscriber.h"
#include "want.h"
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
static const std::string PERMISSION_UPDATE_SCOPE_FONT = "ohos.permission.UPDATE_SCOPE_FONT";
static const std::string REASON_BOOT_COMPLETED = "usual.event.BOOT_COMPLETED";
static const std::string REASON_USER_STOPPING = "usual.event.USER_STOPPING";
static constexpr uint32_t DELAY_MILLISECONDS_FOR_UNLOAD_SA = 10000;
static constexpr int32_t INVALID_USERID = -1;
}

class FontManagerEventSubscriber : public EventFwk::CommonEventSubscriber {
public:
    using EventCallback = std::function<void(int32_t)>;
    FontManagerEventSubscriber(const EventFwk::CommonEventSubscribeInfo &info, EventCallback callback)
        : CommonEventSubscriber(info), callback_(std::move(callback)) {}
    ~FontManagerEventSubscriber() override = default;

    void OnReceiveEvent(const EventFwk::CommonEventData &data) override
    {
        std::string action = data.GetWant().GetAction();
        FONT_LOGI("OnReceiveEvent action=%{public}s", action.c_str());
        if (action == REASON_USER_STOPPING) {
            int32_t userId = data.GetCode();
            FONT_LOGI("USER_STOPPING userId=%{public}d", userId);
            if (userId >= 0) {
                callback_(userId);
            }
        }
    }

private:
    EventCallback callback_;
};
FontManagerServer::FontManagerServer(int32_t saId, bool runOnCreate) : SystemAbility(saId, runOnCreate)
{
}

int32_t FontManagerServer::InstallFont(const int32_t fd, int32_t &outValue)
{
    CallingCountGuard guard(this, false);
    InstallFontInner(fd, outValue);
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
    CallingCountGuard guard(this, false);
    UninstallFontInner(fontName, outValue);
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
    CallingCountGuard guard(this, false);
    return DataMigrationInner(callback);
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
    if (callingCount_ > 0 || isDataMigrationing_ ||
        FontClientRegistry::GetInstance()->GetClientCount() > 0) {
        return;
    }
    auto task = [this]() {
        if (callingCount_ != 0 || isDataMigrationing_ ||
            FontClientRegistry::GetInstance()->GetClientCount() > 0) {
            return;
        }
        FontManagerUtils::ClearAllTempFileDir();
        FontManagerUtils::CleanupScopeFontDirs();
        auto fontSaLoadManager = DelayedSingleton<FontServiceLoadManager>::GetInstance();
        if (fontSaLoadManager != nullptr) {
            FONT_LOGI("FontManagerServer start to unload fontManager SA.");
            fontSaLoadManager->UnloadFontService(FONT_SA_ID);
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

    FontClientRegistry::GetInstance()->SetClientDiedCallback([this]() {
        if (callingCount_ == 0 && !isDataMigrationing_ &&
            FontClientRegistry::GetInstance()->GetClientCount() == 0) {
            AddUnloadFontServiceTask();
        }
    });

    if (reasonName == REASON_BOOT_COMPLETED) {
        auto task = [this]() { CleanupAllScopeFontsOnBoot(); };
        handler_->PostTask(task);
    } else if (reasonName == REASON_USER_STOPPING) {
        int32_t userId = ParseUserIdFromReason(startReason);
        auto task = [this, userId]() { CleanupUserScopeFonts(userId); };
        handler_->PostTask(task);
    } else {
        auto task = [this]() { CleanupAppScopeFontsOnStart(); };
        handler_->PostTask(task);
    }

    AddUnloadFontServiceTask();
    SubscribeCommonEvent();
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
    FontClientRegistry::GetInstance()->SetClientDiedCallback(nullptr);
    if (subscriber_ != nullptr) {
        EventFwk::CommonEventManager::UnSubscribeCommonEvent(subscriber_);
        subscriber_ = nullptr;
    }
}

void FontManagerServer::SubscribeCommonEvent()
{
    if (subscriber_ != nullptr) {
        return;
    }
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(REASON_USER_STOPPING);
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto callback = [this](int32_t userId) {
        if (handler_ == nullptr) {
            FONT_LOGE("SubscribeCommonEvent: handler is null");
            return;
        }
        auto task = [this, userId]() { CleanupUserScopeFonts(userId); };
        handler_->PostTask(task);
    };
    subscriber_ = std::make_shared<FontManagerEventSubscriber>(subscribeInfo, std::move(callback));
    if (!EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber_)) {
        FONT_LOGE("SubscribeCommonEvent failed");
        subscriber_ = nullptr;
    } else {
        FONT_LOGI("SubscribeCommonEvent success");
    }
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

int32_t FontManagerServer::InstallFontWithUserId(const int32_t fd, int32_t userId)
{
    CallingCountGuard guard(this, false);
    int32_t ret = CheckPermission();
    if (ret == ERR_OK) {
        if (userId < 0) {
            FONT_LOGE("Invalid userId: %{public}d", userId);
            ret = ERR_INVALID_PARAM;
        } else {
            ret = FontManager::GetInstance()->InstallFont(fd, userId);
        }
    } else {
        FONT_LOGE("CheckPermission failed, ret: %{public}d", ret);
    }
    return ret;
}

int32_t FontManagerServer::UninstallFontWithUserId(const std::string &fontName, int32_t userId)
{
    CallingCountGuard guard(this, false);
    int32_t ret = CheckPermission();
    if (ret == ERR_OK) {
        if (userId < 0) {
            FONT_LOGE("Invalid userId: %{public}d", userId);
            ret = ERR_INVALID_PARAM;
        } else {
            ret = FontManager::GetInstance()->UninstallFont(fontName, userId);
        }
    } else {
        FONT_LOGE("CheckPermission failed, ret: %{public}d", ret);
    }
    return ret;
}

// ===== Scope font methods =====

int32_t FontManagerServer::CheckScopeFontPermission()
{
    uint32_t callerToken = IPCSkeleton::GetCallingTokenID();
    int result = Security::AccessToken::AccessTokenKit::VerifyAccessToken(callerToken,
        PERMISSION_UPDATE_SCOPE_FONT);
    if (result != Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
        FONT_LOGE("ScopeFont: caller doesn't have permission.");
        return ERR_NO_PERMISSION;
    }
    return ERR_OK;
}

std::string FontManagerServer::GetBundleNameByToken()
{
    uint32_t callerToken = IPCSkeleton::GetCallingTokenID();
    Security::AccessToken::HapTokenInfo tokenInfo;
    int result = Security::AccessToken::AccessTokenKit::GetHapTokenInfo(callerToken, tokenInfo);
    if (result != Security::AccessToken::RET_SUCCESS) {
        FONT_LOGE("GetHapTokenInfo failed");
        return "";
    }
    return tokenInfo.bundleName;
}

int32_t FontManagerServer::GetCallingUserId()
{
#ifdef ACCOUNT_ENABLE
    int32_t userId = INVALID_USERID;
    int32_t ret = AccountSA::OsAccountManager::GetOsAccountLocalIdFromUid(
        IPCSkeleton::GetCallingUid(), userId);
    if (ret != ERR_OK) {
        FONT_LOGE("GetOsAccountLocalIdFromUid failed");
        return INVALID_USERID;
    }
    return userId;
#else
    return INVALID_USERID;
#endif
}

std::string FontManagerServer::MakeAppIdentifier(int32_t scope, int32_t userId, int32_t tokenId)
{
    if (scope == FONT_SCOPE_APP) {
        return "app_" + std::to_string(tokenId);
    }
    return "session_" + std::to_string(tokenId);
}

int32_t FontManagerServer::OnFontObserver(const sptr<IFontClientObserver>& observer)
{
    CallingCountGuard guard(this, true);
    int32_t ret = ERR_OK;
    OnFontObserverInner(observer, ret);
    return ret;
}

void FontManagerServer::OnFontObserverInner(const sptr<IFontClientObserver>& observer, int32_t &ret)
{
    ret = CheckScopeFontPermission();
    if (ret != ERR_OK) return;
    int32_t tokenId = static_cast<int32_t>(IPCSkeleton::GetCallingTokenID());
    int32_t userId = GetCallingUserId();
    std::string bundleName = GetBundleNameByToken();
    ret = FontClientRegistry::GetInstance()->RegisterClient(
        observer->AsObject(), bundleName, userId, tokenId);
}

int32_t FontManagerServer::OffFontObserver(const sptr<IFontClientObserver>& observer)
{
    CallingCountGuard guard(this, true);
    int32_t ret = ERR_OK;
    OffFontObserverInner(ret);
    return ret;
}

void FontManagerServer::OffFontObserverInner(int32_t &ret)
{
    ret = CheckScopeFontPermission();
    if (ret != ERR_OK) return;
    int32_t tokenId = static_cast<int32_t>(IPCSkeleton::GetCallingTokenID());
    ret = FontClientRegistry::GetInstance()->UnregisterClient(tokenId);
}

int32_t FontManagerServer::InstallScopeFont(const int32_t fd, int32_t scope,
    const std::string &srcPath, int32_t &outValue)
{
    CallingCountGuard guard(this, true);
    InstallScopeFontInner(fd, scope, srcPath, outValue);
    return ERR_OK;
}

void FontManagerServer::InstallScopeFontInner(const int32_t fd, int32_t scope,
    const std::string &srcPath, int32_t &outValue)
{
    outValue = CheckScopeFontPermission();
    if (outValue != ERR_OK) return;
    if (scope != FONT_SCOPE_APP && scope != FONT_SCOPE_SESSION) {
        outValue = ERR_INVALID_PARAM;
        return;
    }
    int32_t tokenId = static_cast<int32_t>(IPCSkeleton::GetCallingTokenID());
    int32_t userId = GetCallingUserId();
    if (userId < 0) {
        outValue = ERR_INSTALL_FAIL;
        return;
    }
    if (scope == FONT_SCOPE_APP && !FontClientRegistry::GetInstance()->IsClientRegistered(tokenId)) {
        outValue = ERR_SCOPE_FONT_NOT_REGISTERED;
        return;
    }
    ScopeFontInstallInfo info;
    info.fd = fd;
    info.scope = scope;
    info.srcPath = srcPath;
    info.bundleName = GetBundleNameByToken();
    if (info.bundleName.empty()) {
        FONT_LOGE("InstallScopeFontInner: failed to get bundleName, tokenId=%{public}d", tokenId);
        outValue = ERR_SYSTEM_ERROR;
        return;
    }
    info.appIdentifier = MakeAppIdentifier(scope, userId, tokenId);
    info.userId = userId;
    outValue = FontManager::GetInstance()->InstallScopeFont(info);
}

int32_t FontManagerServer::UninstallScopeFont(const std::string &srcPath, int32_t &outValue)
{
    CallingCountGuard guard(this, true);
    UninstallScopeFontInner(srcPath, outValue);
    return ERR_OK;
}

void FontManagerServer::UninstallScopeFontInner(const std::string &srcPath, int32_t &outValue)
{
    outValue = CheckScopeFontPermission();
    if (outValue != ERR_OK) return;
    int32_t userId = GetCallingUserId();
    if (userId < 0) {
        outValue = ERR_UNINSTALL_FAIL;
        return;
    }
    std::string bundleName = GetBundleNameByToken();
    if (bundleName.empty()) {
        FONT_LOGE("UninstallScopeFontInner: failed to get bundleName");
        outValue = ERR_SYSTEM_ERROR;
        return;
    }
    outValue = FontManager::GetInstance()->UninstallScopeFont(srcPath, bundleName, userId);
}

int32_t FontManagerServer::GetFontScope(const std::string &srcPath, int32_t &outValue)
{
    CallingCountGuard guard(this, true);
    GetFontScopeInner(srcPath, outValue);
    return ERR_OK;
}

void FontManagerServer::GetFontScopeInner(const std::string &srcPath, int32_t &outValue)
{
    outValue = CheckScopeFontPermission();
    if (outValue != ERR_OK) return;
    int32_t userId = GetCallingUserId();
    if (userId < 0) {
        outValue = ERR_SYSTEM_ERROR;
        return;
    }
    outValue = FontManager::GetInstance()->GetFontScope(srcPath, userId);
}

int32_t FontManagerServer::ParseUserIdFromReason(const SystemAbilityOnDemandReason &reason)
{
    std::string value = reason.GetValue();
    if (value.empty()) {
        FONT_LOGE("ParseUserIdFromReason failed, empty value");
        return INVALID_USERID;
    }
    char *endPtr = nullptr;
    errno = 0;
    long userId = strtol(value.c_str(), &endPtr, 10);
    if (endPtr == value.c_str() || *endPtr != '\0' || errno == ERANGE ||
        userId < 0 || userId > INT32_MAX) {
        FONT_LOGE("ParseUserIdFromReason failed, value=%{public}s", value.c_str());
        return INVALID_USERID;
    }
    return static_cast<int32_t>(userId);
}

void FontManagerServer::CleanupAllScopeFontsOnBoot()
{
    auto userIds = FontManagerUtils::GetAllCreatedUserIds();
    for (int32_t uid : userIds) {
        FontManager::GetInstance()->CleanupScopeFontsByUser(uid);
    }
    FontManagerUtils::ClearAllTempFileDir();
    FontManagerUtils::CleanupAllScopeFontDirs();
    FONT_LOGI("CleanupAllScopeFontsOnBoot done");
}

void FontManagerServer::CleanupUserScopeFonts(int32_t userId)
{
    if (userId < 0) {
        FONT_LOGE("CleanupUserScopeFonts: invalid userId=%{public}d", userId);
        return;
    }
    FontManager::GetInstance()->CleanupScopeFontsByUser(userId);
    FONT_LOGI("CleanupUserScopeFonts done, userId=%{public}d", userId);
}

void FontManagerServer::CleanupAppScopeFontsOnStart()
{
    auto userIds = FontManagerUtils::GetAllCreatedUserIds();
    for (int32_t uid : userIds) {
        FontManager::GetInstance()->CleanupAppScopeFontsByUser(uid);
    }
    FontManagerUtils::CleanupAppScopeFontDirs();
    FONT_LOGI("CleanupAppScopeFontsOnStart done");
}
} // namespace FontManager
} // namespace Global
} // namespace OHOS