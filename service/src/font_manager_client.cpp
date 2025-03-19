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

#include "font_manager_client.h"

#include <fcntl.h>

#include "font_define.h"
#include "font_hilog.h"
#include "font_service_load_manager.h"

namespace OHOS {
namespace Global {
namespace FontManager {
#define RETURN_FAIL_WHEN_SERVICE_NULL(service) \
    if ((service) == nullptr) {                \
        FONT_LOGE("Service is null");          \
        return ERR_CALL_IPC;                   \
    }

int32_t FontManagerClient::InstallFont(const std::string &fontPath, int &outValue)
{
    sptr<IFontService> service = FontServiceLoadManager::GetInstance()->GetFontServiceAbility(FONT_SA_ID);
    RETURN_FAIL_WHEN_SERVICE_NULL(service);
    std::string realPath;
    if (!PathToRealPath(fontPath, realPath)) {
        FONT_LOGE("failed to get real path %{private}s, errno %{public}d", fontPath.c_str(), errno);
        return ERR_INVALID_PARAM;
    }
    int fd = open(realPath.c_str(), O_RDONLY);
    if (fd < 0) {
        FONT_LOGE("open font file failed, errno: %{public}d", errno);
        return ERR_INVALID_PARAM;
    }

    int32_t ret = service->InstallFont(fd, outValue);
    if (fd >= 0) {
        close(fd);
    }
    return ret;
}

int32_t FontManagerClient::UninstallFont(const std::string &fontName, int &outValue)
{
    sptr<IFontService> service = FontServiceLoadManager::GetInstance()->GetFontServiceAbility(FONT_SA_ID);
    RETURN_FAIL_WHEN_SERVICE_NULL(service);
    return service->UninstallFont(fontName, outValue);
}

bool FontManagerClient::PathToRealPath(const std::string& path, std::string& realPath)
{
    if (path.empty()) {
        FONT_LOGE("path is empty!");
        return false;
    }

    if ((path.length() >= PATH_MAX)) {
        FONT_LOGE("path len is error, the len is: [%{public}zu]", path.length());
        return false;
    }

    char tmpPath[PATH_MAX] = {0};
    if (realpath(path.c_str(), tmpPath) == nullptr) {
        FONT_LOGE("path (%{public}s) to realpath error: %{public}s", path.c_str(), strerror(errno));
        return false;
    }

    realPath = tmpPath;
    if (access(realPath.c_str(), F_OK) != 0) {
        FONT_LOGE("check realpath (%{private}s) error: %{public}s", realPath.c_str(), strerror(errno));
        return false;
    }
    return true;
}
} // namespace FontManager
} // namespace Global
} // namespace OHOS