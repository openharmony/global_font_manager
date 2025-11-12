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
const std::string INSTALL_PATH_TEST = "/data/service/el1/public/fonts/100/";
const std::string TEMP_PATH_TEST = "/data/service/el1/public/fonts/100/temp/";
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
    bool CopyFile(const string &srcPath, const string &desPath);
    std::shared_ptr<DataMigrationManager> manager_;
};

void DataMigrationManagerTest::SetUpTestCase(void)
{
}

void DataMigrationManagerTest::TearDownTestCase(void)
{
    FontManagerUtils::DeleteDir(TEMP_PATH_TEST, true);
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

bool DataMigrationManagerTest::CopyFile(const string &srcPath, const string &desPath)
{
    int fd = open(srcPath.c_str(), O_RDONLY);
    if (fd < 0) {
        return false;
    }

    if (!FontManagerUtils::CopyFile(fd, desPath)) {
        close(fd);
        return false;
    }
    close(fd);
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
    bool res = manager_->InitAllUserDir({TEST_USERID});
    ASSERT_TRUE(res);
    res = manager_->InitDataMigrationTempDir();
    ASSERT_TRUE(res);
    res = manager_->InitDataMigrationTempDir();
    ASSERT_TRUE(res);
    res = manager_->CopyFileForDataMigration(fontPath, TEST_USERID);
    EXPECT_EQ(res, true);
    res = manager_->CopyFileForDataMigration(fontPath, TEST_USERID);
    EXPECT_EQ(res, true);
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
    if (cb == nullptr) {
        return;
    }
    manager_->DataMigration(cb);
    std::vector<std::string> paths;
    OHOS::GetDirFiles(INSTALL_PATH_TEST, paths);
    EXPECT_EQ(paths.size() == 0, true);
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
    for (const auto &path : srcPaths) {
        std::string fileName = FontManagerUtils::GetFileName(path);
        EXPECT_TRUE(this->CopyFile(path, INSTALL_PATH_APP + fileName));
    }
    sptr<IDataMigrationCallback> cb = new (std::nothrow) TestCallback();
    if (cb == nullptr) {
        return;
    }
    manager_->DataMigration(cb);
    std::vector<std::string> paths;
    OHOS::GetDirFiles(INSTALL_PATH_TEST, paths);
    EXPECT_EQ(paths.size() == srcPaths.size(), true);
}

/**
 * @tc.name: DataMigrationManagerFuncTest004
 * @tc.desc: Test FontManager DataMigration case
 * @tc.type: FUNC
 */
HWTEST_F(DataMigrationManagerTest, DataMigrationManagerFuncTest004, TestSize.Level1)
{
    std::vector<std::string> srcPaths = {
        "/data/test/NotoSansCJK-Regular.ttc",
        "/data/test/NotoSerifCJK-Regular.ttc",
        "/data/test/TestFont_Sans.ttf",
        "/data/test/emptyTTF.ttf",
        "/data/test/200install_fontconfig.json"
    };
    for (const auto &path : srcPaths) {
        std::string fileName = FontManagerUtils::GetFileName(path);
        EXPECT_TRUE(this->CopyFile(path, INSTALL_PATH_APP + fileName));
    }
    sptr<IDataMigrationCallback> cb = new (std::nothrow) TestCallback();
    if (cb == nullptr) {
        return;
    }
    manager_->DataMigration(cb);
    std::vector<std::string> paths;
    OHOS::GetDirFiles(INSTALL_PATH_TEST, paths);
    EXPECT_EQ(paths.size() == 0, true);
}

} // namespace FontManager
} // namespace Global
} // namespace OHOS