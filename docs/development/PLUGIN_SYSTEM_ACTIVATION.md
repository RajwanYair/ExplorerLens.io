# Plugin System Activation Guide
**Sprint 11 - February 17, 2026**

## Overview

The ExplorerLens plugin system allows third-party developers to create custom thumbnail decoders for formats not natively supported by the engine. As of Sprint 11, the plugin system is **fully activated** and ready for use.

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    ThumbnailPipeline                         │
│  (Main orchestration - decides which decoder to use)         │
└────────────────┬────────────────────────────────────────────┘
                 │
                 ├──> Built-in Decoders (WebP, AVIF, RAW, etc.)
                 │
                 └──> PluginManager (Sprint 11 activated)
                       │
                       ├──> Plugin Discovery
                       │    - Scans %LocalAppData%\ExplorerLens\Plugins\
                       │    - Scans %ProgramData%\ExplorerLens\Plugins\
                       │    - Scans %AppData%\ExplorerLens\Plugins\
                       │
                       ├──> Plugin Loading
                       │    - Validates manifest.json
                       │    - Loads DLL (HMODULE)
                       │    - Verifies API version compatibility
                       │
                       ├──> Plugin Execution
                       │    - In-process (direct DLL call)
                       │    - OR via PluginHost.exe (isolated process via IPC)
                       │    - Automatic crash isolation
                       │
                       └──> Plugin Lifecycle
                            - Initialize() on load
                            - Decode() for thumbnails
                            - Shutdown() on unload
```

## Feature Flag

Plugin loading is controlled by the `PipelineConfig.enablePlugins` flag:

```cpp
// Enable plugins (default)
PipelineConfig config;
config.enablePlugins = true;  // Sprint 11: plugins activated
pipeline.Initialize(config);

// Disable plugins (for testing or security)
config.enablePlugins = false;
pipeline.Initialize(config);
```

## Configuration

### Registry Settings

Plugins can be enabled/disabled globally via registry:

```
HKEY_CURRENT_USER\Software\ExplorerLens\PluginsEnabled (DWORD)
  0 = Disabled
  1 = Enabled (default)
```

### Plugin Directories

Plugins are discovered from (in order):
1. `%LocalAppData%\ExplorerLens\Plugins\` (user-specific)
2. `%ProgramData%\ExplorerLens\Plugins\` (system-wide)
3. `%AppData%\ExplorerLens\Plugins\` (roaming profile)

Each plugin must be in its own subdirectory with:
- `plugin_name.dll` - Plugin binary
- `manifest.json` - Plugin metadata

## Plugin Development

### Sample Plugin

A minimal working plugin is provided in `SDK/examples/minimal-plugin/`:

```cpp
// minimal_plugin.cpp
#include "ExplorerLensPlugin.h"

extern "C" {
    __declspec(dllexport) const PluginInfo* plugin_get_info() {
        static const char* extensions[] = { ".xyz", nullptr };
        static PluginInfo info = {
            .api_version = PLUGIN_API_VERSION,
            .plugin_name = "Minimal Example",
            .plugin_version = "1.0.0",
            .supported_extensions = extensions
        };
        return &info;
    }

    __declspec(dllexport) PluginErrorCode plugin_init(const PluginAllocator* alloc) {
        // Initialize your decoder library
        return PLUGIN_SUCCESS;
    }

    __declspec(dllexport) PluginErrorCode plugin_decode(
        const DecodeRequest* request,
        DecodeResult* result,
        PluginProgressCallback progress
    ) {
        // Generate thumbnail
        // ... decode logic ...
        return PLUGIN_SUCCESS;
    }

    __declspec(dllexport) void plugin_cleanup() {
        // Cleanup resources
    }
}
```

### Building a Plugin

1. **Create CMake Project:**
```cmake
cmake_minimum_required(VERSION 3.20)
project(my_plugin)

add_library(my_plugin SHARED plugin.cpp)
target_include_directories(my_plugin PRIVATE ${ExplorerLens_SDK_INCLUDE})
target_link_libraries(my_plugin ExplorerLensPluginSDK)
```

2. **Build:**
```powershell
mkdir build
cd build
cmake .. -G "Visual Studio 18 2026" -A x64
cmake --build . --config Release
```

3. **Deploy:**
```powershell
# Copy to user plugin directory
$pluginDir = "$env:LOCALAPPDATA\ExplorerLens\Plugins\my_plugin"
New-Item -ItemType Directory -Path $pluginDir -Force
Copy-Item Release\my_plugin.dll $pluginDir\
Copy-Item manifest.json $pluginDir\
```

### manifest.json Format

```json
{
  "name": "My Plugin",
  "version": "1.0.0",
  "author": "Your Name",
  "description": "Adds support for XYZ format",
  "api_version": 1,
  "supported_extensions": [".xyz", ".abc"],
  "mime_types": ["image/xyz"],
  "requires_gpu": false,
  "license": "MIT"
}
```

## Testing

### Test Plugin System

```powershell
# Verify plugin infrastructure
.\tests\Test-PluginSystem.ps1

