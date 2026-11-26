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
#include <unistd.h>

#include "font_manager_inner_api.h"
#include "font_manager_utils.h"
#include "font_define.h"
#include "permission_common.h"

namespace {
const std::string INSTALL_PATH_TEST = "/data/service/el1/100/for-all-app/fonts/";
const std::string FONT_PATH = "/data/test/TestFont_Sans.ttf";
constexpr int32_t TEST_USERID = 100;
}

using testing::ext::TestSize;

namespace OHOS {
namespace Global {
namespace FontManager {

class FontManagerInnerApiTest : public testing::Test {
public:
    FontManagerInnerApiTest(){};
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void FontManagerInnerApiTest::SetUpTestCase(void)
{
    FontManagerUtils::DeleteDir(INSTALL_PATH_TEST, false);
}

void FontManagerInnerApiTest::TearDownTestCase(void)
{
    PermissionCommon::ResetTokenAndUid();
}

void FontManagerInnerApiTest::SetUp(void)
{
}

void FontManagerInnerApiTest::TearDown(void)
{
    FontManagerUtils::DeleteDir(INSTALL_PATH_TEST, false);
}

HWTEST_F(FontManagerInnerApiTest, InnerApiInstallFontTest001, TestSize.Level1)
{
    const std::string nonExistPath = "/data/test/nonexist.ttf";
    int32_t ret = FontManagerInnerApi::InstallFont(nonExistPath, TEST_USERID);
    EXPECT_EQ(ret, ERR_FILE_NOT_EXISTS);
}

HWTEST_F(FontManagerInnerApiTest, InnerApiInstallFontTest002, TestSize.Level1)
{
    const std::string emptyPath = "";
    int32_t ret = FontManagerInnerApi::InstallFont(emptyPath, TEST_USERID);
    EXPECT_EQ(ret, ERR_FILE_NOT_EXISTS);
}

HWTEST_F(FontManagerInnerApiTest, InnerApiUninstallFontTest001, TestSize.Level1)
{
    int32_t ret = FontManagerInnerApi::UninstallFont("", TEST_USERID);
    EXPECT_EQ(ret, ERR_NO_PERMISSION);
}

HWTEST_F(FontManagerInnerApiTest, InnerApiInstallFontTest003, TestSize.Level1)
{
    std::string nonExistPath = "/data/test/nonexistent_font_99999.ttf";
    int32_t ret = FontManagerInnerApi::InstallFont(nonExistPath, TEST_USERID);
    EXPECT_NE(ret, ERR_OK);
}

HWTEST_F(FontManagerInnerApiTest, InnerApiUninstallFontTest002, TestSize.Level1)
{
    int32_t ret = FontManagerInnerApi::UninstallFont("NonExistentFont99999", TEST_USERID);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: InnerApiInstallFontTest004
 * @tc.desc: Test InstallFont with valid path
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerInnerApiTest, InnerApiInstallFontTest004, TestSize.Level1)
{
    const std::string fontPath = "/data/test/TestFont_Sans.ttf";
    int32_t ret = FontManagerInnerApi::InstallFont(fontPath, TEST_USERID);
    // With valid path, PathToRealPath succeeds; service may be null -> ERR_INSTALL_FAIL
    // or IPC result. Either way, it should not be ERR_FILE_NOT_EXISTS (file exists).
    EXPECT_NE(ret, ERR_FILE_NOT_EXISTS);
}

/**
 * @tc.name: InnerApiUninstallFontTest003
 * @tc.desc: Test UninstallFont with empty name
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerInnerApiTest, InnerApiUninstallFontTest003, TestSize.Level1)
{
    int32_t ret = FontManagerInnerApi::UninstallFont("", TEST_USERID);
    // Empty name should trigger permission check failure or param error
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: InnerApiInstallFontTest005
 * @tc.desc: Test InstallFont with directory path
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerInnerApiTest, InnerApiInstallFontTest005, TestSize.Level1)
{
    int32_t ret = FontManagerInnerApi::InstallFont("/data/test/", TEST_USERID);
    // Directory path: PathToRealPath succeeds, fopen on directory may fail -> ERR_FILE_NOT_EXISTS
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: InnerApiInstallFontByFdTest001
 * @tc.desc: Test InstallFontByFd with invalid fd
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerInnerApiTest, InnerApiInstallFontByFdTest001, TestSize.Level1)
{
    int32_t ret = FontManagerInnerApi::InstallFontByFd(-1, TEST_USERID);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name: InnerApiInstallFontByFdTest002
 * @tc.desc: Test InstallFontByFd with invalid userId
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerInnerApiTest, InnerApiInstallFontByFdTest002, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    ASSERT_GE(fd, 0);
    int32_t ret = FontManagerInnerApi::InstallFontByFd(fd, -1);
    close(fd);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name: InnerApiInstallFontByFdTest003
 * @tc.desc: Test InstallFontByFd reaches IPC with valid parameters
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerInnerApiTest, InnerApiInstallFontByFdTest003, TestSize.Level1)
{
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    ASSERT_GE(fd, 0);
    int32_t ret = FontManagerInnerApi::InstallFontByFd(fd, TEST_USERID);
    close(fd);
    EXPECT_TRUE(ret == ERR_OK || ret == ERR_NO_PERMISSION || ret == ERR_INSTALL_FAIL);
}

/**
 * @tc.name: InnerApiUninstallFontByFullNameTest001
 * @tc.desc: Test UninstallFontByFullName with empty font name
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerInnerApiTest, InnerApiUninstallFontByFullNameTest001, TestSize.Level1)
{
    int32_t ret = FontManagerInnerApi::UninstallFontByFullName("", TEST_USERID);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: InnerApiUninstallFontByFullNameTest002
 * @tc.desc: Test UninstallFontByFullName with invalid userId
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerInnerApiTest, InnerApiUninstallFontByFullNameTest002, TestSize.Level1)
{
    int32_t ret = FontManagerInnerApi::UninstallFontByFullName("TestFont", -1);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name: InnerApiUninstallFontByFullNameTest003
 * @tc.desc: Test UninstallFontByFullName with a non-existent font name
 * @tc.type: FUNC
 */
HWTEST_F(FontManagerInnerApiTest, InnerApiUninstallFontByFullNameTest003, TestSize.Level1)
{
    int32_t ret = FontManagerInnerApi::UninstallFontByFullName("NonExistentFont99999", TEST_USERID);
    EXPECT_NE(ret, ERR_OK);
}
} // namespace FontManager
} // namespace Global
} // namespace OHOS
