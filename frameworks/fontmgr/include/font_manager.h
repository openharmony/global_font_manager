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

#ifndef GLOBAL_FONT_MANAGER_FONT_MANAGER_H
#define GLOBAL_FONT_MANAGER_FONT_MANAGER_H

#include "singleton.h"
#include "idata_migration_callback.h"
#include "idata_migration_callback_event.h"

namespace OHOS {
namespace Global {
namespace FontManager {
using RemoteCallbackPtr = sptr<IDataMigrationCallback>;
class FontManager : public DelayedSingleton<FontManager> {
    DECLARE_DELAYED_SINGLETON(FontManager);
public:
    int32_t InstallFont(const int32_t &fd, const int32_t userId);
    int32_t UninstallFont(const std::string &fontFullName, const int32_t userId);
    void DataMigration(const RemoteCallbackPtr& callback);
    void EventDataHeartBeatCallback(const RemoteCallbackPtr& callback);

private:
    bool CheckFontConfigPath(const std::string &installPath);
    bool CheckAndInitInstallPath(const std::string &installPath);
    std::string Utf16BEToUtf8(const uint8_t* data, size_t byteLen);
    std::vector<std::string> GetFontFullName(const int32_t &fd);
    std::string GetFormatFullName(const std::vector<std::string> &fullNameVector);
    std::string CopyFileForInstall(const std::string &installPath, const std::string &fileName, const int32_t &fd);
    bool CopyFileForDataMigration(const std::string &srcPath, const int32_t userId);
    std::vector<int32_t> GetAllCreatedUserIds();
    int32_t DataMigrationInner(const RemoteCallbackPtr& callback);
    bool InitAllUserDir(const std::vector<int32_t> userIds);
    bool InitDataMigrationTempDir();
    std::string GetRealPath(const std::string &installPath, const std::string &path);
    int32_t StartOneFileCopyTask(const std::string& path, const std::vector<int32_t>& userIds);
    bool ShouldCallback(int32_t i, int32_t totalCount);
    void EventDataBeginCallback(const RemoteCallbackPtr& callback);
    void EventDataProgressCallback(int32_t i, int32_t size, int32_t idsize, const RemoteCallbackPtr& callback);
    void EventDataResultCallback(int32_t result, const RemoteCallbackPtr& callback);
    void RefreshEventData(const EventData& eventData, const RemoteCallbackPtr& callback);
};
} // namespace FontManager
} // namespace Global
} // namespace OHOS
#endif // GLOBAL_FONT_MANAGER_FONT_MANAGER_H
