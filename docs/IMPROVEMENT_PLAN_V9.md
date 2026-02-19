# DarkThumbs — Comprehensive Improvement & Enhancement Plan

**Created:** 2026-02-25  
**Audit Baseline:** v8.3.0 (174 sprints completed)  
**Target Version:** v9.0.0  
**Scope:** Code quality, documentation sync, feature gaps, shell registration, performance, compatibility, file format coverage

---

## Table of Contents

1. [Executive Summary](#1-executive-summary)
2. [Code vs Documentation Sync Issues](#2-code-vs-documentation-sync-issues)
3. [Critical Code Bugs & Issues](#3-critical-code-bugs--issues)
4. [Shell Registration Gaps](#4-shell-registration-gaps)
5. [Build System & CMakeLists Issues](#5-build-system--cmakelists-issues)
6. [Documentation Overhaul](#6-documentation-overhaul)
7. [Feature Implementation Gaps](#7-feature-implementation-gaps)
8. [File Format Coverage — Best in Class](#8-file-format-coverage--best-in-class)
9. [Performance Enhancements](#9-performance-enhancements)
10. [Compatibility & Platform](#10-compatibility--platform)
11. [Testing & Quality](#11-testing--quality)
12. [Project Governance](#12-project-governance)
13. [Prioritized Sprint Plan](#13-prioritized-sprint-plan)

---

## 1. Executive Summary

### Audit Methodology
- Full source code review: `cbxArchive.h` (2680 lines), `Engine/CMakeLists.txt` (525 lines), all 25+ decoder source files, plugin system, pipeline, memory, cache modules
- Full documentation review: 196+ MD files across `docs/`, `SDK/`, `tests/`, `scripts/`, `packaging/`
- Extension mapping analysis: `CBXShell.rgs`, `GetCBXType()`, decoder `m_extensions[]` arrays
- Cross-reference: MASTER_PLAN.md (174 sprints) vs actual code state

### Key Findings

| Category | Finding | Severity |
|---|---|---|
| Version drift | README says v7.1.0, MASTER_PLAN says v8.3.0, 6+ docs say v5.3-v7.0 | 🔴 Critical |
| Shell registration | ~47 extensions in .rgs vs ~200+ in code — **153+ unregistered** | 🔴 Critical |
| Format routing bug | `.djvu`/`.djv` routed to `CBXTYPE_EPUB` instead of `CBXTYPE_DJVU` | 🔴 Critical |
| Model routing | `CBXTYPE_MODEL` defined but NO extensions mapped in `GetCBXType()` | 🟡 High |
| Decoder overlap | `.avif`/`.heif`/`.heic` claimed by BOTH AVIFDecoder AND HEIFDecoder | 🟡 High |
| Orphaned code | 4+ decoder files exist but NOT in CMakeLists.txt build | 🟡 High |
| Header-only sprints | Sprints 150-174 produced ~25 header files with no .cpp implementations | 🟡 High |
| Stale docs | 8+ docs reference wrong versions, counts, or statuses | 🟡 High |
| Missing CHANGELOG | No v8.x entries despite declaring v8.3.0 | 🟡 High |
| Format gaps | ~20+ common formats missing for "best in class" status | 🔵 Medium |

---

## 2. Code vs Documentation Sync Issues

### 2.1 Version Number Drift

| File | Claims | Actual | Action |
|---|---|---|---|
| `README.md` badge | v7.1.0 | v8.3.0 | Update badge |
| `README.md` body | "Current Version: 7.0.0" | v8.3.0 | Update to 8.3.0 |
| `README.md` body | "Version 7.1 completes 74 sprints" | 174 sprints | Rewrite intro |
| `CHANGELOG.md` | Latest entry is v7.1.0 | v8.3.0 | Add v8.0-v8.3 entries |
| `docs/formats/FORMAT_SUPPORT_MATRIX_V7.md` | v7.0.0, 80+ extensions | v8.3.0, 200+ | Full rewrite as V8 |
| `docs/formats/FORMAT_SUPPORT_ANALYSIS.md` | v5.3.0 | v8.3.0 | Full rewrite or archive |
| `docs/formats/CAPABILITY_AUDIT.md` | "42+ formats verified" | 200+ | Update counts |
| `docs/formats/DECODER_AUDIT_REPORT.md` | 5 decoders audited | 25+ exist | Full re-audit |
| `docs/formats/DECODER_STATUS.md` | v7.1.0 (Sprint 75) | v8.3.0 (Sprint 174) | Update matrix |
| `docs/PERFORMANCE.md` | "Benchmark Suite v7.0.0" | v8.3.0 | Update version |
| `tests/README.md` | "DarkThumbs Version: 6.0.0" | v8.3.0 | Update |
| `CBXShell/cbxArchive.h` header comment | "v4.6" | v8.3.0 | Update comment |
| `docs/formats/HEIF_VALIDATION_STATUS.md` | "shipping in v7.0.0" | OK (historical) | Add v8.3 note |

### 2.2 Format Count Discrepancies

| Document | Claims | Actual Code |
|---|---|---|
| `FORMAT_SUPPORT_MATRIX_V7.md` | "80+ file extensions" | 200+ in GetCBXType |
| `README.md` | "200+ file formats via 24 decoders" | Mostly accurate, but 25+ decoders now |
| `CAPABILITY_AUDIT.md` | "42+ formats verified operational" | 200+ defined in code |
| `FORMAT_SUPPORT_ANALYSIS.md` | Many formats marked "Not Yet Implemented" | TGA, QOI, PSD, DDS, HDR, EXR, ICO all implemented |

### 2.3 Status Mismatches

| Document | Claims | Actual Code |
|---|---|---|
| `FORMAT_SUPPORT_MATRIX_V7.md` | HEIF "🔄 In Progress" | Fully integrated, `HAS_LIBHEIF=ON` |
| `FORMAT_SUPPORT_ANALYSIS.md` | TGA "Not Yet Implemented" | `TGADecoder.cpp` exists and is compiled |
| `FORMAT_SUPPORT_ANALYSIS.md` | QOI "Not Yet Implemented" | `QOIDecoder.cpp` exists and is compiled |
| `FORMAT_SUPPORT_ANALYSIS.md` | PSD "Not Yet Implemented" | `PSDDecoder.cpp` exists and is compiled |
| `FORMAT_SUPPORT_ANALYSIS.md` | ICO "Not Yet Implemented" | `ICODecoder.cpp` exists and is compiled |

---

## 3. Critical Code Bugs & Issues

### 3.1 🔴 `.djvu`/`.djv` Mapped to Wrong CBXTYPE

**File:** `CBXShell/cbxArchive.h` — `GetCBXType()` function  
**Bug:** `.djvu` and `.djv` extensions are routed to `CBXTYPE_EPUB` (value 5)  
**Expected:** Should route to `CBXTYPE_DJVU` (value 22), which is already defined  
**Impact:** DjVu files get incorrect thumbnail generation via EPUB decoder logic  

```cpp
// CURRENT (wrong):
else if (ext == L".djvu" || ext == L".djv") return CBXTYPE_EPUB;

// SHOULD BE:
else if (ext == L".djvu" || ext == L".djv") return CBXTYPE_DJVU;
```

### 3.2 🔴 Model Format Routing Completely Missing

**File:** `CBXShell/cbxArchive.h` — `GetCBXType()` function  
**Bug:** `CBXTYPE_MODEL` is defined as value 80, but `GetCBXType()` has **zero** extension mappings for `.obj`, `.stl`, `.gltf`, `.glb`  
**Impact:** ModelDecoder is compiled and has extensions defined, but will never be reached from the shell extension  

**Fix:** Add to `GetCBXType()`:
```cpp
// 3D Model Formats
else if (ext == L".obj" || ext == L".stl" || ext == L".gltf" || ext == L".glb" ||
         ext == L".fbx" || ext == L".3ds" || ext == L".dae" || ext == L".ply")
    return CBXTYPE_MODEL;
```

### 3.3 🟡 AVIF/HEIF Decoder Extension Overlap

**Problem:** Both `AVIFDecoder` and `HEIFDecoder` claim `.avif`, `.heif`, `.heic`  
- `AVIFDecoder::m_extensions[]` = `.avif`, `.heif`, `.heic`  
- `HEIFDecoder::s_extensions[]` = `.heif`, `.heic`, `.hif`, `.heifs`, `.heics`, `.avci`, `.avcs`, `.avif`, `.heif-sequence`, `.heic-sequence`  

**Impact:** Ambiguous decoder selection. If HEIFDecoder is enabled (HAS_LIBHEIF=ON), the DecoderRegistry may pick either decoder for `.avif` files.  

**Recommended Fix:**
- AVIFDecoder should ONLY handle `.avif` and `.avifs` (pure AV1 images decoded via libavif+dav1d)
- HEIFDecoder should handle `.heic`, `.heif`, `.hif`, `.heics`, `.heifs`, `.avci`, `.avcs`, `.heif-sequence`, `.heic-sequence` (HEIF container decoded via libheif+libde265)
- Remove `.heif`/`.heic` from AVIFDecoder, remove `.avif` from HEIFDecoder

### 3.4 🟡 HEIF Extensions Missing from GetCBXType

These HEIFDecoder extensions have no mapping in `GetCBXType()`:
- `.hif` (Sony/Panasonic HEIF variant)
- `.avci` (AVC Intra coded HEIF)
- `.avcs` (AVC Intra sequence)
- `.heif-sequence` / `.heic-sequence` (HEIF image sequences)

### 3.5 🟡 AVIF Sequence Extension Missing

`GetCBXType()` maps `.avifs` to `CBXTYPE_AVIF` but `AVIFDecoder::m_extensions[]` does NOT include `.avifs`.

### 3.6 🟡 `cbxArchive.h` Version in Header Comment

The file header says "v4.6" — should say v8.3.0. This is cosmetic but misleading for anyone reading the source.

---

## 4. Shell Registration Gaps

### 4.1 Current State

**CBXShell.rgs registers 47 extensions:**
```
Archive:    .cbz .cbr .cb7 .cbt .7z .rar .zip
eBook:      .epub .mobi .azw .azw3 .fb2 .djvu .phz
Images:     .webp .avif .heic .heif .jxl .svg .svgz
Pro Images: .psd .psb .dds .hdr .exr .tga .qoi .ico
Netpbm:     .ppm .pgm .pbm .pnm
Camera RAW: .cr2 .cr3 .nef .arw .dng .orf .rw2 .raf .pef
Documents:  .docx .pptx .xlsx
Fonts:      .ttf .otf .woff .woff2
PDF:        .pdf
```

### 4.2 Extensions in GetCBXType but NOT in .rgs (Priority 1 — Should Add)

These extensions are handled by code but have no shell handler registration:

**Archive/Compression (18 missing):**
`.tar`, `.tar.gz`, `.tgz`, `.tar.bz2`, `.tbz`, `.tb2`, `.tar.xz`, `.txz`, `.tar.zst`, `.tzst`, `.cpio`, `.iso`, `.xar`, `.ar`, `.deb`, `.cab`, `.bz2`, `.zst`

**Video (34+ missing):**
`.mp4`, `.mkv`, `.avi`, `.wmv`, `.mov`, `.flv`, `.webm`, `.m4v`, `.mpg`, `.mpeg`, `.3gp`, `.3g2`, `.asf`, `.m1v`, `.m2v`, `.ts`, `.m2ts`, `.mts`, `.m2t`, `.mp4v`, `.3gp2`, `.3gpp`, `.mk3d`, `.f4v`, `.ogm`, `.ogv`, `.rm`, `.rmvb`, `.dv`, `.mxf`, `.ivf`, `.evo`, `.vob`, `.divx`

**Audio (18+ missing):**
`.mp3`, `.wav`, `.m4a`, `.ape`, `.flac`, `.ogg`, `.mka`, `.opus`, `.tak`, `.wv`, `.mpc`, `.aac`, `.wma`, `.aiff`, `.aif`, `.dsf`, `.dff`, `.alac`

**RAW Camera (18 missing):**
`.crw`, `.nrw`, `.srf`, `.sr2`, `.raw`, `.rwl`, `.srw`, `.3fr`, `.iiq`, `.cap`, `.mos`, `.erf`, `.kdc`, `.dcr`, `.mrw`, `.x3f`, `.mef`, `.bay`, `.gpr`

**Documents (9 missing):**
`.doc`, `.ppt`, `.xls`, `.rtf`, `.odt`, `.odp`, `.ods`, `.xps`, `.oxps`

**Netpbm (2 missing):**
`.pam`, `.pfm`

**Font (3 missing):**
`.ttc`, `.fon`, `.fnt`

**Image (4 missing):**
`.bmp`, `.dib`, `.gif`, `.tiff` (Windows has native handlers, but DarkThumbs could override)

**Icons (1 missing):**
`.cur`

**3D Models (4+ missing):**
`.obj`, `.stl`, `.gltf`, `.glb` (also needs GetCBXType fix first)

### 4.3 Registration Decision Matrix

| Category | Count Missing | Recommendation |
|---|---|---|
| Archive/Compression | 18 | ✅ Add all — core functionality |
| Camera RAW | 18 | ✅ Add all — key differentiator |
| Documents (legacy) | 9 | ✅ Add all — user expectation |
| Fonts | 3 | ✅ Add all — complete coverage |
| Netpbm | 2 | ✅ Add all — simple addition |
| Icons | 1 | ✅ Add — .cur matches .ico |
| 3D Models | 4+ | ✅ Add after GetCBXType fix |
| Video | 34 | ⚠️ Consider carefully — may conflict with Windows native handlers |
| Audio | 18 | ⚠️ Consider carefully — may conflict with media players |
| Standard Image | 4 | ⚠️ Optional — Windows already handles BMP/GIF/TIFF |

**Net effect:** Adding recommended extensions goes from 47 → ~100+ registered extensions.

---

## 5. Build System & CMakeLists Issues

### 5.1 Orphaned Source Files (Not in Build)

These files exist in `Engine/Decoders/` but are NOT registered in `Engine/CMakeLists.txt`:

| File | Type | Action |
|---|---|---|
| `ColorSpaceManager.h` | Header | Add to ENGINE_HEADERS or remove |
| `ExampleDecoder.h` + `ExampleDecoder.cpp` | Source | Add to build (SDK reference) or move to SDK/ |
| `OptimizedArchiveReader.h` | Header | Add to ENGINE_HEADERS or remove |
| `EPUBDecoder.h` | Header | Add to ENGINE_HEADERS (replaces EPUB logic in DocumentDecoder?) |
| `EBookCoverExtractor.h` | Header | ✅ Already registered in ENGINE_HEADERS |

### 5.2 Header-Only Sprint Deliverables

Sprints 150-174 produced many `.h` files registered in ENGINE_HEADERS but with **no corresponding `.cpp` in ENGINE_SOURCES**. These are design documents masquerading as C++ headers. While they compile (header-only), they don't contain real implementations.

**Assessment needed for each:** Is it a genuine header-only implementation (template/inline), or a design spec that needs a `.cpp` to be functional?

Key sprint headers to audit:

| Sprint | Header | Status |
|---|---|---|
| 150 | `PluginSandboxManager.h` | Likely needs .cpp |
| 151 | `PluginTrustChain.h` | Likely needs .cpp |
| 152 | `PluginCompatKit.h` | Likely complete (inline) |
| 153 | `PluginReferencePackV2.h` | Likely complete (inline) |
| 154 | `PluginEcosystemDashboard.h` | Likely needs .cpp |
| 155 | `ARM64BuildConfig.h` | Complete (config only) |
| 156 | `ARM64LibraryMatrix.h` | Complete (data only) |
| 157 | `RuntimeArchValidator.h` | Likely complete (inline) |
| 158 | `ARM64CIIntegration.h` | Likely needs .cpp |
| 159 | `ARM64TestSuite.h` | Likely needs .cpp |
| 160 | `JPEG2000Decoder.h` | Needs .cpp + library integration |
| 161 | `CADDecoderPlugin.h` | Needs .cpp + plugin impl |
| 162 | `GLTFThumbnailDecoder.h` | Needs .cpp |
| 163 | `ScientificFormatDecoder.h` | Needs .cpp |
| 164 | `FormatFallbackEngine.h` | Likely needs .cpp |
| 165 | `ArchiveMemoryCompactor.h` | Likely needs .cpp |
| 166 | `ZeroCopyPipeline.h` | Likely needs .cpp |
| 167 | `AdaptiveCacheBudgetManager.h` | Likely needs .cpp |
| 168 | `HotModeDirectoryMonitor.h` | Likely needs .cpp |
| 169 | `MemoryPressureControllerV2.h` | Likely needs .cpp |
| 170 | `ARM64MatrixValidator.h` | Likely complete (inline) |
| 171 | `InstallerLifecycleManager.h` | Likely needs .cpp |
| 172 | `ReleaseGateV2.h` | Likely needs .cpp |
| 173 | `DocumentationSyncAudit.h` | Likely complete (inline) |
| 174 | `V83ClosureReport.h` | Complete (data only) |

### 5.3 CMake Feature Flags Audit

Current feature flags in `Engine/CMakeLists.txt`:
```cmake
option(HAS_LIBJXL   "Enable JPEG XL support via libjxl"  ON)
option(HAS_LIBHEIF  "Enable HEIF/HEIC support via libheif" ON)
option(HAS_LIBRAW   "Enable RAW camera support via LibRaw" ON)
```

**Missing feature flags (should add):**
- `HAS_LIBAVIF` — Currently no separate flag, AVIF decoder always compiled
- `ENABLE_VIDEO_DECODER` — Video support could be optional
- `ENABLE_AUDIO_DECODER` — Audio support could be optional
- `ENABLE_PDF_DECODER` — PDF (MuPDF) could be optional
- `ENABLE_MODEL_DECODER` — 3D model support could be optional
- `ENABLE_PLUGIN_SYSTEM` — Plugin system toggle (exists in runtime config but not CMake)
- `ENABLE_LIBARCHIVE_SUPPORT` — exists in cbxArchive.h but should be in CMake

---

## 6. Documentation Overhaul

### 6.1 Files Requiring Full Rewrite

| File | Reason | Priority |
|---|---|---|
| `docs/formats/FORMAT_SUPPORT_MATRIX_V7.md` | Still says v7.0.0, 80+ formats | 🔴 P0 |
| `docs/formats/FORMAT_SUPPORT_ANALYSIS.md` | Says v5.3.0, severely stale | 🔴 P0 |
| `docs/formats/DECODER_AUDIT_REPORT.md` | Only 5 of 25+ decoders | 🔴 P0 |
| `CHANGELOG.md` | Missing v8.0, v8.1, v8.2, v8.3 entries | 🔴 P0 |
| `README.md` | Version V7.1, sprint count, format counts | 🟡 P1 |
| `docs/formats/CAPABILITY_AUDIT.md` | Says 42+ formats (should be 200+) | 🟡 P1 |
| `tests/README.md` | Says v6.0.0 | 🟡 P1 |
| `docs/PERFORMANCE.md` | Says v7.0.0 | 🟡 P1 |

### 6.2 Files Requiring Version Updates Only

| File | Current Version | Target |
|---|---|---|
| `docs/formats/DECODER_STATUS.md` | v7.1.0 | v8.3.0 |
| `docs/development/CODE_QUALITY_STANDARDS.md` | v7.1.0 | v8.3.0 |
| `SDK/docs/PLUGIN_SDK.md` | v7.1.0 | v8.3.0 |
| `docs/formats/HEIF_VALIDATION_STATUS.md` | v7.0.0 | v8.3.0 |

### 6.3 Missing Documentation

| Document | Purpose | Priority |
|---|---|---|
| `docs/release-notes/RELEASE_NOTES_v8.0.0.md` | v8.0 release notes | 🔴 P0 |
| `docs/release-notes/RELEASE_NOTES_v8.3.0.md` | v8.3 release notes | 🔴 P0 |
| `docs/formats/FORMAT_SUPPORT_MATRIX_V8.md` | Accurate format matrix | 🔴 P0 |
| `docs/architecture/DECODER_ROUTING.md` | Document GetCBXType() flow | 🟡 P1 |
| `docs/architecture/EXTENSION_REGISTRY.md` | Document .rgs vs GetCBXType vs decoder mappings | 🟡 P1 |
| `docs/development/HEADER_ONLY_AUDIT.md` | Audit sprint headers for implementation status | 🟡 P1 |
| `docs/ARM64_STATUS.md` | ARM64 build & test status | 🔵 P2 |
| `docs/PLUGIN_ECOSYSTEM_STATUS.md` | Plugin system maturity report | 🔵 P2 |

---

## 7. Feature Implementation Gaps

### 7.1 Sprint Headers Needing Real Implementation

These sprint deliverables have headers in `Engine/` but need `.cpp` files with real logic:

| Priority | Header | Effort | Impact |
|---|---|---|---|
| 🔴 P0 | `JPEG2000Decoder.h` (Sprint 160) | High — needs OpenJPEG library | New format support |
| 🟡 P1 | `GLTFThumbnailDecoder.h` (Sprint 162) | Medium — needs tinygltf or similar | 3D format support |
| 🟡 P1 | `FormatFallbackEngine.h` (Sprint 164) | Medium | Robustness |
| 🟡 P1 | `ArchiveMemoryCompactor.h` (Sprint 165) | Medium | Memory efficiency |
| 🟡 P1 | `ZeroCopyPipeline.h` (Sprint 166) | High | Performance |
| 🟡 P1 | `AdaptiveCacheBudgetManager.h` (Sprint 167) | Medium | Cache efficiency |
| 🟡 P1 | `HotModeDirectoryMonitor.h` (Sprint 168) | Medium | UX |
| 🟡 P1 | `MemoryPressureControllerV2.h` (Sprint 169) | Medium | Stability |
| 🔵 P2 | `CADDecoderPlugin.h` (Sprint 161) | High — plugin architecture | Niche format |
| 🔵 P2 | `ScientificFormatDecoder.h` (Sprint 163) | High — FITS/HDF5 libs | Niche format |
| 🔵 P2 | `PluginSandboxManager.h` (Sprint 150) | High | Security |
| 🔵 P2 | `PluginTrustChain.h` (Sprint 151) | High | Security |
| 🔵 P2 | `InstallerLifecycleManager.h` (Sprint 171) | Medium | Distribution |
| 🔵 P2 | `ReleaseGateV2.h` (Sprint 172) | Medium | Release quality |

### 7.2 Decoder Enhancement Opportunities

| Decoder | Current | Enhancement | Priority |
|---|---|---|---|
| ImageDecoder | WIC-based | Add EXIF orientation fix for all formats | 🟡 P1 |
| VideoDecoder | DirectShow/MF | Async decode, seek to interesting frame | 🟡 P1 |
| AudioDecoder | Cover art extraction | Waveform visualization fallback | 🔵 P2 |
| PDFDecoder | First page render | Multi-page preview collage | 🔵 P2 |
| SVGDecoder | Direct2D render | CSS/font support improvements | 🔵 P2 |
| DocumentDecoder | Embedded thumbnail | First-page render for docs without thumbnails | 🔵 P2 |
| ArchiveDecoder | First image extract | Smart image selection (largest/best quality) | 🔵 P2 |
| ModelDecoder | Basic wireframe | Proper 3D rendering with lighting | 🟡 P1 |

---

## 8. File Format Coverage — Best in Class

### 8.1 Current Coverage Summary

| Category | Decoder | Extensions | Count |
|---|---|---|---|
| Standard Images | ImageDecoder | jpg/jpeg/jpe/jfif/png/bmp/dib/gif/tif/tiff | 10 |
| Modern Images | WebPDecoder | webp | 1 |
| Modern Images | AVIFDecoder | avif (+ heif/heic overlap) | 1-3 |
| Modern Images | HEIFDecoder | heif/heic/hif/heifs/heics/avci/avcs/avif/sequences | 10 |
| Modern Images | JXLDecoder | jxl | 1 |
| Professional | PSDDecoder | psd/psb | 2 |
| Professional | DDSDecoder | dds | 1 |
| Professional | HDRDecoder | hdr | 1 |
| Professional | EXRDecoder | exr | 1 |
| Professional | TGADecoder | tga/tpic | 2 |
| Professional | ICODecoder | ico/cur | 2 |
| Utility | QOIDecoder | qoi | 1 |
| Utility | PPMDecoder | ppm/pgm/pbm/pnm/pam/pfm | 6 |
| Vector | SVGDecoder | svg/svgz | 2 |
| Camera RAW | RAWDecoder | cr2/cr3/crw/nef/nrw/arw/srf/sr2/orf/rw2/raw/raf/pef/ptx/dng/rwl/srw/3fr/iiq/x3f/dcr/kdc/mrw/erf/mef/gpr/r3d | 27 |
| Documents | DocumentDecoder | epub/mobi/azw/azw3/fb2/docx/doc/xlsx/xls/pptx/ppt/xps/oxps/djvu/djv/rtf/odt/ods/odp | 19 |
| PDF | PDFDecoder | pdf | 1 |
| Fonts | FontDecoder | ttf/otf/woff/woff2/ttc/fon/fnt | 7 |
| Archives | ArchiveDecoder | zip/cbz/cb7/cbr/cbt/7z/rar/tar/tar.gz/tgz/tar.bz2/tbz/tar.xz/txz | 14 |
| Video | VideoDecoder | mp4/mkv/avi/wmv/mov/flv/webm/m4v/mpg/mpeg/ts/mts/m2ts/3gp/3g2/vob/ogv/rm/rmvb/asf/divx/xvid | 22 |
| Audio | AudioDecoder | mp3/flac/wma/aac/m4a/ogg/opus/wav/aiff/aif/ape/wv/alac/mpc | 14 |
| 3D Models | ModelDecoder | obj/stl/gltf/glb | 4 |
| **TOTAL** | **25 decoders** | | **~149 unique** |

Plus ~50+ additional extensions in GetCBXType (archive variants, video variants, audio variants) = **~200 total**

### 8.2 Missing Formats for Best-in-Class

#### Image Formats — High Priority

| Format | Extension(s) | Library | Effort | Impact |
|---|---|---|---|---|
| PCX | .pcx | stb_image or custom | Low | Legacy format, still common |
| XPM | .xpm | Custom parser | Low | Unix/Linux icon format |
| Animated PNG | .apng | libpng / ImageDecoder | Low | Already handled by PNG? Verify |
| JPEG 2000 | .jp2, .j2k, .jpf, .jpx, .j2c | OpenJPEG 2.5.x | Medium | Medical/GIS/archival use |
| GIMP XCF | .xcf | Custom parser | Medium | Design tool native format |
| Photoshop PSB | Already done | — | — | ✅ Already handled |
| Farbfeld | .ff | Custom (trivial) | Very Low | Suckless image format |
| PIX | .pix | Custom | Low | Alias/Wavefront |
| SGI/RGB | .sgi, .rgb, .rgba, .bw | Custom | Low | Legacy SGI format |

#### Image Formats — Medium Priority

| Format | Extension(s) | Library | Effort | Impact |
|---|---|---|---|---|
| OpenRaster | .ora | minizip + PNG | Low | Open painting format |
| FLIF | .flif | libflif | Medium | Predecessor to JXL |
| BPG | .bpg | libbpg | Medium | Better Portable Graphics |
| KTX/KTX2 | .ktx, .ktx2 | Khronos KTX lib | Medium | GPU texture format (game dev) |
| VTF | .vtf | Custom parser | Medium | Valve Texture Format (gamers) |
| RGBE | .rgbe, .pic | Handled by HDR? | Low | Verify HDR decoder handles |
| Adobe Camera Raw | .xmp sidecar | Custom XML | Low | Metadata enhancement |

#### Vector/Design Formats

| Format | Extension(s) | Library | Effort | Impact |
|---|---|---|---|---|
| EPS | .eps, .epsf | Ghostscript or MuPDF | Medium | Print/design standard |
| WMF/EMF | .wmf, .emf | GDI+ (native Windows) | Low | Windows vector format |
| AI | .ai (PDF-based) | MuPDF/PDF decoder | Low | Adobe Illustrator (most are PDF) |
| CDR | .cdr | Custom/libcdr | High | CorelDRAW |
| Visio | .vsd, .vsdx | Custom | High | Microsoft Visio |

#### Scientific/Medical Formats

| Format | Extension(s) | Library | Effort | Impact |
|---|---|---|---|---|
| DICOM | .dcm, .dicom | DCMTK or custom | High | Medical imaging |
| FITS | .fits, .fit, .fts | cfitsio | High | Astronomy |
| HDF5 | .h5, .hdf5 | HDF5 lib | High | Scientific data |
| NetCDF | .nc, .cdf | NetCDF lib | High | Climate/ocean data |
| NIfTI | .nii, .nii.gz | Custom | High | Neuroimaging |

#### 3D Model Formats

| Format | Extension(s) | Library | Effort | Impact |
|---|---|---|---|---|
| FBX | .fbx | Autodesk FBX SDK | High | Industry standard 3D |
| STEP/IGES | .stp, .step, .igs, .iges | Open CASCADE | Very High | CAD standard |
| 3DS | .3ds | lib3ds | Medium | Legacy 3D format |
| PLY | .ply | Custom parser | Low | Point cloud/mesh |
| DAE (Collada) | .dae | TinyXML + custom | Medium | Exchange format |
| USD | .usd, .usda, .usdc, .usdz | OpenUSD | Very High | Pixar/Apple format |
| 3MF | .3mf | lib3mf | Medium | 3D printing |

#### eBook Formats

| Format | Extension(s) | Status | Notes |
|---|---|---|---|
| CBR | .cbr | ✅ Implemented | Comic book RAR |
| CBW | .cbw | Missing | Comic book web |
| Comic PDF | .pdf | ✅ Via PDF | Comic PDF |
| LIT | .lit | Missing | Older Microsoft eBook |
| LRF | .lrf | Missing | Sony eBook |

#### Archive Formats

| Format | Extension(s) | Library | Effort |
|---|---|---|---|
| WIM | .wim | libarchive | Low (if libarchive enabled) |
| RPM | .rpm | libarchive | Low |
| LZOP | .lzo | lzo lib | Low |
| Snappy | .sz, .snappy | snappy lib | Low |
| Brotli | .br | brotli lib | Low |

### 8.3 Recommended Priority Additions (v9.0)

**Tier 1 — Easy Wins (Low effort, immediate value):**
1. WMF/EMF via GDI+ (already available on Windows)
2. PCX via stb_image
3. Farbfeld (.ff) — trivial custom parser
4. PLY — simple parser, enhances ModelDecoder
5. .ai files — route through PDFDecoder (most AI files are PDF-based)
6. OpenRaster (.ora) — ZIP + PNG extraction
7. SGI/RGB format — simple custom parser

**Tier 2 — Medium Effort, High Value:**
1. JPEG 2000 (.jp2/.j2k) via OpenJPEG — implement Sprint 160 header
2. KTX/KTX2 — GPU texture format popular in game dev
3. XCF (GIMP) — custom parser for layer composite
4. EPS via MuPDF (already linked for PDF)
5. 3DS/DAE — expand ModelDecoder

**Tier 3 — High Effort, Niche Value:**
1. DICOM/FITS — scientific/medical
2. FBX — requires Autodesk SDK
3. STEP/IGES — requires Open CASCADE
4. USD — requires OpenUSD (huge dependency)

---

## 9. Performance Enhancements

### 9.1 Current Performance Baseline

From MASTER_PLAN.md:
- Single thumbnail: 17ms target
- Batch throughput: 235 images/second
- Cache hit: <5ms
- Build time: ~55 sec (Ninja), ~75 sec (MSBuild)

### 9.2 Proposed Improvements

| Enhancement | Current | Target | Effort | Impact |
|---|---|---|---|---|
| D3D12 compute shaders | D3D11 only | D3D12 compute pipeline | High | 2-3x GPU scaling speed |
| Async shell extension | Synchronous | Async IThumbnailProvider | Medium | No Explorer freeze |
| Parallel batch decode | Sequential | Thread pool per-format | Medium | 3-5x batch throughput |
| Memory-mapped I/O | Standard file I/O | CreateFileMapping for large files | Low | Faster for >100MB files |
| SIMD decode acceleration | Basic SIMD scaling | AVX-512 for resize/convert | Medium | 2x CPU decode speed |
| GPU texture cache | CPU thumbnails | Direct GPU texture reuse | Medium | Eliminates CPU→GPU copy |
| Lazy decoder loading | All decoders loaded | On-demand decoder initialization | Low | Faster DLL load time |
| Decoder priority queue | Round-robin | Priority by format popularity | Low | Better perceived speed |
| Pre-decode heuristics | Decode full image | Header-only size estimation | Low | Skip oversized images |
| Vulkan compute | DirectX only | Cross-API GPU support | Very High | Linux/Wine compatibility |

### 9.3 Cache System Improvements

| Enhancement | Description | Priority |
|---|---|---|
| Persistent disk cache | Survive process restart | 🟡 P1 |
| Cache warming on folder open | Pre-generate visible thumbnails | 🟡 P1 |
| Smart eviction | Evict by format decode cost, not just LRU | 🔵 P2 |
| Cache sharing across users | Shared AppData cache location | 🔵 P2 |
| Cache integrity verification | CRC32/xxHash validation | 🔵 P2 |

---

## 10. Compatibility & Platform

### 10.1 Current Compatibility

| Platform | Status | Notes |
|---|---|---|
| Windows 10 x64 | ✅ Full support | Primary target |
| Windows 11 x64 | ✅ Full support | Primary target |
| Windows 10 ARM64 | 🔄 Build config exists | No hardware validation |
| Windows 11 ARM64 | 🔄 Build config exists | No hardware validation |
| Windows Server 2019+ | ❓ Untested | Should work |
| Windows 8.1 | ❌ Not supported | WIC too old |

### 10.2 Platform Improvements

| Enhancement | Description | Priority |
|---|---|---|
| ARM64 hardware CI | Real ARM64 test runners in GitHub Actions | 🟡 P1 |
| Windows 11 features | Tabbed thumbnails, modern shell integration | 🔵 P2 |
| High-DPI awareness | Per-monitor DPI scaling for thumbnails | 🟡 P1 |
| Dark mode thumbnails | Adapt thumbnail background to system theme | 🔵 P2 |
| MSIX packaging | Modern Windows app packaging | 🟡 P1 |
| Auto-update | Built-in update mechanism | 🔵 P2 |
| Telemetry opt-in | Anonymous usage analytics | 🔵 P2 |

### 10.3 Library Version Updates

| Library | Current | Latest | Update Priority |
|---|---|---|---|
| zlib | 1.3.1 | 1.3.1 | ✅ Current |
| LZ4 | 1.10.0 | 1.10.0 | ✅ Current |
| zstd | 1.5.7 | 1.5.7 | ✅ Current |
| minizip-ng | 4.0.10 | 4.0.10 | ✅ Current |
| libwebp | 1.5.0 | 1.5.0 | ✅ Current |
| libavif | 1.3.0 | Check for updates | 🔵 Monitor |
| libjxl | 0.11.1 | Check for updates | 🔵 Monitor |
| libheif | 1.19.5 | Check for updates | 🔵 Monitor |
| LibRaw | 0.21.3 | Check for updates | 🔵 Monitor |
| LZMA SDK | 26.00 | 26.00 | ✅ Current |
| UnRAR | 7.2.2 | Check for updates | 🔵 Monitor |

---

## 11. Testing & Quality

### 11.1 Current State

- **Unit tests:** ~437 (100 original + 337 new from Sprints 150-174)
- **Benchmarks:** 5
- **Pass rate:** 100%
- **Coverage tool:** None configured

### 11.2 Testing Improvements

| Enhancement | Description | Priority |
|---|---|---|
| Code coverage measurement | OpenCppCoverage or gcov via CMake | 🟡 P1 |
| Format-specific test files | Curated test archive per format | 🟡 P1 |
| Fuzz testing | LibFuzzer for decoder inputs | 🟡 P1 |
| Integration tests | End-to-end COM thumbnail tests | 🟡 P1 |
| Performance regression CI | Automated benchmark comparison | 🔵 P2 |
| Memory leak detection | ASAN/MSAN integration | 🟡 P1 |
| Stress testing | 10K+ file batch processing | 🔵 P2 |
| Malformed input testing | Corrupt/truncated files per format | 🟡 P1 |
| Multi-monitor DPI testing | 100%/125%/150%/200% scale factors | 🔵 P2 |

### 11.3 Test Coverage Targets

| Component | Current Tests | Target | Gap |
|---|---|---|---|
| Core decoders (24) | ~80 | 240 (10 per decoder) | 160 |
| Plugin system | ~50 | 100 | 50 |
| Cache system | ~30 | 60 | 30 |
| Memory system | ~40 | 80 | 40 |
| Pipeline | ~30 | 60 | 30 |
| GPU rendering | ~10 | 40 | 30 |
| Shell integration | 0 | 20 | 20 |
| **Total** | **~437** | **~600** | **~360** |

---

## 12. Project Governance

### 12.1 Version Strategy

The project currently has a version identity crisis:
- `MASTER_PLAN.md` declares v8.3.0 (174 sprints)
- `README.md` and `CHANGELOG.md` show v7.1.0 (74 sprints)
- Many docs still reference v5.x/v6.x/v7.0

**Recommended Resolution:**
1. If Sprints 75-174 are truly complete WITH compiled implementations → update everything to v8.3.0
2. If Sprints 75-174 are header-only designs → the real version is still v7.1.0; reframe sprints as "design phase"
3. **Pragmatic approach:** Declare current state as **v7.2.0** (v7.1 + bug fixes + doc sync), then execute real implementations toward v8.0

### 12.2 Sprint Methodology Improvement

**Current issue:** Sprints produce header files without implementations, inflating velocity.

**Recommended changes:**
1. **Definition of Done:** Sprint deliverable must compile AND pass at least 3 tests
2. **No header-only sprints:** Each sprint must include `.h` + `.cpp` + tests
3. **Sprint sizing:** Reduce scope per sprint, increase implementation depth
4. **Code review gate:** Self-review before marking sprint complete

### 12.3 Documentation Governance

| Rule | Description |
|---|---|
| Version sync on release | All docs updated to match release version |
| Format matrix auto-gen | Script to generate FORMAT_SUPPORT_MATRIX from code |
| Doc freshness check | CI step that flags docs with stale version strings |
| Single source of truth | MASTER_PLAN.md is authoritative; all other docs reference it |

---

## 13. Prioritized Sprint Plan

### Phase 1: Foundation Fix (Sprints 175-179) — v8.4.0

| Sprint | Title | Deliverables |
|---|---|---|
| 175 | **Critical Bug Fixes** | Fix djvu→CBXTYPE_DJVU routing, add Model extensions to GetCBXType, fix AVIF/HEIF overlap, add missing HEIF extensions to GetCBXType |
| 176 | **Shell Registration Expansion** | Add 55+ missing extensions to CBXShell.rgs (archives, RAW, documents, fonts, models, Netpbm, .cur) |
| 177 | **Version Normalization** | Update ALL documentation to consistent version, update cbxArchive.h header, write CHANGELOG v8.x entries |
| 178 | **Documentation Rewrite P1** | Rewrite FORMAT_SUPPORT_MATRIX_V8.md, archive FORMAT_SUPPORT_ANALYSIS.md, rewrite DECODER_AUDIT_REPORT |
| 179 | **Build System Cleanup** | Register or remove orphaned files, audit header-only sprints, add missing CMake feature flags |

### Phase 2: Format Expansion (Sprints 180-186) — v9.0.0

| Sprint | Title | Deliverables |
|---|---|---|
| 180 | **Easy Format Wins** | WMF/EMF decoder (GDI+), PCX decoder (stb_image), Farbfeld decoder |
| 181 | **JPEG 2000 Implementation** | Implement Sprint 160 header with OpenJPEG library integration |
| 182 | **Enhanced Model Decoder** | PLY format, DAE format, proper 3D lighting, .rgs registration |
| 183 | **Vector Format Expansion** | EPS via MuPDF, AI routing through PDFDecoder |
| 184 | **Game Texture Formats** | KTX/KTX2 decoder, VTF decoder, enhanced DDS (BC7/ASTC) |
| 185 | **OpenRaster & XCF** | Open image editor format support |
| 186 | **SGI/RGB & Legacy** | SGI image format, XPM format support |

### Phase 3: Performance & Quality (Sprints 187-192) — v9.1.0

| Sprint | Title | Deliverables |
|---|---|---|
| 187 | **Async Shell Extension** | Non-blocking IThumbnailProvider with background decode |
| 188 | **D3D12 Compute Pipeline** | GPU compute shader scaling with D3D11 fallback |
| 189 | **Parallel Batch Decode** | Thread pool decoder with per-format parallelism |
| 190 | **Code Coverage & Fuzzing** | OpenCppCoverage integration, LibFuzzer for decoders |
| 191 | **Memory Safety** | ASAN integration, memory-mapped I/O for large files |
| 192 | **Cache System V2** | Persistent disk cache, cache warming, smart eviction |

### Phase 4: Platform & Polish (Sprints 193-198) — v9.2.0

| Sprint | Title | Deliverables |
|---|---|---|
| 193 | **ARM64 Hardware Validation** | Real ARM64 CI, hardware test results |
| 194 | **High-DPI Support** | Per-monitor DPI awareness, scaled thumbnails |
| 195 | **MSIX Packaging** | Modern Windows packaging, auto-update foundation |
| 196 | **Test Suite Expansion** | 160+ new tests, format-specific test archives |
| 197 | **Malformed Input Hardening** | Corrupt file handling, truncation resilience |
| 198 | **v9.2 Release Gate** | Full doc sync, performance benchmarks, release notes |

### Phase 5: Advanced Features (Sprints 199-204) — v10.0.0

| Sprint | Title | Deliverables |
|---|---|---|
| 199 | **Scientific Format Suite** | DICOM, FITS via Sprint 163 header implementation |
| 200 | **FBX/USD 3D Formats** | Advanced 3D format support |
| 201 | **Plugin Marketplace** | Online plugin discovery and installation |
| 202 | **Vulkan Compute** | Cross-API GPU acceleration |
| 203 | **Python SDK** | Python bindings for engine library |
| 204 | **v10 Release** | Comprehensive release, marketing, documentation |

---

## Appendix A: Complete Extension Audit

### Extensions by Registration Status

| Status | Count | Description |
|---|---|---|
| ✅ In .rgs + GetCBXType + Decoder | 47 | Fully working shell integration |
| ⚠️ In GetCBXType + Decoder only | ~103 | Code handles but no shell registration |
| ❌ In Decoder only | ~10 | Extension in decoder but not routed |
| 🔴 In GetCBXType only | ~5 | Routed but no decoder handles it |

### Extension Registration Coverage by Category

| Category | .rgs | GetCBXType | Decoder | Gap |
|---|---|---|---|---|
| Standard Images | 0 | 10 | 10 | .rgs (Windows handles natively) |
| Modern Images | 4 | 8 | 13+ | GetCBXType missing .hif/.avci/.avcs |
| Pro Images | 8 | 8 | 8 | ✅ Aligned |
| Camera RAW | 9 | 28 | 27 | .rgs missing 18 RAW extensions |
| Archives | 7 | 22+ | 14 | .rgs missing 15 archive extensions |
| Video | 0 | 34+ | 22 | No .rgs registration |
| Audio | 0 | 18+ | 14 | No .rgs registration |
| Documents | 3 | 11+ | 19 | .rgs missing 8 doc extensions |
| Fonts | 4 | 4 | 7 | .rgs missing 3 font extensions |
| PDF | 1 | 1 | 1 | ✅ Aligned |
| 3D Models | 0 | 0 | 4 | Neither .rgs NOR GetCBXType |
| Netpbm | 4 | 6 | 6 | .rgs missing 2 |
| eBook | 6 | 5 | n/a | .rgs has .phz not in GetCBXType |
| Icons | 1 | 1 | 2 | .rgs missing .cur |

---

## Appendix B: Quick Fix Checklist

These items can be fixed immediately with minimal risk:

- [ ] Fix `.djvu`/`.djv` → `CBXTYPE_DJVU` in `GetCBXType()` (cbxArchive.h)
- [ ] Add `.obj`/`.stl`/`.gltf`/`.glb` → `CBXTYPE_MODEL` in `GetCBXType()` (cbxArchive.h)
- [ ] Remove `.heif`/`.heic` from `AVIFDecoder::m_extensions[]` (AVIFDecoder.cpp)
- [ ] Remove `.avif` from `HEIFDecoder::s_extensions[]` (HEIFDecoder.cpp)  
- [ ] Add `.avifs` to `AVIFDecoder::m_extensions[]` (AVIFDecoder.cpp)
- [ ] Add `.hif`/`.avci`/`.avcs` to `GetCBXType()` HEIF section (cbxArchive.h)
- [ ] Update `cbxArchive.h` header version from "v4.6" to "v8.3.0"
- [ ] Update `README.md` version badge and body to v8.3.0
- [ ] Update `README.md` sprint count from 74 to 174
- [ ] Register orphaned files in `Engine/CMakeLists.txt` or remove them

---

*Document generated from comprehensive source code vs documentation audit.*  
*Next review: After Sprint 179 completion.*
