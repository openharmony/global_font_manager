/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "storage_manager_adapter.h"

#include "directory_ex.h"
#include "font_define.h"
#include "font_hilog.h"
#include "parameters.h"

namespace OHOS {
namespace Global {
namespace FontManager {
StorageManagerAdapter::StorageManagerAdapter()
{
}

StorageManagerAdapter::~StorageManagerAdapter()
{
}

namespace {
constexpr int32_t STORAGE_MANAGER_MANAGER_ID = 5003;
}

int32_t StorageManagerAdapter::ReportFontBundleStats(int32_t userId, const std::string &installPath)
{
    std::string businessName = GetFontInstallerBusinessName();
    if (businessName.empty()) { // LCOV_EXCL_BR_LINE
        FONT_LOGE("ReportFontBundleStats businessName is empty");
        return ERR_INVALID_PARAM;
    }
    uint64_t size = GetFontFolderSize(installPath);
    auto proxy = GetStorageManagerProxy();
    if (proxy == nullptr) { // LCOV_EXCL_BR_LINE
        FONT_LOGE("ReportFontBundleStats storageManager proxy is null");
        return ERR_SYSTEM_ERROR;
    }
    StorageManager::ExtBundleStats stats;
    stats.businessName_ = businessName;
    stats.businessSize_ = size;
    int32_t ret = proxy->SetExtBundleStats(userId, stats);
    if (ret != ERR_OK) {
        FONT_LOGE("ReportFontBundleStats SetExtBundleStats failed, ret:%{public}d, userId:%{public}d",
            ret, userId);
    } else { // LCOV_EXCL_BR_LINE
        FONT_LOGI("ReportFontBundleStats success, userId:%{public}d, size:%{public}ju", userId, size);
    }
    return ret;
}

std::string StorageManagerAdapter::GetFontInstallerBusinessName()
{
    std::string businessName = OHOS::system::GetParameter(EXT_STORAGE_BUNDLE_PARAM_KEY, "");
    if (businessName.empty()) { // LCOV_EXCL_BR_LINE
        FONT_LOGE("GetFontInstallerBusinessName param %{public}s is empty", EXT_STORAGE_BUNDLE_PARAM_KEY.c_str());
    }
    return businessName;
}

uint64_t StorageManagerAdapter::GetFontFolderSize(const std::string &installPath)
{
    return OHOS::GetFolderSize(installPath);
}

sptr<StorageManager::IStorageManager> StorageManagerAdapter::GetStorageManagerProxy()
{
    sptr<ISystemAbilityManager> samgrProxy =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) { // LCOV_EXCL_BR_LINE
        FONT_LOGE("GetStorageManagerProxy samgrProxy is null");
        return nullptr;
    }
    auto remote = samgrProxy->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    if (remote == nullptr) { // LCOV_EXCL_BR_LINE
        FONT_LOGE("GetStorageManagerProxy remote is null");
        return nullptr;
    }
    auto storageMgrProxy = iface_cast<StorageManager::IStorageManager>(remote);
    if (storageMgrProxy == nullptr) { // LCOV_EXCL_BR_LINE
        FONT_LOGE("GetStorageManagerProxy iface_cast failed");
    }
    return storageMgrProxy;
}
} // namespace FontManager
} // namespace Global
} // namespace OHOS