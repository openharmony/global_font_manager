# AGENTS.md - 字体管理仓库开发指南

本文件为在 font_manager 仓库中工作的智能编码代理提供指导。

## 概述

`font_manager` 组件（`@ohos.fontManager`，版本 5.0）属于 OpenHarmony `global` 子系统。它提供系统级字体安装/卸载、应用级/会话级第三方字体安装/卸载，以及数据迁移服务。服务以 SystemAbility `66262`（`font_manager_server` 进程）运行，通过 NAPI（JavaScript/ArkTS）和 ANI（ArkTS 原生）绑定对外暴露 API。

- **SystemCapability**：`SystemCapability.Global.FontManager`
- **SA ID**：`66262`
- **许可证**：Apache License 2.0

### 字体作用域级别

| 作用域 | 值 | 生命周期 | 清理触发条件 | 权限 |
|--------|------|----------|--------------|------|
| 用户级 | -1（默认/历史） | 持久 | 仅手动卸载 | `ohos.permission.UPDATE_FONT` |
| 应用级 | 0（`FONT_SCOPE_APP`） | 应用注册期间 | 应用退出 / SA 退出 / 账号停止 / 开机 | `ohos.permission.UPDATE_SCOPE_FONT` |
| 会话级 | 1（`FONT_SCOPE_SESSION`） | 当前开机周期 | 账号停止 / 开机 | `ohos.permission.UPDATE_SCOPE_FONT` |

## 构建系统

本项目使用 GN（Generate Ninja）构建系统。

### 构建命令

```bash
# 构建整个字体管理服务（在 OpenHarmony 根目录执行，如 ~/openharmony）
./build.sh --product-name rk3568 --ccache --build-target font_manager

# 构建所有单元测试（unittest 组）
./build.sh --product-name rk3568 --ccache --build-target //base/global/font_manager/test/unittest:unittest

# 构建单个测试目标
./build.sh --product-name rk3568 --ccache --build-target font_manager_module_test
./build.sh --product-name rk3568 --ccache --build-target hisysevent_adapter_test

# 构建模糊测试
./build.sh --product-name rk3568 --ccache --build-target //base/global/font_manager/test/fuzztest:fuzztest
```

## 仓库结构

```
font_manager/
├── AGENTS.md
├── bundle.json                          # 组件清单与依赖
├── common/include/                      # 共享头文件（无 .cpp，仅头文件）
│   ├── font_define.h                    # 错误码、常量、作用域字体常量
│   ├── font_hilog.h                     # 日志宏（FONT_LOGD/I/W/E）
│   └── idata_migration_listener.h       # 抽象监听器接口
├── frameworks/fontmgr/                  # 核心业务逻辑库
│   ├── fontmgr.gni                      # GN 导入文件（源文件、头文件、依赖）
│   ├── include/                         # 8 个头文件
│   └── src/                             # 8 个实现文件
├── interfaces/
│   ├── ani/                             # ANI（ArkTS 原生）绑定
│   │   ├── ets/@ohos.fontManager.ets    # ArkTS API 声明（含作用域字体 + 观察者）
│   │   ├── include/                     # font_manager_ani.h, ani_data_migration_listener.h
│   │   └── src/                         # .cpp 实现
│   └── js/kits/                         # NAPI（JavaScript）绑定
│       ├── include/                     # font_manager_addon.h, font_napi_callback.h, ...
│       └── src/                         # .cpp 实现
├── sa_profile/
│   ├── 66262.json                       # SA 配置（libpath、按需启动 commonevent）
│   └── BUILD.gn
├── service/                             # IPC 服务层（客户端 + 服务端）
│   ├── BUILD.gn                         # IDL 生成、客户端和服务端库
│   ├── IFontService.idl                 # 主 IPC 接口（10 个方法）
│   ├── IDataMigrationCallback.idl       # 数据迁移回调接口
│   ├── IDataMigrationCallbackEvent.idl  # EventType 枚举 + EventData 结构体
│   ├── IFontClientObserver.idl          # 客户端观察者回调（OnServiceDied）
│   ├── font_manager_client.map          # 符号导出映射
│   ├── client/                          # 客户端侧（代理、SA 加载器、回调/观察者代理）
│   ├── etc/font_manager_server.cfg      # 服务初始化配置（mkdir、uid、权限）
│   ├── inner_api/                       # 内部 API（FontManagerKits, FontManagerInnerApi）
│   ├── param/                           # 系统参数（.para, .para.dac）
│   └── server/                          # 服务端侧（FontManagerServer SA）
└── test/
    ├── common/                          # PermissionCommon 测试工具
    ├── fuzztest/                        # 6 个模糊测试目标 + 工具
    └── unittest/                        # 单元测试 + 测试数据
```

## 任务路径速查

| 任务 | 关键路径 |
|------|----------|
| 新增/修改 IPC 方法 | `service/IFontService.idl` + `service/server/src/font_manager_server.cpp`（桩实现）+ `service/client/src/font_manager_client.cpp`（代理调用） |
| 新增/修改 NAPI 绑定 | `interfaces/js/kits/src/font_manager_addon.cpp` + `interfaces/js/kits/include/font_manager_addon.h` |
| 新增/修改 ANI 绑定 | `interfaces/ani/src/font_manager_ani.cpp` + `interfaces/ani/ets/@ohos.fontManager.ets` |
| 新增/修改错误码 | `common/include/font_define.h` → `enum FontErrorCode` + `font_manager_addon.cpp` 中的 NAPI 错误映射 |
| 新增/修改业务逻辑 | `frameworks/fontmgr/src/font_manager.cpp` + `frameworks/fontmgr/include/font_manager.h` |
| 新增/修改配置模式 | `frameworks/fontmgr/src/font_config.cpp` + `frameworks/fontmgr/include/font_config.h` |
| 新增/修改作用域字体生命周期 | `frameworks/fontmgr/src/font_client_registry.cpp` + `service/server/src/font_manager_server.cpp`（清理方法） |
| 新增/修改 SA 启动配置 | `sa_profile/66262.json` + `service/etc/font_manager_server.cfg` |
| 新增/修改系统参数 | `service/param/font_manager.para` + `service/param/font_manager.para.dac` |
| 新增单元测试 | `test/unittest/src/` + `test/unittest/BUILD.gn` |
| 新增模糊测试 | `test/fuzztest/` + `test/fuzztest/BUILD.gn` |
| 新增/修改内部 API | `service/inner_api/include/font_manager_inner_api.h` + `service/inner_api/src/font_manager_inner_api.cpp` |

## 代码风格指南

### 文件结构与组织

- **版权头**：所有源文件以 Apache 2.0 版权头开头
- **头文件保护**：使用 `#ifndef` 风格保护。注意：代码库有两种格式：
  - `GLOBAL_FONT_MANAGER_<FILENAME>_H`（如 `GLOBAL_FONT_MANAGER_FONT_DEFINE_H`、`GLOBAL_FONT_MANAGER_FONT_MANAGER_H`）
  - `FONT_MANAGER_<FILENAME>_H`（如 `FONT_MANAGER_DATA_MIGRATION_MANAGER_H`）——新文件优先使用 `GLOBAL_FONT_MANAGER_` 前缀
- **包含顺序**：系统头文件，然后项目头文件（字母序）
- **文件命名**：`lowercase_with_underscores.cpp` 和 `lowercase_with_underscores.h`

### 命名空间约定

```cpp
namespace OHOS {
namespace Global {
namespace FontManager {
    // 代码写在这里
} // namespace FontManager
} // namespace Global
} // namespace OHOS
```

### 命名规范

- **类名**：PascalCase（如 `FontManager`、`FontConfig`、`FontManagerClient`、`DataMigrationManager`）
- **函数名**：公开方法使用 PascalCase（如 `InstallFont`、`UninstallFont`、`DataMigration`）
- **变量名**：camelCase（如 `installPath`、`fontFullName`、`userId`）
- **常量**：头文件使用 `constexpr` 或 `inline const`；.cpp 中使用文件作用域 `static constexpr`（如 `MAX_INSTALL_NUM`、`FONT_SA_ID`）
- **成员变量**：camelCase，私有成员加尾下划线（如 `configMap_`、`mapLock_`、`isDataMigrationing_`）
- **枚举**：PascalCase 类型名 + UPPER_SNAKE_CASE 值（如 `FontErrorCode`、`ERR_OK`）

