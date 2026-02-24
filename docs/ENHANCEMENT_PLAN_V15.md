# ExplorerLens v15.0 Enhancement Plan — "Zenith"

**Date:** February 24, 2026
**Current Version:** 14.0.0 "Apex" (348 sprints completed)
**Target Version:** 15.0.0 "Zenith"
**Status:** Planning

---

## Executive Summary

ExplorerLens v14.0 is a mature, feature-rich Windows Shell Extension supporting 200+ file
formats across 50+ decoders with GPU acceleration, multi-tier caching, enterprise deployment,
and an advanced plugin ecosystem. This document identifies **concrete improvements** across
three pillars: Engine, External Libraries, and GUI/UX — prioritized into actionable sprint
blocks that will take the project from "feature-complete" to "production-polished."

---

## Part 1: Current State Assessment

### Strengths

| Area | Details |
|------|---------|
| **Format Coverage** | 50+ decoders, 200+ file extensions (archives, images, video, audio, documents, fonts, 3D models, scientific, geospatial) |
| **GPU Pipeline** | D3D11 + D3D12 + Vulkan + GDI fallback with vendor-specific routing (NVDEC/QuickSync/AMF) |
| **Build Quality** | 0 errors, 0 warnings, 122/122 compilation units, /W4 + /WX-ready |
| **Plugin System** | Full lifecycle: SDK, sandbox, trust chain, marketplace, hot-reload, debugger integration |
| **Caching** | Multi-tier (memory + disk + USN invalidation), sub-ms cache engine, adaptive budget |
| **Enterprise** | GPO/Intune/ConfigMgr policies, MSIX packaging, telemetry, diagnostics |
| **Testing** | ~1,187 unit tests, 5 benchmarks, 100% pass rate |
| **Architecture** | Clean interface separation (IThumbnailDecoder, IFormatDetector, IGPURenderer, ICacheProvider) |

### Weaknesses Found

| # | Issue | Severity | Area |
|---|-------|----------|------|
| W1 | **Version mismatch:** Engine.h says 6.0.0, README says 7.0.0, CMakeLists says 14.0.0 | High | Engine |
| W2 | **MuPDF not built:** HAS_MUPDF=OFF — PDF decoder is stub-only | High | External Libs |
| W3 | **LENSArchive.h is 2783 lines** — monolithic, hard to maintain | High | Shell |
| W4 | **LENSManager uses legacy WTL dialogs** — checkbox-only UI, no modern features | High | GUI/UX |
| W5 | **libwebp built with /MT** — CRT mismatch requires /NODEFAULTLIB:LIBCMT workaround | Medium | External Libs |
| W6 | **libheif/libde265 missing from LIBRARY_INVENTORY.md** | Medium | Docs |
| W7 | **UnRAR DLLs shipped in source tree** (1.2MB binary blobs) | Medium | Shell |
| W8 | **WMFDecoder_old.cpp exists** alongside WMFDecoder.cpp (dead code) | Low | Engine |
| W9 | **Memory pool not implemented** — marked as "Future Enhancement" in architecture doc | Medium | Engine |
| W10 | **PluginHost commented out** in CMakeLists.txt — out-of-process host not building | Medium | Engine |
| W11 | **Only 1 HLSL shader** (thumbnail_resize.hlsl) — no Lanczos/bicubic GPU shaders | Medium | Shell |
| W12 | **Many header-only stubs** — features declared but not wired into production path | Low | Engine |
| W13 | **OnApplyImpl() has 35x copy-paste blocks** — registry handler code is highly repetitive | Medium | GUI |
| W14 | **No IPropertyStore** — no property handler for file metadata in Explorer details pane | Medium | Shell |
| W15 | **README_ARCHITECTURE.md outdated** — describes 38 tests (actual: 1,187), stale CMake options | Low | Docs |

---

## Part 2: Enhancement Recommendations

### Category A — Critical Fixes (Sprints 349-353)

These are issues that should be fixed before any new features are added.

#### A1. Version Synchronization (Sprint 349)

**Problem:** Engine.h hardcodes version 6.0.0, Engine/README.md says 7.0.0, and the project
is actually 14.0.0. This creates confusion and can cause diagnostics/telemetry to report
wrong versions.

