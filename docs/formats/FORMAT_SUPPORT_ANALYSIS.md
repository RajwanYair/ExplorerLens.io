# ⚠️ ARCHIVED — DarkThumbs Format Support Analysis & Implementation Plan

> **This document is archived as of v8.4.0 (Sprint 178).** It was written for v5.3.0 and is
> severely outdated. Many formats listed as "Not Yet Implemented" have since been implemented.
> See **FORMAT_SUPPORT_MATRIX_V8.md** for the current format matrix.

**Generated**: February 5, 2026  
**Version**: 5.3.0  
**Archived**: June 2025 (v8.4.0 Sprint 178)

---

## Current Format Support Status

### ✅ Fully Implemented (Tested & Working)

#### WIC-Based Formats (ImageDecoder)
- **JPEG** (.jpg, .jpeg) - Via Windows Imaging Component
- **PNG** (.png) - Via WIC with transparency support
- **GIF** (.gif) - First frame extraction via WIC
- **BMP** (.bmp) - Via WIC (could be optimized with native decoder)
- **TIFF** (.tif, .tiff) - WIC supports basic TIFF (first page only)

#### Modern Image Formats
- **WebP** (.webp) - ✅ WebPDecoder.cpp (libwebp 1.5.0)
- **AVIF** (.avif) - ✅ AVIFDecoder.cpp (libavif 1.3.0 + dav1d 1.5.1)

#### Archive Formats
- **ZIP** (.zip, .cbz) - ✅ ArchiveDecoder.cpp (minizip-ng 4.0.10)
- **RAR** (.rar, .cbr) - ✅ Via unrar library
- **7-Zip** (.7z, .cb7) - ✅ Via LZMA SDK
- **TAR** (.tar, .cbt) - ✅ Via libarchive

---

## ⚠️ Partially Implemented (Interface Defined, Implementation Stub)

### HEIF/HEIC Format
**Status**: Header complete, implementation stub  
**Files**: 
- `Engine/Decoders/HEIFDecoder.h` ✅ (interface complete)
- `Engine/Decoders/HEIFDecoder.cpp` ⚠️ (stub - needs libheif integration)
- `CBXShell/heif_decoder_native.h/cpp` (legacy code)

**Extensions**: .heif, .heic, .hif, .heifs, .heics, .avci, .avcs, .heif-sequence, .heic-sequence

**Library Required**: **libheif** (not yet built)

**Priority**: 🔴 **HIGH** - Apple's default photo format (iPhone/iPad), widely used

**Implementation Tasks**:
1. Build libheif library with x265/x264 decoder support
2. Link libheif.lib in CMakeLists.txt
3. Implement `HEIFDecoder::DecodeHEIFImage()`
4. Add embedded thumbnail extraction (fast path)
5. Support HDR tone mapping (optional)
6. Uncomment in Engine/CMakeLists.txt
7. Add unit tests
8. Test with real iPhone photos

**Estimated Effort**: 2-3 days

---

### JXL (JPEG XL) Format
**Status**: Header complete, implementation stub  
**Files**:
- `Engine/Decoders/JXLDecoder.h` ✅ (interface complete)
- `Engine/Decoders/JXLDecoder.cpp` ⚠️ (stub - needs libjxl integration)
- `CBXShell/jxl_decoder.h/cpp` (legacy code)

**Extensions**: .jxl

**Library Required**: **libjxl** (built: 0.11.1, not linked)

**Dependencies**: 
- brotli (for JXL decoding)
- highway (SIMD library used by libjxl)

**Priority**: 🟡 **MEDIUM-HIGH** - Modern format with excellent compression

**Implementation Tasks**:
1. Verify libjxl 0.11.1 build is complete
2. Build brotli and highway dependencies
3. Link jxl.lib, jxl_threads.lib in CMakeLists.txt
4. Implement `JXLDecoder::DecodeJXLImage()` using libjxl C++ API
5. Support progressive decoding
6. Support animation (optional)
7. Uncomment in Engine/CMakeLists.txt
8. Add unit tests
9. Test with sample JXL files

**Estimated Effort**: 2-3 days

---

## ❌ Not Yet Implemented (High Priority)

### TIFF Format (Advanced Support)
**Current**: Basic TIFF via WIC (first page only)  
**Needed**: Full multi-page TIFF support with advanced features

**Library Required**: **libtiff** (not built)

**Features to Add**:
- Multi-page TIFF support
- Big TIFF (>4GB) support
- TIFF compression variants (LZW, ZIP, JPEG, etc.)
- 16-bit and floating-point TIFF
- Tiled TIFF support
- EXIF/IPTC metadata preservation

**Priority**: 🟡 **MEDIUM** - Professional photography/scanning format