### 类型与格式

- **指针**：IPC/binder 对象使用 `sptr<T>`，普通智能指针使用 `std::shared_ptr<T>`
- **常量**：编译期使用 `constexpr`，头文件定义使用 `inline const`
- **字符串类型**：统一使用 `std::string`
- **整数类型**：文件描述符和用户 ID 使用 `int32_t`，通用整数使用 `int`
- **错误码**：返回 `int32_t` 或 `ErrCode`（typedef: `using ErrCode = int;`）
- **布尔值**：使用 `bool` 类型，字面量 `true`/`false`
- **并发**：标志/计数器使用 `std::atomic<T>`（如 `std::atomic<bool> isDataMigrationing_`）

### 错误处理

错误码定义在 `common/include/font_define.h` 中的 `enum FontErrorCode`：

```cpp
enum FontErrorCode {
    ERR_OK = 0,
    ERR_NO_PERMISSION = 201,
    ERR_NOT_SYSTEM_APP = 202,
    ERR_INVALID_PARAM = 401,
    ERR_FILE_NOT_EXISTS = 31100101,
    ERR_FILE_VERIFY_FAIL = 31100102,
    ERR_COPY_FAIL = 31100103,
    ERR_INSTALLED_ALRADY = 31100104,
    ERR_MAX_FILE_COUNT = 31100105,
    ERR_INSTALL_FAIL = 31100106,
    ERR_UNINSTALL_FILE_NOT_EXISTS = 31100107,
    ERR_UNINSTALL_REMOVE_FAIL = 31100108,
    ERR_UNINSTALL_FAIL = 31100109,
    ERR_SYSTEM_ERROR = 31100110,
    ERR_DATA_MIGRATIONING = 31100111,
    // 作用域字体管理（第三方应用级 / 会话级）
    ERR_SCOPE_FONT_NOT_FOUND = 31100112,
    ERR_SCOPE_FONT_REPEATED_REGISTER = 31100113,
    ERR_SCOPE_FONT_EXCEED_REGISTER_LIMIT = 31100114,
    ERR_SCOPE_FONT_NOT_REGISTERED = 31100115,
};
```

常见模式——检查条件、记录错误、返回错误码：
```cpp
if (service == nullptr) {
    FONT_LOGE("Service is null");
    return ERR_SYSTEM_ERROR;
}
```

### 日志

使用 `common/include/font_hilog.h` 中定义的自定义日志宏：
- `FONT_LOGD(...)` - Debug 级别（HILOG_DEBUG）
- `FONT_LOGI(...)` - Info 级别（HILOG_INFO）
- `FONT_LOGW(...)` - Warning 级别（HILOG_INFO）
- `FONT_LOGE(...)` - Error 级别（HILOG_ERROR）

日志配置：
- `LOG_DOMAIN = 0xD001E00`
- `LOG_TAG = "FONT_MSG"`

日志格式化符（HiLog 隐私）：
- `%{public}s` - 公开字符串（不脱敏）。TokenId 不属于敏感信息，使用 `%{public}d`
- `%{private}s` - 私有字符串（日志中脱敏）
- `%{public}d` - 公开整数

### 单例模式

使用 `DelayedSingleton<T>` 实现单例类：
```cpp
class FontManager : public DelayedSingleton<FontManager> {
    DECLARE_DELAYED_SINGLETON(FontManager);
public:
    int32_t InstallFont(const int32_t &fd, const int32_t userId);
private:
    FontManager();
    ~FontManager();
};
// 使用方式：
auto manager = FontManager::GetInstance();
```

引用单例（非延迟）使用 `DelayedRefSingleton<T>`：
```cpp
// FontManagerKits::GetInstance() 返回 DelayedRefSingleton<FontManagerClient>::GetInstance()
```

### 线程安全

使用 `std::mutex` + `std::lock_guard` 实现线程安全操作：
```cpp
std::mutex mapLock_;
std::unordered_map<int32_t, FontConfig> configMap_;

FontConfig& SafeGetOrCreateConfig(int32_t userId, const std::string& configPath) {
    std::lock_guard<std::mutex> lock(mapLock_);
    auto result = configMap_.try_emplace(userId, FontConfig(configPath));
    return result.first->second;
}
```

原子标志/计数器：
```cpp
std::atomic<bool> isDataMigrationing_ {false};
std::atomic_uint callingCount_ {0};
```

SA 加载使用条件变量：
```cpp
std::condition_variable proxyConVar_;
std::mutex serviceLock_;
```

**关键**：`std::mutex` 不可递归。在已加锁上下文中调用的内部辅助方法不得锁定同一互斥锁。在 `FontConfig` 中，`WriteToFile` 和 `GetFileData` 不锁定 `configLock_`，因为调用它们的公开方法已持有锁。

### 测试指南

- 使用 `HWTEST_F` 宏编写测试用例
- 测试类命名：`<类名>Test`（如 `FontManagerTest`、`FontConfigTest`）
- 测试用例命名：`<类名>FuncTest<编号>` 或 `<前缀>Test<编号>`（如 `FontManagerFuncTest001`、`InnerApiInstallFontTest001`）
- 测试规模级别：`TestSize.Level1`（Level1 = 关键）
- 必要时使用 `#define private public` 和 `#undef private` 测试私有成员
- 测试权限通过 `PermissionCommon::SetFontManagerPermission(processName)` 设置
- 使用 `IRemoteStub<T>` 模式模拟 IPC 对象（见 `callback_mock.h`、`font_client_registry_test.cpp`）

```cpp
HWTEST_F(FontManagerTest, FontManagerFuncTest001, TestSize.Level1) {
    int fd = open(FONT_PATH.c_str(), O_RDONLY);
    EXPECT_EQ(fd >= 0, true);
    int ret = manager_->InstallFont(fd, TEST_USERID);
    EXPECT_EQ(ret, ERR_OK);
    if (fd >= 0) {
        close(fd);
    }
}
```

## 关键常量

定义在 `common/include/font_define.h`：

| 常量 | 值 | 说明 |
|------|------|------|
| `FONT_SA_ID` | `66262` | SystemAbility ID |
| `INSTALL_PATH_APP` | `"/data/service/el1/public/for-all-app/fonts/"` | 公共安装路径 |
| `INSTALL_PATH_PREFIX` | `"/data/service/el1/"` | 用户安装路径前缀 |
| `INSTALL_PATH_SUFFIX` | `"/for-all-app/fonts/"` | 用户安装路径后缀 |
| `FONT_CONFIG_FILE` | `"install_fontconfig.json"` | 配置文件名 |
| `TEMP_FILE` | `"temp/"` | 临时子目录名 |
| `EXT_STORAGE_BUNDLE_PARAM_KEY` | `"const.fontmanager.extstoragebundle"` | 系统参数键 |
| `APP_FONT_DIR_PREFIX` | `"app_"` | 应用级字体子目录前缀 |
| `FONT_CONFIG_VERSION_7` | `"7.0"` | 作用域字体配置文件版本 |
| `MAX_SCOPE_FONT_APP_NUM` | `5` | 每用户最大注册观察者应用数 |
| `FONT_SCOPE_APP` | `0` | 应用级作用域常量 |
| `FONT_SCOPE_SESSION` | `1` | 会话级作用域常量 |
| `FONT_SCOPE_NONE` | `-1` | 用户级（历史记录） |

源文件中的文件作用域常量：

