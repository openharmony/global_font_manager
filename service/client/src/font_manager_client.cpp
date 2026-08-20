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

#include "font_manager_client.h"

#include <fcntl.h>

#include "font_define.h"
#include "font_hilog.h"
#include "font_service_load_manager.h"

namespace OHOS {
namespace Global {
namespace FontManager {

void FontServiceDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    FONT_LOGI("Font service died");
    if (observer_ != nullptr) {
        observer_->OnServiceDied();
    }
}

FontManagerClient::FontManagerClient() {}
FontManagerClient::~FontManagerClient() {}

int32_t FontManagerClient::InstallFont(const std::string &fontPath, int &outValue)
{
    std::string realPath;
    if (!PathToRealPath(fontPath, realPath)) {
        FONT_LOGE("failed to get real path %{private}s, errno %{public}d", fontPath.c_str(), errno);
        outValue = ERR_FILE_NOT_EXISTS;
        return ERR_OK;
    }
    FILE* fp = fopen(realPath.c_str(), "rb");
    if (!fp) {
        FONT_LOGE("open font file failed");
        outValue = ERR_FILE_NOT_EXISTS;
        return ERR_OK;
    }
    int fd = fileno(fp);
    if (fd < 0) {
        FONT_LOGE("open font file failed, errno: %{public}d", errno);
        outValue = ERR_FILE_NOT_EXISTS;
        (void)fclose(fp);
        return ERR_OK;
    }
    sptr<IFontService> service = FontServiceLoadManager::GetInstance()->GetFontServiceAbility(FONT_SA_ID);
    if (service == nullptr) {
        FONT_LOGE("Service is null");
        outValue = ERR_INSTALL_FAIL;
        (void)fclose(fp);
        return ERR_OK;
    }
    int32_t ret = service->InstallFont(fd, outValue);
    (void)fclose(fp);
    return ret;
}

int32_t FontManagerClient::UninstallFont(const std::string &fontName, int &outValue)
{
    sptr<IFontService> service = FontServiceLoadManager::GetInstance()->GetFontServiceAbility(FONT_SA_ID);
    if (service == nullptr) {
        FONT_LOGE("Service is null");
        outValue = ERR_UNINSTALL_FAIL;
        return ERR_OK;
    }
    return service->UninstallFont(fontName, outValue);
}

int32_t FontManagerClient::DataMigration(std::shared_ptr<IDataMigrationListener> listener)
{
    sptr<IFontService> service = FontServiceLoadManager::GetInstance()->GetFontServiceAbility(FONT_SA_ID);
    if (service == nullptr) {
        FONT_LOGE("Service is null");
        return ERR_SYSTEM_ERROR;
    }
    sptr<DataMigrationCbAgent> cbAgent = new (std::nothrow) DataMigrationCbAgent(std::move(listener));
    if (cbAgent == nullptr) {
        FONT_LOGE("cbAgent is null");
        return ERR_SYSTEM_ERROR;
    }
    sptr<IRemoteObject> remote = cbAgent->AsObject();
    if (remote == nullptr) {
        FONT_LOGE("DataMigration listener is not object");
        return ERR_SYSTEM_ERROR;
    }
    int32_t ret = service->DataMigration(cbAgent);
    return ret;
}

bool FontManagerClient::PathToRealPath(const std::string& path, std::string& realPath)
{
    if (path.empty()) {
        FONT_LOGE("path is empty!");
        return false;
    }

    if ((path.length() >= PATH_MAX)) {
        FONT_LOGE("path len is error, the len is: [%{public}zu]", path.length());
        return false;
    }

    char tmpPath[PATH_MAX] = {0};
    if (realpath(path.c_str(), tmpPath) == nullptr) {
        FONT_LOGE("path (%{public}s) to realpath error: %{public}s", path.c_str(), strerror(errno));
        return false;
    }

    realPath = tmpPath;
    if (access(realPath.c_str(), F_OK) != 0) {
        FONT_LOGE("check realpath (%{private}s) error: %{public}s", realPath.c_str(), strerror(errno));
        return false;
    }
    return true;
}

int32_t FontManagerClient::InstallFontWithUserId(const std::string &fontPath, int32_t userId)
{
    std::string realPath;
    if (!PathToRealPath(fontPath, realPath)) {
        FONT_LOGE("failed to get real path %{private}s, errno %{public}d", fontPath.c_str(), errno);
        return ERR_FILE_NOT_EXISTS;
    }
    FILE* fp = fopen(realPath.c_str(), "rb");
    if (!fp) {
        FONT_LOGE("open font file failed");
        return ERR_FILE_NOT_EXISTS;
    }
    int fd = fileno(fp);
    if (fd < 0) {
        FONT_LOGE("open font file failed, errno: %{public}d", errno);
        (void)fclose(fp);
        return ERR_FILE_NOT_EXISTS;
    }
    sptr<IFontService> service = FontServiceLoadManager::GetInstance()->GetFontServiceAbility(FONT_SA_ID);
    if (service == nullptr) {
        FONT_LOGE("Service is null");
        (void)fclose(fp);
        return ERR_INSTALL_FAIL;
    }
    int32_t ret = service->InstallFontWithUserId(fd, userId);
    (void)fclose(fp);
    return ret;
}

int32_t FontManagerClient::UninstallFontWithUserId(const std::string &fontName, int32_t userId)
{
    sptr<IFontService> service = FontServiceLoadManager::GetInstance()->GetFontServiceAbility(FONT_SA_ID);
    if (service == nullptr) {
        FONT_LOGE("Service is null");
        return ERR_UNINSTALL_FAIL;
    }
    int32_t ret = service->UninstallFontWithUserId(fontName, userId);
    return ret;
}

int32_t FontManagerClient::OnFontObserver(const sptr<IFontClientObserver>& observer)
{
    sptr<IFontService> service = FontServiceLoadManager::GetInstance()->GetFontServiceAbility(FONT_SA_ID);
    if (service == nullptr) {
        FONT_LOGE("OnFontObserver: Service is null");
        return ERR_SYSTEM_ERROR;
    }
    {
        std::lock_guard<std::mutex> lock(observerLock_);
        if (saBinder_ != nullptr && saDeathRecipient_ != nullptr) {
            saBinder_->RemoveDeathRecipient(saDeathRecipient_);
        }
        saBinder_ = service->AsObject();
        saDeathRecipient_ = sptr<FontServiceDeathRecipient>::MakeSptr(observer);
        if (saDeathRecipient_ != nullptr && saBinder_ != nullptr) {
            saBinder_->AddDeathRecipient(saDeathRecipient_);
        }
    }
    int32_t ret = service->OnFontObserver(observer);
    if (ret != ERR_OK && ret != ERR_SCOPE_FONT_REPEATED_REGISTER) {
        std::lock_guard<std::mutex> lock(observerLock_);
        if (saBinder_ != nullptr && saDeathRecipient_ != nullptr) {
            saBinder_->RemoveDeathRecipient(saDeathRecipient_);
        }
        saBinder_ = nullptr;
        saDeathRecipient_ = nullptr;
    }
    return ret;
}

int32_t FontManagerClient::OffFontObserver(const sptr<IFontClientObserver>& observer)
{
    sptr<IFontService> service = FontServiceLoadManager::GetInstance()->GetFontServiceAbility(FONT_SA_ID);
    if (service == nullptr) {
        FONT_LOGE("OffFontObserver: Service is null");
        return ERR_SYSTEM_ERROR;
    }
    int32_t ret = service->OffFontObserver(observer);
    {
        std::lock_guard<std::mutex> lock(observerLock_);
        if (saBinder_ != nullptr && saDeathRecipient_ != nullptr) {
            saBinder_->RemoveDeathRecipient(saDeathRecipient_);
        }
        saBinder_ = nullptr;
        saDeathRecipient_ = nullptr;
    }
    return ret;
}

int32_t FontManagerClient::InstallScopeFont(const std::string &fontPath, int32_t scope, int32_t &outValue)
{
    std::string realPath;
    if (!PathToRealPath(fontPath, realPath)) {
        FONT_LOGE("InstallScopeFont: failed to get real path");
        outValue = ERR_FILE_NOT_EXISTS;
        return ERR_OK;
    }
    FILE* fp = fopen(realPath.c_str(), "rb");
    if (!fp) {
        FONT_LOGE("InstallScopeFont: open file failed");
        outValue = ERR_FILE_NOT_EXISTS;
        return ERR_OK;
    }
    int fd = fileno(fp);
    if (fd < 0) {
        FONT_LOGE("InstallScopeFont: fileno failed");
        outValue = ERR_FILE_NOT_EXISTS;
        (void)fclose(fp);
        return ERR_OK;
    }
    sptr<IFontService> service = FontServiceLoadManager::GetInstance()->GetFontServiceAbility(FONT_SA_ID);
    if (service == nullptr) {
        FONT_LOGE("InstallScopeFont: Service is null");
        outValue = ERR_SYSTEM_ERROR;
        (void)fclose(fp);
        return ERR_OK;
    }
    int32_t ret = service->InstallScopeFont(fd, scope, realPath, outValue);
    (void)fclose(fp);
    return ret;
}

int32_t FontManagerClient::UninstallScopeFont(const std::string &srcPath, int32_t &outValue)
{
    sptr<IFontService> service = FontServiceLoadManager::GetInstance()->GetFontServiceAbility(FONT_SA_ID);
    if (service == nullptr) {
        FONT_LOGE("UninstallScopeFont: Service is null");
        outValue = ERR_SYSTEM_ERROR;
        return ERR_OK;
    }
    return service->UninstallScopeFont(srcPath, outValue);
}

int32_t FontManagerClient::GetFontScope(const std::string &srcPath, int32_t &outValue)
{
    sptr<IFontService> service = FontServiceLoadManager::GetInstance()->GetFontServiceAbility(FONT_SA_ID);
    if (service == nullptr) {
        FONT_LOGE("GetFontScope: Service is null");
        outValue = FONT_SCOPE_NONE;
        return ERR_SYSTEM_ERROR;
    }
    return service->GetFontScope(srcPath, outValue);
}
} // namespace FontManager
} // namespace Global
} // namespace OHOS