**Implementation Tasks**:
1. Build libtiff library
2. Create `TIFFDecoder.h/.cpp` implementing IThumbnailDecoder
3. Support multi-page extraction
4. Add page selection UI in CBXManager
5. Link libtiff.lib in CMakeLists.txt
6. Add to Engine/CMakeLists.txt
7. Unit tests for various TIFF variants

**Estimated Effort**: 2-3 days

---

### ICO Format (Native Decoder)
**Current**: ICO via GDI+ (basic support)  
**Needed**: Native high-quality ICO decoder

**Extensions**: .ico, .cur (cursor)

**Library**: Can be implemented natively (ICO format is simple)

**Features**:
- Multi-resolution icon extraction (16x16, 32x32, 48x48, 256x256)
- PNG-compressed icons (Vista+)
- Alpha channel support
- Best size selection

**Priority**: 🟢 **LOW-MEDIUM** - Common format but GDI+ works

**Implementation Tasks**:
1. Create `ICODecoder.h/.cpp`
2. Implement ICO/CUR header parsing
3. Extract best resolution for thumbnail size
4. Support PNG-compressed icon entries
5. Add to CMakeLists.txt
6. Unit tests

**Estimated Effort**: 1 day

---

### DDS Format (DirectX Texture)
**Current**: Defined (IMGTYPE_DDS 14) but not implemented  
**Needed**: Native DDS decoder with GPU acceleration

**Extensions**: .dds

**Use Cases**: Game textures, 3D content

**Library Options**:
- DirectXTex (Microsoft, recommended)
- Native implementation (format is documented)

**Features**:
- All DDS formats (DXT1/3/5, BC1-7, RGBA, etc.)
- Cubemaps (select face)
- Volume textures (select slice)
- Mipmap chain (select level)
- GPU texture upload (zero-copy)

**Priority**: 🟡 **MEDIUM** - Gaming/3D content

**Implementation Tasks**:
1. Build DirectXTex library OR implement natively
2. Create `DDSDecoder.h/.cpp`
3. Parse DDS header (DDS_HEADER, DDS_HEADER_DXT10)
4. Decompress compressed formats
5. Add GPU path (upload texture directly)
6. Unit tests with various DDS formats

**Estimated Effort**: 2-3 days

---

### TGA Format (Targa)
**Extensions**: .tga, .targa

**Use Cases**: Game development, legacy graphics

**Library**: Can be implemented natively (simple format)

**Features**:
- RLE compression support
- 8/16/24/32-bit support
- Alpha channel
- Color map support

**Priority**: 🟢 **LOW-MEDIUM** - Less common but easy to implement

**Implementation Tasks**:
1. Create `TGADecoder.h/.cpp`
2. Implement TGA header parsing
3. Implement RLE decompression
4. Support all bit depths
5. Add to CMakeLists.txt
6. Unit tests

**Estimated Effort**: 1 day

---

### PSD Format (Photoshop)
**Extensions**: .psd, .psb (large document)

**Library Required**: Custom implementation or third-party

**Features**:
- Composite image extraction (flattened preview)
- Layer thumbnail extraction (optional)
- 8/16/32-bit per channel
- CMYK support (convert to RGB)

**Priority**: 🟡 **MEDIUM** - Professional content creation

**Challenges**: Complex format, proprietary

**Implementation Options**:
1. Extract composite/preview image only (easier)
2. Full layer support (complex)

**Implementation Tasks**:
1. Research PSD format specification
2. Create `PSDDecoder.h/.cpp`
3. Parse PSD header and composite image
4. Convert CMYK to RGB if needed
5. Extract preview/composite bitmap
6. Unit tests

**Estimated Effort**: 3-5 days

---

### EXR Format (OpenEXR HDR)
**Extensions**: .exr

**Library Required**: **OpenEXR** (ILM library)

**Use Cases**: VFX, HDR photography, 3D rendering

**Features**:
- High dynamic range
- 16-bit float (half), 32-bit float
- Multi-channel images
- Tile-based and scanline-based
- Compression (ZIP, PIZ, PXR24, etc.)

**Priority**: 🟢 **LOW-MEDIUM** - Specialized professional use

**Implementation Tasks**:
1. Build OpenEXR library
2. Create `EXRDecoder.h/.cpp`
3. Implement HDR tone mapping for display
4. Support multi-part EXR (select part)
5. Link OpenEXR libs
6. Unit tests

**Estimated Effort**: 2-3 days

---

### HDR Format (Radiance RGBE)
**Extensions**: .hdr, .pic, .rgbe

**Library**: Can be implemented natively (format is simple)

**Features**:
- Radiance RGBE encoding
- High dynamic range
- Tone mapping for display

