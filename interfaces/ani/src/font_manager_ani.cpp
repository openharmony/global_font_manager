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

#include <unordered_map>
#include <array>
#include "ani.h"
#include "font_hilog.h"
#include "font_define.h"
#include "font_manager_kits.h"
#include "font_client_observer_agent.h"
#include "ani_data_migration_listener.h"

namespace OHOS {
namespace Global {
namespace FontManager {

static const char* CLASS_NAME_BUSINESSERROR = "@ohos.base.BusinessError";

static const std::unordered_map<int, std::string> errorMsg = {
    {ERR_NO_PERMISSION, "Permission denied."},
    {ERR_NOT_SYSTEM_APP, "Non-system application."},
    {ERR_INVALID_PARAM, "Parameter error."},
    {ERR_FILE_NOT_EXISTS, "Font does not exist."},
    {ERR_FILE_VERIFY_FAIL, "Font is not supported."},
    {ERR_COPY_FAIL, "Font file copy failed."},
    {ERR_INSTALLED_ALRADY, "Font file installed."},
    {ERR_MAX_FILE_COUNT, "Exceeded maximum number of installed files."},
    {ERR_UNINSTALL_FILE_NOT_EXISTS, "Font file does not exist."},
    {ERR_UNINSTALL_REMOVE_FAIL, "Font file delete error."},
    {ERR_UNINSTALL_FAIL, "Font uninstall error."},
    {ERR_SYSTEM_ERROR, "System error."},
    {ERR_DATA_MIGRATIONING, "Font data migration in progress."},
    {ERR_SCOPE_FONT_REPEATED_REGISTER, "Font observer already registered."},
    {ERR_SCOPE_FONT_EXCEED_REGISTER_LIMIT, "Exceeded maximum number of font observers."},
    {ERR_SCOPE_FONT_NOT_REGISTERED, "Font observer not registered."},
};

ani_int FontManagerAni::InstallFont(ani_env* env, ani_string ani_path)
{
    std::string path = ANIStringToStdString(env, ani_path);
    int errorCode = 0;
    int ret = FontManagerKits::GetInstance().InstallFont(path, errorCode);
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
    int ret = FontManagerKits::GetInstance().UninstallFont(fullName, errorCode);
    if (ret) {
        ThrowError(env, ret);
        return ret;
    }
    if (errorCode) {
        ThrowError(env, errorCode);
    }
    return errorCode;
}

ani_int FontManagerAni::DataMigration(ani_env* env, ani_object callback)
{
    if (callback == nullptr) {
        ThrowError(env, ERR_INVALID_PARAM);
        return ERR_INVALID_PARAM;
    }
    auto aniListener = AniDataMigrationListener::Create(env, callback);
    if (aniListener == nullptr) {
        FONT_LOGE("Failed to create AniDataMigrationListener.");
        ThrowError(env, ERR_SYSTEM_ERROR);
        return ERR_SYSTEM_ERROR;
    }
    int32_t ret = FontManagerKits::GetInstance().DataMigration(aniListener);
    if (ret != ERR_OK) {
        ThrowError(env, ret);
    }
    return ret;
}

class AniObserverRef {
public:
    AniObserverRef(ani_env* env, ani_object observer) : vm_(nullptr), observerRef_(nullptr)
    {
        if (env == nullptr || observer == nullptr) {
            return;
        }
        if (env->GetVM(&vm_) != ANI_OK) {
            FONT_LOGE("AniObserverRef: GetVM failed");
            return;
        }
        if (env->GlobalReference_Create(observer, &observerRef_) != ANI_OK) {
            FONT_LOGE("AniObserverRef: GlobalReference_Create failed");
        }
    }
    ~AniObserverRef()
    {
        if (vm_ == nullptr || observerRef_ == nullptr) {
            return;
        }
        ani_env* env = nullptr;
        bool isAttached = false;
        if (vm_->GetEnv(ANI_VERSION_1, &env) != ANI_OK) {
            if (vm_->AttachCurrentThread(nullptr, ANI_VERSION_1, &env) == ANI_OK) {
                isAttached = true;
            } else {
                FONT_LOGE("AniObserverRef: Failed to attach thread, global reference leaked.");
                return;
            }
        }
        if (env->GlobalReference_Delete(observerRef_) != ANI_OK) {
            FONT_LOGE("AniObserverRef: Failed to delete global reference.");
        }
        if (isAttached) {
            vm_->DetachCurrentThread();
        }
    }
    void CallOnServiceDied()
    {
        if (vm_ == nullptr || observerRef_ == nullptr) {
            return;
        }
        ani_env* env = nullptr;
        bool isAttached = false;
        if (vm_->GetEnv(ANI_VERSION_1, &env) != ANI_OK) {
            if (vm_->AttachCurrentThread(nullptr, ANI_VERSION_1, &env) != ANI_OK) {
                FONT_LOGE("AniObserverRef: AttachCurrentThread failed");
                return;
            }
            isAttached = true;
        }
        ani_status status = env->Object_CallMethodByName_Void(
            static_cast<ani_object>(observerRef_), "onServiceDied", ":");
        if (status != ANI_OK) {
            FONT_LOGE("AniObserverRef: Object_CallMethodByName_Void failed: %{public}d", status);
        }
        if (isAttached) {
            vm_->DetachCurrentThread();
        }
    }
    bool IsValid() const { return vm_ != nullptr && observerRef_ != nullptr; }

private:
    ani_vm* vm_;
    ani_ref observerRef_;
};

ani_int FontManagerAni::OnFontObserver(ani_env* env, ani_object observer)
{
    if (observer == nullptr) {
        ThrowError(env, ERR_INVALID_PARAM);
        return ERR_INVALID_PARAM;
    }
    auto aniRef = std::make_shared<AniObserverRef>(env, observer);
    if (!aniRef->IsValid()) {
        ThrowError(env, ERR_SYSTEM_ERROR);
        return ERR_SYSTEM_ERROR;
    }
    auto agent = sptr<FontClientObserverAgent>::MakeSptr([aniRef]() {
        aniRef->CallOnServiceDied();
    });
    if (agent == nullptr) {
        ThrowError(env, ERR_SYSTEM_ERROR);
        return ERR_SYSTEM_ERROR;
    }
    int32_t ret = FontManagerKits::GetInstance().OnFontObserver(agent);
    if (ret != ERR_OK) {
        ThrowError(env, ret);
    }
    return ret;
}

ani_int FontManagerAni::OffFontObserver(ani_env* env, ani_object observer)
{
    auto dummyAgent = sptr<FontClientObserverAgent>::MakeSptr([](){});
    if (dummyAgent == nullptr) {
        ThrowError(env, ERR_SYSTEM_ERROR);
        return ERR_SYSTEM_ERROR;
    }
    int32_t ret = FontManagerKits::GetInstance().OffFontObserver(dummyAgent);
    if (ret != ERR_OK) {
        ThrowError(env, ret);
    }
    return ret;
}

ani_int FontManagerAni::InstallScopeFont(ani_env* env, ani_string ani_url, ani_int ani_scope)
{
    std::string url = ANIStringToStdString(env, ani_url);
    int32_t scope = ani_scope;
    int32_t outValue = 0;
    int32_t ret = FontManagerKits::GetInstance().InstallScopeFont(url, scope, outValue);
    if (ret != ERR_OK) {
        ThrowError(env, ret);
        return ret;
    }
    if (outValue != ERR_OK) {
        ThrowError(env, outValue);
    }
    return outValue;
}

ani_int FontManagerAni::UninstallScopeFont(ani_env* env, ani_string ani_url)
{
    std::string url = ANIStringToStdString(env, ani_url);
    int32_t outValue = 0;
    int32_t ret = FontManagerKits::GetInstance().UninstallScopeFont(url, outValue);
    if (ret != ERR_OK) {
        ThrowError(env, ret);
        return ret;
    }
    if (outValue != ERR_OK) {
        ThrowError(env, outValue);
    }
    return outValue;
}

ani_int FontManagerAni::GetFontScope(ani_env* env, ani_string ani_url)
{
    std::string url = ANIStringToStdString(env, ani_url);
    int32_t outValue = 0;
    int32_t ret = FontManagerKits::GetInstance().GetFontScope(url, outValue);
    if (ret != ERR_OK) {
        ThrowError(env, ret);
        return ret;
    }
    return outValue;  // -1/0/1, ArkTS layer converts -1 to null
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
        ani_native_function { "dataMigration", nullptr, reinterpret_cast<void*>(DataMigration) },
        ani_native_function { "onFontObserver", nullptr, reinterpret_cast<void*>(OnFontObserver) },
        ani_native_function { "offFontObserver", nullptr, reinterpret_cast<void*>(OffFontObserver) },
        ani_native_function { "nativeInstallScopeFont", nullptr, reinterpret_cast<void*>(InstallScopeFont) },
        ani_native_function { "nativeUninstallScopeFont", nullptr, reinterpret_cast<void*>(UninstallScopeFont) },
        ani_native_function { "nativeGetFontScope", nullptr, reinterpret_cast<void*>(GetFontScope) },
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