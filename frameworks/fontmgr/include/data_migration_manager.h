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

#ifndef FONT_MANAGER_DATA_MIGRATION_UTILS_H
#define FONT_MANAGER_DATA_MIGRATION_UTILS_H

#include <atomic>
#include <string>

#include "singleton.h"
#include "idata_migration_callback.h"
#include "idata_migration_callback_event.h"

namespace OHOS {
namespace Global {
namespace FontManager {
class DataMigrationManager : public DelayedSingleton<DataMigrationManager> {
    DECLARE_DELAYED_SINGLETON(DataMigrationManager);
public:
    void DataMigration(const sptr<IDataMigrationCallback>& callback);

private:
    void StartHeartBeatTask();
    int32_t DataMigrationInner();
    int32_t StartOneFileCopyTask(const std::string& path, const std::vector<int32_t>& userIds);
    bool CopyFileForDataMigration(const std::string &srcPath, const int32_t userId);
    bool ShouldCallback(int32_t i, int32_t totalCount);
    void EventDataHeartBeatCallback();
    void EventDataProgressCallback(int32_t i, int32_t size, int32_t idsize);
    void EventDataResultCallback(int32_t result);
    bool InitAllUserDir(const std::vector<int32_t> userIds);
    bool InitDataMigrationTempDir();
    void RefreshEventData(const EventData& eventData);
    std::atomic<bool> isDataMigrationing_ {false};
    sptr<IDataMigrationCallback> callback_ = nullptr;
};
} // namespace FontManager
} // namespace Global
} // namespace OHOS
#endif // FONT_MANAGER_DATA_MIGRATION_UTILS_H
