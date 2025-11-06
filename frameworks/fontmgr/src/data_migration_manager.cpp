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

#include "data_migration_manager.h"

#include <thread>
#include <fcntl.h>
#include "font_define.h"
#include "font_hilog.h"
#include "file_utils.h"
#include "directory_ex.h"
#ifdef ACCOUNT_ENABLE
#include "os_account_manager.h"
#endif
namespace OHOS {
namespace Global {
namespace FontManager {
namespace {
constexpr int32_t COPY_SPEED = 5 * 1024 / 60; // Mb/s
constexpr int32_t MAX_TRIGGER_COUNT = 100;
constexpr double EPSILON = 0.5;
static constexpr uint32_t HEARTBEAT_INTERVAL = 60;
}

DataMigrationManager::DataMigrationManager()
{
}

DataMigrationManager::~DataMigrationManager()
{
}

void DataMigrationManager::DataMigration(const RemoteCallbackPtr& callback)
{
    FileUtils::DeleteDir(INSTALL_PATH_APP + TEMP_FILE, true);
    isDataMigrationing_ = true;
    int32_t ret = DataMigrationInner(callback);
    isDataMigrationing_ = false;
    EventDataResultCallback(ret, callback);
}

int32_t DataMigrationManager::DataMigrationInner(const RemoteCallbackPtr& callback)
{
    if (OHOS::IsEmptyFolder(INSTALL_PATH_APP)) {
        FONT_LOGE("FontManager INSTALL_PATH_APP is empty.");
        return ERR_NOT_NEED_DATA_MIGRATION;
    }
    std::vector<int32_t> userIds = GetAllCreatedUserIds();
    if (userIds.empty()) {
        FONT_LOGE("FontManager userIds empty.");
        return ERR_DATA_MIGRATION_SYSTEM_ERROR;
    }
    if (!InitAllUserDir(userIds) || !InitDataMigrationTempDir()) {
        FONT_LOGE("FontManager DataMigrationInner InitDir err.");
        return ERR_DATA_MIGRATION_SYSTEM_ERROR;
    }
    std::vector<std::string> paths;
    OHOS::GetDirFiles(INSTALL_PATH_APP, paths);
    for (size_t i = 0; i < paths.size(); ++i) {
        StartHeartBeatTask(callback);
        if (ShouldCallback(i, paths.size())) {
            EventDataProgressCallback(i, paths.size(), userIds.size(), callback);
        }
        int ret = StartOneFileCopyTask(paths[i], userIds);
        if (ret != ERR_OK) {
            return ERR_DATA_MIGRATION_SYSTEM_ERROR;
        }
        FONT_LOGI("FontManager::FileCopyTask suc.FileName:%{public}s.", FileUtils::GetFileName(paths[i]).c_str());
    }
    FileUtils::DeleteDir(INSTALL_PATH_PREFIX + TEMP_FILE, true);
    return ERR_DATA_MIGRATION_FINISH;
}

int32_t DataMigrationManager::StartOneFileCopyTask(const std::string& path, const std::vector<int32_t>& userIds)
{
    for (const auto& userId : userIds) {
        if (!CopyFileForDataMigration(path, userId)) {
            FONT_LOGE("StartOneFileCopyTask copy file %{public}s error", FileUtils::GetFileName(path).c_str());
            return ERR_SYSTEM_ERROR;
        }
    }
    if (!FileUtils::RemoveFile(path)) {
        FONT_LOGE("StartOneFileCopyTask RemoveFile file (%{public}s) error", FileUtils::GetFileName(path).c_str());
        return ERR_SYSTEM_ERROR;
    }
    return ERR_OK;
}

bool DataMigrationManager::CopyFileForDataMigration(const std::string &srcPath, const int32_t userId)
{
    std::string fileName = FileUtils::GetFileName(srcPath);
    std::string tempPath = INSTALL_PATH_PREFIX + TEMP_FILE + fileName;
    std::string desPath = INSTALL_PATH_PREFIX + std::to_string(userId) + "/" + fileName;
    if (FileUtils::CheckPathExist(desPath)) {
        FONT_LOGI("CopyFileForDataMigration path is exist(%{public}s) ", fileName.c_str());
        return true;
    }
    int fd = open(srcPath.c_str(), O_RDONLY);
    if (fd < 0) {
        FONT_LOGE("CopyFileForDataMigration pen font file failed, errno: %{public}d", errno);
        return false;
    }

    if (!FileUtils::CopyFile(fd, tempPath)) {
        FONT_LOGE("CopyFileForDataMigration copy file %{public}s error", fileName.c_str());
        close(fd);
        return false;
    }
    if (!FileUtils::RenameFile(tempPath, desPath)) {
        FONT_LOGE("CopyFileForDataMigration rename file %{public}s error", fileName.c_str());
        FileUtils::RemoveFile(tempPath);
        close(fd);
        return false;
    }
    close(fd);
    return true;
}

void DataMigrationManager::StartHeartBeatTask(const sptr<IDataMigrationCallback>& callback)
{
    std::thread([this, callback]() {
        FONT_LOGI("DataMigrationManager HeartBeat thread started.");
        while (isDataMigrationing_) {
            FONT_LOGI("DataMigrationManager HeartBeat....");
            EventDataHeartBeatCallback(callback);
            std::this_thread::sleep_for(std::chrono::seconds(HEARTBEAT_INTERVAL));
        }
        FONT_LOGI("DataMigrationManager HeartBeat thread stoped.");
    }).detach();
}

bool DataMigrationManager::ShouldCallback(int32_t i, int32_t totalCount)
{
    if (totalCount <= MAX_TRIGGER_COUNT) {
        return true;
    }
    double step = static_cast<double>(totalCount) / static_cast<double>(MAX_TRIGGER_COUNT);
    int32_t expectedTriggerIndex = static_cast<int32_t>(std::round(i / step));
    double triggerPos = expectedTriggerIndex * step;
    return std::fabs(i - triggerPos) < EPSILON;
}

void DataMigrationManager::EventDataProgressCallback(int32_t i, int32_t size, int32_t idsize,
    const RemoteCallbackPtr& callback)
{
    // 以Mb/s计算预估时间
    std::uintmax_t remainSize = (OHOS::GetFolderSize(INSTALL_PATH_APP) * idsize) >> 20;
    int32_t timeRemaining = remainSize / COPY_SPEED;
    if (timeRemaining < 1) {
        timeRemaining = 1;
    }
    // 计算百分百，size / 2用于四舍五入
    int32_t progressPercentage =
        i == 0 ? i : static_cast<int32_t>((static_cast<int64_t>(i) * MAX_TRIGGER_COUNT + size / 2) / size);
    EventData eventData = {.event = ProgressType::PROGRESS_DOING,
                           .timeRemaining = timeRemaining,
                           .progressPercentage = progressPercentage,
                           .progressResult = 0};
    RefreshEventData(eventData, callback);
}

void DataMigrationManager::EventDataResultCallback(int32_t result, const RemoteCallbackPtr& callback)
{
    EventData eventData = {.event = ProgressType::PROGRESS_RESULT,
                           .timeRemaining = 0,
                           .progressPercentage = 0,
                           .progressResult = result};
    RefreshEventData(eventData, callback);
}

void DataMigrationManager::EventDataHeartBeatCallback(const RemoteCallbackPtr& callback)
{
    EventData eventData = {.event = ProgressType::HEART_BEAT,
                           .timeRemaining = 0,
                           .progressPercentage = 0,
                           .progressResult = 0};
    RefreshEventData(eventData, callback);
}

std::vector<int32_t> DataMigrationManager::GetAllCreatedUserIds()
{
    std::vector<int32_t> allUserIds;
#ifdef ACCOUNT_ENABLE
    std::vector<AccountSA::OsAccountInfo> osAccountInfos;
    ErrCode ret = AccountSA::OsAccountManager::QueryAllCreatedOsAccounts(osAccountInfos);
    if (ret != ERR_OK || osAccountInfos.empty()) {
        FONT_LOGE("FontManager::GetAllCreatedUserIds failed.err=%{public}d", ret);
    }
    for (const auto &info : osAccountInfos) {
        allUserIds.push_back(info.GetLocalId());
    }
    return allUserIds;
#else
    FONT_LOGI("FontManager osAccount not support.");
    return allUserIds;
#endif
}

bool DataMigrationManager::InitAllUserDir(const std::vector<int32_t> userIds)
{
    for (const auto& userId : userIds) {
        std::string path = INSTALL_PATH_PREFIX + std::to_string(userId) + "/";
        if (!FileUtils::CreateDirWithPermission(path)) {
            FONT_LOGE("InitAllUserDir CreateDirWithPermission err. userid = %{public}d.", userId);
            return false;
        }
    }
    return true;
}

bool DataMigrationManager::InitDataMigrationTempDir()
{
    std::string tempPath = INSTALL_PATH_PREFIX + TEMP_FILE;
    if (!FileUtils::CheckPathExist(tempPath)) {
        if (!FileUtils::CreateDirWithPermission(tempPath)) {
            return false;
        }
    }
    return true;
}

void DataMigrationManager::RefreshEventData(const EventData& eventData, const RemoteCallbackPtr& callback)
{
    sptr<IRemoteObject> object = callback->AsObject();
    if (object != nullptr) {
        callback->Handle(std::move(eventData));
    }
}
} // namespace FontManager
} // namespace Global
} // namespace OHOS
