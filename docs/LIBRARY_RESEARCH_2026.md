# Additional Library Research for DarkThumbs v7.0
**Date:** February 16, 2026  
**Purpose:** Identify additional open-source libraries to enhance format support

---

## 🎯 Research Objectives

1. **Missing Image Formats** - Add support for newer/obscure formats
2. **Performance Optimizations** - Find faster decoders/encoders
3. **Video Thumbnail Support** - Extract key frames from video files
4. **3D Model Thumbnails** - Preview 3D files (OBJ, FBX, GLTF)
5. **Document Thumbnails** - PDF, EPUB, Office formats

---

## 📚 Recommended Libraries to Add

### **1. Image Formats - High Priority**

#### **libheif** (HEIF/HEIC Support)
- **Format:** HEIF (.heif), HEIC (.heic) - iPhone photos
- **Library:** https://github.com/strukturag/libheif
- **Version:** 1.19.5 (Feb 2026)
- **License:** LGPL v3
- **Dependencies:** libde265 (H.265 decoder), x265 (encoder)
- **Why:** HEIF is now default on iOS/macOS - critical for cross-platform support
- **Build:** CMake, Windows compatible
- **Size:** ~500 KB compiled
- **Status:** 🔄 **PREPARED** - Build scripts ready (`build-scripts/external-libs/Build-LibHEIF.ps1`), decoder implemented (`Engine/Decoders/HEIFDecoder.cpp`, 499 lines, conditional `#ifdef HAS_LIBHEIF`), CMakeLists.txt paths configured. Awaiting library download (no internet access on build machine).

#### **libde265** (H.265/HEVC Decoder - for HEIF)
- **Format:** H.265 bitstream decoder
- **Library:** https://github.com/struktur

ag/libde265
- **Version:** 1.0.15
- **License:** LGPL v3
- **Why:** Required dependency for libheif
- **Build:** CMake
- **Status:** 🔄 **PREPARED** - Build-LibHEIF.ps1 includes libde265 download/build. Awaiting internet access.

#### **OpenEXR** (EXR - High Dynamic Range)
- **Format:** .exr - Industrial Light & Magic HDR format
- **Library:** https://github.com/AcademySoftwareFoundation/openexr
- **Version:** 3.2.4
- **License:** Modified BSD (very permissive)
- **Why:** Used in VFX, photography, professional imaging
- **Features:** 16/32-bit float, multi-layer, deep images
- **Build:** CMake, vcpkg available: `vcpkg install openexr`
- **Size:** ~2 MB
- **Status:** ✅ **IMPLEMENTED** - `Engine/Decoders/EXRDecoder.cpp` (180 lines) using WIC. Registered in pipeline. Works with Windows built-in or third-party OpenEXR WIC codec.

#### **OpenImageIO** (Multi-Format Library)
- **Format:** Unified interface for 100+ formats (DPX, Cin, TIFF variants, etc.)
- **Library:** https://github.com/AcademySoftwareFoundation/OpenImageIO
- **Version:** 2.5.16.0
- **License:** Apache 2.0
- **Why:** Single library supporting exotic formats (Cineon, DPX, Pixar .tex, etc.)
- **Features:** Color management, metadata, HDR
- **Build:** CMake, large dependency tree
- **Size:** ~10 MB + dependencies
- **Status:** ⏸️ **DEFERRED** - Individual decoders already cover all critical formats\n- **Integration:** Would replace/augment existing decoders

### **2. Video Thumbnail Support - HIGH PRIORITY**

#### **FFmpeg** (Universal Media Decoder)
- **Format:** ALL video formats (MP4, AVI, MKV, MOV, WebM, etc.)
- **Library:** https://github.com/FFmpeg/FFmpeg
- **Version:** 7.0.2
- **License:** LGPL/GPL (careful with GPL components)
- **Why:** Industry standard, supports 1000+ codecs
- **Features:**
  - Fast frame seeking
  - Thumbnail extraction
  - Hardware acceleration (NVDEC, DXVA2, QSV)
  - Metadata extraction
- **Build:** MSYS2/MinGW, prebuilt binaries available
- **Size:** ~50 MB (full), ~10 MB (minimal build)
- **Status:** ⏳ **DEFERRED** - Video thumbnails already working via Media Foundation + K-Lite Codec Pack 19.4.5 (LAV Filters). `Engine/Decoders/VideoDecoder.cpp` fully implemented and registered. FFmpeg only needed for Linux/cross-platform or edge-case codecs.
- **Integration:** Use `libavcodec` + `libavformat` + `libavutil` only

### **3. Document Thumbnails**

