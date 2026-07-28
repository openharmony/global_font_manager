# AGENTS.md - Font Manager Codebase Guide

This file provides guidelines for agentic coding agents working in the font_manager repository.

## Overview

The `font_manager` component (`@ohos.fontManager`, version 5.0) is part of the OpenHarmony `global` subsystem. It provides system-level font installation/uninstall, app-level/session-level third-party font installation/uninstall, and data migration services for OpenHarmony devices. The service runs as SystemAbility `66262` (`font_manager_server` process) and exposes APIs via NAPI (JavaScript/ArkTS) and ANI (ArkTS native) bindings.

- **SystemCapability**: `SystemCapability.Global.FontManager`
- **SA ID**: `66262`
- **License**: Apache License 2.0

### Font Scope Levels

| Scope | Value | Lifecycle | Cleanup Trigger | Permission |
|-------|-------|-----------|-----------------|------------|
| User-level | -1 (default/legacy) | Persistent | Manual uninstall only | `ohos.permission.UPDATE_FONT` |
| App-level | 0 (`FONT_SCOPE_APP`) | While app registered | App exit / SA restart / boot | `ohos.permission.UPDATE_SCOPE_FONT` |
| Session-level | 1 (`FONT_SCOPE_SESSION`) | Current boot session | Boot only | `ohos.permission.UPDATE_SCOPE_FONT` |

## Build System

This project uses the GN (Generate Ninja) build system.

### Build Commands

```bash
# Build the entire font manager service (run from OpenHarmony root, e.g. ~/openharmony)
./build.sh --product-name rk3568 --ccache --build-target font_manager

# Build all unit tests
./build.sh --product-name rk3568 --ccache --build-target font_manager_test
```

## Repository Structure

```
font_manager/
├── AGENTS.md
├── bundle.json                          # Component manifest & dependencies
├── common/include/                      # Shared headers (no .cpp, header-only)
│   ├── font_define.h                    # Error codes, constants, scope font constants
│   ├── font_hilog.h                     # Logging macros (FONT_LOGD/I/W/E)
│   └── idata_migration_listener.h       # Abstract listener interface
├── frameworks/fontmgr/                  # Core business logic library
│   ├── fontmgr.gni                      # GN import file (sources, includes, deps)
│   ├── include/                         # 8 headers
│   └── src/                             # 8 implementations
├── interfaces/
│   ├── ani/                             # ANI (ArkTS native) bindings
│   │   ├── ets/@ohos.fontManager.ets    # ArkTS API surface (includes scope font + observer)
│   │   ├── include/                     # font_manager_ani.h, ani_data_migration_listener.h
│   │   └── src/                         # .cpp implementations
│   └── js/kits/                         # NAPI (JavaScript) bindings
│       ├── include/                     # font_manager_addon.h, font_napi_callback.h, ...
│       └── src/                         # .cpp implementations
├── sa_profile/
│   ├── 66262.json                       # SA profile (libpath, start-on-demand commonevent)
│   └── BUILD.gn
├── service/                             # IPC service layer (client + server)
│   ├── BUILD.gn                         # IDL generation, client & server libs
│   ├── IFontService.idl                 # Main IPC interface (11 methods)
│   ├── IDataMigrationCallback.idl       # Data migration callback interface
│   ├── IDataMigrationCallbackEvent.idl  # EventType enum + EventData struct
│   ├── IFontClientObserver.idl          # Client observer callback (OnServiceDied)
│   ├── font_manager_client.map          # Symbol export map
│   ├── client/                          # Client-side (proxy, SA loader, callback/observer agents)
│   ├── etc/font_manager_server.cfg      # Service init config (mkdir, uid, permissions)
│   ├── inner_api/                       # Inner API (FontManagerKits, FontManagerInnerApi)
│   ├── param/                           # System parameters (.para, .para.dac)
│   └── server/                          # Server-side (FontManagerServer SA)
└── test/
    ├── common/                          # PermissionCommon test utility
    ├── fuzztest/                        # 6 fuzz targets + utils
    └── unittest/                        # Unit tests + test data
```

## Code Style Guidelines

### File Structure and Organization

- **Copyright Header**: All source files begin with Apache 2.0 copyright header
- **Include Guards**: Use `#ifndef` style guards. Note: the codebase has two formats in use:
  - `GLOBAL_FONT_MANAGER_<FILENAME>_H` (e.g., `GLOBAL_FONT_MANAGER_FONT_DEFINE_H`, `GLOBAL_FONT_MANAGER_FONT_MANAGER_H`)
  - `FONT_MANAGER_<FILENAME>_H` (e.g., `FONT_MANAGER_DATA_MIGRATION_MANAGER_H`) - prefer the `GLOBAL_FONT_MANAGER_` prefix for new files
- **Include Order**: System headers, then project headers (alphabetical)
- **File Naming**: `lowercase_with_underscores.cpp` and `lowercase_with_underscores.h`

### Namespace Conventions

```cpp
namespace OHOS {
namespace Global {
namespace FontManager {
    // Code here
} // namespace FontManager
} // namespace Global
} // namespace OHOS
```

### Naming Conventions

- **Classes**: PascalCase (e.g., `FontManager`, `FontConfig`, `FontManagerClient`, `DataMigrationManager`)
- **Functions**: PascalCase for public methods (e.g., `InstallFont`, `UninstallFont`, `DataMigration`)
- **Variables**: camelCase (e.g., `installPath`, `fontFullName`, `userId`)
- **Constants**: `constexpr` or `inline const` for header constants; file-scope `static constexpr` in .cpp (e.g., `MAX_INSTALL_NUM`, `FONT_SA_ID`)
- **Member Variables**: camelCase with trailing underscore for private members (e.g., `configMap_`, `mapLock_`, `isDataMigrationing_`)
- **Enums**: PascalCase type name with UPPER_SNAKE_CASE values (e.g., `FontErrorCode`, `ERR_OK`)

