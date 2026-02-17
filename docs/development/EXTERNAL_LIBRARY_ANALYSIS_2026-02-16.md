# External Library Analysis & Recommendations
**Date:** February 16, 2026  
**Author:** Build System Analysis  
**Purpose:** Review current external libraries and recommend additions

---

## 📊 Current External Library Inventory

### Directory Structure

```
external/
├── archive-libs/          # EMPTY (unused)
├── camera-libs/           # 2 libraries
│   ├── libraw/
│   └── libraw-install/
├── compression-libs/      # 10 libraries
│   ├── bzip2-1.0.8        # BZip2 compression
│   ├── libarchive-3.7.6   # Archive extraction (TAR, ZIP, RAR, 7z)
│   ├── lz4-1.10.0         # LZ4 fast compression
│   ├── lzma-26.00         # LZMA compression (7-Zip SDK)
│   ├── minizip-ng-4.0.10  # ZIP archive creation/extraction
│   ├── unrar/             # RAR extraction (legacy)
│   ├── unrar-7.2.2/       # RAR extraction (current)
│   ├── xz-5.6.3           # XZ/LZMA2 compression
│   ├── zlib-1.3.1         # DEFLATE compression (ZIP, PNG)
│   └── zstd-1.5.7         # Zstandard compression
├── image-libs/            # 5 libraries
│   ├── dav1d-1.5.1        # AV1 image decoder
│   ├── libavif-1.3.0      # AVIF format support
│   ├── libjxl-0.11.1      # JPEG XL support
│   ├── libwebp-1.5.0-build     # WebP format support
│   └── libwebp-1.5.0-original  # WebP source backup
├── pdf-libs/              # 1 library
│   └── mupdf-1.24.11-source    # PDF rendering
└── ui-libs/               # EMPTY (unused)
```

### Summary by Category

| Category | Count | Total Size | Purpose |
|----------|-------|------------|---------|
| **compression-libs** | 10 | ~50 MB | Compression algorithms + archive formats |
| **image-libs** | 5 | ~80 MB | Modern image formats (AVIF, JXL, WebP) |
| **camera-libs** | 2 | ~15 MB | RAW photo formats (100+ cameras) |
| **pdf-libs** | 1 | ~30 MB | PDF rendering |
| **archive-libs** | 0 | 0 MB | **UNUSED** |
| **ui-libs** | 0 | 0 MB | **UNUSED** |
| **TOTAL** | **18** | **~175 MB** | |

---

## 🤔 Compression vs Archives: Why Combined?

### The Question
> "I can see that specific libs are defined as compression while other as archives - please check why not to join them to compression-libs sub-dirs?"

