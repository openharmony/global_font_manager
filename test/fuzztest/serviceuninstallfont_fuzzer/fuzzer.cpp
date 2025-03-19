/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include "fuzzer.h"

#include "fuzz_data.h"
#include "font_define.h"
#include "font_hilog.h"
#include "font_service_load_manager.h"

namespace {
void ServiceUnInstallFuzz(const std::string& fontName)
{
    OHOS::sptr<OHOS::Global::FontManager::IFontService> service = OHOS::Global::FontManager::FontServiceLoadManager::
        GetInstance()->GetFontServiceAbility(OHOS::Global::FontManager::FONT_SA_ID);
    if ((service) == nullptr) {
        FONT_LOGE("Service is null");
        return;
    }
    int32_t result;
    (void)service->UninstallFont(fontName, result);
}
}  // namespace

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* input, size_t size)
{
    std::string fontName = NewString(input, size);
    ServiceUnInstallFuzz(fontName);

    return 0;
}