### Type and Formatting

- **Pointers**: Use `sptr<T>` for IPC/binder objects, `std::shared_ptr<T>` for regular smart pointers
- **Constants**: Use `constexpr` for compile-time, `inline const` for header-defined constants
- **String Type**: Use `std::string` consistently
- **Integer Types**: Use `int32_t` for file descriptors and user IDs, `int` for general integers
- **Error Codes**: Return `int32_t` or `ErrCode` (typedef: `using ErrCode = int;`)
- **Boolean**: Use `bool` type with `true`/`false` literals
- **Concurrency**: Use `std::atomic<T>` for flags/counters (e.g., `std::atomic<bool> isDataMigrationing_`)

### Error Handling

Error codes are defined in `common/include/font_define.h` as `enum FontErrorCode`:

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
    // Scope font management (third-party app-level / session-level)
    ERR_SCOPE_FONT_REPEATED_REGISTER = 31100501,
    ERR_SCOPE_FONT_EXCEED_REGISTER_LIMIT = 31100502,
    ERR_SCOPE_FONT_NOT_REGISTERED = 31100503,
};
```

Common pattern - check condition, log error, return error code:
```cpp
if (service == nullptr) {
    FONT_LOGE("Service is null");
    return ERR_SYSTEM_ERROR;
}
```

### Logging

Use custom logging macros defined in `common/include/font_hilog.h`:
- `FONT_LOGD(...)` - Debug level (HILOG_DEBUG)
- `FONT_LOGI(...)` - Info level (HILOG_INFO)
- `FONT_LOGW(...)` - Warning level (HILOG_INFO)
- `FONT_LOGE(...)` - Error level (HILOG_ERROR)

Log configuration:
- `LOG_DOMAIN = 0xD001E00`
- `LOG_TAG = "FONT_MSG"`

Log format specifiers (HiLog privacy):
- `%{public}s` - Public string (not sanitized). TokenId is NOT sensitive, use `%{public}d`.
- `%{private}s` - Private string (sanitized in logs)
- `%{public}d` - Public integer

### Singleton Pattern

Use `DelayedSingleton<T>` for singleton classes:
```cpp
class FontManager : public DelayedSingleton<FontManager> {
    DECLARE_DELAYED_SINGLETON(FontManager);
public:
    int32_t InstallFont(const int32_t &fd, const int32_t userId);
private:
    FontManager();
    ~FontManager();
};
// Usage:
auto manager = FontManager::GetInstance();
```

For reference singletons (non-delayed), use `DelayedRefSingleton<T>`:
```cpp
// FontManagerKits::GetInstance() returns DelayedRefSingleton<FontManagerClient>::GetInstance()
```

### Thread Safety

Use `std::mutex` with `std::lock_guard` for thread-safe operations:
```cpp
std::mutex mapLock_;
std::unordered_map<int32_t, FontConfig> configMap_;

FontConfig& SafeGetOrCreateConfig(int32_t userId, const std::string& configPath) {
    std::lock_guard<std::mutex> lock(mapLock_);
    auto result = configMap_.try_emplace(userId, FontConfig(configPath));
    return result.first->second;
}
```

For atomic flags/counters:
```cpp
std::atomic<bool> isDataMigrationing_ {false};
std::atomic_uint callingCount_ {0};
```

For SA loading with condition variable:
```cpp
std::condition_variable proxyConVar_;
std::mutex serviceLock_;
```

**Critical**: `std::mutex` is NOT recursive. Internal helper methods called from within a locked context must NOT lock the same mutex. In `FontConfig`, `WriteToFile` and `GetFileData` do NOT lock `configLock_` because the calling public method already holds it.

### Testing Guidelines

- Use `HWTEST_F` macro for test cases
- Test class naming: `<ClassName>Test` (e.g., `FontManagerTest`, `FontConfigTest`)
- Test case naming: `<ClassName>FuncTest<Number>` or `<Prefix>Test<Number>` (e.g., `FontManagerFuncTest001`, `InnerApiInstallFontTest001`)
- Test size levels: `TestSize.Level1` (Level1 = critical)
- Use `#define private public` and `#undef private` to test private members when necessary
- Test permissions set up via `PermissionCommon::SetFontManagerPermission(processName)`
- Use `IRemoteStub<T>` pattern for mock IPC objects (see `callback_mock.h`, `font_client_registry_test.cpp`)

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

## Key Constants

Defined in `common/include/font_define.h`:

| Constant | Value | Description |
|----------|-------|-------------|
| `FONT_SA_ID` | `66262` | SystemAbility ID |
| `INSTALL_PATH_APP` | `"/data/service/el1/public/for-all-app/fonts/"` | Public install path |
| `INSTALL_PATH_PREFIX` | `"/data/service/el1/"` | User install path prefix |
| `INSTALL_PATH_SUFFIX` | `"/for-all-app/fonts/"` | User install path suffix |
| `FONT_CONFIG_FILE` | `"install_fontconfig.json"` | Config filename |
| `TEMP_FILE` | `"temp/"` | Temp subdirectory name |
| `EXT_STORAGE_BUNDLE_PARAM_KEY` | `"const.fontmanager.extstoragebundle"` | System parameter key |
| `APP_FONT_DIR_PREFIX` | `"app_"` | App-level font subdirectory prefix |
| `FONT_CONFIG_VERSION_7` | `"7.0"` | Config file version for scope font |
| `MAX_SCOPE_FONT_APP_NUM` | `5` | Max registered observer apps per user |
| `FONT_SCOPE_APP` | `0` | App-level scope constant |
| `FONT_SCOPE_SESSION` | `1` | Session-level scope constant |
| `FONT_SCOPE_NONE` | `-1` | User-level (legacy records) |

File-scope constants in source files:

