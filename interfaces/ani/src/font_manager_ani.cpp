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

#include "font_manager_ani.h"

#include "ani.h"
#include "font_hilog.h"
#include "font_define.h"
#include "font_manager_client.h"
#include <unordered_map>

namespace OHOS {
namespace Global {
namespace FontManager {

static const std::unordered_map<int, std::string> errorMsg = {
    {ERR_NO_PERMISSION, "Permission denied."},
    {ERR_NOT_SYSTEM_APP, "Non-system application."},
    {ERR_FILE_NOT_EXISTS, "Font does not exist."},
    {ERR_FILE_VERIFY_FAIL, "Font is not supported."},
    {ERR_COPY_FAIL, "Font file copy failed."},
    {ERR_INSTALLED_ALRADY, "Font file installed."},
    {ERR_MAX_FILE_COUNT, "Exceeded maximum number of installed files."},
    {ERR_UNINSTALL_FILE_NOT_EXISTS, "Font file does not exist."},
    {ERR_UNINSTALL_REMOVE_FAIL, "Font file delete error."},
};

ani_int FontManagerAni::InstallFont(ani_env* env, ani_string ani_path)
{
    std::string path = ANIStringToStdString(env, ani_path);
    int errorCode = 0;
    int ret = FontManagerClient::InstallFont(path, errorCode);
    if (ret) {
        ThrowError(env, ret);
        return ret;
    }
    if (errorCode) {
        ThrowError(env, errorCode);
    }
    return errorCode;
}

ani_int FontManagerAni::UninstallFont(ani_env* env, ani_string ani_fullName)
{
    std::string fullName = ANIStringToStdString(env, ani_fullName);
    int errorCode = 0;
    int ret = FontManagerClient::UninstallFont(fullName, errorCode);
    if (ret) {
        ThrowError(env, ret);
        return ret;
    }
    if (errorCode) {
        ThrowError(env, errorCode);
    }
    return errorCode;
}

std::string FontManagerAni::ANIStringToStdString(ani_env *env, ani_string ani_str)
{
    ani_size size = 0;
    if (ANI_OK != env->String_GetUTF8Size(ani_str, &size)) {
        FONT_LOGE("GetUTF8Size size failed.");
        return "";
    }
    char buf[size + 1];
    ani_size written  = 0;
    if (ANI_OK != env->String_GetUTF8(ani_str, buf, size + 1, &written)) {
        FONT_LOGE("GetUTF8 string failed.");
        return "";
    }
    buf[written] = '\0';
    return buf;
}

ani_object FontManagerAni::CreateError(ani_env *env, const std::string &msg)
{
    ani_class cls {};
    ani_method method {};
    ani_object obj = nullptr;
    ani_status status = ANI_ERROR;
    if (env == nullptr) {
        FONT_LOGE("null env");
        return nullptr;
    }

    ani_string aniMsg = nullptr;
    if ((status = env->String_NewUTF8(msg.c_str(), msg.size(), &aniMsg)) != ANI_OK) {
        FONT_LOGE("String_NewUTF8 failed %{public}d", status);
        return nullptr;
    }

    ani_ref undefRef;
    if ((status = env->GetUndefined(&undefRef)) != ANI_OK) {
        FONT_LOGE("GetUndefined failed %{public}d", status);
        return nullptr;
    }

    if ((status = env->FindClass("Lescompat/Error;", &cls)) != ANI_OK) {
        FONT_LOGE("FindClass failed %{public}d", status);
        return nullptr;
    }
    if ((status = env->Class_FindMethod(cls, "<ctor>", "Lstd/core/String;Lescompat/ErrorOptions;:V", &method)) !=
        ANI_OK) {
        FONT_LOGE("Class_FindMethod failed %{public}d", status);
        return nullptr;
    }

    if ((status = env->Object_New(cls, method, &obj, aniMsg, undefRef)) != ANI_OK) {
        FONT_LOGE("Object_New failed %{public}d", status);
        return nullptr;
    }
    return obj;
}

ani_object FontManagerAni::CreateBusinessError(ani_env *env, int code, const std::string &msg)
{
    ani_class cls {};
    ani_method method {};
    ani_object obj = nullptr;
    ani_status status = ANI_ERROR;
    if (env == nullptr) {
        FONT_LOGE("null env");
        return nullptr;
    }
    if ((status = env->FindClass("L@ohos/base/BusinessError;", &cls)) != ANI_OK) {
        FONT_LOGE("FindClass failed %{public}d", status);
        return nullptr;
    }
    if ((status = env->Class_FindMethod(cls, "<ctor>", "DLescompat/Error;:V", &method)) != ANI_OK) {
        FONT_LOGE("Class_FindMethod failed %{public}d", status);
        return nullptr;
    }
    ani_object error = CreateError(env, msg);
    if (error == nullptr) {
        FONT_LOGE("error nulll");
        return nullptr;
    }
    ani_double dCode(code);
    if ((status = env->Object_New(cls, method, &obj, dCode, error)) != ANI_OK) {
        FONT_LOGE("Object_New failed %{public}d", status);
        return nullptr;
    }
    return obj;
}

void FontManagerAni::ThrowError(ani_env *env, int errorCode)
{
    std::string msg = "Other error.";
    auto it = errorMsg.find(errorCode);
    if (it != errorMsg.end()) {
        msg = it->second;
    }

    ani_object error = CreateBusinessError(env, errorCode, msg);
    env->ThrowError(static_cast<ani_error>(error));
}

ani_status FontManagerAni::Init(ani_env* env)
{
    static const char* nameSpaceName = "L@ohos/fontManager/fontManager;";
    ani_namespace ns;
    if (ANI_OK != env->FindNamespace(nameSpaceName, &ns)) {
        FONT_LOGE("Find namespace '%{public}s' failed", nameSpaceName);
        return (ani_status)ANI_ERROR;
    }

    std::array nsMethods = {
        ani_native_function { "nativeInstallFont", nullptr, reinterpret_cast<void*>(InstallFont) },
        ani_native_function { "nativeUninstallFont", nullptr, reinterpret_cast<void*>(UninstallFont) },
    };

    if (ANI_OK != env->Namespace_BindNativeFunctions(ns, nsMethods.data(), nsMethods.size())) {
        FONT_LOGE("Cannot bind native methods to '%{public}s'", nameSpaceName);
        return (ani_status)ANI_ERROR;
    };
    return ANI_OK;
}
}
}
}

ANI_EXPORT ani_status ANI_Constructor(ani_vm* vm, uint32_t* result)
{
    ani_env* env;
    if (ANI_OK != vm->GetEnv(ANI_VERSION_1, &env)) {
        FONT_LOGE("Unsupported ANI_VERSION_1");
        return (ani_status)ANI_ERROR;
    }

    auto status = OHOS::Global::FontManager::FontManagerAni::Init(env);
    if (status != ANI_OK) {
        return status;
    }
    *result = ANI_VERSION_1;
    return ANI_OK;
}