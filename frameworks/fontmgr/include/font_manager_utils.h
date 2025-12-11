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

#ifndef FONT_MANAGER_UTILS_H
#define FONT_MANAGER_UTILS_H

#include <filesystem>
#include <string>
#include <vector>

namespace OHOS {
namespace Global {
namespace FontManager {
class FontManagerUtils {
public:
    static bool CheckAndInitInstallPath(const std::string &installPath);
    static bool CheckPathExist(const std::string &pathName);
    static bool CheckFontConfigPath(const std::string &installPath);
    static bool CreateDirWithPermission(const std::string &fileDir);
    static std::string GetFileName(const std::string &path);
    static std::string GetFileDirectory(const std::string &path);
    static bool CopyFile(int32_t sourceFd, const std::string& path);
    static std::string GetFilePathByFd(const int32_t &fd);
    static bool RenameFile(const std::string& src, const std::string& dest);
    static std::string GetFileTime();
    static bool RemoveFile(const std::string &path);
    static void DeleteDir(const std::string &rootPath, bool isDeleteRootDir);
    static std::vector<int32_t> GetAllCreatedUserIds();
    static void ClearAllTempFileDir();
    static std::vector<std::string> GetFullNamesByFd(const int32_t &fd);
    static std::vector<std::string> GetFullNamesByPath(const std::string &path);
private:
    static bool CreateFileWithPermission(const std::string &filePath, const std::string &defaultStr = "");
    static bool CopyFileByFd(int32_t sourceFd, int32_t targetFd);
    static bool RemoveAll(const std::filesystem::path &path);
};
} // namespace FontManager
} // namespace Global
} // namespace OHOS
#endif // FONT_MANAGER_UTILS_H