| Constant | File | Value | Description |
|----------|------|-------|-------------|
| `MAX_INSTALL_NUM` | `font_manager.cpp` | `200` | Max installable fonts per user |
| `MAX_FONT_FILE_SIZE` | `font_manager_utils.cpp` | `1024*1024*1024` (1GB) | Max font file size |
| `DELAY_MILLISECONDS_FOR_UNLOAD_SA` | `font_manager_server.cpp` | `10000` | 10s auto-unload delay |
| `HEARTBEAT_INTERVAL` | `data_migration_manager.cpp` | `60` | Heartbeat seconds |
| `MAX_TRIGGER_COUNT` | `data_migration_manager.cpp` | `100` | Max progress events |
| `COPY_SPEED` | `data_migration_manager.cpp` | `10*1024/60` | Estimated copy speed (Mb/s) |
| `STORAGE_MANAGER_MANAGER_ID` | `storage_manager_adapter.cpp` | `5003` | Storage manager SA ID |

## Core Components

### Frameworks (`frameworks/fontmgr/`)

The core font management logic, built as a library consumed by the server.

#### FontManager (`font_manager.h/.cpp`)
Core install/uninstall business logic. Singleton via `DelayedSingleton<FontManager>`.

**User-level methods:**
- `int32_t InstallFont(const int32_t &fd, const int32_t userId)` - Install font from fd. Validates path, checks duplicates (`ERR_INSTALLED_ALRADY`), enforces `MAX_INSTALL_NUM=200`, copies via temp dir + rename, inserts config record, reports to HiSysEvent/StorageManager, publishes `FontEventType::INSTALL` event.
- `int32_t UninstallFont(const std::string &fontFullName, const int32_t userId)` - Uninstall by full name.

**Scope font methods:**
- `int32_t InstallScopeFont(const ScopeFontInstallInfo &info)` - Install app/session-level font. Upgrades config version, validates srcPath/name dedup, copies to `app_<tokenId>/` or `session_<tokenId>/` subdirectory.
- `int32_t UninstallScopeFont(const std::string &srcPath, const std::string &bundleName, int32_t userId)` - Uninstall by srcPath (URL).
- `int32_t GetFontScope(const std::string &srcPath, int32_t userId)` - Query scope by srcPath. Returns `FONT_SCOPE_NONE` (-1) if not found.
- `int32_t CleanupAppScopeFonts(const std::string &appIdentifier, int32_t userId)` - Clean all app-level fonts for a specific app (called on app death/unregister).
- `int32_t CleanupScopeFontsByUser(int32_t userId)` - Clean all scope fonts (app + session) for a user (boot/user stopping).
- `int32_t CleanupAppScopeFontsByUser(int32_t userId)` - Clean only app-level (scope=0) fonts for a user (SA restart). Session-level fonts preserved.

**ScopeFontInstallInfo struct:**
```cpp
struct ScopeFontInstallInfo {
    int32_t fd = -1;
    int32_t scope = -1;
    std::string srcPath;       // URL provided by app (unique identifier)
    std::string bundleName;
    std::string appIdentifier; // "app_<tokenId>" or "session_<tokenId>"
    int32_t userId = -1;
};
```

Private helpers: `GetFormatFullName`, `CopyFileForInstall`, `SandBoxPathToRealPath`, `SafeGetOrCreateConfig` (thread-safe map access), `GetAppInstallPath`, `ValidateScopeFontForInstall`, `CopyAndInsertScopeFont`.

#### FontConfig (`font_config.h/.cpp`)
Manages the JSON config file (`install_fontconfig.json`). Copy disabled (`DISALLOW_COPY`), move enabled. Single `configLock_` mutex; internal helpers (`WriteToFile`, `GetFileData`, `CheckConfigFile`) do NOT lock (caller holds lock).

**User-level methods:**
- `bool InsertFontRecord(const std::string &fontPath, const std::vector<std::string> &fullNames)` - Add user-level record (no scope field)
- `bool DeleteFontRecord(const std::string &fontPath)` - Remove by fontfullpath
- `int GetInstalledFontsNum()` - Count
- `std::string GetFontFileByName(const std::string &fullName)` - Lookup path by name

**Scope font methods:**
- `bool InsertScopeFontRecord(const FontRecordInfo &record)` - Add scope record (with scope/srcPath/appIdentifier/bundleName)
- `bool DeleteScopeFontRecordByUrl(const std::string &srcPath)` - Remove by srcPath
- `bool DeleteScopeFontRecordByAppId(const std::string &appIdentifier)` - Remove all records for an app
- `std::optional<FontRecordInfo> GetFontRecordByUrl(const std::string &srcPath)` - Lookup by srcPath
- `std::optional<FontRecordInfo> GetFontRecordByName(const std::string &fullName)` - Lookup by font name (for dedup check)
- `std::vector<FontRecordInfo> GetFontRecordsByAppId(const std::string &appIdentifier)` - All records for an app
- `std::vector<FontRecordInfo> GetScopeFontRecords()` - All records where scope >= 0
- `std::vector<FontRecordInfo> GetAppScopeFontRecords()` - All records where scope == FONT_SCOPE_APP (0) only
- `bool CheckAndUpdateFontRecord()` - Upgrade version: int `1` → string `"7.0"`. Does NOT modify old records.
- `int GetTotalInstalledFontsNum()` - Total count (user + scope)

**FontRecordInfo struct:**
```cpp
struct FontRecordInfo {
    std::string fontPath;
    std::vector<std::string> fullNames;
    int32_t scope = -1;           // -1 = user-level, 0 = app, 1 = session
    std::string srcPath;
    std::string appIdentifier;
    std::string bundleName;
};
```

Config JSON format (v7.0):
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

