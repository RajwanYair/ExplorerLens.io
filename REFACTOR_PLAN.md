# DarkThumbs v7.0 — Full Refactoring Plan

> **Date:** January 2026
> **Scope:** Windows 11 best-practice modernization, full file-type coverage, dual GUI (native + web), maximum performance
> **Baseline:** v6.2.0 — 21 decoders, 55 extensions, 0 errors/0 warnings

---

## 1. Executive Summary

This plan refactors DarkThumbs from a legacy ATL/WTL shell extension into a modern Windows 11-native platform with three frontends (Shell Extension, WinUI 3 Manager, Local Web Dashboard), an out-of-process worker for crash isolation, and complete file-format coverage (~120+ extensions across 30+ decoders).

**Key architectural changes:**
1. **Out-of-process thumbnail worker** — isolates decoder crashes from `explorer.exe`
2. **WinUI 3 Manager** — replaces legacy WTL CBXManager with Windows App SDK
3. **Local REST API + Web UI** — lightweight HTTP server with React dashboard
4. **Complete format activation** — JXL, HEIF, SVG, RAW, PDF fully working
5. **DirectX 12 compute** — GPU thumbnail scaling upgrade from D3D11
6. **ARM64 support** — native builds for Snapdragon X Elite / Copilot+ PCs

---

## 2. Gap Analysis — What ROADMAP Tasks Were NOT Applied

### 2.1 Unapplied ROADMAP Sprints

| ROADMAP Item | Sprint | Status | Gaps |
|---|---|---|---|
| **Sprint 1 — JXL Integration** | Phase 1 | **NOT DONE** | libjxl not linked, `HAS_LIBJXL=OFF`, JXLDecoder returns empty bitmap |
| **Sprint 2 — HEIF/HEIC Integration** | Phase 1 | **NOT DONE** | libheif not linked, `HAS_LIBHEIF=OFF`, HEIFDecoder returns empty bitmap |
| **Sprint 3 — SVG Rendering** | Phase 1 | **NOT DONE** | SVGDecoder is placeholder-only, lunasvg not integrated |
| **Sprint 4.8** — Route CBXShell through Engine | Phase 2 | **NOT DONE** | CBXShellClass.cpp still has dual-path (Engine + legacy fallback) |
| **Sprint 4.9** — Gate legacy path behind feature flag | Phase 2 | **NOT DONE** | No feature flag system exists |
| **Sprint 5** — Automated Testing & QA | Phase 2 | **NOT DONE** | No test fixtures per extension, no GoogleTest integration running |
| **Sprint 6.5** — GPU shader for batch resize | Phase 3 | **NOT DONE** | D3D11Renderer exists but no compute shader for batch operations |
| **Sprint 6.6** — Cache pre-warming | Phase 3 | **NOT DONE** | No background pre-warming thread |
| **Sprint 7.5** — Systray notifications | Phase 3 | **NOT DONE** | No systray support in CBXManager |
| **Sprint 8** — MSI/MSIX Installer | Phase 4 | **NOT DONE** | No WiX project, no code signing, no MSIX |
| **Phase 5 — WinUI 3 Manager** | Backlog | **STUB ONLY** | `src/Manager.WinUI/` has WinUI 3 .csproj + XAML stubs but no C++/WinRT interop |
| **Phase 5 — CLI tool** | Backlog | **STUB ONLY** | `src/Tools.CLI/CommandDispatcher.h` exists but not built |
| **Phase 5 — Plugin marketplace** | Backlog | **STUB ONLY** | SDK headers exist, PluginHost stubs exist, no runtime |
| **Phase 5 — Out-of-process worker** | Backlog | **STUB ONLY** | `src/Worker/worker_process.h` (320 lines), `src/ShellHost/shellhost_client.h` (322 lines) — headers only |
| **Phase 5 — Local API Service** | Backlog | **STUB ONLY** | `src/Service/LocalApiService.h` — header only |

### 2.2 Summary: What's Real vs Stubs

