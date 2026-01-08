# cDarkThumbs Engine

**Version:** 5.3.0
**Created:** January 7, 2026
**Status:** Under Development (Sprint 11 - Platform Foundation)

---

## Overview

The DarkThumbs Engine is a standalone, reusable thumbnail generation library extracted from the CBXShell COM extension. It provides a clean interface-based architecture that enables:

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
using namespace DarkThumbs::Engine;

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
    // ...
  
    // Cleanup
    DeleteObject(result.hBitmap);
}
```

---

## Building

### Requirements

- Visual Studio 2026 or later
- CMake 3.20+
- Windows SDK 10.0.26200 or later

### Build Commands

```powershell
# Configure
cmake -B build -G "Visual Studio 17 2026" -A x64

# Build
cmake --build build --config Release

# Test
cd build/Release
.\EngineTests.exe
```

---

## Testing

The Engine includes comprehensive unit tests:

- **Format Detection Tests** (15 tests): Extension detection, file signatures
- **Decoder Tests** (20 tests): Image decoding, archive extraction, error handling
- **Pipeline Tests** (15 tests): End-to-end thumbnail generation, performance

**Target:** 50+ tests, 100% pass rate

---

## Status

### ✅ Completed (Week 1, Days 1-5)

- [X] Directory structure created
- [X] Core interfaces defined (IThumbnailDecoder, IFormatDetector, IGPURenderer, ICacheProvider)
- [X] Common types defined (ThumbnailRequest, ThumbnailResult, FormatType)
- [X] Public API header (Engine.h)
- [X] DecoderRegistry implemented (136 lines)
- [X] FormatDetector implemented (224 lines)
- [X] Unit tests created (14+ tests)
- [X] CMake build system configured

### 🔄 In Progress (Week 2)

- [ ] Decoder implementations (ImageDecoder, WebPDecoder, AVIFDecoder, ArchiveDecoder)
- [ ] Unit test execution and verification

### ⏳ Planned (Weeks 3-4)

- [ ] GPU renderer extraction (D3D11Renderer)
- [ ] CPU fallback renderer
- [ ] Thumbnail pipeline
- [ ] CBXShell integration
- [ ] Comprehensive unit tests (50+ tests target)

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

- [PROJECT_STATUS.md](../PROJECT_STATUS.md) - Overall project status
- [P2_ENGINE_REFACTORING_PLAN.md](../docs/P2_ENGINE_REFACTORING_PLAN.md) - Detailed implementation plan
- [ROADMAP.md](../ROADMAP.md) - Long-term roadmap

---

**Engine Development Lead:** DarkThumbs Team
**Created:** January 7, 2026
**Target Completion:** February 2026 (Sprint 11)