| 常量 | 文件 | 值 | 说明 |
|------|------|------|------|
| `MAX_INSTALL_NUM` | `font_manager.cpp` | `200`（默认，通过 `const.fontmanager.maxinstallnum` 参数配置） | 每用户最大可安装字体数 |
| `MAX_FONT_FILE_SIZE` | `font_manager_utils.cpp` | `1024*1024*1024`（1GB） | 最大字体文件大小 |
| `DELAY_MILLISECONDS_FOR_UNLOAD_SA` | `font_manager_server.cpp` | `10000` | 10 秒自动卸载延迟 |
| `HEARTBEAT_INTERVAL` | `data_migration_manager.cpp` | `60` | 心跳秒数 |
| `MAX_TRIGGER_COUNT` | `data_migration_manager.cpp` | `100` | 最大进度事件数 |
| `COPY_SPEED` | `data_migration_manager.cpp` | `10*1024/60` | 估计拷贝速度（Mb/s） |
| `STORAGE_MANAGER_MANAGER_ID` | `storage_manager_adapter.cpp` | `5003` | 存储管理 SA ID |

## 核心组件

### 框架层（`frameworks/fontmgr/`）

核心字体管理逻辑，作为库被服务端使用。

#### FontManager（`font_manager.h/.cpp`）
核心安装/卸载业务逻辑。通过 `DelayedSingleton<FontManager>` 实现单例。

**用户级方法：**
- `int32_t InstallFont(const int32_t &fd, const int32_t userId)` - 从 fd 安装字体。验证路径、检查重复（`ERR_INSTALLED_ALRADY`）、执行最大安装数限制（默认 200，通过 `const.fontmanager.maxinstallnum` 参数配置）、通过临时目录 + 重命名拷贝、插入配置记录、上报 HiSysEvent/StorageManager、发布 `FontEventType::INSTALL` 事件。
- `int32_t UninstallFont(const std::string &fontFullName, const int32_t userId)` - 按全名卸载。

**作用域字体方法：**
- `int32_t InstallScopeFont(const ScopeFontInstallInfo &info)` - 安装应用/会话级字体。升级配置版本、验证 srcPath/名称去重、拷贝到 `app_<tokenId>/` 或 `session_<tokenId>/` 子目录。
- `int32_t UninstallScopeFont(const std::string &srcPath, const std::string &bundleName, int32_t userId)` - 按 srcPath（URL）卸载。
- `int32_t GetFontScope(const std::string &srcPath, const std::string &bundleName, int32_t userId)` - 按 srcPath 查询作用域。未找到返回 `ERR_SCOPE_FONT_NOT_FOUND`（31100112）。
- `int32_t CleanupAppScopeFonts(const std::string &appIdentifier, int32_t userId)` - 清理指定应用的所有应用级字体（应用死亡/注销时调用）。
- `int32_t CleanupScopeFontsByUser(int32_t userId)` - 清理用户的所有作用域字体（开机/用户停止）。
- `int32_t CleanupAppScopeFontsByUser(int32_t userId)` - 仅清理用户的应用级（scope=0）字体（SA 重启）。会话级字体保留。

**ScopeFontInstallInfo 结构体：**
```cpp
struct ScopeFontInstallInfo {
    int32_t fd = -1;
    int32_t scope = -1;
    std::string srcPath;       // 应用提供的 URL（唯一标识符）
    std::string bundleName;
    std::string appIdentifier; // "app_<tokenId>" 或 "session_<tokenId>"
    int32_t userId = -1;
};
```

私有辅助方法：`GetFormatFullName`、`CopyFileForInstall`、`SandBoxPathToRealPath`、`SafeGetOrCreateConfig`（线程安全 map 访问）、`GetAppInstallPath`、`ValidateScopeFontForInstall`、`CopyAndInsertScopeFont`。

#### FontConfig（`font_config.h/.cpp`）
管理 JSON 配置文件（`install_fontconfig.json`）。禁用拷贝（`DISALLOW_COPY`），启用移动。单一 `configLock_` 互斥锁；内部辅助方法（`WriteToFile`、`GetFileData`、`CheckConfigFile`）不加锁（由调用方持有锁）。

**用户级方法：**
- `bool InsertFontRecord(const std::string &fontPath, const std::vector<std::string> &fullNames)` - 添加用户级记录（无 scope 字段）
- `bool DeleteFontRecord(const std::string &fontPath)` - 按 fontfullpath 删除
- `int GetInstalledFontsNum()` - 计数
- `std::string GetFontFileByName(const std::string &fullName)` - 按名称查找路径

**作用域字体方法：**
- `bool InsertScopeFontRecord(const FontRecordInfo &record)` - 添加作用域记录（含 scope/srcPath/appIdentifier/bundleName）
- `bool DeleteScopeFontRecordByUrl(const std::string &srcPath)` - 按 srcPath 删除
- `bool DeleteScopeFontRecordByAppId(const std::string &appIdentifier)` - 删除应用的所有记录
- `std::optional<FontRecordInfo> GetFontRecordByUrl(const std::string &srcPath)` - 按 srcPath 查找
- `std::optional<FontRecordInfo> GetFontRecordByName(const std::string &fullName)` - 按字体名查找（用于去重检查）
- `std::vector<FontRecordInfo> GetFontRecordsByAppId(const std::string &appIdentifier)` - 应用的所有记录
- `std::vector<FontRecordInfo> GetScopeFontRecords()` - scope >= 0 的所有记录
- `std::vector<FontRecordInfo> GetAppScopeFontRecords()` - 仅 scope == FONT_SCOPE_APP(0) 的记录
- `bool CheckAndUpdateFontRecord()` - 升级版本：int `1` → string `"7.0"`。不修改旧记录。
- `int GetTotalInstalledFontsNum()` - 总计数（用户 + 作用域）

**FontRecordInfo 结构体：**
```cpp
struct FontRecordInfo {
    std::string fontPath;
    std::vector<std::string> fullNames;
    int32_t scope = -1;           // -1 = 用户级, 0 = 应用级, 1 = 会话级
    std::string srcPath;
    std::string appIdentifier;
    std::string bundleName;
};
```

配置 JSON 格式（v7.0）：
```json
{
    "fontlist": [
        {
            "fontfullpath": "/data/service/el1/100/for-all-app/fonts/font.ttf",
            "fullname": ["FontName"]
        },
        {
            "fontfullpath": "/data/service/el1/100/for-all-app/fonts/app_123456/font.ttf",
            "fullname": ["AppFont"],
            "scope": 0,
            "srcPath": "file://apps/some_app/font.ttf",
            "appIdentifier": "app_123456",
            "bundleName": "com.example.app"
        }
    ],
    "version": "7.0"
}
```

**OTA 兼容性**：旧记录（无 `scope`/`srcPath`/`appIdentifier` 字段）被正确视为用户级。`FontRecordInfo::scope` 默认为 -1。所有读取方法使用 `cJSON_GetObjectItem` + nullptr 检查。初始配置创建直接使用 `"version": "7.0"`。

#### FontManagerUtils（`font_manager_utils.h/.cpp`）
静态工具类。所有方法均为静态。

- `static bool CheckAndInitInstallPath(const std::string &installPath)` - 初始化安装目录 + 临时 + 配置
- `static bool CheckAndInitScopeFontPath(const std::string &installPath)` - 初始化作用域目录 + 临时（无配置文件）
- `static bool CheckPathExist(const std::string &pathName)` - `std::filesystem::exists`
- `static bool CheckFontConfigPath(const std::string &installPath)` - 缺失时创建配置 `{"fontlist": [], "version": "7.0"}`
- `static bool CreateDirWithPermission(const std::string &fileDir)` - 创建目录，移除 `others_write`
- `static std::string GetFileName(const std::string &path)` - 最后一个 `/` 之后的子串
- `static std::string GetFileDirectory(const std::string &path)` - 最后一个 `/` 之前的子串
- `static bool CopyFile(int32_t sourceFd, const std::string& path)` - `sendfile()`，chmod 0644，检查 `MAX_FONT_FILE_SIZE`（1GB）
- `static std::string GetFilePathByFd(const int32_t &fd)` - 对 `/proc/<pid>/fd/<fd>` 执行 `readlink`
- `static bool RenameFile(const std::string& src, const std::string& dest)` - `std::filesystem::rename`
- `static std::string GetFileTime()` - 时间戳 `YYYYMMDD-HHMMSS`
- `static bool RemoveFile(const std::string &path)`
- `static void DeleteDir(const std::string &rootPath, bool isDeleteRootDir)`
- `static std::vector<int32_t> GetAllCreatedUserIds()` - 通过 `OsAccountManager`（由 `#ifdef ACCOUNT_ENABLE` 保护）
- `static void ClearAllTempFileDir()` - 清理所有用户的临时目录
- `static void CleanupScopeFontDirs()` - 清理临时 + 移除空的 `app_*/`/`session_*/` 目录（SA 退出）
- `static void CleanupAllScopeFontDirs()` - 强制删除所有 `app_*/` + `session_*/` 目录（开机）
- `static void CleanupAppScopeFontDirs()` - 强制删除所有 `app_*/` 目录（SA 重启）
- `static std::vector<std::string> GetFullNamesByFd(const int32_t &fd)` - `fstat` 预检 1GB + `FontToolSet::GetFontFullName(fd)`（TTF/TTC）
- `static std::vector<std::string> GetFullNamesByPath(const std::string &path)` - 打开文件、获取 fd、调用上面的方法

