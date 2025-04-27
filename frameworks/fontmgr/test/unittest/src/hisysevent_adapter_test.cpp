/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OHOS_GLOBAL_FONT_MANAGER_HISYSEVENT_ADAPTER_TEST_CPP
#define OHOS_GLOBAL_FONT_MANAGER_HISYSEVENT_ADAPTER_TEST_CPP

#include "hisysevent_adapter_test.h"

#include <gtest/gtest.h>

#include "hisysevent_adapter.h"
#include "singleton.h"

using testing::ext::TestSize;

namespace OHOS {
namespace Global {
namespace FontManager {
static const std::string INSTALL_PATH = "/data/service/el1/public/for-all-app/fonts/";
void HisyseventAdapterTest::SetUpTestCase(void) {}
void HisyseventAdapterTest::TearDownTestCase(void) {}
void HisyseventAdapterTest::SetUp(void) {}
void HisyseventAdapterTest::TearDown(void) {}

/* *
 * @tc.name: CollectUserDataSize
 * @tc.desc: Normal
 * @tc.type FUNC
 */
HWTEST_F(HisyseventAdapterTest, CollectUserDataSize, TestSize.Level1)
{
    std::shared_ptr<HisyseventAdapter> adapter = HisyseventAdapter::GetInstance();
    EXPECT_EQ(adapter->CollectUserDataSize(), 0);
}

/* *
 * @tc.name: GetDataPartitionRemainSize
 * @tc.desc: Test GetDataPartitionRemainSize
 * @tc.type FUNC
 */
HWTEST_F(HisyseventAdapterTest, GetDataPartitionRemainSize, TestSize.Level1)
{
    std::shared_ptr<HisyseventAdapter> adapter = HisyseventAdapter::GetInstance();
    std::uint64_t size = adapter->GetDataPartitionRemainSize();
    EXPECT_GE(size, 0);
}

/* *
 * @tc.name: GetFileOrFolderPath
 * @tc.desc: Test GetFileOrFolderPath
 * @tc.type FUNC
 */
HWTEST_F(HisyseventAdapterTest, GetFileOrFolderPath, TestSize.Level1)
{
    std::shared_ptr<HisyseventAdapter> adapter = HisyseventAdapter::GetInstance();
    std::vector<std::string> result = adapter->GetFileOrFolderPath();
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], INSTALL_PATH);
}

/* *
 * @tc.name: GetFileOrFolderSize
 * @tc.desc: Test GetFileOrFolderSize
 * @tc.type FUNC
 */
HWTEST_F(HisyseventAdapterTest, GetFileOrFolderSize, TestSize.Level1)
{
    std::shared_ptr<HisyseventAdapter> adapter = HisyseventAdapter::GetInstance();
    std::vector<std::uint64_t> result = adapter->GetFileOrFolderSize();
    EXPECT_EQ(result.size(), 1);
    EXPECT_GE(result[0], 0);
}
} // namespace FontManager
} // namespace Global
} // namespace OHOS

#endif