| Component | State |
|---|---|
| DarkThumbsEngine.lib (21 decoders) | **Real** — compiles, runs |
| CBXShell.dll (COM shell ext) | **Real** — works in Explorer |
| CBXManager.exe (WTL settings) | **Real** — works |
| EngineAdapter bridge | **Real** — connects Shell → Engine |
| D3D11Renderer | **Real** — basic GPU scaling works |
| ThumbnailCache | **Real** — basic file cache |
| Manager.WinUI (C#/XAML) | **Stub** — project file + XAML templates, not wired |
| Worker process / IPC | **Stub** — headers only, no .cpp files |
| ShellHost client | **Stub** — header only |
| LocalApiService | **Stub** — header only |
| CLI tools | **Stub** — header only |
| PluginHost | **Stub** — headers only |

---

## 3. File Format Coverage Gaps

### 3.1 Current: Working Decoders (21)

| Decoder | Formats | Real Implementation |
|---|---|---|
| ImageDecoder | JPEG, PNG, BMP, GIF, TIFF | ✅ WIC-based |
| WebPDecoder | WebP | ✅ libwebp |
| AVIFDecoder | AVIF | ✅ libavif+dav1d |
| ArchiveDecoder | ZIP, CBZ, 7Z, CB7, TAR, CBT, RAR | ✅ libarchive+unrar |
| ICODecoder | ICO, CUR | ✅ WIC |
| TGADecoder | TGA | ✅ Custom parser |
| QOIDecoder | QOI | ✅ Reference impl |
| PSDDecoder | PSD, PSB | ✅ Custom (RLE) |
| DDSDecoder | DDS | ✅ WIC/custom |
| HDRDecoder | HDR | ✅ RGBE+SSE tone map |
| PPMDecoder | PPM, PGM, PBM, PNM, PAM, PFM | ✅ Custom parser |
| EXRDecoder | EXR | ✅ WIC codec |
| SVGDecoder | SVG, SVGZ | ⚠️ Placeholder only |
| VideoDecoder | MP4, AVI, MKV, etc. (35+) | ✅ Media Foundation |
| AudioDecoder | MP3, FLAC, WAV, etc. (11) | ✅ ID3v2/MF/PropertyStore |
| PDFDecoder | PDF | ⚠️ Shell provider fallback |
| DocumentDecoder | DOCX, PPTX, XLSX, DOC, etc. (19) | ⚠️ Color-coded placeholder |
| FontDecoder | TTF, OTF, WOFF, WOFF2, TTC | ⚠️ Shell provider fallback |
| RAWDecoder | CR2, NEF, ARW, DNG, etc. (20+) | ⚠️ Stub when libraw OFF |
| JXLDecoder | JXL | ❌ Empty bitmap |
| HEIFDecoder | HEIC, HEIF | ❌ Empty bitmap |

### 3.2 Missing Formats to Add for v7.0

| Category | Formats to Add | Method |
|---|---|---|
| **3D/CAD** | .obj, .stl, .fbx, .gltf, .glb, .3ds, .blend, .step, .iges | DirectX 12 mesh render / Assimp lib |
| **Vector** | .ai (Adobe Illustrator), .eps, .emf, .wmf | GDI+ metafile / Ghostscript |
| **Science/Medical** | .fits, .dcm (DICOM), .nii (NIfTI) | Custom header parsers |
| **Geospatial** | .geotiff, .shp (shapefile preview) | GDAL or custom |
| **eBook expanded** | .djvu, .xps, .oxps | DjVuLibre, XPS API |
| **Code/Text** | .md, .json, .xml, .yaml, .csv, .log, .ini, .bat, .ps1, .py, .js, .ts, .cpp, .h, .cs, .java, .rs, .go | Syntax-highlighted text render via DirectWrite |
| **Markdown** | .md rendered preview | Lightweight MD→bitmap renderer |
| **Design** | .sketch, .fig (Figma export), .xd | ZIP-based thumbnail extraction |
| **Image extra** | .jfif, .wbmp, .pcx, .pict, .sgi, .ras, .xpm, .xbm, .jp2, .j2k (JPEG 2000) | OpenJPEG for JP2, custom parsers |
| **Archive expanded** | .lzh, .lha, .arc, .zoo, .sit, .dmg (Apple), .vhd, .vmdk, .wim, .appx, .msix | libarchive + custom |
| **Audio extra** | .aac, .wma, .aiff, .dsf, .dff (DSD) | Media Foundation |
| **Subtitle** | .srt, .ass, .ssa | Text preview |
| **Database** | .sqlite, .db, .mdb, .accdb | Schema preview |

### 3.3 Priority File Formats for v7.0 (High Impact)

**Tier 1 — Must Have (commonly encountered):**
- JPEG XL (.jxl) — already has decoder + lib, just needs linking
- HEIF/HEIC (.heic, .heif) — iPhone standard, already has decoder + lib
- SVG (.svg, .svgz) — very common web format, needs lunasvg
- RAW (.cr2, .nef, .arw, .dng, 20+ extensions) — needs LibRaw linking
- PDF (.pdf) — needs MuPDF integration (lib available in external/)
- JPEG 2000 (.jp2, .j2k) — cinema/medical standard

**Tier 2 — Should Have:**
- Documents (.docx, .pptx, .xlsx) — OLE thumbnail extraction, not just placeholder
- Fonts (.ttf, .otf) — DirectWrite glyph rendering, not shell fallback
- 3D Models (.obj, .stl, .gltf) — growing demand
- Code/Text preview (.md, .json, .xml, .py, .js)
- DjVu (.djvu) — academic papers
- XPS (.xps, .oxps) — Windows native format

**Tier 3 — Nice to Have:**
- AI/EPS vector
- DICOM medical
- Design files
- Additional archives

---

## 4. Windows 11 Best Practices — What to Change

### 4.1 Shell Extension Modernization

| Current | Windows 11 Best Practice | Action |
|---|---|---|
| In-process COM DLL in explorer.exe | **Out-of-process thumbnail provider** via `IThumbnailProvider` + named pipes | Implement worker_process.h/.cpp |
| ATL `CComModule` (legacy) | **WRL** `Module<InProc>` or **C++/WinRT** `winrt::implements` | Migrate COM plumbing to WRL |
| `IExtractImage2` (deprecated) | `IThumbnailProvider` only (Win10+ preferred) | Drop IExtractImage2, keep IThumbnailProvider |
| GDI `CreateDIBSection` for bitmap | **WIC** `IWICBitmap` → `IWICFormatConverter` → `CreateBitmapFromWICBitmap` | Use WIC throughout pipeline |
| DirectX 11 GPU renderer | **DirectX 12** compute pipeline with `ID3D12PipelineState` | Upgrade D3D11Renderer → D3D12Renderer |
| `WINVER=0x0A00` (Win10) | `WINVER=0x0A00` with Win11 feature detection via `IsWindows11OrGreater()` | Feature-detect Mica, rounded corners, Snap Layouts |
| `OutputDebugString` tracing | **ETW** (Event Tracing for Windows) via `TraceLoggingProvider` | Add ETW manifest + provider |
| No telemetry | **Windows Performance Recorder** profiles | Add WPR profile for decode perf |

### 4.2 GUI — WinUI 3 Manager (Replaces WTL CBXManager)

The `src/Manager.WinUI/` project already has a C#/WinUI 3 skeleton. It needs:

| Component | Current State | What to Build |
|---|---|---|
| `Manager.WinUI.csproj` | ✅ References WindowsAppSDK 1.5 | Update to latest WASDK 1.6+ |
| `MainWindow.xaml` | ✅ NavigationView with 5 pages | Wire up all navigation + viewmodels |
| `DashboardPage.xaml` | ✅ 4 status cards XAML | Implement `DashboardViewModel` with real Engine C++ interop |
| `SettingsPage.xaml` | ✅ Stub | Build per-format toggles, cache config, GPU config |
| `DiagnosticsPage.xaml` | ✅ Stub | Wire `DecoderHealthCheck` results |
| `PluginsPage.xaml` | ✅ Stub | Plugin marketplace browser |
| **C++/WinRT interop** | ❌ Missing | `DarkThumbsEngine.dll` → C++/WinRT projection → consumed by C# |
| **MSIX packaging** | ❌ Missing | Package Manager.WinUI as MSIX with CBXShell.dll |
| **Mica/Acrylic backdrop** | ❌ Missing | `SystemBackdrop = new MicaBackdrop()` |
| **Notification center** | ❌ Missing | Windows App SDK `AppNotificationManager` for registration events |

**Architecture:**
```
Manager.WinUI (C#) ──[C++/WinRT]──► DarkThumbsEngine.dll (C++)
                     ──[Named Pipe]──► DarkThumbsWorker.exe (thumbnail worker)
                     ──[HTTP]──────► LocalApiService (web dashboard)
```

### 4.3 Web Interface — Local REST API + React Dashboard

Build on the existing `src/Service/LocalApiService.h` stub:

**Backend (C++ HTTP server):**
```
LocalApiService
├── GET  /api/v1/status          → Engine health, decoder count, cache stats
├── GET  /api/v1/decoders        → List all decoders with capabilities
├── GET  /api/v1/formats         → Supported formats + extension map
├── POST /api/v1/thumbnail       → Generate thumbnail (file path + size → base64 PNG)
├── POST /api/v1/batch           → Batch thumbnail generation
├── GET  /api/v1/cache/stats     → Cache hit rate, size, entries
├── POST /api/v1/cache/clear     → Clear thumbnail cache
├── GET  /api/v1/plugins         → Installed plugins
├── POST /api/v1/plugins/install → Install plugin from URL
├── GET  /api/v1/diagnostics     → Full system diagnostics JSON
├── WebSocket /ws/events         → Real-time decode events + progress
```

**Frontend options (embedded):**
1. **WebView2 inside WinUI 3** — Best native integration, Microsoft Edge runtime
2. **Standalone browser** at `http://localhost:9876` — Zero dependencies, any browser
3. **Both** — WinUI 3 hosts WebView2 for integrated experience, also accessible via browser

**Recommended:** Use `cpp-httplib` (header-only, MIT) for the HTTP server. Embed a React SPA as static assets compiled into resources. WebView2 in WinUI 3 loads `http://localhost:9876`.

### 4.4 Performance Best Practices for Windows 11

| Area | Current | Target |
|---|---|---|
| **Thread pool** | `std::async` | Windows Thread Pool API (`CreateThreadpoolWork`) with priority classes |
| **Memory mapping** | `fread` / `CreateFile+ReadFile` | `CreateFileMappingW` + `MapViewOfFile` for large images |
| **SIMD** | SSE4.1 box-filter scaler | **AVX-512** (Ice Lake+) + **ARM NEON** (Snapdragon X) auto-dispatch |
| **GPU compute** | D3D11 | D3D12 compute pipeline with async copy queues |
| **IO** | Synchronous | `OVERLAPPED` async IO + IO completion ports for batch |
| **Cache** | File-based | Memory-mapped shared cache + SQLite metadata index |
| **Startup** | Full decoder init | Lazy decoder init — only create decoder on first use |
| **Binary size** | All decoders linked | Optional decoder DLLs loaded on demand |

---

## 5. Refactoring Plan — Sprint Breakdown

### Phase A: Foundation Fixes (Sprints A1–A3) — 3 days

> **Goal:** Activate all existing stub decoders, eliminate all dead extensions.

#### Sprint A1 — Activate JXL + HEIF Decoders (1 day)

| # | Task | Details |
|---|------|---------|
| A1.1 | Build libjxl 0.11.1 with CMake (already in external/) | `build-scripts/build-libjxl.ps1`, enable `HAS_LIBJXL=ON` |
| A1.2 | Implement `JXLDecoder::Decode()` with libjxl C API | JxlDecoderCreate → JxlDecoderSubscribeEvents → process JXL_DEC_FULL_IMAGE |
| A1.3 | Build libheif + libde265 (already in external/) | `build-scripts/Build-LibHEIF.ps1`, enable `HAS_LIBHEIF=ON` |
| A1.4 | Implement `HEIFDecoder::Decode()` with libheif C API | heif_context_read → get_primary_image → decode to RGBA |
| A1.5 | Verify both decoders produce valid thumbnails | Unit tests + Explorer test |

#### Sprint A2 — Activate SVG + RAW + PDF Decoders (1 day)

| # | Task | Details |
|---|------|---------|
| A2.1 | Integrate lunasvg (MIT, header-only-ish) into external/ | Git clone, build static lib |
| A2.2 | Replace SVGDecoder placeholder with lunasvg rendering | loadFromFile → renderToBitmap(w,h) → BGRA |
| A2.3 | Build LibRaw, enable `HAS_LIBRAW=ON` | RAWDecoder already has full pImpl structure |
| A2.4 | Integrate MuPDF for PDFDecoder | FPDF_RenderPageBitmap for first page |
| A2.5 | Implement FontDecoder with DirectWrite | IDWriteFontFile → CreateFontFace → render sample text |
| A2.6 | Implement DocumentDecoder OLE thumbnail extraction | IStorage/IStream for OOXML thumbnail.emf |

#### Sprint A3 — Unify Shell → Engine Path (1 day)

| # | Task | Details |
|---|------|---------|
| A3.1 | Route ALL `CBXShellClass::GetThumbnail` through EngineAdapter | Remove legacy `m_cbx.OnExtract` fallback |
| A3.2 | Add feature flag system (`DarkThumbsConfig.h`) | Registry-backed flags: `EnableLegacyPath`, `EnableGPU`, `EnableCache` |
| A3.3 | Remove `#ifdef ENABLE_*_SUPPORT` guards from cbxArchive.h | All formats always registered (decoder handles availability) |
| A3.4 | Delete legacy decoder code paths in cbxArchive.h | Keep only archive extraction; images go through Engine |
| A3.5 | Verify clean build + all 55+ extensions produce thumbnails | Full regression test |

---

### Phase B: Architecture Modernization (Sprints B1–B4) — 5 days

> **Goal:** Out-of-process worker, WRL COM, modern GPU, lazy init.

#### Sprint B1 — Out-of-Process Worker (2 days)

| # | Task | Details |
|---|------|---------|
| B1.1 | Implement `DarkThumbsWorker.exe` from `worker_process.h` stubs | Named pipe IPC, hosts Engine pipeline |
| B1.2 | Implement `IPC::Transport` (named pipes) | `ipc_transport.h` → `ipc_transport.cpp` with OVERLAPPED IO |
| B1.3 | Implement `ShellHostClient` with circuit breaker | Thin client in CBXShell.dll, routes to worker |
| B1.4 | Add warm pool for worker processes | Pre-start 2 workers, keep alive with heartbeat |
| B1.5 | Fallback: if worker unavailable, fall back to in-process | Graceful degradation |
| B1.6 | Add process isolation tests | Crash one decoder, verify Explorer stays alive |

#### Sprint B2 — COM Modernization (1 day)

| # | Task | Details |
|---|------|---------|
| B2.1 | Migrate `CComModule` → WRL `Module<InProc>` | Remove ATL dependency from CBXShell |
| B2.2 | Use `winrt::implements<IThumbnailProvider, IInitializeWithStream>` | C++/WinRT COM implementation |
| B2.3 | Drop `IExtractImage2` legacy interface | Keep only `IThumbnailProvider` |
| B2.4 | Modernize `DllRegisterServer` with `RegCreateKeyEx` | Clean COM registration without ATL |
| B2.5 | Add package identity support for MSIX | `DesktopBridgeAppId` in manifest |

#### Sprint B3 — DirectX 12 GPU Pipeline (1 day)

| # | Task | Details |
|---|------|---------|
| B3.1 | Create `D3D12Renderer` implementing `IGPURenderer` | ID3D12Device4, compute pipeline |
| B3.2 | Lanczos3 compute shader in HLSL → compiled .cso | CS 5.1 with threadgroup (16,16,1) |
| B3.3 | Async copy queue for texture upload/readback | Overlap CPU decode + GPU scale |
| B3.4 | Keep D3D11Renderer as fallback for older GPUs | Feature detection at init |
| B3.5 | Benchmark: D3D12 vs D3D11 vs CPU | Target 2x speedup for batch |

#### Sprint B4 — Performance Optimizations (1 day)

| # | Task | Details |
|---|------|---------|
| B4.1 | Memory-mapped IO for all decoders | `CreateFileMapping` + `MapViewOfFile` |
| B4.2 | Lazy decoder initialization | Create decoder on first `CanDecode` match, not at pipeline init |
| B4.3 | AVX2/AVX-512 auto-dispatch in SIMDScaler | Runtime `__cpuid` check, multi-path |
| B4.4 | ARM64 NEON path for SIMDScaler | `#ifdef _ARM64_` with NEON intrinsics |
| B4.5 | Windows Thread Pool API for batch decode | Replace `std::async` with `CreateThreadpoolWork` + bounded queue |
| B4.6 | Background cache pre-warming | On Explorer folder open, pre-generate visible thumbnails |
| B4.7 | SQLite cache metadata | Replace simple file cache with indexed DB |

---

### Phase C: WinUI 3 Manager (Sprints C1–C3) — 4 days

> **Goal:** Modern native Windows 11 UI replacing WTL CBXManager.

#### Sprint C1 — C++/WinRT Engine Projection (1 day)

| # | Task | Details |
|---|------|---------|
| C1.1 | Create `DarkThumbs.Interop` WinRT component (C++/WinRT) | Wraps Engine API as WinRT types |
| C1.2 | IDL definitions: `ThumbnailService`, `DecoderInfo`, `CacheStats` | `.idl` → MIDL → projection |
| C1.3 | Expose Engine pipeline through WinRT async patterns | `IAsyncOperation<ThumbnailResult>` |
| C1.4 | NuGet package for interop component | Consumed by Manager.WinUI |

#### Sprint C2 — WinUI 3 Dashboard & Settings (2 days)

| # | Task | Details |
|---|------|---------|
| C2.1 | Implement `DashboardViewModel` with real Engine data | Decoder count, cache stats, GPU info |
| C2.2 | Implement `SettingsPage` — per-format enable/disable | Toggle grid with 55+ formats |
| C2.3 | Implement `DiagnosticsPage` — decoder health check | Green/yellow/red status per decoder |
| C2.4 | Implement `FormatsPage` — file type browser | Searchable grid: extension → decoder → status |
| C2.5 | Add Mica backdrop + rounded corners | `SystemBackdrop = new MicaBackdrop()` |
| C2.6 | Add Windows 11 system tray icon | `AppNotificationManager` for events |
| C2.7 | Window restore/position persistence | `ApplicationData.Current.LocalSettings` |

#### Sprint C3 — MSIX Packaging (1 day)

| # | Task | Details |
|---|------|---------|
| C3.1 | Create MSIX package for Manager.WinUI | WAP project with CBXShell.dll |
| C3.2 | Sparse package for COM registration | `DesktopBridgeRegistration` for IThumbnailProvider |
| C3.3 | Auto-update via MSIX App Installer | Sideload URL for updates |
| C3.4 | Optional: Microsoft Store submission | Package validation + screenshots |

---

### Phase D: Web Dashboard (Sprints D1–D2) — 3 days

> **Goal:** Browser-accessible dashboard + thumbnail API.

#### Sprint D1 — Local REST API Server (1 day)

| # | Task | Details |
|---|------|---------|
| D1.1 | Implement `LocalApiService` from stub header | cpp-httplib (header-only MIT) |
| D1.2 | REST endpoints: `/status`, `/decoders`, `/formats`, `/thumbnail`, `/cache` | JSON responses via nlohmann/json |
| D1.3 | POST `/thumbnail` — accept file path, return base64 PNG | Engine pipeline → WIC PNG encode → base64 |
| D1.4 | WebSocket `/ws/events` for real-time updates | Decode events, cache hits, errors |
| D1.5 | Auth token for localhost security | Random token in registry, required header |
| D1.6 | CORS headers for browser access | `Access-Control-Allow-Origin: *` (localhost only) |

#### Sprint D2 — React Web Dashboard (2 days)

| # | Task | Details |
|---|------|---------|
| D2.1 | React + Vite + TailwindCSS project in `web-dashboard/` | Modern SPA |
| D2.2 | Dashboard page — Engine status, decoder grid, cache chart | Chart.js for stats |
| D2.3 | Format browser — searchable table of all supported formats | Filter by category, status |
| D2.4 | Thumbnail tester — drag/drop file or enter path, see thumbnail | Live preview |
| D2.5 | Diagnostics — real-time WebSocket event log | Scrolling event viewer |
| D2.6 | Build SPA → embed as resources in DarkThumbsWorker.exe | Serve static files from memory |
| D2.7 | WebView2 integration in WinUI 3 Manager | Dashboard tab loads localhost |

---

### Phase E: Extended Format Support (Sprints E1–E3) — 3 days

> **Goal:** 30+ decoders, 120+ extensions.

#### Sprint E1 — Tier 2 Formats (1 day)

| # | Task | Details |
|---|------|---------|
| E1.1 | JPEG 2000 decoder via OpenJPEG | `.jp2`, `.j2k`, `.jpx` |
| E1.2 | XPS/OXPS decoder via Windows XPS API | `IXpsOMDocument` → render first page |
| E1.3 | DjVu decoder via DjVuLibre | First page → bitmap |
| E1.4 | Code/text preview decoder | DirectWrite syntax coloring for .py, .js, .cpp, .json, .xml, .md, etc. |
| E1.5 | EMF/WMF vector decoder | GDI+ `Metafile::FromFile` → render to bitmap |

#### Sprint E2 — 3D Model Decoder (1 day)

| # | Task | Details |
|---|------|---------|
| E2.1 | Integrate Assimp (Open Asset Import) | .obj, .stl, .fbx, .gltf, .glb, .3ds |
| E2.2 | D3D12 mesh preview renderer | Wireframe or shaded thumbnail of 3D model |
| E2.3 | Auto-camera framing | Bounding box → fit camera → render |
| E2.4 | Fallback: icon overlay for unsupported mesh features | Graceful degradation |

#### Sprint E3 — Additional Formats (1 day)

| # | Task | Details |
|---|------|---------|
| E3.1 | Additional image formats: .pcx, .sgi, .ras, .jfif | Custom byte-level parsers |
| E3.2 | Additional audio: .aac, .wma, .aiff, .dsf, .dff | Media Foundation |
| E3.3 | Additional archives: .lzh, .wim, .appx, .msix | libarchive |
| E3.4 | Database preview: .sqlite schema diagram | SQLite C API → schema text render |
| E3.5 | Subtitle preview: .srt, .ass first few lines | Text render |

---

### Phase F: Testing & Distribution (Sprints F1–F2) — 2 days

> **Goal:** Production-quality release pipeline.

#### Sprint F1 — Comprehensive Testing (1 day)

| # | Task | Details |
|---|------|---------|
| F1.1 | GoogleTest integration with CTest | cmake --build --target RUN_TESTS |
| F1.2 | One test fixture per registered extension (120+) | Sample files in test-archives/ |
| F1.3 | Performance benchmarks: each decoder < 100ms | Automated regression tracking |
| F1.4 | Memory leak tests via CRT debug heap | _CRTDBG_MAP_ALLOC in Debug |
| F1.5 | Explorer integration test script | regsvr32 + verify thumbnails appear |
| F1.6 | ARM64 cross-compile validation | Build for arm64, test on Snapdragon |

#### Sprint F2 — Release Pipeline (1 day)

| # | Task | Details |
|---|------|---------|
| F2.1 | WiX 4 MSI installer | COM registration, file associations, Start Menu |
| F2.2 | MSIX package | Store-ready with auto-update |
| F2.3 | Authenticode code signing | signtool + timestamp |
| F2.4 | GitHub Actions CI/CD | Build → test → sign → package → release |
| F2.5 | README + documentation update | Screenshots, install guide, API docs |

---

## 6. Architecture Diagram (Target v7.0)

```
┌─────────────────────────────────────────────────────────────┐
│                     Windows 11 Explorer                      │
│  ┌──────────────┐                                            │
│  │ CBXShell.dll │─── IThumbnailProvider ───┐                │
│  │ (thin proxy) │    IInitializeWithStream  │                │
│  └──────────────┘                           │                │
└─────────────────────────────────────────────┼────────────────┘
                                              │ Named Pipe IPC
┌─────────────────────────────────────────────┼────────────────┐
│              DarkThumbsWorker.exe            ▼                │
│  ┌─────────────────────────────────────────────────┐         │
│  │            DarkThumbsEngine.lib                  │         │
│  │  ┌──────────┐  ┌──────────┐  ┌───────────┐     │         │
│  │  │ Decoders │  │ Pipeline │  │ GPU (D3D12)│     │         │
│  │  │  30+     │  │ + Cache  │  │ + D3D11 FB │     │         │
│  │  └──────────┘  └──────────┘  └───────────┘     │         │
│  └─────────────────────────────────────────────────┘         │
│  ┌──────────────────┐  ┌──────────────────────────┐         │
│  │ LocalApiService  │  │ Plugin Host              │         │
│  │ :9876 REST+WS    │  │ 3rd-party decoder DLLs   │         │
│  └──────────────────┘  └──────────────────────────┘         │
└──────────────────────────────────────────────────────────────┘
          │                          │
          │ HTTP/WS                  │ C++/WinRT
          ▼                          ▼
┌──────────────────┐     ┌──────────────────────┐
│ Web Dashboard    │     │ Manager.WinUI        │
│ React SPA        │     │ (WinUI 3 + WebView2) │
│ (any browser)    │     │ MSIX packaged         │
└──────────────────┘     └──────────────────────┘
```

---

## 7. Technology Decisions

| Decision | Choice | Rationale |
|---|---|---|
| COM framework | **WRL** (`wrl/module.h`) | Lighter than ATL, no CRT dependency, Microsoft-recommended |
| GUI framework | **WinUI 3** (Windows App SDK 1.6+) | Native Win11 look, Mica, NavigationView, MSIX |
| Web server | **cpp-httplib** (header-only MIT) | Zero dependencies, easy to embed |
| Web frontend | **React + Vite + TailwindCSS** | Fast dev, small bundle, modern |
| Embedded browser | **WebView2** (Edge runtime) | Ships with Win11, zero extra install |
| GPU renderer | **DirectX 12** with D3D11 fallback | Best perf on Win11, needed for ARM64 |
| IPC | **Named Pipes** (OVERLAPPED) | Fastest local IPC on Windows, kernel-mode |
| HTTP JSON | **nlohmann/json** (header-only MIT) | Standard C++ JSON library |
| SVG rendering | **lunasvg** (MIT) | Lightweight, no dependencies, BGRA output |
| 3D rendering | **Assimp** (BSD) + D3D12 | Industry standard mesh import |
| JPEG 2000 | **OpenJPEG** (BSD) | Reference implementation |
| Installer | **WiX 4** (MSI) + **MSIX** | Enterprise (MSI) + Store (MSIX) |
| CI/CD | **GitHub Actions** | Free for open source, Windows runners |
| Testing | **GoogleTest** + CTest | Already partially integrated |
| Code signing | **Authenticode** (signtool) | Required for SmartScreen trust |

---

## 8. Success Metrics (v7.0 Targets)

| Metric | v6.2.0 (Current) | v7.0 (Target) |
|---|---|---|
| Build warnings | 0 | 0 |
| Unit tests | 70+ | 150+ |
| Registered extensions | 55 | 120+ |
| Engine decoders | 21 | 30+ |
| Working decoders (non-stub) | 15 | 30+ |
| Decode p95 | ~20ms | <15ms |
| Memory per thumbnail | <50 MB | <30 MB |
| GPU acceleration | D3D11 basic | D3D12 compute + D3D11 fallback |
| Platforms | x64 only | x64 + ARM64 |
| GUI | WTL (Win32) | WinUI 3 + Web Dashboard |
| Installer | None | MSI + MSIX |
| Plugin count | 0 (stub) | 2+ working |
| API endpoints | 0 | 12+ REST + WebSocket |
| Process isolation | None (in-process) | Out-of-process worker |

---

## 9. Estimated Timeline

| Phase | Duration | Sprints | Key Deliverable |
|---|---|---|---|
| **Phase A** — Foundation Fixes | 3 days | A1–A3 | All 55 extensions working, no stubs |
| **Phase B** — Architecture | 5 days | B1–B4 | Out-of-process worker, DX12, WRL COM |
| **Phase C** — WinUI 3 Manager | 4 days | C1–C3 | Modern native Windows 11 UI |
| **Phase D** — Web Dashboard | 3 days | D1–D2 | REST API + React SPA |
| **Phase E** — Extended Formats | 3 days | E1–E3 | 30+ decoders, 120+ extensions |
| **Phase F** — Testing & Release | 2 days | F1–F2 | MSI/MSIX, CI/CD, signed release |
| **TOTAL** | **~20 working days** | **17 sprints** | **DarkThumbs v7.0** |

---

## 10. Risk Assessment

| Risk | Impact | Mitigation |
|---|---|---|
| libjxl/libheif build complexity | Medium | Pre-built binaries available; vcpkg fallback |
| Out-of-process IPC latency | High | Named pipes are ~0.1ms; warm pool eliminates startup cost |
| WinUI 3 C++/WinRT interop complexity | Medium | Use C# projection; Engine stays pure C++ |
| ARM64 SIMD porting (SSE→NEON) | Medium | Use `sse2neon.h` translation header; hand-optimize hot paths |
| D3D12 complexity vs D3D11 | Low | Keep D3D11 as fallback; D3D12 only for batch compute |
| MuPDF licensing (AGPL) | High | Use only for thumbnails (not redistribution); or switch to Poppler (GPL) or pdfium (BSD) |
| WebView2 runtime dependency | Low | Ships with Windows 11; fallback to standalone browser |

---

## 11. Quick Start — What to Do First

**Recommended execution order for maximum impact:**

1. **Sprint A1** — JXL + HEIF (highest user demand, libs already present)
2. **Sprint A2** — SVG + RAW + PDF (completes core format coverage)
3. **Sprint A3** — Unify Shell→Engine (eliminates dual-path tech debt)
4. **Sprint B1** — Out-of-process worker (biggest architectural improvement)
5. **Sprint B4** — Performance optimizations (lazy init, MMAP IO)
6. **Sprint C1–C2** — WinUI 3 Manager (best user-facing improvement)
7. **Sprint D1–D2** — Web dashboard (unique differentiator)
8. Everything else follows naturally.

---

*This plan supersedes the Phase 5 backlog in ROADMAP.md. Update ROADMAP.md after each completed sprint.*