**1GB 大小检查**：在 `GetFullNamesByFd`（通过 `fstat` 在解析前预检）和 `CopyFileByFd`（通过 `fstat` 在 sendfile 前预检）中均执行。双重保护。

#### FontClientRegistry（`font_client_registry.h/.cpp`）
客户端注册表。通过 `DelayedSingleton<FontClientRegistry>` 实现单例。通过死亡通知管理应用级字体生命周期。

- `int32_t RegisterClient(const sptr<IRemoteObject> &observerBinder, const std::string &bundleName, int32_t userId, int32_t tokenId)` - 注册客户端、添加死亡通知、检查重复（`ERR_SCOPE_FONT_REPEATED_REGISTER`）和每用户上限 `MAX_SCOPE_FONT_APP_NUM=5`（`ERR_SCOPE_FONT_EXCEED_REGISTER_LIMIT`）
- `int32_t UnregisterClient(int32_t tokenId)` - 注销、移除死亡通知、清理应用字体、通知 SA 可能退出
- `bool IsClientRegistered(int32_t tokenId)`
- `int32_t GetClientCount()`
- `void OnClientDied(int32_t tokenId)` - 客户端死亡回调、清理应用字体、通知 SA
- `void SetClientDiedCallback(ClientDiedCallback callback)` - 设置 SA 退出通知回调

**数据结构：**
```cpp
struct ClientInfo {
    sptr<IRemoteObject> binder;
    std::string bundleName;
    int32_t userId;
    int32_t tokenId;
    std::string appIdentifier;  // "app_<tokenId>"
    sptr<IRemoteObject::DeathRecipient> recipient;
};
```

#### FontEventPublish（`font_event_publish.h/.cpp`**
静态类，用于发布公共事件。所有作用域级别（用户/应用/会话）使用相同的事件。

```cpp
enum FontEventType { INSTALL = 0, UNINSTALL = 1 };
static bool PublishFontUpdate(const FontEventType eventType, const std::string &formatName, const int32_t &userId);
```
- **Action**：`usual.event.FONT_UPDATE_FOR_POLICY`
- **参数**：`eventType`（0=INSTALL, 1=UNINSTALL）、`fontFullNames`（逗号分隔）

#### HisyseventAdapter（`hisysevent_adapter.h/.cpp`）
HiSysEvent 发布器。通过 `DelayedSingleton<HisyseventAdapter>` 实现单例。

- `int CollectUserDataSize(const std::string &path)` - 事件 `USER_DATA_SIZE`，域 `FILEMANAGEMENT`，类型 `STATISTIC`
- `int CollectDataMigrationState(const std::vector<int32_t> &userIds, int32_t result)` - 事件 `FONT_DATA_MIGRATION`，域 `FONT_MANAGER`，类型 `STATISTIC`

#### DataMigrationManager（`data_migration_manager.h/.cpp`**
编排数据迁移。通过 `DelayedSingleton<DataMigrationManager>` 实现单例，同时使用 `std::enable_shared_from_this`。

- `void DataMigration(const sptr<IDataMigrationCallback>& callback)` - 主入口。设置 `isDataMigrationing_=true`、运行 `DataMigrationInner()`、`CheckAndUpdateAllFontRecord()`、按用户上报统计、重置标志、发送结果事件。

迁移流程：`InitDataMigrationEnv`（获取用户 ID，检查空 -> `ERR_NOT_NEED_DATA_MIGRATION`）-> `StartHeartBeatTask`（分离线程，60 秒间隔）-> `StartDataMigration`（将每个文件从 `INSTALL_PATH_APP` 拷贝到所有用户目录）。

进度计算：`progressPercentage = (i*100 + size/2)/size`，`timeRemaining = (folderSize * idsize) >> 20 / COPY_SPEED`。

#### StorageManagerAdapter（`storage_manager_adapter.h/.cpp`**
向 StorageManager SA（ID 5003）上报字体包统计。通过 `DelayedSingleton<StorageManagerAdapter>` 实现单例。

- `int32_t ReportFontBundleStats(int32_t userId, const std::string &installPath)` - 从系统参数 `const.fontmanager.extstoragebundle` 获取业务名、获取文件夹大小、调用 `proxy->SetExtBundleStats(userId, stats)`。

### 公共头文件（`common/include/`）

#### font_define.h
错误码（`FontErrorCode`、`DataMigrationResultCode`）、常量（路径、作用域字体常量）、`using ErrCode = int;`

#### font_hilog.h
日志宏（`FONT_LOGD/I/W/E`）、`LOG_DOMAIN = 0xD001E00`、`LOG_TAG = "FONT_MSG"`。

#### idata_migration_listener.h
抽象监听器接口：
```cpp
class IDataMigrationListener {
public:
    virtual ~IDataMigrationListener() = default;
    virtual void OnHandle(const EventData& eventData) = 0;
};
```

## IPC 架构

### IDL 接口

#### IFontService（`service/IFontService.idl`）
主服务接口（包 `OHOS.Global.FontManager`）：
```
interface IFontService {
    void InstallFont([in] FileDescriptor fd, [out] int outValue);
    void UninstallFont([in] String fontName, [out] int outValue);
    void DataMigration([in] IDataMigrationCallback callbackInfo);
    void InstallFontWithUserId([in] FileDescriptor fd, [in] int userId);
    void UninstallFontWithUserId([in] String fontName, [in] int userId);

    void OnFontObserver([in] IFontClientObserver observer);
    void OffFontObserver([in] IFontClientObserver observer);
    void InstallScopeFont([in] FileDescriptor fd, [in] int scope, [in] String srcPath, [out] int outValue);
    void UninstallScopeFont([in] String srcPath, [out] int outValue);
    void GetFontScope([in] String srcPath, [out] int outValue);
}
```

#### IFontClientObserver（`service/IFontClientObserver.idl`）
客户端观察者回调（用于 SA 死亡通知）：
```
[callback] interface IFontClientObserver {
    void OnServiceDied();
}
```

#### IDataMigrationCallback（`service/IDataMigrationCallback.idl`）
```
callback IDataMigrationCallback {
    void Handle([in] EventData eventData);
}
```

#### IDataMigrationCallbackEvent（`service/IDataMigrationCallbackEvent.idl`）
```
enum EventType { HEART_BEAT = 0, PROGRESS_DOING, PROGRESS_RESULT };
struct EventData { EventType event; int timeRemaining; int progressPercentage; int progressResult; };
```

### IPC 层次结构

```
NAPI/ANI 层（font_manager_addon / font_manager_ani）
    ↓
FontManagerKits（抽象基类, service/inner_api）
    ↓
FontManagerClient（客户端实现, DelayedRefSingleton）
    ↓
FontServiceLoadManager（通过 SAMgr 加载 SA 66262, 5000ms 超时）
    ↓
IFontService Proxy（IDL 生成）
    ↓ ---- IPC ----
FontServiceStub（IDL 生成, 服务端）
    ↓
FontManagerServer（SystemAbility, 权限检查, SA 生命周期）
    ↓
FontManager（frameworks/fontmgr 中的核心业务逻辑）
    ↓
FontConfig（JSON 配置管理）+ FontClientRegistry（客户端注册）
```

### 死亡回调机制

