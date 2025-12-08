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
#include "hisysevent_adapter.h"

namespace OHOS {
namespace Global {
namespace FontManager {
namespace {
constexpr int32_t COPY_SPEED = 10 * 1024 / 60; // Mb/s
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
    isDataMigrationing_ = true;
    int32_t ret = DataMigrationInner();
    if (ret != ERR_OK) {
        FONT_LOGE("FontManager DataMigration err.ErrCode:%{public}d", ret);
    }
    isDataMigrationing_ = false;
    EventDataResult(ret);
    ret = HisyseventAdapter::GetInstance()->CollectDataMigrationState(userIds_, ret);
    if (ret != ERR_OK) {
        FONT_LOGE("FontManager CollectDataMigrationState err.ErrCode:%{public}d", ret);
    }
    callback_ = nullptr;
}

int32_t DataMigrationManager::DataMigrationInner()
{
    int32_t ret = InitDataMigrationEnv();
    if (ret != ERR_OK) {
        return ret;
    }
    StartHeartBeatTask();
    return StartDataMigration();
}

int32_t DataMigrationManager::InitDataMigrationEnv()
{
    userIds_ = FontManagerUtils::GetAllCreatedUserIds();
    if (userIds_.empty()) {
        FONT_LOGE("FontManager get all userIds err.");
        return ERR_GET_ALL_USERIDS;
    }
    FontManagerUtils::DeleteDir(INSTALL_PATH_APP + TEMP_FILE, true);
    if (OHOS::IsEmptyFolder(INSTALL_PATH_APP)) {
        FONT_LOGE("FontManager INSTALL_PATH_APP is empty.");
        return ERR_NOT_NEED_DATA_MIGRATION;
    }
    if (!CheckAllUserDir()) {
        FONT_LOGE("FontManager CheckAllUserDir err.");
        return ERR_CHECK_INSTALL_DIR;
    }
    if (!InitDataMigrationTempDir()) {
        FONT_LOGE("FontManager InitDataMigrationTempDir  err.");
        return ERR_INIT_TEMP_DIR;
    }
    return ERR_OK;
}

int32_t DataMigrationManager::StartDataMigration()
{
    std::vector<std::string> paths;
    OHOS::GetDirFiles(INSTALL_PATH_APP, paths);
    for (size_t i = 0; i < paths.size(); ++i) {
        if (IsShouldUpdateProgress(i, paths.size())) {
            EventDataProgress(i, paths.size(), userIds_.size());
        }
        int ret = StartOneFileCopyTask(paths[i]);
        if (ret != ERR_OK) {
            return ret;
        }
        FONT_LOGI("FontManager::FileCopyTask suc.FileName:%{public}s", FontManagerUtils::GetFileName(paths[i]).c_str());
    }
    return ERR_OK;
}

int32_t DataMigrationManager::StartOneFileCopyTask(const std::string& path)
{
    for (const auto& userId : userIds_) {
        int32_t ret = CopyFileForDataMigration(path, userId);
        if (ret != ERR_OK) {
            FONT_LOGE("StartOneFileCopyTask copy file %{public}s error", FontManagerUtils::GetFileName(path).c_str());
            return ret;
        }
    }
    if (!FontManagerUtils::RemoveFile(path)) {
        FONT_LOGE("StartOneFileCopyTask RemoveFile file (%{public}s) err", FontManagerUtils::GetFileName(path).c_str());
        return ERR_REMOVE_SRC_FILE;
    }
    return ERR_OK;
}

int32_t DataMigrationManager::CopyFileForDataMigration(const std::string &srcPath, const int32_t userId)
{
    std::string fileName = FontManagerUtils::GetFileName(srcPath);
    std::string tempPath = INSTALL_PATH_PREFIX + std::to_string(userId) + INSTALL_PATH_SUFFIX + TEMP_FILE + fileName;
    std::string desPath = INSTALL_PATH_PREFIX + std::to_string(userId) + INSTALL_PATH_SUFFIX + fileName;
    if (FontManagerUtils::CheckPathExist(desPath)) {
        FONT_LOGI("CopyFileForDataMigration path is exist(%{public}s) ", fileName.c_str());
        return ERR_OK;
    }
    int fd = open(srcPath.c_str(), O_RDONLY);
    if (fd < 0) {
        FONT_LOGE("CopyFileForDataMigration pen font file failed, errno: %{public}d", errno);
        return ERR_OPEN_SRC_FILE;
    }

    if (!FontManagerUtils::CopyFile(fd, tempPath)) {
        FONT_LOGE("CopyFileForDataMigration copy file %{public}s error", fileName.c_str());
        close(fd);
        return ERR_COPY_FILE;
    }
    if (!FontManagerUtils::RenameFile(tempPath, desPath)) {
        FONT_LOGE("CopyFileForDataMigration rename file %{public}s error", fileName.c_str());
        FontManagerUtils::RemoveFile(tempPath);
        close(fd);
        return ERR_RENAME_FILE;
    }
    close(fd);
    return ERR_OK;
}

void DataMigrationManager::StartHeartBeatTask()
{
    std::thread([self = shared_from_this()]() {
        FONT_LOGI("DataMigrationManager HeartBeat thread started.");
        while (self->isDataMigrationing_) {
            FONT_LOGI("DataMigrationManager HeartBeat....");
            self->EventDataHeartBeat();
            std::this_thread::sleep_for(std::chrono::seconds(HEARTBEAT_INTERVAL));
        }
        FONT_LOGI("DataMigrationManager HeartBeat thread stoped.");
    }).detach();
}

bool DataMigrationManager::IsShouldUpdateProgress(int32_t i, int32_t totalCount)
{
    if (totalCount <= MAX_TRIGGER_COUNT) {
        return true;
    }
    double step = static_cast<double>(totalCount) / static_cast<double>(MAX_TRIGGER_COUNT);
    int32_t expectedTriggerIndex = static_cast<int32_t>(std::round(i / step));
    double triggerPos = expectedTriggerIndex * step;
    return std::fabs(i - triggerPos) < EPSILON;
}

void DataMigrationManager::EventDataProgress(int32_t i, int32_t size, int32_t idsize)
{
    std::uintmax_t remainSize = (OHOS::GetFolderSize(INSTALL_PATH_APP) * idsize) >> 20;
    int32_t timeRemaining = remainSize / COPY_SPEED;
    if (timeRemaining < 1) {
        timeRemaining = 1;
    }
    int32_t progressPercentage =
        i == 0 ? i : static_cast<int32_t>((static_cast<int64_t>(i) * MAX_TRIGGER_COUNT + size / 2) / size);
    EventData eventData = {.event = EventType::PROGRESS_DOING,
                           .timeRemaining = timeRemaining,
                           .progressPercentage = progressPercentage};
    RefreshEventData(eventData);
}

void DataMigrationManager::EventDataResult(int32_t result)
{
    EventData eventData = {.event = EventType::PROGRESS_RESULT,
                           .progressResult = result};
    RefreshEventData(eventData);
}

void DataMigrationManager::EventDataHeartBeat()
{
    EventData eventData = {.event = EventType::HEART_BEAT};
    RefreshEventData(eventData);
}

bool DataMigrationManager::CheckAllUserDir()
{
    for (const auto& userId : userIds_) {
        std::string path = INSTALL_PATH_PREFIX + std::to_string(userId) + INSTALL_PATH_SUFFIX;
        if (!FontManagerUtils::CheckPathExist(path)) {
            FONT_LOGE("CheckAllUserDir CheckUserDir err. userid = %{public}d.", userId);
            return false;
        }
    }
    return true;
}

bool DataMigrationManager::InitDataMigrationTempDir()
{
    for (const auto& userId : userIds_) {
        std::string tempPath = INSTALL_PATH_PREFIX + std::to_string(userId) + INSTALL_PATH_SUFFIX + TEMP_FILE;
        if (!FontManagerUtils::CheckPathExist(tempPath)) {
            if (!FontManagerUtils::CreateDirWithPermission(tempPath)) {
                return false;
            }
        }
    }
    return true;
}

void DataMigrationManager::RefreshEventData(const EventData& eventData)
{
    if (callback_ && callback_->AsObject() != nullptr) {
        callback_->Handle(std::move(eventData));
    }
}
} // namespace FontManager
} // namespace Global
} // namespace OHOS