**Priority**: 🟢 **LOW** - Niche format

**Implementation Tasks**:
1. Create `HDRDecoder.h/.cpp`
2. Implement RGBE decoding
3. Add tone mapping (Reinhard, ACES, etc.)
4. Unit tests

**Estimated Effort**: 1 day

---

### QOI Format (Quite OK Image)
**Extensions**: .qoi

**Library**: Trivial to implement (single-header library available)

**Features**:
- Lossless compression
- Fast encoding/decoding
- Very simple format (300 lines of C)

**Priority**: 🟢 **LOW** - New format, not widely adopted yet

**Implementation Tasks**:
1. Include qoi.h single-header library
2. Create `QOIDecoder.h/.cpp` wrapper
3. Unit tests

**Estimated Effort**: 0.5 days

---

### JPEG2000 Format
**Extensions**: .jp2, .j2k, .j2c, .jpc

**Library Required**: **OpenJPEG** (open-source)

**Features**:
- Superior compression vs JPEG
- Lossless and lossy
- Progressive decoding
- Region of interest

**Priority**: 🟢 **LOW-MEDIUM** - Medical imaging, digital cinema

**Implementation Tasks**:
1. Build OpenJPEG library
2. Create `JPEG2000Decoder.h/.cpp`
3. Implement decoding pipeline
4. Link openjpeg libs
5. Unit tests

**Estimated Effort**: 2 days

---

### SVG Format (Vector Graphics)
**Extensions**: .svg, .svgz (compressed)

