# ExplorerLens — Strategic Roadmap v5.0 "Rigel"

**Version:** 5.0 — April 2026  
**Current Release:** v39.1.0 "Betelgeuse" (4,978 tests, 0 errors, 0 warnings, 42 Catch2 files)  
**Supersedes:** ROADMAP v4.0 "Betelgeuse → Rigel" → archived to `docs/archive/ROADMAP_V4.md`  
**Prior archive chain:** v3.0 → `docs/archive/ROADMAP_V3.md`, v1/v2 → `docs/archive/`  
**Scope:** Full re-examination of every decision made in v1.0–v4.0. No decision is exempt. Architecture, language, compiler, build system, frontend, backend, GPU, cache, database, external libraries, API design, test strategy, docs, CI/CD, distribution, security, observability, AI/ML, and competitive positioning — all opened.

> "The best time to rethink a decision is before its cost compounds. The second-best time is now."

---

## Table of Contents

1. [Executive Summary](#1-executive-summary)
2. [What the Deep Rethink Reveals](#2-what-the-deep-rethink-reveals)
3. [Expanded Competitive Landscape — 16 Competitors, 26 Dimensions](#3-expanded-competitive-landscape)
4. [Best-in-Class Distillation — 18 Harvested Practices](#4-best-in-class-distillation)
5. [Language, Runtime & Compiler](#5-language-runtime--compiler)
6. [Frontend — Shell, GUI, CLI & Web](#6-frontend)
7. [Backend — Engine, Decode Pipeline, Cache, GPU](#7-backend)
8. [API Design & Error Handling](#8-api-design--error-handling)
9. [External Libraries, APIs & Data Sources](#9-external-libraries-apis--data-sources)
10. [Build System & Toolchain](#10-build-system--toolchain)
11. [Testing & Quality Strategy](#11-testing--quality-strategy)
12. [Database & Persistent Storage](#12-database--persistent-storage)
13. [Documentation & Configuration Standards](#13-documentation--configuration-standards)
14. [CI/CD, Packaging & Distribution](#14-cicd-packaging--distribution)
15. [Infrastructure, Security & Observability](#15-infrastructure-security--observability)
16. [Cross-Platform, AI/ML & Advanced Features](#16-cross-platform-aiml--advanced-features)
17. [Refactor / Rewrite / Delete Register](#17-refactor--rewrite--delete-register)
18. [8-Phase Plan to Best-in-Class](#18-8-phase-plan)
19. [Success Metrics & Exit Criteria](#19-success-metrics--exit-criteria)
20. [Decision Log (v5.0)](#20-decision-log-v50)
21. [AI Tooling Surface & MCP](#21-ai-tooling-surface--mcp)
22. [Consolidated Legacy — What Survives from v4.0](#22-consolidated-legacy)
23. [Sprint Delivery Log (session history)](#23-sprint-delivery-log)

---

## 1. Executive Summary

ExplorerLens is a **Windows Shell Extension** (`IThumbnailProvider` COM in-process DLL) that gives Windows Explorer accurate, fast thumbnails for 200+ file formats through 25+ specialized decoders — formats the OS ignores: modern images (AVIF/JXL/HEIC), RAW camera files, archives (CBZ/CBR/EPUB), PDFs, 3D models, and scientific data.

**What we are:**
- A **native Windows shell citizen** — no separate UI window, no 100 MB framework, no GC pauses.
- **MIT-licensed open source** with an optional enterprise distribution path (signed-plugin SDK, commercial MuPDF, GPO templates).
- A **developer-grade tool**: ETW observability, SBOM, headless CLI, REST server, test corpus.

**What the v5.0 re-examination concludes:**

| Finding | v4.0 verdict | v5.0 action |
|---------|-------------|-------------|
| COM + C++20 for the shell DLL | Keep | ✅ Confirmed. Add C++23 `std::expected` for new internal APIs |
| 1,386 headers vs 269 sources | Reverse | **Implement-before-declare. Merge stubs into .cpp. Target ≤ 1.8:1 by Phase 2** |
| 16-directory Engine layout | Reverse | **Consolidate to 7 by Phase 1** |
| Custom `TEST()` macro suite | Reverse | **Catch2 v3 primary. Every new test in Catch2. Retire macros by Phase 2** |
| MuPDF AGPL license | Reverse | **PDFium (BSD-3) default by Phase 3; MuPDF for enterprise SKU only** |
| No GPU compute path yet | Evolve | **Phase 2: D3D11 compute `resize_bilinear.hlsl`. Phase 3: DXVA2/Media Foundation** |
| No `IPropertyStore` | Gap | **Phase 3: Details pane metadata for 20+ formats** |
| No `IPreviewHandler` | Gap | **Phase 3: Preview pane — the biggest UX win we're leaving on the table** |
| `lens.exe` CLI incomplete | Gap | **Phase 1: 8 commands (generate/info/register/doctor/benchmark/cache/serve/plugin)** |
| `lens-server` Winsock2 | Reverse | **Phase 4: cpp-httplib + thread pool** |
| No crash reporting | Gap | **Phase 4: WER + minidump + opt-in upload** |
| Docs outnumber code (~130 md) | Reverse | **Right-size to ~60. Every doc must reflect working code** |
| Static HTML web page | Keep | ✅ No bundler, no SPA. GitHub Pages + live screenshots |
| 18 external libraries | Keep + Evolve | Add libjpeg-turbo (P0), tinyexr, stb_image, Assimp, FreeType, PDFium, cpp-httplib |
| No SQLite cache | Evolve | **Phase 2: SQLite WAL L2 cache index + mmap blobs** |
| No auto-update | Evolve | **winget/Scoop primary; in-app notification (opt-in) Phase 4** |
| Cross-platform README overclaims | Reverse | **Strip claims. macOS Phase 5. Linux Phase 6. WASM Phase 7** |
| Rust sandbox sidecar | New (evaluation) | **Phase 4 prototype: `windows-rs` COM interop for untrusted-decoder host** |
| AI/ML (`Engine/AI/` stubs) | Reverse | **Fold to Core. No inference until Phase 5, gated behind `[ai]` feature flag** |
| `IContextMenu` right-click preview | New (harvested H2) | **Phase 3: biggest missing shell-citizen feature** |
| PowerToys Peek pattern | New | **Phase 3: implement IPreviewHandler parity with PowerToys Peek** |
| Plugin ecosystem | Evolve | **Phase 3: C-ABI `plugin_api.h` + 1 reference plugin + winget-style discovery** |
| Enterprise / GPO | Evolve | **Phase 4: ADMX/ADML + HKLM policy + AppContainer decode host** |

### North-star: best-in-class, measurable

> For any file format that matters to a Windows user: ExplorerLens produces a **more correct, faster, and lower-footprint** thumbnail than Windows built-in, validated by automated corpus tests (SSIM ≥ 0.98), with zero crashes on a 10K-file fuzz corpus, installable in one command.

---

## 2. What the Deep Rethink Reveals

Every decision from v1.0 through v4.0 audited. Verdict: **Keep** / **Evolve** / **Reverse** / **New**.

### 2.1 Architecture decisions

| # | Decision | Since | v5.0 verdict | Action |
|---|----------|-------|--------------|--------|
| A1 | C++20 for all components | v1 | **Keep + Evolve** | Adopt C++23 features where MSVC v145 supports them (`std::expected`, `std::print`, flat containers) |
| A2 | MSVC v145 compiler | v30 | **Keep** | Only toolset with v145 ABI + full Win SDK 26100 + COM. Clang-cl for nightly UB lane only |
| A3 | CMake 4.3 + Ninja + presets | v32 | **Keep** | Industry standard; sccache-ready; vcpkg manifest |
| A4 | Static linking of all externals | v1 | **Keep** | COM DLL inside `explorer.exe` cannot tolerate loader lock races |
| A5 | `/MD` CRT | v20 | **Keep** | UCRT shared is the Windows norm; `/MT` causes CRT conflicts in COM |
| A6 | Shell / Engine / Manager three-layer split | v1 | **Keep** | Clean; survives every rethink |
| A7 | COM CLSID fixed at `9E6ECB90-5A61-42BD-B851-D3297D9C7F39` | v1 | **Keep** | Changing it breaks upgrade paths |
| A8 | 16-directory Engine layout | v35 | **Reverse** | Premature subdivision. Consolidate to 7 dirs (§7.2). AI/Enterprise/Media/Pipeline/PluginHost are stubs |
| A9 | 1,386 headers vs 269 sources (5.1:1) | various | **Reverse** | Header-first without impl defeats the compiler as first reviewer. Enforce implement-before-declare |
| A10 | STA apartment for COM objects | v1 | **Keep** | Required for `IThumbnailProvider` in `explorer.exe` thread pool |
| A11 | `LENSTYPE` enum for format routing | v1 | **Keep** | Simple, audit-visible, UPPER_CASE enforced by `.clang-tidy` |
| A12 | Zero-warnings policy (`/W4 /WX`) | v30 | **Keep** | Compiler as first line of defense. Never mask with `/wd` |
| A13 | ASLR / DEP / CFG / `/GS` | v30 | **Keep** | Must remain enabled for all binaries |
| A14 | Mixed HRESULT + exceptions internally | v1 | **Evolve** | Exceptions across COM = UB. New rule: `std::expected<T,EngineError>` internal; HRESULT only at COM boundary |
| A15 | No `IPropertyStore` | v1 | **Reverse** | Icaros shows it is the #2 win after thumbnails. Details view empty for all our formats is a gap |
| A16 | No `IPreviewHandler` | v1 | **Reverse** | PowerToys Peek + macOS Preview prove this is what power users want. Phase 3 |
| A17 | No `IContextMenu` | v1 | **Reverse** | SageThumbs right-click preview is a top discoverability win |
| A18 | `lens-server` on Winsock2 thread-per-conn | v35 | **Reverse** | Hand-rolled HTTP parser is an attack surface. cpp-httplib + thread pool |
| A19 | Cross-platform via `#ifdef` stubs in README | v34 | **Reverse** | `#ifdef` is not support. Remove all cross-platform claims. Platform-accurate timeline in §16 |
| A20 | AI/ML stubs in `Engine/AI/` | v36 | **Reverse** | Speculative stub directories add cognitive load with zero function. Fold to Core. No ML until Phase 5 |

### 2.2 Frontend decisions

| # | Decision | Verdict | Action |
|---|----------|---------|--------|
| F1 | WTL for LENSManager | **Keep** | No runtime dep, tiny binary, works in-process. Evaluate WinUI 3 at Phase 4 gate |
| F2 | Static HTML web page | **Keep** | Zero-dependency GitHub Pages. No React/Vue/Next |
| F3 | `lens.exe` CLI | **Evolve** | All 8 commands must be complete by Phase 1 exit |
| F4 | Dark-mode support in LENSManager | **Keep** | Already underway; complete by Phase 2 |
| F5 | No accessibility (UIA) in LENSManager | **Reverse** | Add UIA automation peers + high-contrast + keyboard nav by Phase 4 |
| F6 | English only | **Keep for now** | Resource-string isolation by Phase 3 for Phase 4 localization readiness |

### 2.3 Library and dependency decisions

| # | Decision | Verdict | Action |
|---|----------|---------|--------|
| L1 | MuPDF 1.24.11 (AGPL-3.0) | **Reverse** | AGPL triggers copyleft for `lens-server` (any network distribution). Migrate to PDFium (BSD-3) Phase 3 |
| L2 | No libjpeg-turbo (use WIC) | **Reverse** | libjpeg-turbo is 2–6× faster than WIC JPEG decode; P0 format deserves P0 library |
| L3 | No stb_image | **Evolve** | Excellent for QOI/PNM/TGA/HDR fallback; zero-dep header; add Phase 2 |
| L4 | No DirectXTex | **Evolve** | DDS/BC textures are common in game content; add Phase 2 |
| L5 | No Assimp | **Evolve** | 3D model thumbnails (glTF/OBJ/STL/FBX); add Phase 3 |
| L6 | No FreeType | **Evolve** | Font sample thumbnails; add Phase 3 |
| L7 | No cpp-httplib | **New** | REST server without Winsock2 wheel-reinvention; Phase 4 |
| L8 | No ONNX Runtime | **Deferred** | AI inference, Phase 5 only under `[ai]` feature flag |
| L9 | 13 custom `Build-*.ps1` library scripts | **Reverse** | Retire all 13; vcpkg manifest mode covers every library |
| L10 | NuGet + npm + Container + Maven + RubyGems | **Reverse** | Keep NuGet (SDK). Drop Maven + RubyGems (no consumers). npm only when WASM. Container only when `lens-server` ships |

### 2.4 Testing decisions

| # | Decision | Verdict | Action |
|---|----------|---------|--------|
| T1 | Custom `TEST()`/`ASSERT()` macros (~4,978 tests) | **Reverse** | No fixtures, no parameterization, no XML, no IDE integration. Catch2 primary by Phase 2 |
| T2 | ~21-file synthetic corpus | **Reverse** | Cannot validate 200+ formats. 106-entry MANIFEST.json in place; grow to ≥300 real files Phase 3 |
| T3 | No SSIM baseline tests in CI | **Reverse** | `corpus-validation.yml` gated; SSIM ≥ 0.98 threshold per format |
| T4 | No GPU tests | **Gap** | Phase 2: D3D11 SSIM tests; Phase 3: DXVA2 keyframe validation |
| T5 | No fuzz targets | **Reverse** | `fuzz-ci.yml` exists; implement 20 libFuzzer harnesses Phase 4 |
| T6 | No performance regression gate | **Reverse** | `performance-regression-gate.yml` exists; wire to Google Benchmark Phase 2 |

### 2.5 Security decisions

| # | Decision | Verdict | Action |
|---|----------|---------|--------|
| S1 | Binary runs in `explorer.exe` address space | **Evolve** | Phase 4: out-of-process decode host (COM OOP + AppContainer) for non-P0 decoders |
| S2 | No crash reporting | **Reverse** | WER + `SetUnhandledExceptionFilter` + minidump (opt-in) Phase 4 |
| S3 | No authenticated code signing | **Gap** | Authenticode (EV cert) Phase 4 for all binaries + MSI |
| S4 | LGPL static-link audit (libheif, libde265, LibRaw) | **Action required** | Publish LGPL exception notice in SBOM + `NOTICE` file; legal review Phase 2 |
| S5 | No supply-chain SHA pinning of Actions | **Reverse (done)** | `pin-actions.yml` workflow ✅ S163 |
| S6 | No ASAN/UBSAN CI | **Reverse** | `sanitizer-ci.yml` stub exists; implement Phase 4 |

---

## 3. Expanded Competitive Landscape — 16 Competitors, 26 Dimensions

v5.0 expands from 12 competitors (v4.0) to **16**, adding **PowerToys** (Microsoft's own shell extension suite including Peek), **Eagle** (asset manager), **Figma viewer** (design file thumbnails), and **Finder/Windows Explorer built-in** revisited as the baseline. 26 dimensions (was 22).

### 3.1 Full Competitor Matrix

| Dimension | **ExplorerLens v5 target** | PowerToys | QuickLook | SageThumbs | Icaros | Win built-in | XnView MP | macOS Quick Look | Preview.app | IrfanView | ImageGlass | Nomacs | GNOME Tumbler | libvips | Apache Tika | Eagle | Figma Viewer |
|-----------|---------------------------|-----------|-----------|------------|--------|--------------|-----------|-----------------|-------------|-----------|------------|--------|----------------|---------|-------------|-------|-------------|
| **Type** | Shell ext (IThumbnailProvider) | Shell ext suite | Viewer overlay | Shell ext | Shell ext | WIC handlers | Viewer | OS preview | OS viewer | Viewer | Viewer | Viewer | Daemon | CLI/lib | Parser | Asset mgr | Design viewer |
| **Language** | C++20/23 | C# .NET 9 | C# .NET 8 | C++ (GFL) | C++ | C++ | C++/Qt | Obj-C/Swift | Swift | C++ | C# .NET 8 | C++/Qt | C (GObject) | C | Java | Electron | TypeScript |
| **License** | MIT | MIT ✅ | GPL-3 | GPL-2 | GPL-2 | Proprietary | Freeware | Proprietary | Proprietary | Freeware | GPL-3 | GPL-3 | GPL-2 | LGPL-2.1 | Apache-2 | Proprietary | Proprietary |
| **GitHub stars (Apr 2026)** | new | 110K | 23.1K | SF | — | — | — | — | — | — | 8.7K | 2.1K | — | 27K | 2.8K | — | — |
| **Active maintained** | ✅ | ✅ | ✅ | ❌ 2017 | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| **Native Explorer integration** | ✅ IThumbnailProvider | ✅ Peek=IPreviewHandler | separate window | ✅ | ✅ | ✅ | separate | system | system | separate | separate | separate | native (Nautilus) | N/A | N/A | separate | browser |
| **AVIF / JXL / HEIC** | ✅ all | partial | partial | ❌ | ❌ | HEIC only (22H2+) | ✅ all | ✅ HEIC | ✅ HEIC | partial | ✅ all | ✅ all | ✅ (libheif) | ✅ all | metadata | ✅ HEIC | ❌ |
| **RAW camera (LibRaw)** | ✅ | ❌ | partial | partial | ❌ | limited WIC | ✅ | Apple RAW | ✅ | plugins | limited | ✅ | limited | ✅ | metadata | ✅ | ❌ |
| **Archive thumbs (CBZ/CBR/EPUB)** | ✅ | ❌ | via plugin | ❌ | ❌ | ❌ | limited | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | content | ❌ | ❌ |
| **PDF thumbnails** | ✅ | ❌ | ✅ | ❌ | ❌ | minimal | minimal | ✅ | ✅ | ❌ | ❌ | ❌ | ✅ (evince) | PDF/poppler | ✅ | ✅ | ❌ |
| **Video keyframe thumbs** | Phase 3 | ✅ (FFmpeg) | ✅ | ❌ | ✅ **best** | partial | limited | ✅ | ✅ | ❌ | ❌ | ❌ | ✅ (totem) | ❌ | metadata | ✅ | ❌ |
| **GPU-accelerated decode** | Phase 2 D3D11 | Direct2D | WPF HW | ❌ | ❌ | WIC+DXGI | ❌ | Metal | Metal | ❌ | Direct2D | OpenGL | ❌ | SIMD | ❌ | ❌ | WebGL |
| **IPreviewHandler (Preview pane)** | Phase 3 | ✅ **Peek** | N/A | ❌ | ❌ | partial | ❌ | native | native | ❌ | ❌ | ❌ | native | N/A | N/A | preview | ✅ |
| **IPropertyStore (Details pane)** | Phase 3 | limited | ❌ | ❌ | ✅ video | ✅ images | ❌ | Finder | ✅ | ❌ | ❌ | ❌ | Nautilus | N/A | ✅ | limited | ❌ |
| **IContextMenu (right-click)** | Phase 3 | ✅ multiple | ❌ | ✅ | ❌ | ✅ | ❌ | Finder | Finder | ❌ | ❌ | ❌ | Nautilus | N/A | N/A | ✅ | ❌ |
| **Plugin ecosystem** | Phase 3 C-ABI SDK | ✅ module | ✅ qlplugin | XnView | ❌ | WIC codec | XnView | .qlgenerator | limited | DLL plug | ❌ | plugins | .desktop | N/A | SPI | ❌ | ❌ |
| **Headless CLI** | ✅ lens.exe | limited | ❌ | ❌ | ❌ | ❌ | batch | ✅ qlmanage | ❌ | batch | batch | batch | tumblerctl | ✅ **core** | ✅ **core** | ❌ | ❌ |
| **REST API** | Phase 4 lens-server | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | D-Bus | bindings | ✅ tika-server | ❌ | REST |
| **Enterprise / GPO** | Phase 4 | partial Intune | ❌ | ❌ | ❌ | ✅ full | ❌ | MDM | MDM | ❌ | ❌ | ❌ | org policy | N/A | N/A | ❌ | org |
| **Install footprint** | < 5 MB | 15 MB suite | ~15 MB | ~5 MB | ~30 MB | OS built-in | ~80 MB | OS | OS | ~3 MB | ~15 MB | ~20 MB | ~2 MB | ~5 MB | ~70 MB JVM | 50 MB+ | SaaS |
| **Format depth validated** | ✅ SSIM CI | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | metadata | ❌ | N/A |
| **Open source & auditable** | ✅ MIT | ✅ MIT | ✅ GPL-3 | ✅ GPL-2 | ✅ GPL-2 | ❌ | ❌ | ❌ | ❌ | ❌ | ✅ GPL-3 | ✅ GPL-3 | ✅ GPL-2 | ✅ LGPL | ✅ Apache | ❌ | ❌ |
| **SBOM + supply chain** | ✅ SPDX+CycloneDX | partial | ❌ | ❌ | ❌ | ❌ | ❌ | partial | ❌ | ❌ | ❌ | ❌ | partial | ❌ | ✅ | ❌ | ❌ |
| **Crash reporting** | Phase 4 WER | ✅ Windows telemetry | ❌ | ❌ | ❌ | ✅ WER | ❌ | macOS crash | macOS crash | ❌ | ❌ | ❌ | systemd journal | N/A | N/A | Sentry | Sentry |
| **Cross-platform** | Win now; mac Phase 5; lin Phase 6 | Windows only | Windows only | Windows only | Windows only | Windows only | Win/Mac/Lin | Mac only | Mac only | Windows | Windows | Win/Mac/Lin | Linux | all | all (JVM) | all | all |
| **Corpus-validated quality** | ✅ SSIM-gated CI | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |

### 3.2 Unique competitive advantages (real moat, not aspirational)

1. **Native IThumbnailProvider × modern formats × MIT** — no MIT-licensed modern shell extension exists for AVIF/JXL/HEIC. PowerToys is MIT but does not do thumbnails; QuickLook is GPL-3 + separate window.
2. **Corpus-validated quality in CI** — no competitor runs automated SSIM regression against real files on every commit. This is our clearest technical moat.
3. **Archive + eBook cover thumbnails natively** — CBZ/CBR/CB7/EPUB cover art is invisible to every competitor on Windows.
4. **Headless CLI + REST API + shell integration from one codebase** — Apache Tika is headless-only; QuickLook is GUI-only; we span both.
5. **SBOM + signed MSI + GPO-ready** — no open-source Windows shell extension competitor ships all three.
6. **HDR-correct rendering** — HLG/PQ tone-mapping to SDR. No Windows-native competitor does this.

### 3.3 Gaps vs. best-in-class (concrete actions required)

| Gap | Best competitor | Our Phase |
|-----|-----------------|-----------|
| IPreviewHandler Preview Pane | PowerToys Peek (Microsoft) | 3 |
| IPropertyStore Details columns | Icaros (video), Windows built-in (images) | 3 |
| IContextMenu right-click verbs | SageThumbs, PowerToys | 3 |
| Video keyframe thumbnails | Icaros (best-in-class on Windows) | 3 |
| WER crash reporting | Windows built-in, PowerToys | 4 |
| Out-of-process decode host | GNOME Tumbler isolation model | 4 |
| 3D model thumbnails (glTF/OBJ) | No good Windows competitor | 3 |
| LGPL static-link compliance notice | libvips publishes NOTICE | 2 |

---

## 4. Best-in-Class Distillation — 18 Harvested Practices

v5.0 adds 5 new harvested practices (H14–H18) from the expanded matrix. All are mapped to phases.

| # | Source | Practice | Gap in ExplorerLens v4 | Our Adoption | Phase |
|---|--------|----------|------------------------|-------------|-------|
| **H1** | QuickLook | Community plugin ecosystem (20+ .qlplugin in wild, Scoop-distributed nightly builds) | Plugin SDK exists but no reference plugin, no discovery | C-ABI `plugin_api.h` + FITS reference plugin + winget-style discovery | 3 |
| **H2** | SageThumbs | `IContextMenu` "Preview thumbnail" right-click verb | Not implemented; critical for discoverability | `IContextMenu` verb "Preview thumbnail here" | 3 |
| **H3** | Icaros | FFmpeg video keyframe + `IPropertyStore` metadata in Details view | No video keyframes; Details view empty | Media Foundation keyframes + `IPropertyStore` for 20+ formats | 3 |
| **H4** | XnView MP | Streaming probe-then-decode (`ProbeHeader()` → `DecodeAtSize()`) | `IStreamingDecoder` defined; not wired | Wire into pipeline dispatch | 1 |
| **H5** | macOS Quick Look | SQLite-backed thumbnail cache + `FSEvents`-style invalidation | L1 in-process only; no L2 persistence | SQLite WAL L2 + `ReadDirectoryChangesW` invalidation | 2 |
| **H6** | Preview.app | Live preview pane with PDF page turn / video scrub | No `IPreviewHandler` | `IPreviewHandler` for PDF + images | 3 |
| **H7** | IrfanView | 3 MB binary, `LibRaw::unpack_thumb()` embedded JPEG (100× faster than full decode) | Full LibRaw decode on every RAW | Route LibRaw embedded preview before full decode | 1 |
| **H8** | ImageGlass | MSIX + Store + dark theme + high release cadence | No MSIX; dark mode partial | MSIX Phase 5; dark mode Phase 2; monthly minor cadence | 5/2 |
| **H9** | Nomacs | Stereo / panoramic RAW aware | Not applicable yet | Defer to Phase 7 (3D-photo) | 7 |
| **H10** | GNOME Tumbler | Per-format out-of-process daemon via D-Bus | In-process only; decoder crash = Explorer crash | Out-of-process decode host (COM OOP + AppContainer) | 4 |
| **H11** | libvips | Constant-memory streaming decode regardless of image size; SIMD throughout | SIMD in specific decoders only | Streaming-I/O + SIMD-first pattern in TIFF/PSD | 2 |
| **H12** | Apache Tika | `FormatDetector` as a pure stateless library, separate from decoders | ✅ Done: `StatelessFormatDetector.h` | Wire as mandatory first stage of every decode | 1 |
| **H13** | Windows built-in | `IPropertyStore` schema breadth (~100 properties) | No properties published | Dimension, codec, camera EXIF, duration, color-space | 3 |
| **H14** | **PowerToys Peek** | `IPreviewHandler` composited inside Explorer Preview Pane — the reference Windows implementation | Not implemented — #1 UX gap | `IPreviewHandler` for images, PDF, archives, code, fonts | 3 |
| **H15** | **Eagle (asset manager)** | Perceptual hash + dominant-color palette on thumbnails for duplicate detection / smart sort | No pHash; no palette metadata | pHash + dominant-color palette as optional metadata | 4 |
| **H16** | **libvips CLI + Apache Tika** | Pure-library design: thumbnailer-as-a-library any host can embed | Engine is .lib; no embeddable .dll | Publish `ExplorerLensEngine.dll` for embedding | 4 |
| **H17** | **PowerToys** | MIT-licensed, Microsoft-maintained, 110K stars — proves Windows shell ext at MIT scale | We are early and MIT | Learn their developer onboarding: Dev Home + WinGet integration | 1 |
| **H18** | **Figma / Design tools** | SVG + design file thumbnails via rendering | No SVG/design file thumbnails | SVG via Direct2D Phase 3; Figma/Sketch/XD via WASM plugin Phase 6 | 3/6 |

### 4.1 Anti-patterns deliberately rejected

| Practice | Source | Why we reject |
|----------|--------|---------------|
| Separate viewer window | QuickLook | We are `IThumbnailProvider` — Explorer-native is our entire value |
| .NET 8/9 runtime in Shell DLL | ImageGlass, PowerToys | 15+ MB dep; GC pauses in `explorer.exe` |
| "500+ formats" claim without corpus | XnView MP | Every claimed format must have ≥1 SSIM-tested real file in CI |
| JVM runtime | Apache Tika | Never in a Windows shell extension |
| Separate process for all decoders | GNOME Tumbler | Only for Phase 4 untrusted decoders; P0 stays in-process for latency |
| GPL-contaminated core | QuickLook, SageThumbs, Icaros | MIT license is core moat; no GPL in default build |
| Monolithic 80 MB install | XnView MP | < 5 MB DLL, < 20 MB full install |

---

## 5. Language, Runtime & Compiler

### 5.1 Per-component decision matrix

| Component | Language | Standard | Compiler | Runtime | Rationale |
|-----------|----------|----------|---------|---------|-----------|
| `LENSShell.dll` | C++ | **C++23** | MSVC v145 | `/MD` UCRT | COM ABI; static link; `explorer.exe` constraints |
| `ExplorerLensEngine.lib` | C++ | **C++23** | MSVC v145 | `/MD` | Core library; `std::expected`, `std::print`, flat containers |
| `LENSManager.exe` | C++ + WTL | C++20 | MSVC v145 | `/MD` | Native ~400 KB GUI; no .NET dependency |
| `lens.exe` | C++ | C++20 | MSVC v145 | `/MD` | Same Engine linkage |
| `lens-server` (Phase 4) | C++ + cpp-httplib | C++20 | MSVC v145 | `/MD` | Containerized REST server |
| Build scripts | PowerShell 7.5 | — | — | pwsh | Consolidated; vcpkg retires most custom scripts |
| Rust sidecar (Phase 4 eval) | **Rust 1.80+ stable** | 2024 edition | rustc | — | `windows-rs` COM interop; memory-safe untrusted decoder host |
| AI / ML (Phase 5) | C++ + ONNX Runtime C API | C++20 | MSVC v145 | `/MD` | DirectML EP; quantized models; never in shell DLL hot path |
| Tests | C++ + Catch2 v3 | C++20 | MSVC v145 | `/MD` | Industry standard; replaces custom macros |

### 5.2 C++23 adoption plan

| Feature | Available in MSVC v145 | Adoption phase | Guard |
|---------|----------------------|---------------|-------|
| `std::expected<T,E>` | ✅ 19.43+ | **Phase 1** for all new Engine APIs | `#if __cpp_lib_expected >= 202202L` |
| `std::print` / `std::println` | ✅ 19.48+ | **Phase 2** for diagnostics/CLI | `#if __cpp_lib_print` |
| `std::flat_map` / `std::flat_set` | ✅ 19.50 | **Phase 2** for decoder registry | `#if __cpp_lib_flat_map` |
| Deducing `this` | ✅ | **Phase 2** | Always in MSVC v145 |
| `std::stacktrace` | ✅ 19.44+ | **Phase 4** crash reporting | `#if __cpp_lib_stacktrace` |
| `std::generator` (coroutines) | ✅ 19.44+ | **Phase 3** streaming decoders | `#if __cpp_lib_generator` |
| `import std;` (modules) | experimental | **Phase 5 evaluate only** | Off by default |

### 5.3 Alternatives explicitly rejected

| Alternative | Reason |
|-------------|--------|
| Rust for Engine core | COM interop complexity; ABI incompatibility; contributor barrier |
| C# / .NET for in-process components | GC pauses in `explorer.exe`; 15 MB runtime |
| Go | No stable COM story; GC; binary overhead |
| Zig | Immature toolchain; no MSVC v145 ABI guarantee |
| Python in build-critical path | Must build from PowerShell 7 only |
| CUDA / ROCm | Vendor lock; D3D12 Video + DirectML is the GPU-vendor-agnostic path |
| `__builtin_*` GCC intrinsics | MSVC does not support; use `memcpy()`, `std::bit_cast`, `_mm_*` |

---

## 6. Frontend — Shell, GUI, CLI & Web

### 6.1 `LENSShell.dll` — COM interfaces roadmap

| Interface | Status | Priority | Phase | Notes |
|-----------|--------|----------|-------|-------|
| `IThumbnailProvider` | ✅ Implemented | P0 | — | Validate 16×16 → 1024×1024; all Explorer size classes |
| `IInitializeWithStream` | ✅ Implemented | P0 | — | Prefer streams; enables OneDrive placeholder hydration |
| `IPropertyStore` | ❌ Missing | **P0 Phase 3** | 3 | Details pane: dimensions, codec, camera EXIF, duration, color-space |
| `IContextMenu` | ❌ Missing | **P1 Phase 3** | 3 | Right-click "Preview thumbnail here" [H2] |
| `IPreviewHandler` | ❌ Missing | **P0 Phase 3** | 3 | Preview Pane — PowerToys Peek parity [H14] |
| `IExtractImage2` | Planned | P2 | 2 | Legacy Explorer compatibility |
| `IFilter` | Optional | P3 | 4 | Windows Search indexing of metadata |
| Thread model | STA | — | — | Audit MTA re-entrancy in Explorer thread pool |

#### 6.1.1 IPreviewHandler design [H14]

```
Preview Host (explorer.exe)
    │   SetWindow(hwnd, rect) / SetRect(rect) / DoPreview()
    ▼
LENSPreviewHandler (in LENSShell.dll)
    ├── Images: scale + display in child HWND (GDI+ → Direct2D Phase 2)
    ├── PDF: page 0 full resolution; page-turn keyboard nav
    ├── Archives: cover page + file list
    ├── Fonts: "Aa Bb Cc 0123456789" specimen (FreeType Phase 3)
    └── Video (Phase 4): first keyframe static; scrub bar via Media Foundation
```

### 6.2 `LENSManager.exe` — configuration GUI

| Feature | Status | Phase |
|---------|--------|-------|
| Dark mode (DarkModeController.h) | In progress | 2 |
| Performance dashboard (cache hit/miss, P50/P95 per decoder) | ❌ | 3 |
| Decoder health check | Stub | 2 |
| System tray balloon notifications | ❌ | 3 |
| Plugin manager (list/enable/disable/trust) | ❌ | 3 |
| Localization readiness (resource strings isolated) | ❌ | 3 |
| UIA automation peers + high-contrast + keyboard nav | ❌ | 4 |
| WinUI 3 evaluation gate | — | Phase 4 gate only if justified |

### 6.3 `lens.exe` — CLI (Phase 1: all 8 commands)

| Command | Status | Phase |
|---------|--------|-------|
| `lens generate <file> [-o out.png] [-s 256]` | Partial | 1 |
| `lens info <file>` | Partial | 1 |
| `lens register [--per-user\|--system]` | Partial | 1 |
| `lens doctor` | Stub | 1 |
| `lens benchmark <dir> [--json]` | Stub | 2 |
| `lens cache [stats\|purge\|compact\|warm <dir>]` | Stub | 2 |
| `lens serve [--port 8080] [--workers N]` | Stub | 4 |
| `lens plugin [list\|enable\|disable\|trust\|install <url>]` | Stub | 3 |

### 6.4 Web page (`index.html` → GitHub Pages)

**Decision: Keep static HTML. No framework.**
- Content: live Explorer screenshots, format matrix SVG (generated), download CTA, SSIM coverage badge, SBOM link.
- < 100 KB HTML+CSS; no JavaScript for core content; optional vanilla JS for format-search filter (< 5 KB).

---

## 7. Backend — Engine, Decode Pipeline, Cache, GPU

### 7.1 System architecture

```
Windows Explorer
    │
    ▼
LENSShell.dll
    ├── IThumbnailProvider
    ├── IPropertyStore      (Phase 3)
    ├── IContextMenu        (Phase 3)
    └── IPreviewHandler     (Phase 3)
         │
         ▼
    ExplorerLensEngine.lib
    │
    ├── Core/
    │    ├── StatelessFormatDetector  (pure, no decoder deps)  [H12, D43]
    │    ├── DecoderRegistry          (routing + priority tiers)
    │    ├── DecodePipeline           (probe → route → decode → transform → output) [H4]
    │    ├── CacheProvider            (L1 LRU + L2 SQLite Phase 2)  [H5, D42]
    │    ├── GPURenderer              (D3D11 Phase 2, DXVA2 Phase 3, D3D12 Phase 4)
    │    ├── ObservabilityIntegration (ETW + Event Log + OTLP Phase 4)
    │    └── PluginHost               (C-ABI Phase 3; OOP AppContainer Phase 4)
    ├── Decoders/
    │    ├── P0: JPEG(libjpeg-turbo), PNG, WebP, AVIF, HEIC, JXL, PDF, RAW(LibRaw)
    │    ├── P1: ZIP/CBZ, RAR/CBR, 7Z, EPUB, GIF, TIFF, BMP
    │    ├── P2: EXR, PSD, DDS, SVG, TTF, HDR, QOI, TGA
    │    └── P3: Video(MF), glTF, OBJ, STL, DICOM, FITS
    ├── GPU/     (D3D11 Phase 2, DXVA2 Phase 3, D3D12 Phase 4, Vulkan Phase 5)
    ├── Cache/   (L1 Robin-Hood LRU + L2 SQLite WAL + mmap blobs Phase 2)
    ├── Platform/ (Win32; macOS Phase 5; Linux Phase 6)
    ├── Utils/
    └── Tests/   (Catch2 v3, Google Benchmark, corpus runner, fuzz harnesses)
```

### 7.2 Engine directory consolidation (16 → 7) — Phase 1 mandatory

| Directory | Fate |
|-----------|------|
| `Engine/Core/` | ✅ Keep — absorbs AI, Enterprise, Pipeline, Plugin, PluginHost, CLI |
| `Engine/Decoders/` | ✅ Keep — absorbs Codec, Media |
| `Engine/GPU/` | ✅ Keep |
| `Engine/Cache/` | ✅ Keep — absorbs Memory |
| `Engine/Platform/` | ✅ Keep |
| `Engine/Tests/` | ✅ Keep |
| `Engine/Utils/` | ✅ Keep |
| `Engine/AI/` | **Delete** → fold to Core |
| `Engine/Enterprise/` | **Delete** → fold to Core |
| `Engine/Media/` | **Delete** → fold to Decoders |
| `Engine/Memory/` | **Delete** → fold to Cache |
| `Engine/Pipeline/` | **Delete** → fold to Core |
| `Engine/Plugin/`, `Engine/PluginHost/` | **Delete** → fold to Core |
| `Engine/CLI/` | **Delete** → already in `src/Tools.CLI/` |
| `Engine/Codec/` | **Delete** → fold to Decoders |

### 7.3 Decoder priority tiers

| Tier | Formats | Libraries | P50 budget | SSIM gate |
|------|---------|-----------|-----------|-----------|
| **P0** | JPEG, PNG, WebP, AVIF, HEIC, JXL, PDF cover, RAW embedded | libjpeg-turbo (**new**), libpng/WIC, libwebp, libavif+dav1d, libheif+libde265, libjxl, PDFium (Phase 3), LibRaw embedded [H7] | 5–25 ms | ≥ 0.98 SSIM |
| **P1** | ZIP/CBZ, RAR/CBR, 7Z/CB7, EPUB, GIF, BMP, TIFF, APNG | minizip-ng, UnRAR, LZMA SDK, libarchive, WIC | 5–20 ms | ≥ 0.97 SSIM |
| **P2** | EXR, PSD, DDS/BC, SVG, TTF/OTF, HDR, QOI, TGA, ICO, JP2 | tinyexr, custom PSD, DirectXTex, Direct2D, FreeType, stb_image, WIC, openjpeg | 5–15 ms | ≥ 0.95 SSIM |
| **P3** | MP4/MKV/WebM keyframe, glTF/OBJ/STL, DICOM, FITS, XPS, DWG (plugin) | Media Foundation, Assimp, plugin SDK | 10–30 ms | ≥ 0.90 SSIM |

### 7.4 Cache architecture [H5, D42]

| Tier | Storage | Technology | Budget | Hit-path P50 |
|------|---------|-----------|--------|-------------|
| **L1** | In-process | Robin-Hood hashmap + LRU, XXH3 key | 64 MB configurable | < 500 μs |
| **L2 index** | `%LOCALAPPDATA%\ExplorerLens\cache.db` | SQLite 3 WAL, read-concurrent | unlimited rows | < 3 ms |
| **L2 blobs** | `%LOCALAPPDATA%\ExplorerLens\Cache\*.thumb` | Memory-mapped, size-budgeted | 1 GB default | < 5 ms |
| **Invalidation** | Per-folder watcher | `ReadDirectoryChangesW` (subtree=false) | — | < 100 ms |

**Cache key:** `SHA256(canonical_path ‖ mtime ‖ file_size ‖ target_w×h ‖ decoder_version)`

### 7.5 GPU pipeline phases

| Phase | Feature | Technology | Target |
|-------|---------|-----------|--------|
| 2 | Resize via D3D11 compute | `resize_bilinear.hlsl` (HLSL CS 5.0) | < 0.5 ms 4K→256px |
| 2 | WIC + D3D11 decode hints | `IWICImagingFactory2` with D3D11 device | 1.5–2× JPEG/PNG |
| 3 | Video keyframe extraction | DXVA2 / Media Foundation | < 30 ms first frame |
| 3 | HDR→SDR tone-mapping | `tonemap_pq_srgb.hlsl` | < 0.5 ms |
| 4 | Unified GPU compute + video | D3D12 Video Decode | GPU-vendor-agnostic |
| 5 | Cross-platform GPU | Vulkan Video | macOS/Linux parity |

---

## 8. API Design & Error Handling

### 8.1 Error handling policy (ADR-015)

**New uniform policy — replaces mixed HRESULT/exception pattern:**

| Layer | Mechanism | Reason |
|-------|-----------|--------|
| COM boundary (LENSShell.dll ↔ Explorer) | `HRESULT` only | COM specification requirement |
| Engine internal APIs (new) | `std::expected<T, EngineError>` | C++23; composable; zero exception overhead |
| Engine internal APIs (legacy) | Gradual migration | Phase 1–2 |
| Test harness | C++ exceptions OK | Catch2 uses exceptions |
| Constructors | No-throw; factory `Create()` returns `std::expected` | Prevent cross-module exception propagation |

```cpp
namespace ExplorerLens::Core {

enum class EngineError : uint32_t {
    OK = 0,
    STREAM_TOO_SHORT,
    FORMAT_UNSUPPORTED,
    DIMENSIONS_OVERFLOW,
    DECODE_FAILED,
    GPU_UNAVAILABLE,
    CANCELLED,
    ZIP_BOMB_REJECTED,
    LICENCE_RESTRICTION,
};

// IStreamingDecoder — all new decoders implement this
struct DecodeResult {
    std::expected<Bitmap, EngineError> bitmap;
    std::optional<Metadata>            meta;
    PartialDecodeState                 state; // COMPLETE|PARTIAL|HEADER_ONLY|FAILED
    std::chrono::microseconds          elapsed;
};

struct IStreamingDecoder {
    virtual ~IStreamingDecoder() = default;
    virtual DecodeResult ProbeHeader(std::span<const uint8_t> first16KB) = 0;
    virtual DecodeResult DecodeAtSize(IStream* stream, uint32_t targetPx,
                                      std::stop_token cancel) = 0;
    virtual bool SupportsPartialDecode()   const noexcept = 0;
    virtual bool SupportsEmbeddedPreview() const noexcept { return false; }
    virtual std::string_view DecoderName()    const noexcept = 0;
    virtual uint32_t         DecoderVersion() const noexcept = 0;
};

// HRESULT mapping at COM boundary
constexpr HRESULT ToHRESULT(EngineError e) noexcept {
    switch (e) {
    case EngineError::OK:                  return S_OK;
    case EngineError::FORMAT_UNSUPPORTED:  return WINCODEC_ERR_UNSUPPORTEDPIXELFORMAT;
    case EngineError::DECODE_FAILED:       return E_FAIL;
    case EngineError::DIMENSIONS_OVERFLOW: return WINCODEC_ERR_IMAGESIZEOUTOFRANGE;
    case EngineError::STREAM_TOO_SHORT:    return STG_E_READFAULT;
    case EngineError::ZIP_BOMB_REJECTED:   return E_ABORT;
    case EngineError::CANCELLED:           return E_ABORT;
    default:                               return E_UNEXPECTED;
    }
}

} // namespace ExplorerLens::Core
```

### 8.2 `lens-server` REST API (Phase 4)

Base URL: `http://localhost:8080/api/v1/`  
Auth: `Authorization: Bearer <key>` on all write/decode endpoints.

| Endpoint | Method | Notes |
|----------|--------|-------|
| `/thumbnail` | POST | `{"path":"...", "size":256}` → `{"format":"png","data":"<b64>","elapsed_ms":12}` |
| `/info` | POST | `{"path":"..."}` → `{"format":"jpeg","width":6000,"decoder":"libjpeg-turbo"}` |
| `/formats` | GET | List of all decoders with tier and version |
| `/health` | GET | Unauthenticated health check |
| `/cache/stats` | GET | L1/L2 hit rates and sizes |
| `/cache/purge` | DELETE | Authenticated; purges L2 |

---

## 9. External Libraries, APIs & Data Sources

### 9.1 Full library registry (30 total)

| # | Library | Version | SPDX | Status | Purpose | Phase |
|---|---------|---------|------|--------|---------|-------|
| 1 | zlib | 1.3.1 | Zlib | ✅ | Deflate | now |
| 2 | LZ4 | 1.10.0 | BSD-2-Clause | ✅ | Fast compression | now |
| 3 | zstd | 1.5.7 | BSD-3-Clause | ✅ | Modern compression | now |
| 4 | LZMA SDK | 26.00 | LicenseRef-LZMA | ✅ | 7-zip/xz | now |
| 5 | minizip-ng | 4.0.10 | Zlib | ✅ | ZIP/CBZ | now |
| 6 | bzip2 | 1.0.8 | LicenseRef-bzip2 | ✅ | BZ2 | now |
| 7 | UnRAR | 7.2.2 | LicenseRef-UnRAR | ✅ | RAR/CBR | now |
| 8 | libwebp | 1.5.0 | BSD-3-Clause | ✅ | WebP P0 | now |
| 9 | libavif | 1.3.0 | BSD-2-Clause | ✅ | AVIF P0 | now |
| 10 | libjxl | 0.11.1 | BSD-3-Clause | ✅ | JPEG XL P0 | now |
| 11 | libheif | 1.19.5 | LGPL-3.0-only | ✅ ⚠️ | HEIC P0 | now |
| 12 | libde265 | 1.0.15 | LGPL-3.0-only | ✅ ⚠️ | H.265 HEVC | now |
| 13 | dav1d | 1.5.1 | BSD-2-Clause | ✅ | AV1 P0 | now |
| 14 | LibRaw | 0.21.3 | LGPL-2.1-only | ✅ ⚠️ | RAW P0 | now |
| 15 | openjpeg | 2.5.3 | BSD-2-Clause | ✅ | JPEG-2000 P2 | now |
| 16 | **MuPDF** | **1.24.11** | **AGPL-3.0-only** | ⛔ **Replace Phase 3** | PDF | → PDFium |
| 17 | xz/liblzma | 5.6.3 | LicenseRef-XZ | ✅ | XZ archives | now |
| 18 | libarchive | 3.7.6 | BSD-2-Clause | ✅ | TAR/ISO/misc | now |
| 19 | **libjpeg-turbo** | **3.1.0** | BSD-3-Clause | **Add P0** | JPEG 2–6× faster | **1** |
| 20 | **Catch2** | **3.7.1** | BSL-1.0 | ✅ FetchContent | Test framework | now |
| 21 | **Google Benchmark** | **1.9.x** | Apache-2.0 | Add | Perf regression | **1** |
| 22 | **SQLite 3** | **3.47.x** | Public domain | Add | L2 cache index | **2** |
| 23 | **stb_image** | **2.30** | MIT-0 | Add | QOI/PNM/TGA/HDR | **2** |
| 24 | **tinyexr** | **1.0.9** | BSD-3-Clause | Add | OpenEXR P2 | **2** |
| 25 | **DirectXTex** | **2024.10** | MIT | Add | DDS/BC textures | **2** |
| 26 | **PDFium** | latest | BSD-3-Clause | Add | PDF (replaces MuPDF) | **3** |
| 27 | **FreeType 2** | **2.13.x** | FTL | Add | Font thumbnails | **3** |
| 28 | **Assimp** | **5.4.x** | BSD-3-Clause | Add | 3D model P3 | **3** |
| 29 | **cpp-httplib** | **0.18.x** | MIT | Add | REST server | **4** |
| 30 | **ONNX Runtime** | **1.20.x** | MIT | Add (opt-in) | AI inference | **5** |

⚠️ LGPL: requires LGPL exception notice in SBOM + `NOTICE` file. Legal review Phase 2 (D53).

### 9.2 Libraries to remove from docs/claims today

Strip references to: NVJPEG/CUDA, Intel oneVPL, AMD AMF, nghttp2, OpenCASCADE, IfcOpenShell, Open3D. If they arrive, they arrive as plugins.

### 9.3 Windows APIs consumed

| API | Purpose | Phase |
|-----|---------|-------|
| Windows Imaging Component (WIC) | Image host + DXGI sharing | now |
| Media Foundation | Video keyframe extraction | 3 |
| Windows Property System | `IPropertyStore` schema | 3 |
| Cloud Files API | OneDrive placeholder hydration | 4 |
| Windows Error Reporting (WER) | Crash dump | 4 |
| AppContainer / AppInstaller | Sandboxing + MSIX | 4–5 |
| DirectML | Hardware-agnostic AI inference | 5 |
| GitHub Releases API (HTTPS) | In-app update check | 4 |
| `ReadDirectoryChangesW` | Cache invalidation watcher | 2 |

### 9.4 Corpus data sources

| Source | Formats | License | Pull method |
|--------|---------|---------|------------|
| libjxl reference images | JXL | BSD-3 | `Fetch-Corpus.ps1` SHA-pinned |
| HEIF demo files (Apple/GPAC) | HEIC/AVIF | Sample use | SHA-pinned |
| rawsamples.ch | RAW (DNG/CR2/NEF/ARW/…) | CC0/CC-BY | Attribution file |
| Project Gutenberg | EPUB | Public domain | Header only |
| Blender Open Movies | MP4/MKV | CC-BY | Keyframe only |
| Google Fonts | TTF/OTF | SIL OFL | SHA-pinned |
| Wikimedia Commons (curated) | General images | CC0/CC-BY | SHA-pinned |
| AV1 Image File Format test suite | AVIF | CC0 | AOM reference repo |
| OpenEXR test images | EXR | BSD-3 | OpenEXR repo |

Ingest: `build-scripts/corpus/Fetch-Corpus.ps1 -StrictLicenses` — validates SHA256, writes `MANIFEST.json`.

---

## 10. Build System & Toolchain

### 10.1 Toolchain versions

| Tool | Version | How pinned |
|------|---------|-----------|
| CMake | 4.3.1 | `cmake_minimum_required(VERSION 3.25)` |
| Ninja | 1.13.2 | winget / devcontainer |
| MSVC | cl 19.50 (v145) | VS 2026 BuildTools |
| vcpkg | 2026-02-21 | `vcpkg-configuration.json` baseline |
| Windows SDK | 10.0.26100.0 | VS BuildTools |
| WiX | 6.0.2 | winget |
| sccache | 0.11.x | Cargo / winget |
| PowerShell | 7.5 | winget |
| Rust (Phase 4 eval) | 1.80+ stable | rustup toolchain file |

### 10.2 vcpkg manifest target state

```json
{
  "name": "explorerlens",
  "version": "39.1.0",
  "dependencies": [
    "zlib", "lz4", "zstd", "liblzma", "minizip-ng", "bzip2", "libarchive",
    "libwebp", "libjxl", "libavif", "libheif", "libde265", "libraw",
    "openjpeg", "libjpeg-turbo", "dav1d",
    "catch2", "benchmark",
    "stb", "tinyexr", "directxtex", "sqlite3",
    "freetype", "assimp",
    "pdfium", "cpp-httplib"
  ],
  "features": {
    "ai":     { "description": "Smart-crop + aesthetic score (Phase 5)", "dependencies": ["onnxruntime"] },
    "server": { "description": "REST API lens-server (Phase 4)", "dependencies": ["cpp-httplib"] }
  }
}
```

### 10.3 Build-script retirement plan (13 scripts → vcpkg)

Scripts for zlib, LZ4, zstd, LZMA-SDK, minizip-ng, LibRaw, LibWebP, Dav1d, LibHEIF, and others are retired in Phases 1–2 as vcpkg covers them. `Build-MSVC.ps1` and `Build-All-And-Package.ps1` are **not retired** — they orchestrate the full build.

### 10.4 Build improvements

| Improvement | Priority | Impact |
|-------------|----------|--------|
| PCH for Win32 / STL / COM | **P0** | 30–50% rebuild time reduction |
| `/d1reportTime` compile-time profiling | P1 | Identify slow headers |
| Unity builds | ✅ already | CI option |
| sccache | ✅ already | Dev iteration |
| `import std;` modules | P3 | Phase 5 evaluate only |

---

## 11. Testing & Quality Strategy

### 11.1 Framework policy

| Layer | Framework | Count target | Status |
|-------|-----------|-------------|--------|
| **Unit (new)** | Catch2 v3 | ≥ 500 meaningful | 42 files, ~1,260 tests (Sessions 1–8) |
| **Unit (legacy)** | Custom TEST/ASSERT macros | Phase out by Phase 2 | ~4,978 |
| **Decoder corpus validation** | Catch2 + corpus runner | ≥ 600 (20 formats × 3 files × 3 assertions) | Phase 2 |
| **Integration (COM harness)** | Catch2 | ≥ 50 | Phase 3 |
| **GPU SSIM** | Catch2 + D3D11 | ≥ 20 | Phase 2 |
| **Benchmarks** | Google Benchmark | ≥ 30 BM_ fixtures | Phase 1/2 |
| **Fuzz** | libFuzzer / WinAFL | ≥ 20 targets | Phase 4 |
| **Property-based** | Catch2 `GENERATE` | ≥ 50 | throughout |

**Total target: ~1,300 deep tests replacing ~4,978 shallow ones.**

### 11.2 Current Catch2 file inventory (42 files, Sessions 5–8)

Catch2 files delivered in sequence (see §23 for full sprint log):

`ResultTypeTests`, `ProbeHeaderTests`, `DecoderFallbackChainTests`, `CacheKeyTests`, `CLICommandParserTests`, `ColorSpaceTests`, `CorpusCoverageTests` (S5) → `EngineConfigTests`, `PathValidationTests`, `ObservabilityTests`, `VersionValidationTests`, `FormatFamilyTests`, `ThumbnailBenchmarks`, `MemoryBudgetTests` (S6) → `IStreamingDecoderTests`, `CacheTierTests`, `DecoderPriorityTierTests`, `CLIContractTests`, `PluginSDKContractTests`, `PlatformProfileTests`, `ErrorDomainTests`, `CompressionAlgorithmTests` (S7) → `FormatDetectorTests`, `ThumbnailDimensionTests`, `WICCodecTableTests`, `SecurityBoundaryTests`, `BitmapAlphaTests`, `MultiPageTests`, `COMInterfaceTests`, `LibraryInventoryTests` (S8) + earlier foundation files (`SafeDimensionsTests`, `PipelineIntegrationTests`, `MetadataExtractionTests`, `InputValidationTests`, `DecoderRegistryTests`, `InputValidationTests`, others).

### 11.3 Quality gates in CI

| Gate | Threshold | Workflow |
|------|-----------|----------|
| Zero errors / warnings | always | `build.yml` |
| Test pass rate | 100% | `ci-matrix.yml` |
| Coverage (Engine/Core) | ≥ 80% | `coverage.yml` |
| Performance P95 regression | < 10% | `performance-regression-gate.yml` |
| Binary size growth | < 10% | `binary-size.yml` |
| SSIM P0 formats | ≥ 0.98 | `corpus-validation.yml` |
| SSIM P2 formats | ≥ 0.95 | `corpus-validation.yml` |
| ASAN/UBSAN leaks | 0 | `sanitizer-ci.yml` (Phase 4) |
| Fuzz crash | 0 | `fuzz-ci.yml` (Phase 4) |
| Doc build (mkdocs strict) | pass | `docs-validation.yml` |

---

## 12. Database & Persistent Storage

### 12.1 Storage decisions

| Data | Store | Technology |
|------|-------|-----------|
| L1 cache | In-process | Robin-Hood hashmap + LRU |
| L2 cache index | `cache.db` | SQLite 3 WAL (concurrent reads) |
| L2 cache blobs | `Cache/*.thumb` | Memory-mapped files |
| User settings | Registry | `HKCU\Software\ExplorerLens` |
| Enterprise policy | Registry | `HKLM\Software\Policies\ExplorerLens` (ADMX Phase 4) |
| Corpus manifest | `data/corpus/MANIFEST.json` | JSON (git-tracked) ✅ |
| Benchmark history | `data/benchmarks/history.jsonl` | Append-only JSONL ✅ |
| Baselines | `data/baselines/*.json` | JSON (git-tracked) ✅ |
| Plugin trust | `plugins/trust.db` | SQLite 3 (Phase 3) |
| Telemetry window | `telemetry.db` | SQLite 3 rolling window (opt-in, Phase 4) |

### 12.2 Rejected alternatives

| Alternative | Why |
|-------------|-----|
| LevelDB | Process-exclusive lock breaks multi-Explorer tab scenarios |
| LMDB | Good but niche; SQLite is universally debuggable |
| RocksDB | Server-grade; 10× code size for our use case |
| Custom binary format | Crash-unsafe; no tooling |
| Windows Registry for cache | Too slow for thousands of entries; registry corruption = system damage |

---

## 13. Documentation & Configuration Standards

### 13.1 Right-size policy (130+ md → ~60)

| Tier | Examples | Rule |
|------|---------|------|
| T1 User | README, USER_GUIDE, CHANGELOG, LICENSE, QUICK_START | Reflect only working features; no "planned" language |
| T2 Developer | `docs/development/`, CONTRIBUTING, build-scripts/README | Accurate build + test instructions |
| T3 Architecture | ROADMAP (this), `docs/architecture/`, ADRs | Vision + current state labeled with version |
| T4 Historical | `CHANGELOG-archive.md`, `docs/archive/` | Past roadmaps, old decisions |

### 13.2 ADR registry (as of v39.1.0)

| ADR | Title |
|-----|-------|
| ADR-001 | COM registration strategy (CLSID fixed; per-user default) |
| ADR-010 | Catch2 migration plan (primary by Phase 2) |
| ADR-011 | IStreamingDecoder contract (ProbeHeader + DecodeAtSize) |
| ADR-012 | L2 cache architecture (SQLite WAL + mmap blobs) |
| ADR-013 | Cross-platform PAL (PlatformProfile.h; no runtime #ifdef) |
| ADR-014 | Safe integer overflow (SafeDimensions.h for all dimension math) |
| **ADR-015** | `std::expected` error handling (internal; HRESULT at COM boundary) |
| **ADR-016** | PDFium over MuPDF (BSD-3 vs AGPL; default Phase 3) |
| **ADR-017** | Engine 16→7 consolidation (remove speculative subdirectories) |
| **ADR-018** | IPreviewHandler implementation (PowerToys Peek pattern) |
| **ADR-019** | Rust sandbox sidecar evaluation (Phase 4; `windows-rs` COM interop) |

### 13.3 Standards

- All SVG diagrams use dark palette (13 delivered ✅ v38.2).
- `mkdocs build --strict` mandatory CI check.
- Dev container must produce passing build in < 15 min from clone.
- No doc mentions a feature without a corpus test or a working CLI path.

---

## 14. CI/CD, Packaging & Distribution

### 14.1 Workflow inventory (26 as of v38.7)

| Area | Workflows | Status |
|------|-----------|--------|
| Build | `build.yml`, `ci-matrix.yml`, `reusable-build.yml` | ✅ |
| Quality | `code-quality.yml`, `coverage.yml`, `docs-validation.yml`, `devcontainer-test.yml` | ✅ |
| Performance | `performance-regression-gate.yml` | ✅ |
| Security | `codeql.yml`, `dependency-review.yml`, `sbom.yml`, `pin-actions.yml` | ✅ |
| Corpus | `corpus-validation.yml` | ✅ |
| Release | `release.yml`, `release-drafter.yml`, `publish-packages.yml` | ✅ |
| Pages | `pages.yml` | ✅ |
| Nightly | `nightly.yml` | ✅ |
| Binary size | `binary-size.yml` | ✅ |
| Sanitizers | `sanitizer-ci.yml` | Stub → Phase 4 |
| Fuzz | `fuzz-ci.yml` | Stub → Phase 4 |

### 14.2 Distribution channels

| Channel | Status | Priority |
|---------|--------|----------|
| GitHub Releases (MSI + ZIP + SHA256 + SBOM) | ✅ primary | P0 |
| winget (`ExplorerLens.ExplorerLens`) | ✅ manifest exists | P0 — submit to public bucket Phase 1 |
| Scoop (`explorerlens`) | ✅ manifest exists | P0 — submit Phase 1 |
| Chocolatey | packaging exists | P1 — submit Phase 3 |
| NuGet (SDK) | ✅ | P1 — keep |
| Microsoft Store (MSIX) | planned | Phase 5 |
| ghcr.io container (`lens-server`) | planned | Phase 4 |
| npm | planned | Phase 6 (WASM only) |
| Maven, RubyGems | **Drop** | No consumers |

### 14.3 Release cadence

- **Monthly minor** (vX.Y.0): features + sprint test batch
- **Patch** (vX.Y.Z): bugs + security fixes, < 2-week turnaround
- **Major** (vX.0.0): architectural change (Engine consolidation, PDFium migration)
- All releases: `Bump-Version.ps1` → tag → `release.yml` → winget PR + Scoop PR

---

## 15. Infrastructure, Security & Observability

### 15.1 Security hardening

| Control | Priority | Status |
|---------|----------|--------|
| ASLR / DEP / CFG | P0 | ✅ |
| `/GS` stack canaries | P0 | ✅ |
| `SafeDimensions.h` overflow-safe math | P0 | ✅ S171 |
| Magic-byte + size + dimension validation before decode | P0 | In progress Phase 1 |
| ZIP bomb threshold (uncompressed > 512 MB → reject) | P0 | Phase 1 |
| Path traversal sanitization | P0 | ✅ S192 |
| Supply-chain SHA-pinned Actions | P1 | ✅ S163 |
| SBOM (SPDX + CycloneDX) | P1 | ✅ v37 |
| Private vulnerability disclosure | P1 | ✅ `SECURITY.md` |
| Dependabot | P1 | ✅ v37 |
| **LGPL exception notice in `NOTICE` file** | **P1** | **⏳ Phase 2 mandatory** |
| Authenticode EV code signing | P1 | Phase 4 |
| WER + minidump crash reporting (opt-in) | P1 | Phase 4 |
| ASAN + UBSAN CI | P1 | Phase 4 |
| libFuzzer / WinAFL (≥ 20 harnesses) | P1 | Phase 4 |
| Out-of-process decode host (AppContainer) | P2 | Phase 4 |
| Rust memory-safe sidecar (evaluation) | P3 | Phase 4 |

### 15.2 Observability stack

| Signal | Mechanism | Consumer |
|--------|-----------|----------|
| Decode latency (P50/P95/P99 per decoder) | ETW `ExplorerLens-Engine` | WPA, `lens benchmark`, `lens doctor` |
| Cache hit/miss ratio | Shared-memory perf counters | LENSManager dashboard |
| Error events | Windows Event Log | SIEM, `lens doctor` |
| Crash reports | WER + MiniDumpWriteDump (opt-in) | Phase 4 aggregator |
| Enterprise OTLP export | OpenTelemetry exporter | Phase 4 SIEM / Prometheus |

### 15.3 Crash reporting design (Phase 4)

```cpp
// In DllMain DLL_PROCESS_ATTACH:
SetUnhandledExceptionFilter([](EXCEPTION_POINTERS* ep) -> LONG {
    // 1. MiniDumpWriteDump: MiniDumpWithDataSegs | WithThreadInfo | WithHandleData
    // 2. PII-scrub file paths
    // 3. Enqueue to WER async (respects user consent)
    return EXCEPTION_CONTINUE_SEARCH; // do not swallow; let explorer.exe recover
});
```

Dumps to `%LOCALAPPDATA%\ExplorerLens\CrashDumps\`. Uploaded only with explicit consent.

---

## 16. Cross-Platform, AI/ML & Advanced Features

### 16.1 Honest cross-platform timeline

| Platform | Phase | Entry point | Status |
|----------|-------|-------------|--------|
| Windows 10/11 | 1–4 | `IThumbnailProvider` COM DLL | Core product |
| macOS 14+ | 5 | `QLThumbnailProvider` + same Engine | Homebrew formula + Metal |
| Linux (GNOME) | 6 | GNOME Tumbler + KIO helper | Flatpak + AppImage + Vulkan |
| Web / WASM | 7 | Emscripten + `lens-server` REST | npm |

**Remove from README today:** All "macOS/Linux supported via PAL stubs" claims.

### 16.2 AI/ML — gated behind `[ai]` feature flag (Phase 5 only)

**Policy: AI never runs inside the Shell DLL hot path.**

| Feature | Model | Phase |
|---------|-------|-------|
| Smart crop (saliency-aware) | MobileSaliency v2 INT8, ONNX+DirectML | 5 |
| Aesthetic score | NIMA INT8, ONNX+DirectML | 5 |
| Duplicate detection (pHash) | No model — CPU only | 4 [H15] |
| Dominant color palette | K-means on downsampled thumb | 4 [H15] |
| Scene understanding (CLIP embedding) | distilled-CLIP INT8, opt-in | 6 |
| Generative thumbnails | **Rejected** — not core mission | never |

---

## 17. Refactor / Rewrite / Delete Register

### 17.1 Delete (Phase 1)

| Target | Reason |
|--------|--------|
| `Engine/AI/`, `Engine/Enterprise/`, `Engine/Media/`, `Engine/Memory/`, `Engine/Pipeline/`, `Engine/Plugin/`, `Engine/PluginHost/`, `Engine/CLI/`, `Engine/Codec/` | Speculative stubs → fold to 7-dir layout |
| Maven + RubyGems publish workflows | No consumers |
| All cross-platform claims in README | Stubs ≠ support |
| All 13 `Build-*.ps1` external library scripts | vcpkg replaces them |
| MuPDF references in default-build docs | AGPL risk |

### 17.2 Refactor (Phase 1–2)

| Target | Action |
|--------|--------|
| Custom `TEST()`/`ASSERT()` macros | Gradual Catch2 migration; retire by Phase 2 |
| Header stubs without impl | Implement-before-declare; merge into .cpp |
| Mixed HRESULT/exception error handling | `std::expected<T,EngineError>` for new code |
| External library `Build-*.ps1` scripts | Replace with vcpkg install |
| `LensServer` Winsock2 | cpp-httplib Phase 4 |

### 17.3 Rewrite (Phase 3–4)

| Target | Phase |
|--------|-------|
| `LensServer` (cpp-httplib + thread pool + auth) | 4 |
| MuPDF → PDFium (feature-flagged) | 3 |
| GPU renderer (real HLSL compute shaders) | 2–3 |
| Plugin host (out-of-process AppContainer COM OOP) | 4 |

### 17.4 Add (by phase)

| Addition | Phase |
|---------|-------|
| libjpeg-turbo for JPEG (2–6× speedup) | 1 |
| `LibRaw::unpack_thumb()` embedded preview fast-path [H7] | 1 |
| `lens.exe` all 8 commands complete | 1 |
| Google Benchmark as primary perf framework | 1 |
| `NOTICE` file for LGPL libraries | 2 |
| SQLite 3 L2 cache | 2 |
| D3D11 compute resize shader | 2 |
| stb_image + tinyexr + DirectXTex | 2 |
| `IPropertyStore`, `IContextMenu`, `IPreviewHandler` | 3 |
| PDFium (replaces MuPDF default) | 3 |
| FreeType + Assimp | 3 |
| WER crash reporting + Authenticode signing | 4 |
| Out-of-process AppContainer decode host [H10] | 4 |
| pHash + dominant-color palette [H15] | 4 |
| `ExplorerLensEngine.dll` (embeddable) [H16] | 4 |
| ONNX Runtime + DirectML (opt-in `[ai]`) | 5 |
| macOS QLThumbnailProvider | 5 |

---

## 18. 8-Phase Plan to Best-in-Class

### Phase 1 — Foundation Locked (current, v39.x → v40.0)

**Goal:** Every claimed format family validated by a corpus SSIM test. One-command install.

| Task | Status |
|------|--------|
| Engine 16→7 consolidation | ⏳ |
| libjpeg-turbo for JPEG P0 | ⏳ |
| `LibRaw::unpack_thumb()` embedded preview fast-path | ⏳ |
| `lens.exe` all 8 commands | ⏳ |
| 50+ Catch2 files (1,500+ tests) | ⏳ (42 now) |
| winget + Scoop public bucket submission | ⏳ manifests exist |
| `StatelessFormatDetector` wired into all decoders | ✅ S167 |
| `SafeDimensions.h` in all dimension math | ✅ S171 |
| `PlatformProfile.h` PAL | ✅ S177 |
| 100+ corpus entries | ✅ S187 (106) |
| SHA-pinned GitHub Actions | ✅ S163 |
| All 42 Catch2 files passing | ✅ |

**Exit:** Clean Windows 10 VM + `winget install ExplorerLens.ExplorerLens` → SSIM-validated thumbnails for top 20 format families.

### Phase 2 — Performance & Cache (v40.x)

- SQLite WAL L2 + mmap + `ReadDirectoryChangesW`
- D3D11 compute resize + WIC+D3D11 hints
- Google Benchmark wired to regression gate
- `lens benchmark` JSON output
- stb_image + tinyexr + DirectXTex
- Legacy macros retired; Catch2 only
- LGPL `NOTICE` file published
- LENSManager dark mode complete
- **Targets:** 5 ms JPEG, 8 ms WebP, 20 ms PDF, < 5 MB DLL, < 30 MB idle

### Phase 3 — Breadth & Integration (v41.x)

- `IPropertyStore`, `IContextMenu`, `IPreviewHandler` [H2, H3, H6, H13, H14]
- PDFium replacing MuPDF (feature-flagged)
- Media Foundation video keyframes + DXVA2 [H3]
- FreeType font thumbnails
- Assimp 3D model thumbnails (glTF/OBJ/STL)
- SVG via Direct2D [H18]
- Plugin SDK v1 + FITS reference plugin [H1]
- LENSManager performance dashboard + plugin manager
- Corpus: ≥ 300 files; Chocolatey submitted
- **≥ 80 corpus-validated format families**

### Phase 4 — Enterprise, Server & Hardening (v42.x)

- `lens-server` rewrite (cpp-httplib + thread pool + auth + ghcr.io) [H16]
- WER crash reporting + minidump (opt-in)
- Authenticode EV signing for all binaries + MSI
- ADMX/ADML GPO templates
- Out-of-process decode host (COM OOP + AppContainer) [H10]
- Rust sandbox sidecar prototype [D59]
- ASAN + UBSAN CI; libFuzzer ≥ 20 harnesses
- OpenTelemetry OTLP exporter (enterprise SKU)
- pHash + dominant-color palette [H15]
- MSIX packaging + in-app update check

### Phase 5 — Cross-Platform & AI (v43.x)

- macOS `QLThumbnailProvider` + Metal + Homebrew
- ONNX Runtime + DirectML: smart crop + aesthetic score (`[ai]`)
- MS Store (MSIX) publish
- i18n first locale (Japanese or German)
- UIA accessibility + high-contrast

### Phase 6 — Linux + WASM (v44.x)

- GNOME Tumbler + KDE KIO (Flatpak + AppImage)
- Vulkan Video
- Emscripten WASM; npm publish
- CLIP semantic embedding (opt-in, large model)
- Figma/Sketch/XD thumbnails via WASM plugin [H18]

### Phase 7 — Horizon

- D3D12 Video unified with compute queue
- Predictive pre-generation (navigation-pattern prefetch)
- 3D-photo / stereoscopic pair detection [H9]
- HDR gainmap / Apple ProRAW / Ultra HDR
- Post-quantum Authenticode

### Phase 8 — Ecosystem & Governance

- Third-party plugin registry (winget-manifest model)
- Enterprise commercial SDK license
- Community corpus contribution program
- Microsoft WHQL voluntary testing
- Security bounty program

---

## 19. Success Metrics & Exit Criteria

### Per-phase metrics

| Metric | Phase 1 | Phase 2 | Phase 3 | Phase 4 |
|--------|---------|---------|---------|---------|
| Format families (SSIM-validated) | ≥ 20 | ≥ 40 | ≥ 80 | ≥ 120 |
| Corpus real files | ≥ 100 ✅ 106 | ≥ 200 | ≥ 300 | ≥ 500 |
| Catch2 tests | ≥ 500 | ≥ 1,000 | ≥ 1,300 | ≥ 1,500 |
| SSIM ≥ 0.98 (P0) | in CI | all P0 | P0+P1 | all tiers |
| JPEG 6MP decode P50 | < 20 ms (WIC) | < 5 ms (libjpeg-turbo) | < 3 ms | < 3 ms |
| `LENSShell.dll` size | < 8 MB | < 6 MB | < 5 MB | < 5 MB |
| Idle RSS | < 50 MB | < 40 MB | < 30 MB | < 30 MB |
| Cache hit P50 | N/A | < 1 ms | < 1 ms | < 0.5 ms |
| Header:source ratio | ≤ 2.0:1 | ≤ 1.8:1 | ≤ 1.6:1 | ≤ 1.5:1 |
| Engine subdirectories | 7 | 7 | 7 | 7 |
| Zero errors / warnings | ✅ | ✅ | ✅ | ✅ |

### Best-in-class verification (measurable, not aspirational)

| Claim | How we verify |
|-------|--------------|
| More correct than Windows built-in | SSIM comparison on 106+ real files in CI; pixel-accurate EXIF rotation |
| Faster for every P0 format | Google Benchmark CI with regression gate; compare to WIC baseline |
| Lower footprint | `binary-size.yml` gate; mmap cache ≤ 1 GB disk |
| Zero crashes on 10K-file fuzz corpus | `fuzz-ci.yml` weekly; WER integration |
| One-command install | `winget install ExplorerLens.ExplorerLens` — tested in clean-VM CI |
| License-clean | No copyleft in default binary (after PDFium Phase 3); SBOM in every release |

---

## 20. Decision Log (v5.0)

All v4.0 decisions D1–D48 are preserved. v5.0 adds:

| # | Decision | Rationale |
|---|----------|-----------|
| **D49** | libjpeg-turbo replaces WIC for JPEG decode | 2–6× speedup; JPEG is the #1 format by volume |
| **D50** | `std::expected<T,EngineError>` mandatory for all new Engine APIs | COM-safe; C++23; replaces exception-through-COM anti-pattern |
| **D51** | Engine directory consolidation is Phase 1 mandatory, not deferred | 8 speculative directories with zero implementation are tech debt |
| **D52** | `IPreviewHandler` promoted to Phase 3 P0 | PowerToys Peek analysis: biggest UX win we are leaving on the table |
| **D53** | LGPL static-link compliance notice in `NOTICE` file (Phase 2) | Legal requirement for libheif, libde265, LibRaw static linking |
| **D54** | pHash + dominant-color palette in Phase 4 | Eagle-harvest [H15]; zero model dep; corpus-computable |
| **D55** | `ExplorerLensEngine.dll` (shared library) in Phase 4 | libvips/Tika pattern [H16]; enables embedding without recompile |
| **D56** | Remove all cross-platform claims from README immediately | `#ifdef` stubs are not support; macOS Phase 5, Linux Phase 6 |
| **D57** | Maven + RubyGems workflows deleted Phase 1 | No real consumers; maintenance burden |
| **D58** | cpp-httplib confirmed for `lens-server` | MIT; header-only; thread-pool built in |
| **D59** | Rust sandbox sidecar: formal evaluation Phase 4 | Memory safety for untrusted decoders; `windows-rs` 0.58+ |
| **D60** | PowerToys Peek pattern for IPreviewHandler | Reference implementation from Microsoft |

---

## 21. AI Tooling Surface & MCP

**Current baseline (v39.1.0):** 6 agents, 15 scoped instructions, 14 prompts, 7 skills, 3 MCP servers, 26 workflows.

### 21.1 Asset inventory

| Asset | Count | Location |
|-------|-------|---------|
| Copilot repository instructions | 1 | `.github/copilot-instructions.md` |
| Scoped instruction files | 15 | `.github/instructions/` |
| Custom agents | 6 | `.github/agents/` |
| Prompt templates | 14 | `.github/prompts/` |
| Repository skills | 7 | `.github/skills/` |
| Standards + workflow docs | 8 | `.github/standards/` |
| MCP servers | 3 | `.vscode/mcp.json` (github, filesystem, project-docs) |

### 21.2 Open items

| Item | Priority | Phase |
|------|----------|-------|
| Pin all 26 workflows to action commit SHA | P1 | 1 |
| SQLite MCP server (cache debugging) | P2 | 2 |
| Fetch MCP server (format spec lookup) | P2 | 2 |
| GitHub PAT scopes include `actions:read` | P1 | 1 |
| `library-audit` agent (LGPL + AGPL scanning) | P2 | 2 |
| `spec-fetch.prompt.md` (pull AV1/JXL/HEIF ISO specs) | P2 | 2 |
| Roadmap-guardian prompt (validate PR vs phase) | P2 | 1 |

---

## 22. Consolidated Legacy — What Survives from v4.0

**Fully carried forward (unchanged):**
- D1–D48 decision log (extended to D49–D60)
- Phase plan structure (Phase 1–7 → expanded Phase 1–8)
- Engine consolidation plan (16→7; now Phase 1 mandatory)
- `IStreamingDecoder` interface definition
- Cache architecture (L1 + L2 SQLite WAL + mmap)
- GPU pipeline phases (D3D11→DXVA2→D3D12→Vulkan)
- All test sprint logs
- Document right-sizing policy (130→~60)
- ADRs 001–014 (extended 015–019)

**Upgraded from v4.0:**

| v4.0 | v5.0 change |
|------|------------|
| IPreviewHandler Phase 3 P1 | Promoted to Phase 3 P0 (H14, D52) |
| LGPL notice "later" | Phase 2 mandatory (D53) |
| 12 competitors | 16 competitors + 4 new dimensions |
| 13 harvested practices | 18 harvested practices (H14–H18) |
| `std::expected` "consider" | Mandatory for all new Engine APIs (D50, ADR-015) |
| Engine consolidation "vague" | Concrete deletion list in §7.2, D51 |

**Archived to `docs/archive/ROADMAP_V4.md`:**
- v4.0 full text (preserved)
- ROADMAP_V30 "Deneb" LevelDB/CLIP/HNSW
- ROADMAP_V34 "Arcturus" 350+ extension Phase-1 target
- ROADMAP_V35 "Vega" real-time collab/SDXL-Turbo

---

## 23. Sprint Delivery Log (all sessions)

### Sessions 1–4 (S1–S140) — Foundation

Engine foundation, CMake, CI/CD (22→26 workflows), 13 SVG diagrams, MCP config, 6 agents, 15 instructions, 14 prompts, 7 skills, ADRs 001–014, SBOM, WiX MSI packaging, benchmark stubs, corpus scaffolding.

### Session 5 (S181–S188) — Catch2 Wave 1

S181 `ResultTypeTests` · S182 `ProbeHeaderTests` · S183 `DecoderFallbackChainTests` · S184 `CacheKeyTests` · S185 `CLICommandParserTests` · S186 `ColorSpaceTests` · **S187 MANIFEST.json v2 (16→106 entries)** · S188 `CorpusCoverageTests`

### Session 6 (S191–S198) — Catch2 Wave 2

S191 `EngineConfigTests` · S192 `PathValidationTests` · S193 `ObservabilityTests` · S194 `VersionValidationTests` · S195 `FormatFamilyTests` · **S196 winget+Scoop manifests v38.8** · **S197 `ThumbnailBenchmarks` (Google Benchmark stubs)** · S198 `MemoryBudgetTests`

### Session 7 (S201–S209) — Catch2 Wave 3 + v39.0.0

S201 `IStreamingDecoderTests` · S202 `CacheTierTests` · S203 `DecoderPriorityTierTests` · S204 `CLIContractTests` · S205 `PluginSDKContractTests` · S206 `PlatformProfileTests` · S207 `ErrorDomainTests` · S208 `CompressionAlgorithmTests` · **S209 ROADMAP sync + v39.0.0 "Betelgeuse" release**

### Session 8 (S211–S220) — Catch2 Wave 4 + v39.1.0

S211 `FormatDetectorTests` · S212 `ThumbnailDimensionTests` · S213 `WICCodecTableTests` · S214 `SecurityBoundaryTests` · S215 `BitmapAlphaTests` · S216 `MultiPageTests` · S217 `COMInterfaceTests` · S218 `LibraryInventoryTests` · S219 ROADMAP sync (34→42 Catch2 files) · **S220 v39.1.0 "Betelgeuse" release**

### Session 9 (current) — ROADMAP v5.0

This session: **ROADMAP v5.0 "Rigel"** published. v4.0 archived to `docs/archive/ROADMAP_V4.md`.

**Next sprint priorities (S221+):**

| Priority | Task |
|---------|------|
| **P0** | Engine directory consolidation (16→7) |
| **P0** | libjpeg-turbo integration (JPEG P0) |
| **P0** | `LibRaw::unpack_thumb()` embedded preview fast-path |
| **P1** | `lens.exe` all 8 commands |
| **P1** | Catch2 wave 5: 8 more test files |
| **P1** | `NOTICE` file for LGPL libraries |
| **P1** | winget + Scoop public bucket submission |
| **P2** | Remove cross-platform claims from README |
| **P2** | Delete Maven + RubyGems publish workflows |

---

## How to use this roadmap

1. **Phase 1 is the only priority until its exit metric is met** — clean-VM install → SSIM-validated thumbnails for 20+ format families.
2. **Measure by §19 metrics, not by version number or header count.**
3. **This document supersedes ROADMAP v4.0** (archived at `docs/archive/ROADMAP_V4.md`).
4. **Update §20 Decision Log** whenever a significant choice changes.
5. **Re-run the competitor matrix (§3) quarterly.** Landscape shifts fastest in AVIF/JXL/HEIC and GPU decode.
6. **No new feature lands without:** (a) a corpus test, (b) a perf budget entry, (c) a §17 register row, (d) an ADR if decision-altering.
7. **Pre-sprint collision check (MANDATORY):** `grep_search` every new `struct`, `enum class`, `class` name across `Engine/**/*.h` before committing. Zero matches required.

---

*ExplorerLens ROADMAP v5.0 "Rigel" — April 2026. Current release: v39.1.0 "Betelgeuse".*  
*Next revision target: after Phase 1 exit (v40.0.0 "Rigel").*
