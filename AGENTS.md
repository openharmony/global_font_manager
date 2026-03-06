# AGENTS.md - Font Manager Codebase Guide

This file provides guidelines for agentic coding agents working in the font_manager repository.

## Build System

This project uses the GN (Generate Ninja) build system.

### Build Commands

```bash
# Build the entire font manager service
# need 'cd ~/openharmony' first
./build.sh --product-name rk3568 --ccache --build-target font_manager

# Build all tests
./build.sh --product-name rk3568 --ccache --build-target font_manager_test

```

## Code Style Guidelines

### File Structure and Organization

- **Copyright Header**: All source files must begin with Apache 2.0 copyright header
- **Include Guards**: Use `#ifndef` style guards with `GLOBAL_FONT_MANAGER_<FILENAME>_H` format
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

- **Classes**: PascalCaseCase (e.g., `FontManager`, `FontConfig`, `FontManagerClient`)
- **Functions**: PascalCase for public methods (e.g., `InstallFont`, `UninstallFont`)
- **Variables**: camelCase (e.g., `installPath`, `fontFullName`, `userId`)
- **Constants**: UPPER_SNAKE_CASE or constexpr (e.g., `MAX_INSTALL_NUM`, `FONT_SA_ID`)
- **Member Variables**: camelCase with trailing underscore for private members (e.g., `configMap_`, `mapLock_`)
- **Enums**: PascalCase with UPPER_SNAKE_CASE values (e.g., `FontErrorCode`, `ERR_OK`)

### Type and Formatting

- **Pointers**: Use `sptr<T>` for IPC objects, `std::shared_ptr<T>` for regular smart pointers
- **Constants**: Use `constexpr` for compile-time constants, `inline const` for header constants
- **String Type**: Use `std::string` consistently
- **Integer Types**: Use `int32_t` for file descriptors and user IDs, `int` for general integers
- **Error Codes**: Return `int32_t` or `ErrCode` (typedef for int)
- **Boolean**: Use `bool` type with `true`/`false` literals

### Error Handling

```cpp
// Error codes are defined in font_define.h as enum FontErrorCode
// Common pattern: Check condition, log error, return error code
if (condition) {
    FONT_LOGE("Error message with params: %{public}s", value.c_str());
    return ERR_ERROR_CODE;
}

// Always check for null pointers
if (service == nullptr) {
    FONT_LOGE("Service is null");
    return ERR_SYSTEM_ERROR;
}
```

### Logging

Use custom logging macros defined in `font_hilog.h`:
- `FONT_LOGD(...)` - Debug level
- `FONT_LOGI(...)` - Info level
- `FONT_LOGW(...)` - Warning level
- `FONT_LOGE(...)` - Error level

Log format specifiers:
- `%{public}s` - Public string (not sanitized)
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

### Testing Guidelines

- Use `HWTEST_F` macro for test cases
- Test class naming: `<ClassName>Test` (e.g., `FontManagerTest`)
- Test case naming: `<ClassName>FuncTest<Number>` (e.g., `FontManagerFuncTest001`)
- Test size levels: `TestSize.Level1` (Level1 = critical, Level2 = important, Level3 = normal)
- Use `#define private public` and `#undef private` to test private members when necessary

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

### Key Files and Components

- `frameworks/fontmgr/` - Core font management logic
- `service/client/` - Client-side IPC implementation
- `service/server/` - Server-side IPC implementation
- `interfaces/js/kits/` - NAPI bindings for JavaScript/ArkTS
- `interfaces/ani/` - ANI bindings for ArkTS
- `common/include/` - Shared headers and definitions
- `test/unittest/` - Unit tests
- `test/fuzztest/` - Fuzz tests

### Dependencies

Key external dependencies (from bundle.json):
- `hilog` - Logging framework
- `hisysevent` - System event framework
- `ipc` - IPC communication
- `safwk` - System ability framework
- `napi` - Native API for JavaScript
- `graphic_2d` - Graphics and text rendering
- `cJSON` - JSON parsing
- `c_utils` - Common utilities

### Important Constants

- `FONT_SA_ID = 66262` - System font manager service ID
- `MAX_INSTALL_NUM = 200` - Maximum number of installable fonts
- Font install path: `/data/service/el1/<userId>/for-all-app/fonts/`
- Config file: `install_fontconfig.json`

### Code Review Checklist

- [ ] Copyright header present
- [ ] Proper include guards in headers
- [ ] Namespace hierarchy correct (OHOS::Global::FontManager)
- [ ] Error handling with appropriate error codes
- [ ] Logging with correct log level and format specifiers
- [ ] Thread safety where needed (mutex protection)
- [ ] Resource cleanup (file descriptors, memory)
- [ ] Tests added for new functionality
- [ ] Follow naming conventions consistently

## API Documentation

### Core APIs

**FontManager (Server-side)**
- `int32_t InstallFont(const int32_t &fd, const int32_t userId)` - Install font from file descriptor
- `int32_t UninstallFont(const std::string &fontFullName, const int32_t userId)` - Uninstall font by full name

**FontManagerClient (Client-side)**
- `int32_t InstallFont(const std::string &fontPath, int &outValue)` - Install font from path
- `int32_t UninstallFont(const std::string &fontName, int &outValue)` - Uninstall font by name
- `int32_t DataMigration(std::shared_ptr<IDataMigrationListener> listener)` - Trigger data migration

**FontConfig**
- `bool InsertFontRecord(const std::string &fontPath, const std::vector<std::string> &fullNames)` - Add font record to config
- `bool DeleteFontRecord(const std::string &fontPath)` - Remove font record from config
- `int GetInstalledFontsNum()` - Get count of installed fonts
- `std::string GetFontFileByName(const std::string &fullName)` - Get font file path by name

