# ExplorerLens Plugin SDK

The ExplorerLens Plugin SDK enables developers to extend ExplorerLens with custom thumbnail decoders for new file formats.

## 🚀 Quick Start

### 1. Include the API Header

```cpp
#include "SDK/plugin_api.h"
```

### 2. Implement Required Functions

```cpp
// Get plugin information
PLUGIN_API const PluginInfo* PLUGIN_CALL plugin_get_info(void) {
    static const char* extensions[] = { ".myformat", NULL };
    static const char* mime_types[] = { "image/myformat", NULL };
    
    static PluginInfo info = {
        .plugin_name = "My Format Decoder",
        .plugin_version = "1.0.0",
        .plugin_author = "Your Name",
        .plugin_description = "Decodes .myformat files",
        .plugin_license = "MIT",
        .api_version = EXPLORERLENS_PLUGIN_API_VERSION,
        .supported_extensions = extensions,
        .mime_types = mime_types,
        .capabilities = PLUGIN_CAP_STILL_IMAGE,
        .max_threads = 0,
        .requires_gpu = false,
        .supports_background_loading = true
    };
    return &info;
}

// Initialize plugin
PLUGIN_API PluginErrorCode PLUGIN_CALL plugin_init(const PluginAllocator* allocator) {
    // Store allocator, initialize resources
    return PLUGIN_SUCCESS;
}

// Cleanup
PLUGIN_API void PLUGIN_CALL plugin_cleanup(void) {
    // Free resources
}

// Check if file can be decoded
PLUGIN_API bool PLUGIN_CALL plugin_can_decode(const char* file_path,
                                                const uint8_t* data,
                                                size_t data_size) {
    // Check file header/magic bytes
    return true;
}

// Decode to thumbnail
PLUGIN_API PluginErrorCode PLUGIN_CALL plugin_decode(const DecodeRequest* request,
                                                       DecodeResult* result,
                                                       PluginProgressCallback progress) {
    // Decode image and populate result
    return PLUGIN_SUCCESS;
}

// Free decode result
PLUGIN_API void PLUGIN_CALL plugin_free_result(DecodeResult* result) {
    // Free allocated buffers
}
```

### 3. Build as DLL

**CMakeLists.txt:**
```cmake
cmake_minimum_required(VERSION 3.20)
project(MyFormatPlugin)

add_library(myformat-plugin SHARED plugin.cpp)

target_compile_definitions(myformat-plugin PRIVATE
    EXPLORERLENS_PLUGIN_EXPORTS
)

target_include_directories(myformat-plugin PRIVATE
    ${EXPLORERLENS_SDK_DIR}
)
```

**Build:**
```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

### 4. Test Your Plugin

```bash
plugin-tester.exe myformat-plugin.dll --info
plugin-tester.exe myformat-plugin.dll --test sample.myformat
```

## 📚 Documentation

- **[Plugin SDK Guide](docs/PLUGIN_SDK.md)** - Complete developer guide
- **[API Reference](plugin_api.h)** - Full API documentation
- **[Examples](examples/)** - Sample plugin implementations
  - [minimal-plugin](examples/minimal-plugin/) - Minimal template
  - [SamplePlugin](examples/SamplePlugin/) - Full example

## 🔧 Tools

- **plugin-tester.exe** - Test and validate plugins
  ```bash
  plugin-tester.exe <plugin.dll> [--info] [--test <file>] [--size WxH]
  ```

## 📦 Plugin Package Format (.dtplugin)

```
myplugin.dtplugin/
├── plugin.dll         # Your plugin DLL
├── manifest.json      # Metadata
├── icon.png          # Plugin icon (optional)
└── LICENSE.txt       # License file
```

**manifest.json:**
```json
{
  "name": "My Format Decoder",
  "version": "1.0.0",
  "author": "Your Name",
  "description": "Decodes .myformat files",
  "license": "MIT",
  "api_version": 65536,
  "supported_extensions": [".myformat"],
  "mime_types": ["image/myformat"],
  "requires_gpu": false
}
```

## 🎯 API Stability

The ExplorerLens Plugin API uses a stable C ABI that ensures binary compatibility:

- **Major version** must match between plugin and host
- **Minor version** changes are backward compatible
- Version format: `(major << 16) | minor`

Current API Version: **1.0** (65536)

## 💡 Best Practices

### Memory Management

✅ **DO:**
- Use the host-provided allocator for all persistent allocations
- Free all resources in `plugin_free_result()`
- Handle NULL pointers gracefully

❌ **DON'T:**
- Use `malloc`/`free` directly for plugin-host data exchange
- Leak memory
- Access freed memory

### Threading

- Plugins should be thread-safe
- Use `max_threads` to hint optimal thread count
- Don't create threads unless necessary

### Error Handling

- Return appropriate error codes
- Set `result->error_message` for diagnostics
- Validate all input parameters

### Performance

- Implement `plugin_get_thumbnail()` for embedded thumbnails (fast path)
- Use `plugin_get_metadata()` for metadata-only queries
- Report progress for long operations (>1 second)

## 🔒 Security

Plugins run in the same process as ExplorerLens, so:

- Validate all file input thoroughly
- Be defensive against malformed/malicious files
- Avoid buffer overflows
- Handle errors gracefully

**Future:** Plugin sandboxing via AppContainer (Sprint 14)

## 📖 Examples

### Minimal Plugin Template

See [examples/minimal-plugin/](examples/minimal-plugin/) for a complete working template that you can customize.

### Sample Plugin

See [examples/SamplePlugin/](examples/SamplePlugin/) for a full-featured example with comments.

## 🛠️ Development Workflow

1. **Create plugin** using template
2. **Build** with CMake or Visual Studio
3. **Test** with plugin-tester.exe
4. **Package** as .dtplugin
5. **Distribute** to users

## 📝 Requirements

- **ExplorerLens:** 5.4.0+
- **Compiler:** MSVC 2022+ / GCC 11+ / Clang 14+
- **C++ Standard:** C++20 or C11
- **Windows SDK:** 10.0.26100.0+

## 🤝 Contributing

We welcome plugin contributions! To share your plugin:

1. Test thoroughly with plugin-tester.exe
2. Add MIT license (or compatible)
3. Create PR to plugin gallery
4. Include sample files for testing

## 📮 Support

- **Issues:** [GitHub Issues](https://github.com/yourorg/explorerlens/issues)
- **Discussions:** [GitHub Discussions](https://github.com/yourorg/explorerlens/discussions)
- **Email:** support@explorerlens.example.com

## 📄 License

The Plugin SDK and API headers are licensed under MIT License.

Your plugins can use any license compatible with your use case.

---

**Happy Plugin Development! 🎉**


