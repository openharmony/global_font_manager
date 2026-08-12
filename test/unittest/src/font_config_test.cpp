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
#include "font_define.h"
#include "font_manager.h"
#include "font_manager_utils.h"
#undef private
#undef protected
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
    ASSERT_EQ(FontManagerUtils::CheckAndInitInstallPath(INSTALL_PATH_TEST), true);
    std::string fontFullPath = INSTALL_PATH_TEST + "TestFont_Sans.ttf";
    std::vector<std::string> fullName{"TestFont-Sans"};

    EXPECT_EQ(this->config_.InsertFontRecord(fontFullPath, fullName), true);
    EXPECT_EQ(this->config_.GetInstalledFontsNum() > 0, true);
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
    EXPECT_EQ(this->config_.GetFontFileByName("TestFont-Sans") == fontFullPath, true);

    EXPECT_EQ(this->config_.DeleteFontRecord(fontFullPath), true);
    EXPECT_EQ(this->config_.GetFontFileByName("TestFont-Sans") == "", true);
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
    EXPECT_EQ(this->config_.GetFontFileByName("Noto Sans CJK JP") == fontFullPath, true);
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
    EXPECT_EQ(this->config_.GetFontFileByName("Noto Sans CJK JP") == fontFullPath, true);

    EXPECT_EQ(this->config_.DeleteFontRecord(fontFullPath), true);
    EXPECT_EQ(this->config_.GetFontFileByName("Noto Sans CJK JP") == "", true);
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
    EXPECT_EQ(this->config_.GetInstalledFontsNum() > 0, true);
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
 * @tc.desc: Test CheckAndUpdateFontRecord where fontlist is not an array (only version is checked)
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
    EXPECT_TRUE(this->config_.CheckAndUpdateFontRecord());
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
 * @tc.desc: Test InsertScopeFontRecord and GetFontRecordByUrl
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest015, TestSize.Level1)
{
    ASSERT_EQ(FontManagerUtils::CheckAndInitInstallPath(INSTALL_PATH_TEST), true);
    FontRecordInfo record;
    record.fontPath = INSTALL_PATH_TEST + "app_100/font.ttf";
    record.fullNames = {"TestAppFont"};
    record.scope = FONT_SCOPE_APP;
    record.srcPath = "file://test/font.ttf";
    record.appIdentifier = "app_100";
    record.bundleName = "com.example.app";

    EXPECT_EQ(this->config_.InsertScopeFontRecord(record), true);

    auto result = this->config_.GetFontRecordByUrl("file://test/font.ttf");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->scope, FONT_SCOPE_APP);
    EXPECT_EQ(result->fontPath, record.fontPath);
    EXPECT_EQ(result->appIdentifier, "app_100");
}