**Fix:**
- Update `Engine/Engine.h` version macros to 14.0.0 (or derive from CMake)
- Use `configure_file()` in CMake to generate a `Version.h` from CMakeLists.txt version
- Update `Engine/README.md` version line
- Update `Engine/README_ARCHITECTURE.md` stale test counts and CMake options

#### A2. Build MuPDF and Enable PDF Support (Sprint 350)

**Problem:** `HAS_MUPDF=OFF` means PDF thumbnails use only a stub decoder. MuPDF 1.24.11
source code is already in `external/pdf-libs/` but lacks a build script.

**Fix:**
- Create `build-scripts/external-libs/Build-MuPDF.ps1` following the existing library build pattern
- Build MuPDF with `/MD` CRT, Release configuration
- Set `HAS_MUPDF=ON` in CMakePresets and verify PDF decoder links
- Add MuPDF to `LIBRARY_INVENTORY.md`

#### A3. Rebuild libwebp with /MD CRT (Sprint 351)

**Problem:** libwebp was built with `/MT` (static CRT), forcing a `/NODEFAULTLIB:LIBCMT`
linker workaround. This is fragile and can cause subtle runtime issues.

**Fix:**
- Update `Build-LibWebP-NMake.ps1` to use `-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL`
- Rebuild libwebp 1.5.0 with `/MD`
- Remove `/NODEFAULTLIB:LIBCMT` from `Engine/CMakeLists.txt`
- Verify clean link without workaround

#### A4. Update LIBRARY_INVENTORY.md (Sprint 352)

**Problem:** libheif 1.19.5 and libde265 1.0.15 are used by the Engine but not listed in
the inventory table. MuPDF is listed but not flagged as disabled.

**Fix:**
- Add libheif 1.19.5, libde265 1.0.15 entries
- Add brotli, highway (hwy) entries (libjxl transitive dependencies)
- Mark MuPDF build status accurately
- Add a "Build Status" column (Built / Not Built / Optional)

#### A5. Dead Code Removal (Sprint 353)

**Problem:** `WMFDecoder_old.cpp` exists alongside the current `WMFDecoder.cpp`. The
`CBuffer` template in `LENSArchive.h` is commented out. Several stale references exist.

**Fix:**
- Delete `Engine/Decoders/WMFDecoder_old.cpp`
- Remove commented-out `CBuffer` class from `LENSArchive.h`
- Audit for other `_old` or `_backup` files
- Remove `unzip_new.cpp` if it duplicates `unzip.cpp`

---

### Category B — Architecture Improvements (Sprints 354-363)

#### B1. Refactor LENSArchive.h (Sprints 354-355)

**Problem:** 2783-line monolithic header containing ZIP/RAR wrappers, LENSTYPE enum,
CLENSArchive class, image type detection, and COM integration all in one file.

**Solution:**
- Extract `LENSTYPE` enum and constants → `LENSTypes.h`
- Extract `CUnzip` class → `ZipWrapper.h`
- Extract `CUnRar` class → `RarWrapper.h`
- Extract `IMGTYPE_*` constants → `ImageTypes.h`
- Keep `CLENSArchive` in `LENSArchive.h` but slim it down
- Update all `#include "LENSArchive.h"` references

#### B2. Implement Memory Pool (Sprint 356)

**Problem:** Architecture doc mentions a pre-allocated bitmap pool as "Future Enhancement"
but it's never been implemented. This could reduce allocation overhead by ~30%.

**Solution:**
- Implement bitmap pool in `Engine/Memory/BitmapPool.h/.cpp`
- Pre-allocate pools for common sizes: 128x128, 256x256, 512x512
- Add configurable pool size (default: 50 per size)
- Integrate with `ThumbnailPipeline::GenerateThumbnail()`

#### B3. Unify OnApplyImpl() Registry Handler (Sprint 357)

**Problem:** `LENSManager/MainDlg.cpp` `OnApplyImpl()` has 35 nearly identical blocks:
```cpp
bRet = (BST_CHECKED == Button_GetCheck(GetDlgItem(IDC_CB_XXX)));
if (bRet != m_reg.HasTH(LENS_XXX)) { bRefresh = TRUE; m_reg.SetHandlers(LENS_XXX, bRet); }
```

**Solution:**
- Create a mapping array: `{ IDC_CB_CBZ, LENS_CBZ }, { IDC_CB_CBR, LENS_CBR }, ...`
- Replace 35 blocks with a single loop
- Reduces `MainDlg.cpp` by ~200 lines

