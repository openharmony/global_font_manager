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
#include <atomic>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <thread>
#include <dirent.h>
#include <unistd.h>

#include "font_manager.h"
#include "storage_manager_adapter.h"
#include "font_config.h"
#include "font_manager_utils.h"
#include "font_define.h"
#include "parameters.h"
#include "directory_ex.h"
#include "permission_common.h"

namespace {
const std::string INSTALL_PATH_TEST = "/data/service/el1/100/for-all-app/fonts/";
const std::string TEMP_PATH_TEST = "/data/service/el1/100/for-all-app/fonts/temp/";
const std::string FONT_CONFIG_FILE_TEST = INSTALL_PATH_TEST + "install_fontconfig.json";
const std::string FONT_PATH = "/data/test/NotoSansVai-Regular.ttf";
const std::string FONT_FULL_NAME = "Noto Sans Vai Regular";
const std::string PUBLIC_FONT_DIR = "/data/service/el1/public/for-all-app/fonts/";
const std::string TTC_FONT_PATH = "/data/test/NotoSansCJK-Regular.ttc";
constexpr int32_t TEST_USERID = 100;
constexpr int32_t TEST_TOKEN_ID = 5001;
constexpr int32_t TEST_TOKEN_ID_CLONE = 5002;
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
    FontManagerUtils::DeleteDir(INSTALL_PATH_TEST, false);
    FontManagerUtils::CheckAndInitInstallPath(INSTALL_PATH_TEST);
    FontManagerUtils::CreateFileWithPermission(FONT_CONFIG_FILE_TEST, R"({"fontlist":[],"version":1})");
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
 * @tc.desc: Test FontManager ERR_MAX_FILE_COUNT at max count case
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest006, TestSize.Level1)
{
    int fd = open("/data/test/200install_fontconfig.json", O_RDONLY);
    EXPECT_EQ(FontManagerUtils::CopyFile(fd, FONT_CONFIG_FILE_TEST), true);
    if (fd >= 0) {
        close(fd);
    }
    fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    int ret = manager_->InstallFont(fd, TEST_USERID);
    int32_t maxInstallNum = OHOS::system::GetIntParameter<int32_t>(MAX_INSTALL_NUM_PARAM_KEY,
        DEFAULT_MAX_INSTALL_NUM);
    if (maxInstallNum > DEFAULT_MAX_INSTALL_NUM) {
        EXPECT_EQ(ret, ERR_OK);
    } else {
        EXPECT_EQ(ret, ERR_MAX_FILE_COUNT);
    }
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
 * @tc.desc: Test InstallFont when target file already exists (Rename logic)
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest014, TestSize.Level1)
{
    ASSERT_EQ(FontManagerUtils::CheckAndInitInstallPath(INSTALL_PATH_TEST), true);

    std::string targetFileName = "NotoSansVai-Regular.ttf";
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
            if (name.find("_NotoSansVai-Regular.ttf") != std::string::npos) {
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
 * @tc.desc: Test InstallFont with corrupt config file
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest017, TestSize.Level1)
{
    FontManagerUtils::CheckAndInitInstallPath(INSTALL_PATH_TEST);
    FontManagerUtils::CreateFileWithPermission(FONT_CONFIG_FILE_TEST, "{corrupt json");
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    ASSERT_GE(fd, 0);
    int32_t ret = manager_->InstallFont(fd, TEST_USERID);
    EXPECT_EQ(ret, ERR_INSTALL_FAIL);
    if (fd >= 0) {
        close(fd);
    }
    FontManagerUtils::RemoveFile(FONT_CONFIG_FILE_TEST);
}

/**
 * @tc.name: FontManagerFuncTest018
 * @tc.desc: Test UninstallFont with non-existent font name
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest018, TestSize.Level1)
{
    FontManagerUtils::CheckAndInitInstallPath(INSTALL_PATH_TEST);
    int32_t ret = manager_->UninstallFont("NonExistentFont12345", TEST_USERID);
    EXPECT_EQ(ret, ERR_UNINSTALL_FILE_NOT_EXISTS);
}

/**
 * @tc.name: FontManagerFuncTest019
 * @tc.desc: Test InstallFont with invalid fd
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest019, TestSize.Level1)
{
    FontManagerUtils::CheckAndInitInstallPath(INSTALL_PATH_TEST);
    int32_t ret = manager_->InstallFont(-1, TEST_USERID);
    EXPECT_EQ(ret, ERR_FILE_VERIFY_FAIL);
}

/**
 * @tc.name: FontManagerFuncTest020
 * @tc.desc: Test UninstallFont with corrupt config
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest020, TestSize.Level1)
{
    FontManagerUtils::CheckAndInitInstallPath(INSTALL_PATH_TEST);
    FontManagerUtils::CreateFileWithPermission(FONT_CONFIG_FILE_TEST, "{corrupt");
    int32_t ret = manager_->UninstallFont("SomeFont", TEST_USERID);
    EXPECT_EQ(ret, ERR_UNINSTALL_FILE_NOT_EXISTS);
    FontManagerUtils::RemoveFile(FONT_CONFIG_FILE_TEST);
}

/**
 * @tc.name: FontManagerFuncTest021
 * @tc.desc: Test InstallFont when install path exists but is empty
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest021, TestSize.Level1)
{
    FontManagerUtils::DeleteDir(INSTALL_PATH_TEST, false);
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    int32_t ret = manager_->InstallFont(fd, TEST_USERID);
    EXPECT_EQ(ret, ERR_OK);
    if (fd >= 0) {
        close(fd);
    }
}

/**
 * @tc.name: FontManagerFuncTest022
 * @tc.desc: Test InstallFont when font file already exists at target path
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest022, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    int32_t ret = manager_->InstallFont(fd, TEST_USERID);
    EXPECT_EQ(ret, ERR_OK);
    if (fd >= 0) {
        close(fd);
    }
    fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    ret = manager_->InstallFont(fd, TEST_USERID);
    EXPECT_EQ(ret, ERR_INSTALLED_ALRADY);
    if (fd >= 0) {
        close(fd);
    }
}

/**
 * @tc.name: FontManagerFuncTest023
 * @tc.desc: Test UninstallFont removes font file from disk
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest023, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    int32_t ret = manager_->InstallFont(fd, TEST_USERID);
    EXPECT_EQ(ret, ERR_OK);
    if (fd >= 0) {
        close(fd);
    }
    ret = manager_->UninstallFont(FONT_FULL_NAME, TEST_USERID);
    EXPECT_EQ(ret, ERR_OK);
    ret = manager_->UninstallFont(FONT_FULL_NAME, TEST_USERID);
    EXPECT_EQ(ret, ERR_UNINSTALL_FILE_NOT_EXISTS);
}

/**
 * @tc.name: FontManagerFuncTest024
 * @tc.desc: Test UninstallFont when font file not on disk but in config
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest024, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    int32_t ret = manager_->InstallFont(fd, TEST_USERID);
    EXPECT_EQ(ret, ERR_OK);
    if (fd >= 0) {
        close(fd);
    }
    std::string installedPath = INSTALL_PATH_TEST + "TestFont_Sans.ttf";
    FontManagerUtils::RemoveFile(installedPath);
    ret = manager_->UninstallFont(FONT_FULL_NAME, TEST_USERID);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: FontManagerFuncTest025
 * @tc.desc: Test UninstallFont with TTC font
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest025, TestSize.Level1)
{
    int fd = open(TTC_FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    int32_t ret = manager_->InstallFont(fd, TEST_USERID);
    EXPECT_EQ(ret, ERR_OK);
    if (fd >= 0) {
        close(fd);
    }
    ret = manager_->UninstallFont(TTC_FONT_FULL_NAME[0], TEST_USERID);
    EXPECT_EQ(ret, ERR_OK);
    ret = manager_->UninstallFont(TTC_FONT_FULL_NAME[0], TEST_USERID);
    EXPECT_EQ(ret, ERR_UNINSTALL_FILE_NOT_EXISTS);
}

/**
 * @tc.name: FontManagerFuncTest026
 * @tc.desc: Test InstallFont with existing target file triggers rename
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest026, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    int32_t ret = manager_->InstallFont(fd, TEST_USERID);
    EXPECT_EQ(ret, ERR_OK);
    if (fd >= 0) {
        close(fd);
    }
    FontManagerUtils::RemoveFile(FONT_CONFIG_FILE_TEST);
    FontManagerUtils::CreateFileWithPermission(FONT_CONFIG_FILE_TEST, R"({"fontlist":[],"version":1})");
    manager_->configMap_.clear();
    fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    ret = manager_->InstallFont(fd, TEST_USERID);
    EXPECT_EQ(ret, ERR_OK);
    if (fd >= 0) {
        close(fd);
    }
    bool hasRenamedFile = false;
    DIR *dir = opendir(INSTALL_PATH_TEST.c_str());
    if (dir) {
        struct dirent *ent;
        while ((ent = readdir(dir)) != nullptr) {
            std::string name = ent->d_name;
            if (name.find("_NotoSansVai-Regular.ttf") != std::string::npos) {
                hasRenamedFile = true;
                break;
            }
        }
        closedir(dir);
    }
    EXPECT_TRUE(hasRenamedFile);
}

/**
 * @tc.name: FontManagerFuncTest027
 * @tc.desc: Test UninstallFont with empty font name
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest027, TestSize.Level1)
{
    int32_t ret = manager_->UninstallFont("", TEST_USERID);
    EXPECT_EQ(ret, ERR_UNINSTALL_FILE_NOT_EXISTS);
}

/**
 * @tc.name: FontManagerFuncTest028
 * @tc.desc: Test InstallFont with font having duplicate full name in config
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest028, TestSize.Level1)
{
    std::string configWithFont = R"({"fontlist":[{"fontfullpath":)"
        "\"" + PUBLIC_FONT_DIR + "TestFont_Sans.ttf\","
        R"("fullname":[")" + FONT_FULL_NAME + R"("]}],"version":1})";
    FontManagerUtils::CreateFileWithPermission(FONT_CONFIG_FILE_TEST, configWithFont);
    manager_->configMap_.clear();
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    int32_t ret = manager_->InstallFont(fd, TEST_USERID);
    EXPECT_EQ(ret, ERR_OK);
    if (fd >= 0) {
        close(fd);
    }
}

HWTEST_F(FontManagerTest, FontManagerFuncTest029, TestSize.Level1)
{
    std::vector<std::string> names = {"FontA", "FontB", "FontC"};
    std::string result = manager_->GetFormatFullName(names);
    EXPECT_EQ(result, "FontA,FontB,FontC");
}

HWTEST_F(FontManagerTest, FontManagerFuncTest030, TestSize.Level1)
{
    std::vector<std::string> names = {"FontA"};
    std::string result = manager_->GetFormatFullName(names);
    EXPECT_EQ(result, "FontA");
}

HWTEST_F(FontManagerTest, FontManagerFuncTest031, TestSize.Level1)
{
    std::vector<std::string> names;
    std::string result = manager_->GetFormatFullName(names);
    EXPECT_EQ(result, "");
}

HWTEST_F(FontManagerTest, FontManagerFuncTest032, TestSize.Level1)
{
    std::string installPath = INSTALL_PATH_PREFIX + std::to_string(TEST_USERID) + INSTALL_PATH_SUFFIX;
    std::string path = "/data/service/el1/public/for-all-app/fonts/TestFont.ttf";
    std::string result = manager_->SandBoxPathToRealPath(installPath, path);
    EXPECT_EQ(result, installPath + "TestFont.ttf");
}

HWTEST_F(FontManagerTest, FontManagerFuncTest033, TestSize.Level1)
{
    std::string configPath = INSTALL_PATH_TEST + FONT_CONFIG_FILE;
    FontConfig& config = manager_->SafeGetOrCreateConfig(TEST_USERID, configPath);
    EXPECT_EQ(config.GetInstalledFontsNum(), 0);
    FontConfig& config2 = manager_->SafeGetOrCreateConfig(TEST_USERID, configPath);
    EXPECT_EQ(&config, &config2);
}

HWTEST_F(FontManagerTest, FontManagerFuncTest034, TestSize.Level1)
{
    std::string configPath1 = INSTALL_PATH_TEST + FONT_CONFIG_FILE;
    std::string configPath2 = "/data/service/el1/200/for-all-app/fonts/" + FONT_CONFIG_FILE;
    FontConfig& config1 = manager_->SafeGetOrCreateConfig(100, configPath1);
    FontConfig& config2 = manager_->SafeGetOrCreateConfig(200, configPath2);
    EXPECT_NE(&config1, &config2);
}

HWTEST_F(FontManagerTest, FontManagerFuncTest035, TestSize.Level1)
{
    ASSERT_EQ(FontManagerUtils::CheckAndInitInstallPath(INSTALL_PATH_TEST), true);
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    ASSERT_GE(fd, 0);
    std::string fileName = "TestFont_Sans.ttf";
    std::string tempPath;
    int32_t tempFd = FontManagerUtils::CreatePrivateTempCopy(fd, TEMP_PATH_TEST, tempPath);
    ASSERT_GE(tempFd, 0);
    close(tempFd);
    std::string destPath = manager_->CommitTempFile(tempPath, INSTALL_PATH_TEST, fileName);
    EXPECT_FALSE(destPath.empty());
    if (fd >= 0) {
        close(fd);
    }
    FontManagerUtils::RemoveFile(destPath);
}

HWTEST_F(FontManagerTest, FontManagerFuncTest036, TestSize.Level1)
{
    ASSERT_EQ(FontManagerUtils::CheckAndInitInstallPath(INSTALL_PATH_TEST), true);
    int fd = open("/data/test/nonexistent_12345.ttf", O_RDONLY);
    ASSERT_LT(fd, 0);
    std::string tempPath;
    EXPECT_LT(FontManagerUtils::CreatePrivateTempCopy(fd, TEMP_PATH_TEST, tempPath), 0);
    EXPECT_TRUE(tempPath.empty());
}

HWTEST_F(FontManagerTest, FontManagerFuncTest037, TestSize.Level1)
{
    FontManagerUtils::CheckAndInitInstallPath(INSTALL_PATH_TEST);
    std::string installPath = INSTALL_PATH_PREFIX + std::to_string(TEST_USERID) + INSTALL_PATH_SUFFIX;
    std::string path = "TestFont.ttf";
    std::string result = manager_->SandBoxPathToRealPath(installPath, path);
    EXPECT_EQ(result, installPath + "TestFont.ttf");
}

HWTEST_F(FontManagerTest, FontManagerFuncTest038, TestSize.Level1)
{
    FontManagerUtils::CheckAndInitInstallPath(INSTALL_PATH_TEST);
    std::string configWithFont = R"({"fontlist":[{"fontfullpath":)"
        "\"" + PUBLIC_FONT_DIR + "TestFont_Sans.ttf\","
        R"("fullname":[")" + FONT_FULL_NAME + R"("]}],"version":1})";
    FontManagerUtils::CreateFileWithPermission(FONT_CONFIG_FILE_TEST, configWithFont);
    manager_->configMap_.clear();
    std::string realPath = INSTALL_PATH_TEST + "TestFont_Sans.ttf";
    FontManagerUtils::CreateFileWithPermission(realPath, "");
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    int32_t ret = manager_->InstallFont(fd, TEST_USERID);
    EXPECT_EQ(ret, ERR_INSTALLED_ALRADY);
    if (fd >= 0) {
        close(fd);
    }
}

HWTEST_F(FontManagerTest, FontManagerFuncTest039, TestSize.Level1)
{
    FontManagerUtils::CheckAndInitInstallPath(INSTALL_PATH_TEST);
    std::string configWithFont = R"({"fontlist":[{"fontfullpath":)"
        "\"" + PUBLIC_FONT_DIR + "GhostFont.ttf\","
        R"("fullname":["GhostFont"]}],"version":1})";
    FontManagerUtils::CreateFileWithPermission(FONT_CONFIG_FILE_TEST, configWithFont);
    manager_->configMap_.clear();
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    int32_t ret = manager_->InstallFont(fd, TEST_USERID);
    EXPECT_EQ(ret, ERR_OK);
    if (fd >= 0) {
        close(fd);
    }
}

/**
 * @tc.name: FontManagerFuncTest040
 * @tc.desc: Test InstallScopeFont with app scope
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest040, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    ScopeFontInstallInfo info;
    info.fd = fd;
    info.scope = FONT_SCOPE_APP;
    info.srcPath = "file://test/app_font1.ttf";
    info.bundleName = "com.example.app";
    info.appIdentifier = "app_5001";
    info.userId = TEST_USERID;
    int ret = manager_->InstallScopeFont(info);
    if (fd >= 0) {
        close(fd);
    }
    EXPECT_EQ(ret, ERR_OK);

    auto scopeVal = manager_->GetFontScope("file://test/app_font1.ttf", "com.example.app",
        TEST_TOKEN_ID, TEST_USERID);
    EXPECT_EQ(scopeVal, FONT_SCOPE_APP);
}

/**
 * @tc.name: FontManagerFuncTest041
 * @tc.desc: Test InstallScopeFont duplicate srcPath returns ERR_INSTALLED_ALRADY
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest041, TestSize.Level1)
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
 * @tc.name: FontManagerFuncTest042
 * @tc.desc: Test InstallScopeFont duplicate font name returns ERR_INSTALLED_ALRADY
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest042, TestSize.Level1)
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
 * @tc.name: FontManagerFuncTest043
 * @tc.desc: Test UninstallScopeFont by srcPath
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest043, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    ScopeFontInstallInfo info;
    info.fd = fd;
    info.scope = FONT_SCOPE_APP;
    info.srcPath = "file://test/uninstall_scope.ttf";
    info.bundleName = "com.example.app";
    info.appIdentifier = "app_5001";
    info.userId = TEST_USERID;
    manager_->InstallScopeFont(info);
    if (fd >= 0) {
        close(fd);
    }

    int ret;
    manager_->UninstallScopeFont("file://test/uninstall_scope.ttf", "com.example.app",
        TEST_TOKEN_ID, TEST_USERID);
    ret = manager_->GetFontScope("file://test/uninstall_scope.ttf", "com.example.app",
        TEST_TOKEN_ID, TEST_USERID);
    EXPECT_EQ(ret, ERR_SCOPE_FONT_NOT_FOUND);
}

/**
 * @tc.name: FontManagerFuncTest044
 * @tc.desc: Test UninstallScopeFont with non-existent srcPath
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest044, TestSize.Level1)
{
    int ret = manager_->UninstallScopeFont("file://test/non_existent.ttf", "com.example.app",
        TEST_TOKEN_ID, TEST_USERID);
    EXPECT_EQ(ret, ERR_SCOPE_FONT_NOT_FOUND);
}

/**
 * @tc.name: FontManagerFuncTest045
 * @tc.desc: Test GetFontScope returns ERR_SCOPE_FONT_NOT_FOUND for non-existent
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest045, TestSize.Level1)
{
    int ret = manager_->GetFontScope("file://test/not_installed.ttf", "com.example.app",
        TEST_TOKEN_ID, TEST_USERID);
    EXPECT_EQ(ret, ERR_SCOPE_FONT_NOT_FOUND);
    // Empty srcPath should also return ERR_SCOPE_FONT_NOT_FOUND
    ret = manager_->GetFontScope("", "com.example.app", TEST_TOKEN_ID, TEST_USERID);
    EXPECT_EQ(ret, ERR_SCOPE_FONT_NOT_FOUND);
    ret = manager_->GetFontScope("", "", TEST_TOKEN_ID, TEST_USERID);
    EXPECT_EQ(ret, ERR_SCOPE_FONT_NOT_FOUND);
}

/**
 * @tc.name: FontManagerFuncTest046
 * @tc.desc: Test InstallScopeFont with session scope
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest046, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    ScopeFontInstallInfo info;
    info.fd = fd;
    info.scope = FONT_SCOPE_SESSION;
    info.srcPath = "file://test/session_font.ttf";
    info.bundleName = "com.example.session";
    info.appIdentifier = "session_5001";
    info.userId = TEST_USERID;
    int ret = manager_->InstallScopeFont(info);
    if (fd >= 0) {
        close(fd);
    }
    EXPECT_EQ(ret, ERR_OK);

    auto scopeVal = manager_->GetFontScope("file://test/session_font.ttf", "com.example.session",
        TEST_TOKEN_ID, TEST_USERID);
    EXPECT_EQ(scopeVal, FONT_SCOPE_SESSION);
}

/**
 * @tc.name: FontManagerFuncTest047
 * @tc.desc: Test CleanupAppScopeFonts removes all fonts for an app
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest047, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    ScopeFontInstallInfo info;
    info.fd = fd;
    info.scope = FONT_SCOPE_APP;
    info.srcPath = "file://test/cleanup_app.ttf";
    info.bundleName = "com.example.cleanup";
    info.appIdentifier = "app_5001";
    info.userId = TEST_USERID;
    manager_->InstallScopeFont(info);
    if (fd >= 0) {
        close(fd);
    }

    manager_->CleanupAppScopeFonts("app_5001", TEST_USERID);
    int ret = manager_->GetFontScope("file://test/cleanup_app.ttf", "com.example.cleanup",
        TEST_TOKEN_ID, TEST_USERID);
    EXPECT_EQ(ret, ERR_SCOPE_FONT_NOT_FOUND);
}

/**
 * @tc.name: FontManagerFuncTest048
 * @tc.desc: Test CleanupScopeFontsByUser removes all scope fonts for a user
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest048, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    ScopeFontInstallInfo appInfo;
    appInfo.fd = fd;
    appInfo.scope = FONT_SCOPE_APP;
    appInfo.srcPath = "file://test/cleanup_user_app.ttf";
    appInfo.bundleName = "com.example.app";
    appInfo.appIdentifier = "app_5001";
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
    sessionInfo.appIdentifier = "session_5001";
    sessionInfo.userId = TEST_USERID;
    manager_->InstallScopeFont(sessionInfo);
    if (fd >= 0) {
        close(fd);
    }

    manager_->CleanupScopeFontsByUser(TEST_USERID);
    EXPECT_EQ(manager_->GetFontScope("file://test/cleanup_user_app.ttf",
        "com.example.app", TEST_TOKEN_ID, TEST_USERID), ERR_SCOPE_FONT_NOT_FOUND);
    EXPECT_EQ(manager_->GetFontScope("file://test/cleanup_user_session.ttf",
        "com.example.session", TEST_TOKEN_ID, TEST_USERID), ERR_SCOPE_FONT_NOT_FOUND);
}

/**
 * @tc.name: FontManagerFuncTest049
 * @tc.desc: Test CleanupAppScopeFontsByUser removes only app scope, keeps session
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest049, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    ScopeFontInstallInfo appInfo;
    appInfo.fd = fd;
    appInfo.scope = FONT_SCOPE_APP;
    appInfo.srcPath = "file://test/keep_session_app.ttf";
    appInfo.bundleName = "com.example.app";
    appInfo.appIdentifier = "app_5001";
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
    sessionInfo.appIdentifier = "session_5001";
    sessionInfo.userId = TEST_USERID;
    manager_->InstallScopeFont(sessionInfo);
    if (fd >= 0) {
        close(fd);
    }

    manager_->CleanupAppScopeFontsByUser(TEST_USERID);
    EXPECT_EQ(manager_->GetFontScope("file://test/keep_session_app.ttf",
        "com.example.app", TEST_TOKEN_ID, TEST_USERID), ERR_SCOPE_FONT_NOT_FOUND);
    EXPECT_EQ(manager_->GetFontScope("file://test/keep_session_sess.ttf",
        "com.example.session", TEST_TOKEN_ID, TEST_USERID), FONT_SCOPE_SESSION);
}

/**
 * @tc.name: FontManagerFuncTest050
 * @tc.desc: Test InstallScopeFont with invalid scope returns ERR_INVALID_PARAM
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest050, TestSize.Level1)
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
 * @tc.name: FontManagerFuncTest051
 * @tc.desc: Test CleanupScopeFontsByUser does not call CleanupScopeFontDirs globally
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest051, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    ScopeFontInstallInfo appInfo;
    appInfo.fd = fd;
    appInfo.scope = FONT_SCOPE_APP;
    appInfo.srcPath = "file://test/no_global_cleanup_app.ttf";
    appInfo.bundleName = "com.example.app";
    appInfo.appIdentifier = "app_5001";
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
    sessionInfo.appIdentifier = "session_5001";
    sessionInfo.userId = TEST_USERID;
    manager_->InstallScopeFont(sessionInfo);
    if (fd >= 0) {
        close(fd);
    }

    manager_->CleanupScopeFontsByUser(TEST_USERID);
    EXPECT_EQ(manager_->GetFontScope("file://test/no_global_cleanup_app.ttf",
        "com.example.app", TEST_TOKEN_ID, TEST_USERID), ERR_SCOPE_FONT_NOT_FOUND);
    EXPECT_EQ(manager_->GetFontScope("file://test/no_global_cleanup_session.ttf",
        "com.example.session", TEST_TOKEN_ID, TEST_USERID), ERR_SCOPE_FONT_NOT_FOUND);
}

/**
 * @tc.name: FontManagerFuncTest052
 * @tc.desc: Test CleanupScopeFontsByUser with non-existent config (no crash, no error log)
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest052, TestSize.Level1)
{
    manager_->configMap_.clear();
    int ret = manager_->CleanupScopeFontsByUser(TEST_USERID);
    EXPECT_EQ(ret, ERR_OK);
}
/**
 * @tc.name: FontManagerFuncTest053
 * @tc.desc: Test GetFontScope with bundleName mismatch returns ERR_SCOPE_FONT_NOT_FOUND
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest053, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    ScopeFontInstallInfo info;
    info.fd = fd;
    info.scope = FONT_SCOPE_APP;
    info.srcPath = "file://test/mismatch_font.ttf";
    info.bundleName = "com.example.owner";
    info.appIdentifier = "app_5001";
    info.userId = TEST_USERID;
    int ret = manager_->InstallScopeFont(info);
    if (fd >= 0) {
        close(fd);
    }
    EXPECT_EQ(ret, ERR_OK);

    ret = manager_->GetFontScope("file://test/mismatch_font.ttf", "com.example.other",
        TEST_TOKEN_ID, TEST_USERID);
    EXPECT_EQ(ret, ERR_SCOPE_FONT_NOT_FOUND);
}

/**
 * @tc.name: FontManagerFuncTest054
 * @tc.desc: Test UninstallScopeFont with empty srcPath returns ERR_SCOPE_FONT_NOT_FOUND
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest054, TestSize.Level1)
{
    int ret = manager_->UninstallScopeFont("", "com.example.app", TEST_TOKEN_ID, TEST_USERID);
    EXPECT_EQ(ret, ERR_SCOPE_FONT_NOT_FOUND);
}

/**
 * @tc.name: FontManagerFuncTest055
 * @tc.desc: Test UninstallScopeFont with bundleName mismatch returns ERR_SCOPE_FONT_NOT_FOUND
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest055, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    ScopeFontInstallInfo info;
    info.fd = fd;
    info.scope = FONT_SCOPE_APP;
    info.srcPath = "file://test/uninstall_mismatch.ttf";
    info.bundleName = "com.example.owner";
    info.appIdentifier = "app_5001";
    info.userId = TEST_USERID;
    manager_->InstallScopeFont(info);
    if (fd >= 0) {
        close(fd);
    }

    int ret = manager_->UninstallScopeFont(
        "file://test/uninstall_mismatch.ttf", "com.example.attacker", TEST_TOKEN_ID, TEST_USERID);
    EXPECT_EQ(ret, ERR_SCOPE_FONT_NOT_FOUND);
}

/**
 * @tc.name: FontManagerFuncTest056
 * @tc.desc: Test CleanupAppScopeFonts with non-existent appIdentifier (empty records)
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest056, TestSize.Level1)
{
    int ret = manager_->CleanupAppScopeFonts("app_nonexistent", TEST_USERID);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: FontManagerFuncTest057
 * @tc.desc: Test CleanupAppScopeFontsByUser with empty config (no records)
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest057, TestSize.Level1)
{
    manager_->configMap_.clear();
    int ret = manager_->CleanupAppScopeFontsByUser(TEST_USERID);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: FontManagerFuncTest058
 * @tc.desc: Test InstallScopeFont with invalid fd returns ERR_FILE_VERIFY_FAIL
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest058, TestSize.Level1)
{
    ScopeFontInstallInfo info;
    info.fd = -1;
    info.scope = FONT_SCOPE_APP;
    info.srcPath = "file://test/invalid_fd.ttf";
    info.bundleName = "com.example.app";
    info.appIdentifier = "app_invalid_fd";
    info.userId = TEST_USERID;
    int ret = manager_->InstallScopeFont(info);
    EXPECT_EQ(ret, ERR_FILE_VERIFY_FAIL);
}

/**
 * @tc.name: FontManagerFuncTest059
 * @tc.desc: Test ValidateAndResolveRealPath with valid path
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest059, TestSize.Level1)
{
    std::string installPath = INSTALL_PATH_PREFIX + std::to_string(TEST_USERID) + INSTALL_PATH_SUFFIX;
    std::string path = std::string(INSTALL_PATH_APP) + "ValidFont.ttf";
    std::string result = manager_->ValidateAndResolveRealPath(installPath, path);
    EXPECT_FALSE(result.empty());
    EXPECT_EQ(result, installPath + "ValidFont.ttf");
}

/**
 * @tc.name: FontManagerFuncTest060
 * @tc.desc: Test ValidateAndResolveRealPath rejects ".." traversal
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest060, TestSize.Level1)
{
    std::string installPath = INSTALL_PATH_PREFIX + std::to_string(TEST_USERID) + INSTALL_PATH_SUFFIX;
    std::string path = std::string(INSTALL_PATH_APP) + "..";
    std::string result = manager_->ValidateAndResolveRealPath(installPath, path);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: FontManagerFuncTest061
 * @tc.desc: Test ValidateAndResolveRealPath rejects "." traversal
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest061, TestSize.Level1)
{
    std::string installPath = INSTALL_PATH_PREFIX + std::to_string(TEST_USERID) + INSTALL_PATH_SUFFIX;
    std::string path = std::string(INSTALL_PATH_APP) + ".";
    std::string result = manager_->ValidateAndResolveRealPath(installPath, path);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: FontManagerFuncTest062
 * @tc.desc: Test ValidateAndResolveRealPath rejects empty path
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest062, TestSize.Level1)
{
    std::string installPath = INSTALL_PATH_PREFIX + std::to_string(TEST_USERID) + INSTALL_PATH_SUFFIX;
    std::string result = manager_->ValidateAndResolveRealPath(installPath, "");
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: FontManagerFuncTest063
 * @tc.desc: Test ValidateAndResolveRealPath rejects sibling dir escape (fonts_evil)
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest063, TestSize.Level1)
{
    std::string installPath = INSTALL_PATH_PREFIX + std::to_string(TEST_USERID) + INSTALL_PATH_SUFFIX;
    std::string path = std::string(INSTALL_PATH_APP) + "../fonts_evil/payload.ttf";
    std::string result = manager_->ValidateAndResolveRealPath(installPath, path);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: FontManagerFuncTest064
 * @tc.desc: Test CommitTempFile produces unique destPath with thread tag
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest064, TestSize.Level1)
{
    ASSERT_EQ(FontManagerUtils::CheckAndInitInstallPath(INSTALL_PATH_TEST), true);
    int fd1 = open(FONT_PATH.c_str(), O_RDONLY);
    ASSERT_GE(fd1, 0);
    std::string tempPath1;
    int32_t tempFd1 = FontManagerUtils::CreatePrivateTempCopy(fd1, TEMP_PATH_TEST, tempPath1);
    ASSERT_GE(tempFd1, 0);
    close(tempFd1);
    std::string destPath1 = manager_->CommitTempFile(tempPath1, INSTALL_PATH_TEST, "UniqueA.ttf");
    EXPECT_FALSE(destPath1.empty());
    EXPECT_NE(destPath1.find("UniqueA.ttf"), std::string::npos);
    EXPECT_NE(destPath1.find("_"), std::string::npos);
    close(fd1);

    int fd2 = open(FONT_PATH.c_str(), O_RDONLY);
    ASSERT_GE(fd2, 0);
    std::string tempPath2;
    int32_t tempFd2 = FontManagerUtils::CreatePrivateTempCopy(fd2, TEMP_PATH_TEST, tempPath2);
    ASSERT_GE(tempFd2, 0);
    close(tempFd2);
    std::string destPath2 = manager_->CommitTempFile(tempPath2, INSTALL_PATH_TEST, "UniqueB.ttf");
    EXPECT_FALSE(destPath2.empty());
    EXPECT_NE(destPath2.find("UniqueB.ttf"), std::string::npos);
    EXPECT_NE(destPath1, destPath2);
    close(fd2);

    FontManagerUtils::RemoveFile(destPath1);
    FontManagerUtils::RemoveFile(destPath2);
}

/**
 * @tc.name: FontManagerFuncTest065
 * @tc.desc: Test InstallFont rolls back copied file on max count failure
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest065, TestSize.Level1)
{
    int fd = open("/data/test/200install_fontconfig.json", O_RDONLY);
    EXPECT_EQ(FontManagerUtils::CopyFile(fd, FONT_CONFIG_FILE_TEST), true);
    if (fd >= 0) {
        close(fd);
    }
    fd = open(FONT_PATH.c_str(), O_RDONLY);
    ASSERT_GE(fd, 0);
    int32_t maxInstallNum = OHOS::system::GetIntParameter<int32_t>(MAX_INSTALL_NUM_PARAM_KEY,
        DEFAULT_MAX_INSTALL_NUM);
    int ret = manager_->InstallFont(fd, TEST_USERID);
    if (fd >= 0) {
        close(fd);
    }
    if (maxInstallNum > DEFAULT_MAX_INSTALL_NUM) {
        EXPECT_EQ(ret, ERR_OK);
    } else {
        EXPECT_EQ(ret, ERR_MAX_FILE_COUNT);
    }
    std::string tempDir = INSTALL_PATH_TEST + TEMP_FILE;
    std::vector<std::string> tempFiles;
    OHOS::GetDirFiles(tempDir, tempFiles);
    EXPECT_EQ(tempFiles.size(), 0u);
}

/**
 * @tc.name: FontManagerFuncTest066
 * @tc.desc: App clone scenario: GetFontScope with same bundleName but different tokenId
 *           (different appIdentifier) returns ERR_SCOPE_FONT_NOT_FOUND
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest066, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    ScopeFontInstallInfo info;
    info.fd = fd;
    info.scope = FONT_SCOPE_APP;
    info.srcPath = "file://test/clone_query.ttf";
    info.bundleName = "com.example.app";
    info.appIdentifier = "app_5001";
    info.userId = TEST_USERID;
    int ret = manager_->InstallScopeFont(info);
    if (fd >= 0) {
        close(fd);
    }
    EXPECT_EQ(ret, ERR_OK);

    // Clone app: same bundleName, different tokenId (app_5002) -> appIdentifier mismatch
    ret = manager_->GetFontScope("file://test/clone_query.ttf", "com.example.app",
        TEST_TOKEN_ID_CLONE, TEST_USERID);
    EXPECT_EQ(ret, ERR_SCOPE_FONT_NOT_FOUND);

    // Original app can still query successfully
    ret = manager_->GetFontScope("file://test/clone_query.ttf", "com.example.app",
        TEST_TOKEN_ID, TEST_USERID);
    EXPECT_EQ(ret, FONT_SCOPE_APP);
}

/**
 * @tc.name: FontManagerFuncTest067
 * @tc.desc: App clone scenario: UninstallScopeFont with same bundleName but different tokenId
 *           (different appIdentifier) returns ERR_SCOPE_FONT_NOT_FOUND
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest067, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    ScopeFontInstallInfo info;
    info.fd = fd;
    info.scope = FONT_SCOPE_APP;
    info.srcPath = "file://test/clone_uninstall.ttf";
    info.bundleName = "com.example.app";
    info.appIdentifier = "app_5001";
    info.userId = TEST_USERID;
    int ret = manager_->InstallScopeFont(info);
    if (fd >= 0) {
        close(fd);
    }
    EXPECT_EQ(ret, ERR_OK);

    // Clone app: same bundleName, different tokenId -> appIdentifier mismatch -> blocked
    ret = manager_->UninstallScopeFont("file://test/clone_uninstall.ttf", "com.example.app",
        TEST_TOKEN_ID_CLONE, TEST_USERID);
    EXPECT_EQ(ret, ERR_SCOPE_FONT_NOT_FOUND);

    // Font should still exist (clone failed to uninstall)
    ret = manager_->GetFontScope("file://test/clone_uninstall.ttf", "com.example.app",
        TEST_TOKEN_ID, TEST_USERID);
    EXPECT_EQ(ret, FONT_SCOPE_APP);
}

/**
 * @tc.name: FontManagerFuncTest068
 * @tc.desc: Test UninstallFont with scope font name returns ERR_UNINSTALL_FILE_NOT_EXISTS
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest068, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    ScopeFontInstallInfo info;
    info.fd = fd;
    info.scope = FONT_SCOPE_APP;
    info.srcPath = "file://test/scope_uninstall_via_user.ttf";
    info.bundleName = "com.example.app";
    info.appIdentifier = "app_5001";
    info.userId = TEST_USERID;
    EXPECT_EQ(manager_->InstallScopeFont(info), ERR_OK);
    if (fd >= 0) {
        close(fd);
    }
    // UninstallFont with the scope font's full name should fail (scope >= 0)
    int32_t ret = manager_->UninstallFont(FONT_FULL_NAME, TEST_USERID);
    EXPECT_EQ(ret, ERR_UNINSTALL_FILE_NOT_EXISTS);
}

/**
 * @tc.name: FontManagerFuncTest069
 * @tc.desc: Test UninstallScopeFont with file not on disk
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest069, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    ScopeFontInstallInfo info;
    info.fd = fd;
    info.scope = FONT_SCOPE_APP;
    info.srcPath = "file://test/uninstall_no_file.ttf";
    info.bundleName = "com.example.app";
    info.appIdentifier = "app_5001";
    info.userId = TEST_USERID;
    EXPECT_EQ(manager_->InstallScopeFont(info), ERR_OK);
    if (fd >= 0) {
        close(fd);
    }
    // Remove the font file from disk but keep config record
    auto& fontConfig = manager_->SafeGetOrCreateConfig(TEST_USERID,
        INSTALL_PATH_TEST + FONT_CONFIG_FILE);
    auto record = fontConfig.GetFontRecordByUrl("file://test/uninstall_no_file.ttf");
    ASSERT_TRUE(record.has_value());
    std::string realPath = INSTALL_PATH_TEST + "app_5001/" + FontManagerUtils::GetFileName(record->fontPath);
    FontManagerUtils::RemoveFile(realPath);
    // Uninstall should still succeed (file not on disk, but record exists)
    int32_t ret = manager_->UninstallScopeFont("file://test/uninstall_no_file.ttf",
        "com.example.app", TEST_TOKEN_ID, TEST_USERID);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: FontManagerFuncTest070
 * @tc.desc: Test GetAppInstallPath returns correct path
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest070, TestSize.Level1)
{
    std::string result = manager_->GetAppInstallPath(TEST_USERID, "app_5001");
    std::string expected = INSTALL_PATH_PREFIX + std::to_string(TEST_USERID) + INSTALL_PATH_SUFFIX + "app_5001/";
    EXPECT_EQ(result, expected);
}

/**
 * @tc.name: FontManagerFuncTest071
 * @tc.desc: Test CleanupAppScopeFonts with multiple records
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest071, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    ScopeFontInstallInfo info1;
    info1.fd = fd;
    info1.scope = FONT_SCOPE_APP;
    info1.srcPath = "file://test/cleanup_multi_1.ttf";
    info1.bundleName = "com.example.multi";
    info1.appIdentifier = "app_multi";
    info1.userId = TEST_USERID;
    EXPECT_EQ(manager_->InstallScopeFont(info1), ERR_OK);
    if (fd >= 0) {
        close(fd);
    }
    fd = open(TTC_FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    ScopeFontInstallInfo info2;
    info2.fd = fd;
    info2.scope = FONT_SCOPE_APP;
    info2.srcPath = "file://test/cleanup_multi_2.ttf";
    info2.bundleName = "com.example.multi";
    info2.appIdentifier = "app_multi";
    info2.userId = TEST_USERID;
    EXPECT_EQ(manager_->InstallScopeFont(info2), ERR_OK);
    if (fd >= 0) {
        close(fd);
    }
    EXPECT_EQ(manager_->CleanupAppScopeFonts("app_multi", TEST_USERID), ERR_OK);
    EXPECT_EQ(manager_->GetFontScope("file://test/cleanup_multi_1.ttf",
        "com.example.multi", TEST_TOKEN_ID, TEST_USERID), ERR_SCOPE_FONT_NOT_FOUND);
    EXPECT_EQ(manager_->GetFontScope("file://test/cleanup_multi_2.ttf",
        "com.example.multi", TEST_TOKEN_ID, TEST_USERID), ERR_SCOPE_FONT_NOT_FOUND);
}

/**
 * @tc.name: FontManagerFuncTest072
 * @tc.desc: Test InstallScopeFont with CheckAndUpdateFontRecord fail (corrupt config)
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest072, TestSize.Level1)
{
    FontManagerUtils::CheckAndInitInstallPath(INSTALL_PATH_TEST);
    FontManagerUtils::CreateFileWithPermission(FONT_CONFIG_FILE_TEST, "{corrupt json");
    manager_->configMap_.clear();
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    ScopeFontInstallInfo info;
    info.fd = fd;
    info.scope = FONT_SCOPE_APP;
    info.srcPath = "file://test/corrupt_config.ttf";
    info.bundleName = "com.example.app";
    info.appIdentifier = "app_5001";
    info.userId = TEST_USERID;
    int32_t ret = manager_->InstallScopeFont(info);
    if (fd >= 0) {
        close(fd);
    }
    EXPECT_EQ(ret, ERR_SYSTEM_ERROR);
    FontManagerUtils::RemoveFile(FONT_CONFIG_FILE_TEST);
}

/**
 * @tc.name: FontManagerFuncTest073
 * @tc.desc: Test ValidateAndResolveRealPath rejects path with backslash
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest073, TestSize.Level1)
{
    std::string installPath = INSTALL_PATH_PREFIX + std::to_string(TEST_USERID) + INSTALL_PATH_SUFFIX;
    std::string path = std::string(INSTALL_PATH_APP) + "font\\name.ttf";
    std::string result = manager_->ValidateAndResolveRealPath(installPath, path);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: FontManagerFuncTest074
 * @tc.desc: Test InstallScopeFont with session scope and GetFontScope returns session
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest074, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    ScopeFontInstallInfo info;
    info.fd = fd;
    info.scope = FONT_SCOPE_SESSION;
    info.srcPath = "file://test/session_scope_query.ttf";
    info.bundleName = "com.example.session";
    info.appIdentifier = "session_5001";
    info.userId = TEST_USERID;
    EXPECT_EQ(manager_->InstallScopeFont(info), ERR_OK);
    if (fd >= 0) {
        close(fd);
    }
    EXPECT_EQ(manager_->GetFontScope("file://test/session_scope_query.ttf",
        "com.example.session", TEST_TOKEN_ID, TEST_USERID), FONT_SCOPE_SESSION);
}

/**
 * @tc.name: FontManagerFuncTest076
 * @tc.desc: Test CreatePrivateTempCopy with invalid fd
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest076, TestSize.Level1)
{
    FontManagerUtils::CheckAndInitInstallPath(INSTALL_PATH_TEST);
    std::string tempPath;
    EXPECT_LT(FontManagerUtils::CreatePrivateTempCopy(-1, TEMP_PATH_TEST, tempPath), 0);
    EXPECT_TRUE(tempPath.empty());
}

/**
 * @tc.name: FontManagerFuncTest077
 * @tc.desc: Test GetMaxInstallNum returns valid value
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest077, TestSize.Level1)
{
    int32_t maxNum = manager_->GetMaxInstallNum();
    EXPECT_GE(maxNum, DEFAULT_MAX_INSTALL_NUM);
    // Verify maxNum is a reasonable value (not excessively large)
    EXPECT_LE(maxNum, 10000);
}

/**
 * @tc.name: FontManagerFuncTest078
 * @tc.desc: Test CleanupScopeFontsByUser with records present
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest078, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    ScopeFontInstallInfo info;
    info.fd = fd;
    info.scope = FONT_SCOPE_APP;
    info.srcPath = "file://test/cleanup_user_records.ttf";
    info.bundleName = "com.example.app";
    info.appIdentifier = "app_5001";
    info.userId = TEST_USERID;
    EXPECT_EQ(manager_->InstallScopeFont(info), ERR_OK);
    if (fd >= 0) {
        close(fd);
    }
    EXPECT_EQ(manager_->CleanupScopeFontsByUser(TEST_USERID), ERR_OK);
    EXPECT_EQ(manager_->GetFontScope("file://test/cleanup_user_records.ttf",
        "com.example.app", TEST_TOKEN_ID, TEST_USERID), ERR_SCOPE_FONT_NOT_FOUND);
}

/**
 * @tc.name: FontManagerFuncTest079
 * @tc.desc: Test CleanupAppScopeFontsByUser with records present
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest079, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    ScopeFontInstallInfo info;
    info.fd = fd;
    info.scope = FONT_SCOPE_APP;
    info.srcPath = "file://test/cleanup_app_user_records.ttf";
    info.bundleName = "com.example.app";
    info.appIdentifier = "app_5001";
    info.userId = TEST_USERID;
    EXPECT_EQ(manager_->InstallScopeFont(info), ERR_OK);
    if (fd >= 0) {
        close(fd);
    }
    EXPECT_EQ(manager_->CleanupAppScopeFontsByUser(TEST_USERID), ERR_OK);
    EXPECT_EQ(manager_->GetFontScope("file://test/cleanup_app_user_records.ttf",
        "com.example.app", TEST_TOKEN_ID, TEST_USERID), ERR_SCOPE_FONT_NOT_FOUND);
}

/**
 * @tc.name: FontManagerFuncTest080
 * @tc.desc: Test InstallFont with corrupt config returns ERR_INSTALL_FAIL
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest080, TestSize.Level1)
{
    FontManagerUtils::CheckAndInitInstallPath(INSTALL_PATH_TEST);
    FontManagerUtils::CreateFileWithPermission(FONT_CONFIG_FILE_TEST, "not_valid_json");
    manager_->configMap_.clear();
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    ASSERT_GE(fd, 0);
    int32_t ret = manager_->InstallFont(fd, TEST_USERID);
    if (fd >= 0) {
        close(fd);
    }
    EXPECT_EQ(ret, ERR_INSTALL_FAIL);
    FontManagerUtils::RemoveFile(FONT_CONFIG_FILE_TEST);
}

/**
 * @tc.name: FontManagerFuncTest081
 * @tc.desc: Test InstallScopeFont with session scope and uninstall
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest081, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    ScopeFontInstallInfo info;
    info.fd = fd;
    info.scope = FONT_SCOPE_SESSION;
    info.srcPath = "file://test/session_uninstall.ttf";
    info.bundleName = "com.example.session";
    info.appIdentifier = "session_5001";
    info.userId = TEST_USERID;
    EXPECT_EQ(manager_->InstallScopeFont(info), ERR_OK);
    if (fd >= 0) {
        close(fd);
    }
    EXPECT_EQ(manager_->UninstallScopeFont("file://test/session_uninstall.ttf",
        "com.example.session", TEST_TOKEN_ID, TEST_USERID), ERR_OK);
    EXPECT_EQ(manager_->GetFontScope("file://test/session_uninstall.ttf",
        "com.example.session", TEST_TOKEN_ID, TEST_USERID), ERR_SCOPE_FONT_NOT_FOUND);
}

/**
 * @tc.name: FontManagerFuncTest082
 * @tc.desc: Test GetFontScope with appIdentifier mismatch (session scope)
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest082, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    ScopeFontInstallInfo info;
    info.fd = fd;
    info.scope = FONT_SCOPE_SESSION;
    info.srcPath = "file://test/session_mismatch.ttf";
    info.bundleName = "com.example.session";
    info.appIdentifier = "session_5001";
    info.userId = TEST_USERID;
    EXPECT_EQ(manager_->InstallScopeFont(info), ERR_OK);
    if (fd >= 0) {
        close(fd);
    }
    // Query with correct bundleName but different tokenId -> appIdentifier mismatch
    int32_t ret = manager_->GetFontScope("file://test/session_mismatch.ttf",
        "com.example.session", TEST_TOKEN_ID_CLONE, TEST_USERID);
    EXPECT_EQ(ret, ERR_SCOPE_FONT_NOT_FOUND);
}

/**
 * @tc.name: FontManagerFuncTest083
 * @tc.desc: Test UninstallFont with font name found but path empty in config
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest083, TestSize.Level1)
{
    std::string configWithEmptyPath = R"({"fontlist": [{"fontfullpath": "", "fullname": [")" +
        FONT_FULL_NAME + R"("]}], "version": 1})";
    FontManagerUtils::CreateFileWithPermission(FONT_CONFIG_FILE_TEST, configWithEmptyPath);
    manager_->configMap_.clear();
    int32_t ret = manager_->UninstallFont(FONT_FULL_NAME, TEST_USERID);
    EXPECT_EQ(ret, ERR_UNINSTALL_FILE_NOT_EXISTS);
}

/**
 * @tc.name: FontManagerFuncTest084
 * @tc.desc: Test SandBoxPathToRealPath with non-INSTALL_PATH_APP path
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest084, TestSize.Level1)
{
    std::string installPath = INSTALL_PATH_PREFIX + std::to_string(TEST_USERID) + INSTALL_PATH_SUFFIX;
    std::string path = "/some/other/path/font.ttf";
    std::string result = manager_->SandBoxPathToRealPath(installPath, path);
    EXPECT_EQ(result, installPath + "font.ttf");
}

/**
 * @tc.name: FontManagerFuncTest085
 * @tc.desc: InstallFont must not crash when the caller truncates the fd during install
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest085, TestSize.Level1)
{
    // ftruncate needs a writable fd and must not corrupt the shared /data/test font.
    std::string sourceCopyPath = TEMP_PATH_TEST + "truncate_source.ttf";
    int sourceFd = open(FONT_PATH.c_str(), O_RDONLY);
    ASSERT_GE(sourceFd, 0);
    bool copied = FontManagerUtils::CopyFile(sourceFd, sourceCopyPath);
    close(sourceFd);
    ASSERT_TRUE(copied);

    int fd = open(sourceCopyPath.c_str(), O_RDWR);
    ASSERT_GE(fd, 0);
    std::atomic<bool> truncated {false};
    std::thread truncator([fd, &truncated]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        truncated = (ftruncate(fd, 1024) == 0);
    });
    int32_t ret = manager_->InstallFont(fd, TEST_USERID);
    truncator.join();
    close(fd);
    FontManagerUtils::RemoveFile(sourceCopyPath);

    EXPECT_TRUE(truncated.load());
    // The snapshot may win (OK) or be cut short (copy/verify failure).
    EXPECT_TRUE(ret == ERR_OK || ret == ERR_COPY_FAIL || ret == ERR_FILE_VERIFY_FAIL);
}

/**
 * @tc.name: FontManagerFuncTest086
 * @tc.desc: A font that fails verification must not leave temp files behind
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest086, TestSize.Level1)
{
    int fd = open("/data/test/errorTTF.ttf", O_RDONLY);
    ASSERT_GE(fd, 0);
    int32_t ret = manager_->InstallFont(fd, TEST_USERID);
    close(fd);
    EXPECT_EQ(ret, ERR_FILE_VERIFY_FAIL);

    int32_t tempCount = 0;
    for (const auto &entry : std::filesystem::directory_iterator(TEMP_PATH_TEST)) {
        (void)entry;
        tempCount++;
    }
    EXPECT_EQ(tempCount, 0);
}

/**
 * @tc.name: FontManagerFuncTest087
 * @tc.desc: Duplicate install must reject early and clean up its temp copy
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest087, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    ASSERT_GE(fd, 0);
    EXPECT_EQ(manager_->InstallFont(fd, TEST_USERID), ERR_OK);
    close(fd);

    fd = open(FONT_PATH.c_str(), O_RDONLY);
    ASSERT_GE(fd, 0);
    EXPECT_EQ(manager_->InstallFont(fd, TEST_USERID), ERR_INSTALLED_ALRADY);
    close(fd);

    int32_t tempCount = 0;
    for (const auto &entry : std::filesystem::directory_iterator(TEMP_PATH_TEST)) {
        (void)entry;
        tempCount++;
    }
    EXPECT_EQ(tempCount, 0);
}

/**
 * @tc.name: FontManagerFuncTest088
 * @tc.desc: CleanupAppScopeFonts must wait for the per-user install lock
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest088, TestSize.Level1)
{
    auto& fontConfig = manager_->SafeGetOrCreateConfig(TEST_USERID, FONT_CONFIG_FILE_TEST);
    std::unique_lock<std::mutex> installLock(fontConfig.GetInstallLock());
    std::atomic<bool> observedLocked {false};
    std::atomic<bool> finished {false};
    auto manager = manager_;
    std::thread worker([manager, &fontConfig, &observedLocked, &finished]() {
        // Spin until the main thread is observed holding the same lock.
        while (!observedLocked.load()) {
            if (!fontConfig.GetInstallLock().try_lock()) {
                observedLocked = true;
            } else {
                fontConfig.GetInstallLock().unlock();
                std::this_thread::yield();
            }
        }
        (void)manager->CleanupAppScopeFonts("app_5001", TEST_USERID);
        finished = true;
    });
    while (!observedLocked.load()) {
        std::this_thread::yield();
    }
    EXPECT_FALSE(finished.load());
    installLock.unlock();
    worker.join();
    EXPECT_TRUE(finished.load());
}

/**
 * @tc.name: FontManagerFuncTest089
 * @tc.desc: Test UninstallFontByFullName removes a user-level font
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest089, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    ASSERT_GE(fd, 0);
    EXPECT_EQ(manager_->InstallFont(fd, TEST_USERID), ERR_OK);
    close(fd);

    auto& fontConfig = manager_->SafeGetOrCreateConfig(TEST_USERID, FONT_CONFIG_FILE_TEST);
    ASSERT_TRUE(fontConfig.GetFontRecordByName(FONT_FULL_NAME).has_value());
    EXPECT_EQ(manager_->UninstallFontByFullName(FONT_FULL_NAME, TEST_USERID), ERR_OK);
    EXPECT_FALSE(fontConfig.GetFontRecordByName(FONT_FULL_NAME).has_value());
    EXPECT_EQ(fontConfig.GetTotalInstalledFontsNum(), 0);
}

/**
 * @tc.name: FontManagerFuncTest090
 * @tc.desc: Test UninstallFontByFullName removes an app-level scope font
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest090, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    ASSERT_GE(fd, 0);
    ScopeFontInstallInfo info;
    info.fd = fd;
    info.scope = FONT_SCOPE_APP;
    info.srcPath = "file://test/uninstall_by_fullname_app.ttf";
    info.bundleName = "com.example.app";
    info.appIdentifier = "app_5001";
    info.userId = TEST_USERID;
    ASSERT_EQ(manager_->InstallScopeFont(info), ERR_OK);
    close(fd);

    auto& fontConfig = manager_->SafeGetOrCreateConfig(TEST_USERID, FONT_CONFIG_FILE_TEST);
    auto record = fontConfig.GetFontRecordByUrl(info.srcPath);
    ASSERT_TRUE(record.has_value());
    std::string realPath = INSTALL_PATH_TEST + "app_5001/" +
        FontManagerUtils::GetFileName(record->fontPath);
    ASSERT_TRUE(FontManagerUtils::CheckPathExist(realPath));

    EXPECT_EQ(manager_->UninstallFontByFullName(FONT_FULL_NAME, TEST_USERID), ERR_OK);
    EXPECT_FALSE(FontManagerUtils::CheckPathExist(realPath));
    EXPECT_FALSE(fontConfig.GetFontRecordByUrl(info.srcPath).has_value());
    EXPECT_EQ(manager_->GetFontScope(info.srcPath, info.bundleName, TEST_TOKEN_ID, TEST_USERID),
        ERR_SCOPE_FONT_NOT_FOUND);
}

/**
 * @tc.name: FontManagerFuncTest091
 * @tc.desc: Test UninstallFontByFullName removes a session-level scope font
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest091, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    ASSERT_GE(fd, 0);
    ScopeFontInstallInfo info;
    info.fd = fd;
    info.scope = FONT_SCOPE_SESSION;
    info.srcPath = "file://test/uninstall_by_fullname_session.ttf";
    info.bundleName = "com.example.session";
    info.appIdentifier = "session_5001";
    info.userId = TEST_USERID;
    ASSERT_EQ(manager_->InstallScopeFont(info), ERR_OK);
    close(fd);

    auto& fontConfig = manager_->SafeGetOrCreateConfig(TEST_USERID, FONT_CONFIG_FILE_TEST);
    auto record = fontConfig.GetFontRecordByUrl(info.srcPath);
    ASSERT_TRUE(record.has_value());
    std::string realPath = INSTALL_PATH_TEST + "session_5001/" +
        FontManagerUtils::GetFileName(record->fontPath);
    ASSERT_TRUE(FontManagerUtils::CheckPathExist(realPath));

    EXPECT_EQ(manager_->UninstallFontByFullName(FONT_FULL_NAME, TEST_USERID), ERR_OK);
    EXPECT_FALSE(FontManagerUtils::CheckPathExist(realPath));
    EXPECT_FALSE(fontConfig.GetFontRecordByUrl(info.srcPath).has_value());
}

/**
 * @tc.name: FontManagerFuncTest092
 * @tc.desc: Test UninstallFontByFullName with invalid parameters
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest092, TestSize.Level1)
{
    EXPECT_EQ(manager_->UninstallFontByFullName("NonExistentFont12345", TEST_USERID),
        ERR_UNINSTALL_FILE_NOT_EXISTS);
    EXPECT_EQ(manager_->UninstallFontByFullName("", TEST_USERID), ERR_UNINSTALL_FILE_NOT_EXISTS);
    EXPECT_EQ(manager_->UninstallFontByFullName(FONT_FULL_NAME, -1), ERR_INVALID_PARAM);
}

/**
 * @tc.name: FontManagerFuncTest093
 * @tc.desc: Test UninstallFontByFullName removes a TTC scope record by one of its names
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest093, TestSize.Level1)
{
    int fd = open(TTC_FONT_PATH.c_str(), O_RDONLY);
    ASSERT_GE(fd, 0);
    ScopeFontInstallInfo info;
    info.fd = fd;
    info.scope = FONT_SCOPE_APP;
    info.srcPath = "file://test/uninstall_by_fullname_ttc.ttf";
    info.bundleName = "com.example.ttc";
    info.appIdentifier = "app_5001";
    info.userId = TEST_USERID;
    ASSERT_EQ(manager_->InstallScopeFont(info), ERR_OK);
    close(fd);

    auto& fontConfig = manager_->SafeGetOrCreateConfig(TEST_USERID, FONT_CONFIG_FILE_TEST);
    auto record = fontConfig.GetFontRecordByUrl(info.srcPath);
    ASSERT_TRUE(record.has_value());
    ASSERT_GT(record->fullNames.size(), 1u);
    std::string realPath = INSTALL_PATH_TEST + "app_5001/" +
        FontManagerUtils::GetFileName(record->fontPath);
    ASSERT_TRUE(FontManagerUtils::CheckPathExist(realPath));

    EXPECT_EQ(manager_->UninstallFontByFullName(TTC_FONT_FULL_NAME[0], TEST_USERID), ERR_OK);
    EXPECT_FALSE(FontManagerUtils::CheckPathExist(realPath));
    EXPECT_FALSE(fontConfig.GetFontRecordByUrl(info.srcPath).has_value());
}

/**
 * @tc.name: FontManagerFuncTest094
 * @tc.desc: Test UninstallFontByFullName removes the record when the font file is missing
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest094, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    ASSERT_GE(fd, 0);
    ASSERT_EQ(manager_->InstallFont(fd, TEST_USERID), ERR_OK);
    close(fd);

    auto& fontConfig = manager_->SafeGetOrCreateConfig(TEST_USERID, FONT_CONFIG_FILE_TEST);
    auto record = fontConfig.GetFontRecordByName(FONT_FULL_NAME);
    ASSERT_TRUE(record.has_value());
    std::string realPath = INSTALL_PATH_TEST + FontManagerUtils::GetFileName(record->fontPath);
    ASSERT_TRUE(FontManagerUtils::RemoveFile(realPath));

    EXPECT_EQ(manager_->UninstallFontByFullName(FONT_FULL_NAME, TEST_USERID), ERR_OK);
    EXPECT_FALSE(fontConfig.GetFontRecordByName(FONT_FULL_NAME).has_value());
}

/**
 * @tc.name: FontManagerFuncTest095
 * @tc.desc: Test UninstallFontByFullName keeps a record whose path escapes the install dir
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest095, TestSize.Level1)
{
    std::string configWithBadPath = R"({"fontlist": [{"fontfullpath": ")" +
        INSTALL_PATH_APP + R"(../../etc/passwd", "fullname": [")" + FONT_FULL_NAME +
        R"("], "scope": 0, "appIdentifier": "app_5001"}], "version": "7.0"})";
    FontManagerUtils::CreateFileWithPermission(FONT_CONFIG_FILE_TEST, configWithBadPath);
    manager_->configMap_.clear();

    EXPECT_EQ(manager_->UninstallFontByFullName(FONT_FULL_NAME, TEST_USERID),
        ERR_UNINSTALL_FILE_NOT_EXISTS);
    auto& fontConfig = manager_->SafeGetOrCreateConfig(TEST_USERID, FONT_CONFIG_FILE_TEST);
    EXPECT_TRUE(fontConfig.GetFontRecordByName(FONT_FULL_NAME).has_value());
}
} // namespace FontManager
} // namespace Global
} // namespace OHOS