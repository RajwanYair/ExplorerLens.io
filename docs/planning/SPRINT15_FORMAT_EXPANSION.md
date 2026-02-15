# Sprint 15: Format Support Expansion

**Sprint Duration**: Weeks 19-22 (February 9 - March 8, 2026)  
**Status**: 🚀 STARTING (February 9, 2026)  
**Target**: Expand from 31 to 50+ supported formats

---

## 🎯 Objectives

Complete the format support expansion to achieve **50+ file formats** by implementing:

1. **RAW Camera Formats** (LibRaw) - 15+ formats
2. **HEIF/HEIC Complete Implementation** - Apple photos
3. **JXL Complete Implementation** - JPEG XL
4. **Advanced Image Formats** - PSD, DDS, ICO, SVG
5. **HDR/Professional Formats** - EXR, HDR, TGA
6. **Legacy Formats** - JPEG2000, QOI

---

## 📊 Current Status

### Implemented (31 formats)
- **WIC-Based**: JPEG, PNG, GIF, BMP, TIFF (basic)
- **Modern**: WebP, AVIF (✅ Sprint 13)
- **Archives**: ZIP, RAR, 7Z, TAR, CBZ, CBR, CB7, CBT
- **eBooks**: EPUB
- **Videos**: Basic thumbnails

### In Progress (Stub/Header Only)
- **HEIF/HEIC**: Header complete, needs WIC implementation
- **JXL**: Header complete, needs libjxl integration
- **RAW**: Build script ready, needs decoder implementation

---

## 🗂️ Phase 1: RAW Camera Formats (Week 19)

### **Priority**: 🔴 CRITICAL
Professional photography, enthusiast cameras, DNG format from modern phones

### Implementation Plan

#### Step 1: Build LibRaw (Day 1)
```powershell
.\build-scripts\Build-LibRaw.ps1 -Clean
```

**Expected Output**:
- `external\libraw-install\lib\libraw.lib` (~2-3 MB)
- `external\libraw-install\include\libraw\` (headers)

**Supported Formats** (15+ extensions):
- Canon: `.cr2`, `.cr3`, `.crw`
- Nikon: `.nef`, `.nrw`
- Sony: `.arw`, `.srf`, `.sr2`
- Olympus: `.orf`
- Panasonic: `.rw2`, `.raw`
- Fujifilm: `.raf`
- Pentax: `.pef`, `.ptx`
- Adobe: `.dng` (universal RAW)
- Leica: `.rwl`, `.dng`

#### Step 2: Create RAWDecoder (Day 2-3)

**File**: `Engine/Decoders/RAWDecoder.h`
```cpp
#pragma once
#include "IThumbnailDecoder.h"

namespace DarkThumbs {

class RAWDecoder : public IThumbnailDecoder {
public:
    RAWDecoder();
    ~RAWDecoder() override;

    // IThumbnailDecoder implementation
    bool CanDecode(const std::wstring& filePath) override;
    DecoderResult Decode(
        const std::wstring& filePath,
        uint32_t targetWidth,
        uint32_t targetHeight,
        std::vector<uint8_t>& pixelData,
        uint32_t& actualWidth,
        uint32_t& actualHeight
    ) override;

