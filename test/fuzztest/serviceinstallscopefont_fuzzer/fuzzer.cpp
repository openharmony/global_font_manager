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
#include "fuzzer.h"

#include "fuzz_data.h"
#include "font_define.h"
#include "font_hilog.h"
#include "font_service_load_manager.h"
#include "permission_common.h"

namespace OHOS {
namespace Global {
namespace FontManager {
void ServiceInstallScopeFontFuzz(const int32_t fd, const int32_t scope, const std::string &srcPath)
{
    std::string processName = "ServiceInstallScopeFontFuzz";
    PermissionCommon::SetFontManagerPermission(processName);
    auto service = FontServiceLoadManager::GetInstance()->GetFontServiceAbility(FONT_SA_ID);
    if (service == nullptr) {
        FONT_LOGE("Service is null");
        return;
    }
    int32_t result;
    (void)service->InstallScopeFont(fd, scope, srcPath, result);
    PermissionCommon::ResetTokenAndUid();
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* input, size_t size)
{
    size_t needed = sizeof(int32_t) + sizeof(int32_t);
    if (size < needed) {
        return 0;
    }
    size_t offset = 0;
    int32_t fd = NewInt32(input + offset, size - offset);
    offset += sizeof(int32_t);
    int32_t scope = NewInt32(input + offset, size - offset);
    offset += sizeof(int32_t);
    std::string srcPath = NewString(input + offset, size - offset);

    ServiceInstallScopeFontFuzz(fd, scope, srcPath);
    return 0;
}
} // namespace FontManager
} // namespace Global
} // namespace OHOS