**Library Options**:
- **NanoSVG** (lightweight, rasterizes to bitmap)
- **cairo** + **librsvg** (full-featured)
- **Skia** (Google's graphics library)

**Priority**: 🟡 **MEDIUM** - Common for icons, web graphics

**Challenges**: Needs rasterization to bitmap

**Implementation Tasks**:
1. Evaluate library (NanoSVG recommended for simplicity)
2. Build selected library
3. Create `SVGDecoder.h/.cpp`
4. Implement rasterization at target size
5. Link libraries
6. Unit tests

**Estimated Effort**: 2-3 days

---

### RAW Camera Formats
**Extensions**: .cr2, .cr3, .nef, .arw, .orf, .dng, .rw2, .raw, .raf

**Library Required**: **libraw** (dcraw alternative)

**Features**:
- Camera RAW decoding
- Embedded JPEG extraction (fast path)
- Full RAW processing (slow path)
- Auto white balance
- Exposure adjustment

**Priority**: 🟡 **MEDIUM-HIGH** - Photography enthusiasts

**Implementation Tasks**:
1. Build libraw library
2. Create `RAWDecoder.h/.cpp`
3. Implement embedded preview extraction (fast)
4. Implement full RAW decode (slow, optional)
5. Link libraw
6. Unit tests with various camera RAW files

**Estimated Effort**: 2-3 days

---

## 📊 Format Priority Matrix

| Format | Priority | Complexity | Library | Effort | Impact |
|--------|----------|-----------|---------|--------|--------|
| **HEIF/HEIC** | 🔴 HIGH | Medium | libheif | 2-3d | High (Apple ecosystem) |
| **JXL** | 🟡 MED-HIGH | Medium | libjxl | 2-3d | Medium (future-proof) |
| **TIFF (full)** | 🟡 MEDIUM | Medium | libtiff | 2-3d | Medium (pro use) |
| **RAW** | 🟡 MED-HIGH | Medium | libraw | 2-3d | High (photographers) |
| **DDS** | 🟡 MEDIUM | Medium | DirectXTex | 2-3d | Medium (gaming) |
| **PSD** | 🟡 MEDIUM | High | Custom | 3-5d | Medium (designers) |
| **SVG** | 🟡 MEDIUM | Medium | NanoSVG | 2-3d | Medium (web/icons) |
| **EXR** | 🟢 LOW-MED | Medium | OpenEXR | 2-3d | Low (VFX niche) |
| **ICO** | 🟢 LOW-MED | Low | Native | 1d | Low (but common) |
| **TGA** | 🟢 LOW-MED | Low | Native | 1d | Low (legacy) |
| **JPEG2000** | 🟢 LOW-MED | Medium | OpenJPEG | 2d | Low (medical) |
| **HDR** | 🟢 LOW | Low | Native | 1d | Low (niche) |
| **QOI** | 🟢 LOW | Very Low | Single-header | 0.5d | Low (new) |

---

## 🎯 Recommended Implementation Order

### Phase 1: Complete Modern Formats (Sprint 13, Week 1-2)
1. **HEIF/HEIC** - High priority, widely used
2. **JXL** - Library ready, just needs linking
3. **RAW** - Photography support critical

**Outcome**: Support all major modern image formats

---

### Phase 2: Professional Formats (Sprint 14, Week 1-2)
4. **TIFF (full)** - Multi-page support
5. **PSD** - Photoshop compatibility
6. **DDS** - Gaming/3D content

**Outcome**: Professional workflow support

---

### Phase 3: Vector & HDR (Sprint 14, Week 3-4)
7. **SVG** - Vector graphics
8. **EXR** - HDR/VFX
9. **HDR** - Radiance format

**Outcome**: Vector and HDR support

---

### Phase 4: Legacy & Niche (Sprint 15+)
10. **ICO** - Icon files
11. **TGA** - Targa format
12. **JPEG2000** - Medical imaging
13. **QOI** - Quick format

**Outcome**: Comprehensive format coverage

---

## 📦 Library Build Requirements

### Immediate Needs (Sprint 13)
- [ ] **libheif** (HEIF/HEIC decoder)
  - Dependencies: libde265 or x265/x264
  - Build script needed
  
- [ ] **brotli** (JXL dependency)
  - Simple build with CMake
  
- [ ] **highway** (JXL SIMD dependency)
  - CMake build
  
- [ ] **libraw** (RAW camera formats)
  - Large library, needs careful build

### Future Needs
- [ ] **libtiff** (advanced TIFF)
- [ ] **DirectXTex** (DDS textures)
- [ ] **NanoSVG** (SVG rasterization)
- [ ] **OpenEXR** (EXR HDR)
- [ ] **OpenJPEG** (JPEG2000)

---

## 🎨 Format Support Summary (Target: 50+)

### Current: ~31 formats
- Archives: 4 (ZIP, RAR, 7z, TAR)
- Comic books: 4 (CBZ, CBR, CB7, CBT)
- Images (WIC): 5 (JPEG, PNG, BMP, GIF, TIFF)
- Modern images: 2 (WebP, AVIF)
- Videos: ~15+ (via DirectShow)

### After Phase 1: ~40+ formats
+ HEIF/HEIC (9 extensions)
+ JXL (1)
+ RAW (10+ camera formats)

### After Phase 2: ~45+ formats
+ TIFF (enhanced)
+ PSD (2 extensions)
+ DDS (1)

### After Phase 3: ~48+ formats
+ SVG (2 extensions)
+ EXR (1)
+ HDR (3 extensions)

### After Phase 4: ~53+ formats
+ ICO (2 extensions)
+ TGA (2 extensions)
+ JPEG2000 (4 extensions)
+ QOI (1)

---

## 🚀 Quick Start: Enabling HEIF and JXL

### Step 1: Build libheif
```powershell
cd external
git clone https://github.com/strukturag/libheif.git
cd libheif
mkdir build && cd build
cmake -G "Visual Studio 18 2026" -A x64 ..
cmake --build . --config Release
```

### Step 2: Build brotli and highway for JXL
```powershell
cd external
git clone https://github.com/google/brotli.git
cd brotli && mkdir build && cd build
cmake -G "Visual Studio 18 2026" -A x64 ..
cmake --build . --config Release

cd ../../
git clone https://github.com/google/highway.git
cd highway && mkdir build && cd build
cmake -G "Visual Studio 18 2026" -A x64 ..
cmake --build . --config Release
```

### Step 3: Update Engine/CMakeLists.txt
Uncomment:
```cmake
Decoders/JXLDecoder.h
Decoders/HEIFDecoder.h
Decoders/JXLDecoder.cpp
Decoders/HEIFDecoder.cpp
```

### Step 4: Link libraries
```cmake
target_link_libraries(DarkThumbsEngine PRIVATE
    heif
    jxl
    jxl_threads
    brotlicommon
    brotlidec
    hwy
)
```

### Step 5: Implement decode functions
See existing WebPDecoder.cpp and AVIFDecoder.cpp as reference.

### Step 6: Test
```bash
EngineTests.exe
```

---

## 📋 Summary

**Current Status**:
- ✅ 31+ formats supported
- ⚠️ HEIF/HEIC partially done (needs libheif)
- ⚠️ JXL partially done (needs linking)
- ❌ Many professional formats missing

**Recommended Action**:
1. **Complete HEIF** (2-3 days) - HIGH PRIORITY
2. **Complete JXL** (2-3 days) - HIGH PRIORITY
3. **Add RAW support** (2-3 days) - HIGH PRIORITY
4. Then proceed with professional formats (TIFF, PSD, DDS, SVG)

**Target**: **50+ formats** by end of Sprint 14

---

**Document Version**: 1.0  
**Last Updated**: February 5, 2026  
**Next Review**: After Sprint 13 completion