**OTA compatibility**: Old records (no `scope`/`srcPath`/`appIdentifier` fields) are correctly treated as user-level. `FontRecordInfo::scope` defaults to -1. All read methods use `cJSON_GetObjectItem` + nullptr check. Initial config creation uses `"version": "7.0"` directly.

#### FontManagerUtils (`font_manager_utils.h/.cpp`)
Static utility class. All methods static.

- `static bool CheckAndInitInstallPath(const std::string &installPath)` - Init install dir + temp + config
- `static bool CheckAndInitScopeFontPath(const std::string &installPath)` - Init scope dir + temp (no config file)
- `static bool CheckPathExist(const std::string &pathName)` - `std::filesystem::exists`
- `static bool CheckFontConfigPath(const std::string &installPath)` - Create config with `{"fontlist": [], "version": "7.0"}` if missing
- `static bool CreateDirWithPermission(const std::string &fileDir)` - Create dir, remove `others_write`
- `static std::string GetFileName(const std::string &path)` - Substring after last `/`
- `static std::string GetFileDirectory(const std::string &path)` - Substring up to last `/`
- `static bool CopyFile(int32_t sourceFd, const std::string& path)` - `sendfile()`, chmod 0644, checks `MAX_FONT_FILE_SIZE` (1GB)
- `static std::string GetFilePathByFd(const int32_t &fd)` - `readlink` on `/proc/<pid>/fd/<fd>`
- `static bool RenameFile(const std::string& src, const std::string& dest)` - `std::filesystem::rename`
- `static std::string GetFileTime()` - Timestamp `YYYYMMDD-HHMMSS`
- `static bool RemoveFile(const std::string &path)`
- `static void DeleteDir(const std::string &rootPath, bool isDeleteRootDir)`
- `static std::vector<int32_t> GetAllCreatedUserIds()` - Via `OsAccountManager` (guarded by `#ifdef ACCOUNT_ENABLE`)
- `static void ClearAllTempFileDir()` - Clear temp dirs for all users
- `static void CleanupScopeFontDirs()` - Clean temp + remove empty `app_*/`/`session_*/` dirs (SA exit)
- `static void CleanupAllScopeFontDirs()` - Brute-force delete all `app_*/` + `session_*/` dirs (boot)
- `static void CleanupAppScopeFontDirs()` - Brute-force delete all `app_*/` dirs only (SA restart)
- `static std::vector<std::string> GetFullNamesByFd(const int32_t &fd)` - `fstat` precheck 1GB + `FontToolSet::GetFontFullName(fd)` (TTF/TTC)
- `static std::vector<std::string> GetFullNamesByPath(const std::string &path)` - Open file, get fd, call above

**1GB size check**: Performed in `GetFullNamesByFd` (via `fstat` before font parsing) AND in `CopyFileByFd` (via `fstat` before `sendfile`). Double protection.

#### FontClientRegistry (`font_client_registry.h/.cpp`)
Client registration table. Singleton via `DelayedSingleton<FontClientRegistry>`. Manages app-level font lifecycle via death recipients.

- `int32_t RegisterClient(const sptr<IRemoteObject> &observerBinder, const std::string &bundleName, int32_t userId, int32_t tokenId)` - Register client, add death recipient, check duplicate (`ERR_SCOPE_FONT_REPEATED_REGISTER`) and per-user limit `MAX_SCOPE_FONT_APP_NUM=5` (`ERR_SCOPE_FONT_EXCEED_REGISTER_LIMIT`)
- `int32_t UnregisterClient(int32_t tokenId)` - Unregister, remove death recipient, cleanup app fonts, notify SA may exit
- `bool IsClientRegistered(int32_t tokenId)`
- `int32_t GetClientCount()`
- `void OnClientDied(int32_t tokenId)` - Client death callback, cleanup app fonts, notify SA
- `void SetClientDiedCallback(ClientDiedCallback callback)` - Set SA exit notification callback

**Data structures:**
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

#### FontEventPublish (`font_event_publish.h/.cpp`)
Static class for publishing common events. All scope levels (user/app/session) use the same event.

```cpp
enum FontEventType { INSTALL = 0, UNINSTALL = 1 };
static bool PublishFontUpdate(const FontEventType eventType, const std::string &formatName, const int32_t &userId);
```
- **Action**: `usual.event.FONT_UPDATE_FOR_POLICY`
- **Params**: `eventType` (0=INSTALL, 1=UNINSTALL), `fontFullNames` (comma-separated)

#### HisyseventAdapter (`hisysevent_adapter.h/.cpp`)
HiSysEvent publisher. Singleton via `DelayedSingleton<HisyseventAdapter>`.

- `int CollectUserDataSize(const std::string &path)` - Event `USER_DATA_SIZE`, domain `FILEMANAGEMENT`, type `STATISTIC`
- `int CollectDataMigrationState(const std::vector<int32_t> &userIds, int32_t result)` - Event `FONT_DATA_MIGRATION`, domain `FONT_MANAGER`, type `STATISTIC`

#### DataMigrationManager (`data_migration_manager.h/.cpp`)
Orchestrates data migration. Singleton via `DelayedSingleton<DataMigrationManager>`, also `std::enable_shared_from_this`.

- `void DataMigration(const sptr<IDataMigrationCallback>& callback)` - Main entry. Sets `isDataMigrationing_=true`, runs `DataMigrationInner()`, `CheckAndUpdateAllFontRecord()`, reports per-user stats, resets flag, emits result event.

Migration flow: `InitDataMigrationEnv` (get user IDs, check empty -> `ERR_NOT_NEED_DATA_MIGRATION`) -> `StartHeartBeatTask` (detached thread, 60s interval) -> `StartDataMigration` (copy each file from `INSTALL_PATH_APP` to all user dirs).

Progress calculation: `progressPercentage = (i*100 + size/2)/size`, `timeRemaining = (folderSize * idsize) >> 20 / COPY_SPEED`.

