# ExplorerLens Engine

**Version:** 15.0.0
**Created:** January 7, 2026
**Status:** Active Development (v14.0 "Apex" Complete)

---

## Overview

The ExplorerLens Engine is a standalone, reusable thumbnail generation library extracted from the LENSShell COM extension. It provides a clean interface-based architecture that enables:

- **Independent Development**: Engine can be developed and tested without COM overhead
- **Plugin Support**: Third-party plugins can implement `IThumbnailDecoder`
- **Multi-Application**: Same engine used by shell extension, manager app, and plugins
- **Testability**: Comprehensive unit testing of core functionality

---

## Architecture

```
Engine/
├── Core/                        # Core interfaces and types
│   ├── Types.h                  # Common types (ThumbnailRequest, ThumbnailResult, etc.)
│   ├── IThumbnailDecoder.h      # Decoder interface
│   ├── IFormatDetector.h        # Format detection interface
│   ├── IGPURenderer.h           # GPU rendering interface
│   └── ICacheProvider.h         # Cache interface
├── Decoders/                    # Format-specific decoders
│   ├── ImageDecoder.cpp         # JPEG, PNG, BMP, GIF, TIFF
│   ├── WebPDecoder.cpp          # WebP format
│   ├── AVIFDecoder.cpp          # AVIF format
│   └── ArchiveDecoder.cpp       # ZIP, RAR, 7z archives
├── Pipeline/                    # Orchestration
│   ├── ThumbnailPipeline.cpp    # Main pipeline implementation
│   ├── DecoderRegistry.cpp      # Decoder management
│   └── FormatDetector.cpp       # Format detection implementation
├── GPU/                         # GPU rendering
│   ├── D3D11Renderer.cpp        # DirectX 11 renderer
│   └── CPURenderer.cpp          # CPU fallback
├── Tests/                       # Unit tests
│   ├── FormatDetectionTests.cpp
│   ├── DecoderTests.cpp
│   └── PipelineTests.cpp
└── Engine.h                     # Public API header
```

---

## Key Interfaces

### IThumbnailDecoder

Core interface that all decoders must implement:

```cpp
class IThumbnailDecoder {
public:
    virtual bool CanDecode(const wchar_t* filePath) = 0;
    virtual HRESULT Decode(const ThumbnailRequest& request, 
                          ThumbnailResult& result) = 0;
    virtual DecoderInfo GetInfo() const = 0;
    virtual const wchar_t* GetName() const = 0;
    virtual const wchar_t** GetSupportedExtensions() const = 0;
};
```

### IFormatDetector

Detects file formats by extension or signature:

```cpp
class IFormatDetector {
public:
    virtual FormatType DetectFormat(const wchar_t* filePath) = 0;
    virtual FormatType DetectFromExtension(const wchar_t* extension) = 0;
    virtual bool IsImageFormat(const wchar_t* extension) const = 0;
    virtual bool IsArchiveFormat(const wchar_t* extension) const = 0;
};
```

### IGPURenderer

GPU acceleration abstraction:

```cpp
class IGPURenderer {
public:
    virtual HRESULT Initialize() = 0;
    virtual bool IsAvailable() const = 0;
    virtual HRESULT RenderThumbnail(
        const uint8_t* imageData, 
        uint32_t imageWidth, uint32_t imageHeight,
        uint32_t thumbWidth, uint32_t thumbHeight,
        HBITMAP* outBitmap) = 0;
};
```

---

## Usage Example

```cpp
#include <Engine.h>
using namespace ExplorerLens::Engine;

// Create thumbnail request
ThumbnailRequest request;
request.filePath = L"C:\\photos\\image.jpg";
request.width = 256;
request.height = 256;
request.flags = ThumbnailFlags::UseGPU | ThumbnailFlags::UseCache;

// Generate thumbnail via decoder
IThumbnailDecoder* decoder = /* ... get appropriate decoder ... */;
ThumbnailResult result;
HRESULT hr = decoder->Decode(request, result);

if (SUCCEEDED(hr)) {
    // Use result.hBitmap
    DeleteObject(result.hBitmap);
}
```

---

## Internal Architecture

### ThumbnailPipeline (Main Entry Point)

Orchestrates the full thumbnail generation process. Thread-safe for `GenerateThumbnail()` calls.

**Performance Baseline:**
- First call: ~50–100ms (cold cache, decoder init)
- Cache hit: <1ms (sub-millisecond cache engine)
- Average (warm): ~17ms per thumbnail
- Batch throughput: ~235 img/sec

### Decoder System

Plugin-based architecture with `IThumbnailDecoder` interface. 25 built-in decoders covering 200+ formats. Decoders are tried in registration order; first `CanDecode() = true` match handles the file.

### Cache System

Disk-backed LRU cache with adaptive budget control. Cache keys derived from `MD5(filepath + filesize + mtime + width + height)`. Sub-millisecond cache engine uses robin-hood open-addressing with XXH3 hashing.

### SIMD Optimization

Runtime CPU feature detection for AVX2 (~3–4x), SSE4.1 (~2–3x), or scalar fallback. Used for color conversion (RGBA ↔ BGRA), image scaling, and RAW demosaicing.

### GPU Acceleration

DirectX 11/12 compute shaders for image scaling >4K, batch requests, and HEIF/AVIF decode via WIC GPU path. Falls back to CPU automatically on GPU init failure. Requires DirectX 11.0+, WDDM 2.0+ driver, 256MB VRAM minimum.

### Format Detection

Three-stage detection: extension check (fast path) → magic bytes (first 16 bytes) → full decoder scan (last resort).

### Threading Model

- **Thread-safe:** `GenerateThumbnail()`, cache operations, most decoders (stateless)
- **Main thread only:** `Initialize()`, `Shutdown()`, decoder registration
- **Special:** RAWDecoder requires serialization (LibRaw global state)

---

## Building

### Requirements

- Visual Studio 18 2026 BuildTools (MSVC v145 toolset)
- CMake 3.25+
- Windows SDK 10.0.26100.0

### Build Commands

```powershell
# Recommended (handles vcvars automatically)
.\build-scripts\Build-MSVC.ps1

# Manual (requires vcvars64 sourced first)
cmake --preset default-release
cmake --build --preset default-release -j 8
```

---

## Testing

~1,187 unit tests + 5 benchmark suites. 100% pass rate.

```powershell
ctest --test-dir build -C Release --output-on-failure
```

---

## Design Principles

1. **Interface-Based**: All major components accessed via interfaces
2. **Dependency Injection**: Decoders, renderers, cache providers injected into pipeline
3. **Zero COM Dependencies**: Engine has no COM dependencies (pure C++)
4. **Testability First**: All components designed for unit testing
5. **Performance**: GPU acceleration, smart caching, fast format detection
6. **Extensibility**: Easy to add new decoders via plugin system

---

## References

- [docs/formats/FORMAT_SUPPORT_MATRIX.md](../docs/formats/FORMAT_SUPPORT_MATRIX.md) — Format support and decoder compliance
- [docs/architecture/INTEGRATION_ARCHITECTURE.md](../docs/architecture/INTEGRATION_ARCHITECTURE.md) — Integration architecture
- [docs/ENHANCEMENT_PLAN_V15.md](../docs/ENHANCEMENT_PLAN_V15.md) — Next iteration roadmap

---

**Created:** January 7, 2026

