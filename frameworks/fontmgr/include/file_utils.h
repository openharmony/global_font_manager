/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef FONT_MANAGER_FILE_UTILS_H
#define FONT_MANAGER_FILE_UTILS_H

#include <filesystem>
#include <string>

namespace OHOS {
namespace Global {
namespace FontManager {
class FileUtils {
public:
    static bool CheckPathExist(const std::string &pathName);
    static bool CreatDirWithPermission(const std::string &fileDir);
    static bool CreateFileWithPermission(const std::string &filePath, const std::string &defalutStr = "");
    static std::string GetFileName(const std::string &path);
    static bool CopyFile(int32_t sourceFd, const std::string& path);
    static std::string GetFilePathByFd(const int32_t &fd);
    static bool RenameFile(const std::string& src, const std::string& dest);
    static std::string GetFileTime();
    static bool RemoveFile(const std::string &path);
    static void DeleteDir(const std::string &rootPath, bool isDeleteRootDir);
private:
    static bool CopyFileByFd(int32_t sourceFd, int32_t targetFd);
    static bool RemoveAll(const std::filesystem::path &path);
};
} // namespace FontManager
} // namespace Global
} // namespace OHOS
#endif // FONT_MANAGER_FILE_UTILS_H
