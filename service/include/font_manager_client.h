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

#ifndef GLOBAL_FONT_MANAGER_FONT_SERVICE_CLIENT_H
#define GLOBAL_FONT_MANAGER_FONT_SERVICE_CLIENT_H

#include <string>
#include <unistd.h>
#include <singleton.h>

#include "font_manager_kits.h"
#include "data_migration_cb_agent.h"
#include "font_service_load_manager.h"

namespace OHOS {
namespace Global {
namespace FontManager {
class FontManagerClient : public FontManagerKits, public DelayedSingleton<FontManagerClient> {
    DECLARE_DELAYED_REF_SINGLETON(FontManagerClient);
using CbAgentPtr = sptr<DataMigrationCbAgent>;
public:
    DISALLOW_COPY_AND_MOVE(FontManagerClient);
    int32_t InstallFont(const std::string &fontPath, int &outValue) override;
    int32_t UninstallFont(const std::string &fontName, int &outValue) override;
    int32_t DataMigration(std::unique_ptr<DataMigrationCallback> callback) override;

private:
    bool PathToRealPath(const std::string& path, std::string& realPath);
};
} // namespace FontManager
} // namespace Global
} // namespace OHOS
#endif // GLOBAL_FONT_MANAGER_FONT_SERVICE_CLIENT_H