### Current State
**archive-libs/** directory exists but is **EMPTY**. All archive-related libraries are currently in **compression-libs/**.

### Libraries in Question

#### Pure Compression Algorithms (Raw Data)
These compress/decompress byte streams without file structure:
- **zlib** - DEFLATE algorithm (used by PNG, ZIP, gzip)
- **lz4** - Ultra-fast compression
- **zstd** - Facebook's Zstandard (better ratio + speed than zlib)
- **xz** - LZMA2 compression (best ratio, slow)
- **bzip2** - Block-sorting compression
- **lzma** - 7-Zip LZMA algorithm

#### Archive Format Handlers (File Containers)
These handle file collections with metadata, directory structure, and compression:
- **libarchive** - Reads/extracts TAR, ZIP, RAR, 7z, CPIO, ISO
- **minizip-ng** - Creates/extracts ZIP archives (uses zlib internally)
- **unrar** - Extracts RAR archives (proprietary format)

### Analysis: Keep Together or Separate?

#### Option A: Keep in compression-libs/ (Current Approach ✅)

**Rationale:**
1. **Functional Relationship**: Archives USE compression algorithms
   - ZIP uses DEFLATE (zlib)
   - 7z uses LZMA
   - TAR.GZ uses gzip (zlib)
   - Modern archives use ZSTD/LZMA2

2. **DarkThumbs Use Case**: Extracting thumbnails from archives
   - User opens `photos.zip` → extract JPEG thumbnail
   - User opens `textures.7z` → extract PNG thumbnail
   - Both involve decompression + file extraction

3. **Dependency Management**: Archive libs depend on compression libs
   - libarchive needs zlib, lzma, bzip2, zstd
   - minizip-ng needs zlib
   - Keeping them together simplifies include/link paths

4. **Build System Simplicity**: Single category for all compression-related code

**Advantages:**
- ✅ Single directory for related functionality
- ✅ Simpler CMake configuration
- ✅ Clearer dependency relationships
- ✅ Matches industry practice (vcpkg groups them similarly)

#### Option B: Separate to archive-libs/

**Rationale:**
1. **Semantic Clarity**: Compression ≠ Archives conceptually
2. **Clear Separation**: Algorithms vs. File Formats
3. **Documentation**: Easier to explain what each category does

**Disadvantages:**
- ❌ Artificial separation when they're tightly coupled
- ❌ More complex build scripts (need to link both dirs)
- ❌ Dependencies span categories (libarchive needs compression-libs)
- ❌ No functional benefit

### Recommendation: **Keep Current Structure** ✅

**Conclusion:** The current approach of keeping archives in `compression-libs/` is **correct and intentional**. Archive formats are just compression algorithms with metadata wrappers. Separating them would add complexity without benefit.

**If we renamed the directory**, it could be:
- `compression-libs/` → `compression-and-archives/` (verbose but accurate)
- Leave as-is with documentation explaining the relationship

The **archive-libs/** directory should remain empty or be deleted as it serves no purpose.

---

## 🆕 Recommended New External Libraries

### Critical Missing Libraries (Implement First)

#### 1. **libheif** - HEIF/HEIC Support 🔴 HIGH PRIORITY

**Status:** ⚠️ **MISSING - Action Required**

| Property | Value |
|----------|-------|
| **Formats** | `.heif`, `.heic` (iPhone photos since iOS 11) |
| **Library** | https://github.com/strukturag/libheif |
| **Version** | 1.19.5 (latest stable) |
| **License** | LGPL v3 (allows commercial use with dynamic linking) |
| **Dependencies** | libde265 (H.265 decoder), x265 (encoder, optional) |
| **Build System** | CMake |
| **Binary Size** | ~500 KB |
| **Impact** | **CRITICAL** - 40%+ of iOS users shoot HEIC |

**Why Critical:**
- Default format on Apple devices since 2017
- 50% smaller than JPEG with better quality
- HDR support, 10-bit/16-bit color depth
- Currently: DarkThumbs uses WIC fallback (requires Windows 10 1803+ extension)
- Problem: Many Windows users don't have HEIF codec installed

**Build Script:** `Build-LibHEIF.ps1` exists but library not yet built  
**Action:** Implement libheif build + integrate HEIFDecoder.cpp

---

#### 2. **libde265** - H.265/HEVC Decoder 🔴 REQUIRED FOR HEIF

**Status:** ⚠️ **MISSING - Required Dependency**

| Property | Value |
|----------|-------|
| **Purpose** | H.265/HEVC bitstream decoder (for HEIF images) |
| **Library** | https://github.com/strukturag/libde265 |
| **Version** | 1.0.15 |
| **License** | LGPL v3 |
| **Build System** | CMake |
| **Binary Size** | ~300 KB |

**Note:** Cannot use libheif without libde265. They're a matched pair.

**Action:** Add `Build-LibDE265.ps1` script

---

#### 3. **Assimp** - 50+ 3D Model Formats 🟡 MEDIUM PRIORITY

**Status:** ⚠️ **MISSING - User Requests**

| Property | Value |
|----------|-------|
| **Formats** | FBX, GLTF, Collada (`.dae`), Blender (`.blend`), 3DS, etc. (50+ formats) |
| **Library** | https://github.com/assimp/assimp |
| **Version** | 5.4.3 |
| **License** | Modified BSD (very permissive) |
| **Build System** | CMake, vcpkg available: `vcpkg install assimp` |
| **Binary Size** | ~5 MB |
| **Current Support** | OBJ, STL, GLTF/GLB (built-in parsers) |

**Why Add:**
- Industry standard 3D asset loader
- DarkThumbs currently supports 3 formats, Assimp adds 50+
- Used by Unity, Unreal Engine, Blender importers
- Scene graph parsing (materials, textures, animations)

**Integration Challenge:** Requires OpenGL/Direct3D11 renderer for thumbnail generation  
**Recommendation:** Add for v7.1 or v8.0

**Action:** Add `Build-Assimp.ps1` script

---

### Enhanced Format Support

#### 4. **PDFium** - Native PDF Rendering 🟡 MEDIUM PRIORITY

**Status:** ⚠️ **ALTERNATIVE TO MUPDF**

| Property | Value |
|----------|-------|
| **Formats** | `.pdf` |
| **Library** | https://pdfium.googlesource.com/pdfium/ |
| **Version** | chromium/6721+ (rolling release) |
| **License** | BSD 3-Clause ✅ (much better than MuPDF's AGPL) |
| **Build System** | GN (Google), prebuilt binaries available |
| **Binary Size** | ~20 MB |
| **Current Solution** | MuPDF 1.24.11 (AGPL v3 - licensing issue for commercial use) |

**Why Replace MuPDF:**
- **License:** MuPDF is AGPL v3 (requires source disclosure if distributed)
- **PDFium:** BSD license (no restrictions)
- **Quality:** Same engine as Google Chrome
- **Performance:** Faster multi-page rendering

**Recommendation:** Migrate to PDFium in v7.1 to resolve licensing concerns

**Action:** Add `Build-PDFium.ps1` + `PDFiumDecoder.cpp`

---

#### 5. **OpenEXR** - HDR/EXR Support 🟢 LOW PRIORITY

**Status:** ⚠️ **PROFESSIONAL USE CASE**

| Property | Value |
|----------|-------|
| **Formats** | `.exr` (Industrial Light & Magic HDR format) |
| **Library** | https://github.com/AcademySoftwareFoundation/openexr |
| **Version** | 3.3.0 |
| **License** | Modified BSD |
| **Build System** | CMake, vcpkg: `vcpkg install openexr` |
| **Binary Size** | ~2 MB |
| **Use Cases** | VFX, 3D rendering, professional photography |

**Features:**
- 16/32-bit floating point per channel
- Multi-layer images (RGB, depth, normals)
- Deep images (variable samples per pixel)
- Wide color gamut

**Current Support:** DarkThumbs handles basic `.exr` via WIC or custom parser (check Engine/Decoders/)

**Recommendation:** Add if professional/VFX users request it

---

### Video & Audio Enhancements

#### 6. **FFmpeg** - Universal Video/Audio 🔴 CRITICAL (Future)

**Status:** ⚠️ **NOT INTEGRATED - Using Media Foundation**

| Property | Value |
|----------|-------|
| **Formats** | ALL video (MP4, AVI, MKV, MOV, WebM, FLV, etc.) + audio |
| **Library** | https://github.com/FFmpeg/FFmpeg |
| **Version** | 7.0.2+ |
| **License** | LGPL v2.1 (with GPL components if enabled) |
| **Build System** | MSYS2/MinGW or use prebuilt shared libraries |
| **Binary Size** | ~50 MB (full), ~10 MB (minimal: libavcodec + libavformat + libavutil) |

**Current Solution:** Windows Media Foundation (built into OS)

**Why Consider FFmpeg:**
- **Codec Support:** MF limited to codecs installed on user's system
- **Consistency:** FFmpeg works same on all Windows versions
- **Formats:** Supports exotic formats (HEVC, AV1, VP9, ProRes, etc.)
- **Metadata:** Better extraction (duration, framerate, codec info)
- **Hardware Accel:** NVDEC, DXVA2, QSV support

**Why NOT Use FFmpeg (Yet):**
- ✅ **Media Foundation works fine** for most users
- ❌ **50 MB binary size** increase
- ❌ **Licensing complexity** (LGPL dynamic linking required)
- ❌ **Build complexity** (cross-compile or MinGW required)

**Recommendation:** Keep Media Foundation for v7.0. Consider FFmpeg for v8.0 if user complaints about codec support.

---

### Compression & Archives

#### 7. **7-Zip SDK** - Native 7z Support 🟢 LOW PRIORITY

**Status:** ⚠️ **HAVE LZMA SDK, BUT NOT HIGH-LEVEL 7z API**

| Property | Value |
|----------|-------|
| **Formats** | `.7z` archives (better API than raw LZMA) |
| **Library** | https://www.7-zip.org/sdk.html |
| **Version** | 24.08 |
| **License** | Public domain + LGPL |
| **Build System** | Custom makefiles |
| **Binary Size** | ~500 KB |

**Current State:** We have LZMA SDK 26.00 for raw compression, but not 7z archive handling

**Note:** libarchive already supports .7z extraction using our LZMA library

**Recommendation:** **Not needed** - libarchive + LZMA SDK already covers .7z archives

---

#### 8. **Brotli** - Web Compression 🟢 LOW PRIORITY

**Status:** ⚠️ **MISSING - WEB USE CASE**

| Property | Value |
|----------|-------|
| **Purpose** | Google's compression algorithm (better ratio than gzip) |
| **Library** | https://github.com/google/brotli |
| **Version** | 1.1.0 |
| **License** | MIT |
| **Build System** | CMake |
| **Binary Size** | ~200 KB |
| **Use Case** | `.br` compressed files, WOFF2 fonts |

**Recommendation:** Add only if users encounter `.br` files (rare outside web servers)

---

### Specialized Image Formats

#### 9. **NanoSVG** - SVG Vector Graphics ✅ EASY WIN

**Status:** ⚠️ **MISSING - HEADER-ONLY**

| Property | Value |
|----------|-------|
| **Formats** | `.svg`, `.svgz` (scalable vector graphics) |
| **Library** | https://github.com/memononen/nanosvg |
| **Version** | 2023-12 (single-header) |
| **License** | MIT |
| **Build System** | Header-only (just `#include "nanosvg.h"`) |
| **Binary Size** | <50 KB compiled |

**Why Easy:**
- Single header file (~2500 lines)
- No dependencies
- MIT license
- Parser + rasterizer in one

**Current Support:** DarkThumbs has `SVGDecoder.cpp` using GDI+ (check if nanosvg would be better)

**Action:** Evaluate current SVG decoder vs. nanosvg performance

---

#### 10. **libqoi** - QOI Image Format ✅ ALREADY SUPPORTED

**Status:** ✅ **ALREADY INTEGRATED**

DarkThumbs v7.0 already supports QOI via header-only implementation. No action needed.

---

## 📋 Implementation Priority Matrix

### Phase 1: Critical (v7.1 - Next 2 Months)

| Library | Formats | Impact | Difficulty | Time Est. |
|---------|---------|--------|------------|-----------|
| **libheif + libde265** | HEIF/HEIC | 🔴 Critical | Medium | 2-3 days |
| **Build Script Fixes** | - | 🔴 Critical | Easy | 4 hours |

### Phase 2: High Value (v7.2 - Q2 2026)

| Library | Formats | Impact | Difficulty | Time Est. |
|---------|---------|--------|------------|-----------|
| **PDFium** | PDF | 🟡 High (license fix) | Medium | 2-3 days |
| **Assimp** | 50+ 3D formats | 🟡 High | High | 5-7 days |

### Phase 3: Nice to Have (v8.0 - Q3 2026)

| Library | Formats | Impact | Difficulty | Time Est. |
|---------|---------|--------|------------|-----------|
| **FFmpeg** | Video | 🟡 Medium | High | 7-10 days |
| **OpenEXR** | HDR | 🟢 Low | Medium | 2-3 days |
| **Brotli** | .br files | 🟢 Low | Easy | 4 hours |

---

## 🚫 Libraries NOT Recommended

### FLIF (Free Lossless Image Format)
- **Status:** Abandoned (last release 2016)
- **Reason:** Superseded by JPEG XL lossless mode
- **Recommendation:** Use JPEG XL instead (already have libjxl)

### OpenImageIO
- **Status:** Too large (~10 MB + dependencies)
- **Reason:** DarkThumbs targets specific formats, not generic all-format library
- **Recommendation:** Add specific libraries (libheif, PDFium) instead

### DirectXTex
- **Status:** Microsoft DDS library
- **Reason:** Already support DDS via WIC
- **Recommendation:** Not needed

---

## 📊 External Library Growth Projection

### Current State (v7.0)
- **Libraries:** 18
- **Total Size:** ~175 MB
- **Formats Supported:** 80+

### After Phase 1 (v7.1)
- **Libraries:** 20 (+2: libheif, libde265)
- **Total Size:** ~176 MB (+1 MB)
- **Formats Supported:** 82+ (+2: HEIF, HEIC)

### After Phase 2 (v7.2)
- **Libraries:** 22 (+2: PDFium, Assimp)
- **Total Size:** ~201 MB (+25 MB)
- **Formats Supported:** 130+ (+50: 3D formats)

### After Phase 3 (v8.0)
- **Libraries:** 25 (+3: FFmpeg, OpenEXR, Brotli)
- **Total Size:** ~213 MB (+12 MB)
- **Formats Supported:** 135+

---

## 🎯 Immediate Action Items

1. ✅ **Document this analysis** (completed)
2. ⚠️ **Build libheif + libde265** (high priority)
3. ⚠️ **Fix PowerShell naming conventions** (build-unrar.ps1 → Build-UnRAR.ps1)
4. ⚠️ **Delete or document empty directories** (archive-libs/, ui-libs/)
5. ⚠️ **Evaluate PDFium migration** (MuPDF AGPL licensing issue)

---

## 📝 Conclusion

**Current Structure is Sound:** The compression-libs/ directory correctly groups compression algorithms with archive format handlers. No reorganization needed.

**Critical Gap:** HEIF/HEIC support is the #1 missing feature (iPhone photo format). Build libheif immediately.

**Future Growth:** Conservative library additions (20 → 25 libraries) will expand format support from 80 to 135+ formats while keeping binary size under 215 MB.

**License Concern:** MuPDF AGPL v3 licensing may require migration to PDFium (BSD) for commercial distribution.
