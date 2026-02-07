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
#include <vector>

#define private public
#define protected public
#include "data_migration_manager.h"
#undef private
#undef protected
#include "font_manager_utils.h"
#include "font_define.h"
#include "callback_mock.h"
#include "directory_ex.h"
#include "permission_common.h"

namespace {
const std::string INSTALL_PATH_TEST = "/data/service/el1/100/for-all-app/fonts/";
const std::string TEMP_PATH_TEST = "/data/service/el1/100/for-all-app/fonts/temp/";
constexpr int32_t TEST_USERID = 100;
}

using testing::ext::TestSize;
using namespace std;

namespace OHOS {
namespace Global {
namespace FontManager {

class DataMigrationManagerTest : public testing::Test {
public:
    DataMigrationManagerTest(){};
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

protected:
    bool CopyTestFileToInstallPath(const std::vector<std::string> &srcPaths);
    std::shared_ptr<DataMigrationManager> manager_;
};

void DataMigrationManagerTest::SetUpTestCase(void)
{
}

void DataMigrationManagerTest::TearDownTestCase(void)
{
    FontManagerUtils::DeleteDir(TEMP_PATH_TEST, true);
    PermissionCommon::ResetTokenAndUid();
}

void DataMigrationManagerTest::SetUp(void)
{
    manager_ = DataMigrationManager::GetInstance();
}

void DataMigrationManagerTest::TearDown(void)
{
    PermissionCommon::ResetTokenAndUid();
    FontManagerUtils::DeleteDir(INSTALL_PATH_TEST, false);
    FontManagerUtils::DeleteDir(INSTALL_PATH_APP, false);
}

bool DataMigrationManagerTest::CopyTestFileToInstallPath(const std::vector<std::string> &srcPaths)
{
    for (const auto &path : srcPaths) {
        std::string fileName = FontManagerUtils::GetFileName(path);
        int fd = open(path.c_str(), O_RDONLY);
        if (fd < 0) {
            return false;
        }

        if (!FontManagerUtils::CopyFile(fd, INSTALL_PATH_APP + fileName)) {
            close(fd);
            return false;
        }
        close(fd);
    }
    return true;
}

/**
 * @tc.name: DataMigrationManagerFuncTest001
 * @tc.desc: Test FontManager CopyFileForDataMigration case
 * @tc.type: FUNC
 */
HWTEST_F(DataMigrationManagerTest, DataMigrationManagerFuncTest001, TestSize.Level1)
{
    const std::string fontPath = "/data/test/NotoSansCJK-Regular.ttc";
    manager_->userIds_ = {TEST_USERID};
    bool res = manager_->CheckAllUserDir();
    ASSERT_TRUE(res);
    res = manager_->InitDataMigrationTempDir();
    ASSERT_TRUE(res);
    res = manager_->InitDataMigrationTempDir();
    ASSERT_TRUE(res);
    int32_t ret = manager_->CopyFileForDataMigration(fontPath, TEST_USERID);
    EXPECT_EQ(ret, ERR_OK);
    ret = manager_->CopyFileForDataMigration(fontPath, TEST_USERID);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(FontManagerUtils::RemoveFile(INSTALL_PATH_TEST + "NotoSansCJK-Regular.ttc"), true);
}

/**
 * @tc.name: DataMigrationManagerFuncTest002
 * @tc.desc: Test FontManager DataMigration case
 * @tc.type: FUNC
 */
HWTEST_F(DataMigrationManagerTest, DataMigrationManagerFuncTest002, TestSize.Level1)
{
    sptr<IDataMigrationCallback> cb = new (std::nothrow) TestCallback();
    ASSERT_TRUE(cb != nullptr);
    manager_->DataMigration(cb);
    std::vector<std::string> paths;
    OHOS::GetDirFiles(INSTALL_PATH_TEST, paths);
    EXPECT_TRUE(paths.empty());
}

/**
 * @tc.name: DataMigrationManagerFuncTest003
 * @tc.desc: Test FontManager DataMigration case
 * @tc.type: FUNC
 */
HWTEST_F(DataMigrationManagerTest, DataMigrationManagerFuncTest003, TestSize.Level1)
{
    PermissionCommon::SetFontManagerInitEnv();
    std::vector<std::string> srcPaths = {
        "/data/test/NotoSansCJK-Regular.ttc",
        "/data/test/NotoSerifCJK-Regular.ttc",
        "/data/test/TestFont_Sans.ttf",
        "/data/test/emptyTTF.ttf",
        "/data/test/200install_fontconfig.json"
    };
    EXPECT_TRUE(this->CopyTestFileToInstallPath(srcPaths));
    sptr<IDataMigrationCallback> cb = new (std::nothrow) TestCallback();
    ASSERT_TRUE(cb != nullptr);
    manager_->DataMigration(cb);
    std::vector<std::string> paths;
    OHOS::GetDirFiles(INSTALL_PATH_TEST, paths);
    EXPECT_TRUE(paths.size() == srcPaths.size());
}

/**
 * @tc.name: DataMigrationManagerFuncTest004
 * @tc.desc: Test FontManager DataMigration case
 * @tc.type: FUNC
 */
HWTEST_F(DataMigrationManagerTest, DataMigrationManagerFuncTest004, TestSize.Level1)
{
    std::vector<std::string> srcPaths = {
        "/data/test/NotoSansCJK-Regular.ttc"
    };
    EXPECT_TRUE(this->CopyTestFileToInstallPath(srcPaths));
    sptr<IDataMigrationCallback> cb = new (std::nothrow) TestCallback();
    ASSERT_TRUE(cb != nullptr);
    manager_->DataMigration(cb);
    std::vector<std::string> paths;
    OHOS::GetDirFiles(INSTALL_PATH_TEST, paths);
#ifdef USE_EXTENSION_DATA
    EXPECT_TRUE(paths.empty());
#else
    EXPECT_TRUE(paths.size() == srcPaths.size());
#endif
}

/**
 * @tc.name: DataMigrationManagerFuncTest005
 * @tc.desc: Test CheckAllUserDir fail
 * @tc.type: FUNC
 */
HWTEST_F(DataMigrationManagerTest, DataMigrationManagerFuncTest005, TestSize.Level1)
{
    // 1. 设置 userIds
    manager_->userIds_ = {999}; // 使用一个不存在的用户ID

    // 2. 确保该用户的目录不存在
    std::string userPath = "/data/service/el1/999/for-all-app/fonts/";
    FontManagerUtils::DeleteDir(userPath, true);

    // 3. 执行 CheckAllUserDir
    // 对应代码: CheckPathExist 返回 false -> 返回 false
    bool ret = manager_->CheckAllUserDir();
    
    EXPECT_FALSE(ret);
}
} // namespace FontManager
} // namespace Global
} // namespace OHOS