# DarkThumbs Development Roadmap v6.2

**Current Version:** v6.2.0  
**Last Updated:** June 2025  
**Build Status:** 0 errors, 0 warnings  
**Target Platform:** Windows 10 21H2+ / Windows 11  

---

## 1. Project Summary

DarkThumbs is a **GPU-accelerated Windows shell extension** generating thumbnails for **150+ file formats** in Windows Explorer. Written in C++20 with DirectX 11, it ships as a COM DLL (`CBXShell.dll`) implementing `IThumbnailProvider` + `IExtractImage2`, a WTL management app (`CBXManager.exe`), and a reusable engine library (`DarkThumbsEngine.lib`).

### 1.1 What Ships Today (v6.2.0)

| Component | Status | Size |
|-----------|--------|------|
| DarkThumbsEngine.lib | Production-ready, zero warnings (`/WX`) | 7.48 MB |
| CBXShell.dll (Shell Extension) | COM handler, 55 registered extensions | 1.43 MB |
| CBXManager.exe (Management UI) | Working WTL app with dark mode + health check | 305 KB |
| Unit Tests | 70+ tests | — |

### 1.2 Architecture Overview

```
┌─────────────────────────────────────────────┐
│              Windows Explorer                │
│         (IThumbnailProvider / IExtractImage2) │
├─────────────────────────────────────────────┤
│              CBXShell.dll (COM)              │
│  ┌───────────────┐  ┌────────────────────┐  │
│  │ Legacy Path   │  │ Engine Pipeline    │  │
│  │ (CCBXArchive) │  │ (ThumbnailPipeline)│  │
│  │ ~10 handlers  │  │ DecoderRegistry    │  │
│  └───────────────┘  └────────────────────┘  │
├─────────────────────────────────────────────┤
│             DarkThumbsEngine.lib            │
│  ┌──────────┐ ┌──────┐ ┌───────┐ ┌──────┐  │
│  │ Decoders │ │ GPU  │ │ Cache │ │Plugin│  │
│  │ (15+)    │ │ D3D11│ │ Disk  │ │ SDK  │  │
│  └──────────┘ └──────┘ └───────┘ └──────┘  │
└─────────────────────────────────────────────┘
```

### 1.3 Engine Decoders (v6.2.0)

| Format | Decoder | Status |
|--------|---------|--------|
| JPEG/PNG/BMP/GIF/TIFF | WIC ImageDecoder | **Production** |
| WebP | libwebp | **Production** |
| AVIF | libavif + dav1d | **Production** |
| ICO/CUR | WIC ICODecoder | **Production** |
| TGA | Custom TGADecoder | **Production** |
| QOI | Custom QOIDecoder | **Production** |
| RAW (CR2/NEF/ARW/DNG + 20 more) | WIC RAWDecoder | **Production** |
| Archives (CBZ/CBR/CB7/ZIP/RAR/7Z) | minizip-ng, unrar, lzma | **Production** |
| PSD/PSB (Adobe Photoshop) | Custom PSDDecoder + RLE | **Production v6.1** |
| DDS (DirectDraw Surface) | WIC DDSDecoder | **Production v6.1** |
| HDR (Radiance RGBE) | Custom HDRDecoder + SSE tone mapping | **Production v6.1** |
| PPM/PGM/PBM/PNM/PAM/PFM | Custom PPMDecoder | **Production v6.1** |
| EXR (OpenEXR) | WIC EXRDecoder | **Production v6.1** |
| SVG/SVGZ | SVGDecoder (GDI+ placeholder) | **New v6.2** |
| Video (MP4/MKV/AVI/MOV/WMV +15) | VideoDecoder (Media Foundation) | **New v6.2** |
| Audio (MP3/FLAC/WAV/OGG +10) | AudioDecoder (ID3v2/FLAC album art) | **New v6.2** |
| PDF | PDFDecoder (Shell provider) | **New v6.2** |
| Documents (EPUB/DOCX/MOBI +16) | DocumentDecoder (Shell provider) | **New v6.2** |
| Fonts (TTF/OTF/WOFF +4) | FontDecoder (Shell preview) | **New v6.2** |

