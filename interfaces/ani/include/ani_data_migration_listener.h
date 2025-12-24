/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef GLOBAL_FONT_MANAGER_ANI_DATA_MIGRATION_LISTENER_H
#define GLOBAL_FONT_MANAGER_ANI_DATA_MIGRATION_LISTENER_H

#include <ani.h>
#include <memory>
#include "idata_migration_listener.h"

namespace OHOS {
namespace Global {
namespace FontManager {
class AniDataMigrationListener : public IDataMigrationListener {
public:
    AniDataMigrationListener(ani_env* env, ani_object listener);
    static std::shared_ptr<AniDataMigrationListener> Create(ani_env* env, ani_object listener);
    ~AniDataMigrationListener() override;

    void OnHandle(const EventData& eventData) override;

private:
    bool Init(ani_env* env, ani_object listener);
    void DoHeartbeatCallback(ani_env* env, ani_class cls);
    void DoProgressCallback(ani_env* env, ani_class cls, const EventData& eventData);
    void DoResultCallback(ani_env* env, ani_class cls, const EventData& eventData);

    ani_vm* vm_ = nullptr;
    ani_ref listenerRef_ = nullptr;
};
} // namespace FontManager
} // namespace Global
} // namespace OHOS
#endif // GLOBAL_FONT_MANAGER_ANI_DATA_MIGRATION_LISTENER_H
