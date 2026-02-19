# DarkThumbs Plugin SDK Documentation

## Table of Contents

1. [Introduction](#introduction)
2. [Getting Started](#getting-started)
3. [Plugin Architecture](#plugin-architecture)
4. [API Reference](#api-reference)
5. [Creating Your First Plugin](#creating-your-first-plugin)
6. [Best Practices](#best-practices)
7. [Examples](#examples)
8. [Testing](#testing)
9. [Distribution](#distribution)

---

## Introduction

The DarkThumbs Plugin SDK enables developers to extend DarkThumbs with custom thumbnail decoders for new file formats. Plugins use a stable C ABI that ensures binary compatibility across DarkThumbs versions.

### Key Features

- **Stable C ABI**: Binary compatibility across minor version updates
- **Performance**: Direct memory access, zero-copy decoding, GPU acceleration support
- **Security**: Sandboxed execution, memory limits, crash isolation
- **Easy Distribution**: `.dtplugin` package format with automatic discovery
- **Cross-Platform**: Windows (primary), Linux, macOS support

### System Requirements

- **DarkThumbs**: Version 8.4.0 or later
- **Compiler**: MSVC 2026+ v145 toolset (Windows), GCC 11+ (Linux), Clang 14+ (macOS)
- **C++ Standard**: C++20 or C11
- **Windows SDK**: 10.0.26100.0 or later
- **GPU**: DirectX 11 or DirectX 12 capable GPU (for GPU-accelerated plugins)

---

## Getting Started

### SDK Structure

```
SDK/
├── plugin_api.h              # Main API header
├── docs/                     # Documentation
│   ├── PLUGIN_SDK.md        # This file
│   ├── API_REFERENCE.md      # Complete API reference
│   └── EXAMPLES.md           # Example code
├── examples/                 # Sample plugins
│   ├── webp-plugin/         # WebP decoder example
│   ├── custom-format/       # Custom format example
│   └── minimal-plugin/      # Minimal plugin template
└── tools/                    # Build and test tools
    ├── plugin-tester.exe    # Plugin validation tool
    └── cmake/                # CMake templates
```

### Quick Start

1. **Include the API header**:
   ```cpp
   #include "SDK/plugin_api.h"
   ```

2. **Implement required functions**:
   - `plugin_get_info()` - Return plugin metadata
   - `plugin_init()` - Initialize plugin
   - `plugin_cleanup()` - Cleanup resources
   - `plugin_can_decode()` - Check if file is supported
   - `plugin_decode()` - Decode thumbnail
   - `plugin_free_result()` - Free decode result

3. **Build as DLL**:
   ```cmake
   add_library(myplugin SHARED myplugin.cpp)
   target_link_libraries(myplugin PRIVATE DarkThumbs::SDK)
   ```

4. **Test the plugin**:
   ```bash
   plugin-tester.exe myplugin.dll sample_image.ext
   ```

5. **Package and distribute**:
   ```
   myplugin.dtplugin/
   ├── plugin.dll
   ├── manifest.json
   ├── icon.png
   └── LICENSE.txt
   ```

---

## Plugin Architecture

### Plugin Lifecycle

```
┌─────────────────────────────────────────────────┐
│ Host Application (DarkThumbs)                   │
├─────────────────────────────────────────────────┤
│ 1. Plugin Discovery                             │
│    - Scan plugin directory                      │
│    - Load plugin DLL                            │
│    - Call plugin_get_info()                     │
│                                                  │
│ 2. Plugin Initialization                        │
│    - Validate API version                       │
│    - Call plugin_init()                         │
│    - Register supported formats                 │
│                                                  │
│ 3. Decode Operations (per request)              │
│    - Call plugin_can_decode() to check support  │
│    - Call plugin_decode() to generate thumbnail │
│    - Display result                             │
│    - Call plugin_free_result()                  │
│                                                  │
│ 4. Plugin Shutdown                              │
│    - Call plugin_cleanup()                      │
│    - Unload plugin DLL                          │
└─────────────────────────────────────────────────┘
```

### Memory Management

Plugins use a **host-provided allocator** for all memory allocations:

```c
// Host provides allocator during initialization
PluginAllocator allocator = {
    .alloc = my_alloc_func,
    .free = my_free_func,
    .user_data = context
};
plugin_init(&allocator);

// Plugin uses allocator for all allocations
void* buffer = allocator.alloc(size, allocator.user_data);
// ... use buffer ...
allocator.free(buffer, allocator.user_data);
```

**Rules:**
- ✅ **DO**: Use host allocator for all persistent allocations
- ✅ **DO**: Free all allocations in `plugin_free_result()`
- ❌ **DON'T**: Use `malloc`/`free` directly
- ❌ **DON'T**: Leak memory

### Threading Model

- **Thread-Safe**: All plugin functions **must be thread-safe**
- **Concurrent Decoding**: Multiple threads may call `plugin_decode()` simultaneously
- **Internal Threading**: Plugins may use threads internally (set `max_threads` in info)
- **GPU Acceleration**: Plugins may use GPU for decoding (set `requires_gpu` in info)

---

## API Reference

### Core Functions

#### `plugin_get_info()`

Returns plugin metadata.

```c
PLUGIN_API const PluginInfo* PLUGIN_CALL plugin_get_info(void);
```

**Returns**: Pointer to static `PluginInfo` structure (must remain valid for plugin lifetime)

**Example**:
```c
static const char* supported_extensions[] = {".webp", ".webm", NULL};
static const char* mime_types[] = {"image/webp", NULL};

static const PluginInfo info = {
    .plugin_name = "WebP Decoder",
    .plugin_version = "1.0.0",
    .plugin_author = "Your Name",
    .plugin_description = "WebP image decoder with animation support",
    .plugin_license = "MIT",
    .api_version = DARKTHUMBS_PLUGIN_API_VERSION,
    .supported_extensions = supported_extensions,
    .mime_types = mime_types,
    .capabilities = PLUGIN_CAP_STILL_IMAGE | PLUGIN_CAP_ANIMATION | PLUGIN_CAP_ALPHA,
    .max_threads = 0,  // Unlimited
    .requires_gpu = false,
    .supports_background_loading = true,
};

PLUGIN_API const PluginInfo* PLUGIN_CALL plugin_get_info(void) {
    return &info;
}
```

#### `plugin_init()`

Initialize plugin with host-provided memory allocator.

```c
PLUGIN_API PluginErrorCode PLUGIN_CALL plugin_init(const PluginAllocator* allocator);
```

**Parameters**:
- `allocator`: Host-provided memory allocator (store for later use)

**Returns**: `PLUGIN_SUCCESS` or error code

**Example**:
```c
static PluginAllocator g_allocator;

PLUGIN_API PluginErrorCode PLUGIN_CALL plugin_init(const PluginAllocator* allocator) {
    if (!allocator) return PLUGIN_ERROR_INVALID_PARAMETER;
    
    g_allocator = *allocator;  // Store allocator
    
    // Initialize your decoder library
    // ...
    
    return PLUGIN_SUCCESS;
}
```

#### `plugin_cleanup()`

Cleanup and shutdown plugin.

```c
PLUGIN_API void PLUGIN_CALL plugin_cleanup(void);
```

**Example**:
```c
PLUGIN_API void PLUGIN_CALL plugin_cleanup(void) {
    // Free global resources
    // Shutdown decoder library
    // ...
}
```

#### `plugin_can_decode()`

Check if plugin can decode the given file/data.

```c
PLUGIN_API bool PLUGIN_CALL plugin_can_decode(const char* file_path, 
                                                const uint8_t* data, 
                                                size_t data_size);
```

**Parameters**:
- `file_path`: File path (UTF-8) or `NULL` if using `data`
- `data`: Raw file data or `NULL` if using `file_path`
- `data_size`: Size of `data` in bytes

**Returns**: `true` if plugin can decode, `false` otherwise

**Example**:
```c
PLUGIN_API bool PLUGIN_CALL plugin_can_decode(const char* file_path, 
                                                const uint8_t* data, 
                                                size_t data_size) {
    // Check file extension
    if (file_path) {
        const char* ext = strrchr(file_path, '.');
        if (ext && (strcmp(ext, ".webp") == 0 || strcmp(ext, ".webm") == 0)) {
            return true;
        }
    }
    
    // Check magic bytes (WebP signature)
    if (data && data_size >= 12) {
        return memcmp(data, "RIFF", 4) == 0 && 
               memcmp(data + 8, "WEBP", 4) == 0;
    }
    
    return false;
}
```

#### `plugin_decode()`

Decode image to thumbnail.

```c
PLUGIN_API PluginErrorCode PLUGIN_CALL plugin_decode(const DecodeRequest* request,
                                                       DecodeResult* result,
                                                       PluginProgressCallback progress);
```

**Parameters**:
- `request`: Decode parameters (input file/data, target size, options)
- `result`: Output structure to fill (allocate `pixels` buffer using host allocator)
- `progress`: Optional progress callback (can be `NULL`)

**Returns**: `PLUGIN_SUCCESS` or error code

**Example**:
```c
PLUGIN_API PluginErrorCode PLUGIN_CALL plugin_decode(const DecodeRequest* request,
                                                       DecodeResult* result,
                                                       PluginProgressCallback progress) {
    if (!request || !result) return PLUGIN_ERROR_INVALID_PARAMETER;
    
    // 1. Read file or use provided data
    // 2. Decode image
    // 3. Scale to target size
    // 4. Convert to BGRA32 format
    // 5. Allocate output buffer using host allocator
    // 6. Fill result structure
    
    uint32_t width = request->target_width;
    uint32_t height = request->target_height;
    uint32_t stride = width * 4;  // BGRA32 = 4 bytes per pixel
    size_t buffer_size = stride * height;
    
    // Allocate using host allocator
    uint8_t* pixels = (uint8_t*)g_allocator.alloc(buffer_size, g_allocator.user_data);
    if (!pixels) return PLUGIN_ERROR_OUT_OF_MEMORY;
    
    // ... decode and scale image into pixels buffer ...
    
    // Report progress (optional)
    if (progress) progress(0.5f, request->user_data);
    
    // Fill result
    result->pixels = pixels;
    result->buffer_size = buffer_size;
    result->stride = stride;
    result->width = width;
    result->height = height;
    result->pixel_format = PIXEL_FORMAT_BGRA32;
    result->metadata = NULL;  // Optional
    result->error_code = PLUGIN_SUCCESS;
    result->error_message = NULL;
    
    if (progress) progress(1.0f, request->user_data);
    
    return PLUGIN_SUCCESS;
}
```

#### `plugin_free_result()`

Free resources allocated in `plugin_decode()`.

```c
PLUGIN_API void PLUGIN_CALL plugin_free_result(DecodeResult* result);
```

**Parameters**:
- `result`: Result structure to free

**Example**:
```c
PLUGIN_API void PLUGIN_CALL plugin_free_result(DecodeResult* result) {
    if (!result) return;
    
    // Free pixel buffer
    if (result->pixels) {
        g_allocator.free(result->pixels, g_allocator.user_data);
        result->pixels = NULL;
    }
    
    // Free metadata if allocated
    if (result->metadata) {
        g_allocator.free(result->metadata, g_allocator.user_data);
        result->metadata = NULL;
    }
}
```

---

## Creating Your First Plugin

### Step 1: Project Setup

Create a new C/C++ DLL project:

```cmake
cmake_minimum_required(VERSION 3.20)
project(MyPlugin VERSION 1.0.0 LANGUAGES CXX)

# C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include SDK header
include_directories(${DARKTHUMBS_SDK_DIR})

# Build plugin DLL
add_library(myplugin SHARED
    myplugin.cpp
)

# Set output name
set_target_properties(myplugin PROPERTIES
    OUTPUT_NAME "myplugin"
    PREFIX ""  # No 'lib' prefix on Unix
)

# Link dependencies (your decoder library)
target_link_libraries(myplugin PRIVATE
    mydecoder
)

# Export plugin functions
target_compile_definitions(myplugin PRIVATE
    DARKTHUMBS_PLUGIN_EXPORTS
)
```

### Step 2: Implement Plugin

```cpp
#include "SDK/plugin_api.h"
#include <cstring>
#include <algorithm>

// Plugin information
static const char* supported_exts[] = {".myformat", NULL};
static const char* mime_types[] = {"image/x-myformat", NULL};

static const PluginInfo g_info = {
    .plugin_name = "My Format Decoder",
    .plugin_version = "1.0.0",
    .plugin_author = "Your Name",
    .plugin_description = "Decoder for MY custom format",
    .plugin_license = "MIT",
    .api_version = DARKTHUMBS_PLUGIN_API_VERSION,
    .supported_extensions = supported_exts,
    .mime_types = mime_types,
    .capabilities = PLUGIN_CAP_STILL_IMAGE | PLUGIN_CAP_ALPHA,
    .max_threads = 0,
    .requires_gpu = false,
    .supports_background_loading = true,
};

// Global allocator
static PluginAllocator g_allocator;

// Implement required functions
PLUGIN_API const PluginInfo* PLUGIN_CALL plugin_get_info(void) {
    return &g_info;
}

PLUGIN_API PluginErrorCode PLUGIN_CALL plugin_init(const PluginAllocator* allocator) {
    if (!allocator) return PLUGIN_ERROR_INVALID_PARAMETER;
    g_allocator = *allocator;
    return PLUGIN_SUCCESS;
}

PLUGIN_API void PLUGIN_CALL plugin_cleanup(void) {
    // Cleanup
}

PLUGIN_API bool PLUGIN_CALL plugin_can_decode(const char* file_path, 
                                                const uint8_t* data, 
                                                size_t data_size) {
    // Check extension or magic bytes
    if (file_path) {
        const char* ext = strrchr(file_path, '.');
        return ext && strcmp(ext, ".myformat") == 0;
    }
    return false;
}

PLUGIN_API PluginErrorCode PLUGIN_CALL plugin_decode(const DecodeRequest* request,
                                                       DecodeResult* result,
                                                       PluginProgressCallback progress) {
    // Your decode logic here
    // ...
    return PLUGIN_SUCCESS;
}

PLUGIN_API void PLUGIN_CALL plugin_free_result(DecodeResult* result) {
    if (result && result->pixels) {
        g_allocator.free(result->pixels, g_allocator.user_data);
        result->pixels = NULL;
    }
}
```

### Step 3: Test

```bash
cd build
cmake --build . --config Release
plugin-tester.exe Release/myplugin.dll test_image.myformat
```

### Step 4: Package

Create `manifest.json`:

```json
{
  "name": "My Format Decoder",
  "version": "1.0.0",
  "author": "Your Name",
  "description": "Decoder for MY custom format",
  "license": "MIT",
  "api_version": "1.0",
  "supported_extensions": [".myformat"],
  "mime_types": ["image/x-myformat"],
  "requires_gpu": false,
  "dependencies": []
}
```

Create `.dtplugin` package:

```
myplugin.dtplugin/
├── plugin.dll          (your plugin)
├── manifest.json       (metadata)
├── icon.png           (32x32 icon)
└── LICENSE.txt        (license)
```

---

## Best Practices

### Performance

1. **Fast Format Detection**: `plugin_can_decode()` should be very fast (<1ms)
   - Check extension first (fastest)
   - Check magic bytes only if needed
   - Don't decode the entire file

2. **Efficient Decoding**:
   - Decode directly to target size (avoid intermediate buffers)
   - Use SIMD/AVX2 for scaling (see `SIMDScaler.h`)
   - Support multi-threading for large images
   - Report progress for slow operations

3. **Memory Efficiency**:
   - Minimize allocations
   - Reuse buffers when possible
   - Free resources immediately after use

### Security

1. **Input Validation**:
   - Always validate parameters
   - Check buffer sizes
   - Handle corrupted files gracefully

2. **Error Handling**:
   - Never crash (return error codes instead)
   - Provide meaningful error messages
   - Use `try-catch` for C++ exceptions

3. **Resource Limits**:
   - Set maximum image dimensions
   - Limit memory usage
   - Add timeouts for slow operations

### Compatibility

1. **API Version**: Always check `DARKTHUMBS_PLUGIN_API_VERSION`
2. **Calling Convention**: Use `PLUGIN_CALL` (__stdcall on Windows)
3. **C ABI**: Don't expose C++ classes in API
4. **Dependencies**: Document all external dependencies

---

## Examples

See `SDK/examples/` for complete working examples:

- **webp-plugin**: Full-featured WebP decoder with animation
- **minimal-plugin**: Bare minimum plugin template
- **custom-format**: Custom format with metadata support

---

## Testing

### Manual Testing

```bash
plugin-tester.exe myplugin.dll test_image.ext --verbose
```

### Automated Testing

```cpp
#include <gtest/gtest.h>

TEST(MyPluginTest, CanDecodeValidFile) {
    EXPECT_TRUE(plugin_can_decode("test.myformat", nullptr, 0));
}

TEST(MyPluginTest, DecodeReturnsValidResult) {
    DecodeRequest request = {/* ... */};
    DecodeResult result = {0};
    EXPECT_EQ(PLUGIN_SUCCESS, plugin_decode(&request, &result, nullptr));
    EXPECT_NE(nullptr, result.pixels);
    plugin_free_result(&result);
}
```

---

## Distribution

1. **Package as `.dtplugin`**: Zip with manifest
2. **Host on GitHub**: Use releases for distribution
3. **Submit to Registry**: (Coming soon) Official plugin registry

### Installation

Users install by copying `.dtplugin` folder to:
- Windows: `%APPDATA%\DarkThumbs\Plugins\`
- Linux: `~/.config/darkthumbs/plugins/`
- macOS: `~/Library/Application Support/DarkThumbs/Plugins/`

---

## Support

- **Documentation**: [https://darkthumbs.dev/docs/plugin-sdk](https://darkthumbs.dev/docs/plugin-sdk)
- **API Reference**: `SDK/docs/API_REFERENCE.md`
- **Examples**: `SDK/examples/`
- **Issues**: [GitHub Issues](https://github.com/darkthumbs/darkthumbs/issues)
- **Discussion**: [GitHub Discussions](https://github.com/darkthumbs/darkthumbs/discussions)

## License

MIT License - See `LICENSE.txt`

---

**Copyright © 2026 DarkThumbs Project**
