/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
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

#ifndef GLOBAL_FONT_MANAGER_FONT_ERROR_H
#define GLOBAL_FONT_MANAGER_FONT_ERROR_H
namespace OHOS {
namespace Global {
namespace FontManager {
enum FontErrorCode {
    // Common error code
    SUCCESS = 0,
    ERR_NO_PERMISSION = 201,
    ERR_NOT_SYSTEM_APP = 202,

    ERR_FILE_NOT_EXISTS = 31100101,
    ERR_FILE_VERIFY_FAIL = 31100102,
    ERR_COPY_FAIL = 31100103,
    ERR_INSTALLED_ALRADY = 31100104,
    ERR_MAX_FILE_COUNT = 31100105,
    ERR_INSTALL_FAIL = 31100106,
    ERR_UNINSTALL_FILE_NOT_EXISTS = 31100107,
    ERR_UNINSTALL_REMOVE_FAIL = 31100108,
    ERR_UNINSTALL_FAIL = 31100109,
    // inner error code
    ERR_CALL_IPC = 10,
    ERR_LOAD_SA = 11,
};

const int FONT_SA_ID = 66262;
} // namespace FontManager
} // namespace Global
} // namespace OHOS
#endif // GLOBAL_FONT_MANAGER_FONT_ERROR_H
