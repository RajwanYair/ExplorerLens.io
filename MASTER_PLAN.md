# DarkThumbs v7.0 — Master Plan

> **Date:** February 15, 2026  
> **Version:** v6.2.0 → v7.0  
> **Status:** Active — single source of truth for all project planning  
> **Replaces:** ROADMAP.md, REFACTOR_PLAN.md, SPRINT_PLAN_25.md, SPRINT_PROGRESS.md

---

## 1. What Ships Today (v6.2.0)

| Component | Status | Notes |
|-----------|--------|-------|
| DarkThumbsEngine.lib | **Production** | C++20, Clang 21, 21 decoders compiled, 0 warnings |
| CBXShell.dll | **Production** | ATL/COM shell extension, 55 registered extensions, SEH protection |
| CBXManager.exe | **Production** | WTL settings app, dark mode, hardware diagnostics |
| ModernRuntime.lib | **Skeleton** | IPC transport, worker process, ShellHost client — headers + impl, not wired |
| Manager.WinUI | **Stub** | C#/XAML project, not in build graph, services are placeholders |
| Unit Tests | 70+ passing | Engine tests + integration tests |

### 1.1 Engine Decoders (21 Compiled)

| Decoder | Status | Implementation |
|---------|--------|----------------|
| ImageDecoder (JPEG/PNG/BMP/GIF/TIFF) | **Full** | WIC |
| WebPDecoder | **Full** | libwebp 1.5.0 |
| AVIFDecoder | **Full** | libavif 1.3.0 + dav1d 1.5.1 |
| ArchiveDecoder (ZIP/RAR/7Z/TAR/CBZ/CBR/CB7) | **Full** | libarchive + minizip-ng + unrar |
| ICODecoder | **Full** | WIC |
| TGADecoder | **Full** | Custom parser |
| QOIDecoder | **Full** | Reference impl |
| PSDDecoder (PSD/PSB) | **Full** | Custom RLE parser |
| DDSDecoder | **Full** | WIC |
| HDRDecoder (Radiance RGBE) | **Full** | Custom + SSE tone mapping |
| PPMDecoder (PPM/PGM/PBM/PNM/PAM/PFM) | **Full** | Custom parser |
| EXRDecoder | **Full** | WIC codec |
| RAWDecoder (CR2/NEF/ARW/DNG + 20 more) | **Partial** | WIC + LibRaw when HAS_LIBRAW=ON |
| JXLDecoder | **Partial** | Implemented when HAS_LIBJXL=ON, lib may not be linked |
| HEIFDecoder | **Partial** | Implemented when HAS_LIBHEIF=ON, fallback to WIC |
| SVGDecoder | **Placeholder** | GDI+ gradient only, needs lunasvg |
| VideoDecoder (35+ formats) | **Full** | Media Foundation |
| AudioDecoder (14 formats) | **Full** | ID3v2 / FLAC / PropertyStore + waveform |
| PDFDecoder | **Fallback** | Shell thumbnail provider, needs MuPDF/pdfium |
| DocumentDecoder (19 formats) | **Fallback** | Color-coded placeholder, needs OLE extraction |
| FontDecoder (TTF/OTF/WOFF/WOFF2/TTC) | **Fallback** | Shell preview, needs DirectWrite rendering |

### 1.2 Legacy CBXShell Decoders (Duplication Problem)

CBXShell/ contains 21 legacy decoder files (~113 KB total) that duplicate Engine decoders:

| Legacy File | Engine Equivalent | Action |
|-------------|-------------------|--------|
| audio_thumbnail.cpp/h | AudioDecoder.cpp | **Remove** — route through Engine |
| video_thumbnail.cpp/h | VideoDecoder.cpp | **Remove** — route through Engine |
| pdf_decoder.cpp/h | PDFDecoder.cpp | **Remove** — route through Engine |
| document_thumbnail.cpp/h | DocumentDecoder.cpp | **Remove** — route through Engine |
| font_preview.cpp/h | FontDecoder.cpp | **Remove** — route through Engine |
| svg_decoder.h | SVGDecoder.cpp | **Remove** — route through Engine |
| raw_decoder.cpp/h | RAWDecoder.cpp | **Remove** — route through Engine |
| jxl_decoder.cpp/h | JXLDecoder.cpp | **Remove** — route through Engine |
| heif_decoder_native.cpp/h | HEIFDecoder.cpp | **Remove** — route through Engine |
| webp_decoder.cpp/h | WebPDecoder.cpp | **Remove** — route through Engine |
| avif_decoder.cpp/h | AVIFDecoder.cpp | **Remove** — route through Engine |