# Build and test sample plugin
.\tests\Test-PluginSystem.ps1 -BuildSamplePlugin
```

### Manual Testing

1. **Enable Debug Output:**
   - Set `HKCU\Software\ExplorerLens\DebugMode` = 1
   - Use DebugView to see plugin loading messages

2. **Test Plugin Loading:**
   ```cpp
   ThumbnailPipeline pipeline;
   PipelineConfig config;
   config.enablePlugins = true;
   pipeline.Initialize(config);
   
   // Check logs for:
   // [Pipeline] Scanning for plugins...
   // [Pipeline] Loaded N plugin(s) from <path>
   // [Pipeline] Registering plugin decoder: <name> (v<version>)
   ```

3. **Test Thumbnail Generation:**
   ```cpp
   ThumbnailRequest req;
   req.filePath = L"C:\\test\\sample.xyz";  // Your plugin's format
   req.width = 256;
   req.height = 256;
   
   ThumbnailResult result = pipeline.GenerateThumbnail(req);
   if (SUCCEEDED(result.status)) {
       // Success! Plugin decoded the file
   }
   ```

## LENSManager UI Integration

The plugin management UI in LENSManager allows users to:
- **View Loaded Plugins:** See all discovered plugins
- **Enable/Disable Plugins:** Toggle individual plugins
- **View Plugin Details:** Author, version, supported formats
- **Refresh Plugin List:** Rescan

To access:
1. Open LENSManager.exe
2. Navigate to "Plugins" tab
3. View/manage plugins

## Troubleshooting

### Plugin Not Loading

**Check:**
1. DLL is in correct directory (`%LOCALAPPDATA%\ExplorerLens\Plugins\<plugin_name>\`)
2. `manifest.json` exists alongside DLL
3. API version matches (check `PLUGIN_API_VERSION` in SDK)
4. No missing dependencies (use Dependency Walker)
5. DebugView shows loading messages

### Plugin Crashes

Plugins run in-process by default. To enable crash isolation:

```cpp
// Future Sprint: IPC isolation via PluginHost.exe
config.enablePluginIsolation = true;  // Not yet implemented
```

For now, crashes will terminate Explorer. Test plugins thoroughly!

### Performance

**Expected Overhead:**
- Plugin discovery: ~10-50ms (one-time on startup)
- Plugin loading: ~5-20ms per plugin (one-time)
- Decode call: depends on plugin implementation

**Optimization Tips:**
- Lazy-load heavy libraries in `plugin_init()`
- Cache decoded data when possible
- Implement `plugin_can_decode()` efficiently

## Security Considerations

**Current Status (Sprint 11):**
- ✅ Plugins load from user-controlled directories only
- ✅ API version verification prevents ABI mismatches
- ❌ No code signing verification
- ❌ No sandbox isolation (runs in-process)
- ❌ No capability restrictions

**Future Work (Sprints 17-20):**
- Code signing requirement for system-wide plugins
- Process isolation via PluginHost.exe (IPC)
- AppContainer sandbox
- Capability-based security model

## API Reference

See `SDK/docs/plugin-api-reference.md` for complete API documentation.

### Core Functions

| Function | Description |
|----------|-------------|
| `plugin_get_info()` | Returns plugin metadata (name, version, formats) |
| `plugin_init()` | Initialize plugin (called once on load) |
| `plugin_cleanup()` | Cleanup resources (called on shutdown) |
| `plugin_can_decode()` | Check if plugin can decode a file |
| `plugin_decode()` | Generate thumbnail from file |
| `plugin_get_metadata()` | Extract metadata (optional) |

### Error Codes

| Code | Meaning |
|------|---------|
| `PLUGIN_SUCCESS` | Operation succeeded |
| `PLUGIN_ERROR_UNSUPPORTED_FORMAT` | File format not supported |
| `PLUGIN_ERROR_OUT_OF_MEMORY` | Memory allocation failed |
| `PLUGIN_ERROR_DECODE_FAILED` | Decoding failed (corrupt file, etc.) |
| `PLUGIN_ERROR_TIMEOUT` | Operation timed out |

## Sprint 11 Deliverables

✅ **Completed:**
1. Added `enablePlugins` feature flag to `PipelineConfig`
2. Plugin loading gated by configuration
3. Plugin discovery from standard directories
4. Sample plugin verified and documented
5. Test script created (`Test-PluginSystem.ps1`)
6. Documentation complete

⚠ **Deferred** (future sprints):
- PluginHost.exe IPC isolation (Sprint 17)
- Plugin UI in LENSManager (Sprint 19: WinUI 3 migration)
- Code signing verification (Sprint 16)
- Sandbox security (Sprint 20)

## Next Steps

**For Plugin Developers:**
1. Read `SDK/docs/plugin-development-guide.md`
2. Study sample plugin: `SDK/examples/minimal-plugin/`
3. Build your plugin following the template
4. Test with ExplorerLens v7.0+

**For Users:**
1. Download plugins from trusted sources
2. Extract to `%LOCALAPPDATA%\ExplorerLens\Plugins\`
3. Restart Explorer to activate
4. Check plugin status in LENSManager

**For Contributors:**
- See `MASTER_PLAN.md` Sprint 17 for IPC isolation work
- See Sprint 19 for plugin UI implementation

---

**Plugin System Status:** ✅ **ACTIVE** (Sprint 11 complete)  
**IPC Isolation:** ⚠ Planned (Sprint 17)  
**UI Management:** ⚠ Planned (Sprint 19)

