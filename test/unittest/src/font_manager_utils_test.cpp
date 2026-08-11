/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include <fcntl.h>
#include <string>

#include "font_manager_utils.h"
#include "font_define.h"
#include "permission_common.h"

namespace {
const std::string INSTALL_PATH_TEST = "/data/service/el1/100/for-all-app/fonts/";
const std::string TEMP_PATH_TEST = INSTALL_PATH_TEST + "temp/";
const std::string FONT_PATH = "/data/test/TestFont_Sans.ttf";
}

using testing::ext::TestSize;

namespace OHOS {
namespace Global {
namespace FontManager {

class FontManagerUtilsTest : public testing::Test {
public:
    FontManagerUtilsTest(){};
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void FontManagerUtilsTest::SetUpTestCase(void)
{
    FontManagerUtils::DeleteDir(INSTALL_PATH_TEST, false);
}

void FontManagerUtilsTest::TearDownTestCase(void)
{
    FontManagerUtils::DeleteDir(INSTALL_PATH_TEST, false);
}

void FontManagerUtilsTest::SetUp(void)
{
    PermissionCommon::SetFontManagerInitEnv();
    FontManagerUtils::DeleteDir(INSTALL_PATH_TEST, false);
}

void FontManagerUtilsTest::TearDown(void)
{
    PermissionCommon::RemovePermission();
    FontManagerUtils::DeleteDir(INSTALL_PATH_TEST, false);
}

HWTEST_F(FontManagerUtilsTest, CheckPathExistTest001, TestSize.Level1)
{
    EXPECT_FALSE(FontManagerUtils::CheckPathExist(""));
}

HWTEST_F(FontManagerUtilsTest, CheckPathExistTest002, TestSize.Level1)
{
    EXPECT_TRUE(FontManagerUtils::CheckPathExist("/data/test"));
    EXPECT_FALSE(FontManagerUtils::CheckPathExist("/data/test/nonexistent_path_12345"));
}

HWTEST_F(FontManagerUtilsTest, GetFileNameTest001, TestSize.Level1)
{
    EXPECT_EQ(FontManagerUtils::GetFileName("/data/test/font.ttf"), "font.ttf");
}

HWTEST_F(FontManagerUtilsTest, GetFileNameTest002, TestSize.Level1)
{
    EXPECT_EQ(FontManagerUtils::GetFileName("nofile.ttf"), "nofile.ttf");
}

HWTEST_F(FontManagerUtilsTest, GetFileDirectoryTest001, TestSize.Level1)
{
    EXPECT_EQ(FontManagerUtils::GetFileDirectory("/data/test/font.ttf"), "/data/test/");
}

HWTEST_F(FontManagerUtilsTest, GetFileDirectoryTest002, TestSize.Level1)
{
    EXPECT_EQ(FontManagerUtils::GetFileDirectory("nofile.ttf"), "");
}

HWTEST_F(FontManagerUtilsTest, CreateDirWithPermissionTest001, TestSize.Level1)
{
    EXPECT_FALSE(FontManagerUtils::CreateDirWithPermission(""));
    EXPECT_FALSE(FontManagerUtils::CreateDirWithPermission("/data/test/./bad"));
    EXPECT_FALSE(FontManagerUtils::CreateDirWithPermission("/data/test/x./bad"));
}

HWTEST_F(FontManagerUtilsTest, CreateFileWithPermissionTest001, TestSize.Level1)
{
    EXPECT_FALSE(FontManagerUtils::CreateFileWithPermission("", ""));
    EXPECT_FALSE(FontManagerUtils::CreateFileWithPermission("/data/test/./bad", ""));
    EXPECT_FALSE(FontManagerUtils::CreateFileWithPermission("/data/test/x./bad", ""));
}

HWTEST_F(FontManagerUtilsTest, CreateFileWithPermissionTest002, TestSize.Level1)
{
    std::string path = "/data/test/test_create_file.txt";
    EXPECT_TRUE(FontManagerUtils::CreateFileWithPermission(path, ""));
    FontManagerUtils::RemoveFile(path);
}

HWTEST_F(FontManagerUtilsTest, CreateFileWithPermissionTest003, TestSize.Level1)
{
    std::string path = "/data/test/test_create_file2.txt";
    EXPECT_TRUE(FontManagerUtils::CreateFileWithPermission(path, "test content"));
    FontManagerUtils::RemoveFile(path);
}

HWTEST_F(FontManagerUtilsTest, RenameFileTest001, TestSize.Level1)
{
    EXPECT_FALSE(FontManagerUtils::RenameFile("/data/test/nonexistent_src_12345", "/data/test/dest.txt"));
}

HWTEST_F(FontManagerUtilsTest, RenameFileTest002, TestSize.Level1)
{
    std::string src = "/data/test/test_rename_src.txt";
    std::string dest = "/data/test/test_rename_dest.txt";
    FontManagerUtils::CreateFileWithPermission(src, "test");
    EXPECT_TRUE(FontManagerUtils::RenameFile(src, dest));
    FontManagerUtils::RemoveFile(dest);
}

HWTEST_F(FontManagerUtilsTest, RemoveFileTest001, TestSize.Level1)
{
    EXPECT_TRUE(FontManagerUtils::RemoveFile("/data/test/nonexistent_file_12345"));
}

HWTEST_F(FontManagerUtilsTest, RemoveFileTest002, TestSize.Level1)
{
    std::string path = "/data/test/test_remove.txt";
    FontManagerUtils::CreateFileWithPermission(path, "test");
    EXPECT_TRUE(FontManagerUtils::RemoveFile(path));
}

HWTEST_F(FontManagerUtilsTest, CopyFileByFdTest001, TestSize.Level1)
{
    EXPECT_FALSE(FontManagerUtils::CopyFileByFd(-1, 1));
}

HWTEST_F(FontManagerUtilsTest, CopyFileByFdTest002, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_GE(fd, 0);
    EXPECT_FALSE(FontManagerUtils::CopyFileByFd(fd, -1));
    if (fd >= 0) {
        close(fd);
    }
}

HWTEST_F(FontManagerUtilsTest, CopyFileTest001, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_GE(fd, 0);
    std::string dest = "/data/nonexistent_dir_12345/test_copy.ttf";
    EXPECT_FALSE(FontManagerUtils::CopyFile(fd, dest));
    if (fd >= 0) {
        close(fd);
    }
}

HWTEST_F(FontManagerUtilsTest, CopyFileTest002, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_GE(fd, 0);
    std::string dest = "/data/test/test_copy_success.ttf";
    EXPECT_TRUE(FontManagerUtils::CopyFile(fd, dest));
    FontManagerUtils::RemoveFile(dest);
    if (fd >= 0) {
        close(fd);
    }
}

HWTEST_F(FontManagerUtilsTest, GetFilePathByFdTest001, TestSize.Level1)
{
    std::string path = FontManagerUtils::GetFilePathByFd(-1);
    EXPECT_TRUE(path.empty());
}

HWTEST_F(FontManagerUtilsTest, GetFilePathByFdTest002, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_GE(fd, 0);
    std::string path = FontManagerUtils::GetFilePathByFd(fd);
    EXPECT_FALSE(path.empty());
    if (fd >= 0) {
        close(fd);
    }
}

HWTEST_F(FontManagerUtilsTest, ClearAllTempFileDirTest001, TestSize.Level1)
{
    FontManagerUtils::CheckAndInitInstallPath(INSTALL_PATH_TEST);
    FontManagerUtils::ClearAllTempFileDir();
    EXPECT_FALSE(FontManagerUtils::CheckPathExist(TEMP_PATH_TEST));
}

HWTEST_F(FontManagerUtilsTest, DeleteDirTest001, TestSize.Level1)
{
    FontManagerUtils::DeleteDir("/data/test/nonexistent_dir_12345", true);
    SUCCEED();
}

HWTEST_F(FontManagerUtilsTest, DeleteDirTest002, TestSize.Level1)
{
    std::string dir = "/data/test/test_delete_dir";
    FontManagerUtils::CreateDirWithPermission(dir);
    std::string file1 = dir + "/file1.txt";
    std::string file2 = dir + "/file2.txt";
    FontManagerUtils::CreateFileWithPermission(file1, "test1");
    FontManagerUtils::CreateFileWithPermission(file2, "test2");
    FontManagerUtils::DeleteDir(dir, false);
    EXPECT_FALSE(FontManagerUtils::CheckPathExist(file1));
    EXPECT_FALSE(FontManagerUtils::CheckPathExist(file2));
    EXPECT_TRUE(FontManagerUtils::CheckPathExist(dir));
    FontManagerUtils::DeleteDir(dir, true);
    EXPECT_FALSE(FontManagerUtils::CheckPathExist(dir));
}

HWTEST_F(FontManagerUtilsTest, RemoveAllTest001, TestSize.Level1)
{
    std::string path = "/data/test/test_removeall.txt";
    FontManagerUtils::CreateFileWithPermission(path, "test");
    EXPECT_TRUE(FontManagerUtils::RemoveAll(path));
}

HWTEST_F(FontManagerUtilsTest, CheckAndInitInstallPathTest001, TestSize.Level1)
{
    EXPECT_TRUE(FontManagerUtils::CheckAndInitInstallPath(INSTALL_PATH_TEST));
    EXPECT_TRUE(FontManagerUtils::CheckPathExist(INSTALL_PATH_TEST));
    EXPECT_TRUE(FontManagerUtils::CheckPathExist(TEMP_PATH_TEST));
}

HWTEST_F(FontManagerUtilsTest, GetFileTimeTest001, TestSize.Level1)
{
    std::string time = FontManagerUtils::GetFileTime();
    EXPECT_FALSE(time.empty());
    EXPECT_EQ(time.length(), 15u);
}

HWTEST_F(FontManagerUtilsTest, GetFullNamesByPathTest001, TestSize.Level1)
{
    std::vector<std::string> names = FontManagerUtils::GetFullNamesByPath("/data/test/nonexistent_12345.ttf");
    EXPECT_TRUE(names.empty());
}

HWTEST_F(FontManagerUtilsTest, GetFullNamesByPathTest002, TestSize.Level1)
{
    std::vector<std::string> names = FontManagerUtils::GetFullNamesByPath(FONT_PATH);
    EXPECT_FALSE(names.empty());
}

} // namespace FontManager
} // namespace Global
} // namespace OHOS
