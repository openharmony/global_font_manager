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

#include "font_manager_client.h"

#include <fcntl.h>

#include "font_define.h"
#include "font_hilog.h"
#include "font_service_load_manager.h"

namespace OHOS {
namespace Global {
namespace FontManager {
FontManagerClient::FontManagerClient() {}
FontManagerClient::~FontManagerClient() {}

int32_t FontManagerClient::InstallFont(const std::string &fontPath, int &outValue)
{
    std::string realPath;
    if (!PathToRealPath(fontPath, realPath)) {
        FONT_LOGE("failed to get real path %{private}s, errno %{public}d", fontPath.c_str(), errno);
        outValue = ERR_FILE_NOT_EXISTS;
        return ERR_OK;
    }
    FILE* fp = fopen(realPath.c_str(), "rb");
    if (!fp) {
        FONT_LOGE("open font file failed");
        outValue = ERR_FILE_NOT_EXISTS;
        return ERR_OK;
    }
    int fd = fileno(fp);
    if (fd < 0) {
        FONT_LOGE("open font file failed, errno: %{public}d", errno);
        outValue = ERR_FILE_NOT_EXISTS;
        (void)fclose(fp);
        return ERR_OK;
    }
    sptr<IFontService> service = FontServiceLoadManager::GetInstance()->GetFontServiceAbility(FONT_SA_ID);
    if (service == nullptr) {
        FONT_LOGE("Service is null");
        outValue = ERR_INSTALL_FAIL;
        return ERR_OK;
    }
    int32_t ret = service->InstallFont(fd, outValue);
    (void)fclose(fp);
    return ret;
}

int32_t FontManagerClient::UninstallFont(const std::string &fontName, int &outValue)
{
    sptr<IFontService> service = FontServiceLoadManager::GetInstance()->GetFontServiceAbility(FONT_SA_ID);
    if (service == nullptr) {
        FONT_LOGE("Service is null");
        outValue = ERR_UNINSTALL_FAIL;
        return ERR_OK;
    }
    return service->UninstallFont(fontName, outValue);
}

int32_t FontManagerClient::DataMigration(std::shared_ptr<IDataMigrationListener> listener)
{
    sptr<IFontService> service = FontServiceLoadManager::GetInstance()->GetFontServiceAbility(FONT_SA_ID);
    if (service == nullptr) {
        FONT_LOGE("Service is null");
        return ERR_SYSTEM_ERROR;
    }
    sptr<DataMigrationCbAgent> cbAgent = new (std::nothrow) DataMigrationCbAgent(std::move(listener));
    if (cbAgent == nullptr) {
        FONT_LOGE("cbAgent is null");
        return ERR_SYSTEM_ERROR;
    }
    sptr<IRemoteObject> remote = cbAgent->AsObject();
    if (remote == nullptr) {
        FONT_LOGE("DataMigration listener is not object");
        return ERR_SYSTEM_ERROR;
    }
    return service->DataMigration(cbAgent);
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