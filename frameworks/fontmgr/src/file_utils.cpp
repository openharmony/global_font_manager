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

#include "file_utils.h"

#include <fcntl.h>
#include <fstream>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>

#include "font_hilog.h"

namespace OHOS {
namespace Global {
namespace FontManager {
static constexpr int32_t MAX_SIZE = 1024 * 20;
static constexpr uint32_t TIME_STRING_LENGTH = 20;
static constexpr uint32_t BEGIN_YEAR = 1900;

bool FileUtils::CheckPathExist(const std::string &pathName)
{
    if (pathName.empty()) {
        FONT_LOGE("CheckPathExsit pathName is empty.");
        return false;
    }
    std::error_code errorCode;
    bool ret = std::filesystem::exists(pathName, errorCode);
    if (errorCode.operator bool()) {
        FONT_LOGE("CheckPathExsit %{public}s exists error, error code = %{public}d",
            pathName.c_str(), errorCode.value());
        return false;
    }
    return ret;
}

bool FileUtils::CreateFileWithPermission(const std::string &filePath, const std::string &defalutStr)
{
    if (filePath.empty() || strstr(filePath.c_str(), "/.") != NULL || strstr(filePath.c_str(), "./") != NULL) {
        FONT_LOGE("filePath %{public}s is invalid", filePath.c_str());
        return false;
    }
    std::error_code errorCode;
    std::filesystem::path path(filePath);
    std::fstream file(path, std::ios::out | std::ios::trunc);
    if (!file) {
        FONT_LOGE("Create File path %{public}s failed", filePath.c_str());
        return false;
    }
    if (!defalutStr.empty()) {
        file << defalutStr;
    }
    file.close();

    std::filesystem::permissions(
        filePath, std::filesystem::perms::others_write, std::filesystem::perm_options::remove, errorCode);
    if (errorCode.operator bool()) {
        FONT_LOGE("Assign permissions failed, path = %{public}s,error message : %{public}s",
            filePath.c_str(), errorCode.message().c_str());
        return false;
    }
    return true;
}

bool FileUtils::CreatDirWithPermission(const std::string &fileDir)
{
    if (fileDir.empty() || strstr(fileDir.c_str(), "/.") != NULL || strstr(fileDir.c_str(), "./") != NULL) {
        FONT_LOGE("dirName %{public}s is invalid", fileDir.c_str());
        return false;
    }
    std::error_code errorCode;
    std::filesystem::create_directory(fileDir, errorCode);
    if (errorCode.operator bool()) {
        FONT_LOGE("Create directory dir %{public}s fail, error message : %{public}s", fileDir.c_str(),
            errorCode.message().c_str());
        return false;
    }
    std::filesystem::permissions(fileDir, std::filesystem::perms::others_write,
        std::filesystem::perm_options::remove, errorCode);
    if (errorCode.operator bool()) {
        FONT_LOGE("Assign permissions failed, path = %{public}s,error message : %{public}s", fileDir.c_str(),
            errorCode.message().c_str());
        return false;
    }
    return true;
}

std::string FileUtils::GetFilePathByFd(const int32_t &fd)
{
    auto filePath = std::make_unique<char[]>(PATH_MAX);
    if (filePath == nullptr) {
        FONT_LOGE("make_unique failed , filePath is null !");
        return "";
    }
    std::string linkPath = std::string("/proc/") + std::to_string(getpid()) + "/fd/" + std::to_string(fd);
    ssize_t pathLen = readlink(linkPath.c_str(), filePath.get(), PATH_MAX - 1);
    if (pathLen < 0) {
        FONT_LOGE("fd readlink: %{private}s failed, errno: %{public}d", linkPath.c_str(), errno);
        return "";
    }
    filePath[pathLen] = '\0';
    return std::string(filePath.get());
}

std::string FileUtils::GetFileName(const std::string &path)
{
    std::string split = "/";
    size_t pos = path.find_last_of(split);
    if (pos == std::string::npos) {
        return path;
    }
    return path.substr(pos + 1);
}

bool FileUtils::CopyFile(int32_t sourceFd, const std::string& path)
{
    constexpr int32_t filePermission = 0644;
    int32_t targetFd = open(path.c_str(), O_WRONLY | O_CREAT | O_SYNC, filePermission);
    bool ret = CopyFileByFd(sourceFd, targetFd);
    if (targetFd >= 0) {
        close(targetFd);
    }
    return ret;
}

bool FileUtils::CopyFileByFd(int32_t sourceFd, int32_t targetFd)
{
    if (sourceFd < 0) {
        FONT_LOGE("Failed to open source file");
        return false;
    }
    struct stat sourceStat;
    if (fstat(sourceFd, &sourceStat) < 0) {
        FONT_LOGE("Failed to get source file stat");
        return false;
    }
    if (targetFd < 0) {
        FONT_LOGE("Failed to write to target file");
        return false;
    }
    lseek(sourceFd, 0, SEEK_SET);
    lseek(targetFd, 0, SEEK_SET);
    constexpr int32_t bufferSize = MAX_SIZE;
    char buffer[bufferSize];
    int32_t bytesRead = 0;
    int32_t bytesWritten = 0;
    while ((bytesRead = read(sourceFd, buffer, sizeof(buffer))) > 0) {
        bytesWritten = write(targetFd, buffer, bytesRead);
        if (bytesWritten != bytesRead) {
            FONT_LOGE("Failed to write to target file");
            return false;
        }
    }
    return true;
}

std::string FileUtils::GetFileTime()
{
    struct timeval time;
    gettimeofday(&time, nullptr);
    struct tm timeInfo;
    localtime_r(&time.tv_sec, &timeInfo);

    char timeBuf[TIME_STRING_LENGTH] = {0};
    int result = sprintf_s(timeBuf, TIME_STRING_LENGTH, "%04u%02d%02d-%02d%02d%02d",
        timeInfo.tm_year + BEGIN_YEAR,
        timeInfo.tm_mon + 1,
        timeInfo.tm_mday,
        timeInfo.tm_hour,
        timeInfo.tm_min,
        timeInfo.tm_sec);
    if (result < 0) {
        FONT_LOGE("GetFileTimeStr sprintf_s error");
        return "";
    }
    return timeBuf;
}

bool FileUtils::RenameFile(const std::string& src, const std::string& dest)
{
    if (!CheckPathExist(src)) {
        FONT_LOGI("file %{public}s is not exist", src.c_str());
        return false;
    }
    std::error_code errorCode;
    std::filesystem::rename(src, dest, errorCode);
    if (errorCode.operator bool()) {
        FONT_LOGE("filesystem::rename error, error code = %{public}d", errorCode.value());
        return false;
    }
    return true;
}

bool FileUtils::RemoveFile(const std::string &fileName)
{
    if (!CheckPathExist(fileName)) {
        FONT_LOGI("file %{public}s is not exist", fileName.c_str());
        return true;
    }
    return RemoveAll(fileName);
}

void FileUtils::DeleteDir(const std::string &rootPath, bool isDeleteRootDir)
{
    if (!CheckPathExist(rootPath)) {
        FONT_LOGI("dir %{public}s is not exist", rootPath.c_str());
        return;
    }
    auto myPath = std::filesystem::path(rootPath);
    if (isDeleteRootDir) {
        RemoveAll(myPath);
        return;
    }

    for (auto const &dirEntry : std::filesystem::directory_iterator{ myPath }) {
        RemoveAll(dirEntry.path());
    }
}

bool FileUtils::RemoveAll(const std::filesystem::path &path)
{
    std::error_code errorCode;
    std::filesystem::remove_all(path, errorCode);
    if (errorCode.operator bool()) {
        FONT_LOGI("remove dir %{public}s fail, error message : %{public}s", path.c_str(),
            errorCode.message().c_str());
        return false;
    }
    return true;
}
} // namespace FontManager
} // namespace Global
} // namespace OHOS

