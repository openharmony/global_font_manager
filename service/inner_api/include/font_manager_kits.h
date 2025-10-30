/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef GLOBAL_FONT_MANAGER_FONT_MANAGER_KITS_H
#define GLOBAL_FONT_MANAGER_FONT_MANAGER_KITS_H

#include <string>
#include "singleton.h"
#include "data_migration_callback.h"

namespace OHOS {
namespace Global {
namespace FontManager {
class FontManagerKits {
public:
    DISALLOW_COPY_AND_MOVE(FontManagerKits);
    virtual ~FontManagerKits() = default;
    static FontManagerKits& GetInstance();

    virtual int32_t InstallFont(const std::string &fontPath, int &outValue) = 0;
    virtual int32_t UninstallFont(const std::string &fontName, int &outValue) = 0;
    virtual int32_t DataMigration(std::unique_ptr<DataMigrationCallback> callback) = 0;

protected:
    FontManagerKits() = default;
};
} // namespace FontManager
} // namespace Global
} // namespace OHOS
#endif // GLOBAL_FONT_MANAGER_FONT_MANAGER_KITS_H
