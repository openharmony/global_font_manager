/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include <vector>
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <dirent.h>
#include <unistd.h>

#define private public
#define protected public
#include "font_manager.h"
#include "storage_manager_adapter.h"
#undef private
#undef protected
#include "font_manager_utils.h"
#include "font_define.h"
#include "directory_ex.h"
#include "permission_common.h"

namespace {
const std::string INSTALL_PATH_TEST = "/data/service/el1/100/for-all-app/fonts/";
const std::string TEMP_PATH_TEST = "/data/service/el1/100/for-all-app/fonts/temp/";
const std::string FONT_CONFIG_FILE_TEST = INSTALL_PATH_TEST + "install_fontconfig.json";
const std::string FONT_PATH = "/data/test/TestFont_Sans.ttf";
const std::string FONT_FULL_NAME = "HarmonyOS Sans";
const std::string TTC_FONT_PATH = "/data/test/NotoSansCJK-Regular.ttc";
constexpr int32_t TEST_USERID = 100;
const std::vector<std::string> TTC_FONT_FULL_NAME{"Noto Sans CJK JP",
    "Noto Sans CJK KR",
    "Noto Sans CJK SC",
    "Noto Sans CJK TC",
    "Noto Sans CJK HK",
    "Noto Sans Mono CJK JP",
    "Noto Sans Mono CJK KR",
    "Noto Sans Mono CJK SC",
    "Noto Sans Mono CJK TC",
    "Noto Sans Mono CJK HK"};
}

using testing::ext::TestSize;
using namespace std;