### 1.4 Legacy Shell Decoders (Being Migrated to Engine)

| Format | Handler | Status |
|--------|---------|--------|
| Audio (MP3/FLAC/WMA/AAC/AIFF) | audio_thumbnail → AudioDecoder | **Migrated v6.2** |
| Video (MP4/MKV/AVI/MOV/WMV) | video_thumbnail → VideoDecoder | **Migrated v6.2** |
| PDF | pdf_decoder → PDFDecoder | **Migrated v6.2** |
| Documents (DOCX/PPTX/XLSX) | document_thumbnail → DocumentDecoder | **Migrated v6.2** |
| Fonts (TTF/OTF/WOFF) | font_preview → FontDecoder | **Migrated v6.2** |
| SVG | svg_decoder → SVGDecoder | **Migrated v6.2** |
| eBooks (EPUB/MOBI/DJVU) | archive-based → DocumentDecoder | **Migrated v6.2** |

### 1.5 Explorer Registration (v6.1.0)

Shell registration expanded from **13 → 55 extensions** in `CBXShell.rgs`:
- Comics/Archives: .cbz, .cbr, .cb7, .cbt, .7z, .rar, .zip
- eBooks: .epub, .mobi, .azw, .azw3, .fb2, .djvu, .phz
- Modern Images: .webp, .avif, .heic, .heif, .jxl
- Professional: .psd, .psb, .dds, .hdr, .exr, .tga, .qoi, .ico
- Vector: .svg, .svgz
- Netpbm: .ppm, .pgm, .pbm, .pnm
- Camera RAW: .cr2, .cr3, .nef, .arw, .dng, .orf, .rw2, .raf, .pef
- PDF/Documents: .pdf, .docx, .pptx, .xlsx
- Fonts: .ttf, .otf, .woff, .woff2

### 1.6 Stub Decoders (Not Yet Functional)

| Format | File | Blocker |
|--------|------|---------|
| JPEG XL (.jxl) | `Engine/Decoders/JXLDecoder.cpp` | libjxl not linked |
| HEIF/HEIC (.heif/.heic) | `Engine/Decoders/HEIFDecoder.cpp` | libheif not linked |
| SVG (.svg) | `CBXShell/svg_decoder.h` | Placeholder bitmap only |

---

## 2. Current Open Issues

### 2.1 Critical — Stub Decoders

| ID | Issue | Resolution |
|----|-------|------------|
| C-01 | JXL decoder returns empty bitmap | Build libjxl, link, enable `HAS_LIBJXL=ON` |
| C-02 | HEIF decoder returns empty bitmap | Build libheif + libde265, link, enable `HAS_LIBHEIF=ON` |

### 2.2 High — Feature Gaps

| ID | Issue | Resolution |
|----|-------|------------|
| H-01 | SVG is placeholder bitmap | Integrate lunasvg or resvg library |
| H-02 | Plugin LoadPlugins() commented out | Enable + implement shared memory IPC |
| H-03 | No WiX MSI installer | Create WiX project |

### 2.3 Medium — Quality

| ID | Issue | Resolution |
|----|-------|------------|
| M-01 | Legacy decoders duplicated from Engine | Migrate to Engine-only pipeline |
| M-02 | No automated integration tests | Add Explorer simulation tests |
| M-03 | No performance regression benchmarks | Add decode-time tracking in CI |

---

## 3. Development Plan

### Phase 1 — Complete Format Coverage (HIGH PRIORITY)

> **Goal:** Activate all registered Shell extensions with working decoders. Zero "dead" extensions.

#### Sprint 1 — JPEG XL Integration (1 day)