#### B4. Add IPropertyStore Handler (Sprints 358-359)

**Problem:** ExplorerLens provides thumbnails but no metadata in Explorer's Details pane.
Files show no custom properties (dimensions, format, codec info).

**Solution:**
- Implement `IPropertyStore` / `IPropertyStoreCapabilities` COM interfaces
- Register for supported file extensions
- Expose: image dimensions, color depth, codec name, file format version
- Requires new CLSID registration in Shell extension

#### B5. GPU Shader Library (Sprints 360-361)

**Problem:** Only 1 HLSL shader (`thumbnail_resize.hlsl`). The Engine headers declare
Lanczos, bicubic, and tone-mapping capabilities but these likely run on CPU.

**Solution:**
- Add `lanczos_resize.hlsl` — Lanczos3 kernel (sharper than bilinear)
- Add `bicubic_resize.hlsl` — Mitchell-Netravali bicubic
- Add `hdr_tonemap.hlsl` — Reinhard/ACES tone mapping for HDR → SDR
- Add `color_convert.hlsl` — Wide gamut (P3/Rec.2020) → sRGB
- Integrate shader selection based on source format

#### B6. Enable PluginHost Out-of-Process (Sprints 362-363)

**Problem:** `PluginHost/` exists but `add_subdirectory(PluginHost)` is commented out.
Plugins run in-process, meaning a crash in a plugin crashes Explorer.

**Solution:**
- Fix API mismatches blocking PluginHost build
- Build PluginHost.exe as separate process
- Implement named pipe IPC between LENSShell.dll and PluginHost.exe
- Add process supervision (restart on crash)

---

### Category C — External Library Upgrades (Sprints 364-368)

#### C1. Update Library Versions (Sprint 364)

Check for newer versions of core libraries:

| Library | Current | Latest (as of Feb 2026) | Action |
|---------|---------|------------------------|--------|
| zlib | 1.3.1 | 1.3.1 | Up to date |
| zstd | 1.5.7 | 1.5.7+ | Check minor |
| lz4 | 1.10.0 | 1.10.0 | Up to date |
| minizip-ng | 4.0.10 | 4.0.x | Check minor |
| libwebp | 1.5.0 | 1.5.0 | Up to date |
| libjxl | 0.11.1 | 0.11.x+ | Check |
| libavif | 1.3.0 | 1.3.x+ | Check |
| libheif | 1.19.5 | 1.19.x+ | Check |
| LibRaw | 0.21.3 | 0.21.x+ | Check |
| dav1d | 1.5.1 | 1.5.x+ | Check |
| MuPDF | 1.24.11 | 1.25.x? | Build first |

#### C2. Add OpenJPEG for JPEG 2000 (Sprint 365)

**Problem:** `JPEG2000Decoder.h` exists but JPEG 2000 support relies on WIC, which has
limited J2K support. OpenJPEG provides full JPEG 2000 Part 1/2 decoding.

**Solution:**
- Download OpenJPEG 2.5.x
- Create `build-scripts/external-libs/Build-OpenJPEG.ps1`
- Link into Engine, gate behind `HAS_OPENJPEG` flag
- Wire into `JPEG2000Decoder.cpp`

#### C3. Add FreeType for Enhanced Font Rendering (Sprint 366)

**Problem:** `FontDecoder` likely uses GDI for font preview, which has limited shaping
and no support for OpenType features like ligatures.

**Solution:**
- Add FreeType 2.x + HarfBuzz for advanced text shaping
- Generate richer font previews with sample text in multiple scripts
- Gate behind `HAS_FREETYPE` flag

#### C4. Add FFmpeg/libavcodec for Video Thumbnails (Sprints 367-368)

**Problem:** Video decoder uses Media Foundation (MF), which doesn't support MKV natively,
and has poor codec coverage for older/niche formats. Many video containers are handled
poorly.

**Solution:**
- Integrate FFmpeg's libavformat + libavcodec (LGPL-compatible)
- Build as DLL or dynamic load to maintain licensing flexibility
- Handle: MKV, WebM, FLV, AVI (DivX/Xvid), TS, RMVB, OGV
- Keep MF as fast path for MP4/MOV, FFmpeg as fallback

---

### Category D — GUI/UX Modernization (Sprints 369-378)

