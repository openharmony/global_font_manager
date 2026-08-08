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
#include "font_config.h"
#include "font_manager.h"
#include "font_manager_utils.h"
#include <string>
#include <vector>
#include <cstdlib>
#include "securec.h"

namespace {
const std::string INSTALL_PATH_TEST = "/data/service/el1/100/for-all-app/fonts/";
const std::string FONT_CONFIG_FILE_TEST = INSTALL_PATH_TEST + "install_fontconfig.json";
const std::string INIT_FONT_CONFIG_CONTENT = R"({"fontlist":[],"version":1})";
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
    FontManagerUtils::CheckAndInitInstallPath(INSTALL_PATH_TEST);
    FontManagerUtils::CreateFileWithPermission(FONT_CONFIG_FILE_TEST, INIT_FONT_CONFIG_CONTENT);
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

/**
 * @tc.name: FontConfigFuncTest015
 * @tc.desc: Test InsertFontRecord with missing config file
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest015, TestSize.Level1)
{
    FontManagerUtils::RemoveFile(FONT_CONFIG_FILE_TEST);
    std::vector<std::string> fullNames = {"TestFont"};
    EXPECT_FALSE(this->config_.InsertFontRecord("/data/test/font.ttf", fullNames));
}

/**
 * @tc.name: FontConfigFuncTest016
 * @tc.desc: Test InsertFontRecord with config missing fontlist key
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest016, TestSize.Level1)
{
    FontManagerUtils::CreateFileWithPermission(FONT_CONFIG_FILE_TEST, "{\"version\":1}");
    std::vector<std::string> fullNames = {"TestFont"};
    EXPECT_FALSE(this->config_.InsertFontRecord("/data/test/font.ttf", fullNames));
    FontManagerUtils::RemoveFile(FONT_CONFIG_FILE_TEST);
}

/**
 * @tc.name: FontConfigFuncTest017
 * @tc.desc: Test DeleteFontRecord with non-existent path
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest017, TestSize.Level1)
{
    std::string fontFullPath = INSTALL_PATH_TEST + "TestFont_Sans.ttf";
    std::vector<std::string> fullNames = {"TestFont"};
    ASSERT_TRUE(this->config_.InsertFontRecord(fontFullPath, fullNames));
    EXPECT_FALSE(this->config_.DeleteFontRecord("/data/test/nonexistent.ttf"));
}

/**
 * @tc.name: FontConfigFuncTest018
 * @tc.desc: Test CheckAndUpdateFontRecord with fontlist item missing fontfullpath
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest018, TestSize.Level1)
{
    std::string badConfig = "{\"fontlist\":[{\"other\":\"x\"}],\"version\":0}";
    FontManagerUtils::CreateFileWithPermission(FONT_CONFIG_FILE_TEST, badConfig);
    EXPECT_TRUE(this->config_.CheckAndUpdateFontRecord());
    FontManagerUtils::RemoveFile(FONT_CONFIG_FILE_TEST);
}

/**
 * @tc.name: FontConfigFuncTest019
 * @tc.desc: Test GetInstalledFontsNum with no config file
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest019, TestSize.Level1)
{
    FontManagerUtils::RemoveFile(FONT_CONFIG_FILE_TEST);
    FontConfig newConfig(FONT_CONFIG_FILE_TEST);
    EXPECT_EQ(newConfig.GetInstalledFontsNum(), 0);
}

/**
 * @tc.name: FontConfigFuncTest020
 * @tc.desc: Test GetFontFileByName with no config file
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest020, TestSize.Level1)
{
    FontManagerUtils::RemoveFile(FONT_CONFIG_FILE_TEST);
    FontConfig newConfig(FONT_CONFIG_FILE_TEST);
    std::string result = newConfig.GetFontFileByName("TestFont");
    EXPECT_EQ(result, "");
}

/**
 * @tc.name: FontConfigFuncTest021
 * @tc.desc: Test WriteToFile with null data
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest021, TestSize.Level1)
{
    FontConfig newConfig(FONT_CONFIG_FILE_TEST);
    EXPECT_FALSE(newConfig.WriteToFile(nullptr));
}

/**
 * @tc.name: FontConfigFuncTest022
 * @tc.desc: Test WriteToFile with unwritable path
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest022, TestSize.Level1)
{
    FontConfig newConfig("/data/test/nonexistent_dir_12345/config.json");
    const char srcData[] = "{\"test\":1}";
    char *data = static_cast<char *>(malloc(sizeof(srcData)));
    ASSERT_NE(data, nullptr);
    memcpy_s(data, sizeof(srcData), srcData, sizeof(srcData));
    EXPECT_FALSE(newConfig.WriteToFile(data));
}

/**
 * @tc.name: FontConfigFuncTest023
 * @tc.desc: Test GetFontsMap with valid config containing entries
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest023, TestSize.Level1)
{
    std::string fontFullPath = INSTALL_PATH_TEST + "TestFont_Sans.ttf";
    std::vector<std::string> fullName{"TestFont-Sans"};
    ASSERT_TRUE(this->config_.InsertFontRecord(fontFullPath, fullName));

    auto t = this->config_.GetFontsMap(this->config_.ConfigPath_);
    EXPECT_EQ(t.size(), 1u);
    EXPECT_EQ(t.find(fontFullPath) != t.end(), true);
    EXPECT_EQ(t[fontFullPath].size(), 1u);
    EXPECT_EQ(t[fontFullPath][0], "TestFont-Sans");
}

/**
 * @tc.name: FontConfigFuncTest024
 * @tc.desc: Test GetFontFileByName with multiple fonts and matching name
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest024, TestSize.Level1)
{
    std::string fontFullPath1 = INSTALL_PATH_TEST + "Font1.ttf";
    std::vector<std::string> fullName1{"Font-One"};
    std::string fontFullPath2 = INSTALL_PATH_TEST + "Font2.ttf";
    std::vector<std::string> fullName2{"Font-Two", "Font-2-Alt"};

    ASSERT_TRUE(this->config_.InsertFontRecord(fontFullPath1, fullName1));
    ASSERT_TRUE(this->config_.InsertFontRecord(fontFullPath2, fullName2));

    EXPECT_EQ(this->config_.GetFontFileByName("Font-One"), fontFullPath1);
    EXPECT_EQ(this->config_.GetFontFileByName("Font-Two"), fontFullPath2);
    EXPECT_EQ(this->config_.GetFontFileByName("Font-2-Alt"), fontFullPath2);
    EXPECT_EQ(this->config_.GetFontFileByName("NonExistent"), "");
}

/**
 * @tc.name: FontConfigFuncTest025
 * @tc.desc: Test DeleteFontRecord successful delete path
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest025, TestSize.Level1)
{
    std::string fontFullPath = INSTALL_PATH_TEST + "TestFont_Sans.ttf";
    std::vector<std::string> fullName{"TestFont-Sans"};
    ASSERT_TRUE(this->config_.InsertFontRecord(fontFullPath, fullName));
    EXPECT_EQ(this->config_.GetInstalledFontsNum(), 1);

    EXPECT_TRUE(this->config_.DeleteFontRecord(fontFullPath));
    EXPECT_EQ(this->config_.GetInstalledFontsNum(), 0);
    EXPECT_EQ(this->config_.GetFontFileByName("TestFont-Sans"), "");
}

/**
 * @tc.name: FontConfigFuncTest026
 * @tc.desc: Test DeleteFontRecord with TTC font and multiple names
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest026, TestSize.Level1)
{
    std::string fontFullPath = INSTALL_PATH_TEST + "NotoSansCJK-Regular.ttc";
    std::vector<std::string> fullName{
        "Noto Sans CJK JP", "Noto Sans CJK KR", "Noto Sans CJK SC"};
    ASSERT_TRUE(this->config_.InsertFontRecord(fontFullPath, fullName));
    EXPECT_EQ(this->config_.GetInstalledFontsNum(), 1);

    EXPECT_TRUE(this->config_.DeleteFontRecord(fontFullPath));
    EXPECT_EQ(this->config_.GetInstalledFontsNum(), 0);
    EXPECT_EQ(this->config_.GetFontFileByName("Noto Sans CJK SC"), "");
}

/**
 * @tc.name: FontConfigFuncTest027
 * @tc.desc: Test CheckAndUpdateFontRecord with non-version config migration
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest027, TestSize.Level1)
{
    FontManagerUtils::RemoveFile(FONT_CONFIG_FILE_TEST);
    std::string oldConfig = R"({"fontlist":[{"fontfullpath":"/data/test/dummy.ttf","fullname":["DummyFont"]}]})";
    FontManagerUtils::CreateFileWithPermission(FONT_CONFIG_FILE_TEST, oldConfig);
    EXPECT_TRUE(this->config_.CheckAndUpdateFontRecord());
}

/**
 * @tc.name: FontConfigFuncTest028
 * @tc.desc: Test InsertFontRecord with empty fullNames vector
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest028, TestSize.Level1)
{
    std::string fontFullPath = INSTALL_PATH_TEST + "EmptyFont.ttf";
    std::vector<std::string> emptyNames;
    EXPECT_TRUE(this->config_.InsertFontRecord(fontFullPath, emptyNames));
    EXPECT_EQ(this->config_.GetInstalledFontsNum(), 1);
}

/**
 * @tc.name: FontConfigFuncTest029
 * @tc.desc: Test GetFontsMap with empty config
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest029, TestSize.Level1)
{
    auto t = this->config_.GetFontsMap(this->config_.ConfigPath_);
    EXPECT_EQ(t.size(), 0u);
}

/**
 * @tc.name: FontConfigFuncTest030
 * @tc.desc: Test InsertFontRecord multiple times with same path
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest030, TestSize.Level1)
{
    std::string fontFullPath = INSTALL_PATH_TEST + "DupFont.ttf";
    std::vector<std::string> fullName1{"DupFont1"};
    std::vector<std::string> fullName2{"DupFont2"};
    EXPECT_TRUE(this->config_.InsertFontRecord(fontFullPath, fullName1));
    EXPECT_TRUE(this->config_.InsertFontRecord(fontFullPath, fullName2));
    EXPECT_EQ(this->config_.GetFontFileByName("DupFont1"), fontFullPath);
    EXPECT_EQ(this->config_.GetFontFileByName("DupFont2"), "");
}

HWTEST_F(FontConfigTest, FontConfigFuncTest031, TestSize.Level1)
{
    FontManagerUtils::RemoveFile(FONT_CONFIG_FILE_TEST);
    FontConfig newConfig(FONT_CONFIG_FILE_TEST);
    std::vector<std::string> fullNames = {"TestFont"};
    EXPECT_FALSE(newConfig.DeleteFontRecord("/data/test/font.ttf"));
}

HWTEST_F(FontConfigTest, FontConfigFuncTest032, TestSize.Level1)
{
    std::string fontFullPath = INSTALL_PATH_TEST + "TestFont_Sans.ttf";
    std::vector<std::string> fullName{"TestFont-Sans"};
    ASSERT_TRUE(this->config_.InsertFontRecord(fontFullPath, fullName));
    EXPECT_EQ(this->config_.GetInstalledFontsNum(), 1);
    this->config_.fontsMap_.clear();
    EXPECT_EQ(this->config_.GetInstalledFontsNum(), 1);
}

HWTEST_F(FontConfigTest, FontConfigFuncTest033, TestSize.Level1)
{
    std::string fontFullPath = INSTALL_PATH_TEST + "TestFont_Sans.ttf";
    std::vector<std::string> fullName{"TestFont-Sans"};
    ASSERT_TRUE(this->config_.InsertFontRecord(fontFullPath, fullName));
    this->config_.fontsMap_.clear();
    EXPECT_EQ(this->config_.GetFontFileByName("TestFont-Sans"), fontFullPath);
}

HWTEST_F(FontConfigTest, FontConfigFuncTest034, TestSize.Level1)
{
    std::string fontFullPath = INSTALL_PATH_TEST + "TestFont_Sans.ttf";
    std::vector<std::string> fullName{"TestFont-Sans"};
    ASSERT_TRUE(this->config_.InsertFontRecord(fontFullPath, fullName));
    this->config_.fontsMap_.clear();
    EXPECT_TRUE(this->config_.DeleteFontRecord(fontFullPath));
}

HWTEST_F(FontConfigTest, FontConfigFuncTest035, TestSize.Level1)
{
    FontManagerUtils::RemoveFile(FONT_CONFIG_FILE_TEST);
    FontConfig newConfig(FONT_CONFIG_FILE_TEST);
    std::vector<std::string> fullNames = {"TestFont"};
    EXPECT_FALSE(newConfig.DeleteFontRecord("/data/test/font.ttf"));
    EXPECT_EQ(newConfig.GetFontFileByName("TestFont"), "");
    EXPECT_EQ(newConfig.GetInstalledFontsNum(), 0);
}

HWTEST_F(FontConfigTest, FontConfigFuncTest036, TestSize.Level1)
{
    std::string fontFullPath = INSTALL_PATH_TEST + "TestFont_Sans.ttf";
    std::vector<std::string> fullName{"TestFont-Sans"};
    ASSERT_TRUE(this->config_.InsertFontRecord(fontFullPath, fullName));
    FontManagerUtils::RemoveFile(FONT_CONFIG_FILE_TEST);
    EXPECT_FALSE(this->config_.DeleteFontRecord(fontFullPath));
}

HWTEST_F(FontConfigTest, FontConfigFuncTest037, TestSize.Level1)
{
    std::string fontFullPath = INSTALL_PATH_TEST + "TestFont_Sans.ttf";
    std::vector<std::string> fullName{"TestFont-Sans"};
    ASSERT_TRUE(this->config_.InsertFontRecord(fontFullPath, fullName));
    FontManagerUtils::RemoveFile(FONT_CONFIG_FILE_TEST);
    EXPECT_FALSE(this->config_.InsertFontRecord(INSTALL_PATH_TEST + "Font2.ttf", {"Font2"}));
}

HWTEST_F(FontConfigTest, FontConfigFuncTest038, TestSize.Level1)
{
    std::string fontFullPath = INSTALL_PATH_TEST + "TestFont_Sans.ttf";
    std::vector<std::string> fullName{"TestFont-Sans"};
    ASSERT_TRUE(this->config_.InsertFontRecord(fontFullPath, fullName));
    FontManagerUtils::RemoveFile(FONT_CONFIG_FILE_TEST);
    EXPECT_FALSE(this->config_.CheckAndUpdateFontRecord());
}

HWTEST_F(FontConfigTest, FontConfigFuncTest039, TestSize.Level1)
{
    long size = 0;
    char* data = this->config_.GetFileData("/data/test/nonexistent_file_12345.txt", size);
    EXPECT_EQ(data, nullptr);
}

HWTEST_F(FontConfigTest, FontConfigFuncTest040, TestSize.Level1)
{
    std::string emptyPath = INSTALL_PATH_TEST + "empty_test.txt";
    FontManagerUtils::CreateFileWithPermission(emptyPath, "");
    long size = 0;
    char* data = this->config_.GetFileData(emptyPath, size);
    EXPECT_NE(data, nullptr);
    if (data) {
        free(data);
    }
    FontManagerUtils::RemoveFile(emptyPath);
}

HWTEST_F(FontConfigTest, FontConfigFuncTest041, TestSize.Level1)
{
    std::string content = "hello world";
    std::string testPath = INSTALL_PATH_TEST + "read_test.txt";
    FontManagerUtils::CreateFileWithPermission(testPath, content);
    long size = 0;
    char* data = this->config_.GetFileData(testPath, size);
    EXPECT_NE(data, nullptr);
    if (data) {
        std::string result;
        result.assign(data, size);
        EXPECT_EQ(result.substr(0, content.size()), content);
        free(data);
    }
    FontManagerUtils::RemoveFile(testPath);
}

HWTEST_F(FontConfigTest, FontConfigFuncTest042, TestSize.Level1)
{
    std::string fontFullPath = INSTALL_PATH_TEST + "TestFont_Sans.ttf";
    std::vector<std::string> fullName{"TestFont-Sans"};
    ASSERT_TRUE(this->config_.InsertFontRecord(fontFullPath, fullName));
    auto result = this->config_.GetFontsMap(this->config_.ConfigPath_);
    EXPECT_EQ(result.size(), 1u);
    EXPECT_EQ(result.begin()->second.size(), 1u);
    EXPECT_EQ(result.begin()->second[0], "TestFont-Sans");
}

HWTEST_F(FontConfigTest, FontConfigFuncTest043, TestSize.Level1)
{
    FontManagerUtils::RemoveFile(FONT_CONFIG_FILE_TEST);
    auto result = this->config_.GetFontsMap(this->config_.ConfigPath_);
    EXPECT_EQ(result.size(), 0u);
}

HWTEST_F(FontConfigTest, FontConfigFuncTest044, TestSize.Level1)
{
    FontManagerUtils::CreateFileWithPermission(FONT_CONFIG_FILE_TEST, "{\"version\":1}");
    auto result = this->config_.GetFontsMap(this->config_.ConfigPath_);
    EXPECT_EQ(result.size(), 0u);
}

HWTEST_F(FontConfigTest, FontConfigFuncTest045, TestSize.Level1)
{
    std::string fontFullPath = INSTALL_PATH_TEST + "TestFont_Sans.ttf";
    std::vector<std::string> fullName{"TestFont-Sans"};
    ASSERT_TRUE(this->config_.InsertFontRecord(fontFullPath, fullName));
    EXPECT_EQ(this->config_.GetFontFileByName("TestFont-Sans"), fontFullPath);
    EXPECT_EQ(this->config_.GetFontFileByName("NonExistent"), "");
    EXPECT_EQ(this->config_.GetFontFileByName(""), "");
}
}  // namespace FontManager
}  // namespace Global
}  // namespace OHOS
