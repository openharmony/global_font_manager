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

#ifndef FONT_MANAGER_JS_FUNC_REF_HOLDER_H
#define FONT_MANAGER_JS_FUNC_REF_HOLDER_H

#include "nocopyable.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace Global {
namespace FontManager {
class JsFuncRefHolder : public NoCopyable {
public:
    JsFuncRefHolder(napi_env env, napi_value value);
    ~JsFuncRefHolder() override;
    bool IsValid() const;
    napi_ref Get() const;
private:
    napi_env env_ {nullptr};
    napi_ref ref_ {nullptr};
};
} // namespace FontManager
} // namespace Global
} // namespace OHOS
#endif // FONT_MANAGER_JS_FUNC_REF_HOLDER_H
