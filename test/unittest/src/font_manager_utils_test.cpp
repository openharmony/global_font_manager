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
#include <cerrno>
#include <fcntl.h>
#include <string>
#include <unistd.h>

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
    EXPECT_TRUE(FontManagerUtils::CheckPathExist("/data"));
    EXPECT_TRUE(FontManagerUtils::CheckPathExist("/data/test"));
    EXPECT_FALSE(FontManagerUtils::CheckPathExist("/data/test/nonexistent_path_12345"));
    EXPECT_FALSE(FontManagerUtils::CheckPathExist("/data/nonexistent_path_99999"));
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
    // DeleteDir on non-existent path should not crash and return void
    FontManagerUtils::DeleteDir("/data/test/nonexistent_dir_12345", true);
    // Verify the path still doesn't exist (no side effects)
    EXPECT_FALSE(FontManagerUtils::CheckPathExist("/data/test/nonexistent_dir_12345"));
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
    EXPECT_FALSE(FontManagerUtils::CheckPathExist(path));
    // Non-existent path should also return true
    std::filesystem::path nonExistentPath("/data/test/nonexistent_removeall_12345");
    EXPECT_FALSE(std::filesystem::exists(nonExistentPath));
    EXPECT_TRUE(FontManagerUtils::RemoveAll(nonExistentPath));
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

