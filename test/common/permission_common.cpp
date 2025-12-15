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

#include "permission_common.h"

#include <unistd.h>
#include "access_token.h"
#include "accesstoken_kit.h"
#include "access_token_error.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

namespace OHOS {
namespace Global {
namespace FontManager {
constexpr int PERMISSION_NUM = 2;
constexpr int ROOT_UID = 0;
constexpr int FONT_MANAGER_UID = 1015;
uint64_t PermissionCommon::selfTokenId_ = 0;
void PermissionCommon::SetFontManagerPermission(const std::string &processName)
{
    if (selfTokenId_ == 0) {
        selfTokenId_ = GetSelfTokenID();
    }
    uint64_t tokenId;
    const char* perms[PERMISSION_NUM] = {
        "ohos.permission.MANAGE_LOCAL_ACCOUNTS",
        "ohos.permission.UPDATE_FONT"
    };
    std::string process = processName;
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = PERMISSION_NUM,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = process.c_str(),
        .aplStr = "system_basic",
    };

    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    seteuid(ROOT_UID);
    Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
    seteuid(FONT_MANAGER_UID);
}

void PermissionCommon::SetUid()
{
    seteuid(FONT_MANAGER_UID);
}

void PermissionCommon::ResetUid()
{
    seteuid(ROOT_UID);
}

void PermissionCommon::ResetTokenAndUid()
{
    seteuid(ROOT_UID);
    if (selfTokenId_ != 0) {
        SetSelfTokenID(selfTokenId_);
        selfTokenId_ = 0;
    }
}

bool PermissionCommon::IsOriginalUTEnv()
{
    return (geteuid() == ROOT_UID);
}

void PermissionCommon::SetFontManagerInitEnv()
{
    SetFontManagerPermission("font_manager_server");
    seteuid(FONT_MANAGER_UID);
}

void PermissionCommon::GrantPermission(const std::vector<std::string> &perms)
{
    if (selfTokenId_ == 0) {
        selfTokenId_ = GetSelfTokenID();
    }

    std::vector<const char*> permPtrs;
    for (const auto& perm : perms) {
        permPtrs.push_back(perm.c_str());
    }

    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = static_cast<int>(permPtrs.size()),
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = permPtrs.data(),
        .acls = nullptr,
        .processName = "font_manager_ut_test",
        .aplStr = "system_basic",
    };

    uint64_t tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

void PermissionCommon::RemovePermission()
{
    if (selfTokenId_ == 0) {
        selfTokenId_ = GetSelfTokenID();
    }

    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = 0,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = nullptr,
        .acls = nullptr,
        .processName = "font_manager_ut_empty",
        .aplStr = "system_basic",
    };

    uint64_t tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}
} // namespace FontManager
} // namespace Global
} // namespace OHOS
