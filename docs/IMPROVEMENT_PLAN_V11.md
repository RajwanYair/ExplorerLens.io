# DarkThumbs — Comprehensive Improvement & Enhancement Plan v11

**Created:** February 20, 2026  
**Audit Baseline:** v10.5.0 (248 sprints completed, per copilot-instructions.md)  
**Source of Truth:** Source code analysis + full documentation audit  
**Target Version:** v11.0.0  
**Scope:** Version sync, code vs docs gaps, file format coverage, performance, compatibility, quality

---

## Table of Contents

1. [Executive Summary](#1-executive-summary)
2. [Critical Version & Documentation Drift](#2-critical-version--documentation-drift)
3. [Source Code vs Documentation Gap Analysis](#3-source-code-vs-documentation-gap-analysis)
4. [Engine Architecture Review](#4-engine-architecture-review)
5. [File Format Coverage — Current vs Best-in-Class](#5-file-format-coverage--current-vs-best-in-class)
6. [Shell Registration Audit](#6-shell-registration-audit)
7. [Code Quality & Implementation Gaps](#7-code-quality--implementation-gaps)
8. [Performance Enhancement Plan](#8-performance-enhancement-plan)
9. [Compatibility & Platform Improvements](#9-compatibility--platform-improvements)
10. [Testing & Quality Strategy](#10-testing--quality-strategy)
11. [Missing File Formats — Best-in-Class Roadmap](#11-missing-file-formats--best-in-class-roadmap)
12. [Prioritized Sprint Plan (v11.0.0)](#12-prioritized-sprint-plan-v110)
13. [Library Upgrade Roadmap](#13-library-upgrade-roadmap)
14. [Documentation Governance](#14-documentation-governance)

---

## 1. Executive Summary

### Audit Methodology

- Full source tree analysis: `cbxArchive.h` (2782 lines), `Engine/CMakeLists.txt` (711 lines), all 40+ decoder files, GPU pipeline, plugin system, memory, cache, pipeline modules
- Documentation review: All MD files (README, MASTER_PLAN, CHANGELOG, IMPROVEMENT_PLAN_V9, FORMAT_SUPPORT_MATRIX_V8, copilot-instructions.md)
- Shell registration analysis: `CBXShell.rgs` (1028 lines, 93 registered extensions)
- Cross-reference: copilot-instructions.md (v10.5.0, 248 sprints, 687 tests) vs actual code/docs state
- Engine directory analysis: Core/, Decoders/, GPU/, Cache/, Memory/, Pipeline/, Plugin/, Utils/, AI/, Cloud/, Codec/, Shell/ — 200+ source files

### Critical Finding: Massive Version Drift

The project has a **severe version identity crisis** spanning multiple layers:

| Source | Claimed Version | Claimed Sprints | Claimed Tests |
|--------|----------------|-----------------|---------------|
| `.github/copilot-instructions.md` | **v10.5.0** | **248** | **~687** |
| `CHANGELOG.md` (latest entry) | v8.4.0 | 177 | — |
| `README.md` | **v8.4.0** | 176 | 437 |
| `MASTER_PLAN.md` header | **v7.1.0** | 149-174 | 100 |
| `Engine/CMakeLists.txt` project() | **v7.0.0** | — | — |
| `FORMAT_SUPPORT_MATRIX_V8.md` | v8.4.0 | 178 | — |
| `IMPROVEMENT_PLAN_V9.md` | v8.3.0 → v9.0.0 | 174 | 437 |
| Sprint docs in `sprints-v8/` | 173 docs (SPRINT_76 through SPRINT_248) | — | — |

**Bottom line:** The copilot-instructions.md represents the intended latest state (v10.5.0, 248 sprints), but nearly every other document lags behind by 1-4 major versions. The sprint docs (199-248) exist but the CHANGELOG, README, and most docs haven't been updated to reflect Sprints 178-248.

### Impact Assessment

| Category | Finding | Severity |
|----------|---------|----------|
| Version drift | 6+ different version numbers across project | 🔴 Critical |
| CHANGELOG gap | Missing entries for v8.4.0 → v10.5.0 (Sprints 178-248) | 🔴 Critical |
| README stale | Still shows v8.4.0, 176 sprints, 437 tests | 🔴 Critical |
| MASTER_PLAN stale | Header says v7.1.0, body discusses up to Sprint 49 | 🔴 Critical |
| CMakeLists version | project(VERSION 7.0.0) — 3 major versions behind | 🟡 High |
| Header-only sprints | Many sprint headers (150-248) have no .cpp implementations | 🟡 High |
| Missing formats | ~30+ common formats have no decoder or CBXTYPE | 🟡 High |
| Format docs accurate | FORMAT_SUPPORT_MATRIX_V8.md is well-written and mostly accurate for implemented code | ✅ Good |
| Shell registrations | 93 extensions registered — solid coverage | ✅ Good |
| Decoder count | 40+ decoder files in Engine/Decoders/ — excellent | ✅ Good |

---

## 2. Critical Version & Documentation Drift

### 2.1 Files Requiring Immediate Version Update to v10.5.0

| File | Current Version | Action |
|------|----------------|--------|
| `README.md` | v8.4.0, 176 sprints, 437 tests | Update to v10.5.0, 248 sprints, ~687 tests |
| `CHANGELOG.md` | Latest entry v8.4.0 | Add entries for v9.0 through v10.5.0 |
| `MASTER_PLAN.md` | Header says v7.1.0 | Full rewrite — or archive and create v11 plan |
| `Engine/CMakeLists.txt` | project(VERSION 7.0.0) | Update to 10.5.0 |
| `Engine/Core/BuildConfig.h` | No version constant | Add `DT_VERSION_MAJOR/MINOR/PATCH` |
| `docs/IMPROVEMENT_PLAN_V9.md` | Targets v9.0.0 | Archive — superseded by this document |
| `docs/formats/FORMAT_SUPPORT_MATRIX_V8.md` | v8.4.0 (Sprint 178) | Update to v10.5.0 (Sprint 248) — add new decoders |
| `docs/LIBRARY_RESEARCH_2026.md` | v7.0.0 | Update version header |

### 2.2 CHANGELOG Gap Analysis

The CHANGELOG needs entries for these version blocks:

| Version | Sprints | Key Deliverables |
|---------|---------|-----------------|
| v8.4.0 | 175-177 | Bug fixes, shell expansion, version normalization ✅ Exists |
| v8.5.0? | 178-186 | Format expansion (WMF/EMF, PCX, Farbfeld, JP2, EPS, KTX/VTF, ORA/XCF, SGI/XPM) |
| v9.0.0? | 187-198 | Async shell, D3D12 compute, parallel decode, coverage, memory safety, cache v2, ARM64, MSIX |
| v10.0.0 | 199-204 | GPU Pipeline V2, D3D12 Compute, Shader Compiler, Pipeline Cache, GPU Memory Pool, Release Gate V5 |
| v10.1.0 | 205-209 | Async Decode, Thread Pool V2, Priority Queue, Decode Cache V2, Release Gate V6 |
| v10.2.0 | 210-214 | Format Detection V2, MIME Resolver, Codec Registry, Stream Analyzer, Release Gate V7 |
| v10.3.0 | 215-219 | ETW Provider V2, Perf Counters, Diagnostic Logger, Health Monitor, Release Gate V8 |
| v10.4.0 | 220-234 | Accessibility, Cloud Sync, Converter, Enterprise, Watch Folder, Diagnostics, Benchmark V2, Localization, Theme, Telemetry, Update Engine, Shell Preview, Batch Processing, Release Gate V12 |
| v10.5.0 | 235-248 | File Hash, Registry Manager, Error Recovery, Log Rotation, Resource Pool, CLI Parser, Metadata Extractor, Notifications, Content Indexer, Network Diagnostics, Config Migration, Release Gate V15 |

### 2.3 Sprint Documentation Coverage

- Sprint docs exist: SPRINT_76.md through SPRINT_248.md (173 files)
- Missing sprint docs: Sprints 1-75 are in `sprints-archive/` (separate folder)
- Coverage: Sprint docs for 199-248 exist and describe v10.x features

---

## 3. Source Code vs Documentation Gap Analysis

### 3.1 What Code Actually Has (Verified from Source Tree)

**Decoders (Engine/Decoders/) — 40+ files:**

| Decoder | Has .h | Has .cpp | Status |
|---------|--------|----------|--------|
| ArchiveDecoder | ✅ | ✅ | Production |
| AudioDecoder | ✅ | ✅ | Production |
| AVIFDecoder | ✅ | ✅ | Production |
| DDSDecoder | ✅ | ✅ | Production |
| DICOMDecoder | ✅ | ✅ | Implementation — needs library integration |
| DocumentDecoder | ✅ | ✅ | Production |
| EBookDecoder | ✅ | ✅ | Production |
| EPSDecoder | ✅ | ✅ | Production |
| EXRDecoder | ✅ | ✅ | Production |
| ExampleDecoder | ✅ | ✅ | SDK reference |
| FarbfeldDecoder | ✅ | ✅ | Production |
| FITSDecoder | ✅ | ✅ | Implementation — needs library integration |
| FontDecoder | ✅ | ✅ | Production |
| GeospatialDecoder | ✅ | ✅ | Implementation |
| HDRDecoder | ✅ | ✅ | Production |
| HEIFDecoder | ✅ | ✅ | Production |
| ICODecoder | ✅ | ✅ | Production |
| ImageDecoder | ✅ | ✅ | Production (WIC) |
| JPEG2000Decoder | ✅ | ✅ | Production |
| JXLDecoder | ✅ | ✅ | Production |
| JXRWICDecoder | ✅ | — | Header-only (WIC) |
| KTXTextureDecoder | ✅ | ✅ | Production |
| ModelDecoder | ✅ | ✅ | Production |
| OpenRasterDecoder | ✅ | ✅ | Production |
| PCXDecoder | ✅ | ✅ | Production |
| PDFDecoder | ✅ | ✅ | Production |
| PluginDecoder | ✅ | ✅ | Plugin bridge |
| PPMDecoder | ✅ | ✅ | Production |
| PSDDecoder | ✅ | ✅ | Production |
| QOIDecoder | ✅ | ✅ | Production |
| RAWDecoder | ✅ | ✅ | Production |
| SGIDecoder | ✅ | ✅ | Production |
| SVGDecoder | ✅ | ✅ | Production |
| TGADecoder | ✅ | ✅ | Production |
| VideoDecoder | ✅ | ✅ | Production |
| VTFDecoder | ✅ | ✅ | Production |
| WebPDecoder | ✅ | ✅ | Production |
| WMFDecoder | ✅ | ✅ | Production |
| XCFDecoder | ✅ | ✅ | Production |
| XPMDecoder | ✅ | ✅ | Production |

**Header-only modules (design/spec, various Engine/ dirs):**
- AnimatedThumbnailDecoder.h, ArchiveGridPreview.h, CADFormatPlugin.h
- ColorSpaceManager.h, EBookCoverExtractor.h, EPUBDecoder.h
- GLTFModelDecoder.h, OptimizedArchiveReader.h, ScientificFormatPlugin.h, VideoEnhancer.h
- Advanced3DFormatDecoder (.h + .cpp)

**GPU Pipeline:**
- D3D11Renderer (.h + .cpp) — Production
- D3D12ComputePipeline (.h + .cpp) — Production
- GDIRenderer (.h + .cpp) — CPU fallback
- VulkanComputePipeline (.h + .cpp) — Future/Linux

**Additional Engine Modules (Core/, Utils/, Memory/, etc.):**
- 80+ files covering caching, memory management, pipeline, observability, telemetry, accessibility, localization, security, etc.

### 3.2 What Docs Claim vs Reality

| Claim (Docs) | Actual (Code) | Gap |
|--------------|---------------|-----|
| "200+ file extensions" | ~200 extensions in GetCBXType() mapping | ✅ Accurate |
| "25 specialized decoders" | 40+ decoder files, ~30 with .cpp implementations | Docs undercounts |
| "93 shell registrations" | 93 extensions in CBXShell.rgs | ✅ Accurate |
| "687 unit tests" | Claimed in copilot-instructions — needs build verification | Unverified |
| D3D12 support | D3D12ComputePipeline.cpp exists | ✅ Exists |
| Vulkan support | VulkanComputePipeline.cpp exists | Exists — completion status unclear |
| ARM64 support | Build configs + toolchain files exist | Infrastructure only |
| DICOM decoder | DICOMDecoder.cpp exists (196 lines) | Needs library integration |
| FITS decoder | FITSDecoder.cpp exists | Needs library integration |
| Geospatial decoder | GeospatialDecoder.cpp exists | Needs library integration |

### 3.3 CBXTYPE Enum vs Decoder Coverage

The CBXTYPE enum in `cbxArchive.h` defines 50+ types (values 0-93). Cross-referencing with decoders:

**Well-covered (CBXTYPE + Decoder + Shell registration):**
- All archive types (ZIP/CBZ/7Z/RAR/TAR/BZIP2/ZSTD/LZMA/LZ4/ISO/CAB/CPIO/DEB)
- Modern images (WEBP/AVIF/HEIC/HEIF/JXL)
- Professional images (PSD/DDS/HDR/EXR/TGA/ICO/QOI/PPM)
- Camera RAW (27 extensions)
- Documents (DOCX/DOC/PPTX/PPT/XLSX/XLS/PDF/RTF/ODT/ODP/ODS)
- Fonts (TTF/OTF/WOFF/WOFF2/TTC)
- eBooks (EPUB/MOBI/AZW/FB2/DJVU)
- v8.4+ formats (WMF/EMF/PCX/Farbfeld/JP2/EPS/KTX/VTF/ORA/XCF/SGI/XPM)

**Missing CBXTYPE definitions for existing decoders:**
- DICOM (.dcm, .dicom) — DICOMDecoder exists, no CBXTYPE
- FITS (.fits, .fit, .fts) — FITSDecoder exists, no CBXTYPE
- Geospatial (.shp, .geotiff, .kml) — GeospatialDecoder exists, no CBXTYPE
- DPX/Cineon (.dpx, .cin) — No decoder, no CBXTYPE

---

## 4. Engine Architecture Review

### 4.1 Architecture Strengths

1. **Clean separation:** CBXShell (COM) → EngineAdapter → DarkThumbsEngine.lib → Decoders
2. **GPU pipeline:** D3D11 primary + D3D12 compute + GDI fallback + Vulkan future
3. **Plugin isolation:** PluginHost.exe process isolation via IPC
4. **Multi-tier cache:** Memory LRU → SQLite → Disk with adaptive budget management
5. **Memory management:** 5-tier pressure controller, compactor, zero-copy pipeline, hot-mode
6. **Observability:** ETW provider, structured logging, telemetry, diagnostics dashboard
7. **40+ decoders** covering all major format categories

### 4.2 Architecture Concerns

1. **cbxArchive.h is 2782 lines** — monolithic header handling format detection, archive logic, and type definitions. Should be split.
2. **`#define CBXTYPE int`** uses preprocessor defines instead of `enum class` — type safety concern.
3. **GetCBXType()** is a single massive if-else chain — should be a lookup table.
4. **Format→Decoder routing** happens in multiple places (GetCBXType, DecoderRegistry, individual decoder m_extensions) — desynchronization risk.
5. **No central format registry** — format metadata is scattered across cbxArchive.h defines, decoder source files, .rgs registration, and docs.

### 4.3 Architectural Improvements

| Improvement | Description | Impact |
|-------------|-------------|--------|
| Format Registry singleton | Central `FormatRegistry` that maps extension→CBXTYPE→Decoder→shell-registered | Eliminates routing bugs |
| CBXTYPE to enum class | Replace `#define CBXTYPE_X Y` with `enum class FormatType : uint16_t` | Type safety |
| Split cbxArchive.h | Separate into FormatTypes.h, ArchiveHandler.h, FormatDetector.h | Maintainability |
| Extension→Decoder LUT | Replace if-else chain with `std::unordered_map<wstring, FormatType>` | Performance + clarity |
| Auto-generate .rgs | Script to generate CBXShell.rgs from FormatRegistry data | Eliminates registration drift |

---

## 5. File Format Coverage — Current vs Best-in-Class

### 5.1 Current Format Count

| Category | Extensions | Decoders | Shell Registered |
|----------|-----------|----------|-----------------|
| Standard Images | 10 | ImageDecoder(WIC) | 0 (Windows native) |
| Modern Images | 14 | WebP/AVIF/HEIF/JXL | 7 |
| Professional Images | 20+ | PSD/DDS/HDR/EXR/TGA/ICO/QOI/WMF/PCX/FF/JP2/EPS/KTX/VTF/ORA/XCF/SGI/XPM | 30+ |
| Vector Graphics | 4 | SVGDecoder + WMFDecoder | 4 |
| Netpbm | 6 | PPMDecoder | 6 |
| Camera RAW | 27 | RAWDecoder(LibRaw) | 25 |
| Archives | 22+ | ArchiveDecoder | 16 |
| Video | 22 | VideoDecoder(MediaFoundation) | 0 |
| Audio | 14 | AudioDecoder(MediaFoundation) | 0 |
| Documents | 19 | DocumentDecoder | 12 |
| PDF | 1 | PDFDecoder | 1 |
| Fonts | 7 | FontDecoder | 5 |
| 3D Models | 8 | ModelDecoder | 8 |
| eBooks | 7 | DocumentDecoder | 7 |
| Scientific | 3+ | DICOM/FITS (partial) | 0 |
| **TOTAL** | **~200+** | **~30 production decoders** | **93** |

### 5.2 Competitor Analysis — What Best-in-Class Covers

Comparing against QuickLook (macOS), Icaros Shell Extensions, SageThumbs, and other Windows thumbnail providers:

| Format | DarkThumbs | Icaros | SageThumbs | QuickLook(mac) |
|--------|-----------|--------|------------|----------------|
| WebP | ✅ | ✅ | ✅ | ✅ |
| AVIF | ✅ | ❌ | ❌ | ✅ |
| HEIC/HEIF | ✅ | ❌ | ❌ | ✅ |
| JXL | ✅ | ❌ | ❌ | ❌ |
| PSD | ✅ | ❌ | ✅ | ✅ |
| Camera RAW (27) | ✅ | ❌ | ✅ | ✅ |
| SVG | ✅ | ❌ | ❌ | ✅ |
| PDF | ✅ | ❌ | ❌ | ✅ |
| 3D Models | ✅ | ❌ | ❌ | ✅ |
| Fonts | ✅ | ❌ | ❌ | ✅ |
| Archives (thumbnails) | ✅ | ❌ | ❌ | ✅ |
| EPS | ✅ | ❌ | ❌ | ✅ |
| JPEG 2000 | ✅ | ❌ | ❌ | ✅ |
| FITS | 🔄 | ❌ | ❌ | ❌ |
| DICOM | 🔄 | ❌ | ❌ | ❌ |
| DPX/Cineon | ❌ | ❌ | ❌ | ✅ |
| USD/USDZ | ❌ | ❌ | ❌ | ✅ |
| Markdown (.md) | ❌ | ❌ | ❌ | ✅ |
| Source Code | ❌ | ❌ | ❌ | ✅ |

**DarkThumbs already leads in image format breadth.** Key gaps vs QuickLook are DPX/Cineon, USD, and preview-for-text formats.

---

## 6. Shell Registration Audit

### 6.1 Current Registration (93 extensions in CBXShell.rgs)

**Archives (16):** .cbz, .cbr, .cb7, .cbt, .7z, .rar, .zip, .tar, .tgz, .bz2, .xz, .zst, .iso, .cab, .cpio, .deb

**eBooks (8):** .epub, .mobi, .azw, .azw3, .fb2, .djvu, .djv, .phz

**Modern Images (7):** .webp, .avif, .avifs, .heic, .heif, .hif, .jxl

**Vector/Design (2):** .svg, .svgz

**Professional (8):** .psd, .psb, .dds, .hdr, .exr, .tga, .qoi, .ico, .cur

**Netpbm (6):** .ppm, .pgm, .pbm, .pnm, .pam, .pfm

**Camera RAW (22):** .cr2, .cr3, .crw, .nef, .nrw, .arw, .srf, .sr2, .dng, .orf, .rw2, .raf, .pef, .dcr, .mrw, .x3f, .srw, .rwl, .3fr, .iiq, .erf, .kdc, .mef, .gpr

**PDF (1):** .pdf

**Documents (10):** .docx, .doc, .pptx, .ppt, .xlsx, .xls, .rtf, .odt, .odp, .ods, .xps

**Fonts (5):** .ttf, .otf, .woff, .woff2, .ttc

**3D Models (8):** .obj, .stl, .gltf, .glb, .fbx, .3ds, .ply, .dae

**v8.4+ Formats (10):** .wmf, .emf, .pcx, .ff, .jp2, .j2k, .j2c, .jpx, .jph, .eps, .epsf, .ps, .ai, .ktx, .ktx2, .vtf, .ora, .xcf, .sgi, .bw, .xpm

### 6.2 Extensions With Code Support But No Shell Registration

These should be considered for addition:

| Extension | CBXTYPE | Decoder | Risk | Recommendation |
|-----------|---------|---------|------|----------------|
| .lz4 | CBXTYPE_LZ4 (21) | ArchiveDecoder | Low | ✅ Add |
| .tbz, .txz, .tzst | TAR variants | ArchiveDecoder | Low | ✅ Add |
| .xar, .ar | Archive types | ArchiveDecoder | Low | ✅ Add |
| .chm | CBXTYPE_CHM (23) | DocumentDecoder | Low | ✅ Add |
| .raw, .ptx, .r3d | RAW camera | RAWDecoder | Low | ✅ Add |
| .rgb, .rgba | SGI variants | SGIDecoder | Low | ✅ Add |

### 6.3 Formats NOT Recommended for Shell Registration

| Extension | Reason |
|-----------|--------|
| .mp4/.mkv/.avi/etc (video) | Conflicts with Windows native video thumbnails |
| .mp3/.flac/.wav/etc (audio) | Conflicts with Windows media thumbnails |
| .jpg/.png/.bmp/.gif (standard images) | Windows handles these natively and well |

---

## 7. Code Quality & Implementation Gaps

### 7.1 Header-Only Sprint Audit

Many sprints (especially 150-174 and 199-248) produced header files with class definitions but the actual implementation varies. Per copilot-instructions.md, the test framework uses custom macros (`TEST()`, `RUN_TEST()`, `ASSERT()`) — not GTest — so these are lightweight test coverage.

**Assessment: Sprints 199-248 headers provide architectural design with inline implementations.** Many use `struct` and inline methods that compile as header-only. This is valid for configuration/data classes but insufficient for complex logic.

### 7.2 Decoders Needing Library Integration

| Decoder | Current State | Required Library | Priority |
|---------|--------------|-----------------|----------|
| DICOMDecoder | Stub with data structures | DCMTK or custom DICOM parser | 🔵 P2 — Niche |
| FITSDecoder | Stub with data structures | cfitsio | 🔵 P2 — Niche |
| GeospatialDecoder | Stub with data structures | GDAL or custom | 🔵 P3 — Very Niche |

### 7.3 Code Quality Improvements

| Item | Current | Improvement | Priority |
|------|---------|-------------|----------|
| CBXTYPE defines | `#define CBXTYPE int` | `enum class FormatType : uint16_t` | 🟡 P1 |
| GetCBXType() | 200+ line if-else | `std::unordered_map` lookup table | 🟡 P1 |
| cbxArchive.h size | 2782 lines monolith | Split into 3-4 focused headers | 🟡 P1 |
| Format registration | Manual sync (.rgs, GetCBXType, decoder arrays) | Auto-generated from single source | 🟡 P1 |
| Error handling | Mixed patterns | Consistent error_code / expected<T> | 🔵 P2 |
| Decoder interface | Virtual + mixed patterns | Clean IThumbnailDecoder interface enforcement | 🔵 P2 |

---

## 8. Performance Enhancement Plan

### 8.1 Current Performance Targets (from docs)

- Single thumbnail: 17ms
- Batch throughput: 235 img/sec
- Cache hit: <5ms
- Build time: ~55 sec (Ninja)

### 8.2 Enhancement Opportunities

| Enhancement | Expected Gain | Effort | Priority |
|-------------|--------------|--------|----------|
| **D3D12 compute pipeline activation** | 2-3x GPU throughput | Medium (code exists) | 🟡 P1 |
| **Async IThumbnailProvider** | No Explorer freeze | Medium (AsyncShellExtension exists) | 🟡 P1 |
| **SIMD-accelerated resize** | 2x CPU decode speed | Low (SIMDAccelerator/SIMDScaler exist) | 🟡 P1 |
| **Thread pool batch decode** | 3-5x batch throughput | Medium (ParallelBatchDecoder exists) | 🟡 P1 |
| **Memory-mapped I/O for large files** | 35% p95 latency reduction | Low (design exists) | 🟡 P1 |
| **Persistent disk cache** | Survive process restart | Medium (PersistentDiskCache exists) | 🔵 P2 |
| **USN journal cache invalidation** | 80% fewer stale thumbnails | Medium (USNCacheInvalidation.h exists) | 🔵 P2 |
| **Decoder priority queue** | Better perceived speed | Low (DecoderPriority.h exists) | 🔵 P2 |
| **Pre-decode size estimation** | Skip oversized images | Low | 🔵 P2 |
| **Vulkan compute backend** | Linux/Wine compatibility | Very High | 🔵 P3 |

### 8.3 Build Performance

| Improvement | Current | Target |
|-------------|---------|--------|
| Ninja build | ~55 sec | <45 sec (precompiled headers) |
| MSBuild | ~75 sec | <60 sec |
| Incremental rebuild | Varies | <15 sec (unity build for Engine) |

---

## 9. Compatibility & Platform Improvements

### 9.1 Current Platform Support

| Platform | Status | Notes |
|----------|--------|-------|
| Windows 10 x64 (1809+) | ✅ Full support | Primary target |
| Windows 11 x64 | ✅ Full support | Primary target |
| Windows 11 ARM64 | 🔄 Build config only | No hardware validation |
| Windows Server 2019+ | ❓ Untested | Should work |

### 9.2 Platform Roadmap

| Enhancement | Description | Priority |
|-------------|-------------|----------|
| ARM64 hardware validation | Real ARM64 device testing (Surface Pro X, etc.) | 🟡 P1 |
| Windows 11 24H2 features | Tabbed Explorer, modern context menus | 🟡 P1 |
| Per-monitor DPI awareness | Correct thumbnail sizes across different DPI monitors | 🟡 P1 |
| MSIX packaging | Modern app packaging with auto-update | 🟡 P1 |
| Microsoft Store submission | Broader distribution channel | 🔵 P2 |
| Dark mode thumbnail backgrounds | Adapt to system theme | 🔵 P2 |
| Windows on ARM (native AArch64) | Full native ARM64 binary | 🔵 P2 |
| HDR display thumbnail rendering | Correct tone mapping for HDR monitors | 🔵 P3 |

---

## 10. Testing & Quality Strategy

### 10.1 Current State

- **Framework:** Custom macros (TEST/RUN_TEST/ASSERT) + Google Benchmark
- **Tests:** ~687 (per copilot-instructions.md)
- **Pass rate:** 100%
- **Coverage tool:** None
- **Fuzz testing:** Design exists (SEHFuzzingEngine.h, ContinuousFuzzEngine.h) — implementation unclear

### 10.2 Testing Improvements

| Enhancement | Description | Priority |
|-------------|-------------|----------|
| Code coverage measurement | OpenCppCoverage or MSVC profiling | 🟡 P1 |
| Real file test corpus | Curated test files for every supported format | 🟡 P1 |
| Integration tests | End-to-end COM thumbnail extraction tests | 🟡 P1 |
| Fuzz testing activation | LibFuzzer/WinAFL for decoder inputs | 🟡 P1 |
| ASAN/MSAN | AddressSanitizer for memory safety | 🟡 P1 |
| Performance regression CI | Automated benchmark comparison in CI | 🔵 P2 |
| Stress testing | 10K+ file batch processing soak test | 🔵 P2 |
| Visual regression | Reference thumbnail comparison | 🔵 P2 |

### 10.3 Test Coverage Targets

| Component | Current (est.) | Target | Gap |
|-----------|---------------|--------|-----|
| Core decoders (30+) | ~200 | 300 (10/decoder) | 100 |
| Pipeline & cache | ~100 | 150 | 50 |
| GPU rendering | ~20 | 60 | 40 |
| Memory system | ~80 | 100 | 20 |
| Plugin system | ~50 | 80 | 30 |
| Shell integration | ~10 | 40 | 30 |
| v10.x features | ~250 | 250 | 0 |
| **Total** | **~687** | **~980** | **~270** |

---

## 11. Missing File Formats — Best-in-Class Roadmap

### 11.1 Tier 1 — Easy Wins (Low effort, high user demand)

| Format | Extension(s) | Implementation | Why |
|--------|-------------|----------------|-----|
| Animated PNG | .apng | ImageDecoder WIC + frame extraction | Common web format |
| DPX | .dpx | Custom header parser (simple format) | Film/TV post-production |
| Cineon | .cin | Custom header parser | Film scanning |
| JFIF/EXIF variants | .jfif, .jpe | Already handled by WIC | Verify coverage |
| PIX (Alias) | .pix | Custom parser | 3D industry |
| .pic (Softimage) | .pic | Custom parser | VFX industry |
| RGBE / .pic (Radiance) | .pic, .rgbe | HDRDecoder extension | Already partially handled |

### 11.2 Tier 2 — Medium Effort, High Value

| Format | Extension(s) | Library/Approach | Why |
|--------|-------------|-----------------|-----|
| FLIF | .flif | libflif | Predecessor to JXL, archival use |
| BPG | .bpg | libbpg | Better Portable Graphics |
| 3MF | .3mf | lib3mf / ZIP+XML | 3D printing standard |
| USD/USDZ | .usd, .usda, .usdc, .usdz | OpenUSD (large dependency) | Apple/Pixar format, growing |
| Markdown | .md | Custom renderer → bitmap | Developer files (unique feature) |
| Source code | .c, .cpp, .py, .js, .ts | Syntax highlighting → bitmap | Developer files (unique feature) |
| CSV data preview | .csv | Table rendering → bitmap | Common data format |
| JSON preview | .json | Formatted text → bitmap | Common config format |

### 11.3 Tier 3 — High Effort, Niche Value

| Format | Extension(s) | Library | Why |
|--------|-------------|---------|-----|
| DICOM completion | .dcm, .dicom | DCMTK | Medical imaging — decoder stub exists |
| FITS completion | .fits, .fit, .fts | cfitsio | Astronomy — decoder stub exists |
| HDF5 | .h5, .hdf5 | HDF5 lib | Scientific data |
| NetCDF | .nc, .cdf | NetCDF lib | Climate data |
| NIfTI | .nii, .nii.gz | Custom | Neuroimaging |
| STEP/IGES | .stp, .step, .igs | Open CASCADE | CAD industry standard |
| CDR | .cdr | libcdr | CorelDRAW |
| Visio | .vsd, .vsdx | Custom/COM | Microsoft Visio |

### 11.4 Tier 4 — Unique Differentiators

These would make DarkThumbs truly unique among thumbnail providers:

| Feature | Description | Impact |
|---------|-------------|--------|
| **Markdown rendering** | Render .md files as formatted document thumbnails | Developer appeal |
| **Code syntax highlighting** | Render source code files with syntax colors | Developer appeal |
| **Notebook preview** | Render .ipynb (Jupyter) as document thumbnail | Data science appeal |
| **Spreadsheet preview** | Render CSV/TSV as mini table thumbnail | Business appeal |
| **ZIP content preview** | Show grid of contained images (already partially done) | Archive users |
| **Database schema preview** | Render .sqlite schema as diagram thumbnail | Developer appeal |

---

## 12. Prioritized Sprint Plan (v11.0.0)

### Phase 0: Version Synchronization (Sprint 249) — CRITICAL

| Sprint | Title | Deliverables |
|--------|-------|-------------|
| 249 | **Version Sync & Doc Cleanup** | 1. Update README.md to v10.5.0/248 sprints/687 tests. 2. Update Engine/CMakeLists.txt project(VERSION 10.5.0). 3. Add CHANGELOG entries for v9.0→v10.5 (summary per block). 4. Archive MASTER_PLAN.md as MASTER_PLAN_V7.md, create MASTER_PLAN_V11.md. 5. Archive IMPROVEMENT_PLAN_V9.md. 6. Update FORMAT_SUPPORT_MATRIX_V8.md → FORMAT_SUPPORT_MATRIX_V10.md with new decoders. 7. Add version constants to BuildConfig.h |

### Phase 1: Architecture & Quality Foundation (Sprints 250-254) — v10.6.0

| Sprint | Title | Key Deliverables |
|--------|-------|-----------------|
| 250 | **Format Registry Refactor** | Replace #define CBXTYPE with enum class FormatType. Create FormatRegistry singleton mapping extension→type→decoder. Auto-validate .rgs sync. |
| 251 | **cbxArchive.h Split** | Split monolithic header into FormatTypes.h, ArchiveHandler.h, FormatDetector.h. Replace GetCBXType() if-else with lookup table. |
| 252 | **Shell Registration Expansion V2** | Add ~15 missing extensions (.lz4, .tbz, .txz, .chm, .raw, .ptx, .r3d, .rgb, .rgba, .xar, .ar). Auto-gen .rgs validator script. |
| 253 | **Test Infrastructure Upgrade** | Add OpenCppCoverage integration. Create curated test file corpus (1 file per format, 50+ formats). Add ASAN build configuration. |
| 254 | **Release Gate V16** | Validate all v10.6 changes. Version sync verification. Build 0 errors/0 warnings gate. |

### Phase 2: New Format Decoders (Sprints 255-261) — v11.0.0

| Sprint | Title | Key Deliverables |
|--------|-------|-----------------|
| 255 | **DPX/Cineon Decoder** | Production DPX decoder with 10-bit film log support. Cineon decoder. CBXTYPE_DPX, CBXTYPE_CIN definitions. Shell registration. |
| 256 | **APNG & Animated Format Enhancement** | Validate APNG handling via WIC. Add animated WebP/JXL first-frame extraction improvement. |
| 257 | **Markdown/Code Preview Decoder** | Custom text→bitmap renderer with syntax highlighting. Support .md, .txt, .c, .cpp, .py, .js, .ts, .json. Shell registration for code files. |
| 258 | **DICOM Decoder Completion** | Integrate DCMTK or implement minimal DICOM parser for common transfer syntaxes. Add .dcm/.dicom to CBXTYPE and .rgs. |
| 259 | **FITS Decoder Completion** | Integrate cfitsio or implement minimal FITS reader for 2D images. Add .fits/.fit/.fts to CBXTYPE and .rgs. |
| 260 | **3MF/USD Format Support** | 3MF decoder (ZIP+XML+mesh). USD evaluation (may defer due to dependency size). |
| 261 | **Release Gate V17** | Format validation, decoder test coverage, shell registration audit. |

### Phase 3: Performance Activation (Sprints 262-267) — v11.1.0

| Sprint | Title | Key Deliverables |
|--------|-------|-----------------|
| 262 | **D3D12 Pipeline Activation** | Activate D3D12ComputePipeline for real GPU workloads. D3D11 automatic fallback. Benchmark comparison. |
| 263 | **Async Shell Extension** | Activate AsyncThumbnailProvider for non-blocking Explorer. Progress indicator for slow decodes. |
| 264 | **SIMD Acceleration** | Activate SIMDAccelerator/SIMDScaler for resize operations. AVX2 fast paths. SSE4.1 fallback. |
| 265 | **Parallel Batch Decode** | Activate ParallelBatchDecoder. Thread pool with per-format concurrency control. |
| 266 | **Persistent Cache & USN** | Activate PersistentDiskCache. Add USN journal cache invalidation. Cache warming on folder open. |
| 267 | **Release Gate V18** | Performance regression gates. Benchmark targets: <12ms single, >400 img/sec batch, <3ms cache. |

### Phase 4: Platform & Distribution (Sprints 268-273) — v11.2.0

| Sprint | Title | Key Deliverables |
|--------|-------|-----------------|
| 268 | **ARM64 Validation Sprint** | Real ARM64 hardware testing. Fix any ARM64-specific issues. NEON SIMD paths. |
| 269 | **MSIX Packaging** | Modern MSIX package. Proper capabilities declarations. Auto-update foundation. |
| 270 | **Windows 11 24H2 Integration** | Modern context menu integration. Tabbed Explorer thumbnail refresh. |
| 271 | **Test Suite Expansion** | Add 100+ real-file decoder tests. Integration tests for COM IThumbnailProvider. |
| 272 | **Fuzz Testing Activation** | WinAFL or LibFuzzer harness for all decoders. Corrupt file handling validation. |
| 273 | **v11.2 Release Gate** | Full platform validation. ARM64 + x64 + Windows 10/11 matrix. |

### Phase 5: Advanced Features (Sprints 274-280) — v12.0.0

| Sprint | Title | Key Deliverables |
|--------|-------|-----------------|
| 274 | **Vulkan Compute Backend** | Complete VulkanComputePipeline. Linux/Wine compatibility path. |
| 275 | **Plugin Marketplace V2** | Online plugin discovery. Signed plugin distribution. |
| 276 | **AI-Enhanced Thumbnails** | DirectML super-resolution. Content-aware cropping. |
| 277 | **Spreadsheet/Data Preview** | CSV/TSV/JSON rendered as mini-table thumbnails. |
| 278 | **USD/USDZ Support** | Pixar/Apple 3D format support. |
| 279 | **Auto-Update Engine** | Built-in update mechanism. Delta updates. |
| 280 | **v12.0 Release** | Major release with full documentation, marketing materials. |

---

## 13. Library Upgrade Roadmap

### 13.1 Current Library Versions

| Library | Current | Latest Known | Status | Update Priority |
|---------|---------|-------------|--------|----------------|
| zlib | 1.3.1 | 1.3.1 | ✅ Current | — |
| LZ4 | 1.10.0 | 1.10.0 | ✅ Current | — |
| zstd | 1.5.7 | 1.5.7 | ✅ Current | — |
| LZMA SDK | 26.00 | 26.00 | ✅ Current | — |
| minizip-ng | 4.0.10 | 4.0.10 | ✅ Current | — |
| UnRAR | 7.2.2 | 7.2.2 | ✅ Current | — |
| libwebp | 1.5.0 | 1.5.0 | ✅ Current | — |
| libavif | 1.3.0 | Check | 🔵 Monitor | Low |
| libjxl | 0.11.1 | Check | 🔵 Monitor | Low |
| libheif | 1.19.5 | Check | 🔵 Monitor | Low |
| libde265 | 1.0.15 | Check | 🔵 Monitor | Low |
| LibRaw | 0.21.3 | Check | 🔵 Monitor | Low |
| dav1d | 1.5.1 | Check | 🔵 Monitor | Low |

### 13.2 New Libraries to Evaluate

| Library | Purpose | License | Priority |
|---------|---------|---------|----------|
| DCMTK | DICOM medical image decoding | BSD | 🔵 P2 |
| cfitsio | FITS astronomical image decoding | MIT-like | 🔵 P2 |
| lib3mf | 3MF 3D printing format | BSD | 🔵 P2 |
| OpenJPEG | JPEG 2000 (if WIC+built-in insufficient) | BSD | 🔵 P2 |
| PDFium | PDF rendering (BSD alternative to MuPDF AGPL) | BSD | 🟡 P1 |
| OpenUSD | USD/USDZ 3D format | Apache 2.0 | 🔵 P3 |
| tree-sitter | Syntax highlighting for code preview | MIT | 🔵 P2 |
| cmark | Markdown parsing/rendering | BSD | 🔵 P2 |

---

## 14. Documentation Governance

### 14.1 Proposed Documentation Structure

```
docs/
├── INDEX.md                          — Master documentation index
├── architecture/
│   ├── PROJECT_STRUCTURE.md          — Directory layout
│   ├── DECODER_ROUTING.md            — NEW: Extension→CBXTYPE→Decoder flow
│   ├── FORMAT_REGISTRY.md            — NEW: Auto-generated format registry
│   └── INTEGRATION_ARCHITECTURE.md   — System architecture
├── formats/
│   ├── FORMAT_SUPPORT_MATRIX.md      — Current version (auto-generated)
│   ├── DECODER_STATUS.md             — Per-decoder implementation status
│   └── CAPABILITY_AUDIT.md           — Format capability verification
├── development/
│   ├── BUILD_QUICK_REFERENCE.md      — Build commands
│   ├── CODE_QUALITY_STANDARDS.md     — Coding standards
│   ├── sprints-v8/                   — Sprint 76-248 docs
│   └── sprints-archive/              — Sprint 1-75 docs
├── release-notes/
│   ├── RELEASE_NOTES_v10.5.0.md      — NEW
│   └── [older versions]
└── [other existing dirs]
```

### 14.2 Governance Rules

| Rule | Description |
|------|-------------|
| **Version sync on release** | All docs updated to match release version |
| **Automated format matrix** | Script generates FORMAT_SUPPORT_MATRIX from cbxArchive.h + .rgs |
| **Doc freshness CI check** | Script flags any MD file with version older than current |
| **Single source of truth** | copilot-instructions.md is authoritative for version/sprint count |
| **Sprint Definition of Done** | Must include: .h, .cpp (if applicable), tests, sprint doc, CMake registration |
| **No orphaned files** | Every .h/.cpp in Engine/ must be in CMakeLists.txt |
| **CHANGELOG discipline** | Every version bump gets a CHANGELOG entry before merge |

### 14.3 Auto-Sync Script Specification

Create `build-scripts/utilities/Sync-Documentation.ps1` that:
1. Reads version from `Engine/CMakeLists.txt` project(VERSION)
2. Scans all .md files for version strings
3. Reports mismatches
4. Optionally auto-updates version strings
5. Generates FORMAT_SUPPORT_MATRIX.md from cbxArchive.h CBXTYPE defines + CBXShell.rgs

---

## Summary: Top 10 Actions for v11.0.0

| # | Action | Impact | Effort |
|---|--------|--------|--------|
| 1 | **Fix version drift** — sync all docs to v10.5.0 | Credibility | Low |
| 2 | **Add CHANGELOG entries** for v9.0→v10.5 | Traceability | Low |
| 3 | **Refactor CBXTYPE to enum class** + lookup table | Code quality | Medium |
| 4 | **Split cbxArchive.h** into focused headers | Maintainability | Medium |
| 5 | **Add DPX/Cineon decoder** | Format coverage | Low |
| 6 | **Add Markdown/Code preview decoder** | Unique differentiator | Medium |
| 7 | **Activate D3D12 compute pipeline** | Performance 2-3x | Medium |
| 8 | **Activate async shell extension** | UX (no Explorer freeze) | Medium |
| 9 | **Complete DICOM/FITS decoders** | Scientific market | High |
| 10 | **ARM64 hardware validation** | Platform reach | Medium |

---

*This document supersedes IMPROVEMENT_PLAN_V9.md*  
*Next review: After Sprint 254 completion*  
*Auto-generated sections should be refreshed via Sync-Documentation.ps1*
