/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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

namespace {
void ServiceInstallFuzz(const int32_t fd)
{
    OHOS::sptr<OHOS::Global::FontManager::IFontService> service = OHOS::Global::FontManager::FontServiceLoadManager::
        GetInstance()->GetFontServiceAbility(OHOS::Global::FontManager::FONT_SA_ID);
    if ((service) == nullptr) {
        FONT_LOGE("Service is null");
        return;
    }
    int32_t result;
    (void)service->InstallFont(fd, result);
}
}  // namespace

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* input, size_t size)
{
    int32_t fd = NewInt32(input, size);

    ServiceInstallFuzz(fd);

    return 0;
}
