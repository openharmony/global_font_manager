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

#include "font_manager_utils.h"

#include <fcntl.h>
#include <fstream>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/sendfile.h>
#include <cerrno>

#include "font_hilog.h"
#include "font_define.h"
#include "securec.h"
#ifdef ACCOUNT_ENABLE
#include "os_account_manager.h"
#endif
#include "rosen_text/font_tool_set.h"

namespace OHOS {
namespace Global {
namespace FontManager {
static constexpr uint32_t TIME_STRING_LENGTH = 20;
static constexpr uint32_t BEGIN_YEAR = 1900;
static constexpr size_t MAX_FONT_FILE_SIZE = 1024 * 1024 * 1024; // 1024MB

bool FontManagerUtils::CheckAndInitInstallPath(const std::string &installPath)
{
    if (!CheckPathExist(installPath)) {
        if (!CreateDirWithPermission(installPath)) {
            return false;
        }
    }
    std::string installTempPath = installPath + TEMP_FILE;
    if (!CheckPathExist(installTempPath)) {
        if (!CreateDirWithPermission(installTempPath)) {
            return false;
        }
    }
    return CheckFontConfigPath(installPath);
}

bool FontManagerUtils::CheckAndInitScopeFontPath(const std::string &installPath)
{
    if (!CheckPathExist(installPath)) {
        if (!CreateDirWithPermission(installPath)) {
            return false;
        }
    }
    std::string installTempPath = installPath + TEMP_FILE;
    if (!CheckPathExist(installTempPath)) {
        if (!CreateDirWithPermission(installTempPath)) {
            return false;
        }
    }
    return true;
}

void FontManagerUtils::ClearAllTempFileDir()
{
    FONT_LOGI("FontManagerUtils::ClearAllTempFileDir begin.");
    auto userIds = GetAllCreatedUserIds();
    for (const auto &userId : userIds) {
        std::string installPath = INSTALL_PATH_PREFIX + std::to_string(userId) + INSTALL_PATH_SUFFIX;
        if (CheckPathExist(installPath + TEMP_FILE)) {
            DeleteDir(installPath + TEMP_FILE, true);
        }
    }
}

void FontManagerUtils::CleanupScopeFontDirs()
{
    FONT_LOGI("FontManagerUtils::CleanupScopeFontDirs begin.");
    auto userIds = GetAllCreatedUserIds();
    for (const auto &userId : userIds) {
        std::string installPath = INSTALL_PATH_PREFIX + std::to_string(userId) + INSTALL_PATH_SUFFIX;
        if (!CheckPathExist(installPath)) {
            continue;
        }
        std::vector<std::string> dirsToRemove;
        std::error_code ec;
        for (const auto &entry : std::filesystem::directory_iterator(installPath, ec)) {
            if (!entry.is_directory()) {
                continue;
            }
            std::string dirName = entry.path().filename().string();
            if (dirName.rfind("app_", 0) != 0 && dirName.rfind("session_", 0) != 0) {
                continue;
            }
            std::string dirPath = entry.path().string();
            std::string tempPath = dirPath + "/" + TEMP_FILE;
            if (CheckPathExist(tempPath)) {
                DeleteDir(tempPath, true);
            }
            if (std::filesystem::is_empty(entry.path(), ec)) {
                dirsToRemove.push_back(dirPath);
                FONT_LOGI("CleanupScopeFontDirs will remove empty dir: %{public}s", dirName.c_str());
            }
        }
        for (const auto &dirPath : dirsToRemove) {
            std::filesystem::remove(dirPath, ec);
        }
    }
}

void FontManagerUtils::CleanupAllScopeFontDirs()
{
    FONT_LOGI("FontManagerUtils::CleanupAllScopeFontDirs begin.");
    auto userIds = GetAllCreatedUserIds();
    for (const auto &userId : userIds) {
        std::string installPath = INSTALL_PATH_PREFIX + std::to_string(userId) + INSTALL_PATH_SUFFIX;
        if (!CheckPathExist(installPath)) {
            continue;
        }
        std::vector<std::string> dirsToRemove;
        std::error_code ec;
        for (const auto &entry : std::filesystem::directory_iterator(installPath, ec)) {
            if (!entry.is_directory()) {
                continue;
            }
            std::string dirName = entry.path().filename().string();
            if (dirName.rfind("app_", 0) != 0 && dirName.rfind("session_", 0) != 0) {
                continue;
            }
            dirsToRemove.push_back(entry.path().string());
            FONT_LOGI("CleanupAllScopeFontDirs will remove dir: %{public}s", dirName.c_str());
        }
        for (const auto &dirPath : dirsToRemove) {
            DeleteDir(dirPath, true);
        }
    }
}

void FontManagerUtils::CleanupAppScopeFontDirs()
{
    FONT_LOGI("FontManagerUtils::CleanupAppScopeFontDirs begin.");
    auto userIds = GetAllCreatedUserIds();
    for (const auto &userId : userIds) {
        std::string installPath = INSTALL_PATH_PREFIX + std::to_string(userId) + INSTALL_PATH_SUFFIX;
        if (!CheckPathExist(installPath)) {
            continue;
        }
        std::vector<std::string> dirsToRemove;
        std::error_code ec;
        for (const auto &entry : std::filesystem::directory_iterator(installPath, ec)) {
            if (!entry.is_directory()) {
                continue;
            }
            std::string dirName = entry.path().filename().string();
            if (dirName.rfind("app_", 0) != 0) {
                continue;
            }
            dirsToRemove.push_back(entry.path().string());
            FONT_LOGI("CleanupAppScopeFontDirs will remove dir: %{public}s", dirName.c_str());
        }
        for (const auto &dirPath : dirsToRemove) {
            DeleteDir(dirPath, true);
        }
    }
}

bool FontManagerUtils::CheckFontConfigPath(const std::string &installPath)
{
    if (FontManagerUtils::CheckPathExist(installPath + FONT_CONFIG_FILE)) {
        return true;
    }
    std::string font_list = R"({
        "fontlist": [],
        "version": "7.0"
    })";
    return CreateFileWithPermission(installPath + FONT_CONFIG_FILE, font_list);
}

std::vector<int32_t> FontManagerUtils::GetAllCreatedUserIds()
{
    std::vector<int32_t> allUserIds;
#ifdef ACCOUNT_ENABLE
    std::vector<AccountSA::OsAccountInfo> osAccountInfos;
    ErrCode ret = AccountSA::OsAccountManager::QueryAllCreatedOsAccounts(osAccountInfos);
    if (ret != ERR_OK || osAccountInfos.empty()) {
        FONT_LOGE("FontManagerUtils::GetAllCreatedUserIds failed.err=%{public}d", ret);
    }
    for (const auto &info : osAccountInfos) {
        allUserIds.push_back(info.GetLocalId());
    }
    return allUserIds;
#else
    FONT_LOGI("FontManagerUtils osAccount not support.");
    return allUserIds;
#endif
}

bool FontManagerUtils::CheckPathExist(const std::string &pathName)
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

bool FontManagerUtils::CreateFileWithPermission(const std::string &filePath, const std::string &defaultStr)
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
    if (!defaultStr.empty()) {
        file << defaultStr;
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

bool FontManagerUtils::CreateDirWithPermission(const std::string &fileDir)
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

std::string FontManagerUtils::GetFilePathByFd(const int32_t &fd)
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

std::string FontManagerUtils::GetFileName(const std::string &path)
{
    std::string split = "/";
    size_t pos = path.find_last_of(split);
    if (pos == std::string::npos) {
        return path;
    }
    return path.substr(pos + 1);
}

std::string FontManagerUtils::GetFileDirectory(const std::string &path)
{
    std::string split = "/";
    size_t pos = path.find_last_of(split);
    if (pos == std::string::npos) {
        return "";
    }
    return path.substr(0, pos + 1);
}

bool FontManagerUtils::CopyFile(int32_t sourceFd, const std::string& path)
{
    constexpr int32_t filePermission = 0644;
    int32_t targetFd = -1;
    bool ret = false;
    FILE* targetFile = fopen(path.c_str(), "wb");
    if (targetFile) {
        targetFd = fileno(targetFile);
        if (targetFd >= 0) {
            fchmod(targetFd, filePermission);
            ret = CopyFileByFd(sourceFd, targetFd);
        }
        (void)fclose(targetFile);
    } else {
        FONT_LOGE("FontManagerUtils CopyFile Failed to open source file");
    }
    return ret;
}

bool FontManagerUtils::CopyFileByFd(int32_t sourceFd, int32_t targetFd)
{
    if (sourceFd < 0) {
        FONT_LOGE("Failed to open source file fd");
        return false;
    }
    if (targetFd < 0) {
        FONT_LOGE("Failed to open target file fd");
        return false;
    }
    lseek(sourceFd, 0, SEEK_SET);
    lseek(targetFd, 0, SEEK_SET);
    struct stat sourceStat;
    if (fstat(sourceFd, &sourceStat) < 0) {
        FONT_LOGE("Failed to get source file stat");
        return false;
    }

    if (sourceStat.st_size < 0 || static_cast<size_t>(sourceStat.st_size) > MAX_FONT_FILE_SIZE) {
        FONT_LOGE("Source file size invalid or too large: %{public}lld", static_cast<long long>(sourceStat.st_size));
        return false;
    }

    off_t offset = 0;
    size_t bytesSent = 0;
    size_t fileSize = static_cast<size_t>(sourceStat.st_size);

    while (bytesSent < fileSize) {
        ssize_t ret = sendfile(targetFd, sourceFd, &offset, fileSize - bytesSent);
        if (ret < 0) {
            if (errno == EINTR) {
                continue;
            }
            FONT_LOGE("Failed to send file data: %s", strerror(errno));
            return false;
        }
        if (ret == 0) {
            FONT_LOGE("Unexpected EOF reading source file. Expected %zu, got %zu", fileSize, bytesSent);
            return false;
        }
        bytesSent += static_cast<size_t>(ret);
    }
    return true;
}

std::string FontManagerUtils::GetFileTime()
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

bool FontManagerUtils::RenameFile(const std::string& src, const std::string& dest)
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

bool FontManagerUtils::RemoveFile(const std::string &fileName)
{
    if (!CheckPathExist(fileName)) {
        FONT_LOGI("file %{public}s is not exist", fileName.c_str());
        return true;
    }
    return RemoveAll(fileName);
}

void FontManagerUtils::DeleteDir(const std::string &rootPath, bool isDeleteRootDir)
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

bool FontManagerUtils::RemoveAll(const std::filesystem::path &path)
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

std::vector<std::string> FontManagerUtils::GetFullNamesByFd(const int32_t &fd)
{
    std::vector<std::string> fullNames;
    struct stat sourceStat;
    if (fstat(fd, &sourceStat) < 0) {
        FONT_LOGE("GetFullNamesByFd: fstat failed, errno: %{public}d", errno);
        return fullNames;
    }
    if (sourceStat.st_size < 0 || static_cast<size_t>(sourceStat.st_size) > MAX_FONT_FILE_SIZE) {
        FONT_LOGE("GetFullNamesByFd: file too large, size: %{public}lld", static_cast<long long>(sourceStat.st_size));
        return fullNames;
    }
    auto& fontToolSet = OHOS::Rosen::FontToolSet::GetInstance();
    fullNames = fontToolSet.GetFontFullName(fd);
    if (fullNames.empty()) {
        FONT_LOGE("GetFullNamesByFd FullNameList is empty.");
    }
    return fullNames;
}

std::vector<std::string> FontManagerUtils::GetFullNamesByPath(const std::string &path)
{
    std::vector<std::string> fullNames;
    FILE* fp = fopen(path.c_str(), "rb");
    if (!fp) {
        FONT_LOGE("GetFullNamesByPath open font file failed");
        return fullNames;
    }
    int fd = fileno(fp);
    if (fd < 0) {
        FONT_LOGE("open font file failed, errno: %{public}d", errno);
        (void)fclose(fp);
        return fullNames;
    }
    auto& fontToolSet = OHOS::Rosen::FontToolSet::GetInstance();
    fullNames = fontToolSet.GetFontFullName(fd);
    if (fullNames.empty()) {
        FONT_LOGE("GetFullNamesByPath FullNameList is empty.path:%{public}s", path.c_str());
    }
    (void)fclose(fp);
    return fullNames;
}
} // namespace FontManager
} // namespace Global
} // namespace OHOS

