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

#ifndef GLOBAL_FONT_MANAGER_HISYSEVENT_ADAPTER_H
#define GLOBAL_FONT_MANAGER_HISYSEVENT_ADAPTER_H

#include <cstdint>
#include <string>

#include "singleton.h"

namespace OHOS {
namespace Global {
namespace FontManager {
class HisyseventAdapter : public DelayedSingleton<HisyseventAdapter> {
    DECLARE_DELAYED_SINGLETON(HisyseventAdapter);

public:
    int CollectUserDataSize();
    uint64_t GetDataPartitionRemainSize();
    std::vector<std::string> GetFileOrFolderPath();
    std::vector<uint64_t> GetFileOrFolderSize();
};
} // namespace FontManager
} // namespace Global
} // namespace OHOS

#endif // GLOBAL_FONT_MANAGER_HISYSEVENT_ADAPTER_H