#### StorageManagerAdapter (`storage_manager_adapter.h/.cpp`)
Reports font bundle stats to StorageManager SA (ID 5003). Singleton via `DelayedSingleton<StorageManagerAdapter>`.

- `int32_t ReportFontBundleStats(int32_t userId, const std::string &installPath)` - Gets business name from system parameter `const.fontmanager.extstoragebundle`, gets folder size, calls `proxy->SetExtBundleStats(userId, stats)`.

### Common Headers (`common/include/`)

#### font_define.h
Error codes (`FontErrorCode`, `DataMigrationResultCode`), constants (paths, scope font constants), `using ErrCode = int;`

#### font_hilog.h
Logging macros (`FONT_LOGD/I/W/E`), `LOG_DOMAIN = 0xD001E00`, `LOG_TAG = "FONT_MSG"`.

#### idata_migration_listener.h
Abstract listener interface:
```cpp
class IDataMigrationListener {
public:
    virtual ~IDataMigrationListener() = default;
    virtual void OnHandle(const EventData& eventData) = 0;
};
```

## IPC Architecture

### IDL Interfaces

#### IFontService (`service/IFontService.idl`)
Main service interface (package `OHOS.Global.FontManager`):
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

#### IFontClientObserver (`service/IFontClientObserver.idl`)
Client observer callback (for SA death notification):
```
[callback] interface IFontClientObserver {
    void OnServiceDied();
}
```

#### IDataMigrationCallback (`service/IDataMigrationCallback.idl`)
```
callback IDataMigrationCallback {
    void Handle([in] EventData eventData);
}
```

#### IDataMigrationCallbackEvent (`service/IDataMigrationCallbackEvent.idl`)
```
enum EventType { HEART_BEAT = 0, PROGRESS_DOING, PROGRESS_RESULT };
struct EventData { EventType event; int timeRemaining; int progressPercentage; int progressResult; };
```

### IPC Layer Hierarchy

```
NAPI/ANI Layer (font_manager_addon / font_manager_ani)
    ↓
FontManagerKits (abstract base, service/inner_api)
    ↓
FontManagerClient (client impl, DelayedRefSingleton)
    ↓
FontServiceLoadManager (loads SA 66262 via SAMgr, 5000ms timeout)
    ↓
IFontService Proxy (IDL-generated)
    ↓ ---- IPC ----
FontServiceStub (IDL-generated, server-side)
    ↓
FontManagerServer (SystemAbility, permission check, SA lifecycle)
    ↓
FontManager (core business logic in frameworks/fontmgr)
    ↓
FontConfig (JSON config management) + FontClientRegistry (client registration)
```

### Death Callback Mechanism

#### Client death → Server感知
```
Client process dies
    → FontClientDeathRecipient::OnRemoteDied (binder thread)
    → FontClientRegistry::OnClientDied(tokenId)
    → CleanupAppScopeFonts(appIdentifier, userId)  // delete app-level fonts
    → NotifyClientDied() → AddUnloadFontServiceTask()  // schedule SA exit
```

#### SA death → Client感知
```
SA process dies
    → FontServiceDeathRecipient::OnRemoteDied (binder thread, client-side)
    → observer_->OnServiceDied()  // notify app to re-register + re-install
```

**FontServiceDeathRecipient** (in `font_manager_client.h`): Added to SA proxy binder in `OnFontObserver`. `OnFontObserver` removes old recipient before adding new (prevents accumulation). `OffFontObserver` does IPC first, then removes recipient (ensures stub survives during server processing).

### SA Startup Cleanup Strategy

| Scenario | startReason | Cleanup Scope | Method |
|----------|-------------|---------------|--------|
| Boot | `BOOT_COMPLETED` | All users: app(scope=0) + session(scope=1) | `CleanupAllScopeFontsOnBoot()` |
| User stopping | `USER_STOPPING` | That user: app + session | `CleanupUserScopeFonts(userId)` |
| SA restart (killed) | Other | All users: **app only** (scope=0) | `CleanupAppScopeFontsOnStart()` |

### SA Exit Strategy

- Registered clients → SA stays resident
- No registered clients → 10s inactivity auto-exit (`DELAY_MILLISECONDS_FOR_UNLOAD_SA = 10000`)
- Exit task: `ClearAllTempFileDir()` + `CleanupScopeFontDirs()` (clean temp + remove empty `app_*/`/`session_*/` dirs, NOT delete fonts with files)
- Task name: `"font_service_unload"`

### Server (`service/server/`)

#### FontManagerServer (`font_manager_server.h/.cpp`)
- Inherits: `SystemAbility`, `FontServiceStub`
- Macros: `DECLARE_SYSTEM_ABILITY(FontManagerServer)`, `REGISTER_SYSTEM_ABILITY_BY_ID(FontManagerServer, FONT_SA_ID, false)`, `DISALLOW_COPY_AND_MOVE(FontManagerServer)`

**Outer/Inner pattern** for all 11 IPC methods:
```cpp
int32_t InstallScopeFont(...) {
    RemoveUnloadFontServiceTask();
    callingCount_++;
    InstallScopeFontInner(fd, scope, srcPath, outValue);  // permission + logic
    callingCount_--;
    if (callingCount_ == 0 && GetClientCount() == 0) {
        AddUnloadFontServiceTask();
    }
    return ERR_OK;
}
```

**Permission checks:**
- `CheckPermission()` - `ohos.permission.UPDATE_FONT` (user-level operations)
- `CheckScopeFontPermission()` - `ohos.permission.UPDATE_SCOPE_FONT` (scope font operations)

**App identity (server-side):**
- `GetBundleNameByToken()` - `AccessTokenKit::GetHapTokenInfo(callerToken, tokenInfo)` → `tokenInfo.bundleName`
- `GetCallingUserId()` - `OsAccountManager::GetOsAccountLocalIdFromUid`
- `MakeAppIdentifier(scope, userId, tokenId)` - `"app_<tokenId>"` or `"session_<tokenId>"`

