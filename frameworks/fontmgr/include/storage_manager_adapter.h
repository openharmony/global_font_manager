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

#ifndef GLOBAL_FONT_MANAGER_STORAGE_MANAGER_ADAPTER_H
#define GLOBAL_FONT_MANAGER_STORAGE_MANAGER_ADAPTER_H

#include <cstdint>
#include <string>

#include "singleton.h"
#include "storage_manager_proxy.h"
#include "iservice_registry.h"

namespace OHOS {
namespace Global {
namespace FontManager {
class StorageManagerAdapter : public DelayedSingleton<StorageManagerAdapter> {
    DECLARE_DELAYED_SINGLETON(StorageManagerAdapter);

public:
    int32_t ReportFontBundleStats(int32_t userId, const std::string &installPath);

private:
    std::string GetFontInstallerBusinessName();
    uint64_t GetFontFolderSize(const std::string &installPath);
    sptr<StorageManager::IStorageManager> GetStorageManagerProxy();
};
} // namespace FontManager
} // namespace Global
} // namespace OHOS
#endif // GLOBAL_FONT_MANAGER_STORAGE_MANAGER_ADAPTER_H