#### 客户端死亡 → 服务端感知
```
客户端进程死亡
    → FontClientDeathRecipient::OnRemoteDied（binder 线程）
    → FontClientRegistry::OnClientDied(tokenId)
    → CleanupAppScopeFonts(appIdentifier, userId)  // 删除应用级字体
    → NotifyClientDied() → AddUnloadFontServiceTask()  // 调度 SA 退出
```

#### SA 死亡 → 客户端感知
```
SA 进程死亡
    → FontServiceDeathRecipient::OnRemoteDied（binder 线程, 客户端侧）
    → observer_->OnServiceDied()  // 通知应用重新注册 + 重新安装
```

**FontServiceDeathRecipient**（在 `font_manager_client.h` 中）：在 `OnFontObserver` 中添加到 SA 代理 binder。`OnFontObserver` 先移除旧接收器再添加新的（防止累积）。`OffFontObserver` 先执行 IPC 再移除接收器（确保服务端处理期间 stub 存活）。

### SA 启动清理策略

| 场景 | startReason | 清理范围 | 方法 |
|------|-------------|----------|------|
| 开机 | `BOOT_COMPLETED` | 所有用户：应用(scope=0) + 会话(scope=1) | `CleanupAllScopeFontsOnBoot()` |
| 用户停止 | `USER_STOPPING` | 该用户：应用 + 会话 | `CleanupUserScopeFonts(userId)` |
| SA 重启（被杀） | 其他 | 所有用户：**仅应用级**(scope=0) | `CleanupAppScopeFontsOnStart()` |

### SA 退出策略

- 有注册客户端 → SA 常驻
- 无注册客户端 → 10 秒空闲自动退出（`DELAY_MILLISECONDS_FOR_UNLOAD_SA = 10000`）
- 退出任务：`ClearAllTempFileDir()` + `CleanupScopeFontDirs()`（清理临时 + 移除空 `app_*/`/`session_*/` 目录，不删除有文件的字体）
- 任务名：`"font_service_unload"`

### 服务端（`service/server/`）

#### FontManagerServer（`font_manager_server.h/.cpp`）
- 继承：`SystemAbility`、`FontServiceStub`
- 宏：`DECLARE_SYSTEM_ABILITY(FontManagerServer)`、`REGISTER_SYSTEM_ABILITY_BY_ID(FontManagerServer, FONT_SA_ID, false)`、`DISALLOW_COPY_AND_MOVE(FontManagerServer)`

**Outer/Inner 模式**，用于全部 10 个 IPC 方法：
```cpp
int32_t InstallScopeFont(...) {
    RemoveUnloadFontServiceTask();
    callingCount_++;
    InstallScopeFontInner(fd, scope, srcPath, outValue);  // 权限 + 逻辑
    callingCount_--;
    if (callingCount_ == 0 && GetClientCount() == 0) {
        AddUnloadFontServiceTask();
    }
    return ERR_OK;
}
```

**权限检查：**
- `CheckPermission()` - `ohos.permission.UPDATE_FONT`（用户级操作）
- `CheckScopeFontPermission()` - `ohos.permission.UPDATE_SCOPE_FONT`（作用域字体操作）

**应用身份（服务端侧）：**
- `GetBundleNameByToken()` - `AccessTokenKit::GetHapTokenInfo(callerToken, tokenInfo)` → `tokenInfo.bundleName`
- `GetCallingUserId()` - `OsAccountManager::GetOsAccountLocalIdFromUid`
- `MakeAppIdentifier(scope, userId, tokenId)` - `"app_<tokenId>"` 或 `"session_<tokenId>"`

**生命周期：**
- `OnStart()`：创建 `EventHandler`、注册 `ClientDiedCallback`、根据原因投递清理任务、`Publish(this)`
- `OnStop()`：仅记录日志
- 自动卸载：10 秒空闲后，当 `callingCount_ == 0`、未在迁移中、且 `GetClientCount() == 0` 时

### 客户端（`service/client/`）

#### FontManagerClient（`font_manager_client.h/.cpp`）
- 继承：`FontManagerKits`、`DelayedSingleton<FontManagerClient>`（`DECLARE_DELAYED_REF_SINGLETON`）
- `DISALLOW_COPY_AND_MOVE(FontManagerClient)`

**FontServiceDeathRecipient**（内部类）：监控 SA 死亡 → 调用 `observer_->OnServiceDied()`。

方法：
- `int32_t InstallFont/UninstallFont/DataMigration` - 用户级操作
- `int32_t InstallFontWithUserId/UninstallFontWithUserId` - 带显式 userId
- `int32_t OnFontObserver(const sptr<IFontClientObserver>& observer)` - 移除旧死亡接收器 → 添加新的 → IPC
- `int32_t OffFontObserver(const sptr<IFontClientObserver>& observer)` - 先 IPC → 再移除死亡接收器
- `int32_t InstallScopeFont/UninstallScopeFont/GetFontScope` - 作用域字体操作

#### FontServiceLoadManager（`font_service_load_manager.h/.cpp`）
单例。管理 SA 加载，使用 `LoadSaStatus` 枚举（`WAIT_RESULT`、`SUCCESS`、`FAIL`）。
- `sptr<IFontService> GetFontServiceAbility(int32_t systemAbilityId)` - 快速路径 `CheckSystemAbility`，否则 `LoadSa()` 带 5000ms 条件变量等待
- `bool UnloadFontService(int32_t systemAbilityId)` - 锁定 `serviceLock_`，调用 `samgr->UnloadSystemAbility`

#### FontClientObserverAgent（`font_client_observer_agent.h/.cpp`）
继承 `FontClientObserverStub`。包装 `std::function<void()>` 回调用于 `OnServiceDied`。

#### DataMigrationCbAgent（`data_migration_cb_agent.h/.cpp`**
继承 `DataMigrationCallbackStub`。包装 `IDataMigrationListener` 用于 IPC 回调。

### 内部 API（`service/inner_api/`）

#### FontManagerKits（`font_manager_kits.h/.cpp`**
抽象基类。`DISALLOW_COPY_AND_MOVE`。
- `static FontManagerKits& GetInstance()` - 返回 `DelayedRefSingleton<FontManagerClient>::GetInstance()`
- 纯虚方法：`InstallFont`、`UninstallFont`、`DataMigration`、`OnFontObserver`、`OffFontObserver`、`InstallScopeFont`、`UninstallScopeFont`、`GetFontScope`

#### FontManagerInnerApi（`font_manager_inner_api.h/.cpp`）
静态方法（内部 kits，暴露头文件）：
- `static int32_t InstallFont(const std::string &fontPath, int32_t userId)`
- `static int32_t UninstallFont(const std::string &fontName, int32_t userId)`
- `static int32_t InstallScopeFont(const std::string &fontPath, int32_t scope, int32_t userId)`
- `static int32_t UninstallScopeFont(const std::string &srcPath, int32_t userId)`

## NAPI 绑定（`interfaces/js/kits/`）

### FontManagerAddon（`font_manager_addon.h/.cpp`**
- 初始化：`FontManagerAddonInit()` 绑定 `installFont`、`uninstallFont`、`dataMigration`、`onFontObserver`、`offFontObserver`、`installScopeFont`、`uninstallScopeFont`、`getFontScope`
- 用户级安装/卸载：异步工作（`ProcessFontByValue`），Promise + callback
- 作用域字体安装/卸载：异步工作，返回 `Promise<void>`（成功 resolve undefined，失败 reject BusinessError）
- 作用域字体 getScope：异步工作，返回 `Promise<FontScope>`（成功 resolve FontScope，失败 reject BusinessError）
- `OnFontObserver`/`OffFontObserver`：同步，返回 `void`（失败抛出 BusinessError），创建 `FontClientObserverAgent`
- `FontScope` 枚举暴露为 `FontScope.APP`（0）和 `FontScope.SESSION`（1）

错误消息映射包含所有作用域字体错误码（31100112-31100115）。

### JsDataMigrationListener（`js_data_migration_listener.h/.cpp`**
实现 `IDataMigrationListener`。通过 `napi_send_event`（高优先级）分发事件。

