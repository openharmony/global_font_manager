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

#include "ani_data_migration_listener.h"
#include "font_hilog.h"
#include "font_define.h"

namespace OHOS {
namespace Global {
namespace FontManager {
AniDataMigrationListener::AniDataMigrationListener(ani_env* env, ani_object listener)
    : vm_(nullptr), listenerRef_(nullptr)
{
}

bool AniDataMigrationListener::Init(ani_env* env, ani_object listener)
{
    if (env == nullptr) {
        FONT_LOGE("AniDataMigrationListener: env is nullptr");
        return false;
    }
    if (env->GetVM(&vm_) != ANI_OK) {
        FONT_LOGE("AniDataMigrationListener: GetVM failed");
        return false;
    }
    if (env->GlobalReference_Create(listener, &listenerRef_) != ANI_OK) {
        FONT_LOGE("AniDataMigrationListener: GlobalReference_Create failed");
        return false;
    }
    return true;
}

std::shared_ptr<AniDataMigrationListener> AniDataMigrationListener::Create(ani_env* env, ani_object listener)
{
    auto instance = std::make_shared<AniDataMigrationListener>(env, listener);
    if (!instance->Init(env, listener)) {
        FONT_LOGE("AniDataMigrationListener: Create failed due to init error.");
        return nullptr;
    }
    return instance;
}

AniDataMigrationListener::~AniDataMigrationListener()
{
    if (vm_ != nullptr && listenerRef_ != nullptr) {
        ani_env* env = nullptr;
        bool isAttached = false;
        if (vm_->GetEnv(ANI_VERSION_1, &env) != ANI_OK) {
            if (vm_->AttachCurrentThread(nullptr, ANI_VERSION_1, &env) == ANI_OK) {
                FONT_LOGI("AniDataMigrationListener Destructor: AttachCurrentThread suc.");
                isAttached = true;
            } else {
                FONT_LOGE("AniDataMigrationListener Destructor: Failed to attach thread, global reference leaked.");
                return;
            }
        }
        env->GlobalReference_Delete(listenerRef_);
        if (isAttached) {
            vm_->DetachCurrentThread();
        }
    }
}

void AniDataMigrationListener::OnHandle(const EventData& eventData)
{
    if (vm_ == nullptr || listenerRef_ == nullptr) {
        FONT_LOGE("AniDataMigrationListener: OnHandle vm or listenerRef is nullptr");
        return;
    }

    ani_env* env = nullptr;
    bool isAttached = false;
    if (vm_->GetEnv(ANI_VERSION_1, &env) != ANI_OK) {
        if (vm_->AttachCurrentThread(nullptr, ANI_VERSION_1, &env) != ANI_OK) {
            FONT_LOGE("AniDataMigrationListener: AttachCurrentThread failed");
            return;
        }
        isAttached = true;
    }

    ani_class cls = nullptr;
    if (env->FindClass("@ohos.fontManager.fontManager.DataMigrationCallback", &cls) != ANI_OK) {
        FONT_LOGE("AniDataMigrationListener: GetClass failed");
        if (isAttached) {
            vm_->DetachCurrentThread();
        }
        return;
    }

    if (eventData.event == EventType::HEART_BEAT) {
        DoHeartbeatCallback(env, cls);
    } else if (eventData.event == EventType::PROGRESS_DOING) {
        DoProgressCallback(env, cls, eventData);
    } else if (eventData.event == EventType::PROGRESS_RESULT) {
        DoResultCallback(env, cls, eventData);
    }

    if (isAttached) {
        vm_->DetachCurrentThread();
    }
}

void AniDataMigrationListener::DoHeartbeatCallback(ani_env* env, ani_class cls)
{
    ani_method method = nullptr;
    if (env->Class_FindMethod(cls, "onHeartBeat", nullptr, &method) != ANI_OK) {
        FONT_LOGE("DoHeartbeatCallback: FindMethod onHeartBeat failed");
        return;
    }
    if (env->Object_CallMethod_Void(static_cast<ani_object>(listenerRef_), method) != ANI_OK) {
        FONT_LOGE("DoHeartbeatCallback: CallMethod onHeartBeat failed");
    }
}

void AniDataMigrationListener::DoProgressCallback(ani_env* env, ani_class cls, const EventData& eventData)
{
    ani_method method = nullptr;
    if (env->Class_FindMethod(cls, "onProgress", nullptr, &method) != ANI_OK) {
        FONT_LOGE("DoProgressCallback: FindMethod onProgress failed");
        return;
    }

    ani_class dataCls = nullptr;
    if (env->FindClass("@ohos.fontManager.fontManager.DataMigrationProgressImpl", &dataCls) != ANI_OK) {
        FONT_LOGE("Find ProgressData class failed");
        return;
    }
    
    ani_method ctor = nullptr;
    if (env->Class_FindMethod(dataCls, "<ctor>", nullptr, &ctor) != ANI_OK) {
        FONT_LOGE("Find ProgressData ctor method failed");
        return;
    }

    ani_object dataObj = nullptr;
    if (env->Object_New(dataCls, ctor, &dataObj, eventData.timeRemaining, eventData.progressPercentage) != ANI_OK) {
        FONT_LOGE("New ProgressData object failed");
        return;
    }

    if (env->Object_CallMethod_Void(static_cast<ani_object>(listenerRef_), method, dataObj) != ANI_OK) {
        FONT_LOGE("DoProgressCallback: CallMethod onProgress failed");
    }
}

void AniDataMigrationListener::DoResultCallback(ani_env* env, ani_class cls, const EventData& eventData)
{
    ani_method method = nullptr;
    if (env->Class_FindMethod(cls, "onResult", "I:V", &method) != ANI_OK) {
        FONT_LOGE("DoResultCallback: FindMethod onResult failed");
        return;
    }
    if (env->Object_CallMethod_Void(static_cast<ani_object>(listenerRef_), method,
        static_cast<ani_int>(eventData.progressResult)) != ANI_OK) {
        FONT_LOGE("DoResultCallback: CallMethod onResult failed");
    }
}

} // namespace FontManager
} // namespace Global
} // namespace OHOS