| # | Task | Method | Effort |
|---|------|--------|--------|
| 1.1 | Build libjxl 0.11+ with brotli + highway via CMake | `build-scripts/build-libjxl.ps1` | 1-2h |
| 1.2 | Place libs in `external/image-libs/libjxl/lib/` | Manual copy | 5m |
| 1.3 | Enable `HAS_LIBJXL=ON` in Engine CMake, link `jxl.lib` + `jxl_threads.lib` | CMakeLists.txt edit | 15m |
| 1.4 | Implement `JXLDecoder::Decode()` → `JxlDecoderCreate` → `JxlDecoderSubscribeEvents` → `JxlDecoderSetInput` → process `JXL_DEC_FULL_IMAGE` → BGRA pixel copy | Replace stub code | 30m |
| 1.5 | Unit tests: 8-bit, 16-bit, animated JXL (first frame), lossless | test-archives/ | 30m |
| 1.6 | Verify: 0 errors, 0 warnings, tests pass | Build + test | 15m |

**Technical approach:** Use libjxl's C API directly. Decode to RGB buffer, convert to BGRA `CreateDIBSection`. Handle ICC profiles via `JxlDecoderGetColorAsEncodedProfile`. Cap decode at 50 MP to prevent OOM in Explorer process.

#### Sprint 2 — HEIF/HEIC Integration (1 day)

| # | Task | Method | Effort |
|---|------|--------|--------|
| 2.1 | Build libheif + libde265 (HEVC) + x265 (optional encode) via CMake | `build-scripts/Build-LibHEIF.ps1` | 1-2h |
| 2.2 | Enable `HAS_LIBHEIF=ON`, link `heif.lib` + `de265.lib` | CMake + vcxproj | 15m |
| 2.3 | Implement `HEIFDecoder::Decode()` → `heif_context_read_from_file` → `heif_context_get_primary_image_handle` → `heif_decode_image` to `heif_chroma_interleaved_RGBA` → BGRA copy | Replace stub | 30m |
| 2.4 | Handle HEIF multi-image containers (use primary or largest thumbnail), EXIF orientation | Additional logic | 30m |
| 2.5 | Test with iPhone HEIC photos (portrait, HDR, live photo .heic) | Manual + automated | 30m |
| 2.6 | Verify build | Build + test | 15m |

**Technical approach:** Use libheif C API. For thumbnails, prefer embedded thumbnail via `heif_image_handle_get_thumbnail()` (much faster than full decode). Fall back to full decode + scale for images without embedded thumbnails.

#### Sprint 3 — SVG Rendering (1 day)

| # | Task | Method | Effort |
|---|------|--------|--------|
| 3.1 | Integrate lunasvg (MIT license, lightweight, no dependencies) | Git submodule or source copy to external/ | 30m |
| 3.2 | Build lunasvg as static lib via CMake | Add to build scripts | 30m |
| 3.3 | Replace placeholder in `svg_decoder.h` → `lunasvg::Document::loadFromFile()` → `document->renderToBitmap(width, height)` → BGRA CreateDIBSection | Code replacement | 1h |
| 3.4 | Handle SVGZ (gzip-compressed SVG) via zlib inflate before parsing | Additional decompress step | 30m |
| 3.5 | Test with complex SVGs (gradients, filters, text, viewBox scaling) | test-archives/ | 30m |

**Technical approach:** lunasvg renders to a `Bitmap` object with premultiplied BGRA32 pixels — directly compatible with `CreateDIBSection`. For SVGZ, use existing zlib (already in external/compression-libs/) to inflate, then pass uncompressed SVG string to lunasvg.

### Phase 2 — Pipeline Unification & Quality (MEDIUM PRIORITY)

> **Goal:** Consolidate dual-path architecture. Every format goes through Engine pipeline.

#### Sprint 4 — Migrate Legacy Decoders to Engine ✅ COMPLETED (v6.2)

