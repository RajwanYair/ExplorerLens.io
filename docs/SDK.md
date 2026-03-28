# ExplorerLens SDK — v2 Reference Guide

**Version:** 23.6.0 "Vega-W"  
**Classification:** Public  
**Updated:** 2026-03-28

---

## Overview

The ExplorerLens SDK provides two integration paths:

| Integration Path | Use Case |
|-----------------|---------|
| **C ABI PublicAPI.h** | Stable extern "C" symbols — any language via P/Invoke, ctypes, etc. |
| **Format Plugin DLL** | Ship a new format decoder as a separate DLL loaded by the engine |
| **Thumbnail Provider DLL** | High-level provider registration for custom thumbnail rendering |

---

## Quick Start

### 1. Generate a Thumbnail (C++)

```cpp
#include "Engine/SDK/PublicAPI.h"

LENS_ENGINE_HANDLE hEngine = nullptr;
LensEngineCreate(&hEngine);

LENS_THUMB_OPTIONS opts = { 256, 256, 0, 10000 };
LENS_THUMBNAIL_HANDLE hThumb = nullptr;
LENS_RESULT r = LensGenerateThumbnail(hEngine, L"C:\\photo.cr3", &opts, &hThumb);
if (r == LENS_OK) {
    LENS_IMAGE_DESC desc = {};
    LensGetImageDesc(hThumb, &desc);
    // desc.pixels contains BGRA32 bitmap data
    LensReleaseThumbnail(hThumb);
}
LensEngineDestroy(hEngine);
```

### 2. Write a Format Plugin DLL

Implement and export `LensCreateFormatPlugin` using `FormatPluginSDK.h`:

```cpp
#include "Engine/SDK/FormatPluginSDK.h"

extern "C" LENS_API LENS_RESULT WINAPI LensCreateFormatPlugin(
    void** ppCtx,
    LENS_FORMAT_PLUGIN_VTABLE_V2* pVTable,
    LENS_FORMAT_PLUGIN_MANIFEST*  pManifest)
{
    // Fill vtable with your decode functions
    pVTable->abiVersion  = LENS_FORMAT_PLUGIN_ABI;
    pVTable->DecodeFile  = MyDecodeFile;
    pVTable->FreeFrameV2 = MyFreeFrame;
    pVTable->Destroy     = MyDestroy;

    // Fill manifest
    pManifest->extensionCount = 2;
    wcscpy_s(pManifest->extensions[0], L".myf");
    wcscpy_s(pManifest->extensions[1], L".myx");
    wcscpy_s(pManifest->name, L"MyFormat Decoder");
    pManifest->version = (1 << 16) | (0 << 8) | 0;

    *ppCtx = new MyPluginContext();
    return LENS_OK;
}
```

### 3. Register a Thumbnail Provider (runtime)

```cpp
#include "Engine/SDK/ThumbnailProviderSDK.h"

LENS_PROVIDER_DESC desc = {};
desc.structSize = sizeof(desc);
wcscpy_s(desc.name, L"MyCADThumb");
desc.extensionCount = 1;
wcscpy_s(desc.extensions[0], L".stp");
desc.CreateInstance = MyCreateInstance;

uint32_t providerId = 0;
LensRegisterThumbnailProvider(hEngine, &desc, &providerId);
```

---

## API Reference

### Engine Lifecycle

| Function | Description |
|----------|-------------|
| `LensEngineCreate(phEngine)` | Initialize engine, GPU device, and cache |
| `LensEngineDestroy(hEngine)` | Flush cache and release all resources |
| `LensGetAPIVersion()` | Returns `(major<<16)\|(minor<<8)\|patch` |
| `LensGetVersionString()` | Returns e.g. `"20.4.0"` |

### Thumbnail Generation

| Function | Description |
|----------|-------------|
| `LensGenerateThumbnail(hEngine, path, opts, phThumb)` | Decode from file path |
| `LensGenerateThumbnailFromStream(hEngine, pStream, hintExt, opts, phThumb)` | Decode from `IStream*` |
| `LensGetImageDesc(hThumb, pDesc)` | Access pixel buffer + dim |
| `LensReleaseThumbnail(hThumb)` | Free pixel memory |

### Cache Management

| Function | Description |
|----------|-------------|
| `LensClearCache(hEngine)` | Evict all cached thumbnails |
| `LensGetCacheStats(hEngine, &used, &cap)` | Query current usage |

### Format Query

| Function | Description |
|----------|-------------|
| `LensIsFormatSupported(ext)` | Returns 1/0 for e.g. `L".heic"` |

---

## ABI Versioning

Plugins **must** export `LensPluginGetVersion()` returning `uint32_t` encoded as
`(major<<16)|(minor<<8)|patch`. `SDKVersionGuard` validates this on load:

| Engine Major | Compatible Plugin Major | Notes |
|-------------|------------------------|-------|
| 20 | 20 | Same major — fully compatible |
| 20 | 19 | Rejected — below minimum v19.0 |
| 20 | 21 | `NewerMinor` — load with caution |

---

## Testing Plugins

Use `MockShellEnvironment` to test plugins without a Windows Shell host:

```cpp
#include "Engine/SDK/MockShellEnvironment.h"
using namespace ExplorerLens::Engine::Testing;

MockShellEnvironment env;
auto* stream = MockStreamFromFile(L"sample.myf");
// pass stream to your plugin vtable's DecodeStream...
stream->Release();
```

---

## lens.exe CLI Reference

```
Usage: lens <command> [options]

Commands:
  generate  <file> [--output <path>] [--width N] [--height N] [--hq]
  batch     <dir>  --output-dir <dir> [--filter *.psd] [--recursive] [--threads N]
  cache     [clear | stats | warm <dir>]
  info      <file>
  formats
  version
  help      [<command>]
```

---

## Changelog

### v20.4.0 "Quasar-U"
- `PublicAPI.h` — stable extern "C" ABI surface
- `ThumbnailProviderSDK.h` — high-level provider registration
- `LensCLI.h` / `CLICommandParser.h` — lens.exe driver and argument parser
- `FormatPluginSDK.h` — v2 format plugin contract with async decode
- `BatchCLI.h` — batch processing driver with progress bar
- `SDKVersionGuard.h` — ABI compatibility checking
- `MockShellEnvironment.h` — test harness for SDK authors
