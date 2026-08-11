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
#include <climits>

#include "font_manager_client.h"
#include "font_service_load_manager.h"
#include "font_sa_load_callback.h"
#include "data_migration_cb_agent.h"

#include "font_define.h"
#include "font_manager_utils.h"
#include "permission_common.h"

namespace {
const std::string FONT_PATH = "/data/test/TestFont_Sans.ttf";
const std::string INSTALL_PATH_TEST = "/data/service/el1/100/for-all-app/fonts/";
constexpr int32_t TEST_USERID = 100;
}

using testing::ext::TestSize;

namespace OHOS {
namespace Global {
namespace FontManager {

class FontManagerClientTest : public testing::Test {
public:
    FontManagerClientTest(){};
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void FontManagerClientTest::SetUpTestCase(void)
{
    FontManagerUtils::DeleteDir(INSTALL_PATH_TEST, false);
}

void FontManagerClientTest::TearDownTestCase(void)
{
    PermissionCommon::ResetTokenAndUid();
}

void FontManagerClientTest::SetUp(void)
{
}

void FontManagerClientTest::TearDown(void)
{
    FontManagerUtils::DeleteDir(INSTALL_PATH_TEST, false);
}

HWTEST_F(FontManagerClientTest, PathToRealPathTest001, TestSize.Level1)
{
    auto client = DelayedSingleton<FontManagerClient>::GetInstance();
    std::string realPath;
    EXPECT_FALSE(client->PathToRealPath("", realPath));
}

HWTEST_F(FontManagerClientTest, PathToRealPathTest002, TestSize.Level1)
{
    auto client = DelayedSingleton<FontManagerClient>::GetInstance();
    std::string realPath;
    std::string longPath(PATH_MAX, 'a');
    EXPECT_FALSE(client->PathToRealPath(longPath, realPath));
}

HWTEST_F(FontManagerClientTest, PathToRealPathTest003, TestSize.Level1)
{
    auto client = DelayedSingleton<FontManagerClient>::GetInstance();
    std::string realPath;
    EXPECT_FALSE(client->PathToRealPath("/data/test/nonexistent_path_12345.ttf", realPath));
}

HWTEST_F(FontManagerClientTest, PathToRealPathTest004, TestSize.Level1)
{
    auto client = DelayedSingleton<FontManagerClient>::GetInstance();
    std::string realPath;
    EXPECT_TRUE(client->PathToRealPath(FONT_PATH, realPath));
    EXPECT_FALSE(realPath.empty());
}

HWTEST_F(FontManagerClientTest, InstallFontTest001, TestSize.Level1)
{
    auto client = DelayedSingleton<FontManagerClient>::GetInstance();
    int outValue = 0;
    int32_t ret = client->InstallFont("", outValue);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(outValue, ERR_FILE_NOT_EXISTS);
}

HWTEST_F(FontManagerClientTest, InstallFontTest002, TestSize.Level1)
{
    auto client = DelayedSingleton<FontManagerClient>::GetInstance();
    int outValue = 0;
    int32_t ret = client->InstallFont("/data/test/nonexistent_12345.ttf", outValue);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(outValue, ERR_FILE_NOT_EXISTS);
}

HWTEST_F(FontManagerClientTest, InstallFontTest003, TestSize.Level1)
{
    auto client = DelayedSingleton<FontManagerClient>::GetInstance();
    int outValue = 0;
    std::string longPath(PATH_MAX, 'a');
    int32_t ret = client->InstallFont(longPath, outValue);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(outValue, ERR_FILE_NOT_EXISTS);
}

HWTEST_F(FontManagerClientTest, InstallFontTest004, TestSize.Level1)
{
    auto client = DelayedSingleton<FontManagerClient>::GetInstance();
    int outValue = 0;
    int32_t ret = client->InstallFont(FONT_PATH, outValue);
    EXPECT_NE(ret, ERR_FILE_NOT_EXISTS);
}

HWTEST_F(FontManagerClientTest, UninstallFontTest001, TestSize.Level1)
{
    auto client = DelayedSingleton<FontManagerClient>::GetInstance();
    int outValue = 0;
    int32_t ret = client->UninstallFont("TestFont", outValue);
    EXPECT_NE(ret, ERR_FILE_NOT_EXISTS);
}

HWTEST_F(FontManagerClientTest, DataMigrationTest001, TestSize.Level1)
{
    auto client = DelayedSingleton<FontManagerClient>::GetInstance();
    int32_t ret = client->DataMigration(nullptr);
    EXPECT_NE(ret, ERR_OK);
}

HWTEST_F(FontManagerClientTest, InstallFontWithUserIdTest001, TestSize.Level1)
{
    auto client = DelayedSingleton<FontManagerClient>::GetInstance();
    int32_t ret = client->InstallFontWithUserId("", TEST_USERID);
    EXPECT_EQ(ret, ERR_FILE_NOT_EXISTS);
}

HWTEST_F(FontManagerClientTest, InstallFontWithUserIdTest002, TestSize.Level1)
{
    auto client = DelayedSingleton<FontManagerClient>::GetInstance();
    int32_t ret = client->InstallFontWithUserId("/data/test/nonexistent_12345.ttf", TEST_USERID);
    EXPECT_EQ(ret, ERR_FILE_NOT_EXISTS);
}

HWTEST_F(FontManagerClientTest, InstallFontWithUserIdTest003, TestSize.Level1)
{
    auto client = DelayedSingleton<FontManagerClient>::GetInstance();
    int32_t ret = client->InstallFontWithUserId(FONT_PATH, TEST_USERID);
    EXPECT_NE(ret, ERR_FILE_NOT_EXISTS);
}

HWTEST_F(FontManagerClientTest, UninstallFontWithUserIdTest001, TestSize.Level1)
{
    auto client = DelayedSingleton<FontManagerClient>::GetInstance();
    int32_t ret = client->UninstallFontWithUserId("TestFont", TEST_USERID);
    EXPECT_NE(ret, ERR_FILE_NOT_EXISTS);
}

HWTEST_F(FontManagerClientTest, FontServiceLoadManagerTest001, TestSize.Level1)
{
    auto loadMgr = FontServiceLoadManager::GetInstance();
    sptr<IFontService> result = loadMgr->GetFontServiceAbility(FONT_SA_ID);
    EXPECT_NE(result, nullptr);
}

HWTEST_F(FontManagerClientTest, FontServiceLoadManagerTest002, TestSize.Level1)
{
    auto loadMgr = FontServiceLoadManager::GetInstance();
    loadMgr->OnLoadSystemAbilitySuccess();
    EXPECT_EQ(loadMgr->loadSaStatus_, LoadSaStatus::SUCCESS);
}

HWTEST_F(FontManagerClientTest, FontServiceLoadManagerTest003, TestSize.Level1)
{
    auto loadMgr = FontServiceLoadManager::GetInstance();
    loadMgr->OnLoadSystemAbilityFail();
    EXPECT_EQ(loadMgr->loadSaStatus_, LoadSaStatus::FAIL);
}

HWTEST_F(FontManagerClientTest, FontServiceLoadManagerTest004, TestSize.Level1)
{
    auto loadMgr = FontServiceLoadManager::GetInstance();
    loadMgr->InitStatus();
    EXPECT_EQ(loadMgr->loadSaStatus_, LoadSaStatus::WAIT_RESULT);
}

HWTEST_F(FontManagerClientTest, FontServiceLoadManagerTest005, TestSize.Level1)
{
    auto loadMgr = FontServiceLoadManager::GetInstance();
    bool ret = loadMgr->UnloadFontService(FONT_SA_ID);
    EXPECT_FALSE(ret);
}

HWTEST_F(FontManagerClientTest, FontSALoadCallbackTest001, TestSize.Level1)
{
    sptr<FontSALoadCallback> callback = new FontSALoadCallback();
    sptr<IRemoteObject> remoteObj = nullptr;
    callback->OnLoadSystemAbilitySuccess(FONT_SA_ID, remoteObj);
    auto loadMgr = FontServiceLoadManager::GetInstance();
    EXPECT_EQ(loadMgr->loadSaStatus_, LoadSaStatus::SUCCESS);
}

HWTEST_F(FontManagerClientTest, FontSALoadCallbackTest002, TestSize.Level1)
{
    sptr<FontSALoadCallback> callback = new FontSALoadCallback();
    callback->OnLoadSystemAbilityFail(FONT_SA_ID);
    auto loadMgr = FontServiceLoadManager::GetInstance();
    EXPECT_EQ(loadMgr->loadSaStatus_, LoadSaStatus::FAIL);
}

HWTEST_F(FontManagerClientTest, DataMigrationCbAgentTest001, TestSize.Level1)
{
    sptr<DataMigrationCbAgent> agent = new DataMigrationCbAgent(nullptr);
    EventData eventData;
    ErrCode result = agent->Handle(eventData);
    EXPECT_EQ(result, ERR_SYSTEM_ERROR);
}

HWTEST_F(FontManagerClientTest, DataMigrationCbAgentTest002, TestSize.Level1)
{
    class TestListener : public IDataMigrationListener {
    public:
        void OnHandle(const EventData& eventData) override {}
    };
    auto listener = std::make_shared<TestListener>();
    sptr<DataMigrationCbAgent> agent = new DataMigrationCbAgent(listener);
    EventData eventData;
    ErrCode result = agent->Handle(eventData);
    EXPECT_EQ(result, ERR_OK);
}
} // namespace FontManager
} // namespace Global
} // namespace OHOS