### JsFuncRefHolder（`js_func_ref_holder.h/.cpp`**
继承 `NoCopyable`。持有 JS 函数的 `napi_ref`。析构函数通过 `napi_send_event` 立即删除引用。

## ANI 绑定（`interfaces/ani/`）

### ArkTS API（`ets/@ohos.fontManager.ets`）
- `enum FontScope { APP = 0, SESSION = 1 }`
- `interface FontClientObserver { onServiceDied(): void }`
- `function onFontObserver(observer: FontClientObserver): void`
- `function offFontObserver(observer: FontClientObserver): void`
- `function installScopeFont(url: string, scope: FontScope): Promise<void>`
- `function uninstallScopeFont(url: string): Promise<void>`
- `function getFontScope(url: string): Promise<FontScope>`
- 用户级：`installFont`、`uninstallFont`、`dataMigration`（封装在 `taskpool.execute` 中）

### FontManagerAni（`font_manager_ani.h/.cpp`**
- `OnFontObserver`/`OffFontObserver`：使用 `AniObserverRef` 辅助器创建 `FontClientObserverAgent`
- `AniObserverRef`：通过 `ani_vm*` + 全局 `ani_ref` 实现跨线程 ANI 回调，使用 `Object_CallMethodByName_Void(obj, "onServiceDied", ":")`
- `InstallScopeFont`/`UninstallScopeFont`：返回 `void` 的同步原生函数（出错抛出 BusinessError）；封装在 `taskpool.execute((): void => ...)` 中返回 `Promise<void>`
- `GetFontScope`：返回 `ani_int`（scope 值）的同步原生函数；封装在 `taskpool.execute((): int => ...)` 中返回 `Promise<FontScope>`

## 系统能力配置

### SA 配置（`sa_profile/66262.json`）
```json
{
    "process": "font_manager_server",
    "systemability": [{
        "name": 66262,
        "libpath": "libfont_manager_server.z.so",
        "run-on-create": false,
        "distributed": false,
        "dump_level": 1,
        "start-on-demand": {
            "allow-update": true,
            "commonevent": [
                { "name": "usual.event.BOOT_COMPLETED" },
                { "name": "usual.event.USER_STOPPING" }
            ]
        }
    }]
}
```

注意：`sa_profile` JSON 不支持 commonevent 条目的 `permission` 字段。`USER_STOPPING` 所需的 `ohos.permission.INTERACT_ACROSS_LOCAL_ACCOUNTS` 由 SAMgr/foundation 进程持有。

### 服务配置（`service/etc/font_manager_server.cfg`）
- **开机任务**：`mkdir /data/service/el1/public/for-all-app/fonts/ 0755 font_manager font_manager`
- **服务**：uid `font_manager`、gid `["font_manager", "shell"]`、ondemand `true`、secon `u:r:font_manager_server:s0`
- **权限**：`ohos.permission.MANAGE_LOCAL_ACCOUNTS`、`ohos.permission.STORAGE_MANAGER`、`ohos.permission.INTERACT_ACROSS_LOCAL_ACCOUNTS`

### 系统参数（`service/param/`）
- `persist.fontmanager.updateflag = false`（DAC：`font_manager:font_manager:0775`）
- `const.fontmanager.extstoragebundle` - 由 `StorageManagerAdapter` 读取用于包统计业务名

## 权限

- **API 权限检查**（服务端侧）：
  - `ohos.permission.UPDATE_FONT` - 用户级字体安装/卸载、数据迁移。通过 `AccessTokenKit::VerifyAccessToken` 验证。返回 `ERR_NO_PERMISSION`（201）。
  - `ohos.permission.UPDATE_SCOPE_FONT` - 作用域字体操作（安装/卸载/观察者）。通过 `CheckScopeFontPermission()` 验证。
- **服务进程权限**（`service/etc/font_manager_server.cfg`）：`ohos.permission.MANAGE_LOCAL_ACCOUNTS`、`ohos.permission.STORAGE_MANAGER`、`ohos.permission.INTERACT_ACROSS_LOCAL_ACCOUNTS`
- **测试权限**（`test/common/permission_common`）：`ohos.permission.MANAGE_LOCAL_ACCOUNTS`、`ohos.permission.UPDATE_FONT`、APL `system_basic`。测试 UID：`ROOT_UID=0`、`FONT_MANAGER_UID=1015`。

## 构建配置

### GN 构建目标

| 目标 | 路径 | 说明 |
|------|------|------|
| `font_service_ability` | `service:font_service_ability` | 服务组（服务端 + 客户端 + etc + param） |
| `font_manager_server` | `service:font_manager_server` | 服务端共享库（sa 类型） |
| `font_manager_client` | `service:font_manager_client` | 客户端共享库（inner_kits） |
| `fontmanager` | `interfaces/js/kits:fontmanager` | NAPI 绑定 |
| `ani_package_font_manager` | `interfaces/ani:ani_package_font_manager` | ANI 绑定 |
| `font_server_profile` | `sa_profile:font_server_profile` | SA 配置 |

### 组件依赖（来自 `bundle.json`）

`ability_base`、`ability_runtime`、`access_token`、`common_event_service`、`bounds_checking_function`、`c_utils`、`cJSON`、`eventhandler`、`hilog`、`hisysevent`、`hitrace`、`init`、`libuv`、`ipc`、`napi`、`node`、`safwk`、`samgr`、`graphic_2d`、`runtime_core`、`os_account`、`storage_service`

关键框架依赖（`fontmgr.gni`）：`graphic_2d:2d_graphics`、`graphic_2d:rosen_text`（字体名提取）、`init:libbegetutil`、`storage_service:storage_manager_sa_proxy`、`common_event_service:cesfwk_innerkits`。

### 内部 Kits

暴露头文件：`service/inner_api/include/font_manager_inner_api.h`，来自目标 `//base/global/font_manager/service:font_manager_client`。

### 符号映射
- `service/font_manager_client.map` - 导出 `FontManagerKits::GetInstance()`、`FontManagerInnerApi::InstallFont`/`UninstallFont`（仅用户级；作用域字体内部 API 方法未导出）、`FontServiceLoadManager` 符号（版本 1.0）
- `interfaces/ani/fontManager_ani.map` - 仅导出 `ANI_Constructor`

### 构建特性

- 客户端库：cflags `-Os`（优化大小）、`branch_protector_ret: "pac_ret"`
- 服务端库：sanitize（`boundary_sanitize`、`cfi`、`cfi_cross_dso`、`integer_overflow`、`ubsan`）
- 当 `os_account` 部分存在时条件定义 `ACCOUNT_ENABLE`
- 当 `current_cpu == "arm64"` 时条件定义 `USE_EXTENSION_DATA`
- 测试定义 `SUPPORT_GRAPHICS`

## 字体格式处理

- **支持格式**：TTF（1 个全名）和 TTC（多个全名），通过 `OHOS::Rosen::FontToolSet::GetInstance().GetFontFullName(fd)`（来自 `graphic_2d:rosen_text`）
- **最大文件大小**：1024 MB（1GB）——在 `GetFullNamesByFd`（解析前 fstat 预检）和 `CopyFileByFd`（sendfile 前 fstat 预检）中均执行。双重保护。
- **最大安装数**：每用户 200 个字体（默认，通过 `const.fontmanager.maxinstallnum` 参数配置）
- **文件拷贝**：`sendfile()` 系统调用带 EINTR 重试循环、`chmod 0644`
- **重复处理（用户级）**：如果目标文件已存在，在文件名前添加时间戳 `YYYYMMDD-HHMMSS_`
- **重复处理（作用域）**：相同 `srcPath`（URL）→ `ERR_INSTALLED_ALRADY`。相同字体名 → `ERR_INSTALLED_ALRADY`。
- **配置存储**：每用户 `install_fontconfig.json`，映射字体路径到全名数组 + 作用域元数据

## 数据迁移

### 迁移流程

将字体从公共安装路径（`INSTALL_PATH_APP`）迁移到各用户路径（`/data/service/el1/<userId>/for-all-app/fonts/`）。