/**
 * @tc.name: FontConfigFuncTest016
 * @tc.desc: Test DeleteScopeFontRecordByUrl
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest016, TestSize.Level1)
{
    ASSERT_EQ(FontManagerUtils::CheckAndInitInstallPath(INSTALL_PATH_TEST), true);
    FontRecordInfo record;
    record.fontPath = INSTALL_PATH_TEST + "app_100/font.ttf";
    record.fullNames = {"TestAppFont"};
    record.scope = FONT_SCOPE_APP;
    record.srcPath = "file://test/font2.ttf";
    record.appIdentifier = "app_100";
    record.bundleName = "com.example.app";

    EXPECT_EQ(this->config_.InsertScopeFontRecord(record), true);
    EXPECT_EQ(this->config_.DeleteScopeFontRecordByUrl("file://test/font2.ttf"), true);
    EXPECT_FALSE(this->config_.GetFontRecordByUrl("file://test/font2.ttf").has_value());
}

/**
 * @tc.name: FontConfigFuncTest017
 * @tc.desc: Test GetFontRecordsByAppId
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest017, TestSize.Level1)
{
    ASSERT_EQ(FontManagerUtils::CheckAndInitInstallPath(INSTALL_PATH_TEST), true);
    FontRecordInfo record1;
    record1.fontPath = INSTALL_PATH_TEST + "app_100/font1.ttf";
    record1.fullNames = {"AppFont1"};
    record1.scope = FONT_SCOPE_APP;
    record1.srcPath = "file://test/font1.ttf";
    record1.appIdentifier = "app_100";
    record1.bundleName = "com.example.app";

    FontRecordInfo record2;
    record2.fontPath = INSTALL_PATH_TEST + "app_100/font2.ttf";
    record2.fullNames = {"AppFont2"};
    record2.scope = FONT_SCOPE_APP;
    record2.srcPath = "file://test/font2.ttf";
    record2.appIdentifier = "app_100";
    record2.bundleName = "com.example.app";

    FontRecordInfo record3;
    record3.fontPath = INSTALL_PATH_TEST + "session_200/font3.ttf";
    record3.fullNames = {"SessionFont1"};
    record3.scope = FONT_SCOPE_SESSION;
    record3.srcPath = "file://test/font3.ttf";
    record3.appIdentifier = "session_200";
    record3.bundleName = "com.example.session";

    EXPECT_EQ(this->config_.InsertScopeFontRecord(record1), true);
    EXPECT_EQ(this->config_.InsertScopeFontRecord(record2), true);
    EXPECT_EQ(this->config_.InsertScopeFontRecord(record3), true);

    auto appRecords = this->config_.GetFontRecordsByAppId("app_100");
    EXPECT_EQ(appRecords.size(), 2u);

    auto sessionRecords = this->config_.GetFontRecordsByAppId("session_200");
    EXPECT_EQ(sessionRecords.size(), 1u);
}

/**
 * @tc.name: FontConfigFuncTest018
 * @tc.desc: Test GetScopeFontRecords returns both app and session scope
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest018, TestSize.Level1)
{
    ASSERT_EQ(FontManagerUtils::CheckAndInitInstallPath(INSTALL_PATH_TEST), true);
    FontRecordInfo appRecord;
    appRecord.fontPath = INSTALL_PATH_TEST + "app_100/app_font.ttf";
    appRecord.fullNames = {"AppFont"};
    appRecord.scope = FONT_SCOPE_APP;
    appRecord.srcPath = "file://test/app.ttf";
    appRecord.appIdentifier = "app_100";

    FontRecordInfo sessionRecord;
    sessionRecord.fontPath = INSTALL_PATH_TEST + "session_200/session_font.ttf";
    sessionRecord.fullNames = {"SessionFont"};
    sessionRecord.scope = FONT_SCOPE_SESSION;
    sessionRecord.srcPath = "file://test/session.ttf";
    sessionRecord.appIdentifier = "session_200";

    this->config_.InsertScopeFontRecord(appRecord);
    this->config_.InsertScopeFontRecord(sessionRecord);

    auto scopeRecords = this->config_.GetScopeFontRecords();
    EXPECT_EQ(scopeRecords.size(), 2u);
}

/**
 * @tc.name: FontConfigFuncTest019
 * @tc.desc: Test GetAppScopeFontRecords returns only app scope (scope=0)
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest019, TestSize.Level1)
{
    ASSERT_EQ(FontManagerUtils::CheckAndInitInstallPath(INSTALL_PATH_TEST), true);
    FontRecordInfo appRecord;
    appRecord.fontPath = INSTALL_PATH_TEST + "app_100/app_font.ttf";
    appRecord.fullNames = {"AppFont"};
    appRecord.scope = FONT_SCOPE_APP;
    appRecord.srcPath = "file://test/app.ttf";
    appRecord.appIdentifier = "app_100";

    FontRecordInfo sessionRecord;
    sessionRecord.fontPath = INSTALL_PATH_TEST + "session_200/session_font.ttf";
    sessionRecord.fullNames = {"SessionFont"};
    sessionRecord.scope = FONT_SCOPE_SESSION;
    sessionRecord.srcPath = "file://test/session.ttf";
    sessionRecord.appIdentifier = "session_200";

    this->config_.InsertScopeFontRecord(appRecord);
    this->config_.InsertScopeFontRecord(sessionRecord);

    auto appRecords = this->config_.GetAppScopeFontRecords();
    EXPECT_EQ(appRecords.size(), 1u);
    EXPECT_EQ(appRecords[0].scope, FONT_SCOPE_APP);
}

/**
 * @tc.name: FontConfigFuncTest020
 * @tc.desc: Test GetFontRecordByName finds scope font by fullName
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest020, TestSize.Level1)
{
    ASSERT_EQ(FontManagerUtils::CheckAndInitInstallPath(INSTALL_PATH_TEST), true);
    FontRecordInfo record;
    record.fontPath = INSTALL_PATH_TEST + "app_100/font.ttf";
    record.fullNames = {"UniqueScopeFont"};
    record.scope = FONT_SCOPE_APP;
    record.srcPath = "file://test/unique.ttf";
    record.appIdentifier = "app_100";

    this->config_.InsertScopeFontRecord(record);

    auto result = this->config_.GetFontRecordByName("UniqueScopeFont");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->scope, FONT_SCOPE_APP);
    EXPECT_EQ(result->srcPath, "file://test/unique.ttf");
}

/**
 * @tc.name: FontConfigFuncTest021
 * @tc.desc: Test GetTotalInstalledFontsNum counts all records
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest021, TestSize.Level1)
{
    ASSERT_EQ(FontManagerUtils::CheckAndInitInstallPath(INSTALL_PATH_TEST), true);
    this->config_.InsertFontRecord(INSTALL_PATH_TEST + "user_font.ttf", {"UserFont"});

    FontRecordInfo scopeRecord;
    scopeRecord.fontPath = INSTALL_PATH_TEST + "app_100/scope_font.ttf";
    scopeRecord.fullNames = {"ScopeFont"};
    scopeRecord.scope = FONT_SCOPE_APP;
    scopeRecord.srcPath = "file://test/scope.ttf";
    scopeRecord.appIdentifier = "app_100";
    this->config_.InsertScopeFontRecord(scopeRecord);

    EXPECT_EQ(this->config_.GetTotalInstalledFontsNum(), 2);
}

/**
 * @tc.name: FontConfigFuncTest022
 * @tc.desc: Test DeleteScopeFontRecordByAppId removes all records for an app
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest022, TestSize.Level1)
{
    ASSERT_EQ(FontManagerUtils::CheckAndInitInstallPath(INSTALL_PATH_TEST), true);
    FontRecordInfo r1;
    r1.fontPath = INSTALL_PATH_TEST + "app_100/f1.ttf";
    r1.fullNames = {"F1"};
    r1.scope = FONT_SCOPE_APP;
    r1.srcPath = "file://test/f1.ttf";
    r1.appIdentifier = "app_100";

    FontRecordInfo r2;
    r2.fontPath = INSTALL_PATH_TEST + "app_100/f2.ttf";
    r2.fullNames = {"F2"};
    r2.scope = FONT_SCOPE_APP;
    r2.srcPath = "file://test/f2.ttf";
    r2.appIdentifier = "app_100";

    this->config_.InsertScopeFontRecord(r1);
    this->config_.InsertScopeFontRecord(r2);

    EXPECT_EQ(this->config_.DeleteScopeFontRecordByAppId("app_100"), true);
    EXPECT_EQ(this->config_.GetFontRecordsByAppId("app_100").size(), 0u);
}

/**
 * @tc.name: FontConfigFuncTest023
 * @tc.desc: Test legacy records (no scope field) are not returned by GetAppScopeFontRecords
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest023, TestSize.Level1)
{
    ASSERT_EQ(FontManagerUtils::CheckAndInitInstallPath(INSTALL_PATH_TEST), true);
    this->config_.InsertFontRecord(INSTALL_PATH_TEST + "legacy.ttf", {"LegacyFont"});

    auto appRecords = this->config_.GetAppScopeFontRecords();
    EXPECT_EQ(appRecords.size(), 0u);

    auto scopeRecords = this->config_.GetScopeFontRecords();
    EXPECT_EQ(scopeRecords.size(), 0u);
}

/**
 * @tc.name: FontConfigFuncTest024
 * @tc.desc: Test CheckAndUpdateFontRecord upgrades version 1 to 7.0
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest024, TestSize.Level1)
{
    std::string v1Json = "{\"fontlist\":[],\"version\":1}";
    FILE *fp = fopen(FONT_CONFIG_FILE_TEST.c_str(), "w");
    if (fp) {
        fwrite(v1Json.c_str(), 1, v1Json.length(), fp);
        (void)fclose(fp);
    }
    EXPECT_TRUE(this->config_.CheckAndUpdateFontRecord());
    EXPECT_TRUE(this->config_.CheckAndUpdateFontRecord());
}

/**
 * @tc.name: FontConfigFuncTest025
 * @tc.desc: Test initial config file has version "7.0"
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest025, TestSize.Level1)
{
    FontManagerUtils::RemoveFile(FONT_CONFIG_FILE_TEST);
    ASSERT_EQ(FontManagerUtils::CheckFontConfigPath(INSTALL_PATH_TEST), true);
    EXPECT_TRUE(this->config_.CheckAndUpdateFontRecord());
}

/**
 * @tc.name: FontConfigFuncTest026
 * @tc.desc: Test InsertFontRecord with missing config file
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest026, TestSize.Level1)
{
    FontManagerUtils::RemoveFile(FONT_CONFIG_FILE_TEST);
    std::vector<std::string> fullNames = {"TestFont"};
    EXPECT_FALSE(this->config_.InsertFontRecord("/data/test/font.ttf", fullNames));
}

/**
 * @tc.name: FontConfigFuncTest027
 * @tc.desc: Test InsertFontRecord with config missing fontlist key
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest027, TestSize.Level1)
{
    FontManagerUtils::CreateFileWithPermission(FONT_CONFIG_FILE_TEST, "{\"version\":1}");
    std::vector<std::string> fullNames = {"TestFont"};
    EXPECT_FALSE(this->config_.InsertFontRecord("/data/test/font.ttf", fullNames));
    FontManagerUtils::RemoveFile(FONT_CONFIG_FILE_TEST);
}

/**
 * @tc.name: FontConfigFuncTest028
 * @tc.desc: Test DeleteFontRecord with non-existent path
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest028, TestSize.Level1)
{
    std::string fontFullPath = INSTALL_PATH_TEST + "TestFont_Sans.ttf";
    std::vector<std::string> fullNames = {"TestFont"};
    ASSERT_TRUE(this->config_.InsertFontRecord(fontFullPath, fullNames));
    EXPECT_FALSE(this->config_.DeleteFontRecord("/data/test/nonexistent.ttf"));
}

/**
 * @tc.name: FontConfigFuncTest029
 * @tc.desc: Test CheckAndUpdateFontRecord with fontlist item missing fontfullpath
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest029, TestSize.Level1)
{
    std::string badConfig = "{\"fontlist\":[{\"other\":\"x\"}],\"version\":0}";
    FontManagerUtils::CreateFileWithPermission(FONT_CONFIG_FILE_TEST, badConfig);
    EXPECT_TRUE(this->config_.CheckAndUpdateFontRecord());
    FontManagerUtils::RemoveFile(FONT_CONFIG_FILE_TEST);
}

/**
 * @tc.name: FontConfigFuncTest030
 * @tc.desc: Test GetInstalledFontsNum with no config file
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest030, TestSize.Level1)
{
    FontManagerUtils::RemoveFile(FONT_CONFIG_FILE_TEST);
    FontConfig newConfig(FONT_CONFIG_FILE_TEST);
    EXPECT_EQ(newConfig.GetInstalledFontsNum(), 0);
}

/**
 * @tc.name: FontConfigFuncTest031
 * @tc.desc: Test GetFontFileByName with no config file
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest031, TestSize.Level1)
{
    FontManagerUtils::RemoveFile(FONT_CONFIG_FILE_TEST);
    FontConfig newConfig(FONT_CONFIG_FILE_TEST);
    std::string result = newConfig.GetFontFileByName("TestFont");
    EXPECT_EQ(result, "");
}

/**
 * @tc.name: FontConfigFuncTest032
 * @tc.desc: Test WriteToFile with null data
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest032, TestSize.Level1)
{
    FontConfig newConfig(FONT_CONFIG_FILE_TEST);
    EXPECT_FALSE(newConfig.WriteToFile(nullptr));
}

/**
 * @tc.name: FontConfigFuncTest033
 * @tc.desc: Test WriteToFile with unwritable path
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest033, TestSize.Level1)
{
    FontConfig newConfig("/data/test/nonexistent_dir_12345/config.json");
    const char srcData[] = "{\"test\":1}";
    char *data = static_cast<char *>(malloc(sizeof(srcData)));
    ASSERT_NE(data, nullptr);
    memcpy_s(data, sizeof(srcData), srcData, sizeof(srcData));
    EXPECT_FALSE(newConfig.WriteToFile(data));
}

/**
 * @tc.name: FontConfigFuncTest034
 * @tc.desc: Test GetFontFileByName with multiple fonts and matching name
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest034, TestSize.Level1)
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
 * @tc.name: FontConfigFuncTest035
 * @tc.desc: Test DeleteFontRecord successful delete path
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest035, TestSize.Level1)
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
 * @tc.name: FontConfigFuncTest036
 * @tc.desc: Test DeleteFontRecord with TTC font and multiple names
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest036, TestSize.Level1)
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
 * @tc.name: FontConfigFuncTest037
 * @tc.desc: Test CheckAndUpdateFontRecord with non-version config migration
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest037, TestSize.Level1)
{
    FontManagerUtils::RemoveFile(FONT_CONFIG_FILE_TEST);
    std::string oldConfig = R"({"fontlist":[{"fontfullpath":"/data/test/dummy.ttf","fullname":["DummyFont"]}]})";
    FontManagerUtils::CreateFileWithPermission(FONT_CONFIG_FILE_TEST, oldConfig);
    EXPECT_TRUE(this->config_.CheckAndUpdateFontRecord());
}

/**
 * @tc.name: FontConfigFuncTest038
 * @tc.desc: Test InsertFontRecord with empty fullNames vector
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest038, TestSize.Level1)
{
    std::string fontFullPath = INSTALL_PATH_TEST + "EmptyFont.ttf";
    std::vector<std::string> emptyNames;
    EXPECT_TRUE(this->config_.InsertFontRecord(fontFullPath, emptyNames));
    EXPECT_EQ(this->config_.GetInstalledFontsNum(), 1);
}

/**
 * @tc.name: FontConfigFuncTest039
 * @tc.desc: Test InsertFontRecord multiple times with same path
 * @tc.type: FUNC
 */
