# ExplorerLens Plugin Developer Guide

> **SDK Version:** 2.0.0 | **Engine Version:** 23.7.0 "Vega-X"

## Overview

The ExplorerLens Plugin SDK enables third-party developers to add support for new
file formats to ExplorerLens without modifying the core engine. Plugins are loaded
dynamically as DLLs and communicate via a stable C ABI defined in `SDK/plugin_api.h`.

## Quick Start

### 1. Create a Plugin DLL

```c
// my_format_plugin.c
#include "plugin_api.h"

// Plugin metadata — required
PLUGIN_API plugin_result_t plugin_get_info(plugin_info_t* info) {
    info->api_version_major = 1;
    info->api_version_minor = 0;
    info->api_version_patch = 0;
    strncpy_s(info->name, sizeof(info->name), "MyFormat Plugin", _TRUNCATE);
    strncpy_s(info->version, sizeof(info->version), "1.0.0", _TRUNCATE);
    strncpy_s(info->author, sizeof(info->author), "Your Name", _TRUNCATE);
    strncpy_s(info->description, sizeof(info->description),
              "Adds support for .myf files", _TRUNCATE);
    return PLUGIN_SUCCESS;
}

// Format registration — called during plugin discovery
PLUGIN_API plugin_result_t plugin_get_supported_formats(
    plugin_format_t* formats, uint32_t* count)
{
    if (*count < 1) {
        *count = 1;
        return PLUGIN_ERROR_BUFFER_TOO_SMALL;
    }
    strncpy_s(formats[0].extension, sizeof(formats[0].extension), ".myf", _TRUNCATE);
    strncpy_s(formats[0].mime_type, sizeof(formats[0].mime_type),
              "application/x-myformat", _TRUNCATE);
    formats[0].priority = 100; // Higher = preferred
    *count = 1;
    return PLUGIN_SUCCESS;
}

// Thumbnail generation — the core decode function
PLUGIN_API plugin_result_t plugin_generate_thumbnail(
    const plugin_decode_request_t* request,
    plugin_decode_result_t* result)
{
    // request->file_path contains the file path (UTF-16)
    // request->requested_width / requested_height contain target size
    // request->stream_data / stream_size contain file data if available

    // Decode your format here...
    // Fill result->pixel_data with BGRA8 pixels
    // Set result->width, result->height, result->stride

    result->width = request->requested_width;
    result->height = request->requested_height;
    result->stride = result->width * 4;
    result->pixel_format = PLUGIN_PIXEL_FORMAT_BGRA8;

    size_t size = (size_t)result->stride * result->height;
    result->pixel_data = (uint8_t*)plugin_alloc(size);
    if (!result->pixel_data) return PLUGIN_ERROR_OUT_OF_MEMORY;

    // Fill with a test pattern (magenta)
    for (size_t i = 0; i < size; i += 4) {
        result->pixel_data[i + 0] = 255; // B
        result->pixel_data[i + 1] = 0;   // G
        result->pixel_data[i + 2] = 255; // R
        result->pixel_data[i + 3] = 255; // A
    }

    result->pixel_data_size = (uint32_t)size;
    return PLUGIN_SUCCESS;
}

// Lifecycle hooks
PLUGIN_API plugin_result_t plugin_initialize(void) {
    return PLUGIN_SUCCESS;
}

PLUGIN_API plugin_result_t plugin_shutdown(void) {
    return PLUGIN_SUCCESS;
}
```

### 2. Build the Plugin

**CMakeLists.txt:**

```cmake
cmake_minimum_required(VERSION 3.20)
project(MyFormatPlugin VERSION 1.0.0 LANGUAGES C)

add_library(MyFormatPlugin SHARED my_format_plugin.c)

# Point to ExplorerLens SDK header
target_include_directories(MyFormatPlugin PRIVATE
    "${EXPLORERLENS_SDK_DIR}")

# Export symbols
target_compile_definitions(MyFormatPlugin PRIVATE PLUGIN_EXPORTS)

# Windows-specific
if(MSVC)
    target_compile_options(MyFormatPlugin PRIVATE /W4 /MD)
endif()
```

### 3. Install the Plugin

Copy the DLL to the ExplorerLens plugins directory:

```text
%LOCALAPPDATA%\ExplorerLens\Plugins\MyFormatPlugin.dll
```

Or register via LENSManager.exe → Plugins → Install From File.

## Plugin API Reference

### Required Exports

