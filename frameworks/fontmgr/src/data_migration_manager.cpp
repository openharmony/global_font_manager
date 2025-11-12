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
#include "font_manager_utils.h"
#include "directory_ex.h"

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

void DataMigrationManager::DataMigration(const sptr<IDataMigrationCallback>& callback)
{
    callback_ = callback;
    FontManagerUtils::DeleteDir(INSTALL_PATH_APP + TEMP_FILE, true);
    isDataMigrationing_ = true;
    int32_t ret = DataMigrationInner();
    isDataMigrationing_ = false;
    EventDataResultCallback(ret);
    callback_ = nullptr;
}

int32_t DataMigrationManager::DataMigrationInner()
{
    if (OHOS::IsEmptyFolder(INSTALL_PATH_APP)) {
        FONT_LOGE("FontManager INSTALL_PATH_APP is empty.");
        return ERR_NOT_NEED_DATA_MIGRATION;
    }
    std::vector<int32_t> userIds = FontManagerUtils::GetAllCreatedUserIds();
    if (userIds.empty()) {
        FONT_LOGE("FontManager userIds empty.");
        return ERR_DATA_MIGRATION_SYSTEM_ERROR;
    }
    if (!InitAllUserDir(userIds) || !InitDataMigrationTempDir()) {
        FONT_LOGE("FontManager DataMigrationInner InitDir err.");
        return ERR_DATA_MIGRATION_SYSTEM_ERROR;
    }
    StartHeartBeatTask();
    std::vector<std::string> paths;
    OHOS::GetDirFiles(INSTALL_PATH_APP, paths);
    for (size_t i = 0; i < paths.size(); ++i) {
        if (ShouldCallback(i, paths.size())) {
            EventDataProgressCallback(i, paths.size(), userIds.size());
        }
        int ret = StartOneFileCopyTask(paths[i], userIds);
        if (ret != ERR_OK) {
            return ERR_DATA_MIGRATION_SYSTEM_ERROR;
        }
        FONT_LOGI("FontManager::FileCopyTask suc.FileName:%{public}s.", FontManagerUtils::GetFileName(paths[i]).c_str());
    }
    FontManagerUtils::DeleteDir(INSTALL_PATH_PREFIX + TEMP_FILE, true);
    return ERR_DATA_MIGRATION_FINISH;
}

int32_t DataMigrationManager::StartOneFileCopyTask(const std::string& path, const std::vector<int32_t>& userIds)
{
    for (const auto& userId : userIds) {
        if (!CopyFileForDataMigration(path, userId)) {
            FONT_LOGE("StartOneFileCopyTask copy file %{public}s error", FontManagerUtils::GetFileName(path).c_str());
            return ERR_SYSTEM_ERROR;
        }
    }
    if (!FontManagerUtils::RemoveFile(path)) {
        FONT_LOGE("StartOneFileCopyTask RemoveFile file (%{public}s) error", FontManagerUtils::GetFileName(path).c_str());
        return ERR_SYSTEM_ERROR;
    }
    return ERR_OK;
}

bool DataMigrationManager::CopyFileForDataMigration(const std::string &srcPath, const int32_t userId)
{
    std::string fileName = FontManagerUtils::GetFileName(srcPath);
    std::string tempPath = INSTALL_PATH_PREFIX + TEMP_FILE + fileName;
    std::string desPath = INSTALL_PATH_PREFIX + std::to_string(userId) + "/" + fileName;
    if (FontManagerUtils::CheckPathExist(desPath)) {
        FONT_LOGI("CopyFileForDataMigration path is exist(%{public}s) ", fileName.c_str());
        return true;
    }
    int fd = open(srcPath.c_str(), O_RDONLY);
    if (fd < 0) {
        FONT_LOGE("CopyFileForDataMigration pen font file failed, errno: %{public}d", errno);
        return false;
    }

    if (!FontManagerUtils::CopyFile(fd, tempPath)) {
        FONT_LOGE("CopyFileForDataMigration copy file %{public}s error", fileName.c_str());
        close(fd);
        return false;
    }
    if (!FontManagerUtils::RenameFile(tempPath, desPath)) {
        FONT_LOGE("CopyFileForDataMigration rename file %{public}s error", fileName.c_str());
        FontManagerUtils::RemoveFile(tempPath);
        close(fd);
        return false;
    }
    close(fd);
    return true;
}

void DataMigrationManager::StartHeartBeatTask()
{
    std::thread([this]() {
        FONT_LOGI("DataMigrationManager HeartBeat thread started.");
        while (isDataMigrationing_) {
            FONT_LOGI("DataMigrationManager HeartBeat....");
            EventDataHeartBeatCallback();
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

void DataMigrationManager::EventDataProgressCallback(int32_t i, int32_t size, int32_t idsize)
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
    EventData eventData = {.event = EventType::PROGRESS_DOING,
                           .timeRemaining = timeRemaining,
                           .progressPercentage = progressPercentage,
                           .progressResult = 0};
    RefreshEventData(eventData);
}

void DataMigrationManager::EventDataResultCallback(int32_t result)
{
    EventData eventData = {.event = EventType::PROGRESS_RESULT,
                           .timeRemaining = 0,
                           .progressPercentage = 0,
                           .progressResult = result};
    RefreshEventData(eventData);
}

void DataMigrationManager::EventDataHeartBeatCallback()
{
    EventData eventData = {.event = EventType::HEART_BEAT,
                           .timeRemaining = 0,
                           .progressPercentage = 0,
                           .progressResult = 0};
    RefreshEventData(eventData);
}

bool DataMigrationManager::InitAllUserDir(const std::vector<int32_t> userIds)
{
    for (const auto& userId : userIds) {
        std::string path = INSTALL_PATH_PREFIX + std::to_string(userId) + "/";
        if (!FontManagerUtils::CreateDirWithPermission(path)) {
            FONT_LOGE("InitAllUserDir CreateDirWithPermission err. userid = %{public}d.", userId);
            return false;
        }
    }
    return true;
}

bool DataMigrationManager::InitDataMigrationTempDir()
{
    std::string tempPath = INSTALL_PATH_PREFIX + TEMP_FILE;
    if (!FontManagerUtils::CheckPathExist(tempPath)) {
        if (!FontManagerUtils::CreateDirWithPermission(tempPath)) {
            return false;
        }
    }
    return true;
}

void DataMigrationManager::RefreshEventData(const EventData& eventData)
{
    sptr<IRemoteObject> object = callback_->AsObject();
    if (object != nullptr) {
        callback_->Handle(std::move(eventData));
    }
}
} // namespace FontManager
} // namespace Global
} // namespace OHOS