1. `InitDataMigrationEnv()` - 获取所有 OS 账号用户 ID、删除临时目录、检查 `INSTALL_PATH_APP` 是否为空（返回 `ERR_NOT_NEED_DATA_MIGRATION`）、初始化临时目录
2. `StartHeartBeatTask()` - 分离线程每 60 秒发送 `HEART_BEAT` 事件（使用 `weak_ptr` 检测完成）
3. `StartDataMigration()` - 列出 `INSTALL_PATH_APP` 中的文件，拷贝每个到所有用户目录
4. `CheckAndUpdateAllFontRecord()` - 对每个用户调用 `FontConfig::CheckAndUpdateFontRecord()` 升级配置版本到 "7.0"
5. 每用户 `ReportFontBundleStats()`、`CollectDataMigrationState()`、发送 `PROGRESS_RESULT`

### 迁移错误码（`DataMigrationResultCode`）

```cpp
enum DataMigrationResultCode {
    ERR_NOT_NEED_DATA_MIGRATION = 1,
    ERR_GET_ALL_USERIDS,        // 2
    ERR_CHECK_INSTALL_DIR,      // 3
    ERR_INIT_TEMP_DIR,          // 4
    ERR_OPEN_SRC_FILE,          // 5
    ERR_COPY_FILE,              // 6
    ERR_RENAME_FILE,            // 7
    ERR_REMOVE_SRC_FILE,        // 8
};
```

## 测试

### 测试结构

```
test/
├── common/
│   ├── permission_common.h/.cpp       # PermissionCommon 工具
├── fuzztest/
│   ├── utils/fuzz_data.h/.cpp         # NewInt32, NewString 辅助
│   ├── serviceinstallfont_fuzzer/     # InstallFont 模糊测试
│   ├── serviceuninstallfont_fuzzer/   # UninstallFont 模糊测试
│   ├── servicedatamigration_fuzzer/   # DataMigration 模糊测试
│   ├── serviceinstallscopefont_fuzzer/ # InstallScopeFont 模糊测试（fd + scope + srcPath）
│   ├── serviceuninstallscopefont_fuzzer/ # UninstallScopeFont 模糊测试（srcPath）
│   └── servicegetfontscope_fuzzer/    # GetFontScope 模糊测试（srcPath）
└── unittest/
    ├── BUILD.gn
    ├── ohos_test.xml                 # 推送 11 个测试文件到 /data/test/
    ├── data/                          # 测试字体文件（LFS 跟踪）
    ├── include/
    │   ├── callback_mock.h            # TestCallback（IRemoteStub<IDataMigrationCallback>）
    │   └── hisysevent_adapter_test.h
    └── src/
        ├── font_manager_test.cpp      # FontManagerTest（58 个用例）
        ├── font_config_test.cpp       # FontConfigTest（58 个用例）
        ├── font_client_registry_test.cpp # FontClientRegistryTest（14 个用例）
        ├── data_migration_manager_test.cpp  # DataMigrationManagerTest（22 个用例）
        ├── font_manager_utils_test.cpp      # FontManagerUtilsTest（28 个用例）
        ├── font_manager_client_test.cpp     # FontManagerClientTest（23 个用例）
        ├── font_manager_inner_api_test.cpp  # FontManagerInnerApiTest（5 个用例）
        ├── hisysevent_adapter_test.cpp      # HisyseventAdapterTest（6 个用例）
        └── storage_manager_adapter_test.cpp # StorageManagerAdapterTest（6 个用例）
```

### 测试目标

- `ohos_unittest("font_manager_module_test")` - 核心测试 + `fontmgr_src`，定义 `SUPPORT_GRAPHICS`
- `ohos_unittest("hisysevent_adapter_test")` - HiSysEvent 测试
- 组 `unittest`：两个测试目标
- 组 `fuzztest`：6 个模糊测试目标

### 模糊测试

| 目标 | 模糊测试函数 |
|------|--------------|
| `ServiceInstallFontFuzzTest` | `service->InstallFont(fd, result)` |
| `ServiceUnInstallFontFuzzTest` | `service->UninstallFont(fontName, result)` |
| `ServiceDataMigrationFuzzTest` | `service->DataMigration(cb)` |
| `ServiceInstallScopeFontFuzzTest` | `service->InstallScopeFont(fd, scope, srcPath, result)` |
| `ServiceUninstallScopeFontFuzzTest` | `service->UninstallScopeFont(srcPath, result)` |
| `ServiceGetFontScopeFuzzTest` | `service->GetFontScope(srcPath, result)` |

模糊测试配置：`max_len=1000`、`max_total_time=120s`、`rss_limit_mb=4096`。

### 测试工具

- `PermissionCommon` - 模拟权限设置（`SetFontManagerPermission`、`SetFontManagerInitEnv`、`GrantPermission`、`ResetTokenAndUid`）
- `TestCallback` - `IRemoteStub<IDataMigrationCallback>` mock
- `MockFontClientObserver` - `IRemoteStub<IFontClientObserver>` mock（在 `font_client_registry_test.cpp` 中）
- 测试字体位于 `/data/test/`（由 `ohos_test.xml` preparer 推送）

### 运行测试

```bash
# 构建单元测试（unittest 组）
./build.sh --product-name rk3568 --ccache --build-target //base/global/font_manager/test/unittest:unittest

# 或构建单个测试目标
./build.sh --product-name rk3568 --ccache --build-target font_manager_module_test
./build.sh --product-name rk3568 --ccache --build-target hisysevent_adapter_test

# 运行测试（需设备）
./font_manager_module_test
./hisysevent_adapter_test
```

## 常见模式

### 文件描述符处理

```cpp
if (fd < 0) {
    FONT_LOGE("Invalid file descriptor");
    return ERR_INVALID_PARAM;
}
if (fd >= 0) {
    close(fd);
}
```

### 字体名提取

```cpp
// 从字体文件提取全名（TTF: 1 个, TTC: 多个）
std::vector<std::string> fullNames = FontManagerUtils::GetFullNamesByFd(fd);
// 解析前通过 fstat 预检 1GB
```

### 配置文件操作

```cpp
// 线程安全配置访问
FontConfig& config = SafeGetOrCreateConfig(userId, configPath);
// 用户级: InsertFontRecord/DeleteFontRecord
// 作用域: InsertScopeFontRecord/DeleteScopeFontRecordByUrl
```

## 代码审查清单

- [ ] 版权头存在
- [ ] 头文件保护正确
- [ ] 命名空间层次正确（`OHOS::Global::FontManager`）
- [ ] 错误处理使用适当错误码
- [ ] 日志使用正确级别和格式化符
- [ ] 需要处线程安全（互斥锁保护、原子标志）
- [ ] 无递归互斥锁锁定（内部辅助方法不得锁定与调用方相同的互斥锁）
- [ ] 资源清理（文件描述符、内存、napi_ref、ani_ref）
- [ ] 新功能已添加测试
- [ ] 一致遵循命名规范
- [ ] IPC 方法存在权限检查（用户级 `UPDATE_FONT`，作用域 `UPDATE_SCOPE_FONT`）
- [ ] 服务端方法 SA 卸载任务调度（Outer/Inner 模式）
- [ ] 死亡接收器清理（OnFontObserver 中无累积）
- [ ] OffFontObserver：先 IPC 后清理（服务端处理期间 stub 存活）

## 知识路由

### 编辑前，请声明任务类别、已读文件和适用的约束。

