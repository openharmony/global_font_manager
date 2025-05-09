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

namespace OHOS {
namespace Global {
namespace FontManager {
using HiSysEventNameSpace = OHOS::HiviewDFX::HiSysEvent;

static const std::string DATA_PARTITION_NAME = "/data";
static const std::string INSTALL_PATH = "/data/service/el1/public/for-all-app/fonts/";
static const std::string COMPONENT_NAME = "font_manager";

HisyseventAdapter::HisyseventAdapter() {}
HisyseventAdapter::~HisyseventAdapter() {}

int HisyseventAdapter::CollectUserDataSize()
{
    std::string componentName = COMPONENT_NAME;
    std::string partitionName = DATA_PARTITION_NAME;
    std::uint64_t remainPartitionSize = this->GetDataPartitionRemainSize();
    std::vector<std::string> fileOrFolderPath = this->GetFileOrFolderPath();
    std::vector<std::uint64_t> fileOrFolderSize = this->GetFileOrFolderSize();
    int ret = HiSysEventWrite(HiSysEventNameSpace::Domain::FILEMANAGEMENT, "USER_DATA_SIZE",
        HiSysEventNameSpace::EventType::STATISTIC, "COMPONENT_NAME", componentName, "PARTITION_NAME", partitionName,
        "REMAIN_PARTITION_SIZE", remainPartitionSize, "FILE_OR_FOLDER_PATH", fileOrFolderPath, "FILE_OR_FOLDER_SIZE",
        fileOrFolderSize);
    return ret;
}

std::uint64_t HisyseventAdapter::GetDataPartitionRemainSize()
{
    std::string partitionName = DATA_PARTITION_NAME;
    struct statfs stat;
    if (statfs(partitionName.c_str(), &stat) != 0) {
        return -1;
    }
    std::uint64_t blockSize = stat.f_bsize;
    std::uint64_t freeSize = stat.f_bfree * blockSize;
    constexpr double units = 1024.0;
    std::uint64_t freeSizeMb = freeSize / (units * units);
    return freeSizeMb;
}

std::vector<std::string> HisyseventAdapter::GetFileOrFolderPath()
{
    std::vector<std::string> vec;
    vec.push_back(INSTALL_PATH);
    return vec;
}

std::vector<std::uint64_t> HisyseventAdapter::GetFileOrFolderSize()
{
    std::vector<std::uint64_t> vec;
    vec.push_back(OHOS::GetFolderSize(INSTALL_PATH));
    return vec;
}
} // namespace FontManager
} // namespace Global
} // namespace OHOS

#endif // GLOBAL_FONT_MANAGER_HISYSEVENT_ADAPTER_CPP