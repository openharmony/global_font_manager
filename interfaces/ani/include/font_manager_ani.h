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
#ifndef GLOBAL_FONT_MANAGER_FONT_ANI_H
#define GLOBAL_FONT_MANAGER_FONT_ANI_H

#include <string>
#include <ani.h>

namespace OHOS {
namespace Global {
namespace FontManager {
class FontManagerAni {
public:
    static ani_int InstallFont(ani_env* env, ani_string path);
    static ani_int UninstallFont(ani_env* env, ani_string fullName);
    static ani_int DataMigration(ani_env* env, ani_object callback);
    static void OnFontObserver(ani_env* env, ani_object observer);
    static void OffFontObserver(ani_env* env, ani_object observer);
    static void InstallScopeFont(ani_env* env, ani_string url, ani_int scope);
    static void UninstallScopeFont(ani_env* env, ani_string url);
    static ani_int GetFontScope(ani_env* env, ani_string url);
    static ani_status Init(ani_env* env);

private:
    static std::string ANIStringToStdString(ani_env *env, ani_string ani_str);
    static void ThrowError(ani_env *env, int errorCode);
    static void ThrowAniError(ani_env *env, ani_int code, const std::string &message);
};
}
}
}
#endif