| 任务类别 | 需读文件 | 约束 |
|----------|----------|------|
| 新增/修改 IPC 方法 | `service/IFontService.idl`、`service/BUILD.gn`（IDL 生成）、`service/server/src/font_manager_server.cpp`、`service/client/src/font_manager_client.cpp` | IDL 生成的 stub/proxy 不得手动编辑；见约束 |
| 新增/修改公开 SDK API | `interfaces/ani/ets/@ohos.fontManager.ets`、`interfaces/js/kits/src/font_manager_addon.cpp` | API 签名和错误码是公开契约；见约束 |
| 新增/修改错误码 | `common/include/font_define.h`、`interfaces/js/kits/src/font_manager_addon.cpp`（错误映射） | 错误码数值是公开契约；见约束 |
| 新增/修改权限检查 | `service/server/src/font_manager_server.cpp`（`CheckPermission`、`CheckScopeFontPermission`） | 权限字符串是系统安全契约；见约束 |
| 新增/修改 SA 生命周期 | `sa_profile/66262.json`、`service/etc/font_manager_server.cfg`、`service/server/src/font_manager_server.cpp`（`OnStart`、`OnStop`） | SA ID 66262 固定不变；见约束 |
| 新增/修改配置模式 | `frameworks/fontmgr/src/font_config.cpp`、`frameworks/fontmgr/include/font_config.h` | 配置 JSON 模式须保持 OTA 兼容性；见约束 |
| 新增/修改数据迁移 | `frameworks/fontmgr/src/data_migration_manager.cpp` | `isDataMigrationing_` 标志防止并发迁移，不得绕过 |
| 新增/修改作用域字体 | `frameworks/fontmgr/src/font_client_registry.cpp`、`frameworks/fontmgr/src/font_manager.cpp`（作用域方法） | 每用户限制 `MAX_SCOPE_FONT_APP_NUM=5` 在注册表中强制执行 |

### 词汇表

| 术语 | 含义 | 位置 |
|------|------|------|
| 作用域字体 | 应用级或会话级第三方字体（相对于用户级持久字体） | `frameworks/fontmgr/` |
| 应用标识 | 字符串 `"app_<tokenId>"` 或 `"session_<tokenId>"`，用于作用域字体隔离 | `service/server/src/font_manager_server.cpp` |
| 死亡接收器 | 用于客户端/SA 死亡通知的 binder 死亡回调 | `service/client/include/font_manager_client.h`（`FontServiceDeathRecipient`）、`frameworks/fontmgr/src/font_client_registry.cpp` |
| OTA 兼容性 | 旧配置记录（无 scope 字段）视为用户级；`scope` 默认为 -1 | `frameworks/fontmgr/src/font_config.cpp` |
| Outer/Inner 模式 | 服务端 IPC 方法在 `RemoveUnloadFontServiceTask`/`callingCount_++`/`Inner()`/`callingCount_--`/`AddUnloadFontServiceTask` 中包装逻辑 | `service/server/src/font_manager_server.cpp` |
| IDL 生成文件 | `*_proxy.cpp`、`*_stub.cpp` 由 `idl_gen_interface` 从 `.idl` 文件自动生成 | `service/BUILD.gn` |

## 约束 — 禁止 / 需确认

### 禁止

- **禁止**手动编辑 IDL 生成的 `*_proxy.cpp` 或 `*_stub.cpp` 文件——这些由 `service/BUILD.gn` 中的 `idl_gen_interface` 从 `.idl` 文件自动生成。请编辑 `.idl` 文件并重新构建。
- **禁止**更改 `FontErrorCode` 数值（如 `ERR_OK = 0`、`ERR_NO_PERMISSION = 201`、`ERR_FILE_NOT_EXISTS = 31100101`）——这些是被应用引用的公开 API 契约。
- **禁止**更改 SA ID `66262`（`common/include/font_define.h` 中的 `FONT_SA_ID`）——它已在 `sa_profile/66262.json` 中全系统注册。
- **禁止**更改权限字符串（`ohos.permission.UPDATE_FONT`、`ohos.permission.UPDATE_SCOPE_FONT`）——它们是系统安全契约。
- **禁止**在没有 OTA 迁移路径的情况下更改 `install_fontconfig.json` 模式版本（`"7.0"`）——旧记录必须仍能正确解析。
- **禁止**绕过 `isDataMigrationing_` 原子标志——它防止并发数据迁移。
- **禁止**使用 `std::recursive_mutex`——代码库使用非递归 `std::mutex`；内部辅助方法不得锁定与调用方相同的互斥锁（见线程安全）。
- **禁止**遗漏服务端 IPC 方法中的 Outer/Inner 模式——每个方法必须调度/取消调度 SA 卸载任务。

### 需确认

- **需确认**在 `IFontService.idl` 中新增或删除 IDL 方法——会改变 IPC 二进制接口，需要同步更新 NAPI + ANI 绑定。
- **需确认**更改 `MAX_INSTALL_NUM`（200）或 `MAX_SCOPE_FONT_APP_NUM`（5）——影响系统资源限制。
- **需确认**更改 `DELAY_MILLISECONDS_FOR_UNLOAD_SA`（10000）——影响 SA 生命周期和功耗。
- **需确认**修改 `font_manager_client.map` 或 `fontManager_ani.map` 符号导出列表——改变共享库 ABI。
- **需确认**更改 `service/etc/font_manager_server.cfg` 进程权限——影响系统安全态势。

### 不变式

- `FontConfig` 中的 `std::mutex`（`configLock_`）不可递归——`WriteToFile`、`GetFileData`、`CheckConfigFile` 不得锁定它；调用它们的公开方法已持有锁。
- `OnFontObserver` 必须先移除旧死亡接收器再添加新的——防止接收器累积。
- `OffFontObserver` 必须先执行 IPC 再移除死亡接收器——确保服务端处理期间 stub 存活。
- 无 `scope`/`srcPath`/`appIdentifier` 字段的旧配置记录视为用户级（`scope = -1`）。
- SA 清理策略：开机 → 所有作用域字体；用户停止 → 该用户的作用域字体；SA 重启 → 仅应用级（会话级保留）。

## 验证

### 构建验证

```bash
# 构建整个字体管理服务
./build.sh --product-name rk3568 --ccache --build-target font_manager

# 构建所有单元测试（unittest 组）
./build.sh --product-name rk3568 --ccache --build-target //base/global/font_manager/test/unittest:unittest

# 构建单个测试目标
./build.sh --product-name rk3568 --ccache --build-target font_manager_module_test
./build.sh --product-name rk3568 --ccache --build-target hisysevent_adapter_test

# 构建模糊测试
./build.sh --product-name rk3568 --ccache --build-target //base/global/font_manager/test/fuzztest:fuzztest
```

### Lint 和静态分析

本仓库中不存在 `.clang-format` 或 `.clang-tidy` 配置。服务端共享库（`font_manager_server`）通过 `BUILD.gn` 自动启用以下 sanitizer：
- `boundary_sanitize`
- `cfi`（控制流完整性）+ `cfi_cross_dso`
- `integer_overflow`
- `ubsan`（未定义行为 sanitizer）

这些在构建时运行——无需单独的 lint 命令。

### 运行测试

```bash
# 运行单元测试（需设备，测试文件推送到 /data/test/）
./font_manager_module_test
./hisysevent_adapter_test
```

测试数据（11 个字体/配置文件）由 `ohos_test.xml` preparer 推送到 `/data/test/`。需要 LFS 获取测试字体二进制文件。

### 提交前检查清单

- [ ] 构建无错误（`--build-target font_manager`）
- [ ] 单元测试可构建（`--build-target //base/global/font_manager/test/unittest:unittest`）
- [ ] 如果新增 IPC 方法：已更新 `.idl` 文件、服务端 stub 实现、客户端代理调用、NAPI + ANI 绑定
- [ ] 如果新增错误码：已添加到 `enum FontErrorCode`、NAPI 错误消息映射已更新
- [ ] 如果修改权限：服务端 `CheckPermission`/`CheckScopeFontPermission` 已更新
- [ ] 线程安全已验证（无递归互斥锁、并发状态使用原子标志）
- [ ] SA 卸载任务调度已验证（Outer/Inner 模式）
- [ ] 所有新源文件版权头存在
- [ ] 头文件保护遵循 `GLOBAL_FONT_MANAGER_<NAME>_H` 约定

### 完成定义

任务完成需满足：
1. 构建无错误
2. 相关单元测试可构建并通过（如有设备）
3. 无 IDL 生成文件被手动编辑
4. 最终回复说明：改了什么、涉及哪些文件、适用哪些清单项

### 降级方案

如果没有设备可用无法执行测试：
1. 运行仅构建验证（`--build-target font_manager` 和 `--build-target //base/global/font_manager/test/unittest:unittest`）
2. 在 PR 描述中说明测试已构建但未执行（无设备）
3. 列出需要设备验证的测试用例
