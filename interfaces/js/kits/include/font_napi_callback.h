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

#ifndef FONT_MANAGER_FONT_NAPI_CALLBACK_H
#define FONT_MANAGER_FONT_NAPI_CALLBACK_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "font_define.h"
#include "font_hilog.h"

namespace OHOS {
namespace Global {
namespace FontManager {
struct FontNapiCallback {
    napi_async_work work_;
    napi_deferred deferred_;
    std::string value_;
    std::string errMsg_;
    bool success_;
    int errCode_;
    int32_t scope_;       // input: FontScope for InstallScopeFont
    int32_t queryResult_; // output: scope for GetFontScope (-1/0/1)
    bool isQuery_;        // true if this is a GetFontScope query
    bool isVoidResult_;  // true for Promise<void> APIs (scope font install/uninstall)

    FontNapiCallback() : work_(nullptr), deferred_(nullptr), success_(true),
        errCode_(ERR_OK), scope_(0), queryResult_(-1), isQuery_(false), isVoidResult_(false) {}

    void SetErrorMsg(const std::string &msg, int32_t errCode)
    {
        errMsg_ = msg;
        success_ = false;
        errCode_ = errCode;
        FONT_LOGE("%{public}s, err = %{public}d", msg.c_str(), errCode_);
    }
};
} // namespace FontManager
} // namespace Global
} // namespace OHOS
#endif // FONT_MANAGER_FONT_NAPI_CALLBACK_H