namespace OHOS {
namespace Global {
namespace FontManager {

class FontManagerTest : public testing::Test {
public:
    FontManagerTest(){};
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

protected:
    std::shared_ptr<FontManager> manager_;
};

void FontManagerTest::SetUpTestCase(void)
{
    FontManagerUtils::DeleteDir(INSTALL_PATH_TEST, false);
}

void FontManagerTest::TearDownTestCase(void)
{
    PermissionCommon::ResetTokenAndUid();
}

void FontManagerTest::SetUp(void)
{
    manager_ = FontManager::GetInstance();
    manager_->configMap_.clear();
}

void FontManagerTest::TearDown(void)
{
    FontManagerUtils::DeleteDir(INSTALL_PATH_TEST, false);
    FontManagerUtils::DeleteDir(INSTALL_PATH_APP, false);
}

/**
 * @tc.name: FontManagerFuncTest001
 * @tc.desc: Test FontManager InstallFont case
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest001, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    int ret = manager_->InstallFont(fd, TEST_USERID);
    EXPECT_EQ(ret, ERR_OK);
    if (fd >= 0) {
        close(fd);
    }
    fd = open(FONT_PATH.c_str(), O_RDONLY);
    ret = manager_->InstallFont(fd, TEST_USERID);
    EXPECT_EQ(ret, ERR_INSTALLED_ALRADY);
}

/**
 * @tc.name: FontManagerFuncTest002
 * @tc.desc: Test FontManager UninstallFont case
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest002, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    int ret = manager_->InstallFont(fd, TEST_USERID);
    EXPECT_EQ(ret, ERR_OK);
    if (fd >= 0) {
        close(fd);
    }
    ret = manager_->UninstallFont(FONT_FULL_NAME, TEST_USERID);
    EXPECT_EQ(ret, ERR_OK);
    ret = manager_->UninstallFont(FONT_FULL_NAME, TEST_USERID);
    EXPECT_EQ(ret, ERR_UNINSTALL_FILE_NOT_EXISTS);
    ret = manager_->UninstallFont("", TEST_USERID);
    EXPECT_EQ(ret, ERR_UNINSTALL_FILE_NOT_EXISTS);
}

/**
 * @tc.name: FontManagerFuncTest003
 * @tc.desc: Test FontManager GetFontFullName case
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest003, TestSize.Level1)
{
    auto fullNameVector = FontManagerUtils::GetFullNamesByPath(FONT_PATH);
    std::vector<std::string> fullName{FONT_FULL_NAME};
    EXPECT_EQ(fullName, fullNameVector);
}

/**
 * @tc.name: FontManagerFuncTest004
 * @tc.desc: Test FontManager GetFontFullName error case
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest004, TestSize.Level1)
{
    const std::string errTTFPath = "/data/test/errorTTF.ttf";
    const std::string emptyTTFPath = "/data/test/emptyTTF.ttf";
    const std::string errorType = "/data/test/errorType.txt";
    const std::string errorTTCPath = "/data/test/errorTTC.ttc";
    const std::string emptyTTCPath = "/data/test/emptyTTC.ttc";

    auto fullNameVector = FontManagerUtils::GetFullNamesByPath(errTTFPath);
    EXPECT_EQ(fullNameVector.size(), 0);

    fullNameVector = FontManagerUtils::GetFullNamesByPath(emptyTTFPath);
    EXPECT_EQ(fullNameVector.size(), 0);

    fullNameVector = FontManagerUtils::GetFullNamesByPath(errorType);
    EXPECT_EQ(fullNameVector.size(), 0);

    fullNameVector = FontManagerUtils::GetFullNamesByPath(errorTTCPath);
    EXPECT_EQ(fullNameVector.size(), 0);

    fullNameVector = FontManagerUtils::GetFullNamesByPath(emptyTTCPath);
    EXPECT_EQ(fullNameVector.size(), 0);
}

/**
 * @tc.name: FontManagerFuncTest005
 * @tc.desc: Test FontManager CheckFontConfigPath case
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest005, TestSize.Level1)
{
    int ret = FontManagerUtils::CheckFontConfigPath(INSTALL_PATH_TEST);
    EXPECT_EQ(ret, true);
    FontManagerUtils::RemoveFile(FONT_CONFIG_FILE_TEST);
    ret = FontManagerUtils::CheckFontConfigPath(INSTALL_PATH_TEST);
    EXPECT_EQ(ret, true);
    ret = FontManagerUtils::CheckFontConfigPath(INSTALL_PATH_TEST);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: FontManagerFuncTest006
 * @tc.desc: Test FontManager ERR_MAX_FILE_COUNT more than 200 case
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest006, TestSize.Level1)
{
    int fd = open("/data/test/200install_fontconfig.json", O_RDONLY);
    EXPECT_EQ(FontManagerUtils::CopyFile(fd, FONT_CONFIG_FILE_TEST), true);
    fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    int ret = manager_->InstallFont(fd, TEST_USERID);
    EXPECT_EQ(ret, ERR_MAX_FILE_COUNT);
    if (fd >= 0) {
        close(fd);
    }
}

/**
 * @tc.name: FontManagerFuncTest007
 * @tc.desc: Test FontManager ERR_MAX_FILE_COUNT less than 200 case
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest007, TestSize.Level1)
{
    int fd = open("/data/test/199install_fontconfig.json", O_RDONLY);
    EXPECT_EQ(FontManagerUtils::CopyFile(fd, FONT_CONFIG_FILE_TEST), true);
    if (fd > 0) {
        close(fd);
    }
    fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    int ret = manager_->InstallFont(fd, TEST_USERID);
    EXPECT_EQ(ret, ERR_OK);
    if (fd >= 0) {
        close(fd);
    }
}

/**
 * @tc.name: FontManagerFuncTest008
 * @tc.desc: Test FontManager TTC InstallFont case
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest008, TestSize.Level1)
{
    int fd = open(TTC_FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    int ret = manager_->InstallFont(fd, TEST_USERID);
    EXPECT_EQ(ret, ERR_OK);
    if (fd >= 0) {
        close(fd);
    }
    fd = open(TTC_FONT_PATH.c_str(), O_RDONLY);
    ret = manager_->InstallFont(fd, TEST_USERID);
    EXPECT_EQ(ret, ERR_INSTALLED_ALRADY);
}

/**
 * @tc.name: FontManagerFuncTest009
 * @tc.desc: Test FontManager TTC UninstallFont case
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest009, TestSize.Level1)
{
    int fd = open(TTC_FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    int ret = manager_->InstallFont(fd, TEST_USERID);
    EXPECT_EQ(ret, ERR_OK);
    if (fd >= 0) {
        close(fd);
    }
    ret = manager_->UninstallFont(TTC_FONT_FULL_NAME[0], TEST_USERID);
    EXPECT_EQ(ret, ERR_OK);
    ret = manager_->UninstallFont(TTC_FONT_FULL_NAME[0], TEST_USERID);
    EXPECT_EQ(ret, ERR_UNINSTALL_FILE_NOT_EXISTS);
    ret = manager_->UninstallFont("", TEST_USERID);
    EXPECT_EQ(ret, ERR_UNINSTALL_FILE_NOT_EXISTS);
}

/**
 * @tc.name: FontManagerFuncTest010
 * @tc.desc: Test FontManager TTC GetFontFullName case
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest010, TestSize.Level1)
{
    auto fullNameVector = FontManagerUtils::GetFullNamesByPath(TTC_FONT_PATH);
    std::sort(fullNameVector.begin(), fullNameVector.end());
    auto ttcFullName = TTC_FONT_FULL_NAME;
    std::sort(ttcFullName.begin(), ttcFullName.end());
    EXPECT_EQ(fullNameVector, ttcFullName);
}

/**
 * @tc.name: FontManagerFuncTest011
 * @tc.desc: Test FontManager TTC GetFontFullName case
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest011, TestSize.Level1)
{
    const std::string errTTFPath = "/data/test/errorTTF.ttf";
    const std::string emptyTTFPath = "/data/test/emptyTTF.ttf";
    const std::string errorType = "/data/test/errorType.txt";
    const std::string errorTTCPath = "/data/test/errorTTC.ttc";
    const std::string emptyTTCPath = "/data/test/emptyTTC.ttc";
    int fd = open(errTTFPath.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    int ret = manager_->InstallFont(fd, TEST_USERID);
    EXPECT_EQ(ret, ERR_FILE_VERIFY_FAIL);
    if (fd >= 0) {
        close(fd);
    }
    fd = open(emptyTTFPath.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    ret = manager_->InstallFont(fd, TEST_USERID);
    EXPECT_EQ(ret, ERR_FILE_VERIFY_FAIL);
    if (fd >= 0) {
        close(fd);
    }
    fd = open(errorType.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    ret = manager_->InstallFont(fd, TEST_USERID);
    EXPECT_EQ(ret, ERR_FILE_VERIFY_FAIL);
    if (fd >= 0) {
        close(fd);
    }
    fd = open(errorTTCPath.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    ret = manager_->InstallFont(fd, TEST_USERID);
    EXPECT_EQ(ret, ERR_FILE_VERIFY_FAIL);
    if (fd >= 0) {
        close(fd);
    }
    fd = open(emptyTTCPath.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    ret = manager_->InstallFont(fd, TEST_USERID);
    EXPECT_EQ(ret, ERR_FILE_VERIFY_FAIL);
    if (fd >= 0) {
        close(fd);
    }
}

/**
 * @tc.name: FontManagerFuncTest012
 * @tc.desc: Test FontManager Repeat InstallFont case
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest012, TestSize.Level1)
{
    FontManagerUtils::CreateDirWithPermission("/data/test/testRepeats/");
    const std::string fontPath1 = "/data/test/TestFont_Sans.ttf";
    const std::string fontPath4 = "/data/test/NotoSansVai-Regular.ttf";
    const std::string fontPath5 = "/data/test/testRepeats/TestFont_Sans.ttf";

    int fd = open(fontPath1.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    int ret = manager_->InstallFont(fd, TEST_USERID);
    if (fd > 0) {
        close(fd);
    }
    EXPECT_EQ(ret, ERR_OK);
    ASSERT_TRUE(FontManagerUtils::CheckPathExist("/data/test/testRepeats/"));
    fd = open(fontPath4.c_str(), O_RDONLY);
    FontManagerUtils::CopyFile(fd, fontPath5);
    if (fd > 0) {
        close(fd);
    }
    fd = open(fontPath5.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    ret = manager_->InstallFont(fd, TEST_USERID);
    if (fd > 0) {
        close(fd);
    }
    EXPECT_EQ(ret, ERR_OK);
    std::filesystem::path rPath("/data/test/testRepeats/");
    for (const auto &file : std::filesystem::directory_iterator(rPath)) {
        if (file.is_regular_file()) {
            std::filesystem::remove(file);
        }
    }
    EXPECT_EQ(rmdir("/data/test/testRepeats") != -1, true);
}

/**
 * @tc.name: FontManagerFuncTest013
 * @tc.desc: Test FontManager Repeat InstallFont case
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest013, TestSize.Level1)
{
    FontManagerUtils::CreateDirWithPermission("/data/test/testRepeats/");
    const std::string fontPath2 = "/data/test/NotoSansCJK-Regular.ttc";
    const std::string fontPath3 = "/data/test/NotoSerifCJK-Regular.ttc";
    const std::string fontPath6 = "/data/test/testRepeats/NotoSerifCJK-Regular.ttc";

    int fd = open(fontPath2.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    int ret = manager_->InstallFont(fd, TEST_USERID);
    if (fd > 0) {
        close(fd);
    }
    EXPECT_EQ(ret, ERR_OK);
    ASSERT_TRUE(FontManagerUtils::CheckPathExist("/data/test/testRepeats/"));
    fd = open(fontPath3.c_str(), O_RDONLY);
    FontManagerUtils::CopyFile(fd, fontPath6);
    if (fd > 0) {
        close(fd);
    }
    fd = open(fontPath6.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    ret = manager_->InstallFont(fd, TEST_USERID);
    if (fd > 0) {
        close(fd);
    }
    EXPECT_EQ(ret, ERR_OK);
    std::filesystem::path rPath("/data/test/testRepeats/");
    for (const auto &file : std::filesystem::directory_iterator(rPath)) {
        if (file.is_regular_file()) {
            std::filesystem::remove(file);
        }
    }
    EXPECT_EQ(rmdir("/data/test/testRepeats") != -1, true);
}

/**
 * @tc.name: FontManagerFuncTest014
 * @tc.desc: Test CopyFileForInstall when target file already exists (Rename logic)
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest014, TestSize.Level1)
{
    ASSERT_EQ(FontManagerUtils::CheckAndInitInstallPath(INSTALL_PATH_TEST), true);

    std::string targetFileName = "TestFont_Sans.ttf";
    std::string targetPath = INSTALL_PATH_TEST + targetFileName;
    int fdTarget = open(targetPath.c_str(), O_CREAT | O_RDWR, 0666);
    ASSERT_GE(fdTarget, 0);
    close(fdTarget);

    int fdSource = open(FONT_PATH.c_str(), O_RDONLY);
    ASSERT_GE(fdSource, 0);

    int ret = manager_->InstallFont(fdSource, TEST_USERID);
    close(fdSource);

    EXPECT_EQ(ret, ERR_OK);

    bool hasRenamedFile = false;
    DIR *dir = opendir(INSTALL_PATH_TEST.c_str());
    if (dir) {
        struct dirent *ent;
        while ((ent = readdir(dir)) != nullptr) {
            std::string name = ent->d_name;
            if (name.find("_TestFont_Sans.ttf") != std::string::npos) {
                hasRenamedFile = true;
                break;
            }
        }
        closedir(dir);
    }
    EXPECT_TRUE(hasRenamedFile);
}

/**
 * @tc.name: FontManagerFuncTest015
 * @tc.desc: Test InstallFont triggers ReportFontBundleStats
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest015, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    int ret = manager_->InstallFont(fd, TEST_USERID);
    EXPECT_EQ(ret, ERR_OK);
    if (fd >= 0) {
        close(fd);
    }
    auto adapter = StorageManagerAdapter::GetInstance();
    uint64_t size = adapter->GetFontFolderSize(INSTALL_PATH_TEST);
    EXPECT_GT(size, 0);
}

/**
 * @tc.name: FontManagerFuncTest016
 * @tc.desc: Test UninstallFont triggers ReportFontBundleStats
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest016, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    int ret = manager_->InstallFont(fd, TEST_USERID);
    EXPECT_EQ(ret, ERR_OK);
    if (fd >= 0) {
        close(fd);
    }
    ret = manager_->UninstallFont(FONT_FULL_NAME, TEST_USERID);
    EXPECT_EQ(ret, ERR_OK);
    auto adapter = StorageManagerAdapter::GetInstance();
    uint64_t size = adapter->GetFontFolderSize(INSTALL_PATH_TEST);
    EXPECT_GT(size, 0);
}
} // namespace FontManager
} // namespace Global
} // namespace OHOS