#### D1. Categorized Format Groups with Collapsible Sections (Sprint 369)

**Problem:** 35 checkboxes flat in a dialog with no visual hierarchy. Users can't
quickly find the format they care about.

**Solution (within current WTL framework):**
- Group checkboxes into collapsible sections: Archives, Comics, eBooks, Images, Media, Documents, Specialized
- Add group headers with Select All/None per group
- Use TreeView or custom owner-draw for collapsible groups

#### D2. Format Status Indicators (Sprint 370)

**Problem:** Users don't know if a format handler is actually working vs. registered
but non-functional (e.g., PDF when MuPDF is not built).

**Solution:**
- Add traffic-light icons next to each checkbox: green (working), yellow (degraded), red (unavailable)
- `DecoderHealthCheck.h` already exists — wire it to produce live status
- Show tooltip on hover: "WebP: Active — libwebp 1.5.0, GPU accelerated"

#### D3. Settings Import/Export Improvements (Sprint 371)

**Problem:** Config snapshot exists (`CaptureCurrentConfig()`, `LoadConfigFromFile()`)
but the UI for it is minimal.

**Solution:**
- Add Export Settings button (JSON format)
- Add Import Settings with preview dialog showing what will change
- Support drag-and-drop of config files
- Add "Reset to Defaults" button

#### D4. Performance Dashboard Tab (Sprint 372)

**Problem:** Engine collects rich statistics (cache hit rates, GPU vs CPU times, decoder
performance) but none of this is visible to users.

**Solution:**
- Add a "Performance" tab to LENSManager
- Show: cache hit rate %, avg thumbnail time, GPU utilization
- Show per-format decoder statistics
- Add "Run Benchmark" button using `PerformanceBenchmarkV2`

#### D5. Dark Mode Full Support (Sprints 373-374)

**Problem:** `DarkModeHelper.h` exists but WTL dialogs have limited dark mode support.
Custom drawing is needed for checkboxes, radio buttons, and status bar.

**Solution:**
- Implement owner-draw for all checkbox/radio controls in dark mode
- Dark theme for status bar, tooltips, buttons
- Follow Windows 10/11 dark mode detection via `ShouldAppsUseDarkMode()`
- Test with high contrast themes

#### D6. System Tray Integration (Sprint 375)

**Problem:** No way to quickly access settings or see status without opening LENSManager.

**Solution:**
- Add system tray icon when LENSManager is running
- Show context menu: Enabled formats, Performance stats, Open Settings
- Show balloon notifications on COM registration changes

#### D7. Modern UI Migration Plan (Sprints 376-378)

**Problem:** WTL is a 20-year-old framework with no touch support, no DPI-per-monitor
awareness, and poor accessibility. Long-term, the Manager needs modernization.

**Phase 1 (Sprint 376):** Research WinUI 3 + Windows App SDK feasibility
- Can XAML Islands work inside a utility that needs admin rights?
- Evaluate MSIX packaging requirements
- Prototype a single-page settings UI in WinUI 3

**Phase 2 (Sprint 377):** Implement hybrid approach
- Keep WTL dialog as fallback for older Windows versions
- WinUI 3 settings page for Windows 11+
- Shared backend (CRegManager) for both UIs

**Phase 3 (Sprint 378):** Feature parity
- Match all 35+ format toggles in new UI
- Add visual format gallery (icon grid with format thumbnails)
- Touch-friendly layout for tablets

---

### Category E — Quality & DevOps (Sprints 379-388)

#### E1. Continuous Integration Hardening (Sprint 379)

- Add GitHub Actions matrix: x64 Debug, x64 Release, ARM64 cross-compile
- Add automated test run on PR
- Add build badge to README.md
- Cache external library builds in CI

#### E2. Code Coverage Reporting (Sprint 380)

- Integrate OpenCppCoverage or MSVC `/PROFILE` for coverage
- Set coverage target: 80% for Engine/Core, 60% for Decoders
- Generate HTML coverage report in CI

#### E3. Integration Test Suite (Sprints 381-382)

- Create `tests/integration/` with real file corpus
- Test each decoder with actual files (not just stub data)
- Add `data/corpus/` samples for each supported format
- Measure actual decode times and compare to performance targets

#### E4. Fuzzing Campaign (Sprint 383)

