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
#include <functional>
#define private public
#define protected public
#include "ifont_client_observer.h"
#include "font_client_registry.h"
#undef private
#undef protected
#include "iremote_stub.h"
#include "font_define.h"

using testing::ext::TestSize;

namespace OHOS {
namespace Global {
namespace FontManager {

class MockFontClientObserver : public IRemoteStub<IFontClientObserver> {
public:
    ErrCode OnServiceDied() override
    {
        return ERR_OK;
    }
    bool AddDeathRecipient(const sptr<DeathRecipient> &recipient) override
    {
        return true;
    }
    bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient) override
    {
        return true;
    }
};

class MockFontClientObserverFail : public IRemoteStub<IFontClientObserver> {
public:
    ErrCode OnServiceDied() override
    {
        return ERR_OK;
    }
    bool AddDeathRecipient(const sptr<DeathRecipient> &recipient) override
    {
        return false;
    }
    bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient) override
    {
        return false;
    }
};

class FontClientRegistryTest : public testing::Test {
public:
    FontClientRegistryTest() {}
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp()
    {
        registry_ = FontClientRegistry::GetInstance();
        registry_->clients_.clear();
        registry_->userClients_.clear();
    }
    void TearDown()
    {
        registry_->clients_.clear();
        registry_->userClients_.clear();
    }

protected:
    std::shared_ptr<FontClientRegistry> registry_;
};

/**
 * @tc.name: FontClientRegistryTest001
 * @tc.desc: Test RegisterClient success
 * @tc.type: FUNC
 */
HWTEST_F(FontClientRegistryTest, FontClientRegistryTest001, TestSize.Level1)
{
    sptr<MockFontClientObserver> observer = new MockFontClientObserver();
    int32_t ret = registry_->RegisterClient(observer->AsObject(), "com.example.app", 100, 100001);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(registry_->GetClientCount(), 1);
    EXPECT_EQ(registry_->IsClientRegistered(100001), true);
}

/**
 * @tc.name: FontClientRegistryTest002
 * @tc.desc: Test RegisterClient duplicate returns ERR_SCOPE_FONT_REPEATED_REGISTER
 * @tc.type: FUNC
 */
HWTEST_F(FontClientRegistryTest, FontClientRegistryTest002, TestSize.Level1)
{
    sptr<MockFontClientObserver> observer = new MockFontClientObserver();
    registry_->RegisterClient(observer->AsObject(), "com.example.app", 100, 100002);
    int32_t ret = registry_->RegisterClient(observer->AsObject(), "com.example.app", 100, 100002);
    EXPECT_EQ(ret, ERR_SCOPE_FONT_REPEATED_REGISTER);
    EXPECT_EQ(registry_->GetClientCount(), 1);
}

/**
 * @tc.name: FontClientRegistryTest003
 * @tc.desc: Test RegisterClient exceed limit returns ERR_SCOPE_FONT_EXCEED_REGISTER_LIMIT
 * @tc.type: FUNC
 */
HWTEST_F(FontClientRegistryTest, FontClientRegistryTest003, TestSize.Level1)
{
    for (int32_t i = 0; i < MAX_SCOPE_FONT_APP_NUM; i++) {
        sptr<MockFontClientObserver> observer = new MockFontClientObserver();
        registry_->RegisterClient(observer->AsObject(), "com.example.app" + std::to_string(i),
            100, 200000 + i);
    }
    sptr<MockFontClientObserver> observer = new MockFontClientObserver();
    int32_t ret = registry_->RegisterClient(observer->AsObject(), "com.example.overflow", 100, 299999);
    EXPECT_EQ(ret, ERR_SCOPE_FONT_EXCEED_REGISTER_LIMIT);
    EXPECT_EQ(registry_->GetClientCount(), MAX_SCOPE_FONT_APP_NUM);
}

/**
 * @tc.name: FontClientRegistryTest004
 * @tc.desc: Test UnregisterClient success
 * @tc.type: FUNC
 */
HWTEST_F(FontClientRegistryTest, FontClientRegistryTest004, TestSize.Level1)
{
    sptr<MockFontClientObserver> observer = new MockFontClientObserver();
    registry_->RegisterClient(observer->AsObject(), "com.example.app", 100, 300001);
    int32_t ret = registry_->UnregisterClient(300001);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(registry_->GetClientCount(), 0);
    EXPECT_EQ(registry_->IsClientRegistered(300001), false);
}

/**
 * @tc.name: FontClientRegistryTest005
 * @tc.desc: Test UnregisterClient not registered returns ERR_SCOPE_FONT_NOT_REGISTERED
 * @tc.type: FUNC
 */
HWTEST_F(FontClientRegistryTest, FontClientRegistryTest005, TestSize.Level1)
{
    int32_t ret = registry_->UnregisterClient(999999);
    EXPECT_EQ(ret, ERR_SCOPE_FONT_NOT_REGISTERED);
}

/**
 * @tc.name: FontClientRegistryTest006
 * @tc.desc: Test OnClientDied removes client
 * @tc.type: FUNC
 */
HWTEST_F(FontClientRegistryTest, FontClientRegistryTest006, TestSize.Level1)
{
    sptr<MockFontClientObserver> observer = new MockFontClientObserver();
    registry_->RegisterClient(observer->AsObject(), "com.example.app", 100, 400001);
    EXPECT_EQ(registry_->GetClientCount(), 1);
    registry_->OnClientDied(400001);
    EXPECT_EQ(registry_->GetClientCount(), 0);
    EXPECT_EQ(registry_->IsClientRegistered(400001), false);
}

/**
 * @tc.name: FontClientRegistryTest007
 * @tc.desc: Test OnClientDied with non-existent tokenId
 * @tc.type: FUNC
 */
HWTEST_F(FontClientRegistryTest, FontClientRegistryTest007, TestSize.Level1)
{
    registry_->OnClientDied(888888);
    EXPECT_EQ(registry_->GetClientCount(), 0);
}

/**
 * @tc.name: FontClientRegistryTest008
 * @tc.desc: Test SetClientDiedCallback and NotifyClientDied
 * @tc.type: FUNC
 */
HWTEST_F(FontClientRegistryTest, FontClientRegistryTest008, TestSize.Level1)
{
    bool called = false;
    registry_->SetClientDiedCallback([&called]() { called = true; });
    sptr<MockFontClientObserver> observer = new MockFontClientObserver();
    registry_->RegisterClient(observer->AsObject(), "com.example.app", 100, 500001);
    registry_->UnregisterClient(500001);
    EXPECT_TRUE(called);
}

/**
 * @tc.name: FontClientRegistryTest009
 * @tc.desc: Test different users can register independently
 * @tc.type: FUNC
 */
HWTEST_F(FontClientRegistryTest, FontClientRegistryTest009, TestSize.Level1)
{
    sptr<MockFontClientObserver> obs1 = new MockFontClientObserver();
    sptr<MockFontClientObserver> obs2 = new MockFontClientObserver();
    registry_->RegisterClient(obs1->AsObject(), "com.example.app1", 100, 600001);
    registry_->RegisterClient(obs2->AsObject(), "com.example.app2", 101, 600002);
    EXPECT_EQ(registry_->GetClientCount(), 2);
}

/**
 * @tc.name: FontClientRegistryTest010
 * @tc.desc: Test RegisterClient with null binder returns ERR_SYSTEM_ERROR
 * @tc.type: FUNC
 */
HWTEST_F(FontClientRegistryTest, FontClientRegistryTest010, TestSize.Level1)
{
    int32_t ret = registry_->RegisterClient(nullptr, "com.example.app", 100, 700001);
    EXPECT_EQ(ret, ERR_SYSTEM_ERROR);
    EXPECT_EQ(registry_->GetClientCount(), 0);
}

/**
 * @tc.name: FontClientRegistryTest011
 * @tc.desc: Test RegisterClient with AddDeathRecipient failure returns ERR_SYSTEM_ERROR
 * @tc.type: FUNC
 */
HWTEST_F(FontClientRegistryTest, FontClientRegistryTest011, TestSize.Level1)
{
    sptr<MockFontClientObserverFail> observer = new MockFontClientObserverFail();
    int32_t ret = registry_->RegisterClient(observer->AsObject(), "com.example.app", 100, 700002);
    EXPECT_EQ(ret, ERR_SYSTEM_ERROR);
    EXPECT_EQ(registry_->GetClientCount(), 0);
}

/**
 * @tc.name: FontClientRegistryTest012
 * @tc.desc: Test RegisterClient sets appIdentifier to app_<tokenId> format
 * @tc.type: FUNC
 */
HWTEST_F(FontClientRegistryTest, FontClientRegistryTest012, TestSize.Level1)
{
    sptr<MockFontClientObserver> observer = new MockFontClientObserver();
    int32_t tokenId = 700003;
    registry_->RegisterClient(observer->AsObject(), "com.example.app", 100, tokenId);
    auto it = registry_->clients_.find(tokenId);
    ASSERT_NE(it, registry_->clients_.end());
    EXPECT_EQ(it->second.appIdentifier, "app_" + std::to_string(tokenId));
}

/**
 * @tc.name: FontClientRegistryTest013
 * @tc.desc: Test UnregisterClient with null binder and recipient skips RemoveDeathRecipient
 * @tc.type: FUNC
 */
HWTEST_F(FontClientRegistryTest, FontClientRegistryTest013, TestSize.Level1)
{
    sptr<MockFontClientObserver> observer = new MockFontClientObserver();
    int32_t tokenId = 700004;
    registry_->RegisterClient(observer->AsObject(), "com.example.app", 100, tokenId);
    registry_->clients_[tokenId].binder = nullptr;
    registry_->clients_[tokenId].recipient = nullptr;
    int32_t ret = registry_->UnregisterClient(tokenId);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(registry_->IsClientRegistered(tokenId), false);
}

/**
 * @tc.name: FontClientRegistryTest014
 * @tc.desc: Test OnClientDied with empty appIdentifier skips cleanup
 * @tc.type: FUNC
 */
HWTEST_F(FontClientRegistryTest, FontClientRegistryTest014, TestSize.Level1)
{
    sptr<MockFontClientObserver> observer = new MockFontClientObserver();
    int32_t tokenId = 700005;
    registry_->RegisterClient(observer->AsObject(), "com.example.app", 100, tokenId);
    registry_->clients_[tokenId].appIdentifier = "";
    registry_->OnClientDied(tokenId);
    EXPECT_EQ(registry_->IsClientRegistered(tokenId), false);
    EXPECT_EQ(registry_->GetClientCount(), 0);
}
} // namespace FontManager
} // namespace Global
} // namespace OHOS