### 1.3 Script Duplication (Cleanup Needed)

| Duplicate | Location A | Location B | Resolution |
|-----------|-----------|-----------|------------|
| build.ps1 | build-scripts/ | scripts/ | Keep scripts/build.ps1 as canonical |
| Sign-Binaries.ps1 | build-scripts/ | scripts/release/ | Keep scripts/release/ |
| Library builders | 5 scripts in library-builders/ | 3 in production/ | Consolidate into library-builders/ |

---

## 2. Architecture (Current → Target v7.0)

### Current Architecture
```
Explorer ──► CBXShell.dll (in-process COM)
             ├── EngineAdapter → ThumbnailPipeline → 21 Decoders
             ├── Legacy fallback (cbxArchive)
             └── D3D11 GPU + File Cache
```

### Target v7.0 Architecture
```
Explorer ──► CBXShell.dll (thin proxy)
             └── Named Pipe ──► DarkThumbsWorker.exe
                                ├── DarkThumbsEngine.lib (30+ decoders)
                                ├── D3D12 GPU + SQLite Cache
                                ├── Plugin Host (3rd-party decoders)
                                └── LocalApiService (:9876)
                                    ├── REST API → Manager.WinUI (C#/WinUI 3)
                                    └── WebSocket → Web Dashboard (React)
```

---

## 3. Execution Plan — 8 Phases, 40 Tasks

### Phase 1: Project Cleanup & Consolidation (Tasks 1-5)
> Remove duplication, unify build, establish clean baseline

| # | Task | Priority | Status |
|---|------|----------|--------|
| 1 | Remove stale docs and temp build artifacts | P0 | **Done** |
| 2 | Consolidate planning docs into single MASTER_PLAN.md | P0 | **Done** |
| 3 | Remove legacy CBXShell decoders, route all through Engine | P0 | Queued |
| 4 | Consolidate duplicate build scripts | P1 | Queued |
| 5 | Update .gitignore for build artifacts, clean git index | P1 | Queued |

### Phase 2: Engine Quality & Format Activation (Tasks 6-12)
> Make every registered extension produce a real thumbnail

| # | Task | Priority | Details |
|---|------|----------|---------|
| 6 | Verify JXL decoder works end-to-end (HAS_LIBJXL=ON path) | P0 | Code exists, verify linkage |
| 7 | Verify HEIF decoder works (HAS_LIBHEIF=ON or WIC path) | P0 | Multiple fallback strategies |
| 8 | SVG: integrate lunasvg for real vector rendering | P1 | Replace GDI+ placeholder |
| 9 | PDF: integrate pdfium or Windows.Data.Pdf for real rendering | P1 | Replace shell provider fallback |
| 10 | Document: implement OLE thumbnail extraction from OOXML | P1 | docProps/thumbnail.emf extraction |
| 11 | Font: implement DirectWrite glyph rendering | P1 | IDWriteFontFile → sample text render |
| 12 | RAW: verify LibRaw integration and embedded JPEG path | P1 | Fastest path for camera RAW |

### Phase 3: Shell Extension Modernization (Tasks 13-17)
> Unify shell → engine path, add feature flags, remove legacy code

| # | Task | Priority | Details |
|---|------|----------|---------|
| 13 | Route ALL CBXShell thumbnails through EngineAdapter only | P0 | Remove dual-path, keep SEH |
| 14 | Add feature flag system (registry-backed) for decoder toggles | P1 | Already partially in Config.h |
| 15 | Gate out-of-process worker behind feature flag | P1 | Wire ShellHostClient into CBXShellClass |
| 16 | Drop IExtractImage2 (deprecated), keep IThumbnailProvider only | P2 | Windows 10+ only |
| 17 | Migrate ATL CComModule → WRL Module\<InProc\> | P2 | Remove ATL dependency |

### Phase 4: Out-of-Process Worker (Tasks 18-22)
> Crash isolation for explorer.exe stability

| # | Task | Priority | Details |
|---|------|----------|---------|
| 18 | Create DarkThumbsWorker.exe main() entry point | P0 | Hosts IPCServer + ThumbnailPipeline |
| 19 | Wire ShellHostClient circuit breaker into CBXShell | P0 | Fall back to in-process on worker failure |
| 20 | Implement warm worker pool (2 pre-started processes) | P1 | Job objects + heartbeat |
| 21 | Add ETW tracing with correlation IDs (shell → worker → decoder) | P2 | TraceLoggingProvider |
| 22 | Stress test: crash decoder, verify Explorer survives | P0 | Validation gate |