- Wire `ContinuousFuzzEngine` into CI
- Generate fuzzed inputs for top 10 decoders
- Run with AddressSanitizer (ASan) enabled
- Set up crash bucket tracking

#### E5. Static Analysis CI Gate (Sprint 384)

- Run clang-tidy in CI (`.clang-tidy` already exists)
- Add cppcheck as secondary analyzer
- Block PR merge on new warnings

#### E6. SBOM and Supply Chain (Sprint 385)

- Generate SBOM from `LIBRARY_INVENTORY.md` automatically
- Verify library hashes against known-good values
- Add `SupplyChainIntegrityV2` verification in the build pipeline

#### E7. Documentation Refresh (Sprints 386-387)

- Update all README versions to 14.0.0 / 15.0.0
- Update architecture doc test counts (38 → 1,187)
- Update CMake options table in architecture doc
- Generate API reference from code comments (Doxygen or similar)
- Create "Getting Started" guide for plugin developers

#### E8. Installer Improvements (Sprint 388)

- Finalize WiX 6 MSI packaging (`ExplorerLens.wxs` exists)
- Add silent install mode for enterprise: `msiexec /i ExplorerLens.msi /qn`
- Add upgrade detection (major version migrations)
- MSIX Store packaging for Windows 11

---

### Category F — Performance Optimization (Sprints 389-393)

#### F1. Zero-Copy Pipeline Activation (Sprint 389)

`ZeroCopyPipeline.h` exists but may not be wired into the production path. Verify
and activate zero-copy upload from decoder → GPU → cache.

#### F2. Parallel I/O Pipeline (Sprint 390)

`ParallelIOPipeline.h` exists. Activate parallel file reads for batch thumbnail
scenarios (e.g., folder with 1000 images).

#### F3. SIMD Scaler Optimization (Sprint 391)

- Verify AVX2 path is active at runtime (not just compiled)
- Add ARM64 NEON scaler path for ARM builds
- Benchmark against known baselines:
  - Target: <12ms for 4K → 256x256 (AVX2)
  - Target: <20ms for 4K → 256x256 (NEON)

#### F4. Persistent PSO Cache (Sprint 392)

`PipelineStateCacheV2.h` exists for D3D12 PSO caching. Ensure PSOs are serialized
to disk and reloaded on startup to eliminate shader compilation stalls.

#### F5. Cache Warming Service (Sprint 393)

`CacheWarmingService.h` exists. Activate proactive cache warming:
- Monitor recently-opened folders
- Pre-generate thumbnails in background during idle
- Integrate with `WatchFolderEngine` for new file detection

---

## Part 3: Sprint Execution Roadmap

### Phase 1: Foundation (Sprints 349-353) — 1 week

| Sprint | Task | Category | Priority |
|--------|------|----------|----------|
| 349 | Version synchronization | A1 | Critical |
| 350 | Build MuPDF, enable PDF | A2 | Critical |
| 351 | Rebuild libwebp with /MD | A3 | Critical |
| 352 | Update LIBRARY_INVENTORY.md | A4 | Critical |
| 353 | Dead code removal | A5 | Critical |

### Phase 2: Architecture (Sprints 354-363) — 2 weeks

| Sprint | Task | Category | Priority |
|--------|------|----------|----------|
| 354-355 | Refactor LENSArchive.h | B1 | High |
| 356 | Implement Memory Pool | B2 | High |
| 357 | Unify OnApplyImpl() | B3 | High |
| 358-359 | IPropertyStore handler | B4 | High |
| 360-361 | GPU Shader Library | B5 | High |
| 362-363 | PluginHost out-of-process | B6 | High |

### Phase 3: Libraries (Sprints 364-368) — 1 week

| Sprint | Task | Category | Priority |
|--------|------|----------|----------|
| 364 | Library version audit + updates | C1 | Medium |
| 365 | OpenJPEG for JPEG 2000 | C2 | Medium |
| 366 | FreeType for fonts | C3 | Medium |
| 367-368 | FFmpeg for video | C4 | Medium |

### Phase 4: GUI/UX (Sprints 369-378) — 2 weeks

| Sprint | Task | Category | Priority |
|--------|------|----------|----------|
| 369 | Categorized format groups | D1 | High |
| 370 | Format status indicators | D2 | High |
| 371 | Settings import/export | D3 | Medium |
| 372 | Performance dashboard tab | D4 | Medium |
| 373-374 | Full dark mode | D5 | Medium |
| 375 | System tray integration | D6 | Low |
| 376-378 | WinUI 3 migration plan | D7 | Low |