**Lifecycle:**
- `OnStart()`: Create `EventHandler`, register `ClientDiedCallback`, post cleanup task based on reason, `Publish(this)`
- `OnStop()`: Logs only
- Auto-unload: After 10s inactivity, when `callingCount_ == 0`, not migrating, and `GetClientCount() == 0`

### Client (`service/client/`)

#### FontManagerClient (`font_manager_client.h/.cpp`)
- Inherits: `FontManagerKits`, `DelayedSingleton<FontManagerClient>` (`DECLARE_DELAYED_REF_SINGLETON`)
- `DISALLOW_COPY_AND_MOVE(FontManagerClient)`

**FontServiceDeathRecipient** (inner class): Monitors SA death → calls `observer_->OnServiceDied()`.

Methods:
- `int32_t InstallFont/UninstallFont/DataMigration` - User-level operations
- `int32_t InstallFontWithUserId/UninstallFontWithUserId` - With explicit userId
- `int32_t OnFontObserver(const sptr<IFontClientObserver>& observer)` - Remove old death recipient → add new → IPC
- `int32_t OffFontObserver(const sptr<IFontClientObserver>& observer)` - IPC first → then remove death recipient
- `int32_t InstallScopeFont/UninstallScopeFont/GetFontScope` - Scope font operations

#### FontServiceLoadManager (`font_service_load_manager.h/.cpp`)
Singleton. Manages SA loading with `LoadSaStatus` enum (`WAIT_RESULT`, `SUCCESS`, `FAIL`).
- `sptr<IFontService> GetFontServiceAbility(int32_t systemAbilityId)` - Fast path `CheckSystemAbility`, else `LoadSa()` with 5000ms condition variable wait
- `bool UnloadFontService(int32_t systemAbilityId)` - Locks `serviceLock_`, calls `samgr->UnloadSystemAbility`

#### FontClientObserverAgent (`font_client_observer_agent.h/.cpp`)
Inherits `FontClientObserverStub`. Wraps `std::function<void()>` callback for `OnServiceDied`.

#### DataMigrationCbAgent (`data_migration_cb_agent.h/.cpp`)
Inherits `DataMigrationCallbackStub`. Wraps `IDataMigrationListener` for IPC callback.

### Inner API (`service/inner_api/`)

#### FontManagerKits (`font_manager_kits.h/.cpp`)
Abstract base. `DISALLOW_COPY_AND_MOVE`.
- `static FontManagerKits& GetInstance()` - Returns `DelayedRefSingleton<FontManagerClient>::GetInstance()`
- Pure virtual: `InstallFont`, `UninstallFont`, `DataMigration`, `OnFontObserver`, `OffFontObserver`, `InstallScopeFont`, `UninstallScopeFont`, `GetFontScope`

#### FontManagerInnerApi (`font_manager_inner_api.h/.cpp`)
Static methods (inner kits, exposed header):
- `static int32_t InstallFont(const std::string &fontPath, int32_t userId)`
- `static int32_t UninstallFont(const std::string &fontName, int32_t userId)`
- `static int32_t InstallScopeFont(const std::string &fontPath, int32_t scope, int32_t userId)`
- `static int32_t UninstallScopeFont(const std::string &srcPath, int32_t userId)`

## NAPI Bindings (`interfaces/js/kits/`)

### FontManagerAddon (`font_manager_addon.h/.cpp`)
- Init: `FontManagerAddonInit()` binds `installFont`, `uninstallFont`, `dataMigration`, `onFontObserver`, `offFontObserver`, `installScopeFont`, `uninstallScopeFont`, `getFontScope`
- User-level install/uninstall: Async work (`ProcessFontByValue`), Promise + callback
- Scope font install/uninstall/getScope: Async work, Promise + callback
- `OnFontObserver`/`OffFontObserver`: Synchronous, creates `FontClientObserverAgent`

Error message mapping includes all scope font error codes (31100501-31100503).

### JsDataMigrationListener (`js_data_migration_listener.h/.cpp`)
Implements `IDataMigrationListener`. Dispatches events via `napi_send_event` (high priority).

### JsFuncRefHolder (`js_func_ref_holder.h/.cpp`)
Inherits `NoCopyable`. Holds `napi_ref` to JS function. Destructor deletes reference via `napi_send_event` immediate.

## ANI Bindings (`interfaces/ani/`)

### ArkTS API (`ets/@ohos.fontManager.ets`)
- `enum FontScope { app = 0, session = 1 }`
- `interface FontClientObserver { onServiceDied(): void }`
- `function onFontObserver(observer: FontClientObserver): int`
- `function offFontObserver(observer: FontClientObserver): int`
- `function installScopeFont(url: string, scope: FontScope): Promise<int>`
- `function uninstallScopeFont(url: string): Promise<int>`
- `function getFontScope(url: string): Promise<FontScope | null>` (null if not installed)
- User-level: `installFont`, `uninstallFont`, `dataMigration` (wrapped in `taskpool.execute`)

### FontManagerAni (`font_manager_ani.h/.cpp`)
- `OnFontObserver`/`OffFontObserver`: Creates `FontClientObserverAgent` with `AniObserverRef` helper
- `AniObserverRef`: Cross-thread ANI callback via `ani_vm*` + global `ani_ref`, uses `Object_CallMethodByName_Void(obj, "onServiceDied", ":")`
- `InstallScopeFont`/`UninstallScopeFont`/`GetFontScope`: Synchronous native functions

## System Ability Configuration

### SA Profile (`sa_profile/66262.json`)
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

Note: `sa_profile` JSON does NOT support a `permission` field for commonevent entries. The `ohos.permission.INTERACT_ACROSS_LOCAL_ACCOUNTS` required by `USER_STOPPING` is held by the SAMgr/foundation process.

