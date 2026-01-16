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
#define private public
#define protected public
#include "font_config.h"
#include "font_manager.h"
#include "font_manager_utils.h"
#undef private
#undef protected
#include <string>
#include <vector>

namespace {
const std::string INSTALL_PATH_TEST = "/data/service/el1/100/for-all-app/fonts/";
const std::string FONT_CONFIG_FILE_TEST = INSTALL_PATH_TEST + "install_fontconfig.json";
}

using testing::ext::TestSize;
using namespace std;

namespace OHOS {
namespace Global {
namespace FontManager {

class FontConfigTest : public testing::Test {
public:
    FontConfigTest() : config_(FONT_CONFIG_FILE_TEST)
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

protected:
    FontConfig config_;
    FontManager fontManager_;
};

void FontConfigTest::SetUpTestCase(void)
{
    FontManagerUtils::RemoveFile(FONT_CONFIG_FILE_TEST);
}

void FontConfigTest::TearDownTestCase(void)
{
    FontManagerUtils::RemoveFile(FONT_CONFIG_FILE_TEST);
}

void FontConfigTest::SetUp(void)
{
    FontManagerUtils::RemoveFile(FONT_CONFIG_FILE_TEST);
}

void FontConfigTest::TearDown(void)
{
    FontManagerUtils::RemoveFile(FONT_CONFIG_FILE_TEST);
}

/**
 * @tc.name: FontConfigFuncTest001
 * @tc.desc: Test FontConfig TTF InsertFontRecord case
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest001, TestSize.Level1)
{
    ASSERT_EQ(FontManagerUtils::CheckAndInitInstallPath(INSTALL_PATH_TEST), true);
    std::string fontFullPath = INSTALL_PATH_TEST + "TestFont_Sans.ttf";
    std::vector<std::string> fullName{"TestFont-Sans"};

    EXPECT_EQ(this->config_.InsertFontRecord(fontFullPath, fullName), true);

    auto t = this->config_.GetFontsMap(this->config_.ConfigPath_);
    EXPECT_EQ(t.find(fontFullPath) == t.end(), false);
}

/**
 * @tc.name: FontConfigFuncTest002
 * @tc.desc: Test FontConfig TTF DeleteFontRecord case
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest002, TestSize.Level1)
{
    ASSERT_EQ(FontManagerUtils::CheckAndInitInstallPath(INSTALL_PATH_TEST), true);
    std::string fontFullPath = INSTALL_PATH_TEST + "TestFont_Sans.ttf";
    std::vector<std::string> fullName{"TestFont-Sans"};

    EXPECT_EQ(this->config_.InsertFontRecord(fontFullPath, fullName), true);

    auto t = this->config_.GetFontsMap(this->config_.ConfigPath_);

    EXPECT_EQ(t.find(fontFullPath) == t.end(), false);

    EXPECT_EQ(this->config_.DeleteFontRecord(fontFullPath), true);

    t = this->config_.GetFontsMap(this->config_.ConfigPath_);

    EXPECT_EQ(t.find(fontFullPath) == t.end(), true);
}

/**
 * @tc.name: FontConfigFuncTest003
 * @tc.desc: Test FontConfig TTF GetInstalledFontsNum case
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest003, TestSize.Level1)
{
    ASSERT_EQ(FontManagerUtils::CheckAndInitInstallPath(INSTALL_PATH_TEST), true);
    std::string fontFullPath1 = INSTALL_PATH_TEST + "TestFont_Sans1.ttf";
    std::vector<std::string> fullName1{"TestFont-Sans1"};
    std::string fontFullPath2 = INSTALL_PATH_TEST + "TestFont_Sans2.ttf";
    std::vector<std::string> fullName2{"TestFont-Sans2"};
    std::string fontFullPath3 = INSTALL_PATH_TEST + "TestFont_Sans3.ttf";
    std::vector<std::string> fullName3{"TestFont-Sans3"};
    std::string fontFullPath4 = INSTALL_PATH_TEST + "TestFont_Sans4.ttf";
    std::vector<std::string> fullName4{"TestFont-Sans4"};

    EXPECT_EQ(this->config_.InsertFontRecord(fontFullPath1, fullName1), true);
    EXPECT_EQ(this->config_.InsertFontRecord(fontFullPath2, fullName2), true);
    EXPECT_EQ(this->config_.InsertFontRecord(fontFullPath3, fullName3), true);
    EXPECT_EQ(this->config_.InsertFontRecord(fontFullPath4, fullName4), true);

    EXPECT_EQ(this->config_.GetInstalledFontsNum(), 4);
}

/**
 * @tc.name: FontConfigFuncTest004
 * @tc.desc: Test FontConfig TTF getFontFileByName case
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest004, TestSize.Level1)
{
    ASSERT_EQ(FontManagerUtils::CheckAndInitInstallPath(INSTALL_PATH_TEST), true);
    std::string fontFullPath = INSTALL_PATH_TEST + "TestFont_Sans.ttf";
    std::vector<std::string> fullName{"TestFont-Sans"};

    EXPECT_EQ(this->config_.InsertFontRecord(fontFullPath, fullName), true);

    EXPECT_EQ(this->config_.GetFontFileByName("TestFont-Sans"), fontFullPath);
    EXPECT_EQ(this->config_.GetFontFileByName("ERROR"), "");
}

/**
 * @tc.name: FontConfigFuncTest005
 * @tc.desc: Test FontConfig TTC InsertFontRecord case
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest005, TestSize.Level1)
{
    ASSERT_EQ(FontManagerUtils::CheckAndInitInstallPath(INSTALL_PATH_TEST), true);
    std::string fontFullPath = INSTALL_PATH_TEST + "NotoSansCJK-Regular.ttc";
    std::vector<std::string> fullName{
        "Noto Sans CJK JP", "Noto Sans CJK KR", "Noto Sans CJK SC", "Noto Sans CJK TC",
        "Noto Sans CJK HK", "Noto Sans Mono CJK JP", "Noto Sans Mono CJK KR",
        "Noto Sans Mono CJK SC", "Noto Sans Mono CJK TC", "Noto Sans Mono CJK HK"};

    EXPECT_EQ(this->config_.InsertFontRecord(fontFullPath, fullName), true);

    auto t = this->config_.GetFontsMap(this->config_.ConfigPath_);
    EXPECT_EQ(t.find(fontFullPath) == t.end(), false);
}

/**
 * @tc.name: FontConfigFuncTest006
 * @tc.desc: Test FontConfig TTC DeleteFontRecord case
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest006, TestSize.Level1)
{
    ASSERT_EQ(FontManagerUtils::CheckAndInitInstallPath(INSTALL_PATH_TEST), true);
    std::string fontFullPath = INSTALL_PATH_TEST + "NotoSansCJK-Regular.ttc";
    std::vector<std::string> fullName{
        "Noto Sans CJK JP", "Noto Sans CJK KR", "Noto Sans CJK SC", "Noto Sans CJK TC",
        "Noto Sans CJK HK", "Noto Sans Mono CJK JP", "Noto Sans Mono CJK KR",
        "Noto Sans Mono CJK SC", "Noto Sans Mono CJK TC", "Noto Sans Mono CJK HK"};

    EXPECT_EQ(this->config_.InsertFontRecord(fontFullPath, fullName), true);

    auto t = this->config_.GetFontsMap(this->config_.ConfigPath_);

    EXPECT_EQ(t.find(fontFullPath) == t.end(), false);

    EXPECT_EQ(this->config_.DeleteFontRecord(fontFullPath), true);

    t = this->config_.GetFontsMap(this->config_.ConfigPath_);

    EXPECT_EQ(t.find(fontFullPath) == t.end(), true);
}

/**
 * @tc.name: FontConfigFuncTest007
 * @tc.desc: Test FontConfig TTC GetInstalledFontsNum case
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest007, TestSize.Level1)
{
    ASSERT_EQ(FontManagerUtils::CheckAndInitInstallPath(INSTALL_PATH_TEST), true);
    std::string fontFullPath1 = INSTALL_PATH_TEST + "NotoSansCJK-Regular1.ttc";
    std::vector<std::string> fullName1{"Noto Serif CJK JP",
        "Noto Serif CJK KR", "Noto Serif CJK SC", "Noto Serif CJK TC", "Noto Serif CJK HK"};
    std::string fontFullPath2 = INSTALL_PATH_TEST + "NotoSansCJK-Regular2.ttc";
    std::vector<std::string> fullName2{"Noto Serif CJK JP",
        "Noto Serif CJK KR", "Noto Serif CJK SC", "Noto Serif CJK TC", "Noto Serif CJK HK"};
    std::string fontFullPath3 = INSTALL_PATH_TEST + "NotoSansCJK-Regular3.ttc";
    std::vector<std::string> fullName3{"Noto Serif CJK JP",
        "Noto Serif CJK KR", "Noto Serif CJK SC", "Noto Serif CJK TC", "Noto Serif CJK HK"};
    std::string fontFullPath4 = INSTALL_PATH_TEST + "NotoSansCJK-Regular4.ttc";
    std::vector<std::string> fullName4{"Noto Serif CJK JP",
        "Noto Serif CJK KR", "Noto Serif CJK SC", "Noto Serif CJK TC", "Noto Serif CJK HK"};

    EXPECT_EQ(this->config_.InsertFontRecord(fontFullPath1, fullName1), true);
    EXPECT_EQ(this->config_.InsertFontRecord(fontFullPath2, fullName2), true);
    EXPECT_EQ(this->config_.InsertFontRecord(fontFullPath3, fullName3), true);
    EXPECT_EQ(this->config_.InsertFontRecord(fontFullPath4, fullName4), true);

    EXPECT_EQ(this->config_.GetInstalledFontsNum(), 4);
}

/**
 * @tc.name: FontConfigFuncTest008
 * @tc.desc: Test FontConfig TTC getFontFileByName case
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest008, TestSize.Level1)
{
    ASSERT_EQ(FontManagerUtils::CheckAndInitInstallPath(INSTALL_PATH_TEST), true);
    std::string fontFullPath = INSTALL_PATH_TEST + "NotoSansCJK-Regular.ttc";
    std::vector<std::string> fullName{
        "Noto Serif CJK JP", "Noto Serif CJK KR", "Noto Serif CJK SC", "Noto Serif CJK TC", "Noto Serif CJK HK"};

    EXPECT_EQ(this->config_.InsertFontRecord(fontFullPath, fullName), true);

    EXPECT_EQ(this->config_.GetFontFileByName("Noto Serif CJK SC"), fontFullPath);
    EXPECT_EQ(this->config_.GetFontFileByName("ERROR"), "");
}

/**
 * @tc.name: FontConfigFuncTest009
 * @tc.desc: Test FontConfig TTC DeleteFontRecord case
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest009, TestSize.Level1)
{
    ASSERT_EQ(FontManagerUtils::CheckAndInitInstallPath(INSTALL_PATH_TEST), true);
    std::string fontFullPath = INSTALL_PATH_TEST + "NotoSansCJK-Regular.ttc";
    std::vector<std::string> fullName{
        "Noto Serif CJK JP", "Noto Serif CJK KR", "Noto Serif CJK SC", "Noto Serif CJK TC", "Noto Serif CJK HK"};

    EXPECT_EQ(this->config_.InsertFontRecord(fontFullPath, fullName), true);
    EXPECT_EQ(this->config_.DeleteFontRecord(fontFullPath), true);
    EXPECT_EQ(this->config_.GetFontFileByName("Noto Serif CJK SC"), "");
}

/**
 * @tc.name: FontConfigFuncTest010
 * @tc.desc: Test FontConfig TTC CheckAndInitInstallPath case
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest010, TestSize.Level1)
{
    ASSERT_EQ(FontManagerUtils::CheckPathExist(INSTALL_PATH_TEST), true);
    std::string fontList = R"({
        "fontlist": []
    })";
    ASSERT_EQ(FontManagerUtils::CreateFileWithPermission(FONT_CONFIG_FILE_TEST, fontList), true);
    ASSERT_EQ(FontManagerUtils::CheckAndInitInstallPath(INSTALL_PATH_TEST), true);
    std::string fontFullPath = INSTALL_PATH_TEST + "NotoSansCJK-Regular.ttc";
    std::vector<std::string> fullName{
        "Noto Serif CJK JP", "Noto Serif CJK KR", "Noto Serif CJK SC", "Noto Serif CJK TC", "Noto Serif CJK HK"};

    EXPECT_EQ(this->config_.InsertFontRecord(fontFullPath, fullName), true);
    EXPECT_EQ(this->config_.CheckAndUpdateFontRecord(), true);
    EXPECT_EQ(this->config_.fontsMap_.find(fontFullPath)->second.size(), 0);
}

/**
 * @tc.name: FontConfigFuncTest011
 * @tc.desc: Test CheckAndUpdateFontRecord with corrupted JSON content
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest011, TestSize.Level1)
{
    std::string badJson = "{ \"fontlist\": [";
    FILE *fp = fopen(FONT_CONFIG_FILE_TEST.c_str(), "w");
    if (fp) {
        fwrite(badJson.c_str(), 1, badJson.length(), fp);
        (void)fclose(fp);
    }

    EXPECT_FALSE(this->config_.CheckAndUpdateFontRecord());
}

/**
 * @tc.name: FontConfigFuncTest012
 * @tc.desc: Test CheckAndUpdateFontRecord with existing version field
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest012, TestSize.Level1)
{
    std::string versionJson = "{\"version\": 1, \"fontlist\": []}";
    FILE *fp = fopen(FONT_CONFIG_FILE_TEST.c_str(), "w");
    if (fp) {
        fwrite(versionJson.c_str(), 1, versionJson.length(), fp);
        (void)fclose(fp);
    }

    EXPECT_TRUE(this->config_.CheckAndUpdateFontRecord());
}

/**
 * @tc.name: FontConfigFuncTest013
 * @tc.desc: Test CheckAndUpdateFontRecord where fontlist is not an array
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest013, TestSize.Level1)
{
    std::string invalidListJson = "{\"fontlist\": \"not_an_array\"}";
    FILE *fp = fopen(FONT_CONFIG_FILE_TEST.c_str(), "w");
    if (fp) {
        fwrite(invalidListJson.c_str(), 1, invalidListJson.length(), fp);
        (void)fclose(fp);
    }
    EXPECT_FALSE(this->config_.CheckAndUpdateFontRecord());
}

/**
 * @tc.name: FontConfigFuncTest014
 * @tc.desc: Test CheckConfigFile with non-existent file
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest014, TestSize.Level1)
{
    FontManagerUtils::RemoveFile(FONT_CONFIG_FILE_TEST);

    std::string result = this->config_.CheckConfigFile(FONT_CONFIG_FILE_TEST);
    EXPECT_EQ(result, "");
}
}  // namespace FontManager
}  // namespace Global
}  // namespace OHOS
