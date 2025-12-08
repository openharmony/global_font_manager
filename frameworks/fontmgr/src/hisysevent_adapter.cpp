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

#ifndef GLOBAL_FONT_MANAGER_HISYSEVENT_ADAPTER_CPP
#define GLOBAL_FONT_MANAGER_HISYSEVENT_ADAPTER_CPP

#include "hisysevent_adapter.h"

#include <string>
#include <sys/statfs.h>

#include "directory_ex.h"
#include "font_hilog.h"
#include "hisysevent.h"
#include "font_define.h"

namespace OHOS {
namespace Global {
namespace FontManager {
using HiSysEventNameSpace = OHOS::HiviewDFX::HiSysEvent;

static const std::string DATA_PARTITION_NAME = "/data";
static const std::string COMPONENT_NAME = "font_manager";
static constexpr char FONT_MANAGER[] = "FONT_MANAGER";
HisyseventAdapter::HisyseventAdapter() {}
HisyseventAdapter::~HisyseventAdapter() {}

int HisyseventAdapter::CollectUserDataSize(const std::string &path)
{
    std::string componentName = COMPONENT_NAME;
    std::string partitionName = DATA_PARTITION_NAME;
    std::uint64_t remainPartitionSize = this->GetDataPartitionRemainSize();
    std::vector<std::string> fileOrFolderPath = this->GetFileOrFolderPath(path);
    std::vector<std::uint64_t> fileOrFolderSize = this->GetFileOrFolderSize(path);
    int ret = HiSysEventWrite(HiSysEventNameSpace::Domain::FILEMANAGEMENT, "USER_DATA_SIZE",
        HiSysEventNameSpace::EventType::STATISTIC, "COMPONENT_NAME", componentName, "PARTITION_NAME", partitionName,
        "REMAIN_PARTITION_SIZE", remainPartitionSize, "FILE_OR_FOLDER_PATH", fileOrFolderPath, "FILE_OR_FOLDER_SIZE",
        fileOrFolderSize);
    return ret;
}

int HisyseventAdapter::CollectDataMigrationState(const std::vector<int32_t> &userIds, int32_t result)
{
    int32_t count = 0;
    int64_t size = 0;
    int32_t userCount = 0;
    if (!userIds.empty()) {
        std::vector<std::string> sucPaths;
        OHOS::GetDirFiles(INSTALL_PATH_PREFIX + std::to_string(userIds.back()) + INSTALL_PATH_SUFFIX, sucPaths);
        count = sucPaths.size();
        size = OHOS::GetFolderSize(INSTALL_PATH_PREFIX + std::to_string(userIds.back()) + INSTALL_PATH_SUFFIX);
        userCount = userIds.size();
    }
    std::vector<std::string> errPaths;
    OHOS::GetDirFiles(INSTALL_PATH_APP, errPaths);
    int32_t errCount = errPaths.size();
    int64_t errSize = OHOS::GetFolderSize(INSTALL_PATH_APP);
    int64_t freeRom = static_cast<int64_t>(GetDataPartitionRemainSize());
    return HiSysEventWrite(FONT_MANAGER, "FONT_DATA_MIGRATION",
        HiSysEventNameSpace::EventType::STATISTIC, "COUNT", count, "SIZE", size,
        "COUNT_ERR", errCount, "SIZE_ERR", errSize, "USER_COUNT",
        userCount, "FREE_ROM", freeRom, "STATE", result);
}

std::uint64_t HisyseventAdapter::GetDataPartitionRemainSize()
{
    std::string partitionName = DATA_PARTITION_NAME;
    struct statfs stat;
    if (statfs(partitionName.c_str(), &stat) != 0) {
        return std::numeric_limits<uint64_t>::max();
    }
    const std::uint64_t blockSize = stat.f_bsize;
    const std::uint64_t freeBlocks = stat.f_bavail;
    const std::uint64_t freeSize = freeBlocks * blockSize;
    constexpr std::uint64_t mb = 1024ull * 1024ull;
    return freeSize / mb;
}

std::vector<std::string> HisyseventAdapter::GetFileOrFolderPath(const std::string &path)
{
    std::vector<std::string> vec;
    vec.push_back(path);
    return vec;
}

std::vector<std::uint64_t> HisyseventAdapter::GetFileOrFolderSize(const std::string &path)
{
    std::vector<std::uint64_t> vec;
    vec.push_back(OHOS::GetFolderSize(path));
    return vec;
}
} // namespace FontManager
} // namespace Global
} // namespace OHOS

#endif // GLOBAL_FONT_MANAGER_HISYSEVENT_ADAPTER_CPP