### Service Config (`service/etc/font_manager_server.cfg`)
- **Boot job**: `mkdir /data/service/el1/public/for-all-app/fonts/ 0755 font_manager font_manager`
- **Service**: uid `font_manager`, gid `["font_manager", "shell"]`, ondemand `true`, secon `u:r:font_manager_server:s0`
- **Permissions**: `ohos.permission.MANAGE_LOCAL_ACCOUNTS`, `ohos.permission.STORAGE_MANAGER`, `ohos.permission.INTERACT_ACROSS_LOCAL_ACCOUNTS`

### System Parameters (`service/param/`)
- `persist.fontmanager.updateflag = false` (DAC: `font_manager:font_manager:0775`)
- `const.fontmanager.extstoragebundle` - read by `StorageManagerAdapter` for bundle stats business name

## Permissions

- **API permission check** (server-side):
  - `ohos.permission.UPDATE_FONT` - User-level font install/uninstall, data migration. Verified via `AccessTokenKit::VerifyAccessToken`. Returns `ERR_NO_PERMISSION` (201).
  - `ohos.permission.UPDATE_SCOPE_FONT` - Scope font operations (install/uninstall/observer). Verified via `CheckScopeFontPermission()`.
- **Service process permissions** (`service/etc/font_manager_server.cfg`): `ohos.permission.MANAGE_LOCAL_ACCOUNTS`, `ohos.permission.STORAGE_MANAGER`, `ohos.permission.INTERACT_ACROSS_LOCAL_ACCOUNTS`
- **Test permissions** (`test/common/permission_common`): `ohos.permission.MANAGE_LOCAL_ACCOUNTS`, `ohos.permission.UPDATE_FONT`, APL `system_basic`. Test UIDs: `ROOT_UID=0`, `FONT_MANAGER_UID=1015`.

## Build Configuration

### GN Build Targets

| Target | Path | Description |
|--------|------|-------------|
| `font_service_ability` | `service:font_service_ability` | Service group (server + client + etc + param) |
| `font_manager_server` | `service:font_manager_server` | Server shared lib (sa type) |
| `font_manager_client` | `service:font_manager_client` | Client shared lib (inner_kits) |
| `fontmanager` | `interfaces/js/kits:fontmanager` | NAPI bindings |
| `ani_package_font_manager` | `interfaces/ani:ani_package_font_manager` | ANI bindings |
| `font_server_profile` | `sa_profile:font_server_profile` | SA profile |
| `fontmgr` | `frameworks/fontmgr:fontmgr` | Core library (via `fontmgr.gni`) |

### Component Dependencies (from `bundle.json`)

`ability_base`, `ability_runtime`, `access_token`, `common_event_service`, `bounds_checking_function`, `c_utils`, `cJSON`, `eventhandler`, `hilog`, `hisysevent`, `hitrace`, `init`, `libuv`, `ipc`, `napi`, `node`, `safwk`, `samgr`, `graphic_2d`, `runtime_core`, `os_account`, `storage_service`

Key framework deps (`fontmgr.gni`): `graphic_2d:2d_graphics`, `graphic_2d:rosen_text` (font name extraction), `init:libbegetutil`, `storage_service:storage_manager_sa_proxy`, `common_event_service:cesfwk_innerkits`.

### Inner Kits

Exposed header: `service/inner_api/include/font_manager_inner_api.h` from target `//base/global/font_manager/service:font_manager_client`.

### Symbol Maps
- `service/font_manager_client.map` - Exports `FontManagerKits::GetInstance()`, `FontManagerInnerApi` methods (incl. scope font), `FontServiceLoadManager` symbols (version 1.0)
- `interfaces/ani/fontManager_ani.map` - Exports only `ANI_Constructor`

### Build Features

- Client lib: cflags `-Os` (optimize for size), `branch_protector_ret: "pac_ret"`
- Server lib: sanitize (`boundary_sanitize`, `cfi`, `cfi_cross_dso`, `integer_overflow`, `ubsan`)
- Conditional define `ACCOUNT_ENABLE` when `os_account` part present
- Conditional define `USE_EXTENSION_DATA` when `current_cpu == "arm64"`
- Test define `SUPPORT_GRAPHICS`

## Font Format Handling

- **Supported formats**: TTF (1 fullname) and TTC (multiple fullnames) via `OHOS::Rosen::FontToolSet::GetInstance().GetFontFullName(fd)` from `graphic_2d:rosen_text`
- **Max file size**: 1024 MB (1GB) - enforced in `GetFullNamesByFd` (fstat precheck before parsing) AND `CopyFileByFd` (fstat before sendfile). Double protection.
- **Max install count**: 200 fonts per user (`MAX_INSTALL_NUM`)
- **File copy**: `sendfile()` syscall with EINTR retry loop, `chmod 0644`
- **Duplicate handling (user-level)**: If target file exists, prepend timestamp `YYYYMMDD-HHMMSS_` to filename
- **Duplicate handling (scope)**: Same `srcPath` (URL) → `ERR_INSTALLED_ALRADY`. Same font name → `ERR_INSTALLED_ALRADY`.
- **Config storage**: Per-user `install_fontconfig.json` mapping font paths to full name arrays + scope metadata

## Data Migration

### Migration Process

Migrates fonts from the public install path (`INSTALL_PATH_APP`) to per-user paths (`/data/service/el1/<userId>/for-all-app/fonts/`).

1. `InitDataMigrationEnv()` - Get all OS account user IDs, delete temp dirs, check if `INSTALL_PATH_APP` is empty (returns `ERR_NOT_NEED_DATA_MIGRATION`), init temp dirs
2. `StartHeartBeatTask()` - Detached thread emits `HEART_BEAT` events every 60 seconds (uses `weak_ptr` to detect completion)
3. `StartDataMigration()` - List files in `INSTALL_PATH_APP`, copy each to all user dirs
4. `CheckAndUpdateAllFontRecord()` - For each user, call `FontConfig::CheckAndUpdateFontRecord()` to upgrade config version to "7.0"
5. `ReportFontBundleStats()` per user, `CollectDataMigrationState()`, emit `PROGRESS_RESULT`

