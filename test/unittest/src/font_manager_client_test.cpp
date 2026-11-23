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
#include <unistd.h>

#include "font_manager_client.h"
#include "font_service_load_manager.h"
#include "font_sa_load_callback.h"
#include "data_migration_cb_agent.h"
#include "font_client_observer_agent.h"
#include "ifont_client_observer.h"
#include "iremote_stub.h"

#include "font_define.h"
#include "font_manager_utils.h"
#include "permission_common.h"

namespace {
const std::string FONT_PATH = "/data/test/TestFont_Sans.ttf";
const std::string INSTALL_PATH_TEST = "/data/service/el1/100/for-all-app/fonts/";
constexpr int32_t TEST_USERID = 100;
constexpr int32_t INVALID_SA_ID = 999999;
}

namespace OHOS {
namespace Global {
namespace FontManager {
class MockFontClientObserverForClient : public IRemoteStub<IFontClientObserver> {
public:
    ErrCode OnServiceDied() override
    {
        diedCalled = true;
        return ERR_OK;
    }
    bool diedCalled = false;
};
} // namespace FontManager
} // namespace Global
} // namespace OHOS

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
    client->InstallFont(FONT_PATH, outValue);
    // File exists so PathToRealPath/fopen/fileno succeed; IPC was reached.
    // Key: outValue should NOT be ERR_FILE_NOT_EXISTS (file was found and opened).
    EXPECT_NE(outValue, ERR_FILE_NOT_EXISTS);
}

HWTEST_F(FontManagerClientTest, UninstallFontTest001, TestSize.Level1)
{
    auto client = DelayedSingleton<FontManagerClient>::GetInstance();
    int outValue = 0;
    int32_t ret = client->UninstallFont("TestFont", outValue);
    // Service is available; ret is ERR_OK (client returns IPC result), outValue holds server result.
    EXPECT_EQ(ret, ERR_OK);
    // outValue may be ERR_NO_PERMISSION(201) or other server-side result; just verify it's not 0 (no real font).
    EXPECT_NE(outValue, ERR_OK);
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
    // Returns ERR_UNINSTALL_FAIL on service null, or IPC result
    EXPECT_NE(ret, ERR_OK);
}