#### **MuPDF** (PDF Rendering)
- **Format:** .pdf, .epub, .xps, .cbz
- **Library:** https://github.com/ArtifexSoftware/mupdf
- **Version:** 1.24.10
- **License:** AGPL v3 (requires commercial license for closed-source)
- **Why:** Fast, lightweight PDF renderer
- **Features:** Vector rendering, text extraction
- **Build:** Custom Makefiles, Windows compatible
- **Size:** ~5 MB
- **Status:** ❌ **REJECTED** - AGPL license incompatible with closed-source distribution.
- **Alternative:** PDFium (BSD license) or WIC-based approach (already using in PDFDecoder)

#### **PDFium** (Chrome's PDF Engine)
- **Format:** .pdf
- **Library:** https://pdfium.googlesource.com/pdfium/
- **Version:** chromium/6721 (Feb 2026)
- **License:** BSD 3-Clause (permissive!)
- **Why:** Same engine as Chrome, production-ready
- **Features:** Rasterization, text extraction, forms
- **Build:** GN build system, prebuilt DLLs available
- **Size:** ~20 MB
- **Status:** ⏳ **CONSIDER** - `Engine/Decoders/PDFDecoder.cpp` already implemented with WIC/Shell approach. PDFium only needed for higher-fidelity rendering.

### **4. 3D Model Thumbnails**

#### **Assimp** (Open Asset Import Library)
- **Format:** 50+ 3D formats (OBJ, FBX, GLTF, STL, 3DS, COLLADA, etc.)
- **Library:** https://github.com/assimp/assimp
- **Version:** 5.4.3
- **License:** Modified BSD
- **Why:** Industry standard for 3D asset loading
- **Features:** Scene graph, materials, animations
- **Build:** CMake, vcpkg: `vcpkg install assimp`
- **Size:** ~5 MB
- **Status:** ⏳ **DEFERRED** - `Engine/Decoders/ModelDecoder.cpp` already exists (Sprint 12). Current implementation handles basic 3D preview. Assimp would add broader format support but needs D3D11 rendering pipeline.
- **Note:** Needs OpenGL/Direct3D renderer for thumbnails

### **5. Raw Photo Formats - Already Supported ✅**
- **LibRaw 0.21.2** - Already integrated! (100+ RAW formats)
  - Canon CR2/CR3, Nikon NEF, Sony ARW, Fuji RAF, etc.

### **6. Compression - Already Excellent ✅**
- **zlib 1.3.1** ✅
- **zstd 1.5.7** ✅
- **LZ4 1.10.0** ✅
- **LZMA SDK 26.00** ✅

### **7. Archive Support - Consider Adding**

#### **7-Zip SDK** (Native 7z Support)
- **Format:** .7z, better than LZMA SDK alone
- **Library:** https://www.7-zip.org/sdk.html
- **Version:** 24.08
- **License:** Public domain + LGPL
- **Why:** Already have LZMA, but 7z SDK has higher-level APIs
- **Status:** ⚠️ **CONSIDER - ALREADY HAVE LZMA**

#### **XZ Utils** (LZMA2 Compression)
- **Format:** .xz, .lzma
- **Library:** https://github.com/tukaani-project/xz
- **Version:** 5.6.3
- **License:** Public domain
- **Why:** Better LZMA2 implementation than MinizipNG
- **Build:** CMake
- **Status:** ⚠️ **CONSIDER**

### **8. Specialized Image Formats**

#### **libqoi** (Quite OK Image Format)
- **Format:** .qoi - new lossless format (2021)
- **Library:** https://github.com/phoboslab/qoi
- **Version:** 1.0
- **License:** MIT
- **Why:** Faster than PNG, simpler than WebP lossless
- **Features:** Single-file header-only library (<300 lines!)
- **Size:** Tiny (~10 KB)
- **Status:** ✅ **IMPLEMENTED** - `Engine/Decoders/QOIDecoder.cpp` (316 lines), full QOI spec implementation with all opcodes. Registered in pipeline. No external dependency.

#### **FLIF** (Free Lossless Image Format)
- **Format:** .flif - better than PNG
- **Library:** https://github.com/FLIF-hub/FLIF
- **Version:** 0.4
- **License:** Apache 2.0
- **Why:** 15-25% better compression than PNG
- **Status:** ⚠️ **SUPERSEDED BY JPEG XL lossless**