### Phase 5: GPU & Performance (Tasks 23-27)
> D3D12 compute, SIMD upgrades, IO optimization

| # | Task | Priority | Details |
|---|------|----------|---------|
| 23 | D3D12 compute shader for Lanczos3 batch resize | P2 | Keep D3D11 fallback |
| 24 | AVX-512 / ARM64 NEON auto-dispatch in SIMDScaler | P2 | Runtime cpuid check |
| 25 | Memory-mapped IO for all decoders (CreateFileMapping) | P1 | ~2x for large files |
| 26 | Lazy decoder initialization (create on first CanDecode match) | P1 | Already partially done |
| 27 | Cache pre-warming background thread | P2 | Pre-generate visible thumbnails |

### Phase 6: WinUI 3 Manager (Tasks 28-32)
> Modern Windows 11 native GUI replacing WTL CBXManager

| # | Task | Priority | Details |
|---|------|----------|---------|
| 28 | Create C++/WinRT interop component wrapping Engine API | P1 | DarkThumbs.Interop |
| 29 | Wire WinUI 3 service implementations to Engine interop | P1 | Replace Task.Delay stubs |
| 30 | Implement Settings page — per-format toggles, cache, GPU | P1 | ToggleSwitch grid |
| 31 | Implement Diagnostics page — decoder health + live status | P2 | Green/yellow/red |
| 32 | Add Mica backdrop, MSIX packaging, auto-update | P2 | Windows 11 native chrome |

### Phase 7: Web Dashboard & API (Tasks 33-36)
> Browser-accessible local dashboard for power users

| # | Task | Priority | Details |
|---|------|----------|---------|
| 33 | Implement LocalApiService with cpp-httplib | P2 | REST endpoints + WebSocket |
| 34 | Build React SPA dashboard (status, formats, cache, preview) | P3 | Vite + TailwindCSS |
| 35 | Embed SPA in WinUI 3 via WebView2 | P3 | Dashboard tab |
| 36 | Add auth token + localhost binding + rate limits | P2 | Security baseline |

### Phase 8: Testing, Packaging & Release (Tasks 37-40)
> Production-quality release pipeline

| # | Task | Priority | Details |
|---|------|----------|---------|
| 37 | GoogleTest integration — 1 fixture per extension (120+) | P1 | CTest integration |
| 38 | Memory leak testing via CRT debug heap | P1 | 10,000 thumbnail stress |
| 39 | WiX 4 MSI installer + MSIX package | P1 | COM registration, file assoc |
| 40 | GitHub Actions CI/CD: build → test → sign → release | P2 | Authenticode + checksums |

---

## 4. Priority Matrix

| Priority | Tasks | Criteria |
|----------|-------|----------|
| **P0** — Blocks production | 1, 2, 3, 6, 7, 13, 18, 19, 22 | Must fix before any release |
| **P1** — Core functionality | 4, 5, 8-12, 14-15, 20, 25-26, 28-30, 37-39 | Required for v7.0 |
| **P2** — Quality & polish | 16-17, 21, 23-24, 27, 31-32, 33, 36, 40 | Important but deferrable |
| **P3** — Future enhancement | 34, 35 | Nice to have |

---

## 5. Success Metrics (v7.0 Targets)

| Metric | v6.2 (Current) | v7.0 (Target) |
|--------|----------------|---------------|
| Build warnings | 0 | 0 |
| Unit tests | 70+ | 200+ |
| Registered extensions | 55 | 120+ |
| Working decoders (non-stub) | 15 | 28+ |
| Decode p95 | ~20ms | <15ms |
| Memory per thumbnail | <50 MB | <30 MB |
| GPU | D3D11 basic | D3D12 compute + D3D11 fallback |
| Platforms | x64 | x64 + ARM64 |
| GUI | WTL (Win32) | WinUI 3 + Web Dashboard |
| Installer | None | MSI + MSIX |
| Process isolation | None (in-process) | Out-of-process worker |
| Legacy decoder duplication | 21 files (113 KB) | 0 |

---

## 6. Technology Decisions

