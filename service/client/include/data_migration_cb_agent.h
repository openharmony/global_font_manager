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

#ifndef GLOBAL_FONT_MANAGER_FONT_MANAGER_CB_AGENT_H
#define GLOBAL_FONT_MANAGER_FONT_MANAGER_CB_AGENT_H

#include "data_migration_callback_stub.h"
#include "idata_migration_listener.h"

namespace OHOS {
namespace Global {
namespace FontManager {
class DataMigrationCbAgent : public DataMigrationCallbackStub {
public:
    explicit DataMigrationCbAgent(std::shared_ptr<IDataMigrationListener> listener);
    virtual ~DataMigrationCbAgent() override;
    ErrCode Handle(const EventData& eventData) override;

private:
    std::shared_ptr<IDataMigrationListener> listener_;
};
} // namespace FontManager
} // namespace Global
} // namespace OHOS
#endif // GLOBAL_FONT_MANAGER_FONT_MANAGER_CB_AGENT_H