HWTEST_F(FontConfigTest, FontConfigFuncTest039, TestSize.Level1)
{
    std::string fontFullPath = INSTALL_PATH_TEST + "DupFont.ttf";
    std::vector<std::string> fullName1{"DupFont1"};
    std::vector<std::string> fullName2{"DupFont2"};
    EXPECT_TRUE(this->config_.InsertFontRecord(fontFullPath, fullName1));
    EXPECT_TRUE(this->config_.InsertFontRecord(fontFullPath, fullName2));
    EXPECT_EQ(this->config_.GetFontFileByName("DupFont1"), fontFullPath);
    EXPECT_EQ(this->config_.GetFontFileByName("DupFont2"), "");
}

HWTEST_F(FontConfigTest, FontConfigFuncTest040, TestSize.Level1)
{
    FontManagerUtils::RemoveFile(FONT_CONFIG_FILE_TEST);
    FontConfig newConfig(FONT_CONFIG_FILE_TEST);
    std::vector<std::string> fullNames = {"TestFont"};
    EXPECT_FALSE(newConfig.DeleteFontRecord("/data/test/font.ttf"));
}

HWTEST_F(FontConfigTest, FontConfigFuncTest041, TestSize.Level1)
{
    FontManagerUtils::RemoveFile(FONT_CONFIG_FILE_TEST);
    FontConfig newConfig(FONT_CONFIG_FILE_TEST);
    std::vector<std::string> fullNames = {"TestFont"};
    EXPECT_FALSE(newConfig.DeleteFontRecord("/data/test/font.ttf"));
    EXPECT_EQ(newConfig.GetFontFileByName("TestFont"), "");
    EXPECT_EQ(newConfig.GetInstalledFontsNum(), 0);
}

