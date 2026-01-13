# DarkThumbs Plugin API Documentation

**Version:** 5.3.0 (Engine v1.0.0)  
**Last Updated:** January 13, 2026  
**Target Audience:** Plugin developers creating custom thumbnail decoders

## Table of Contents

1. [Overview](#overview)
2. [Interface Contract](#interface-contract)
3. [Core Data Structures](#core-data-structures)
4. [Creating a Custom Decoder](#creating-a-custom-decoder)
5. [Best Practices](#best-practices)
6. [Example Implementation](#example-implementation)
7. [Registration & Integration](#registration--integration)
8. [Error Handling](#error-handling)
9. [Performance Guidelines](#performance-guidelines)

---

## Overview

The DarkThumbs Engine provides a plugin architecture for adding custom thumbnail decoders. Any format can be supported by implementing the `IThumbnailDecoder` interface. Decoders are registered with the `DecoderRegistry` and automatically invoked by the `ThumbnailPipeline` when matching file formats are detected.

**Key Features:**
- Clean interface-based architecture
- Thread-safe decoder registration
- Automatic format detection
- Cache integration
- GPU acceleration support
- Performance profiling

**Plugin Types:**
- **Image Decoders**: Handle image formats (JPEG, PNG, WebP, AVIF, JXL, etc.)
- **Archive Decoders**: Extract images from archives (ZIP, RAR, 7Z, CBZ, CBR)
- **Document Decoders**: Generate thumbnails from documents (PDF, EPUB)
- **Media Decoders**: Extract album art or video frames (MP3, FLAC, MP4, MKV)

---

## Interface Contract

### IThumbnailDecoder Interface

All decoders must implement this interface located in `Engine/Core/IThumbnailDecoder.h`:

```cpp
class IThumbnailDecoder
{
public:
    virtual ~IThumbnailDecoder() = default;
    
    // Format Detection
    virtual bool CanDecode(const wchar_t* filePath) = 0;
    
    // Thumbnail Generation
    virtual HRESULT Decode(
        const ThumbnailRequest& request, 
        ThumbnailResult& result) = 0;
    
    // Metadata
    virtual DecoderInfo GetInfo() const = 0;
    virtual const wchar_t* GetName() const = 0;
    virtual const wchar_t** GetSupportedExtensions() const = 0;
    virtual uint32_t GetExtensionCount() const = 0;
    
    // Capabilities
    virtual bool SupportsGPU() const = 0;
    virtual bool IsArchiveDecoder() const = 0;
};
```

### Method Specifications

#### `CanDecode(const wchar_t* filePath)`

**Purpose:** Quick check if decoder supports the file format.

**Parameters:**
- `filePath`: Full path to file (e.g., `C:\Images\photo.webp`)

**Returns:**
- `true`: Decoder can handle this file
- `false`: Decoder cannot handle this file

**Guidelines:**
- **MUST be fast** (< 1ms) - called for every thumbnail request
- Check file extension first (cheapest)
- Optionally validate magic bytes/signature
- **DO NOT** fully decode the file
- **DO NOT** perform I/O-heavy operations

**Example:**
```cpp
bool CanDecode(const wchar_t* filePath) override {
    const wchar_t* ext = wcsrchr(filePath, L'.');
    if (!ext) return false;
    
    return (_wcsicmp(ext, L".webp") == 0);
}
```

#### `Decode(const ThumbnailRequest& request, ThumbnailResult& result)`

**Purpose:** Decode file and generate thumbnail bitmap.

**Parameters:**
- `request`: Input request with file path, dimensions, flags
- `result`: Output result structure (decoder populates)

**Returns:**
- `S_OK` (0x00000000): Success
- `E_FAIL` (0x80004005): Generic failure
- `E_INVALIDARG` (0x80070057): Invalid request parameters
- `E_OUTOFMEMORY` (0x8007000E): Memory allocation failed
- `HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)`: File not found
- Custom error codes as appropriate

**Responsibilities:**
1. Open and read source file
2. Decode image data
3. Scale to requested dimensions (respecting aspect ratio if `PreserveAspect` flag set)
4. Create Windows `HBITMAP` with 32-bit BGRA pixels
5. Set `result.hBitmap`, `result.width`, `result.height`
6. Set `result.status` to `S_OK` or error code
7. Return same `HRESULT` as `result.status`

**Critical Requirements:**
- **MUST** create valid `HBITMAP` on success
- **MUST** set `result.hBitmap = nullptr` on failure
- **MUST** use 32-bit BGRA pixel format (`BI_RGB` with `biBitCount=32`)
- Caller owns bitmap - use `DeleteObject()` to free
- **MUST** be thread-safe (multiple simultaneous calls)
- Handle files up to 2GB+

**Example:**
```cpp
HRESULT Decode(const ThumbnailRequest& request, ThumbnailResult& result) override {
    result.hBitmap = nullptr;
    result.status = E_FAIL;
    
    // Validate input
    if (!request.filePath || request.width == 0 || request.height == 0) {
        result.status = E_INVALIDARG;
        return E_INVALIDARG;
    }
    
    // Decode file (implementation specific)
    int imageWidth, imageHeight;
    BYTE* pixelData = DecodeImageFile(request.filePath, &imageWidth, &imageHeight);
    if (!pixelData) {
        result.status = E_FAIL;
        return E_FAIL;
    }
    
    // Scale to target size
    BYTE* scaled = ScaleImage(pixelData, imageWidth, imageHeight, 
                             request.width, request.height);
    free(pixelData);
    
    // Create HBITMAP
    result.hBitmap = CreateBGRABitmap(scaled, request.width, request.height);
    free(scaled);
    
    if (!result.hBitmap) {
        result.status = E_OUTOFMEMORY;
        return E_OUTOFMEMORY;
    }
    
    result.width = request.width;
    result.height = request.height;
    result.status = S_OK;
    return S_OK;
}
```

#### `GetInfo()`

Returns decoder metadata:

```cpp
struct DecoderInfo {
    const wchar_t* name;           // e.g., L"WebP Decoder"
    const wchar_t* version;        // e.g., L"1.0.0"
    const wchar_t* description;    // e.g., L"WebP image decoder using libwebp 1.5.0"
    bool supportsGPU;              // GPU acceleration available
    bool isArchiveDecoder;         // Handles archives (ZIP, RAR)
};
```

#### `GetSupportedExtensions()`

**Returns:** Null-terminated array of file extensions.

**Requirements:**
- Extensions **MUST** include leading dot (`.webp`, not `webp`)
- Array **MUST** remain valid for decoder lifetime (use `static`)
- Last element **MUST** be `nullptr`
- Extensions are case-insensitive

**Example:**
```cpp
static const wchar_t* m_extensions[] = { L".webp", nullptr };

const wchar_t** GetSupportedExtensions() const override {
    return m_extensions;
}

uint32_t GetExtensionCount() const override {
    return 1; // Not including nullptr terminator
}
```

---

## Core Data Structures

### ThumbnailRequest

Input structure passed to `Decode()`:

```cpp
struct ThumbnailRequest {
    const wchar_t* filePath;       // Full path: "C:\Images\photo.webp"
    uint32_t width;                // Target width (pixels)
    uint32_t height;               // Target height (pixels)
    ThumbnailFlags flags;          // Generation flags (bitfield)
    const wchar_t* archiveEntry;   // Archive entry path (optional)
};
```

**ThumbnailFlags:**
```cpp
enum class ThumbnailFlags : uint32_t {
    None            = 0,
    FastMode        = 1 << 0,  // Speed over quality
    UseGPU          = 1 << 1,  // Enable GPU acceleration
    UseCache        = 1 << 2,  // Check cache first
    HighQuality     = 1 << 3,  // High-quality filtering
    PreserveAspect  = 1 << 4,  // Preserve aspect ratio (default)
};
```

**Usage:**
```cpp
if (request.flags & ThumbnailFlags::FastMode) {
    // Use fast decoding path
}

if (request.flags & ThumbnailFlags::PreserveAspect) {
    // Calculate aspect-preserving dimensions
    float aspectRatio = (float)imageWidth / imageHeight;
    uint32_t scaledWidth = request.width;
    uint32_t scaledHeight = (uint32_t)(scaledWidth / aspectRatio);
}
```

### ThumbnailResult

Output structure filled by `Decode()`:

```cpp
struct ThumbnailResult {
    HBITMAP hBitmap;              // Generated bitmap (required)
    uint32_t width;               // Actual width (required)
    uint32_t height;              // Actual height (required)
    HRESULT status;               // Result code (required)
    bool fromCache;               // Set by pipeline, not decoder
    bool usedGPU;                 // True if GPU acceleration used
    uint32_t generationTimeMs;    // Set by pipeline, not decoder
};
```

**Decoder Responsibilities:**
- Set `hBitmap`, `width`, `height`, `status`
- Optionally set `usedGPU` if decoder used GPU
- Pipeline sets `fromCache` and `generationTimeMs`

---

## Creating a Custom Decoder

### Step 1: Create Header File

`MyFormatDecoder.h`:

```cpp
#pragma once

#include "../Core/IThumbnailDecoder.h"

namespace DarkThumbs {
namespace Engine {

class MyFormatDecoder : public IThumbnailDecoder {
public:
    MyFormatDecoder();
    ~MyFormatDecoder() override = default;

    // IThumbnailDecoder interface
    bool CanDecode(const wchar_t* filePath) override;
    HRESULT Decode(const ThumbnailRequest& request, ThumbnailResult& result) override;
    DecoderInfo GetInfo() const override;
    const wchar_t* GetName() const override { return L"MyFormatDecoder"; }
    const wchar_t** GetSupportedExtensions() const override;
    uint32_t GetExtensionCount() const override { return 1; }
    bool SupportsGPU() const override { return false; }
    bool IsArchiveDecoder() const override { return false; }

private:
    // Extension list (static for lifetime guarantee)
    static const wchar_t* m_extensions[];
};

} // namespace Engine
} // namespace DarkThumbs
```

### Step 2: Implement CPP File

`MyFormatDecoder.cpp`:

```cpp
#include "MyFormatDecoder.h"
#include <fstream>
#include <vector>

namespace DarkThumbs {
namespace Engine {

const wchar_t* MyFormatDecoder::m_extensions[] = { L".myformat", nullptr };

MyFormatDecoder::MyFormatDecoder() {
    // Initialize decoder (load libraries, allocate resources)
}

bool MyFormatDecoder::CanDecode(const wchar_t* filePath) {
    if (!filePath) return false;
    
    const wchar_t* ext = wcsrchr(filePath, L'.');
    if (!ext) return false;
    
    return (_wcsicmp(ext, L".myformat") == 0);
}

HRESULT MyFormatDecoder::Decode(
    const ThumbnailRequest& request, 
    ThumbnailResult& result) 
{
    result.hBitmap = nullptr;
    result.status = E_FAIL;
    
    // 1. Validate input
    if (!request.filePath) {
        result.status = E_INVALIDARG;
        return E_INVALIDARG;
    }
    
    // 2. Read file
    std::ifstream file(request.filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        result.status = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        return result.status;
    }
    
    size_t fileSize = file.tellg();
    file.seekg(0);
    std::vector<BYTE> fileData(fileSize);
    file.read((char*)fileData.data(), fileSize);
    file.close();
    
    // 3. Decode format-specific data
    int imageWidth, imageHeight;
    std::vector<BYTE> rgbaPixels = DecodeMyFormat(
        fileData.data(), fileSize, &imageWidth, &imageHeight);
    
    if (rgbaPixels.empty()) {
        result.status = E_FAIL;
        return E_FAIL;
    }
    
    // 4. Calculate target dimensions (preserve aspect if requested)
    uint32_t targetWidth = request.width;
    uint32_t targetHeight = request.height;
    
    if (request.flags & ThumbnailFlags::PreserveAspect) {
        float aspect = (float)imageWidth / imageHeight;
        targetHeight = (uint32_t)(targetWidth / aspect);
        if (targetHeight > request.height) {
            targetHeight = request.height;
            targetWidth = (uint32_t)(targetHeight * aspect);
        }
    }
    
    // 5. Scale image (use bilinear or better filtering)
    std::vector<BYTE> scaled = ScaleRGBA(
        rgbaPixels.data(), imageWidth, imageHeight,
        targetWidth, targetHeight);
    
    // 6. Convert RGBA to BGRA (Windows format)
    for (size_t i = 0; i < scaled.size(); i += 4) {
        std::swap(scaled[i], scaled[i + 2]); // Swap R and B
    }
    
    // 7. Create HBITMAP (32-bit BGRA DIB section)
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = targetWidth;
    bmi.bmiHeader.biHeight = -(int)targetHeight; // Top-down DIB
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    
    void* pBits = nullptr;
    HBITMAP hBitmap = CreateDIBSection(
        nullptr, &bmi, DIB_RGB_COLORS, &pBits, nullptr, 0);
    
    if (!hBitmap) {
        result.status = E_OUTOFMEMORY;
        return E_OUTOFMEMORY;
    }
    
    // 8. Copy pixels to bitmap
    memcpy(pBits, scaled.data(), targetWidth * targetHeight * 4);
    
    // 9. Fill result structure
    result.hBitmap = hBitmap;
    result.width = targetWidth;
    result.height = targetHeight;
    result.status = S_OK;
    result.usedGPU = false;
    
    return S_OK;
}

DecoderInfo MyFormatDecoder::GetInfo() const {
    DecoderInfo info;
    info.name = L"My Format Decoder";
    info.version = L"1.0.0";
    info.description = L"Decodes .myformat files";
    info.supportsGPU = false;
    info.isArchiveDecoder = false;
    return info;
}

const wchar_t** MyFormatDecoder::GetSupportedExtensions() const {
    return m_extensions;
}

} // namespace Engine
} // namespace DarkThumbs
```

---

## Best Practices

### Performance

1. **Fast Format Detection**
   - `CanDecode()` must be < 1ms
   - Check extension first, magic bytes second
   - Avoid file I/O if possible

2. **Efficient Decoding**
   - Use incremental/streaming decoding for large files
   - Support partial decoding (decode only thumbnail resolution)
   - Implement fast paths for common cases

3. **Memory Management**
   - Minimize allocations
   - Reuse buffers across calls (thread-local storage)
   - Free resources immediately after use

4. **Threading**
   - All methods **MUST** be thread-safe
   - Multiple simultaneous `Decode()` calls are expected
   - Use thread-local state or mutex protection

### Quality

1. **Scaling Algorithms**
   - Use high-quality filtering (bilinear minimum, bicubic preferred)
   - Respect `ThumbnailFlags::HighQuality` flag
   - Consider pre-scaling for large images (512x512 → 256x256)

2. **Aspect Ratio**
   - Always respect `ThumbnailFlags::PreserveAspect`
   - Default behavior should preserve aspect ratio
   - Center or pad if exact dimensions required

3. **Color Management**
   - Preserve embedded color profiles when possible
   - Convert to sRGB for display
   - Handle alpha channel correctly (pre-multiplied vs straight)

### Error Handling

1. **Graceful Failures**
   - Return appropriate `HRESULT` codes
   - Set `result.status` and `result.hBitmap = nullptr` on error
   - Never throw exceptions (performance penalty)
   - Log errors using `OutputDebugString` for diagnostics

2. **Resource Cleanup**
   - Free all resources on error paths
   - Use RAII patterns (smart pointers, scoped guards)
   - Verify bitmap creation succeeded before returning

---

## Example Implementation

### Real-World Example: WebPDecoder

Reference implementation from `Engine/Decoders/WebPDecoder.cpp`:

```cpp
HRESULT WebPDecoder::Decode(
    const ThumbnailRequest& request, 
    ThumbnailResult& result) 
{
    result.hBitmap = nullptr;
    result.status = E_FAIL;
    
    // Validate
    if (!request.filePath) return E_INVALIDARG;
    
    // Read file
    std::ifstream file(request.filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }
    
    size_t size = file.tellg();
    file.seekg(0);
    std::vector<uint8_t> data(size);
    file.read((char*)data.data(), size);
    file.close();
    
    // Decode WebP (libwebp)
    int width, height;
    uint8_t* rgba = WebPDecodeRGBA(data.data(), size, &width, &height);
    if (!rgba) return E_FAIL;
    
    // Calculate aspect-preserving dimensions
    uint32_t targetWidth = request.width;
    uint32_t targetHeight = request.height;
    
    if (request.flags & ThumbnailFlags::PreserveAspect) {
        float aspect = (float)width / height;
        targetHeight = (uint32_t)(targetWidth / aspect);
    }
    
    // Scale using stb_image_resize
    std::vector<uint8_t> scaled(targetWidth * targetHeight * 4);
    stbir_resize_uint8(rgba, width, height, 0,
                      scaled.data(), targetWidth, targetHeight, 0, 4);
    WebPFree(rgba);
    
    // Convert RGBA → BGRA
    for (size_t i = 0; i < scaled.size(); i += 4) {
        std::swap(scaled[i], scaled[i + 2]);
    }
    
    // Create HBITMAP
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = targetWidth;
    bmi.bmiHeader.biHeight = -(int)targetHeight;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    
    void* pBits = nullptr;
    result.hBitmap = CreateDIBSection(nullptr, &bmi, DIB_RGB_COLORS, &pBits, nullptr, 0);
    if (!result.hBitmap) return E_OUTOFMEMORY;
    
    memcpy(pBits, scaled.data(), scaled.size());
    
    result.width = targetWidth;
    result.height = targetHeight;
    result.status = S_OK;
    result.usedGPU = false;
    
    return S_OK;
}
```

---

## Registration & Integration

### Static Registration (Current Method)

Edit `Engine/Pipeline/DecoderRegistry.cpp`:

```cpp
#include "MyFormatDecoder.h"

DecoderRegistry::DecoderRegistry() {
    RegisterDecoder(std::make_unique<ImageDecoder>());
    RegisterDecoder(std::make_unique<WebPDecoder>());
    RegisterDecoder(std::make_unique<AVIFDecoder>());
    RegisterDecoder(std::make_unique<ArchiveDecoder>());
    RegisterDecoder(std::make_unique<MyFormatDecoder>());  // ADD YOUR DECODER
}
```

### Dynamic Plugin Loading (Planned - Week 6)

Future support for runtime plugin loading:

```cpp
// Load external decoder plugin (DLL)
HMODULE hPlugin = LoadLibrary(L"MyFormatDecoder.dtplugin");
auto CreateDecoder = (IThumbnailDecoder*(*)())GetProcAddress(hPlugin, "CreateDecoder");
registry.RegisterDecoder(std::unique_ptr<IThumbnailDecoder>(CreateDecoder()));
```

**Plugin DLL Requirements:**
- Export `IThumbnailDecoder* CreateDecoder()` function
- Link against DarkThumbsEngine.lib
- Use `/MD` runtime (multithreaded DLL)
- Compatible ABI (C++17, MSVC 2022)

---

## Error Handling

### HRESULT Codes

Use standard Windows HRESULT codes:

| Code | Value | Meaning |
|------|-------|---------|
| `S_OK` | `0x00000000` | Success |
| `E_FAIL` | `0x80004005` | Generic failure |
| `E_INVALIDARG` | `0x80070057` | Invalid argument |
| `E_OUTOFMEMORY` | `0x8007000E` | Memory allocation failed |
| `E_NOTIMPL` | `0x80004001` | Not implemented |
| `E_UNEXPECTED` | `0x8000FFFF` | Unexpected error |
| `HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)` | `0x80070002` | File not found |
| `HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED)` | `0x80070005` | Access denied |

### Diagnostic Logging

Use `OutputDebugString` for Windows debug logging:

```cpp
#include <stdio.h>

void LogDecodeError(const wchar_t* filePath, HRESULT hr) {
    wchar_t msg[512];
    swprintf_s(msg, L"[MyFormatDecoder] Failed to decode %s: HRESULT=0x%08X\n",
              filePath, hr);
    OutputDebugStringW(msg);
}
```

View logs in Visual Studio **Output Window** or **DebugView**.

---

## Performance Guidelines

### Benchmarking Results (Reference)

From EngineBenchmark on test machine:

- **Cache Hit:** 2-3ms average (87% hit rate after warmup)
- **Cache Miss:** 27-43ms average (first decode)
- **Batch Throughput:** 377 images/second
- **Pipeline Overhead:** 8.7ms average (total including cache, decode, scale)

### Target Metrics

Your decoder should aim for:

- **CanDecode():** < 1ms (extension check only)
- **Small images (< 1MB):** < 50ms decode time
- **Large images (> 5MB):** < 200ms decode time
- **Memory usage:** < 100MB peak per thread

### Optimization Tips

1. **Incremental Decoding**
   - Decode only required resolution
   - Use progressive/subsampled decoding
   - Example: JPEG can decode 1/2, 1/4, 1/8 scale

2. **Caching**
   - Cache file metadata (dimensions, format)
   - Reuse decoded buffers (thread-local storage)
   - Engine handles thumbnail caching automatically

3. **SIMD Optimization**
   - Use SSE/AVX for pixel operations
   - RGBA→BGRA conversion benefits from SIMD
   - Scaling algorithms benefit from SIMD

4. **GPU Acceleration**
   - Set `SupportsGPU() = true` if using GPU
   - Use Direct3D 11 or compute shaders
   - Handle GPU unavailable gracefully (fallback to CPU)

---

## COM Initialization Requirements

### Standalone Applications

If your decoder uses COM-based Windows APIs (WIC, DirectX, Media Foundation), **the host application MUST initialize COM**:

```cpp
#include <objbase.h>

int main() {
    // Initialize COM (REQUIRED for WIC, DirectX, etc.)
    HRESULT hrCOM = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hrCOM)) {
        return -1;
    }
    
    // Use thumbnail engine
    DarkThumbs::Engine::ThumbnailPipeline pipeline;
    // ...
    
    // Cleanup COM
    CoUninitialize();
    return 0;
}
```

### Shell Extension DLLs

Explorer automatically initializes COM before loading shell extensions. **No explicit COM initialization needed** when running as shell extension.

### Decoder Implementations

**Decoders should NOT call `CoInitialize()` themselves.** The host application is responsible for COM initialization.

---

## Additional Resources

- **Engine Source Code:** `Engine/` directory
- **Example Decoders:**
  - `Engine/Decoders/ImageDecoder.cpp` - WIC-based decoder (JPEG, PNG, BMP, GIF, TIFF)
  - `Engine/Decoders/WebPDecoder.cpp` - libwebp-based decoder
  - `Engine/Decoders/AVIFDecoder.cpp` - libavif-based decoder
  - `Engine/Decoders/ArchiveDecoder.cpp` - Archive extraction decoder
- **Testing:** `Engine/Tests/EngineBenchmark.cpp` - Performance validation tool
- **Type Definitions:** `Engine/Core/Types.h` - All core structures and enums

---

## Support & Contributing

**Questions?** Contact the DarkThumbs development team.

**Contributing Decoders:**
1. Implement `IThumbnailDecoder` interface
2. Add comprehensive tests to `Engine/Tests/`
3. Update this documentation with format-specific notes
4. Submit pull request with decoder + tests + docs

**Roadmap:**
- **Sprint 11 (Current):** API documentation, performance optimization
- **Sprint 12 (Week 6):** Dynamic plugin loading, JXL decoder update
- **Sprint 13:** Plugin SDK, developer samples, marketplace integration

---

**Document Version:** 1.0  
**Engine Version:** 1.0.0 (DarkThumbs 5.3.0)  
**Last Updated:** January 13, 2026