### Phase 5: Quality & Performance (Sprints 379-393) — 3 weeks

| Sprint | Task | Category | Priority |
|--------|------|----------|----------|
| 379 | CI hardening | E1 | High |
| 380 | Code coverage | E2 | Medium |
| 381-382 | Integration tests | E3 | High |
| 383 | Fuzzing campaign | E4 | Medium |
| 384 | Static analysis gate | E5 | Medium |
| 385 | SBOM + supply chain | E6 | Low |
| 386-387 | Documentation refresh | E7 | Medium |
| 388 | Installer improvements | E8 | Medium |
| 389 | Zero-copy pipeline | F1 | High |
| 390 | Parallel I/O | F2 | High |
| 391 | SIMD scaler optimization | F3 | Medium |
| 392 | Persistent PSO cache | F4 | Medium |
| 393 | Cache warming service | F5 | Medium |

---

## Part 4: Quick Wins (Can Be Done Today)

These require minimal effort and have immediate impact:

1. **Fix Engine.h version** — 5 minutes, 3 line change
2. **Delete WMFDecoder_old.cpp** — 1 minute, reduces confusion
3. **Add libheif/libde265 to LIBRARY_INVENTORY.md** — 5 minutes
4. **Loop-ify OnApplyImpl()** — 30 minutes, removes 200 lines of copy-paste
5. **Update README_ARCHITECTURE.md test count** — 5 minutes

---

## Part 5: Metrics for v15.0 Release

| Metric | v14.0 | v15.0 Target |
|--------|-------|-------------|
| Format count | 200+ | 220+ |
| Decoder count | 50+ | 55+ |
| Build warnings | 0 | 0 |
| Unit tests | 1,187 | 1,500+ |
| Integration tests | 0 | 50+ |
| Code coverage | Unknown | 70%+ |
| Avg thumbnail time | 17ms | 12ms |
| Cache hit time | <5ms | <2ms |
| Max batch throughput | 235 img/s | 350 img/s |
| PDF support | Stub | Full (MuPDF) |
| JPEG 2000 support | WIC-only | OpenJPEG |
| GUI framework | WTL only | WTL + WinUI 3 prototype |
| CI matrix | Manual | Automated (3 configs) |

---

## Appendix: File Inventory of Items to Change

### Files to Modify

| File | Change |
|------|--------|
| `Engine/Engine.h` | Update version 6.0.0 → 14.0.0 |
| `Engine/README.md` | Update version 7.0.0 → 14.0.0 |
| `Engine/README_ARCHITECTURE.md` | Update test count, CMake options |
| `Engine/CMakeLists.txt` | Remove /NODEFAULTLIB:LIBCMT after libwebp rebuild |
| `external/LIBRARY_INVENTORY.md` | Add libheif, libde265, brotli, highway |
| `LENSShell/LENSArchive.h` | Extract types, remove dead code |
| `LENSManager/MainDlg.cpp` | Refactor OnApplyImpl() |

### Files to Delete

| File | Reason |
|------|--------|
| `Engine/Decoders/WMFDecoder_old.cpp` | Superseded by WMFDecoder.cpp |

### Files to Create

| File | Purpose |
|------|---------|
| `build-scripts/external-libs/Build-MuPDF.ps1` | Build MuPDF 1.24.11 |
| `build-scripts/external-libs/Build-OpenJPEG.ps1` | Build OpenJPEG 2.5.x |
| `LENSShell/LENSTypes.h` | Extracted LENSTYPE enum constants |
| `LENSShell/ZipWrapper.h` | Extracted CUnzip class |
| `LENSShell/RarWrapper.h` | Extracted CUnRar class |
| `Engine/Memory/BitmapPool.h` | Pre-allocated bitmap pool |
| `Engine/Memory/BitmapPool.cpp` | Pool implementation |
| `Engine/Version.h.in` | CMake version template |
| `LENSShell/shaders/lanczos_resize.hlsl` | Lanczos3 resize shader |
| `LENSShell/shaders/hdr_tonemap.hlsl` | HDR → SDR tone mapping |
| `docs/ENHANCEMENT_PLAN_V15.md` | This document |