    std::wstring GetDecoderName() const override { return L"RAWDecoder (LibRaw 0.21.2)"; }
    std::vector<std::wstring> GetSupportedExtensions() const override;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace DarkThumbs
```

**File**: `Engine/Decoders/RAWDecoder.cpp`

**Key Implementation Points**:
1. Use LibRaw C++ API (`LibRaw` class)
2. Extract embedded thumbnail first (fast path - ~5ms)
3. Fall back to full decode if thumbnail missing (slow path - ~50-200ms)
4. Handle DCB demosaicing for quality
5. Apply white balance and exposure
6. Convert to sRGB color space
7. Handle rotations from EXIF

**Dependencies**:
```cmake
# Engine/CMakeLists.txt
find_package(LibRaw REQUIRED)
target_link_libraries(DarkThumbsEngine PRIVATE LibRaw::LibRaw)
```

#### Step 3: CMake Configuration (Day 3)

**File**: `CMakeLists.txt` (root)
```cmake
# Find LibRaw
set(LibRaw_DIR "${CMAKE_SOURCE_DIR}/external/libraw-install/lib/cmake/LibRaw")
find_package(LibRaw QUIET)

if(LibRaw_FOUND)
    message(STATUS "✅ LibRaw found: ${LibRaw_VERSION}")
    set(ENABLE_RAW_SUPPORT ON)
else()
    message(STATUS "⚠️  LibRaw not found - RAW format support disabled")
    set(ENABLE_RAW_SUPPORT OFF)
endif()
```

**Configure Flag**:
```cmake
# Engine/CMakeLists.txt
if(ENABLE_RAW_SUPPORT)
    target_sources(DarkThumbsEngine PRIVATE
        Decoders/RAWDecoder.cpp
        Decoders/RAWDecoder.h
    )
    target_compile_definitions(DarkThumbsEngine PRIVATE ENABLE_RAW_SUPPORT)
    target_link_libraries(DarkThumbsEngine PRIVATE LibRaw::LibRaw)
endif()
```

#### Step 4: Testing (Day 3-4)

**Test Files Required**:
- Canon CR2/CR3 samples
- Nikon NEF samples
- Sony ARW samples
- DNG samples (Pixel/iPhone)

**Test Cases**:
1. Embedded thumbnail extraction (speed test)
2. Full RAW decode (quality test)
3. EXIF orientation handling
4. White balance application
5. Exposure adjustment
6. Various camera models

**Performance Target**:
- Embedded thumbnail: < 10ms
- Full decode: < 200ms
- Cache hit: < 1ms

---

## 🍎 Phase 2: HEIF/HEIC Native Implementation (Week 20)

### **Priority**: 🔴 HIGH  
Apple's default format for iPhone/iPad photos since iOS 11

### Implementation Strategy: WIC-Based

**Advantage**: Windows 11 includes native HEIF codec support via WIC  
**No External Library Needed**: Use existing WIC infrastructure

#### Step 1: Update Existing WIC Decoder (Day 1)

**File**: `CBXShell/heif_decoder_native.cpp` (already exists - 194 lines)

**Current Status**: Complete, using WIC  
**Action**: Verify and optimize

**Test Registration**:
```cpp
// Verify WIC HEIF codec installed
IWICImagingFactory* factory = GetWICFactory();
IWICBitmapDecoder* decoder = nullptr;
HRESULT hr = factory->CreateDecoderFromFilename(
    L"test.heic",
    nullptr,
    GENERIC_READ,
    WICDecodeMetadataCacheOnDemand,
    &decoder
);
// hr == S_OK means HEIF codec installed
```

#### Step 2: Feature Validation (Day 2)

**Test Coverage**:
- Single image HEIC
- HEIF sequences
- AVCI/AVCS variants
- HDR images (tone mapping)
- Embedded thumbnails

**Windows 11 Support Matrix**:
| Format | Extension | WIC Support |
|--------|-----------|-------------|
| HEIF Image | .heif | ✅ Yes |
| HEIC Image | .heic | ✅ Yes |
| HIF | .hif | ✅ Yes |
| AVCI | .avci | ✅ Yes |
| AVCS | .avcs | ✅ Yes |
| HEIF Sequence | .heif-sequence | ⚠️ First frame only |

#### Step 3: Performance Optimization (Day 2-3)

**Optimizations**:
1. Use `WICBitmapCacheOnDemand` for faster loading
2. Extract embedded thumbnail when available
3. Downscale during decode (not after)
4. Hardware acceleration via WIC

**Benchmark Target**:
- Small HEIC (< 5MB): < 50ms
- Large HEIC (> 10MB): < 150ms
- With thumbnail: < 20ms

---

## 🎨 Phase 3: JPEG XL Complete Implementation (Week 20)

### **Priority**: 🟡 MEDIUM-HIGH  
Modern, efficient format with excellent compression

#### Step 1: Verify libjxl Build (Day 1)

**File**: `CBXShell/jxl_decoder.cpp` (already exists - 292 lines)

**Current Status**: Complete implementation using libjxl 0.11.1  
**Action**: Integration with Engine

#### Step 2: Engine Integration (Day 2)

**Create**: `Engine/Decoders/JXLDecoder.h/.cpp`

**Wrap Existing Implementation**:
```cpp
// Use CBXShell/jxl_decoder.cpp as reference
#include "IThumbnailDecoder.h"

class JXLDecoder : public IThumbnailDecoder {
    // Delegate to libjxl 0.11.1
    // Support both naked (0xFF 0x0A) and container formats
    // Use parallel runner for speed
};
```

**Dependencies**:
```cmake
target_link_libraries(DarkThumbsEngine PRIVATE
    jxl::jxl
    jxl::jxl_threads
)
```

#### Step 3: Testing (Day 3)

**Test Files**:
- Naked JXL (0xFF 0x0A signature)
- Container JXL (ftyp box)
- Progressive JXL
- Animated JXL (first frame)

---

## 🖼️ Phase 4: Professional Formats (Week 21)

### PSD (Photoshop) - Priority 🟡 MEDIUM

**Library**: Custom parser (PSD format is well-documented)  
**Effort**: 2 days  
**Extensions**: `.psd`, `.psb`

**Implementation**:
1. Read composite image (merged layer data)
2. Extract layer preview if composite missing
3. Handle 8/16/32-bit depth
4. Support large documents (.psb)

### DDS (DirectX Texture) - Priority 🟢 LOW-MEDIUM

**Library**: DirectXTex  
**Effort**: 2 days  
**Extensions**: `.dds`

**Implementation**:
1. Link DirectXTex (already built for DirectX 11 integration)
2. Handle BC1-BC7 compression
3. Extract mip level 0
4. Convert to RGBA

### ICO (Icons) - Priority 🟢 LOW

**Library**: Native implementation  
**Effort**: 1 day  
**Extensions**: `.ico`, `.cur`

**Implementation**:
1. Parse ICO header structure
2. Select best resolution (closest to target size)
3. Handle PNG-compressed icons (Vista+)
4. Extract with alpha channel

### SVG (Vector Graphics) - Priority 🟢 LOW

**Library**: NanoSVG (single-header)  
**Effort**: 1 day  
**Extensions**: `.svg`, `.svgz`

**Implementation**:
1. Integrate NanoSVG rasterizer
2. Render at target resolution
3. Handle SVGZ (gzip-compressed SVG)
4. Basic CSS support

---

## 🌅 Phase 5: HDR & Professional (Week 22)

### HDR (Radiance RGBE) - Priority 🟢 LOW-MEDIUM

**Library**: Native implementation (simple format)  
**Effort**: 1 day  
**Extensions**: `.hdr`, `.rgbe`, `.pic`

**Features**:
- Read Radiance RGBE format
- Tone mapping to LDR
- Exposure adjustment

### TGA (Targa) - Priority 🟢 LOW

**Library**: Native implementation  
**Effort**: 1 day  
**Extensions**: `.tga`, `.targa`

**Features**:
- RLE and uncompressed
- 16/24/32-bit color
- Alpha channel support

### JPEG2000 - Priority 🟢 LOW

**Library**: OpenJPEG  
**Effort**: 2 days  
**Extensions**: `.jp2`, `.j2k`, `.jpf`, `.jpx`

**Features**:
- Decode to RGBA
- Handle progressive
- Multi-resolution support

### QOI (Quite OK Image) - Priority 🟢 LOW

**Library**: Single-header (qoi.h)  
**Effort**: 0.5 days  
**Extensions**: `.qoi`

**Features**:
- Fast decode
- Lossless
- Simple integration

---

## 📋 Implementation Checklist

### Week 19: RAW Formats
- [ ] Build LibRaw 0.21.2
- [ ] Create RAWDecoder.h/.cpp
- [ ] Implement embedded thumbnail extraction
- [ ] Implement full RAW decode
- [ ] CMake configuration
- [ ] Unit tests (5+ camera models)
- [ ] Performance benchmarks
- [ ] Update DecoderRegistry

### Week 20: HEIF/HEIC + JXL
- [ ] Verify HEIF WIC codec on Windows 11
- [ ] Test heif_decoder_native.cpp
- [ ] Optimize HEIF performance
- [ ] Integrate JXL decoder with Engine
- [ ] Test JXL formats (naked + container)
- [ ] Performance benchmarks

### Week 21: Professional Formats
- [ ] Implement PSD decoder
- [ ] Integrate DirectXTex for DDS
- [ ] Implement ICO decoder
- [ ] Integrate NanoSVG

### Week 22: HDR & Completion
- [ ] Implement HDR/RGBE decoder
- [ ] Implement TGA decoder
- [ ] Integrate OpenJPEG for JPEG2000
- [ ] Integrate QOI decoder
- [ ] Final testing & documentation
- [ ] Update FORMAT_SUPPORT_ANALYSIS.md

---

## 🎯 Success Criteria

### Functional Requirements
- ✅ All decoders implement IThumbnailDecoder interface
- ✅ Registration with DecoderRegistry
- ✅ Zero crashes on malformed files
- ✅ Proper error handling and fallback

### Performance Requirements
- ✅ RAW embedded thumbnail: < 10ms
- ✅ RAW full decode: < 200ms
- ✅ HEIF: < 50ms (avg)
- ✅ JXL: < 100ms (avg)
- ✅ All others: < 50ms (avg)

### Quality Requirements
- ✅ Correct color space conversion
- ✅ Alpha channel preserved
- ✅ EXIF orientation respected
- ✅ High-quality downscaling

### Coverage Requirements
- ✅ **50+ formats supported** (target)
- ✅ 15+ RAW camera formats
- ✅ 6+ HEIF variants
- ✅ All professional formats (PSD, DDS, EXR, etc.)

---

## 📈 Progress Tracking

**Current**: 31 formats  
**Target**: 50+ formats  
**Remaining**: 19+ formats

### Format Count by Phase
- Phase 1 (RAW): +15 formats → **46 total**
- Phase 2 (HEIF): +6 formats → **52 total** ✅ TARGET MET
- Phase 3 (JXL): +1 format → **53 total**
- Phase 4 (Professional): +4 formats → **57 total**
- Phase 5 (HDR/Legacy): +5 formats → **62 total**

---

## 🔗 Related Documents

- [FORMAT_SUPPORT_ANALYSIS.md](FORMAT_SUPPORT_ANALYSIS.md) - Detailed format analysis
- [SPRINT14_PLUGIN_SECURITY.md](SPRINT14_PLUGIN_SECURITY.md) - Previous sprint (Security)
- [ROADMAP.md](../ROADMAP.md) - Overall project roadmap
- [Build-LibRaw.ps1](../build-scripts/Build-LibRaw.ps1) - LibRaw build script

---

**Created**: February 9, 2026  
**Sprint Owner**: DarkThumbs Development Team  
**Target Completion**: March 8, 2026 (4 weeks)