| # | Task | Description | Status |
|---|------|-------------|--------|
| 4.1 | Create `Engine/Decoders/SVGDecoder.h/.cpp` | GDI+ rendering, SVGZ detection | ✅ Done |
| 4.2 | Create `Engine/Decoders/VideoDecoder.h/.cpp` using Media Foundation | 20 formats, MF Source Reader | ✅ Done |
| 4.3 | Create `Engine/Decoders/AudioDecoder.h/.cpp` for album art extraction | ID3v2/FLAC/PropertySystem + waveform | ✅ Done |
| 4.4 | Create `Engine/Decoders/PDFDecoder.h/.cpp` | Shell provider + placeholder | ✅ Done |
| 4.5 | Create `Engine/Decoders/DocumentDecoder.h/.cpp` | 19 formats, color-coded placeholders | ✅ Done |
| 4.6 | Create `Engine/Decoders/FontDecoder.h/.cpp` | Shell font preview + placeholder | ✅ Done |
| 4.7 | Register all new decoders in ThumbnailPipeline and update CMakeLists | Pipeline + CMake | ✅ Done |
| 4.8 | Route all CBXShell `OnExtract` calls through Engine pipeline | CBXShell.cpp modification | ⬜ Pending |
| 4.9 | Gate legacy path behind feature flag | Gradual migration | ⬜ Pending |

**Technical approach for each decoder:**
- **Video:** Use `IMFSourceReader` from Media Foundation → `ReadSample` at timestamp 0 → `MFCreateMemoryBuffer` → convert to BGRA bitmap. No external dependencies needed.
- **Audio:** Read ID3v2 tags (MP3), Vorbis comments (FLAC/OGG), or WMA metadata via `IPropertyStore` for embedded album art JPEG. Fall back to generic audio icon.
- **PDF:** Use pdfium's `FPDF_RenderPageBitmap` for first page. Already in external/pdf-libs/.
- **Document:** Use OLE structured storage (`IStorage`/`IStream`) to extract embedded thumbnail from OOXML packages (docx is just a ZIP with `docProps/thumbnail.emf` or `_rels/.rels` → first image).
- **Font:** Use DirectWrite `IDWriteFontFile` → `CreateFontFace` → `GetGlyphRunOutline` to render sample text onto offscreen bitmap. Zero dependencies beyond Windows SDK.

#### Sprint 5 — Automated Testing & QA (2 days)

| # | Task | Description | Status |
|---|------|-------------|--------|
| 5.1 | Create test fixtures for ALL 55 registered extensions | One sample file per extension | ⬜ Pending |
| 5.2 | Automated decode test: each fixture must produce valid HBITMAP | GoogleTest integration | ⬜ Pending |
| 5.3 | Add performance benchmarks: each decoder must complete < 100ms for thumbnail | Benchmark suite | ⬜ Pending |
| 5.4 | Memory leak testing via CRT debug heap (`_CRTDBG_MAP_ALLOC`) | Debug build tests |
| 5.5 | Explorer integration test: register DLL, verify thumbnails appear | PowerShell automation |
| 5.6 | Cross-version test matrix: Win10 21H2, Win11 23H2, Win11 24H2 | VM or multi-boot |
| 5.7 | Expand unit tests from 43 → 80+ (add tests for PSD, DDS, HDR, PPM, EXR) | New test files |

### Phase 3 — Performance & Polish (MEDIUM PRIORITY)

> **Goal:** Best-in-class decode speed. Every thumbnail < 50ms at p95.

#### Sprint 6 — Performance Optimization ✅ MOSTLY COMPLETE (v6.2)

| # | Task | Description | Status |
|---|------|-------------|--------|
| 6.1 | Decode time profiling: measure every decoder across 100 test files | PerformanceProfiler expanded | ✅ Done |
| 6.2 | Add SIMD scaling path for non-WIC decoders (SSE4.1 → box filter) | `Utils/SIMDScaler.cpp` | ✅ Done |
| 6.3 | Implement progressive decode for PSD (RLE decompression + thumbnail resource) | PSDDecoder optimization | ✅ Done |
| 6.4 | Optimize HDR tone mapping with SSE vectorization | HDRDecoder SSE4.1 | ✅ Done |
| 6.5 | GPU shader for batch resize | D3D11Renderer enhancement | ⬜ Pending |
| 6.6 | Cache pre-warming | Background thread pool | ⬜ Pending |
| 6.7 | Thread pool tuning: parallel batch decode with bounded concurrency | Pipeline `GenerateThumbnailsBatch` | ✅ Done |

