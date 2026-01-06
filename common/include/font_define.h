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

#ifndef GLOBAL_FONT_MANAGER_FONT_DEFINE_H
#define GLOBAL_FONT_MANAGER_FONT_DEFINE_H

#include <string>

namespace OHOS {
namespace Global {
namespace FontManager {
using ErrCode = int;
enum FontErrorCode {
    // Common error code
    ERR_OK = 0,
    ERR_NO_PERMISSION = 201,
    ERR_NOT_SYSTEM_APP = 202,
    ERR_INVALID_PARAM = 401,

    ERR_FILE_NOT_EXISTS = 31100101,
    ERR_FILE_VERIFY_FAIL = 31100102,
    ERR_COPY_FAIL = 31100103,
    ERR_INSTALLED_ALRADY = 31100104,
    ERR_MAX_FILE_COUNT = 31100105,
    ERR_INSTALL_FAIL = 31100106,
    ERR_UNINSTALL_FILE_NOT_EXISTS = 31100107,
    ERR_UNINSTALL_REMOVE_FAIL = 31100108,
    ERR_UNINSTALL_FAIL = 31100109,
    ERR_SYSTEM_ERROR = 31100110,
    ERR_DATA_MIGRATIONING = 31100111,
};

enum DataMigrationResultCode {
    ERR_NOT_NEED_DATA_MIGRATION = 1,
    ERR_GET_ALL_USERIDS,
    ERR_CHECK_INSTALL_DIR,
    ERR_INIT_TEMP_DIR,
    ERR_OPEN_SRC_FILE,
    ERR_COPY_FILE,
    ERR_RENAME_FILE,
    ERR_REMOVE_SRC_FILE,
};

inline const int FONT_SA_ID = 66262;
inline const std::string INSTALL_PATH_APP = "/data/service/el1/public/for-all-app/fonts/";
inline const std::string INSTALL_PATH_PREFIX = "/data/service/el1/";
inline const std::string INSTALL_PATH_SUFFIX = "/for-all-app/fonts/";
inline const std::string FONT_CONFIG_FILE = "install_fontconfig.json";
inline const std::string TEMP_FILE = "temp/";
} // namespace FontManager
} // namespace Global
} // namespace OHOS
#endif // GLOBAL_FONT_MANAGER_FONT_DEFINE_H