HWTEST_F(FontConfigTest, FontConfigFuncTest042, TestSize.Level1)
{
    std::string fontFullPath = INSTALL_PATH_TEST + "TestFont_Sans.ttf";
    std::vector<std::string> fullName{"TestFont-Sans"};
    ASSERT_TRUE(this->config_.InsertFontRecord(fontFullPath, fullName));
    FontManagerUtils::RemoveFile(FONT_CONFIG_FILE_TEST);
    EXPECT_FALSE(this->config_.DeleteFontRecord(fontFullPath));
}

HWTEST_F(FontConfigTest, FontConfigFuncTest043, TestSize.Level1)
{
    std::string fontFullPath = INSTALL_PATH_TEST + "TestFont_Sans.ttf";
    std::vector<std::string> fullName{"TestFont-Sans"};
    ASSERT_TRUE(this->config_.InsertFontRecord(fontFullPath, fullName));
    FontManagerUtils::RemoveFile(FONT_CONFIG_FILE_TEST);
    EXPECT_FALSE(this->config_.InsertFontRecord(INSTALL_PATH_TEST + "Font2.ttf", {"Font2"}));
}

HWTEST_F(FontConfigTest, FontConfigFuncTest044, TestSize.Level1)
{
    std::string fontFullPath = INSTALL_PATH_TEST + "TestFont_Sans.ttf";
    std::vector<std::string> fullName{"TestFont-Sans"};
    ASSERT_TRUE(this->config_.InsertFontRecord(fontFullPath, fullName));
    FontManagerUtils::RemoveFile(FONT_CONFIG_FILE_TEST);
    EXPECT_FALSE(this->config_.CheckAndUpdateFontRecord());
}

HWTEST_F(FontConfigTest, FontConfigFuncTest045, TestSize.Level1)
{
    long size = 0;
    char* data = this->config_.GetFileData("/data/test/nonexistent_file_12345.txt", size);
    EXPECT_EQ(data, nullptr);
}

HWTEST_F(FontConfigTest, FontConfigFuncTest046, TestSize.Level1)
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

HWTEST_F(FontConfigTest, FontConfigFuncTest047, TestSize.Level1)
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

HWTEST_F(FontConfigTest, FontConfigFuncTest048, TestSize.Level1)
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