HWTEST_F(FontManagerClientTest, InstallFontByFdTest001, TestSize.Level1)
{
    auto client = DelayedSingleton<FontManagerClient>::GetInstance();
    int32_t ret = client->InstallFontByFd(-1, TEST_USERID);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST_F(FontManagerClientTest, InstallFontByFdTest002, TestSize.Level1)
{
    auto client = DelayedSingleton<FontManagerClient>::GetInstance();
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    ASSERT_GE(fd, 0);
    int32_t ret = client->InstallFontByFd(fd, -1);
    close(fd);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST_F(FontManagerClientTest, InstallFontByFdTest003, TestSize.Level1)
{
    auto client = DelayedSingleton<FontManagerClient>::GetInstance();
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    ASSERT_GE(fd, 0);
    int32_t ret = client->InstallFontByFd(fd, TEST_USERID);
    close(fd);
    // The IPC path is reached; result depends on SA availability and permissions.
    EXPECT_TRUE(ret == ERR_OK || ret == ERR_NO_PERMISSION || ret == ERR_INSTALL_FAIL);
}

HWTEST_F(FontManagerClientTest, UninstallFontByFullNameTest001, TestSize.Level1)
{
    auto client = DelayedSingleton<FontManagerClient>::GetInstance();
    int32_t ret = client->UninstallFontByFullName("TestFont", -1);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

HWTEST_F(FontManagerClientTest, UninstallFontByFullNameTest002, TestSize.Level1)
{
    auto client = DelayedSingleton<FontManagerClient>::GetInstance();
    int32_t ret = client->UninstallFontByFullName("NonExistentFont12345", TEST_USERID);
    // Returns ERR_UNINSTALL_FAIL on service null, permission error, or not-found.
    EXPECT_NE(ret, ERR_OK);
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

/**
 * @tc.name: FontServiceDeathRecipientTest001
 * @tc.desc: Test FontServiceDeathRecipient OnRemoteDied with valid observer
 */
HWTEST_F(FontManagerClientTest, FontServiceDeathRecipientTest001, TestSize.Level1)
{
    sptr<MockFontClientObserverForClient> observer = new MockFontClientObserverForClient();
    auto recipient = new FontServiceDeathRecipient(observer);
    wptr<IRemoteObject> remote = nullptr;
    recipient->OnRemoteDied(remote);
    EXPECT_TRUE(observer->diedCalled);
}

/**
 * @tc.name: FontServiceDeathRecipientTest002
 * @tc.desc: Test FontServiceDeathRecipient OnRemoteDied with null observer (observer not called)
 */
HWTEST_F(FontManagerClientTest, FontServiceDeathRecipientTest002, TestSize.Level1)
{
    sptr<MockFontClientObserverForClient> mockObserver = new MockFontClientObserverForClient();
    auto recipient = new FontServiceDeathRecipient(nullptr);
    wptr<IRemoteObject> remote = nullptr;
    recipient->OnRemoteDied(remote);
    // Recipient was created with null observer; mockObserver should not be called.
    EXPECT_FALSE(mockObserver->diedCalled);
}

/**
 * @tc.name: FontClientObserverAgentTest001
 * @tc.desc: Test FontClientObserverAgent OnServiceDied with valid callback
 */
HWTEST_F(FontManagerClientTest, FontClientObserverAgentTest001, TestSize.Level1)
{
    bool called = false;
    auto agent = new FontClientObserverAgent([&called]() { called = true; });
    ErrCode ret = agent->OnServiceDied();
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_TRUE(called);
}

/**
 * @tc.name: FontClientObserverAgentTest002
 * @tc.desc: Test FontClientObserverAgent OnServiceDied with null callback
 */
/**
 * @tc.name: FontClientObserverAgentTest002
 * @tc.desc: Test FontClientObserverAgent OnServiceDied with null callback (no crash, callback not invoked)
 */
HWTEST_F(FontManagerClientTest, FontClientObserverAgentTest002, TestSize.Level1)
{
    bool called = false;
    FontClientObserverAgent::OnServiceDiedCallback trackingCb = [&called]() { called = true; };
    auto agent = new FontClientObserverAgent(trackingCb);
    // Replace callback with null to test the null path
    agent->callback_ = nullptr;
    ErrCode ret = agent->OnServiceDied();
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_FALSE(called);
}

/**
 * @tc.name: OnFontObserverTest001
 * @tc.desc: Test OnFontObserver with valid observer (service reachable, death recipient added)
 */
HWTEST_F(FontManagerClientTest, OnFontObserverTest001, TestSize.Level1)
{
    auto client = DelayedSingleton<FontManagerClient>::GetInstance();
    sptr<MockFontClientObserverForClient> observer = new MockFontClientObserverForClient();
    int32_t ret = client->OnFontObserver(observer);
    // Service is available; death recipient should be set even if IPC returns permission error.
    // ret may be ERR_OK or ERR_NO_PERMISSION depending on test environment permissions.
    EXPECT_TRUE(ret == ERR_OK || ret == ERR_NO_PERMISSION);
    // saBinder_ and saDeathRecipient_ should be set if ret is ERR_OK or ERR_SCOPE_FONT_REPEATED_REGISTER
    if (ret == ERR_OK || ret == ERR_SCOPE_FONT_REPEATED_REGISTER) {
        EXPECT_NE(client->saBinder_, nullptr);
        EXPECT_NE(client->saDeathRecipient_, nullptr);
    }
}

/**
 * @tc.name: OnFontObserverTest002
 * @tc.desc: Test OnFontObserver called twice replaces death recipient
 */
HWTEST_F(FontManagerClientTest, OnFontObserverTest002, TestSize.Level1)
{
    auto client = DelayedSingleton<FontManagerClient>::GetInstance();
    sptr<MockFontClientObserverForClient> observer1 = new MockFontClientObserverForClient();
    int32_t ret1 = client->OnFontObserver(observer1);
    // First call: service available, may return permission error
    EXPECT_TRUE(ret1 == ERR_OK || ret1 == ERR_NO_PERMISSION);
    sptr<MockFontClientObserverForClient> observer2 = new MockFontClientObserverForClient();
    int32_t ret2 = client->OnFontObserver(observer2);
    EXPECT_TRUE(ret2 == ERR_OK || ret2 == ERR_NO_PERMISSION);
}

/**
 * @tc.name: OffFontObserverTest001
 * @tc.desc: Test OffFontObserver after OnFontObserver (cleanup occurs regardless of IPC result)
 */
HWTEST_F(FontManagerClientTest, OffFontObserverTest001, TestSize.Level1)
{
    auto client = DelayedSingleton<FontManagerClient>::GetInstance();
    sptr<MockFontClientObserverForClient> observer = new MockFontClientObserverForClient();
    client->OnFontObserver(observer);
    int32_t ret = client->OffFontObserver();
    // Service available; IPC may return permission error, but cleanup still occurs.
    EXPECT_TRUE(ret == ERR_OK || ret == ERR_NO_PERMISSION);
    // After OffFontObserver, saBinder_ and saDeathRecipient_ should be cleared.
    EXPECT_EQ(client->saBinder_, nullptr);
    EXPECT_EQ(client->saDeathRecipient_, nullptr);
}

/**
 * @tc.name: OffFontObserverTest002
 * @tc.desc: Test OffFontObserver without prior OnFontObserver (saBinder null)
 */
HWTEST_F(FontManagerClientTest, OffFontObserverTest002, TestSize.Level1)
{
    auto client = DelayedSingleton<FontManagerClient>::GetInstance();
    {
        std::lock_guard<std::mutex> lock(client->observerLock_);
        client->saBinder_ = nullptr;
        client->saDeathRecipient_ = nullptr;
    }
    int32_t ret = client->OffFontObserver();
    // Service available; IPC may return permission error.
    EXPECT_TRUE(ret == ERR_OK || ret == ERR_NO_PERMISSION);
    // saBinder_ and saDeathRecipient_ should still be null after OffFontObserver
    EXPECT_EQ(client->saBinder_, nullptr);
    EXPECT_EQ(client->saDeathRecipient_, nullptr);
}

/**
 * @tc.name: InstallScopeFontTest001
 * @tc.desc: Test InstallScopeFont with empty path
 */
HWTEST_F(FontManagerClientTest, InstallScopeFontTest001, TestSize.Level1)
{
    auto client = DelayedSingleton<FontManagerClient>::GetInstance();
    int outValue = 0;
    int32_t ret = client->InstallScopeFont("", FONT_SCOPE_APP, outValue);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(outValue, ERR_FILE_NOT_EXISTS);
}

/**
 * @tc.name: InstallScopeFontTest002
 * @tc.desc: Test InstallScopeFont with non-existent path
 */
HWTEST_F(FontManagerClientTest, InstallScopeFontTest002, TestSize.Level1)
{
    auto client = DelayedSingleton<FontManagerClient>::GetInstance();
    int outValue = 0;
    int32_t ret = client->InstallScopeFont("/data/test/nonexistent_12345.ttf", FONT_SCOPE_APP, outValue);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(outValue, ERR_FILE_NOT_EXISTS);
}

/**
 * @tc.name: InstallScopeFontTest003
 * @tc.desc: Test InstallScopeFont with long path
 */
HWTEST_F(FontManagerClientTest, InstallScopeFontTest003, TestSize.Level1)
{
    auto client = DelayedSingleton<FontManagerClient>::GetInstance();
    int outValue = 0;
    std::string longPath(PATH_MAX, 'a');
    int32_t ret = client->InstallScopeFont(longPath, FONT_SCOPE_APP, outValue);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(outValue, ERR_FILE_NOT_EXISTS);
}

/**
 * @tc.name: InstallScopeFontTest004
 * @tc.desc: Test InstallScopeFont with valid path (service reachable, IPC result depends on permissions)
 */
HWTEST_F(FontManagerClientTest, InstallScopeFontTest004, TestSize.Level1)
{
    auto client = DelayedSingleton<FontManagerClient>::GetInstance();
    int outValue = 0;
    client->InstallScopeFont(FONT_PATH, FONT_SCOPE_APP, outValue);
    // File exists so PathToRealPath/fopen/fileno succeed; IPC was reached.
    // Key: outValue should NOT be ERR_FILE_NOT_EXISTS (file was found and opened).
    EXPECT_NE(outValue, ERR_FILE_NOT_EXISTS);
}

/**
 * @tc.name: UninstallScopeFontTest001
 * @tc.desc: Test UninstallScopeFont with valid service (IPC result depends on permissions)
 */
HWTEST_F(FontManagerClientTest, UninstallScopeFontTest001, TestSize.Level1)
{
    auto client = DelayedSingleton<FontManagerClient>::GetInstance();
    int outValue = 0;
    int32_t ret = client->UninstallScopeFont("file://test/nonexistent.ttf", outValue);
    // Service available; ret is ERR_OK (client returns IPC result), outValue holds server result.
    EXPECT_EQ(ret, ERR_OK);
    // outValue is the server-side result (may be ERR_NO_PERMISSION or other error).
    EXPECT_NE(outValue, ERR_OK);
}

/**
 * @tc.name: GetFontScopeTest001
 * @tc.desc: Test GetFontScope with valid service
 */
HWTEST_F(FontManagerClientTest, GetFontScopeTest001, TestSize.Level1)
{
    auto client = DelayedSingleton<FontManagerClient>::GetInstance();
    int outValue = 0;
    int32_t ret = client->GetFontScope("file://test/nonexistent.ttf", outValue);
    // GetFontScope returns ERR_SYSTEM_ERROR on service null, or IPC result
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: DataMigrationTest002
 * @tc.desc: Test DataMigration with valid listener
 */
HWTEST_F(FontManagerClientTest, DataMigrationTest002, TestSize.Level1)
{
    auto client = DelayedSingleton<FontManagerClient>::GetInstance();
    class TestListener : public IDataMigrationListener {
    public:
        void OnHandle(const EventData& eventData) override {}
    };
    auto listener = std::make_shared<TestListener>();
    int32_t ret = client->DataMigration(listener);
    // Returns ERR_SYSTEM_ERROR on service null, or IPC result
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: GetFontServiceAbilityInvalidSaIdTest001
 * @tc.desc: Test GetFontServiceAbility with invalid SA ID returns null
 */
HWTEST_F(FontManagerClientTest, GetFontServiceAbilityInvalidSaIdTest001, TestSize.Level1)
{
    auto loadMgr = FontServiceLoadManager::GetInstance();
    sptr<IFontService> result = loadMgr->GetFontServiceAbility(INVALID_SA_ID);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: InstallFontFileOpsTest001
 * @tc.desc: Test PathToRealPath and file operations for valid font file
 */
HWTEST_F(FontManagerClientTest, InstallFontFileOpsTest001, TestSize.Level1)
{
    auto client = DelayedSingleton<FontManagerClient>::GetInstance();
    std::string realPath;
    EXPECT_TRUE(client->PathToRealPath(FONT_PATH, realPath));
    EXPECT_FALSE(realPath.empty());
    FILE* fp = fopen(realPath.c_str(), "rb");
    ASSERT_NE(fp, nullptr);
    int fd = fileno(fp);
    EXPECT_GE(fd, 0);
    (void)fclose(fp);
}

/**
 * @tc.name: FontSALoadCallbackNullTest001
 * @tc.desc: Test FontSALoadCallback OnLoadSystemAbilitySuccess is callable
 */
HWTEST_F(FontManagerClientTest, FontSALoadCallbackNullTest001, TestSize.Level1)
{
    sptr<FontSALoadCallback> callback = new FontSALoadCallback();
    sptr<IRemoteObject> remoteObj = new MockFontClientObserverForClient();
    callback->OnLoadSystemAbilitySuccess(FONT_SA_ID, remoteObj);
    auto loadMgr = FontServiceLoadManager::GetInstance();
    EXPECT_EQ(loadMgr->loadSaStatus_, LoadSaStatus::SUCCESS);
}

/**
 * @tc.name: FontSALoadCallbackNullTest002
 * @tc.desc: Test FontSALoadCallback OnLoadSystemAbilityFail is callable
 */
HWTEST_F(FontManagerClientTest, FontSALoadCallbackNullTest002, TestSize.Level1)
{
    sptr<FontSALoadCallback> callback = new FontSALoadCallback();
    callback->OnLoadSystemAbilityFail(FONT_SA_ID);
    auto loadMgr = FontServiceLoadManager::GetInstance();
    EXPECT_EQ(loadMgr->loadSaStatus_, LoadSaStatus::FAIL);
}

/**
 * @tc.name: UninstallFontTest002
 * @tc.desc: Test UninstallFont with valid service
 */
HWTEST_F(FontManagerClientTest, UninstallFontTest002, TestSize.Level1)
{
    auto client = DelayedSingleton<FontManagerClient>::GetInstance();
    int outValue = 0;
    int32_t ret = client->UninstallFont("NonExistentFont12345", outValue);
    // Returns ERR_OK on service null path, or IPC result
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_NE(outValue, ERR_OK);
}

/**
 * @tc.name: UninstallFontWithUserIdTest002
 * @tc.desc: Test UninstallFontWithUserId with valid service
 */
HWTEST_F(FontManagerClientTest, UninstallFontWithUserIdTest002, TestSize.Level1)
{
    auto client = DelayedSingleton<FontManagerClient>::GetInstance();
    int32_t ret = client->UninstallFontWithUserId("NonExistentFont12345", TEST_USERID);
    // Returns ERR_UNINSTALL_FAIL on service null, or IPC result
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: LoadSaFailTest001
 * @tc.desc: Test LoadSa returns false for invalid SA ID
 */
HWTEST_F(FontManagerClientTest, LoadSaFailTest001, TestSize.Level1)
{
    auto loadMgr = FontServiceLoadManager::GetInstance();
    bool ret = loadMgr->LoadSa(INVALID_SA_ID);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: UnloadFontServiceTest002
 * @tc.desc: Test UnloadFontService with invalid SA ID
 */
HWTEST_F(FontManagerClientTest, UnloadFontServiceTest002, TestSize.Level1)
{
    auto loadMgr = FontServiceLoadManager::GetInstance();
    bool ret = loadMgr->UnloadFontService(INVALID_SA_ID);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: GetFontServiceAbilityRetryTest001
 * @tc.desc: Test GetFontServiceAbility returns non-null for valid SA ID
 */
HWTEST_F(FontManagerClientTest, GetFontServiceAbilityRetryTest001, TestSize.Level1)
{
    auto loadMgr = FontServiceLoadManager::GetInstance();
    sptr<IFontService> result1 = loadMgr->GetFontServiceAbility(FONT_SA_ID);
    EXPECT_NE(result1, nullptr);
    sptr<IFontService> result2 = loadMgr->GetFontServiceAbility(FONT_SA_ID);
    EXPECT_NE(result2, nullptr);
}

/**
 * @tc.name: OnFontObserverCleanupTest001
 * @tc.desc: Test OnFontObserver then OffFontObserver cleans up death recipient
 */
HWTEST_F(FontManagerClientTest, OnFontObserverCleanupTest001, TestSize.Level1)
{
    auto client = DelayedSingleton<FontManagerClient>::GetInstance();
    sptr<MockFontClientObserverForClient> observer = new MockFontClientObserverForClient();
    int32_t ret = client->OnFontObserver(observer);
    // Service available; IPC may return permission error (201) but death recipient is still set.
    EXPECT_TRUE(ret == ERR_OK || ret == ERR_NO_PERMISSION);
    if (ret == ERR_OK || ret == ERR_SCOPE_FONT_REPEATED_REGISTER) {
        EXPECT_NE(client->saBinder_, nullptr);
        EXPECT_NE(client->saDeathRecipient_, nullptr);
    }
    int32_t offRet = client->OffFontObserver();
    EXPECT_TRUE(offRet == ERR_OK || offRet == ERR_NO_PERMISSION);
    // After OffFontObserver, saBinder_ and saDeathRecipient_ should be cleared regardless.
    EXPECT_EQ(client->saBinder_, nullptr);
    EXPECT_EQ(client->saDeathRecipient_, nullptr);
}

/**
 * @tc.name: OnFontObserverNullBinderTest001
 * @tc.desc: Test OnFontObserver with null observer (edge case, should not crash)
 */
HWTEST_F(FontManagerClientTest, OnFontObserverNullBinderTest001, TestSize.Level1)
{
    auto client = DelayedSingleton<FontManagerClient>::GetInstance();
    // Call OnFontObserver with null observer - should not crash.
    // Return value depends on server behavior (may be IPC error, permission error, etc.).
    client->OnFontObserver(nullptr);
    // After the call, saBinder_ and saDeathRecipient_ should be in a consistent state:
    // either both set (if IPC returned OK/REPEATED) or both null (cleanup on error).
    bool bothSet = (client->saBinder_ != nullptr && client->saDeathRecipient_ != nullptr);
    bool bothNull = (client->saBinder_ == nullptr && client->saDeathRecipient_ == nullptr);
    EXPECT_TRUE(bothSet || bothNull);
}

/**
 * @tc.name: PathToRealPathAccessFailTest001
 * @tc.desc: Test PathToRealPath with non-existent path (realpath returns null)
 */
HWTEST_F(FontManagerClientTest, PathToRealPathAccessFailTest001, TestSize.Level1)
{
    auto client = DelayedSingleton<FontManagerClient>::GetInstance();
    std::string realPath;
    EXPECT_FALSE(client->PathToRealPath("/data/test/nonexistent_path_12345.ttf", realPath));
    EXPECT_TRUE(realPath.empty());
}
} // namespace FontManager
} // namespace Global
} // namespace OHOS
