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

#include <gtest/gtest.h>
#include "font_manager_test.h"
#define private public
#define protected public
#include "font_manager.h"
#undef private
#undef protected
#include "file_utils.h"
#include "font_define.h"
#include <fcntl.h>
#include <string>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <dirent.h>
#include <unistd.h>

namespace {
const std::string INSTALL_PATH = "/data/service/el1/public/for-all-app/fonts/";
const std::string TEMP_PATH = "/data/service/el1/public/for-all-app/fonts/temp/";
const std::string FONT_CONFIG_FILE = INSTALL_PATH + "install_fontconfig.json";
const std::string FONT_PATH = "/data/test/HarmonyOS_Sans.ttf";
const std::string fontfullname = "HarmonyOS Sans";
const std::string TTC_FONT_PATH = "/data/test/NotoSansCJK-Regular.ttc";
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
    FontManager manager_;
};

void FontManagerTest::SetUpTestCase(void)
{}

void FontManagerTest::TearDownTestCase(void)
{
    FileUtils::DeleteDir(TEMP_PATH, true);
}

void FontManagerTest::SetUp(void)
{}

void FontManagerTest::TearDown(void)
{
    FileUtils::DeleteDir(INSTALL_PATH, false);
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
    int ret = this->manager_.InstallFont(fd);
    EXPECT_EQ(ret, SUCCESS);
    if (fd >= 0) {
        close(fd);
    }
    fd = open(FONT_PATH.c_str(), O_RDONLY);
    ret = this->manager_.InstallFont(fd);
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
    int ret = this->manager_.InstallFont(fd);
    EXPECT_EQ(ret, SUCCESS);
    if (fd >= 0) {
        close(fd);
    }
    ret = this->manager_.UninstallFont(fontfullname);
    EXPECT_EQ(ret, SUCCESS);
    ret = this->manager_.UninstallFont(fontfullname);
    EXPECT_EQ(ret, ERR_UNINSTALL_FILE_NOT_EXISTS);
    ret = this->manager_.UninstallFont("");
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name: FontManagerFuncTest003
 * @tc.desc: Test FontManager GetFontFullName case
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest003, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    auto fullNameVector = this->manager_.GetFontFullName(fd);
    std::vector<std::string> fullName{"HarmonyOS Sans"};
    EXPECT_EQ(fullName, fullNameVector);
    if (fd >= 0) {
        close(fd);
    }
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
    int fd = open(errTTFPath.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    auto fullNameVector = this->manager_.GetFontFullName(fd);
    EXPECT_EQ(fullNameVector.size(), 0);
    if (fd >= 0) {
        close(fd);
    }
    fd = open(emptyTTFPath.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    fullNameVector = this->manager_.GetFontFullName(fd);
    EXPECT_EQ(fullNameVector.size(), 0);
    if (fd >= 0) {
        close(fd);
    }
    fd = open(errorType.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    fullNameVector = this->manager_.GetFontFullName(fd);
    EXPECT_EQ(fullNameVector.size(), 0);
    if (fd >= 0) {
        close(fd);
    }
    fd = open(errorTTCPath.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    fullNameVector = this->manager_.GetFontFullName(fd);
    EXPECT_EQ(fullNameVector.size(), 0);
    if (fd >= 0) {
        close(fd);
    }
    fd = open(emptyTTCPath.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    fullNameVector = this->manager_.GetFontFullName(fd);
    EXPECT_EQ(fullNameVector.size(), 0);
    if (fd >= 0) {
        close(fd);
    }
}

/**
 * @tc.name: FontManagerFuncTest005
 * @tc.desc: Test FontManager CheckFontConfigPath case
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest005, TestSize.Level1)
{
    int ret = this->manager_.CheckFontConfigPath();
    EXPECT_EQ(ret, true);
    FileUtils::RemoveFile(FONT_CONFIG_FILE);
    ret = this->manager_.CheckFontConfigPath();
    EXPECT_EQ(ret, true);
    ret = this->manager_.CheckFontConfigPath();
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
    EXPECT_EQ(FileUtils::CopyFile(fd, FONT_CONFIG_FILE), true);
    fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    int ret = this->manager_.InstallFont(fd);
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
    EXPECT_EQ(FileUtils::CopyFile(fd, FONT_CONFIG_FILE), true);
    if (fd > 0) {
        close(fd);
    }
    fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    int ret = this->manager_.InstallFont(fd);
    EXPECT_EQ(ret, SUCCESS);
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
    int ret = this->manager_.InstallFont(fd);
    EXPECT_EQ(ret, SUCCESS);
    if (fd >= 0) {
        close(fd);
    }
    fd = open(TTC_FONT_PATH.c_str(), O_RDONLY);
    ret = this->manager_.InstallFont(fd);
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
    int ret = this->manager_.InstallFont(fd);
    EXPECT_EQ(ret, SUCCESS);
    if (fd >= 0) {
        close(fd);
    }
    ret = this->manager_.UninstallFont(TTC_FONT_FULL_NAME[0]);
    EXPECT_EQ(ret, SUCCESS);
    ret = this->manager_.UninstallFont(TTC_FONT_FULL_NAME[0]);
    EXPECT_EQ(ret, ERR_UNINSTALL_FILE_NOT_EXISTS);
    ret = this->manager_.UninstallFont("");
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name: FontManagerFuncTest010
 * @tc.desc: Test FontManager TTC GetFontFullName case
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest010, TestSize.Level1)
{
    int fd = open(TTC_FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    auto fullNameVector = this->manager_.GetFontFullName(fd);
    std::sort(fullNameVector.begin(), fullNameVector.end());
    auto ttcFullName = TTC_FONT_FULL_NAME;
    std::sort(ttcFullName.begin(), ttcFullName.end());
    EXPECT_EQ(fullNameVector, ttcFullName);
    if (fd >= 0) {
        close(fd);
    }
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
    int ret = this->manager_.InstallFont(fd);
    EXPECT_EQ(ret, ERR_FILE_VERIFY_FAIL);
    if (fd >= 0) {
        close(fd);
    }
    fd = open(emptyTTFPath.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    ret = this->manager_.InstallFont(fd);
    EXPECT_EQ(ret, ERR_FILE_VERIFY_FAIL);
    if (fd >= 0) {
        close(fd);
    }
    fd = open(errorType.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    ret = this->manager_.InstallFont(fd);
    EXPECT_EQ(ret, ERR_FILE_VERIFY_FAIL);
    if (fd >= 0) {
        close(fd);
    }
    fd = open(errorTTCPath.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    ret = this->manager_.InstallFont(fd);
    EXPECT_EQ(ret, ERR_FILE_VERIFY_FAIL);
    if (fd >= 0) {
        close(fd);
    }
    fd = open(emptyTTCPath.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    ret = this->manager_.InstallFont(fd);
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
    FileUtils::CreatDirWithPermission("/data/test/testRepeats/");
    const std::string fontPath1 = "/data/test/HarmonyOS_Sans.ttf";
    const std::string fontPath4 = "/data/test/NotoSansVai-Regular.ttf";
    const std::string fontPath5 = "/data/test/testRepeats/HarmonyOS_Sans.ttf";

    int fd = open(fontPath1.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    int ret = this->manager_.InstallFont(fd);
    if (fd > 0) {
        close(fd);
    }
    EXPECT_EQ(ret, SUCCESS);
    fd = open(fontPath4.c_str(), O_RDONLY);
    FileUtils::CopyFile(fd, fontPath5);
    if (fd > 0) {
        close(fd);
    }
    fd = open(fontPath5.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    ret = this->manager_.InstallFont(fd);
    if (fd > 0) {
        close(fd);
    }
    EXPECT_EQ(ret, SUCCESS);
    if (!FileUtils::CheckPathExist("/data/test/testRepeats/")) {
        return;
    }
    std::filesystem::path rPath("/data/test/testRepeats/");
    for (const auto &file : std::filesystem::directory_iterator(rPath)) {
        if (file.is_regular_file()) {
            std::filesystem::remove(file);
        }
    }
    EXPECT_EQ(rmdir("/data/test/testRepeats") != -1, true);
}

/**
 * @tc.name: FontManagerFuncTest012
 * @tc.desc: Test FontManager Repeat InstallFont case
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerTest, FontManagerFuncTest013, TestSize.Level1)
{
    FileUtils::CreatDirWithPermission("/data/test/testRepeats/");
    const std::string fontPath2 = "/data/test/NotoSansCJK-Regular.ttc";
    const std::string fontPath3 = "/data/test/NotoSerifCJK-Regular.ttc";
    const std::string fontPath6 = "/data/test/testRepeats/NotoSerifCJK-Regular.ttc";

    int fd = open(fontPath2.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    int ret = this->manager_.InstallFont(fd);
    if (fd > 0) {
        close(fd);
    }
    EXPECT_EQ(ret, SUCCESS);
    fd = open(fontPath3.c_str(), O_RDONLY);
    FileUtils::CopyFile(fd, fontPath6);
    if (fd > 0) {
        close(fd);
    }
    fd = open(fontPath6.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    ret = this->manager_.InstallFont(fd);
    if (fd > 0) {
        close(fd);
    }
    EXPECT_EQ(ret, SUCCESS);
    if (!FileUtils::CheckPathExist("/data/test/testRepeats/")) {
        return;
    }
    std::filesystem::path rPath("/data/test/testRepeats/");
    for (const auto &file : std::filesystem::directory_iterator(rPath)) {
        if (file.is_regular_file()) {
            std::filesystem::remove(file);
        }
    }
    EXPECT_EQ(rmdir("/data/test/testRepeats") != -1, true);
}

} // namespace FontManager
} // namespace Global
} // namespace OHOS