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

#include <gtest/gtest.h>
#include "font_config_test.h"
#define private public
#define protected public
#include "font_config.h"
#include "font_manager.h"
#undef private
#undef protected
#include "file_utils.h"
#include <string>
#include <vector>

namespace {
const std::string INSTALL_PATH = "/data/service/el1/public/for-all-app/fonts/";
const std::string TEMP_PATH = "/data/service/el1/public/for-all-app/fonts/temp/";
const std::string FONT_CONFIG_FILE = INSTALL_PATH + "install_fontconfig.json";
}

using testing::ext::TestSize;
using namespace std;

namespace OHOS {
namespace Global {
namespace FontManager {

class FontConfigTest : public testing::Test {
public:
    FontConfigTest() : config_(FONT_CONFIG_FILE)
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
    FileUtils::RemoveFile(FONT_CONFIG_FILE);
}

void FontConfigTest::TearDownTestCase(void)
{
    FileUtils::RemoveFile(FONT_CONFIG_FILE);
}

void FontConfigTest::SetUp(void)
{
    FileUtils::RemoveFile(FONT_CONFIG_FILE);
}

void FontConfigTest::TearDown(void)
{
    FileUtils::RemoveFile(FONT_CONFIG_FILE);
}

/**
 * @tc.name: FontConfigFuncTest001
 * @tc.desc: Test FontConfig TTF InsertFontRecord case
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest001, TestSize.Level1)
{
    ASSERT_EQ(fontManager_.CheckFontConfigPath(), true);
    std::string fontFullPath = INSTALL_PATH + "HarmonyOS_Sans.ttf";
    std::vector<std::string> fullName{"HarmonyOS-Sans"};

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
    ASSERT_EQ(fontManager_.CheckFontConfigPath(), true);
    std::string fontFullPath = INSTALL_PATH + "HarmonyOS_Sans.ttf";
    std::vector<std::string> fullName{"HarmonyOS-Sans"};

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
    ASSERT_EQ(fontManager_.CheckFontConfigPath(), true);
    std::string fontFullPath1 = INSTALL_PATH + "HarmonyOS_Sans1.ttf";
    std::vector<std::string> fullName1{"HarmonyOS-Sans1"};
    std::string fontFullPath2 = INSTALL_PATH + "HarmonyOS_Sans2.ttf";
    std::vector<std::string> fullName2{"HarmonyOS-Sans2"};
    std::string fontFullPath3 = INSTALL_PATH + "HarmonyOS_Sans3.ttf";
    std::vector<std::string> fullName3{"HarmonyOS-Sans3"};
    std::string fontFullPath4 = INSTALL_PATH + "HarmonyOS_Sans4.ttf";
    std::vector<std::string> fullName4{"HarmonyOS-Sans4"};

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
    ASSERT_EQ(fontManager_.CheckFontConfigPath(), true);
    std::string fontFullPath = INSTALL_PATH + "HarmonyOS_Sans.ttf";
    std::vector<std::string> fullName{"HarmonyOS-Sans"};

    EXPECT_EQ(this->config_.InsertFontRecord(fontFullPath, fullName), true);

    EXPECT_EQ(this->config_.GetFontFileByName("HarmonyOS-Sans"), fontFullPath);
    EXPECT_EQ(this->config_.GetFontFileByName("ERROR"), "");
}

/**
 * @tc.name: FontConfigFuncTest005
 * @tc.desc: Test FontConfig TTC InsertFontRecord case
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest005, TestSize.Level1)
{
    ASSERT_EQ(fontManager_.CheckFontConfigPath(), true);
    std::string fontFullPath = INSTALL_PATH + "NotoSansCJK-Regular.ttc";
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
    ASSERT_EQ(fontManager_.CheckFontConfigPath(), true);
    std::string fontFullPath = INSTALL_PATH + "NotoSansCJK-Regular.ttc";
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
    ASSERT_EQ(fontManager_.CheckFontConfigPath(), true);
    std::string fontFullPath1 = INSTALL_PATH + "NotoSansCJK-Regular1.ttc";
    std::vector<std::string> fullName1{"Noto Serif CJK JP",
        "Noto Serif CJK KR", "Noto Serif CJK SC", "Noto Serif CJK TC", "Noto Serif CJK HK"};
    std::string fontFullPath2 = INSTALL_PATH + "NotoSansCJK-Regular2.ttc";
    std::vector<std::string> fullName2{"Noto Serif CJK JP",
        "Noto Serif CJK KR", "Noto Serif CJK SC", "Noto Serif CJK TC", "Noto Serif CJK HK"};
    std::string fontFullPath3 = INSTALL_PATH + "NotoSansCJK-Regular3.ttc";
    std::vector<std::string> fullName3{"Noto Serif CJK JP",
        "Noto Serif CJK KR", "Noto Serif CJK SC", "Noto Serif CJK TC", "Noto Serif CJK HK"};
    std::string fontFullPath4 = INSTALL_PATH + "NotoSansCJK-Regular4.ttc";
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
    ASSERT_EQ(fontManager_.CheckFontConfigPath(), true);
    std::string fontFullPath = INSTALL_PATH + "NotoSansCJK-Regular.ttc";
    std::vector<std::string> fullName{
        "Noto Serif CJK JP", "Noto Serif CJK KR", "Noto Serif CJK SC", "Noto Serif CJK TC", "Noto Serif CJK HK"};

    EXPECT_EQ(this->config_.InsertFontRecord(fontFullPath, fullName), true);

    EXPECT_EQ(this->config_.GetFontFileByName("Noto Serif CJK SC"), fontFullPath);
    EXPECT_EQ(this->config_.GetFontFileByName("ERROR"), "");
}

}  // namespace FontManager
}  // namespace Global
}  // namespace OHOS
