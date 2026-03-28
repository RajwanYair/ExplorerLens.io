# ExplorerLens Plugin Development Guide
**Version:** 23.5.0 "Vega-V" 
**Last Updated:** March 27, 2026

---

## Table of Contents
1. [Introduction](#introduction)
2. [Architecture Overview](#architecture-overview)
3. [Creating a Custom Decoder](#creating-a-custom-decoder)
4. [Decoder Interface](#decoder-interface)
5. [Build System Integration](#build-system-integration)
6. [Testing Your Decoder](#testing-your-decoder)
7. [Packaging & Distribution](#packaging--distribution)
8. [Advanced Topics](#advanced-topics)
9. [Example: Complete Decoder](#example-complete-decoder)

---

## Introduction

### What Are Plugins?

**ExplorerLens plugins** are custom thumbnail decoders that extend format support beyond the built-in codecs.

**Use cases:**
- Proprietary image formats (medical imaging, scientific data)
- Niche file types (CAD drawings, GIS data)
- Custom container formats (game assets, archives)
- Hardware-accelerated decoders (NVDEC, Intel QSV)

---

### Plugin Architecture

**Key concepts:**
- **Decoder:** C++ class implementing `IThumbnailDecoder` interface
- **Registration:** Automatic via `DecoderRegistry::RegisterDecoder()`
- **Format Routing:** Based on file extension (`.xyz` → `XYZDecoder`)
- **Lifecycle:** Decoders created on-demand, destroyed after use

---

## Architecture Overview

### ExplorerLens Engine Structure

```
ExplorerLensEngine.lib
├── DecoderRegistry (singleton)
│ ├── RegisterDecoder(extension, factory)
│ ├── GetDecoderForExtension(extension) → IThumbnailDecoder*
│ └── ListSupportedExtensions() → vector<string>
├── IThumbnailDecoder (interface)
│ ├── CanDecode(filePath) → bool
│ ├── Decode(filePath, maxWidth, maxHeight) → ThumbnailBitmap
│ └── GetFormatName() → string
├── ThumbnailBitmap (output)
│ ├── width, height, stride
│ ├── data (RGBA8888 pixel buffer)
│ └── format (DXGI_FORMAT_R8G8B8A8_UNORM)
└── CacheManager
 ├── Get(key) → ThumbnailBitmap
 └── Put(key, bitmap)
```

---

### Decoder Lifecycle

```
1. User opens folder in Explorer
 ↓
2. Explorer requests thumbnail via IThumbnailProvider::GetThumbnail()
 ↓
3. ExplorerLens checks cache: CacheManager::Get(filePath)
 ↓ (cache miss)
4. DecoderRegistry::GetDecoderForExtension(".xyz")
 ↓
5. XYZDecoder* decoder = XYZDecoderFactory()
 ↓
6. decoder->CanDecode(filePath) → true
 ↓
7. ThumbnailBitmap bitmap = decoder->Decode(filePath, 256, 256)
 ↓
8. CacheManager::Put(filePath, bitmap)
 ↓
9. Render bitmap to Explorer thumbnail
 ↓
10. delete decoder
```

---

## Creating a Custom Decoder

### Step 1: Define Decoder Class

```cpp
// MyCustomDecoder.h
#pragma once
#include "IThumbnailDecoder.h"
#include <string>
#include <vector>

namespace ExplorerLens {

class MyCustomDecoder : public IThumbnailDecoder {
public:
 MyCustomDecoder() = default;
 ~MyCustomDecoder() override = default;

 // IThumbnailDecoder interface
 bool CanDecode(const std::wstring& filePath) const override;
 
 ThumbnailBitmap Decode(
 const std::wstring& filePath,
 uint32_t maxWidth,
 uint32_t maxHeight
 ) override;
 
 std::string GetFormatName() const override {
 return "MyCustomFormat";
 }
 
 std::vector<std::string> GetSupportedExtensions() const override {
 return {".xyz", ".custom"};
 }

private:
 // Helper functions
 bool ValidateHeader(const uint8_t* data, size_t size) const;
 std::vector<uint8_t> DecompressData(const uint8_t* compressed, size_t size) const;
};

} // namespace ExplorerLens
```

---

### Step 2: Implement Decoder Logic

```cpp
// MyCustomDecoder.cpp
#include "MyCustomDecoder.h"
#include <fstream>
#include <stdexcept>

namespace ExplorerLens {

bool MyCustomDecoder::CanDecode(const std::wstring& filePath) const {
 // Quick validation: Check file extension
 if (filePath.size() < 4) {
 return false;
 }
 
 std::wstring ext = filePath.substr(filePath.size() - 4);
 std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
 
 if (ext != L".xyz" && ext != L".custom") {
 return false;
 }
 
 // Validate file header (magic bytes)
 std::ifstream file(filePath, std::ios::binary);
 if (!file.is_open()) {
 return false;
 }
 
 uint8_t header[4];
 file.read(reinterpret_cast<char*>(header), 4);
 
 // Check magic bytes: "XYZ\0"
 return header[0] == 'X' && header[1] == 'Y' && 
 header[2] == 'Z' && header[3] == 0;
}

ThumbnailBitmap MyCustomDecoder::Decode(
 const std::wstring& filePath,
 uint32_t maxWidth,
 uint32_t maxHeight
) {
 // Read entire file
 std::ifstream file(filePath, std::ios::binary | std::ios::ate);
 if (!file.is_open()) {
 throw std::runtime_error("Failed to open file");
 }
 
 size_t fileSize = file.tellg();
 file.seekg(0);
 
 std::vector<uint8_t> fileData(fileSize);
 file.read(reinterpret_cast<char*>(fileData.data()), fileSize);
 
 // Validate header
 if (!ValidateHeader(fileData.data(), fileSize)) {
 throw std::runtime_error("Invalid file header");
 }
 
 // Parse dimensions (bytes 4-11: width, height as uint32_t little-endian)
 uint32_t imageWidth = *reinterpret_cast<const uint32_t*>(&fileData[4]);
 uint32_t imageHeight = *reinterpret_cast<const uint32_t*>(&fileData[8]);
 
 // Decompress pixel data (starting at byte 12)
 const uint8_t* compressedData = fileData.data() + 12;
 size_t compressedSize = fileSize - 12;
 
 std::vector<uint8_t> pixelData = DecompressData(compressedData, compressedSize);
 
 // Calculate thumbnail size (maintain aspect ratio)
 float scale = std::min(
 static_cast<float>(maxWidth) / imageWidth,
 static_cast<float>(maxHeight) / imageHeight
 );
 
 uint32_t thumbWidth = static_cast<uint32_t>(imageWidth * scale);
 uint32_t thumbHeight = static_cast<uint32_t>(imageHeight * scale);
 
 // Resize image (bilinear interpolation)
 std::vector<uint8_t> thumbData = ResizeImage(
 pixelData.data(), imageWidth, imageHeight,
 thumbWidth, thumbHeight
 );
 
 // Create ThumbnailBitmap
 ThumbnailBitmap result;
 result.width = thumbWidth;
 result.height = thumbHeight;
 result.stride = thumbWidth * 4; // RGBA = 4 bytes per pixel
 result.format = DXGI_FORMAT_R8G8B8A8_UNORM;
 result.data = std::move(thumbData);
 
 return result;
}

bool MyCustomDecoder::ValidateHeader(const uint8_t* data, size_t size) const {
 if (size < 12) {
 return false; // Too small
 }
 
 // Check magic bytes
 if (data[0] != 'X' || data[1] != 'Y' || data[2] != 'Z' || data[3] != 0) {
 return false;
 }
 
 // Check dimensions are reasonable (< 100,000 pixels)
 uint32_t width = *reinterpret_cast<const uint32_t*>(&data[4]);
 uint32_t height = *reinterpret_cast<const uint32_t*>(&data[8]);
 
 if (width == 0 || height == 0 || width > 100000 || height > 100000) {
 return false;
 }
 
 return true;
}

std::vector<uint8_t> MyCustomDecoder::DecompressData(
 const uint8_t* compressed,
 size_t size
) const {
 // Implement decompression logic here
 // Example: zlib, LZ4, custom algorithm
 
 // Placeholder: Copy as-is (assumes uncompressed)
 std::vector<uint8_t> decompressed(compressed, compressed + size);
 return decompressed;
}

} // namespace ExplorerLens
```

---

### Step 3: Register Decoder

**Auto-registration via static initializer:**

```cpp
// MyCustomDecoderRegistration.cpp
#include "MyCustomDecoder.h"
#include "DecoderRegistry.h"

namespace ExplorerLens {

// Factory function
static IThumbnailDecoder* CreateMyCustomDecoder() {
 return new MyCustomDecoder();
}

// Auto-register on DLL load
static bool s_registered = []() {
 DecoderRegistry::GetInstance().RegisterDecoder(
 ".xyz",
 CreateMyCustomDecoder
 );
 DecoderRegistry::GetInstance().RegisterDecoder(
 ".custom",
 CreateMyCustomDecoder
 );
 return true;
}();

} // namespace ExplorerLens
```

---

## Decoder Interface

### IThumbnailDecoder API

```cpp
class IThumbnailDecoder {
public:
 virtual ~IThumbnailDecoder() = default;
 
 // Check if decoder can handle this file
 // Return false to skip (no exception thrown)
 virtual bool CanDecode(const std::wstring& filePath) const = 0;
 
 // Decode file to thumbnail bitmap
 // Throw std::runtime_error or DecoderException on failure
 virtual ThumbnailBitmap Decode(
 const std::wstring& filePath,
 uint32_t maxWidth,
 uint32_t maxHeight
 ) = 0;
 
 // Human-readable format name (for logging/diagnostics)
 virtual std::string GetFormatName() const = 0;
 
 // List of supported extensions (lowercase, with dot)
 virtual std::vector<std::string> GetSupportedExtensions() const = 0;
};
```

---

### ThumbnailBitmap Structure

```cpp
struct ThumbnailBitmap {
 uint32_t width; // Thumbnail width in pixels
 uint32_t height; // Thumbnail height in pixels
 uint32_t stride; // Row stride in bytes (usually width * 4)
 DXGI_FORMAT format; // Pixel format (DXGI_FORMAT_R8G8B8A8_UNORM)
 std::vector<uint8_t> data; // Pixel data (RGBA, 8-bit per channel)
 
 // Helper: Get pixel at (x, y)
 uint32_t GetPixel(uint32_t x, uint32_t y) const {
 const uint8_t* pixel = &data[y * stride + x * 4];
 return (pixel[3] << 24) | (pixel[2] << 16) | (pixel[1] << 8) | pixel[0];
 }
 
 // Helper: Set pixel at (x, y)
 void SetPixel(uint32_t x, uint32_t y, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
 uint8_t* pixel = &data[y * stride + x * 4];
 pixel[0] = r;
 pixel[1] = g;
 pixel[2] = b;
 pixel[3] = a;
 }
};
```

---

### DecoderRegistry API

```cpp
class DecoderRegistry {
public:
 // Singleton access
 static DecoderRegistry& GetInstance();
 
 // Register decoder for extension (e.g., ".webp")
 void RegisterDecoder(
 const std::string& extension,
 DecoderFactory factory
 );
 
 // Get decoder for file extension
 IThumbnailDecoder* GetDecoderForExtension(const std::string& ext);
 
 // Check if extension is supported
 bool IsExtensionSupported(const std::string& ext) const;
 
 // List all registered extensions
 std::vector<std::string> GetSupportedExtensions() const;
 
 // List all decoder names
 std::vector<std::string> GetDecoderNames() const;
};
```

---

## Build System Integration

### CMakeLists.txt

```cmake
# Add custom decoder to Engine library
target_sources(ExplorerLensEngine PRIVATE
 Engine/decoders/MyCustomDecoder.cpp
 Engine/decoders/MyCustomDecoder.h
 Engine/decoders/MyCustomDecoderRegistration.cpp
)

# Link external library (if needed)
target_link_libraries(ExplorerLensEngine PRIVATE
 mycustomlib # Example: libxyz.lib
)
```

---

### As a Separate Plugin DLL (Advanced)

```cmake
# CMakeLists.txt (plugin project)
add_library(MyCustomDecoderPlugin SHARED
 src/MyCustomDecoder.cpp
 src/MyCustomDecoder.h
 src/PluginEntry.cpp
)

target_link_libraries(MyCustomDecoderPlugin PRIVATE
 ExplorerLensEngine # Link against Engine for interfaces
 mycustomlib
)

# Export plugin entry point
target_compile_definitions(MyCustomDecoderPlugin PRIVATE
 PLUGIN_EXPORT=__declspec(dllexport)
)

# Install plugin to Extensions folder
install(TARGETS MyCustomDecoderPlugin
 RUNTIME DESTINATION Extensions
)
```

**Plugin entry point:**

```cpp
// PluginEntry.cpp
#include "MyCustomDecoder.h"
#include "IDecoderPlugin.h"

extern "C" {

PLUGIN_EXPORT IDecoderPlugin* CreatePlugin() {
 return new MyCustomDecoderPlugin();
}

PLUGIN_EXPORT void DestroyPlugin(IDecoderPlugin* plugin) {
 delete plugin;
}

} // extern "C"
```

---

## Testing Your Decoder

### Unit Tests

```cpp
// test/TestMyCustomDecoder.cpp
#include "gtest/gtest.h"
#include "MyCustomDecoder.h"
#include <fstream>

TEST(MyCustomDecoderTest, CanDecodeValidFile) {
 ExplorerLens::MyCustomDecoder decoder;
 
 // Create test file
 std::ofstream file("test.xyz", std::ios::binary);
 file << "XYZ\0"; // Magic bytes
 uint32_t width = 100, height = 100;
 file.write(reinterpret_cast<const char*>(&width), 4);
 file.write(reinterpret_cast<const char*>(&height), 4);
 // ... write pixel data
 file.close();
 
 // Test
 EXPECT_TRUE(decoder.CanDecode(L"test.xyz"));
}

TEST(MyCustomDecoderTest, DecodeProducesValidBitmap) {
 ExplorerLens::MyCustomDecoder decoder;
 
 auto bitmap = decoder.Decode(L"test.xyz", 256, 256);
 
 EXPECT_GT(bitmap.width, 0);
 EXPECT_GT(bitmap.height, 0);
 EXPECT_LE(bitmap.width, 256);
 EXPECT_LE(bitmap.height, 256);
 EXPECT_EQ(bitmap.format, DXGI_FORMAT_R8G8B8A8_UNORM);
 EXPECT_EQ(bitmap.data.size(), bitmap.stride * bitmap.height);
}

TEST(MyCustomDecoderTest, RejectInvalidFile) {
 ExplorerLens::MyCustomDecoder decoder;
 
 EXPECT_FALSE(decoder.CanDecode(L"notafile.txt"));
}

TEST(MyCustomDecoderTest, ThrowOnCorruptedFile) {
 ExplorerLens::MyCustomDecoder decoder;
 
 // Create corrupted file (truncated header)
 std::ofstream file("corrupt.xyz", std::ios::binary);
 file << "XY"; // Incomplete magic
 file.close();
 
 EXPECT_THROW(
 decoder.Decode(L"corrupt.xyz", 256, 256),
 std::runtime_error
 );
}
```

---

### Integration Tests

```cpp
// test/IntegrationTests.cpp
TEST(IntegrationTest, DecoderRegistered) {
 auto& registry = ExplorerLens::DecoderRegistry::GetInstance();
 
 EXPECT_TRUE(registry.IsExtensionSupported(".xyz"));
 
 auto* decoder = registry.GetDecoderForExtension(".xyz");
 ASSERT_NE(decoder, nullptr);
 EXPECT_EQ(decoder->GetFormatName(), "MyCustomFormat");
 
 delete decoder;
}

TEST(IntegrationTest, EndToEndDecode) {
 auto& registry = ExplorerLens::DecoderRegistry::GetInstance();
 
 // Get decoder
 auto* decoder = registry.GetDecoderForExtension(".xyz");
 ASSERT_NE(decoder, nullptr);
 
 // Decode test file
 auto bitmap = decoder->Decode(L"testdata/sample.xyz", 256, 256);
 
 // Verify output
 EXPECT_GT(bitmap.width, 0);
 EXPECT_GT(bitmap.height, 0);
 
 // Save as PNG for manual inspection
 SaveAsPNG(bitmap, "output_thumbnail.png");
 
 delete decoder;
}
```

---

### Manual Testing (Explorer)

1. **Build and install:**
 ```powershell
 cmake --build build --config Release
 copy build\bin\Release\LENSShell.dll "C:\Program Files\ExplorerLens\"
 regsvr32 "C:\Program Files\ExplorerLens\LENSShell.dll"
 ```

2. **Enable format in LENSManager:**
 - Launch LENSManager.exe
 - Select "MyCustomFormat (.xyz)"
 - Click "Install"

3. **Test in Explorer:**
 - Copy test `.xyz` files to a folder
 - Open folder in Explorer
 - Switch to thumbnails view
 - Verify thumbnails appear

4. **Check logs:**
 ```powershell
 Get-Content "C:\ProgramData\ExplorerLens\Logs\Engine.log" -Tail 50
 ```

---

## Packaging & Distribution

### Option 1: Merge into Engine

**Pros:**
- No plugin loading overhead
- Simple deployment
- Single DLL

**Cons:**
- Requires Engine rebuild
- Not user-replaceable

**Method:** Add decoder to `Engine/decoders/` and rebuild

---

### Option 2: Separate Plugin DLL

**Pros:**
- Independent versioning
- Users can install/uninstall
- No Engine rebuild needed

**Cons:**
- Plugin loading complexity
- Version compatibility issues

**Directory structure:**
```
C:\Program Files\ExplorerLens\
├── LENSShell.dll
├── ExplorerLensEngine.lib
└── Extensions\
 ├── MyCustomDecoderPlugin.dll
 └── mycustomlib.dll (dependency)
```

---

### Distribution Checklist

- [ ] Test on clean Windows installation
- [ ] Include VC++ Redistributable (or static link)
- [ ] Sign binaries with code signing certificate
- [ ] Provide uninstaller (remove registry keys)
- [ ] Document supported file format versions
- [ ] Include sample files for testing
- [ ] Write user guide with screenshots

---

## Advanced Topics

### GPU-Accelerated Decoding

**Using Direct3D 11 for image processing:**

```cpp
#include "D3D11Renderer.h"

ThumbnailBitmap MyCustomDecoder::Decode(...) {
 // ... decode to full-size bitmap
 
 // Use GPU renderer for resizing
 D3D11Renderer& renderer = D3D11Renderer::GetInstance();
 
 // Upload full-size image to GPU texture
 ID3D11Texture2D* fullTexture = renderer.CreateTexture(
 imageWidth, imageHeight,
 pixelData.data()
 );
 
 // Resize on GPU (bilinear filter)
 ID3D11Texture2D* thumbTexture = renderer.Resize(
 fullTexture, thumbWidth, thumbHeight,
 D3D11_FILTER_MIN_MAG_MIP_LINEAR
 );
 
 // Download thumbnail from GPU
 std::vector<uint8_t> thumbData = renderer.DownloadTexture(thumbTexture);
 
 // Clean up
 fullTexture->Release();
 thumbTexture->Release();
 
 // Return result
 ThumbnailBitmap result;
 result.width = thumbWidth;
 result.height = thumbHeight;
 result.data = std::move(thumbData);
 return result;
}
```

---

### Multi-Page Documents

**Example: TIFF with multiple pages**

```cpp
ThumbnailBitmap TIFFDecoder::Decode(...) {
 // Open TIFF
 TIFF* tif = TIFFOpen(filePath.c_str(), "r");
 
 // Default: Decode first page
 uint32_t pageIndex = 0;
 
 // Check registry for override
 DWORD preferredPage = GetRegistryDWORD("TIFFPageIndex", 0);
 if (preferredPage < TIFFNumberOfDirectories(tif)) {
 pageIndex = preferredPage;
 }
 
 // Seek to page
 TIFFSetDirectory(tif, pageIndex);
 
 // Decode page...
 
 TIFFClose(tif);
 return bitmap;
}
```

---

### Animated Formats (GIF, WebP)

**Extract first frame only:**

```cpp
ThumbnailBitmap AnimatedDecoder::Decode(...) {
 // Decode all frames
 std::vector<Frame> frames = DecodeAllFrames(filePath);
 
 if (frames.empty()) {
 throw std::runtime_error("No frames in file");
 }
 
 // Return first frame as thumbnail
 return frames[0].bitmap;
}
```

**Or create montage:**

```cpp
ThumbnailBitmap AnimatedDecoder::Decode(...) {
 std::vector<Frame> frames = DecodeAllFrames(filePath);
 
 // Create 2x2 grid of first 4 frames
 ThumbnailBitmap montage = CreateMontage(
 frames, 2, 2, thumbWidth, thumbHeight
 );
 
 return montage;
}
```

---

### Error Handling Best Practices

```cpp
ThumbnailBitmap MyDecoder::Decode(...) {
 try {
 // Attempt decode
 return DecodeInternal(filePath, maxWidth, maxHeight);
 }
 catch (const std::bad_alloc&) {
 // Out of memory - log and rethrow
 Logger::GetInstance().LogError("OOM decoding " + filePath);
 throw DecoderException("Out of memory");
 }
 catch (const std::exception& e) {
 // Generic error - log details
 Logger::GetInstance().LogError(
 "Decode failed: " + std::string(e.what())
 );
 throw DecoderException("Decode error: " + std::string(e.what()));
 }
 catch (...) {
 // Unknown error - log and rethrow
 Logger::GetInstance().LogError("Unknown error decoding " + filePath);
 throw DecoderException("Unknown decode error");
 }
}
```

---

## Example: Complete Decoder

### Minimal Decoder Template

```cpp
// MinimalDecoder.h
#pragma once
#include "IThumbnailDecoder.h"

namespace ExplorerLens {

class MinimalDecoder : public IThumbnailDecoder {
public:
 bool CanDecode(const std::wstring& filePath) const override {
 // Check extension
 return filePath.ends_with(L".min");
 }
 
 ThumbnailBitmap Decode(
 const std::wstring& filePath,
 uint32_t maxWidth,
 uint32_t maxHeight
 ) override {
 // Create solid color thumbnail (placeholder)
 ThumbnailBitmap result;
 result.width = std::min(maxWidth, 256u);
 result.height = std::min(maxHeight, 256u);
 result.stride = result.width * 4;
 result.format = DXGI_FORMAT_R8G8B8A8_UNORM;
 result.data.resize(result.stride * result.height);
 
 // Fill with blue color
 for (uint32_t y = 0; y < result.height; ++y) {
 for (uint32_t x = 0; x < result.width; ++x) {
 result.SetPixel(x, y, 0, 0, 255, 255); // Blue
 }
 }
 
 return result;
 }
 
 std::string GetFormatName() const override {
 return "MinimalFormat";
 }
 
 std::vector<std::string> GetSupportedExtensions() const override {
 return {".min"};
 }
};

// Auto-register
static bool s_minimalRegistered = []() {
 DecoderRegistry::GetInstance().RegisterDecoder(
 ".min",
 []() -> IThumbnailDecoder* { return new MinimalDecoder(); }
 );
 return true;
}();

} // namespace ExplorerLens
```

---

**Next Steps:**
- Study existing decoders in `Engine/decoders/`
- Refer to [Architecture Documentation](ARCHITECTURE.md)
- Join developer discussions on GitHub

---

**Last Updated:** February 18, 2026 
**Version:** 7.1.0