### Migration Error Codes (`DataMigrationResultCode`)

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

## Testing

### Test Structure

```
test/
├── common/
│   ├── permission_common.h/.cpp       # PermissionCommon utility
├── fuzztest/
│   ├── utils/fuzz_data.h/.cpp         # NewInt32, NewString helpers
│   ├── serviceinstallfont_fuzzer/     # InstallFont fuzz
│   ├── serviceuninstallfont_fuzzer/   # UninstallFont fuzz
│   ├── servicedatamigration_fuzzer/   # DataMigration fuzz
│   ├── serviceinstallscopefont_fuzzer/ # InstallScopeFont fuzz (fd + scope + srcPath)
│   ├── serviceuninstallscopefont_fuzzer/ # UninstallScopeFont fuzz (srcPath)
│   └── servicegetfontscope_fuzzer/    # GetFontScope fuzz (srcPath)
└── unittest/
    ├── BUILD.gn
    ├── ohos_test.xml                 # Pushes 11 test files to /data/test/
    ├── data/                          # Test font files (LFS-tracked)
    ├── include/
    │   ├── callback_mock.h            # TestCallback (IRemoteStub<IDataMigrationCallback>)
    │   └── hisysevent_adapter_test.h
    └── src/
        ├── font_manager_test.cpp      # FontManagerTest (27 cases: 16 user-level + 11 scope)
        ├── font_config_test.cpp       # FontConfigTest (25 cases: 14 original + 11 scope)
        ├── font_client_registry_test.cpp # FontClientRegistryTest (9 cases)
        ├── data_migration_manager_test.cpp  # DataMigrationManagerTest (6 cases)
        ├── font_manager_inner_api_test.cpp  # FontManagerInnerApiTest
        ├── hisysevent_adapter_test.cpp      # HisyseventAdapterTest
        └── storage_manager_adapter_test.cpp # StorageManagerAdapterTest (6 cases)
```

### Test Targets

- `ohos_unittest("font_manager_module_test")` - Core tests + `fontmgr_src`, define `SUPPORT_GRAPHICS`
- `ohos_unittest("hisysevent_adapter_test")` - HiSysEvent tests
- Group `unittest`: both test targets
- Group `fuzztest`: 6 fuzz targets

### Fuzz Tests

| Target | Function Fuzzed |
|--------|-----------------|
| `ServiceInstallFontFuzzTest` | `service->InstallFont(fd, result)` |
| `ServiceUnInstallFontFuzzTest` | `service->UninstallFont(fontName, result)` |
| `ServiceDataMigrationFuzzTest` | `service->DataMigration(cb)` |
| `ServiceInstallScopeFontFuzzTest` | `service->InstallScopeFont(fd, scope, srcPath, result)` |
| `ServiceUninstallScopeFontFuzzTest` | `service->UninstallScopeFont(srcPath, result)` |
| `ServiceGetFontScopeFuzzTest` | `service->GetFontScope(srcPath, result)` |

Fuzz config: `max_len=1000`, `max_total_time=120s`, `rss_limit_mb=4096`.

### Test Utilities

- `PermissionCommon` - Mock permission setup (`SetFontManagerPermission`, `SetFontManagerInitEnv`, `GrantPermission`, `ResetTokenAndUid`)
- `TestCallback` - `IRemoteStub<IDataMigrationCallback>` mock
- `MockFontClientObserver` - `IRemoteStub<IFontClientObserver>` mock (in `font_client_registry_test.cpp`)
- Test fonts located at `/data/test/` (pushed by `ohos_test.xml` preparer)

### Running Tests

```bash
# Build unit tests
./build.sh --product-name rk3568 --ccache --build-target font_manager_test

# Run tests (device required)
./font_manager_module_test
./hisysevent_adapter_test
```

## Common Patterns

### File Descriptor Handling

```cpp
if (fd < 0) {
    FONT_LOGE("Invalid file descriptor");
    return ERR_INVALID_PARAM;
}
if (fd >= 0) {
    close(fd);
}
```

### Font Name Extraction

```cpp
// Extract full names from font file (TTF: 1 name, TTC: multiple)
std::vector<std::string> fullNames = FontManagerUtils::GetFullNamesByFd(fd);
// 1GB size precheck via fstat before parsing
```

### Configuration File Operations

```cpp
// Thread-safe config access
FontConfig& config = SafeGetOrCreateConfig(userId, configPath);
// User-level: InsertFontRecord/DeleteFontRecord
// Scope: InsertScopeFontRecord/DeleteScopeFontRecordByUrl
```

## Code Review Checklist

- [ ] Copyright header present
- [ ] Proper include guards in headers
- [ ] Namespace hierarchy correct (`OHOS::Global::FontManager`)
- [ ] Error handling with appropriate error codes
- [ ] Logging with correct log level and format specifiers
- [ ] Thread safety where needed (mutex protection, atomic flags)
- [ ] No recursive mutex locking (internal helpers must not lock same mutex as caller)
- [ ] Resource cleanup (file descriptors, memory, napi_ref, ani_ref)
- [ ] Tests added for new functionality
- [ ] Follow naming conventions consistently
- [ ] Permission check present for IPC methods (`UPDATE_FONT` for user-level, `UPDATE_SCOPE_FONT` for scope)
- [ ] SA unload task scheduling for server methods (Outer/Inner pattern)
- [ ] Death recipient cleanup (no accumulation in OnFontObserver)
- [ ] OffFontObserver: IPC before cleanup (stub survives during server processing)