| Area | Choice | Rationale |
|------|--------|-----------|
| COM framework | WRL (`wrl/module.h`) | Lighter than ATL, Microsoft-recommended |
| GUI | WinUI 3 (Windows App SDK 1.6+) | Native Win11, Mica, MSIX |
| Web server | cpp-httplib (header-only MIT) | Zero deps, embeddable |
| Web frontend | React + Vite + TailwindCSS | Small bundle, fast dev |
| GPU compute | D3D12 with D3D11 fallback | Best Win11 perf, ARM64 ready |
| IPC | Named Pipes (OVERLAPPED) | Fastest local IPC on Windows |
| SVG | lunasvg (MIT) | Lightweight, BGRA output |
| JSON | nlohmann/json (header-only MIT) | C++ standard |
| Installer | WiX 4 (MSI) + MSIX | Enterprise + Store |
| Testing | GoogleTest + CTest | Industry standard |
| CI/CD | GitHub Actions | Free for open source |

---

## 7. File Organization (Target)

```
DarkThumbs/
├── README.md                    # Project overview
├── MASTER_PLAN.md               # This file — single planning doc
├── CHANGELOG.md                 # Version history
├── LICENSE                      # MIT
├── CMakeLists.txt               # Root CMake (Engine + ModernRuntime)
├── CBXShell.sln                 # MSBuild solution (Shell + Manager)
│
├── Engine/                      # C++20 thumbnail engine library
│   ├── Core/                    # Interfaces, config, types
│   ├── Decoders/                # 21+ format decoders
│   ├── Pipeline/                # ThumbnailPipeline + registry
│   ├── GPU/                     # D3D11/D3D12 renderers
│   ├── Cache/                   # Thumbnail cache
│   ├── Plugin/                  # Plugin host + sandbox
│   ├── Utils/                   # SIMD, profiler, EXIF, hardware
│   └── Tests/                   # Engine unit tests
│
├── CBXShell/                    # COM shell extension (thin proxy)
│   ├── CBXShellClass.cpp/h      # IThumbnailProvider impl
│   ├── EngineAdapter.cpp/h      # Engine bridge
│   └── StdAfx.*                 # Precompiled headers
│
├── CBXManager/                  # Legacy WTL settings app
│
├── src/                         # v7.0 modular components
│   ├── Worker/                  # Out-of-process worker (IPC, process mgmt)
│   ├── ShellHost/               # Shell → worker client
│   ├── Service/                 # Local API service
│   ├── Manager.WinUI/           # WinUI 3 settings app
│   ├── PluginHost/              # Plugin sandbox
│   ├── Tools.CLI/               # CLI thumbnail tool
│   └── Tools.PSModule/          # PowerShell module
│
├── build-scripts/               # Build automation
│   ├── external-libs/           # Per-library build scripts
│   └── library-builders/        # Orchestration scripts
│
├── scripts/                     # Dev/release/test scripts
│   ├── release/                 # Release pipeline
│   └── test/                    # Test automation
│
├── external/                    # Third-party dependencies
├── SDK/                         # Plugin SDK
├── packaging/                   # Installers (WiX, MSIX, InnoSetup)
├── tests/                       # Integration + benchmark tests
├── docs/                        # Detailed documentation
└── tools/                       # Dev tools
```

---

## 8. Immediate Execution Queue (Tasks 1-10)

These are the first 10 tasks to execute right now:

### Task 1: ✅ Remove stale docs and temp build artifacts
**Status:** DONE  
Removed: 7 stale log files, 2 temp build dirs, 15 stale MD files

### Task 2: ✅ Consolidate planning docs into MASTER_PLAN.md
**Status:** DONE  
Merged content from: ROADMAP.md, REFACTOR_PLAN.md, SPRINT_PLAN_25.md, SPRINT_PROGRESS.md

### Task 3: Remove legacy CBXShell decoders, route all through Engine
**Status:** Executing  
Remove 21 legacy decoder files from CBXShell/, update vcxproj to remove them from build

### Task 4: Consolidate duplicate build scripts
**Status:** Queued  
Remove duplicate build.ps1, Sign-Binaries.ps1, consolidate library-builders/

### Task 5: Update .gitignore, clean git index
**Status:** Queued

### Task 6: Verify JXL decoder end-to-end
**Status:** Queued

### Task 7: Verify HEIF decoder end-to-end
**Status:** Queued

### Task 8: SVG — integrate lunasvg
**Status:** Queued

### Task 9: PDF — implement real first-page rendering
**Status:** Queued

### Task 10: Document — implement OLE thumbnail extraction
**Status:** Queued