/**
 * @tc.name: CheckAndInitScopeFontPathTest001
 * @tc.desc: Test CheckAndInitScopeFontPath with new path
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerUtilsTest, CheckAndInitScopeFontPathTest001, TestSize.Level1)
{
    std::string scopePath = "/data/test/test_scope_path/";
    FontManagerUtils::DeleteDir(scopePath, true);
    EXPECT_TRUE(FontManagerUtils::CheckAndInitScopeFontPath(scopePath));
    EXPECT_TRUE(FontManagerUtils::CheckPathExist(scopePath));
    EXPECT_TRUE(FontManagerUtils::CheckPathExist(scopePath + "temp/"));
    FontManagerUtils::DeleteDir(scopePath, true);
}

/**
 * @tc.name: CheckAndInitScopeFontPathTest002
 * @tc.desc: Test CheckAndInitScopeFontPath with existing path
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerUtilsTest, CheckAndInitScopeFontPathTest002, TestSize.Level1)
{
    std::string scopePath = "/data/test/test_scope_path2/";
    FontManagerUtils::CreateDirWithPermission(scopePath);
    EXPECT_TRUE(FontManagerUtils::CheckAndInitScopeFontPath(scopePath));
    FontManagerUtils::DeleteDir(scopePath, true);
}

/**
 * @tc.name: ClearAllTempFileDirTest002
 * @tc.desc: Test ClearAllTempFileDir removes temp files
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerUtilsTest, ClearAllTempFileDirTest002, TestSize.Level1)
{
    FontManagerUtils::CheckAndInitInstallPath(INSTALL_PATH_TEST);
    // Create a temp file
    std::string tempFile = TEMP_PATH_TEST + "test_temp.txt";
    FontManagerUtils::CreateFileWithPermission(tempFile, "test");
    EXPECT_TRUE(FontManagerUtils::CheckPathExist(tempFile));
    FontManagerUtils::ClearAllTempFileDir();
    EXPECT_FALSE(FontManagerUtils::CheckPathExist(tempFile));
}

/**
 * @tc.name: CleanupScopeFontDirsTest001
 * @tc.desc: Test CleanupScopeFontDirs with no scope dirs
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerUtilsTest, CleanupScopeFontDirsTest001, TestSize.Level1)
{
    FontManagerUtils::CheckAndInitInstallPath(INSTALL_PATH_TEST);
    FontManagerUtils::CleanupScopeFontDirs();
    // No crash, temp dir should still exist
    EXPECT_TRUE(FontManagerUtils::CheckPathExist(INSTALL_PATH_TEST));
}

/**
 * @tc.name: CleanupScopeFontDirsTest002
 * @tc.desc: Test CleanupScopeFontDirs removes empty app dir
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerUtilsTest, CleanupScopeFontDirsTest002, TestSize.Level1)
{
    FontManagerUtils::CheckAndInitInstallPath(INSTALL_PATH_TEST);
    std::string appDir = INSTALL_PATH_TEST + "app_test_cleanup/";
    FontManagerUtils::CreateDirWithPermission(appDir);
    EXPECT_TRUE(FontManagerUtils::CheckPathExist(appDir));
    FontManagerUtils::CleanupScopeFontDirs();
    // Empty app_ dir should be removed
    EXPECT_FALSE(FontManagerUtils::CheckPathExist(appDir));
}

/**
 * @tc.name: CleanupAllScopeFontDirsTest001
 * @tc.desc: Test CleanupAllScopeFontDirs with no scope dirs
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerUtilsTest, CleanupAllScopeFontDirsTest001, TestSize.Level1)
{
    FontManagerUtils::CheckAndInitInstallPath(INSTALL_PATH_TEST);
    FontManagerUtils::CleanupAllScopeFontDirs();
    EXPECT_TRUE(FontManagerUtils::CheckPathExist(INSTALL_PATH_TEST));
}

/**
 * @tc.name: CleanupAllScopeFontDirsTest002
 * @tc.desc: Test CleanupAllScopeFontDirs removes app dir
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerUtilsTest, CleanupAllScopeFontDirsTest002, TestSize.Level1)
{
    FontManagerUtils::CheckAndInitInstallPath(INSTALL_PATH_TEST);
    std::string appDir = INSTALL_PATH_TEST + "app_test_all_cleanup/";
    FontManagerUtils::CreateDirWithPermission(appDir);
    EXPECT_TRUE(FontManagerUtils::CheckPathExist(appDir));
    FontManagerUtils::CleanupAllScopeFontDirs();
    EXPECT_FALSE(FontManagerUtils::CheckPathExist(appDir));
}

/**
 * @tc.name: CleanupAppScopeFontDirsTest001
 * @tc.desc: Test CleanupAppScopeFontDirs with no app dirs
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerUtilsTest, CleanupAppScopeFontDirsTest001, TestSize.Level1)
{
    FontManagerUtils::CheckAndInitInstallPath(INSTALL_PATH_TEST);
    FontManagerUtils::CleanupAppScopeFontDirs();
    EXPECT_TRUE(FontManagerUtils::CheckPathExist(INSTALL_PATH_TEST));
}

/**
 * @tc.name: CleanupAppScopeFontDirsTest002
 * @tc.desc: Test CleanupAppScopeFontDirs with app and session dirs
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerUtilsTest, CleanupAppScopeFontDirsTest002, TestSize.Level1)
{
    FontManagerUtils::CheckAndInitInstallPath(INSTALL_PATH_TEST);
    std::string appDir = INSTALL_PATH_TEST + "app_cleanup_test/";
    std::string sessionDir = INSTALL_PATH_TEST + "session_keep_test/";
    FontManagerUtils::CreateDirWithPermission(appDir);
    FontManagerUtils::CreateDirWithPermission(sessionDir);
    FontManagerUtils::CleanupAppScopeFontDirs();
    EXPECT_FALSE(FontManagerUtils::CheckPathExist(appDir));
    EXPECT_TRUE(FontManagerUtils::CheckPathExist(sessionDir));
    FontManagerUtils::DeleteDir(sessionDir, true);
}

/**
 * @tc.name: CopyFileByFdTest003
 * @tc.desc: Test CopyFileByFd with valid fds
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerUtilsTest, CopyFileByFdTest003, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_GE(fd, 0);
    std::string destPath = "/data/test/test_copybyfd_stat.ttf";
    int targetFd = open(destPath.c_str(), O_CREAT | O_WRONLY, 0644);
    EXPECT_GE(targetFd, 0);
    EXPECT_TRUE(FontManagerUtils::CopyFileByFd(fd, targetFd));
    if (fd >= 0) {
        close(fd);
    }
    if (targetFd >= 0) {
        close(targetFd);
    }
    FontManagerUtils::RemoveFile(destPath);
}

/**
 * @tc.name: CopyFileTest003
 * @tc.desc: Test CopyFile with target that can't be opened
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerUtilsTest, CopyFileTest003, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_GE(fd, 0);
    std::string destPath = "/data/test/";
    EXPECT_FALSE(FontManagerUtils::CopyFile(fd, destPath));
    if (fd >= 0) {
        close(fd);
    }
}

/**
 * @tc.name: IsValidFontFdTest001
 * @tc.desc: Test IsValidFontFd with regular, empty, directory and invalid fds
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerUtilsTest, IsValidFontFdTest001, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    ASSERT_GE(fd, 0);
    EXPECT_TRUE(FontManagerUtils::IsValidFontFd(fd));
    close(fd);

    EXPECT_FALSE(FontManagerUtils::IsValidFontFd(-1));

    int emptyFd = open("/data/test/emptyTTF.ttf", O_RDONLY);
    ASSERT_GE(emptyFd, 0);
    EXPECT_FALSE(FontManagerUtils::IsValidFontFd(emptyFd));
    close(emptyFd);

    int dirFd = open("/data/test", O_RDONLY | O_DIRECTORY);
    ASSERT_GE(dirFd, 0);
    EXPECT_FALSE(FontManagerUtils::IsValidFontFd(dirFd));
    close(dirFd);
}

/**
 * @tc.name: MakeAppIdentifierTest001
 * @tc.desc: Test MakeAppIdentifier with APP and SESSION scope
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerUtilsTest, MakeAppIdentifierTest001, TestSize.Level1)
{
    std::string result = FontManagerUtils::MakeAppIdentifier(FONT_SCOPE_APP, 5001);
    EXPECT_EQ(result, "app_5001");
    std::string result2 = FontManagerUtils::MakeAppIdentifier(FONT_SCOPE_APP, 0);
    EXPECT_EQ(result2, "app_0");
    std::string result3 = FontManagerUtils::MakeAppIdentifier(FONT_SCOPE_SESSION, 5001);
    EXPECT_EQ(result3, "session_5001");
    std::string result4 = FontManagerUtils::MakeAppIdentifier(FONT_SCOPE_SESSION, 9999);
    EXPECT_EQ(result4, "session_9999");
}

/**
 * @tc.name: CheckFontConfigPathTest001
 * @tc.desc: Test CheckFontConfigPath with existing config and missing config
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerUtilsTest, CheckFontConfigPathTest001, TestSize.Level1)
{
    FontManagerUtils::CheckAndInitInstallPath(INSTALL_PATH_TEST);
    EXPECT_TRUE(FontManagerUtils::CheckPathExist(INSTALL_PATH_TEST));
    // Config already exists, should return true
    EXPECT_TRUE(FontManagerUtils::CheckFontConfigPath(INSTALL_PATH_TEST));
    // Remove config, CheckFontConfigPath should recreate it
    std::string configFile = INSTALL_PATH_TEST + "install_fontconfig.json";
    FontManagerUtils::RemoveFile(configFile);
    EXPECT_TRUE(FontManagerUtils::CheckFontConfigPath(INSTALL_PATH_TEST));
    EXPECT_TRUE(FontManagerUtils::CheckPathExist(configFile));
}

/**
 * @tc.name: GetAllCreatedUserIdsTest001
 * @tc.desc: Test GetAllCreatedUserIds returns non-empty
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerUtilsTest, GetAllCreatedUserIdsTest001, TestSize.Level1)
{
    auto userIds = FontManagerUtils::GetAllCreatedUserIds();
    EXPECT_FALSE(userIds.empty());
    EXPECT_GE(userIds.size(), 1u);
    for (const auto &userId : userIds) {
        EXPECT_GE(userId, 0);
    }
}

/**
 * @tc.name: RemoveFileTest003
 * @tc.desc: Test RemoveFile returns true for existing file
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerUtilsTest, RemoveFileTest003, TestSize.Level1)
{
    std::string path = "/data/test/test_remove_existing.txt";
    FontManagerUtils::CreateFileWithPermission(path, "test");
    EXPECT_TRUE(FontManagerUtils::RemoveFile(path));
    EXPECT_FALSE(FontManagerUtils::CheckPathExist(path));
}

/**
 * @tc.name: GetFilePathByFdTest003
 * @tc.desc: Test GetFilePathByFd with valid fd returns correct path
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerUtilsTest, GetFilePathByFdTest003, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_GE(fd, 0);
    std::string path = FontManagerUtils::GetFilePathByFd(fd);
    EXPECT_FALSE(path.empty());
    EXPECT_EQ(path, FONT_PATH);
    if (fd >= 0) {
        close(fd);
    }
}

/**
 * @tc.name: CreateDirWithPermissionTest002
 * @tc.desc: Test CreateDirWithPermission with valid dir
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerUtilsTest, CreateDirWithPermissionTest002, TestSize.Level1)
{
    std::string dir = "/data/test/test_create_dir_valid";
    FontManagerUtils::DeleteDir(dir, true);
    EXPECT_TRUE(FontManagerUtils::CreateDirWithPermission(dir));
    EXPECT_TRUE(FontManagerUtils::CheckPathExist(dir));
    FontManagerUtils::DeleteDir(dir, true);
}

/**
 * @tc.name: GetFullNamesByPrivateFdTest001
 * @tc.desc: A private temp copy stays parseable after the caller truncates the source fd.
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerUtilsTest, GetFullNamesByPrivateFdTest001, TestSize.Level1)
{
    ASSERT_TRUE(FontManagerUtils::CheckAndInitInstallPath(INSTALL_PATH_TEST));
    int srcFd = open(FONT_PATH.c_str(), O_RDONLY);
    ASSERT_GE(srcFd, 0);
    std::string tempPath;
    int32_t tempFd = FontManagerUtils::CreatePrivateTempCopy(srcFd, TEMP_PATH_TEST, tempPath);
    ASSERT_GE(tempFd, 0);
    EXPECT_FALSE(tempPath.empty());

    std::vector<std::string> names = FontManagerUtils::GetFullNamesByPrivateFd(tempFd);
    EXPECT_FALSE(names.empty());

    // The caller still owns its fd and can truncate it; the snapshot must survive.
    ASSERT_GE(ftruncate(srcFd, 0), 0);
    std::vector<std::string> namesAfterTruncate = FontManagerUtils::GetFullNamesByPrivateFd(tempFd);
    EXPECT_EQ(names, namesAfterTruncate);

    close(tempFd);
    close(srcFd);
    FontManagerUtils::RemoveFile(tempPath);

    EXPECT_TRUE(FontManagerUtils::GetFullNamesByPrivateFd(-1).empty());
}

/**
 * @tc.name: CreatePrivateTempCopyTest001
 * @tc.desc: Invalid and non-regular fds must not leave temp files behind.
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerUtilsTest, CreatePrivateTempCopyTest001, TestSize.Level1)
{
    ASSERT_TRUE(FontManagerUtils::CheckAndInitInstallPath(INSTALL_PATH_TEST));
    std::string tempPath;
    EXPECT_LT(FontManagerUtils::CreatePrivateTempCopy(-1, TEMP_PATH_TEST, tempPath), 0);
    EXPECT_TRUE(tempPath.empty());

    int dirFd = open("/data/test", O_RDONLY | O_DIRECTORY);
    ASSERT_GE(dirFd, 0);
    EXPECT_LT(FontManagerUtils::CreatePrivateTempCopy(dirFd, TEMP_PATH_TEST, tempPath), 0);
    close(dirFd);
    EXPECT_TRUE(tempPath.empty());
}

/**
 * @tc.name: GetFullNamesByPathNormalTest
 * @tc.desc: Regression: GetFullNamesByPath still returns full names.
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerUtilsTest, GetFullNamesByPathNormalTest, TestSize.Level1)
{
    std::vector<std::string> byPath = FontManagerUtils::GetFullNamesByPath(FONT_PATH);
    EXPECT_FALSE(byPath.empty());
}

} // namespace FontManager
} // namespace Global
} // namespace OHOS