**Targets:**
| Metric | Current | Target |
|--------|---------|--------|
| JPEG/PNG decode | ~10ms | <5ms |
| WebP decode | ~15ms | <10ms |
| RAW decode | ~40ms | <25ms |
| PSD decode | N/A | <30ms |
| HDR decode | N/A | <20ms |
| Archive first-image | ~50ms | <30ms |

#### Sprint 7 — UI & User Experience ✅ MOSTLY COMPLETE (v6.2)

| # | Task | Description | Status |
|---|------|-------------|--------|
| 7.1 | CBXManager: add per-format enable/disable toggles | New checkbox IDs + handlers | ✅ Done |
| 7.2 | CBXManager: show decoder status (installed/missing libs) | DecoderHealthCheck.h | ✅ Done |
| 7.3 | CBXManager: show registered extension count and decode statistics | About dialog stats | ✅ Done |
| 7.4 | Windows dark mode: scrollbars, tooltips, accent colors | DarkModeHelper.h enhancements | ✅ Done |
| 7.5 | Systray notification when registration succeeds/fails | Optional enhancement | ⬜ Pending |

### Phase 4 — Distribution & Packaging (LOW PRIORITY)

#### Sprint 8 — MSI Installer & Signing (2 days)

| # | Task | Description |
|---|------|-------------|
| 8.1 | Create WiX v4 project in `packaging/wix/` | MSI installer |
| 8.2 | Include COM registration, file association, Start Menu shortcuts | WiX components |
| 8.3 | Set up Authenticode code signing via `signtool.exe` | Certificate + build script |
| 8.4 | MSIX package for Microsoft Store submission | `packaging/msix/` |
| 8.5 | Automated release script: build → test → sign → package → checksum | `release-scripts/` |
| 8.6 | GitHub Releases integration with CI/CD | GitHub Actions |

### Phase 5 — Future Features (Backlog)

| Feature | Description | Priority | Notes |
|---------|-------------|----------|-------|
| WinUI 3 Manager | Replace CBXManager with modern XAML UI | Low | Stubs exist in `src/Manager.WinUI/` |
| Plugin marketplace | Download/install third-party decoders | Low | SDK headers ready |
| CLI thumbnail tool | `darkthumbs.exe --input file.psd --output thumb.png` | Low | Stubs in `src/Tools.CLI/` |
| ETW telemetry | Enterprise event tracing for IT admins | Low | Header stubs exist |
| Cloud files | OneDrive/Dropbox placeholder file handling | Low | `StorageProviderFileTypeInfo` API |
| Animated format first-frame | GIF/APNG/WebP animation → first frame | Low | Already works for GIF via WIC |
| JPEG XL animation | JXL animated → first frame thumbnail | Low | After Sprint 1 |
| RAW histogram overlay | Show exposure histogram on RAW thumbnails | Very Low | Niche feature |

---

## 4. Best Practices & Methodology

### 4.1 Decoder Implementation Pattern

Every new decoder follows this contract:

```cpp
class NewDecoder : public IThumbnailDecoder {
    // 1. Extension matching + magic byte validation
    bool CanDecode(const wchar_t* filePath) override;
    
    // 2. Decode → HBITMAP (32bppBGRA via CreateDIBSection)
    HRESULT Decode(const ThumbnailRequest& request,
                   ThumbnailResult& result) override;
    
    // 3. Metadata for registry
    DecoderInfo GetInfo() const override;
};
```

**Rules:**
- Always validate magic bytes, not just extension
- Cap file reads (50 MB for thumbnails, 200 MB for HDR)
- Return `E_FAIL` gracefully — never crash Explorer
- Prefer embedded thumbnails over full decode (PSD, HEIF, RAW)
- Use WIC when platform codec exists (DDS, ICO, EXR)
- Thread-safe: no mutable shared state, guard WIC factory with mutex