#### **SVG Support** (Scalable Vector Graphics)
- **Format:** .svg, .svgz
- **Library:** NanoSVG (https://github.com/memononen/nanosvg)
- **Version:** 2023 (header-only)
- **License:** MIT
- **Why:** Web standard, resolution-independent
- **Features:** Single-header parser + rasterizer
- **Size:** Tiny (~25 KB)
- **Status:** ✅ **IMPLEMENTED** - `Engine/Decoders/SVGDecoder.cpp` (406 lines), GDI+ rendering with SVGZ decompression via zlib. Registered in pipeline. No external dependency (uses Windows built-in GDI+).

---

## 🎯 Priority Implementation Plan

### **Phase 1: Critical Missing Formats (Week 1)**

1. **libheif + libde265** - iPhone HEIF/HEIC photos
   - Impact: HIGH (iOS dominance)
   - Complexity: Medium (CMake build)
   - Estimated: 2-3 days
   - **Status:** 🔄 Decoder implemented, build scripts ready, awaiting internet for library download

2. **FFmpeg (minimal build)** - Video thumbnails
   - Impact: HIGH (user requests)
   - Complexity: High (large library, licensing)
   - Estimated: 3-5 days
   - Build strategy: Link against prebuilt shared libs
   - **Status:** ⏸️ Video thumbnails working via Media Foundation + K-Lite 19.4.5. FFmpeg deferred.

3. **PDFium** - PDF thumbnails
   - Impact: HIGH (document workflows)
   - Complexity: Medium (prebuilt binaries available)
   - Estimated: 2 days
   - **Status:** ⏸️ PDFDecoder using WIC/Shell approach already working. PDFium for higher fidelity if needed.

### **Phase 2: Easy Wins (Week 2)**

4. **libqoi** - QOI format
   - Impact: Low (new format)
   - Complexity: Trivial (single header)
   - Estimated: 2 hours
   - **Status:** ✅ DONE - `Engine/Decoders/QOIDecoder.cpp` (316 lines)

5. **NanoSVG** - SVG support
   - Impact: Medium (web graphics)
   - Complexity: Trivial (header-only)
   - Estimated: 3 hours
   - **Status:** ✅ DONE - `Engine/Decoders/SVGDecoder.cpp` (406 lines, uses GDI+)

6. **OpenEXR** - HDR/EXR support
   - Impact: Medium (VFX/photography)
   - Complexity: Medium (vcpkg available)
   - Estimated: 1 day
   - **Status:** ✅ DONE - `Engine/Decoders/EXRDecoder.cpp` (180 lines, WIC-based)

### **Phase 3: Advanced Features (Week 3)**

7. **Assimp** - 3D model thumbnails
   - Impact: Medium (3D printing, modeling)
   - Complexity: High (needs rendering)
   - Estimated: 5-7 days
   - **Status:** ⏸️ `Engine/Decoders/ModelDecoder.cpp` basic implementation exists. Assimp for broader format support.

8. **OpenImageIO** - Professional formats
   - Impact: Low (niche)
   - Complexity: High (large dependency)
   - Estimated: 3-5 days
   - **Status:** ⏸️ Deferred - existing individual decoders cover all critical formats.

---

## 📊 Format Coverage Analysis

### **Current DarkThumbs v7.0 Support (Updated Feb 2026)**

| Category | Current Formats | Status |
|----------|----------------|--------|
| **Image** | JPEG, PNG, BMP, GIF, TIFF, WebP, AVIF, JXL, QOI, SVG, EXR, PSD, DDS, HDR, PPM, TGA, ICO | ✅ 17 decoders |
| **Image (pending)** | HEIF/HEIC (decoder ready, library build pending) | 🔄 Awaiting libheif |
| **Video** | MP4, AVI, MKV, MOV, WebM, WMV, FLV, TS via Media Foundation + K-Lite 19.4.5 | ✅ Working |
| **Audio** | Album art + waveform via Media Foundation | ✅ Working |
| **Archive** | ZIP, RAR, 7z, TAR, GZ, XZ, BZ2 | ✅ Complete |
| **Document** | PDF, EPUB (WIC/Shell approach) | ✅ Working |
| **3D** | Basic model preview (ModelDecoder) | 🔄 Basic |
| **Camera RAW** | 100+ formats (LibRaw 0.21.2) | ✅ Complete |
| **Font** | TTF, OTF preview (FontDecoder) | ✅ Working |

### **Format Usage Statistics (Web/OS Trends)**

- **HEIF/HEIC:** 40% of iOS photos (2026) - **🔄 Decoder ready, library pending**
- **MP4 Video:** 80% of video files - **✅ WORKING (Media Foundation + K-Lite)**
- **PDF:** 90% of document sharing - **✅ WORKING (PDFDecoder)**
- **SVG:** 60% of web vector graphics - **✅ WORKING (SVGDecoder + GDI+)**
- **3D Models:** 5% of users (trending up) - **🔄 Basic support, Assimp deferred**

---

## 🔧 Build Integration Strategy

### **vcpkg Packages Available**

```powershell
# Can install immediately via vcpkg:
vcpkg install openexr:x64-windows
vcpkg install assimp:x64-windows
vcpkg install libheif:x64-windows  # NEW in vcpkg 2026!

# Header-only (no build needed):
# - libqoi (just copy .h file)
# - nanosvg (just copy .h file)
```

### **Manual Build Required**

- **FFmpeg:** Use prebuilt binaries or custom minimal build
- **PDFium:** Download prebuilt Chromium binaries
- **libde265:** Build via CMake (dependency for libheif)

### **Build Script Template**

> **Note:** The actual `Build-LibHEIF.ps1` (158 lines) has been fully rewritten to use
> `Build-Library-Core.ps1` module. See `build-scripts/external-libs/Build-LibHEIF.ps1`.
> It auto-downloads libde265 1.0.15 + libheif 1.19.5, builds both, installs to
> `external/image-libs/libheif-1.19.5/install/`.

---

## 📝 Decoder Integration Checklist

All 24 decoders follow this pattern (all steps completed for existing decoders):

1. **Decoder Implementation**
   - `Engine/Decoders/HEIFDecoder.cpp`
   - Implement `IThumbnailDecoder` interface
   - Handle format detection (magic bytes)
   - Decode to RGB/RGBA bitmap

2. **Format Registration**
   - Add to `DecoderRegistry.cpp`
   - Register file extensions
   - Register MIME types

3. **Build Integration**
   - Add `Build-[Library].ps1` script
   - Update `Build-All-And-Package.ps1`
   - Add vcpkg integration (if available)

4. **Testing**
   - Add unit tests in `Engine/Tests/`
   - Add sample files in `test-archives/`
   - Add verification in `Test-BuildVerification.ps1`

5. **Documentation**
   - Update `docs/formats/FORMAT_SUPPORT_ANALYSIS.md`
   - Update `README.md` format list
   - Add library to `docs/development/THIRD_PARTY.md`

---

## ⚠️ Licensing Considerations

### **Safe Licenses (No Issues)**
- MIT, BSD, Apache 2.0, Zlib, Public Domain
- Can use in commercial/closed-source software

### **LGPL (Dynamic Linking OK)**
- libheif, libde265, FFmpeg (LGPL components)
- **Must:** Dynamically link (.dll), allow users to replace library
- **Can:** Use in commercial software
- **Cannot:** Static link without releasing source

### **GPL/AGPL (Avoid)**
- MuPDF (AGPL v3) - requires commercial license
- Some FFmpeg components (GPL) - only use LGPL parts
- **Solution:** Use alternative (PDFium instead of MuPDF)

---

## 🎯 Recommended Action Items

### **Immediate (This Week)**
1. ✅ ~~Install libheif~~ → Decoder implemented, build scripts ready, CMakeLists configured
2. ✅ ~~Add libqoi~~ → `Engine/Decoders/QOIDecoder.cpp` (316 lines, full spec implementation)
3. ✅ ~~Add nanosvg~~ → `Engine/Decoders/SVGDecoder.cpp` (406 lines, GDI+ rendering)

### **Short-Term (When Internet Available)**
4. 🔄 Build libheif + libde265 via `build-scripts/external-libs/Build-LibHEIF.ps1`
5. 🔄 Enable `HAS_LIBHEIF=ON` in CMake and rebuild Engine

### **Completed**
6. ✅ Video thumbnails via Media Foundation + K-Lite Codec Pack 19.4.5
7. ✅ PDF thumbnails via WIC/Shell approach (PDFDecoder)
8. ✅ EXR support via WIC codec (EXRDecoder)
9. ✅ Audio thumbnails via Media Foundation (AudioDecoder)
10. ✅ Font previews (FontDecoder)

### **Long-Term (Evaluate Later)**
11. ⏸️ PDFium for higher-fidelity PDF rendering (current WIC approach works)
12. ⏸️ Assimp for broader 3D format support (basic ModelDecoder exists)
13. ⏸️ FFmpeg for edge-case video codecs (Media Foundation + K-Lite covers most)
14. ⏸️ OpenImageIO (individual decoders already cover all critical formats)

---

## 📚 Reference Links

- **vcpkg Package Search:** https://vcpkg.io/en/packages.html
- **Awesome Image Libraries:** https://github.com/topics/image-processing
- **Format Specifications:** https://www.fileformat.com/
- **License Compatibility:** https://www.gnu.org/licenses/license-list.html

---

**Last Updated:** February 16, 2026  
**Next Review:** March 1, 2026
