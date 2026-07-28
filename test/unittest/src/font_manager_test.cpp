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
#include "font_config.h"
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

/**
 * @tc.name: FontManagerFuncTest017
 * @tc.desc: Test InstallScopeFont with app scope
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest017, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    ScopeFontInstallInfo info;
    info.fd = fd;
    info.scope = FONT_SCOPE_APP;
    info.srcPath = "file://test/app_font1.ttf";
    info.bundleName = "com.example.app";
    info.appIdentifier = "app_test_001";
    info.userId = TEST_USERID;
    int ret = manager_->InstallScopeFont(info);
    if (fd >= 0) {
        close(fd);
    }
    EXPECT_EQ(ret, ERR_OK);

    auto scopeVal = manager_->GetFontScope("file://test/app_font1.ttf", TEST_USERID);
    EXPECT_EQ(scopeVal, FONT_SCOPE_APP);
}

/**
 * @tc.name: FontManagerFuncTest018
 * @tc.desc: Test InstallScopeFont duplicate srcPath returns ERR_INSTALLED_ALRADY
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest018, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    ScopeFontInstallInfo info;
    info.fd = fd;
    info.scope = FONT_SCOPE_APP;
    info.srcPath = "file://test/dup_src.ttf";
    info.bundleName = "com.example.app";
    info.appIdentifier = "app_test_002";
    info.userId = TEST_USERID;
    int ret = manager_->InstallScopeFont(info);
    if (fd >= 0) {
        close(fd);
    }
    EXPECT_EQ(ret, ERR_OK);

    fd = open(FONT_PATH.c_str(), O_RDONLY);
    info.fd = fd;
    ret = manager_->InstallScopeFont(info);
    if (fd >= 0) {
        close(fd);
    }
    EXPECT_EQ(ret, ERR_INSTALLED_ALRADY);
}

/**
 * @tc.name: FontManagerFuncTest019
 * @tc.desc: Test InstallScopeFont duplicate font name returns ERR_INSTALLED_ALRADY
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest019, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    ScopeFontInstallInfo info;
    info.fd = fd;
    info.scope = FONT_SCOPE_APP;
    info.srcPath = "file://test/name_dup1.ttf";
    info.bundleName = "com.example.app";
    info.appIdentifier = "app_test_003";
    info.userId = TEST_USERID;
    int ret = manager_->InstallScopeFont(info);
    if (fd >= 0) {
        close(fd);
    }
    EXPECT_EQ(ret, ERR_OK);

    fd = open(FONT_PATH.c_str(), O_RDONLY);
    info.fd = fd;
    info.srcPath = "file://test/name_dup2.ttf";
    info.appIdentifier = "app_test_004";
    ret = manager_->InstallScopeFont(info);
    if (fd >= 0) {
        close(fd);
    }
    EXPECT_EQ(ret, ERR_INSTALLED_ALRADY);
}

/**
 * @tc.name: FontManagerFuncTest020
 * @tc.desc: Test UninstallScopeFont by srcPath
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest020, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    ScopeFontInstallInfo info;
    info.fd = fd;
    info.scope = FONT_SCOPE_APP;
    info.srcPath = "file://test/uninstall_scope.ttf";
    info.bundleName = "com.example.app";
    info.appIdentifier = "app_test_005";
    info.userId = TEST_USERID;
    manager_->InstallScopeFont(info);
    if (fd >= 0) {
        close(fd);
    }

    int ret;
    manager_->UninstallScopeFont("file://test/uninstall_scope.ttf", "com.example.app", TEST_USERID);
    ret = manager_->GetFontScope("file://test/uninstall_scope.ttf", TEST_USERID);
    EXPECT_EQ(ret, FONT_SCOPE_NONE);
}

/**
 * @tc.name: FontManagerFuncTest021
 * @tc.desc: Test UninstallScopeFont with non-existent srcPath
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest021, TestSize.Level1)
{
    int ret = manager_->UninstallScopeFont("file://test/non_existent.ttf", "com.example.app", TEST_USERID);
    EXPECT_EQ(ret, ERR_UNINSTALL_FILE_NOT_EXISTS);
}

/**
 * @tc.name: FontManagerFuncTest022
 * @tc.desc: Test GetFontScope returns FONT_SCOPE_NONE for non-existent
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest022, TestSize.Level1)
{
    int ret = manager_->GetFontScope("file://test/not_installed.ttf", TEST_USERID);
    EXPECT_EQ(ret, FONT_SCOPE_NONE);
}

/**
 * @tc.name: FontManagerFuncTest023
 * @tc.desc: Test InstallScopeFont with session scope
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest023, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    ScopeFontInstallInfo info;
    info.fd = fd;
    info.scope = FONT_SCOPE_SESSION;
    info.srcPath = "file://test/session_font.ttf";
    info.bundleName = "com.example.session";
    info.appIdentifier = "session_test_001";
    info.userId = TEST_USERID;
    int ret = manager_->InstallScopeFont(info);
    if (fd >= 0) {
        close(fd);
    }
    EXPECT_EQ(ret, ERR_OK);

    auto scopeVal = manager_->GetFontScope("file://test/session_font.ttf", TEST_USERID);
    EXPECT_EQ(scopeVal, FONT_SCOPE_SESSION);
}

/**
 * @tc.name: FontManagerFuncTest024
 * @tc.desc: Test CleanupAppScopeFonts removes all fonts for an app
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest024, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    ScopeFontInstallInfo info;
    info.fd = fd;
    info.scope = FONT_SCOPE_APP;
    info.srcPath = "file://test/cleanup_app.ttf";
    info.bundleName = "com.example.cleanup";
    info.appIdentifier = "app_cleanup_001";
    info.userId = TEST_USERID;
    manager_->InstallScopeFont(info);
    if (fd >= 0) {
        close(fd);
    }

    manager_->CleanupAppScopeFonts("app_cleanup_001", TEST_USERID);
    int ret = manager_->GetFontScope("file://test/cleanup_app.ttf", TEST_USERID);
    EXPECT_EQ(ret, FONT_SCOPE_NONE);
}

/**
 * @tc.name: FontManagerFuncTest025
 * @tc.desc: Test CleanupScopeFontsByUser removes all scope fonts for a user
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest025, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    ScopeFontInstallInfo appInfo;
    appInfo.fd = fd;
    appInfo.scope = FONT_SCOPE_APP;
    appInfo.srcPath = "file://test/cleanup_user_app.ttf";
    appInfo.bundleName = "com.example.app";
    appInfo.appIdentifier = "app_cleanup_user";
    appInfo.userId = TEST_USERID;
    manager_->InstallScopeFont(appInfo);
    if (fd >= 0) {
        close(fd);
    }

    fd = open(FONT_PATH.c_str(), O_RDONLY);
    ScopeFontInstallInfo sessionInfo;
    sessionInfo.fd = fd;
    sessionInfo.scope = FONT_SCOPE_SESSION;
    sessionInfo.srcPath = "file://test/cleanup_user_session.ttf";
    sessionInfo.bundleName = "com.example.session";
    sessionInfo.appIdentifier = "session_cleanup_user";
    sessionInfo.userId = TEST_USERID;
    manager_->InstallScopeFont(sessionInfo);
    if (fd >= 0) {
        close(fd);
    }

    manager_->CleanupScopeFontsByUser(TEST_USERID);
    EXPECT_EQ(manager_->GetFontScope("file://test/cleanup_user_app.ttf", TEST_USERID), FONT_SCOPE_NONE);
    EXPECT_EQ(manager_->GetFontScope("file://test/cleanup_user_session.ttf", TEST_USERID), FONT_SCOPE_NONE);
}

/**
 * @tc.name: FontManagerFuncTest026
 * @tc.desc: Test CleanupAppScopeFontsByUser removes only app scope, keeps session
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest026, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    ScopeFontInstallInfo appInfo;
    appInfo.fd = fd;
    appInfo.scope = FONT_SCOPE_APP;
    appInfo.srcPath = "file://test/keep_session_app.ttf";
    appInfo.bundleName = "com.example.app";
    appInfo.appIdentifier = "app_keep_session";
    appInfo.userId = TEST_USERID;
    manager_->InstallScopeFont(appInfo);
    if (fd >= 0) {
        close(fd);
    }

    fd = open(TTC_FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    ScopeFontInstallInfo sessionInfo;
    sessionInfo.fd = fd;
    sessionInfo.scope = FONT_SCOPE_SESSION;
    sessionInfo.srcPath = "file://test/keep_session_sess.ttf";
    sessionInfo.bundleName = "com.example.session";
    sessionInfo.appIdentifier = "session_keep_session";
    sessionInfo.userId = TEST_USERID;
    manager_->InstallScopeFont(sessionInfo);
    if (fd >= 0) {
        close(fd);
    }

    manager_->CleanupAppScopeFontsByUser(TEST_USERID);
    EXPECT_EQ(manager_->GetFontScope("file://test/keep_session_app.ttf", TEST_USERID), FONT_SCOPE_NONE);
    EXPECT_EQ(manager_->GetFontScope("file://test/keep_session_sess.ttf", TEST_USERID), FONT_SCOPE_SESSION);
}

/**
 * @tc.name: FontManagerFuncTest027
 * @tc.desc: Test InstallScopeFont with invalid scope returns ERR_INVALID_PARAM
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest027, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    ScopeFontInstallInfo info;
    info.fd = fd;
    info.scope = 99;
    info.srcPath = "file://test/invalid_scope.ttf";
    info.bundleName = "com.example.app";
    info.appIdentifier = "app_invalid";
    info.userId = TEST_USERID;
    int ret = manager_->InstallScopeFont(info);
    if (fd >= 0) {
        close(fd);
    }
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name: FontManagerFuncTest028
 * @tc.desc: Test CleanupScopeFontsByUser does not call CleanupScopeFontDirs globally
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest028, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    ScopeFontInstallInfo appInfo;
    appInfo.fd = fd;
    appInfo.scope = FONT_SCOPE_APP;
    appInfo.srcPath = "file://test/no_global_cleanup_app.ttf";
    appInfo.bundleName = "com.example.app";
    appInfo.appIdentifier = "app_no_global";
    appInfo.userId = TEST_USERID;
    manager_->InstallScopeFont(appInfo);
    if (fd >= 0) {
        close(fd);
    }

    fd = open(TTC_FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    ScopeFontInstallInfo sessionInfo;
    sessionInfo.fd = fd;
    sessionInfo.scope = FONT_SCOPE_SESSION;
    sessionInfo.srcPath = "file://test/no_global_cleanup_session.ttf";
    sessionInfo.bundleName = "com.example.session";
    sessionInfo.appIdentifier = "session_no_global";
    sessionInfo.userId = TEST_USERID;
    manager_->InstallScopeFont(sessionInfo);
    if (fd >= 0) {
        close(fd);
    }

    manager_->CleanupScopeFontsByUser(TEST_USERID);
    EXPECT_EQ(manager_->GetFontScope("file://test/no_global_cleanup_app.ttf", TEST_USERID), FONT_SCOPE_NONE);
    EXPECT_EQ(manager_->GetFontScope("file://test/no_global_cleanup_session.ttf", TEST_USERID), FONT_SCOPE_NONE);
}

/**
 * @tc.name: FontManagerFuncTest029
 * @tc.desc: Test CleanupScopeFontsByUser with non-existent config (no crash, no error log)
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest029, TestSize.Level1)
{
    manager_->configMap_.clear();
    int ret = manager_->CleanupScopeFontsByUser(TEST_USERID);
    EXPECT_EQ(ret, ERR_OK);
}
} // namespace FontManager
} // namespace Global
} // namespace OHOS