### 4.2 Shell Extension Safety

Since CBXShell.dll runs **inside `explorer.exe`**, all code must:
- Never call `exit()` or `abort()` — always return `HRESULT`
- Catch all exceptions at COM boundaries (`try/catch(...)` in `OnExtract`)
- Time-box decode to < 500ms (Explorer kills slow handlers)
- Limit memory to < 50 MB per thumbnail request
- Never show UI (no `MessageBox`, no `OutputDebugString` in Release)

### 4.3 Build & Release Checklist

```
□ msbuild /p:Configuration=Release /p:Platform=x64 → 0 errors, 0 warnings
□ DarkThumbsTests.exe → all tests pass
□ regsvr32 CBXShell.dll → thumbnails appear in Explorer
□ No memory leaks (CRT debug heap clean)
□ All 55 registered extensions produce thumbnails (or graceful fallback)
```

### 4.4 Coding Standards

- C++20, `/std:c++20`, `/W4`, `/WX` (warnings-as-errors)
- `/MD` runtime (Multi-threaded DLL) — matches all external libs
- Windows 11 SDK target: `WINVER=0x0A00`, `_WIN32_WINNT=0x0A00`
- Namespace: `DarkThumbs::Engine` for all Engine code
- No STL exceptions in COM boundary code
- RAII for all resources (`ComPtr`, `unique_ptr`, `HBITMAP` with `DeleteObject`)

---

## 5. Build Instructions

### Quick Build (Production)
```powershell
.\scripts\build.ps1 -Configuration Release
```

### Direct MSBuild
```powershell
msbuild CBXShell.sln /p:Configuration=Release /p:Platform=x64 /m /v:minimal
```

### Clean Build
```powershell
.\scripts\build.ps1 -Configuration Release -Clean
```

### Build External Libraries
```powershell
.\Build-Production-SlowMachine.ps1 -Clean
```

### Register Shell Extension
```powershell
regsvr32 x64\Release\CBXShell.dll
```

---

## 6. Testing

```powershell
# Run all unit tests:
cd x64\Release
.\DarkThumbsTests.exe

# Run specific test suite:
.\DarkThumbsTests.exe --gtest_filter="DecoderTest.*"
```

---

## 7. Success Metrics

| Metric | v6.0.0 | v6.1.0 | v6.2.0 | Target v7.0 |
|--------|--------|--------|--------|-------------|
| Build warnings | 0 | 0 | 0 | 0 |
| Unit tests | 43/43 | 43/43 | 70+ | 80+ |
| Registered extensions | 13 | 55 | 55 | 70+ |
| Engine decoders | 10 | 15 | 21 | 21+ |
| Decode p95 | ~25ms | ~25ms | ~20ms | <20ms |
| Memory per thumbnail | N/A | <50 MB | <50 MB | <30 MB |
| Plugin count | 0 | 0 | 0 | 2+ |
| Installer | None | None | None | MSI + MSIX |

---

## 8. Revision History

| Date | Version | Changes |
|------|---------|---------|
| Jun 2025 | v6.2 | 6 new Engine decoders (SVG, Video, Audio, PDF, Document, Font), SIMD box-filter scaler, HDR SSE tone mapping, PSD RLE decompression, parallel batch decode, expanded profiling, decoder health check, CBXManager format toggles + dark mode scrollbars/accents, 70+ unit tests |
| Jun 2025 | v6.1 | Major format expansion: 5 new Engine decoders (PSD, DDS, HDR, PPM, EXR), shell registration 13→55 extensions, 60+ new format entries in GetCBXType, fixed ENABLE_WEBP/JXL defines, fixed duplicate RAW, rewritten roadmap with technical implementation details |
| Jun 2025 | v6.0 | Complete rewrite — realistic plan based on actual codebase audit. Reduced from 28 sprints to 6 focused sprints. |
| Feb 2026 | v5.0 | Original best-in-class edition (28 sprint plan) |
| Feb 2026 | v4.0 | Consolidated from multiple roadmap versions |