| Function | Purpose |
| ---------- | --------- |
| `plugin_get_info` | Return plugin metadata |
| `plugin_get_supported_formats` | Register handled file extensions |
| `plugin_generate_thumbnail` | Generate a thumbnail for a file |
| `plugin_initialize` | One-time initialization |
| `plugin_shutdown` | Cleanup before unload |

### Optional Exports

| Function | Purpose |
| ---------- | --------- |
| `plugin_get_metadata` | Extract file properties |
| `plugin_validate_file` | Quick format validation |
| `plugin_get_capabilities` | Report feature flags |

### Data Types

```c
// Plugin info structure
typedef struct {
    uint32_t api_version_major;
    uint32_t api_version_minor;
    uint32_t api_version_patch;
    char     name[128];
    char     version[32];
    char     author[128];
    char     description[512];
} plugin_info_t;

// Decode request (engine → plugin)
typedef struct {
    const wchar_t* file_path;
    const uint8_t* stream_data;
    uint32_t       stream_size;
    uint32_t       requested_width;
    uint32_t       requested_height;
    uint32_t       flags;
} plugin_decode_request_t;

// Decode result (plugin → engine)
typedef struct {
    uint8_t* pixel_data;         // BGRA8 pixel buffer
    uint32_t pixel_data_size;
    uint32_t width;
    uint32_t height;
    uint32_t stride;
    uint32_t pixel_format;       // PLUGIN_PIXEL_FORMAT_BGRA8
} plugin_decode_result_t;

// Format descriptor
typedef struct {
    char     extension[16];      // e.g., ".myf"
    char     mime_type[128];
    uint32_t priority;           // Higher = preferred decoder
} plugin_format_t;
```

### Error Codes

| Code | Name | Meaning |
| ------ | ------ | --------- |
| 0 | `PLUGIN_SUCCESS` | Operation succeeded |
| 1 | `PLUGIN_ERROR_GENERIC` | Unspecified error |
| 2 | `PLUGIN_ERROR_INVALID_PARAM` | Bad parameter |
| 3 | `PLUGIN_ERROR_OUT_OF_MEMORY` | Allocation failed |
| 4 | `PLUGIN_ERROR_FORMAT_NOT_SUPPORTED` | File format not recognized |
| 5 | `PLUGIN_ERROR_DECODE_FAILED` | Decode failed |
| 6 | `PLUGIN_ERROR_BUFFER_TOO_SMALL` | Output buffer too small |
| 7 | `PLUGIN_ERROR_NOT_INITIALIZED` | Plugin not initialized |

### Memory Management

```c
// Allocate memory that the engine will free
void* plugin_alloc(size_t size);

// Free memory allocated by plugin_alloc
void plugin_free(void* ptr);
```

Always use `plugin_alloc` for `pixel_data` in `plugin_decode_result_t`.
The engine takes ownership and will call `plugin_free` when done.

## Security & Sandboxing

Plugins run inside a sandboxed environment:

- **Job Object:** CPU time limits, memory limits, restricted process creation
- **Trust Chain:** Plugins can be code-signed for verified publisher status
- **Capabilities:** Plugins declare required capabilities in their info struct

### Trust Levels

| Level | Description | Capabilities |
| ------- | ------------- | -------------- |
| Untrusted | Unknown publisher | Sandbox only, no filesystem |
| Signed | Valid code signature | Filesystem read (registered extensions) |
| Trusted | ExplorerLens-verified | Full capabilities |

## Debugging Plugins

1. Build with debug info (`/Zi` on MSVC)
2. Attach Visual Studio to `explorer.exe` or `prevhost.exe`
3. Set breakpoints in your `plugin_generate_thumbnail` function
4. Browse to a folder containing your format files in Explorer

### Diagnostic Logging

```c
// Optional: implement logging callback
PLUGIN_API void plugin_set_log_callback(plugin_log_fn callback) {
    g_log = callback;
}

// Use in your plugin:
if (g_log) g_log(PLUGIN_LOG_INFO, "Decoding file: %s", filename);
```

## Performance Guidelines

- **Target decode time:** < 50ms per thumbnail
- **Memory limit:** < 256 MB per decode operation
- **Thread safety:** `plugin_generate_thumbnail` may be called from multiple threads
- **No global state:** Use the request/result pattern, not global variables
- **Early exit:** Return `PLUGIN_ERROR_FORMAT_NOT_SUPPORTED` quickly for unrecognized files

## Versioning

The plugin API follows semantic versioning:

- **Major:** Breaking ABI changes (existing plugins won't load)
- **Minor:** New optional functions added (backward compatible)
- **Patch:** Bug fixes only (fully compatible)

Current: **v1.0.0** — stable, no breaking changes planned for v15.x engine releases.
