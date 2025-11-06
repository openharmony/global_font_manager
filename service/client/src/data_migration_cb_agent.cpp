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

#include "font_define.h"
#include "data_migration_cb_agent.h"
#include "data_migration_callback.h"

namespace OHOS {
namespace Global {
namespace FontManager {
DataMigrationCbAgent::DataMigrationCbAgent(std::unique_ptr<DataMigrationCallback> callback)
    : DataMigrationCallbackStub(), callback_(std::move(callback))
{}

DataMigrationCbAgent::~DataMigrationCbAgent()
{}

ErrCode DataMigrationCbAgent::Handle(const EventData& eventData)
{
    if (callback_ != nullptr) {
        callback_->OnHandle(eventData);
        return ERR_OK;
    }
    return ERR_SYSTEM_ERROR;
}
} // namespace FontManager
} // namespace Global
} // namespace OHOS