**FontManagerUtils (Static utilities)**
- `static bool CheckAndInitInstallPath(const std::string &installPath)` - Initialize install directory
- `static std::vector<std::string> GetFullNamesByFd(const int32_t &fd)` - Extract font names from fd
- `static std::vector<std::string> GetFullNamesByPath(const std::string &path)` - Extract font names from path

## IPC Architecture

### IPC Interface Hierarchy

```
FontServiceStub (IPC interface)
    ↑
FontManagerServer (SystemAbility implementation)
    ↓
FontManager (Core business logic)
    ↓
FontConfig (Configuration management)
```

### IPC Communication Flow

1. **Client Request**: FontManagerClient receives ArkTS/ANI call
2. **Parameter Validation**: Client validates path and converts to fd
3. **IPC Call**: Client calls server via FontServiceProxy
4. **Permission Check**: Server verifies system app permission
5. **Core Processing**: FontManager handles install/uninstall
6. **Response**: Result returned through IPC callback

## Data Migration

### Migration Process

The font manager supports data migration between user directories:
- Migrates fonts from old install path to new path
- Uses `DataMigrationManager` for orchestration
- Supports callback via `IDataMigrationListener`
- Prevents concurrent operations with `isDataMigrationing_` flag

### Migration Error Codes

```cpp
enum DataMigrationResultCode {
    ERR_NOT_NEED_DATA_MIGRATION = 1,
    ERR_GET_ALL_USERIDS,
    ERR_CHECK_INSTALL_DIR,
    ERR_INIT_TEMP_DIR,
    ERR_OPEN_SRC_FILE,
    ERR_COPY_FILE,
    ERR_RENAME_FILE,
    ERR_REMOVE_SRC_FILE,
};
```

## Common Patterns

### File Descriptor Handling

```cpp
// Always check fd validity
if (fd < 0) {
    FONT_LOGE("Invalid file descriptor");
    return ERR_INVALID_PARAM;
}

// Close fd after use
if (fd >= 0) {
    close(fd);
}
```

### Path Operations

```cpp
// Get filename from path
std::string fileName = FontManagerUtils::GetFileName(fontPath);

// Get directory from path
std::string dir = FontManagerUtils::GetFileDirectory(fontPath);

// Check if path exists
if (!FontManagerUtils::CheckPathExist(path)) {
    FONT_LOGE("Path does not exist: %{public}s", path.c_str());
    return ERR_FILE_NOT_EXISTS;
}
```

### Font Name Extraction

```cpp
// Extract full names from font file (supports TTF and TTC)
std::vector<std::string> fullNames = FontManagerUtils::GetFullNamesByFd(fd);

// Format full name for storage
std::string formattedName = GetFormatFullName(fullNames);
```

### Configuration File Operations

```cpp
// Thread-safe config access
FontConfig& config = SafeGetOrCreateConfig(userId, configPath);

// Insert font record
config.InsertFontRecord(fontPath, fullNames);

// Delete font record
config.DeleteFontRecord(fontPath);
```

## Build Configuration

### GN Build Targets

- `//base/global/font_manager/frameworks/fontmgr:fontmgr` - Core library
- `//base/global/font_manager/service:font_service_ability` - Service component
- `//base/global/font_manager/interfaces/js/kits:fontmanager` - NAPI bindings
- `//base/global/font_manager/interfaces/ani:ani_package_font_manager` - ANI bindings
- `//base/global/font_manager/sa_profile:font_server_profile` - SystemAbility profile

### Component Dependencies

Core dependencies from `bundle.json`:
- `hilog` - Logging
- `hisysevent` - System events
- `ipc` - IPC framework
- `safwk` - System Ability framework
- `napi` - JavaScript native API
- `graphic_2d` - Graphics and font rendering
- `cJSON` - JSON parsing
- `c_utils` - Common utilities
- `access_token` - Permission management
- `os_account` - User account management

## Testing

### Test Structure

```
test/unittest/
├── include/           # Test headers
├── src/              # Test implementations
│   ├── font_manager_test.cpp
│   ├── font_config_test.cpp
│   ├── data_migration_manager_test.cpp
│   └── hisysesevent_adapter_test.cpp
└── BUILD.gn

test/fuzztest/
├── serviceinstallfont_fuzzer/
├── serviceuninstallfont_fuzzer/
└── servicedatamigration_fuzzer/
```

### Test Utilities

- `PermissionCommon` - Mock permission setup
- `CallbackMock` - Mock callback implementations
- Test fonts located at `/data/test/`

### Running Tests

```bash
# Build unit tests
./build.sh --product-name rk3568 --ccache --build-target font_manager_test

# Run tests (device required)
./font_manager_test
```

## Event Publishing

### HiSysEvent Integration

The font manager publishes system events via `HiSysEventAdapter`:
- Font installation events
- Font uninstallation events
- Error events

Event format:
```cpp
HiSysEvent::Write(HiSysEvent::Domain::GLOBAL_FONT_MANAGER,
    eventName, HiSysEvent::EventType::BEHAVIOR, params);
```

## System Ability Configuration

### SA Profile

- **SA ID**: `FONT_SA_ID = 66262`
- **Profile file**: `sa_profile/font_manager_sa_profile.xml`
- **SystemCapability**: `SystemCapability.Global.FontManager`
- **On-demand**: Supports on-demand loading/unloading

### Service Lifecycle

- **OnStart**: Initialize service, register with SAMgr
- **OnStop**: Cleanup resources
- **Unload Task**: Auto-unload after inactivity timeout
