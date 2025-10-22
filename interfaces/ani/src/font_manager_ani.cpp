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

static const char* CLASS_NAME_BUSINESSERROR = "@ohos.base.BusinessError";

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

void FontManagerAni::ThrowAniError(ani_env *env, ani_int code, const std::string &message)
{
    ani_class cls {};
    if (ANI_OK != env->FindClass(CLASS_NAME_BUSINESSERROR, &cls)) {
        FONT_LOGE("find class %{public}s failed", CLASS_NAME_BUSINESSERROR);
        return;
    }
    ani_method ctor {};
    if (ANI_OK != env->Class_FindMethod(cls, "<ctor>", ":", &ctor)) {
        FONT_LOGE("find method BusinessError constructor failed");
        return;
    }
    ani_object error {};
    if (ANI_OK != env->Object_New(cls, ctor, &error)) {
        FONT_LOGE("new object %{public}s failed", CLASS_NAME_BUSINESSERROR);
        return;
    }
    if (ANI_OK != env->Object_SetPropertyByName_Int(error, "code_", code)) {
        FONT_LOGE("set property BusinessError.code failed");
        return;
    }
    ani_string messageRef {};
    if (ANI_OK != env->String_NewUTF8(message.c_str(), message.size(), &messageRef)) {
        FONT_LOGE("new message string failed");
        return;
    }
    if (ANI_OK != env->Object_SetPropertyByName_Ref(error, "message", static_cast<ani_ref>(messageRef))) {
        FONT_LOGE("set property BusinessError.message failed");
        return;
    }
    if (ANI_OK != env->ThrowError(static_cast<ani_error>(error))) {
        FONT_LOGE("throwError ani_error object failed");
    }
}

void FontManagerAni::ThrowError(ani_env *env, int errorCode)
{
    std::string msg = "Other error.";
    auto it = errorMsg.find(errorCode);
    if (it != errorMsg.end()) {
        msg = it->second;
    }
    ThrowAniError(env, errorCode, msg);
}

ani_status FontManagerAni::Init(ani_env* env)
{
    static const char* nameSpaceName = "@ohos.fontManager.fontManager";
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