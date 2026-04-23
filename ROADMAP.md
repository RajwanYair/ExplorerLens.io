# ExplorerLens — Strategic Roadmap v4.0 "Betelgeuse → Rigel"

**Version:** 4.0 — April 2026
**Current Release:** v38.5.0 "Betelgeuse" (4,978 tests, 0 errors, 0 warnings)
**Supersedes:** ROADMAP v3.0 "Antares" (archived to `docs/archive/ROADMAP_V3.md`), v2.0, V35 "Vega", V34 "Arcturus", V30 "Deneb"
**Scope:** Full re-examination of every decision — architecture, frontend, backend, language, libraries, APIs, database, infrastructure, tests, docs, CI/CD, distribution, AI surface, and competitive positioning. Nothing is assumed correct until re-justified here.

---

## Table of Contents

1. [Executive Summary](#1-executive-summary)
2. [Deep Rethink — Every Decision On The Table](#2-deep-rethink)
3. [Expanded Competitive Landscape](#3-expanded-competitive-landscape)
4. [Harvested Practices — The Best-in-Class Distillation](#4-harvested-practices)
5. [Language, Runtime & Compiler](#5-language-runtime--compiler)
6. [Frontend — Shell, GUI, CLI & Web](#6-frontend)
7. [Backend — Engine, Decode Pipeline, Cache, GPU](#7-backend)
8. [External Libraries, APIs & Data Sources](#8-external-libraries-apis--data-sources)
9. [Build System & Toolchain](#9-build-system--toolchain)
10. [Testing & Quality Strategy](#10-testing--quality-strategy)
11. [Documentation & Configuration Standards](#11-documentation--configuration-standards)
12. [CI/CD, Packaging & Distribution](#12-cicd-packaging--distribution)
13. [AI Tooling Surface & MCP](#13-ai-tooling-surface--mcp)
14. [Database & Persistent Storage](#14-database--persistent-storage)
15. [Infrastructure, Security & Observability](#15-infrastructure-security--observability)
16. [Cross-Platform, AI/ML & Advanced Features](#16-cross-platform-aiml--advanced-features)
17. [Refactor / Rewrite / Delete Register](#17-refactor--rewrite--delete-register)
18. [7-Phase Plan to Best-in-Class](#18-7-phase-plan)
19. [Success Metrics](#19-success-metrics)
20. [Decision Log (v4.0)](#20-decision-log-v40)
21. [Consolidated Legacy — What Survived From V3.0 and Earlier](#21-consolidated-legacy)

---

## 1. Executive Summary

ExplorerLens is a Windows Shell Extension (`IThumbnailProvider` COM in-process DLL) that produces thumbnails for 200+ file extensions through 25+ specialized decoders, linking 18 statically-compiled external libraries. As of v38.3.0 it ships with a professional CMake/MSVC v145 build system, 22 CI/CD workflows, a WiX MSI installer, a configuration GUI (WTL), a CLI (`lens.exe`), and an extensive AI-tooling surface (6 agents, 15 instructions, 14 prompts, 7 skills, 3 MCP servers).

### What the re-examination revealed

**Strong foundations we keep:**
COM architecture, C++20 + MSVC v145, CMake + Ninja, static linking of externals, zero-warnings discipline, security flags (ASLR/DEP/CFG), `/MD` CRT, and the layered Shell / Engine / Manager split.

**Genuine gaps that must be closed:**
No real test corpus (only ~21 synthetic files vs. 100+ needed), no real GPU compute path (only architecture headers), ~1,386 headers against ~269 sources (5.1:1 ratio), MuPDF AGPL license risk, no crash reporting (WER), no `IPropertyStore` metadata provider, no auto-update path, LensServer uses thread-per-connection Winsock2, cross-platform is stubs only, and a custom test framework that blocks parameterization / XML reporting.

**New strategic bets introduced in v4.0:**
1. Adopt **C++23 `std::expected<T,E>`** for new engine APIs (keep `HRESULT` at the COM boundary).
2. Evaluate a **Rust sandbox sidecar** (WASM / out-of-process) for untrusted decoders as an AppContainer alternative.
3. Target **Vulkan Video + D3D12 Video** as the long-term GPU decode path, with WIC + D3D11 hints as the Phase-2 stepping stone.
4. Replace the **custom test harness with Catch2 v3** (already integrated, gated migration continues).
5. Consolidate from **18 Engine subdirectories to 7** — kill premature subdivision.
6. Pick a winning **PDF library**: PDFium (BSD) over MuPDF (AGPL) unless commercial license is obtained.
7. Introduce **ONNX Runtime + DirectML** for optional AI-assisted thumbnails (smart crop, aesthetic score) behind a feature flag.
8. Harvest **13 concrete best-in-class practices** from the expanded competitor matrix (§3–§4).

### North-star metric

> By end of Phase 2: For any file a Windows user can legally store, the ExplorerLens thumbnail is **more correct, faster, smaller in RAM, and lower in install footprint** than Windows built-in, QuickLook, Icaros, SageThumbs, ImageGlass, and XnView MP. Validated by automated corpus-based tests, not by feature tables.

---

## 2. Deep Rethink

Every major category re-examined. Verdict column: **Keep** = decision stands, **Evolve** = change is needed, **Reverse** = prior decision was wrong.

| # | Domain | Previous Decision | Re-examination | Verdict | New Direction |
|---|--------|-------------------|---------------|---------|--------------|
| 1 | Language | C++20 for everything | Only COM boundary actually needs native C++; CLI & server could be Go/Rust | **Keep + Evolve** | C++20 for Engine/Shell; selectively adopt C++23 (`std::expected`, `std::print`); explore Rust sandbox sidecar Phase 4 |
| 2 | Compiler | MSVC v145 only | Still the only toolset with full v145 ABI + COM support; Clang-cl gives cross-compile but not Phase-1 value | **Keep** | MSVC v145 production. Clang-cl nightly CI lane Phase 2 for UB sanitizer only |
| 3 | Build system | CMake 4.3 + Ninja + presets | Industry standard; sccache-ready; handles externals via vcpkg manifest | **Keep** | Enable `vcpkg` manifest mode as primary; retire 13 custom `Build-*.ps1` scripts over Phases 1–3 |
| 4 | Tests framework | Custom `TEST()`/`ASSERT()` macros (~4,744 tests) | Cannot parameterize, no fixtures, no XML, no IDE integration | **Reverse** | Catch2 v3 primary by end of Phase 1; replace ~4,744 shallow tests with ~1,200 deep ones |
| 5 | Test corpus | `data/corpus/` with ~21 synthetic files | Impossible to validate 200+ formats this way | **Reverse** | Source 100+ CC0/public-domain files via scripted ingest; SSIM thresholds in CI |
| 6 | Engine directory layout | 16 subdirectories | Premature subdivision; AI/Enterprise/Media folders carry stubs | **Reverse** | Consolidate to 7: Core, Decoders, GPU, Cache, Platform, Tests, Utils |
| 7 | Headers | 1,386 headers / 269 sources (5.1:1) | Most headers are stubs or declarations without impl | **Reverse** | Implement-before-declare rule; target ≤1.6:1 by Phase 1 exit |
| 8 | Frontend GUI | WTL (Win32, ~400 KB) | Modern UI trend is WinUI 3 but adds 100+ MB | **Keep** | WTL for v1; evaluate WinUI 3 for Manager v2 (Phase 4); never for Shell DLL |
| 9 | CLI | `lens.exe` (partial) | Needed for headless, CI, and `doctor` command | **Evolve** | Finish `generate`, `info`, `register`, `doctor`, `benchmark`, `cache`, `serve` commands in Phase 1 |
| 10 | Web frontend | Static `index.html` moved to `docs/` | GitHub Pages good enough; no SPA needed | **Keep** | Add live thumbnail screenshots; no React/Vue/Next.js |
| 11 | GPU strategy | "DirectX 11/12/Vulkan GPU planned" | No shaders exist; no device creation in hot path | **Evolve** | Honest: Phase 2 = WIC+D3D11 hints + `resize_bilinear.hlsl`. Phase 3 = DXVA2/D3D12 Video. Phase 4 = Vulkan Video parity |
| 12 | REST API | `LensServer` Winsock2 thread-per-connection | Not scalable; hand-rolled HTTP parser is attack surface | **Reverse** | Phase 4 rewrite on `cpp-httplib` or `Boost.Beast` + thread pool; dockerized |
| 13 | Database | "No DB needed" | True for settings; cache index needs one | **Evolve** | SQLite 3 for L2 cache index (WAL mode); registry for settings; no other DB |
| 14 | PDF library | MuPDF 1.24.11 (AGPL-3.0) | AGPL triggers viral copyleft when distributed with a network service | **Reverse** | Migrate to **PDFium** (BSD-3-Clause) in Phase 3; document fallback to commercial MuPDF for enterprise SKU |
| 15 | Crash reporting | None | Silent failures inside `explorer.exe` hurt reputation | **Reverse** | Add WER + `SetUnhandledExceptionFilter` + minidump upload (opt-in) in Phase 4 |
| 16 | Metadata provider | None | Explorer Details view empty for our formats — Icaros shows this works | **Reverse** | Implement `IPropertyStore` for dimensions, codec, camera, duration in Phase 3 |
| 17 | Auto-update | Manual download only | Competitors ship via Store / winget with auto-upgrade | **Evolve** | winget + Scoop + Chocolatey primary; in-app update check (opt-in) Phase 4; MSIX Phase 5 |
| 18 | Package registries | NuGet + npm + Container + Maven + RubyGems | Only NuGet had real C++ SDK consumers | **Reverse** | Keep NuGet. Defer Container to Phase 4 (when `lens-server` ships). Drop Maven + RubyGems. npm only when WASM lands Phase 6 |
| 19 | AI/ML features | `Engine/AI/` with stubs | No inference runtime; speculative | **Evolve** | Fold to `Core/`; add ONNX Runtime + DirectML execution provider **only when** smart-crop ships (Phase 5) |
| 20 | Cross-platform | macOS/Linux PAL stubs | `#ifdef` ≠ support; README overclaims | **Reverse** | Remove all cross-platform claims from README; macOS Quick Look → Phase 5; Linux tumbler → Phase 6 |
| 21 | Plugin system | Header-only SDK | No ABI stability test, no sample plugin | **Evolve** | Ship C ABI `plugin_api.h` + one reference plugin (DICOM or FITS) in Phase 3; sandbox the runtime Phase 4 |
| 22 | Documentation | 130+ markdown files | Docs outpace code | **Reverse** | Right-size to ~60; move aspiration to this roadmap; `mkdocs build --strict` in CI |
| 23 | Dead code | `src/LensServer/`, `src/PluginHost/`, `src/Tools.PSModule/`, `Engine/Tests/FuzzTargets/` | Not wired to CMake | **Done** | Removed in v36.2.0 |
| 24 | Observability | Custom `ObservabilityIntegration` | Good direction, but no OTLP exporter | **Evolve** | Keep ETW + Event Log. Add optional OpenTelemetry OTLP exporter Phase 4 for enterprise SKU |
| 25 | Security model | Binary runs in `explorer.exe` trust | Any decoder crash = Explorer crash | **Evolve** | Phase 4: out-of-process decode host via COM OOP (+AppContainer) for non-trusted formats |
| 26 | Error handling | Mixed HRESULT + exceptions internally | Exceptions across COM = UB | **Evolve** | `std::expected<T,EngineError>` internal; `HRESULT` only at COM boundary; exceptions only in test harness |
| 27 | Internationalization | None | Competitors localize UI (QuickLook, ImageGlass) | **Evolve** | Resource-only strings in LENSManager by Phase 4; English-default Shell DLL has no UI |
| 28 | Accessibility | None | Manager uses raw Win32 dialogs | **Evolve** | UIA automation peers + high-contrast theme compliance by Phase 4 |
| 29 | Licensing | MIT for code | Sound; matches competitive moat | **Keep** | Keep MIT. Publish a separate commercial-only "Enterprise" NuGet for signed-plugin SDK Phase 6 |
| 30 | Telemetry | None | Cannot learn which formats users actually hit | **Evolve** | Opt-in ETW + aggregate telemetry ("which format decoded / how fast / success") Phase 4 |

---

## 3. Expanded Competitive Landscape

Twelve competitors examined across 22 dimensions. Previous roadmap had 9; v4.0 adds **Nomacs**, **Preview.app (macOS)**, **Gnome Tumbler / totem-video-thumbnailer (Linux)**, **libvips thumbnailer**, and **Apache Tika** (headless reference) to capture Linux / CLI / macOS-native / library-thumbnailer patterns previously missing.

### 3.1 Full Competitor Matrix (22 dimensions × 12 competitors)

| Dimension | **ExplorerLens (target)** | QuickLook (QL-Win) | SageThumbs | Icaros | Win built-in | XnView MP | macOS Quick Look | Preview.app | IrfanView | ImageGlass | Nomacs | GNOME Tumbler | libvips CLI | Apache Tika |
|-----------|---------------------------|---------------------|------------|--------|--------------|-----------|-------------------|-------------|-----------|------------|--------|----------------|-------------|-------------|
| **Type** | Shell ext (IThumbnailProvider) | Viewer overlay | Shell ext | Shell ext | WIC handlers | Viewer | OS preview | OS viewer | Viewer | Viewer | Viewer | Thumbnailer daemon | CLI/library | Parser library |
| **Language** | C++20 | C# .NET 8 | C++ (GFL) | C++ (FFmpeg) | C++ | C++/Qt | Obj-C/Swift | Swift | C++ | C# .NET 8 | C++/Qt | C (GObject) | C (+bindings) | Java |
| **GitHub stars** | new | 23.1K | — (SF) | — | — | — | — | — | — | 8.7K | 2.1K | — | 2.8K |
| **Active** | ✅ | ✅ | ❌ 2017 | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| **License** | MIT | GPL-3.0 | GPL-2.0 | GPL-2.0 | Prop. | Freeware | Prop. | Prop. | Freeware | GPL-3.0 | GPL-3.0 | GPL-2.0 | LGPL-2.1 | Apache-2.0 |
| **Image formats** | 200+ | 100+ (plugins) | 162 (GFL) | few | ~30 | 500+ | ~30 + plug | ~40 | 100+ | 80+ | 100+ (Qt) | via GdkPixbuf | 300+ (via libheif/magick) | N/A |
| **AVIF/JXL/HEIC** | ✅ all | partial | ❌ | ❌ | HEIC only | ✅ all | ✅ HEIC | ✅ HEIC | partial | ✅ all | ✅ all | ✅ (new libheif) | ✅ all | metadata only |
| **RAW** | ✅ LibRaw | partial | partial | ❌ | limited | ✅ | Apple RAW | ✅ | plug | limited | ✅ LibRaw | limited | ✅ | metadata |
| **Video thumb** | ✅ MF/FFmpeg Phase 3 | ✅ | ❌ | ✅ **best-in-class** | partial | limited | ✅ | ✅ | ❌ | ❌ | ❌ | ✅ (totem) | ❌ | metadata |
| **Archive thumb** | ✅ | via plugin | ❌ | ❌ | ❌ | limited | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | content extract |
| **Document thumb** | ✅ (PDF/EPUB) | ✅ | ❌ | ❌ | minimal | minimal | ✅ | ✅ | ❌ | ❌ | ❌ | ✅ (evince) | PDF via poppler | ✅ |
| **GPU accel** | Planned D3D11/12 + Vulkan | WPF HW-composite | ❌ | ❌ | WIC+DXGI share | ❌ | Metal | Metal | ❌ | Direct2D | OpenGL | ❌ | SIMD only | ❌ |
| **Explorer integration** | native | separate window | native | native | native | separate | native | separate | separate | separate | separate | native (Nautilus) | N/A | N/A |
| **Plugin ecosystem** | C-ABI SDK Phase 3 | ✅ `.qlplugin` (20+) | XnView plug | ❌ | WIC codec pack | XnView | `.qlgenerator` | limited | DLL plug | ❌ | plug | thumbnailer .desktop | N/A | parser SPI |
| **Preview pane (IPreviewHandler)** | Phase 3 | N/A | ❌ | ❌ | partial | ❌ | integrated | integrated | ❌ | ❌ | ❌ | native | N/A | N/A |
| **Metadata columns (IPropertyStore)** | Phase 3 | ❌ | ❌ | ✅ video | ✅ images | ❌ | via Finder | ✅ | ❌ | ❌ | ❌ | via Nautilus | N/A | ✅ (Tika's job) |
| **Enterprise/GPO** | Phase 4 | ❌ | ❌ | ❌ | ✅ | ❌ | MDM | MDM | ❌ | ❌ | ❌ | per-user only | N/A | N/A |
| **Headless/CLI** | ✅ lens.exe | ❌ | ❌ | ❌ | ❌ | batch | ✅ qlmanage | ❌ | batch | batch | batch | `tumblerctl` | ✅ (core strength) | ✅ (core strength) |
| **REST API** | Phase 4 lens-server | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | D-Bus | via bindings | ✅ tika-server |
| **Install size** | < 5 MB | ~15 MB | ~5 MB | ~30 MB | — | ~80 MB | — | — | ~3 MB | ~15 MB | ~20 MB | ~2 MB | ~5 MB core | ~70 MB (JVM) |
| **Cross-platform** | Win (mac/lin Phase 5–6) | Win | Win | Win | Win | Win/Mac/Lin | Mac | Mac | Win | Win | Win/Mac/Lin | Lin | all | all (JVM) |
| **Open source** | MIT ✅ | GPL-3 | GPL-2 | GPL-2 | ❌ | ❌ | ❌ | ❌ | ❌ | GPL-3 | GPL-3 | GPL-2 | LGPL-2.1 | Apache-2 |

### 3.2 Unique niches (not filled by any single competitor)

1. **Native-shell × modern-formats × MIT** — QuickLook is GPL-3 + separate window; SageThumbs abandoned; Icaros focuses on video. No MIT-licensed modern shell extension exists.
2. **Archive & e-book cover thumbnails via native `IThumbnailProvider`** — CBZ/CBR/EPUB is invisible to Explorer today.
3. **Headless server + CLI + shell ext from one codebase** — Apache Tika is headless-only; QuickLook is UI-only; we span both.
4. **Enterprise-ready (GPO + ETW + SBOM + signed MSI) + plugin-extensible** — no competitor combines this.
5. **GPU compute resize path with HDR tone-mapping** — no Windows competitor does real D3D11 compute for thumbnails.

---

## 4. Harvested Practices — The Best-in-Class Distillation

Thirteen concrete practices extracted from the matrix and mapped to our roadmap phases.

| # | Source | Practice | Our Adoption |
|---|--------|----------|--------------|
| H1 | QuickLook | `.qlplugin` community ecosystem (20+ community plugins, Store + Scoop + nightly) | Ship C-ABI `plugin_api.h` + one reference plugin by Phase 3; winget + Scoop + Choco + Store (MSIX) by Phase 4; nightly CI builds in Phase 1 |
| H2 | SageThumbs | Context-menu preview (right-click → thumbnail) for instant discoverability | Implement `IContextMenu` verb "Preview thumbnail here" in Phase 3 |
| H3 | Icaros | FFmpeg-based video keyframe extraction + `IPropertyStore` for duration/codec | Use Media Foundation first (Phase 3), add `IPropertyStore` image+video metadata to Details view |
| H4 | XnView MP | Streaming "probe then decode" pipeline — header read before full decode | New `IStreamingDecoder` interface with `ProbeHeader()` + `DecodeAtSize()` in Phase 1 |
| H5 | macOS Quick Look | SQLite-indexed thumbnail cache + `FSEvents` invalidation | L2 cache = SQLite (WAL) + `ReadDirectoryChangesW` watcher in Phase 2 |
| H6 | Preview.app | Live preview scrubbing on hover (video / GIF / PDF page-turn) | Phase 4 `IPreviewHandler` with scrub bar (media + PDF only) |
| H7 | IrfanView | Tiny binary (< 3 MB), embedded RAW preview for instant thumbs | Phase 1: extract `LibRaw::unpack_thumb()` (100× faster); keep DLL ≤ 5 MB |
| H8 | ImageGlass | MS Store + WinUI-like dark theme, high release cadence | MSIX package Phase 5; dark theme in LENSManager already underway |
| H9 | Nomacs | Panoramic/RAW/stereo + peer-to-peer sync for photographers | Harvest stereo-pair detection for future Phase 6 3D-photo support only |
| H10 | GNOME Tumbler | Out-of-process thumbnailer daemon per-format via D-Bus | Our Phase 4 out-of-process decode host mirrors this isolation model (COM OOP + AppContainer) |
| H11 | libvips | SIMD + streaming I/O for constant memory irrespective of image size | Adopt streaming decode + SIMD intrinsics for TIFF/PSD via libvips-style patterns in Phase 2 |
| H12 | Apache Tika | Format detection via magic-byte registry separate from decoders | Already aligned; formalize `FormatDetector` as pure library with no decoder deps |
| H13 | Windows built-in | `IPropertyStore` schema coverage (~100 properties) and WIC COM composition | Publish our property schema to the Windows Property System in Phase 3 |

### 4.1 Anti-patterns deliberately rejected

| Competitor choice | Why we reject it |
|-------------------|------------------|
| QuickLook's separate-window UX | We are native `IThumbnailProvider` — Explorer-integrated is our whole pitch |
| ImageGlass's .NET runtime | 15 MB baseline; we stay ≤ 5 MB and work inside `explorer.exe` |
| Icaros's FFmpeg-heavy 30 MB install | We lazy-load FFmpeg-like dependencies behind a plugin boundary |
| XnView MP's 500+ formats via thin wrappers | Every format we claim must have a corpus-validated real-file decode. Quality > count |
| Apache Tika's JVM runtime | Never. The Shell DLL must stay AOT-native |
| SageThumbs' abandonware track | Pin a release cadence (semver + monthly minor) in Phase 3 |

---

## 5. Language, Runtime & Compiler

### 5.1 Decision matrix

| Component | Language | Compiler | Runtime | Rationale |
|-----------|----------|----------|---------|-----------|
| `LENSShell.dll` (COM) | **C++20 → C++23** | MSVC v145 | `/MD` (UCRT) | COM ABI stability, static linking, `explorer.exe` constraints |
| `ExplorerLensEngine.lib` | C++20 → C++23 | MSVC v145 | `/MD` | Core library; selective C++23 for `std::expected`, `std::print` when stable |
| `LENSManager.exe` | C++20 + WTL | MSVC v145 | `/MD` | Native, ~400 KB, no .NET dependency |
| `lens.exe` (CLI) | C++20 | MSVC v145 | `/MD` | Same engine linkage; no Python/Go for dependency hygiene |
| `lens-server` (REST, Phase 4) | C++20 + cpp-httplib | MSVC v145 | `/MD` | Same binary family; containerized |
| Build scripts | PowerShell 7 | — | pwsh | Consolidated in `build-scripts/core/Build-Library-Core.ps1` |
| Optional sandbox sidecar (Phase 4 eval) | **Rust 1.80+ (stable)** | rustc | — | Memory-safe untrusted-decoder host; speaks COM via `windows-rs` crate |
| AI / ML (Phase 5) | C++20 + ONNX Runtime C API | MSVC v145 | `/MD` | DirectML execution provider for hardware-agnostic NPU/GPU inference |
| Tests | C++20 + Catch2 v3 | MSVC v145 | `/MD` | Industry standard; replaces custom macros |

### 5.2 C++23 adoption plan

| Feature | When | Guard |
|---------|------|-------|
| `std::expected<T,E>` | Phase 1 for new Engine APIs | `#if __cpp_lib_expected >= 202202L` |
| `std::print` / `std::println` | Phase 2 (when MSVC ships `<print>`) | `#if __cpp_lib_print` |
| `std::flat_map` / `std::flat_set` | Phase 2 | `#if __cpp_lib_flat_map` |
| Deducing `this` (explicit object params) | Phase 2 | Always in MSVC v145 |
| Modules (`import std;`) | Phase 5 | Experimental; evaluate after MSVC modules are production-stable |

### 5.3 What we explicitly say no to

| Alternative | Why rejected |
|-------------|--------------|
| Rust for the Engine core | COM interop pain; contributors lose; no ABI guarantee for DLL exports |
| C# / .NET | 15+ MB runtime; GC pauses inside `explorer.exe`; QuickLook proves it's viable but fatter than we want |
| Go | No stable COM story; runtime GC; 2–4 MB per binary overhead |
| Zig | Young toolchain; no MSVC v145 ABI guarantee yet |
| C++ modules today | MSVC modules still too green for shipping libraries |
| CUDA / ROCm | Vendor lock-in; D3D12 Video + DirectML covers GPU-vendor-agnostic path |

---

## 6. Frontend

Three UI surfaces + one web page. Each owns a narrow role.

### 6.1 `LENSShell.dll` — the core frontend (COM)

| Surface | Current | Phase target |
|---------|---------|--------------|
| `IThumbnailProvider` | ✅ Implemented | Validate at 16×16 → 1024×1024 (Extra Large Icons) |
| `IInitializeWithStream` | ✅ | Prefer streams over file handles to enable OneDrive placeholders |
| `IExtractImage2` | Planned | Phase 2 — legacy Explorer compatibility |
| `IPreviewHandler` | Planned | Phase 3 — Preview Pane |
| `IPropertyStore` | Planned | Phase 3 — Details view columns (dimensions, codec, camera, duration) — [H3, H13] |
| `IContextMenu` | Planned | Phase 3 — right-click "Preview thumbnail here" — [H2] |
| `IFilter` | Optional | Phase 4 — Windows Search indexing of metadata |
| Threading | STA | Audit MTA re-entrancy in Phase 1 (explorer's thread pool) |

### 6.2 `LENSManager.exe` — configuration GUI

- **Framework:** WTL (keep). WinUI 3 considered and rejected for Phase 1–3 (100 MB dep).
- **Phase 4 v2 GUI** (evaluation gate): move to **WinUI 3** only if the cost is justified by: localization, accessibility (UIA), high-contrast theme, dark mode parity with Windows 11. Otherwise keep WTL.
- **Accessibility:** UIA automation peers + high-contrast + keyboard nav by Phase 4.
- **Localization:** Win32 resource strings + MUI fallback by Phase 4. English default only.
- **System tray:** Balloon notifications on decode error surges.
- **Dashboard:** Performance counters, cache hit rate, per-decoder latency P50/P95/P99 (read from shared-memory counters populated by Engine).

### 6.3 `lens.exe` — CLI

| Command | Purpose | Phase |
|---------|---------|-------|
| `lens generate <file> [-o out.png] [-s 256]` | End-to-end decoder validation | 1 |
| `lens info <file>` | Format detection + metadata dump | 1 |
| `lens register [--per-user]` | Shell-ext registration | 1 |
| `lens doctor` | System diagnostics (GPU, libs, registration, cache) | 1 |
| `lens benchmark <dir>` | Batch with P50/P95/P99 JSON | 2 |
| `lens cache [stats|purge|compact]` | Cache management | 2 |
| `lens serve --port 8080` | Launch `lens-server` in-process | 4 |
| `lens plugin [list|enable|disable|trust]` | Plugin management | 3 |

### 6.4 Web page (`docs/index.html`)

- **Stack decision:** Static HTML + minimal CSS. No React / Vue / Next.js. No bundler.
- **Reason:** Marketing page; GitHub Pages; zero infrastructure cost; fast page-load; no dependency maintenance.
- **Content:** Live screenshots of real Explorer windows, formats matrix SVG, download CTA, badge wall.

---

## 7. Backend

### 7.1 Architecture (keep — correct)

```
Windows Explorer
    │
    ▼
LENSShell.dll (COM IThumbnailProvider, IPropertyStore, IContextMenu, IPreviewHandler)
    │
    ▼
ExplorerLensEngine.lib
    ├── FormatDetector (magic-byte registry — pure, no decoder deps) [H12]
    ├── DecoderRegistry (format → decoder routing)
    ├── DecodePipeline (probe → route → decode-at-size → transform → output) [H4]
    ├── CacheProvider (L1 LRU + L2 SQLite/mmap) [H5]
    ├── GPURenderer (WIC+D3D11 Phase 2 → D3D12 Video Phase 3 → Vulkan Video Phase 4)
    ├── ObservabilityIntegration (ETW + Event Log + opt-in OTLP)
    └── PluginHost (C-ABI, in-process Phase 3 → OOP AppContainer Phase 4) [H10]
    │
    ▼
External libraries (statically linked, /MD, 18 libs)
```

### 7.2 Engine directory consolidation (16 → 7)

```
Engine/
├── Core/        ← detection, routing, pipeline, observability, enterprise, plugin host
├── Decoders/    ← all format decoders + video frame extraction (absorbs Media/Codec)
├── GPU/         ← D3D11 compute, DXVA2/D3D12 Video, Vulkan Video Phase 4+
├── Cache/       ← L1+L2 (absorbs Memory/)
├── Platform/    ← Win32 (macOS Phase 5, Linux Phase 6)
├── Tests/       ← Catch2 + benchmarks + corpus runner + fuzz harnesses
└── Utils/       ← release gates, installer lifecycle, shared helpers
```

Folds: `AI/`, `Enterprise/`, `Pipeline/`, `Plugin/`, `PluginHost/`, `CLI/` → `Core/`. `Codec/`, `Media/` → `Decoders/`. `Memory/` → `Cache/`.

### 7.3 Decoder priority tier table (Phase 1 must-work)

| Tier | Formats | Library | P50 budget | Validation |
|------|---------|---------|-----------|-------------|
| **P0** | JPEG, PNG, WebP, AVIF, HEIC, JXL, PDF, RAW (LibRaw) | libjpeg-turbo, WIC, libwebp, libavif+dav1d, libheif+libde265, libjxl, MuPDF→PDFium, LibRaw | 5–25 ms | ≥3 real files × 3 sizes |
| **P1** | ZIP/CBZ, RAR/CBR, 7Z/CB7, EPUB, GIF, BMP, TIFF | minizip-ng, UnRAR, LZMA SDK, libarchive, WIC, libtiff | 5–20 ms | ≥3 files each |
| **P2** | EXR, PSD, DDS, SVG, TTF/OTF, HDR, QOI, TGA | tinyexr, custom PSD, DirectXTex, WIC+Direct2D, FreeType, stb_image | 5–15 ms | ≥2 files each |
| **P3 (Phase 3)** | MP4/MKV/WebM keyframe, glTF, STL, OBJ, DICOM, FITS, NeRF (plugin) | Media Foundation, tinygltf, Assimp, optional plugins | 10–30 ms | ≥2 files each |

### 7.4 `IStreamingDecoder` — harvested from XnView MP [H4]

```cpp
struct DecodeResult {
    HRESULT hr;
    std::expected<Bitmap, EngineError> bitmap; // C++23
    PartialDecodeState state;  // COMPLETE / PARTIAL / HEADER_ONLY / FAILED
    std::optional<Metadata> meta;
    std::chrono::microseconds elapsed;
};

struct IStreamingDecoder {
    virtual DecodeResult ProbeHeader(std::span<const uint8_t> first16KB) = 0;
    virtual DecodeResult DecodeAtSize(IStream* stream, uint32_t target,
                                      std::stop_token cancel) = 0;
    virtual bool SupportsPartialDecode() const noexcept = 0;
    virtual bool SupportsEmbeddedPreview() const noexcept { return false; }  // RAW fast path [H7]
};
```

### 7.5 Cache architecture (harvested from macOS Quick Look) [H5]

| Tier | Storage | Tech | Budget | Hit-path P50 |
|------|---------|------|--------|--------------|
| L1 | In-process memory | Robin-Hood hashmap, LRU, XXH3 keys | 64 MB | < 500 μs |
| L2 index | `%LOCALAPPDATA%\ExplorerLens\cache.db` | SQLite 3 (WAL, read-concurrent) | unlimited rows | < 3 ms |
| L2 blob | `%LOCALAPPDATA%\ExplorerLens\Cache\*.thumb` | Memory-mapped, size-budgeted | 1 GB default | < 5 ms |
| Invalidation | Per-folder watcher | `ReadDirectoryChangesW` | — | immediate |
| Cache key | `SHA256(canonical_path ‖ mtime ‖ size ‖ target_w×target_h ‖ decoder_version)` | — | — | — |

---

## 8. External Libraries, APIs & Data Sources

### 8.1 Current library inventory (18 — keep all but MuPDF)

| Library | Version | License | Verdict |
|---------|---------|---------|---------|
| zlib | 1.3.1 | Zlib | ✅ |
| LZ4 | 1.10.0 | BSD | ✅ |
| zstd | 1.5.7 | BSD/GPL dual | ✅ |
| LZMA SDK | 26.00 | Public domain | ✅ |
| minizip-ng | 4.0.10 | Zlib | ✅ |
| UnRAR | 7.2.2 | UnRAR (permissive) | ✅ |
| libwebp | 1.5.0 | BSD | ✅ |
| libavif | 1.3.0 | BSD | ✅ |
| libjxl | 0.11.1 | BSD | ✅ |
| libheif | 1.19.5 | LGPL-3 | ✅ (dynamic-linking clause satisfied; we static-link only with LGPL exception audit) |
| libde265 | 1.0.15 | LGPL-3 | ✅ (same) |
| dav1d | 1.5.1 | BSD | ✅ |
| LibRaw | 0.21.3 | LGPL-2.1 / CDDL | ✅ |
| **MuPDF** | **1.24.11** | **AGPL-3.0** | **⚠️ Replace — see §8.2** |
| openjpeg | 2.5.3 | BSD | ✅ |
| bzip2 | 1.0.8 | bzip2-1.0.6 | ✅ |
| xz/liblzma | 5.6.3 | Public domain | ✅ |
| libarchive | 3.7.6 | BSD | ✅ |

### 8.2 MuPDF → PDFium migration (Phase 3)

AGPL-3.0 forces copyleft for any network-served derivative (our `lens-server` Phase 4). Three options:

| Option | Cost | Verdict |
|--------|------|---------|
| Commercial MuPDF license | ~$5K/yr per developer | Keep as **Enterprise SKU** fallback |
| Migrate to **PDFium** (BSD-3) | ~2–3 dev-weeks | **Primary path** |
| Poppler (GPL-2 with exception) | Viral w/ network service | Rejected |

Plan: feature-flag switch in Phase 3, PDFium default in Phase 4. No user-visible regression.

### 8.3 New libraries to add

| Library | License | Purpose | Phase |
|---------|---------|---------|-------|
| **Catch2 v3** | BSL-1.0 | Test framework | 1 |
| **libjpeg-turbo** | BSD/IJG | 2–4× JPEG speedup vs. WIC | 1 |
| **Google Benchmark** | Apache-2.0 | Perf regression gates | 1 |
| **DirectXTex** | MIT | DDS/BC textures | 2 |
| **stb_image** | Public domain | QOI/PNM/TGA/BMP fallback | 2 |
| **PDFium** | BSD-3 | PDF (replaces MuPDF) | 3 |
| **FreeType 2** | FTL/GPL-2 dual | Font sample rendering | 3 |
| **cpp-httplib** | MIT | REST server (replaces Winsock2) | 4 |
| **ONNX Runtime + DirectML EP** | MIT + MIT | AI inference | 5 |
| **Assimp** | BSD-3 | 3D model viewer (glTF+OBJ+STL+FBX) | 3 |
| **tinyexr** | BSD-3 | OpenEXR | 2 |

### 8.4 Libraries to remove from claims (no integration today)

NVJPEG / CUDA, Intel oneVPL, AMD AMF, nghttp2, OpenCASCADE, IfcOpenShell — strip references from code and docs. If they land later, it's via the plugin SDK, not the core DLL.

### 8.5 External APIs consumed

| API | Purpose | Phase |
|-----|---------|-------|
| Windows Imaging Component (WIC) | Universal image host | already |
| Media Foundation | Video keyframe extraction | 3 |
| Windows Property System | `IPropertyStore` schema | 3 |
| Cloud Files API (CF) | OneDrive / cloud placeholder hydration detection | 4 |
| Windows Error Reporting (WER) | Crash dump upload | 4 |
| AppContainer / AppInstaller | Sandboxing + MSIX | 4–5 |
| DirectML | GPU-agnostic AI inference | 5 |
| GitHub Releases API | In-app update check | 4 |
| winget / Scoop / Choco bucket APIs | Distribution | 3+ |

### 8.6 Data sources

| Source | Purpose | How we pull |
|--------|---------|-------------|
| AV1 Image File Format test suite | AVIF corpus | Scripted download, CC0-validated |
| JPEG XL reference samples | JXL corpus | Official libjxl test files |
| HEIF demo repo | HEIC corpus | Apple + GPAC sample files |
| RAW camera samples (rawsamples.ch) | RAW corpus | CC0/CC-BY, attribution file |
| Project Gutenberg | EPUB corpus | Public domain |
| sample-videos.com / Blender Open Movies | Video corpus | CC0/CC-BY |
| Google Fonts | TTF/OTF corpus | SIL OFL |
| Wikimedia Commons | General images | CC0/CC-BY |

Ingest script: `build-scripts/corpus/Fetch-Corpus.ps1` (Phase 1) — fetches, verifies SHA256, updates `MANIFEST.json`.

---

## 9. Build System & Toolchain

### 9.1 Toolchain (keep — all current)

| Tool | Version | Verdict |
|------|---------|---------|
| CMake | 4.3.1 | ✅ |
| Ninja | 1.13.2 | ✅ |
| MSVC v145 (cl 19.50) | VS 18 2026 BuildTools | ✅ |
| vcpkg | 2026-02-21 | ✅ |
| Windows SDK | 10.0.26100.0 | ✅ |
| WiX | 6.0.2 | ✅ |
| sccache | 0.11.x | ✅ (integrated v36) |
| PowerShell | 7.5 | ✅ |

### 9.2 Build-system improvements

| Improvement | Priority | Impact |
|-------------|----------|--------|
| PCH for Windows / STL / COM | P0 | 30–50% rebuild time |
| vcpkg manifest primary → retire 13 `Build-*.ps1` | P1 | Simpler onboarding |
| Compile-time profiling `/d1reportTime` | P1 | Identify slow headers |
| Unity builds | ✅ already | CI option |
| sccache | ✅ already | Dev iteration |
| C++ modules `import std;` | P3 | Only when MSVC modules are production-stable |
| Cross-platform via CMake presets | P3 | Phase 5 macOS |

### 9.3 vcpkg manifest (enhanced)

```json
{
  "name": "explorerlens",
  "version": "38.3.0",
  "dependencies": [
    "zlib", "lz4", "zstd", "liblzma", "minizip-ng", "bzip2", "libarchive",
    "libwebp", "libjxl", "libavif", "libheif", "libraw",
    "pdfium", "openjpeg",
    "dav1d", "libde265", "libjpeg-turbo",
    "catch2", "benchmark",
    "directxtex", "freetype", "assimp", "tinyexr",
    "cpp-httplib"
  ],
  "features": {
    "ai": { "description": "Smart crop + aesthetic scoring", "dependencies": ["onnxruntime"] },
    "server": { "description": "REST API", "dependencies": ["cpp-httplib"] }
  }
}
```

### 9.4 Container build (Dockerfile)

- Upgrade Dockerfile base to VS 2026 BuildTools when image ships on `mcr.microsoft.com`.
- Two-stage: `buildtools` + `runtime`.
- Produces `lens-server` image for Phase 4 ghcr.io release.

---

## 10. Testing & Quality Strategy

### 10.1 Current state

| Metric | Value | Issue |
|--------|-------|-------|
| Tests | ~4,744 | Many test only default-construction |
| Framework | Custom macros | No fixtures / parameterization / XML |
| Corpus | ~21 files | **Blocking** |
| GPU tests | 0 | No GPU path exercised |
| Fuzz tests | 0 active | `FuzzTargets/` removed — not yet replaced |

### 10.2 Target stack

| Layer | Framework | Count target |
|-------|-----------|--------------|
| Unit | Catch2 v3 | 500 meaningful |
| Decoder validation | Catch2 + corpus runner | ~600 (20 formats × 3 files × 3 sizes × 3 assertions) |
| Integration | Catch2 + COM harness | 50 |
| GPU SSIM tests | Catch2 + D3D11 | 20 |
| Benchmarks | Google Benchmark | 30 |
| Fuzz | libFuzzer / WinAFL | 20 targets, continuous |
| Property-based | Catch2 `GENERATE` | 50 |

**Total target: ~1,270 deep tests replacing ~4,744 shallow ones.**

### 10.3 Test corpus plan (§7.3 references)

```
data/corpus/
├── images/ {jpeg,png,webp,avif,heic,jxl,gif,bmp,tiff,ico,qoi,tga,exr,hdr,psd}/
├── archives/ {zip,rar,7z,tar,gz,bz2,xz,cbz,cbr,cb7}/
├── documents/ {pdf,epub,mobi,fb2}/
├── fonts/ {ttf,otf,woff,woff2}/
├── models/ {gltf,glb,obj,stl,fbx}/
├── raw/ {cr2,cr3,nef,arw,dng,raf,orf,rw2,pef,x3f}/
├── video/ {mp4,mkv,webm,avi}/
└── MANIFEST.json (SHA256, expected SSIM, source attribution, license)
```

Sourcing via `build-scripts/corpus/Fetch-Corpus.ps1`. Every file CC0 or explicitly licensed for redistribution.

### 10.4 Catch2 migration plan (see ADR-010)

1. ✅ Catch2 added via `FetchContent` v36.0.0
2. All new tests in Catch2 format
3. `LEGACY_TEST()` bridge macro for gradual migration
4. Migrate tests that exercise behavior; delete mechanical stubs
5. Retire `EngineTestsMacros.h` when zero callers remain
6. Target: Catch2 primary by Phase 1 exit

### 10.5 Quality gates in CI

| Gate | Threshold | Enforced in |
|------|-----------|-------------|
| Zero errors / zero warnings | — | `build.yml` |
| Test pass rate | 100% | `ci-matrix.yml` |
| Coverage (Engine/Core) | ≥ 80% | `coverage.yml` |
| Performance P95 regression | < 10% | `performance-regression-gate.yml` |
| Binary size growth | < 10% | `binary-size.yml` (new Phase 1) |
| SSIM per-format baseline | ≥ 0.98 | `corpus-validation.yml` |
| ASAN leaks | 0 | new Phase 4 `sanitizer-ci.yml` |

---

## 11. Documentation & Configuration Standards

### 11.1 Right-size to ~60 files

| Tier | Files | Rule |
|------|-------|------|
| T1 user | README, USER_GUIDE, CHANGELOG, LICENSE | reflect **only** working features |
| T2 developer | `docs/development/`, `CONTRIBUTING`, `standards/` | accurate build & test instructions |
| T3 architecture | ROADMAP (this), `docs/architecture/`, ADRs | vision + current state clearly labeled |
| T4 historical | `CHANGELOG-archive`, `docs/archive/` | ROADMAP_V3.md lives here now |

### 11.2 Documentation actions

- `mkdocs build --strict` as a CI check.
- All diagrams as **SVG with dark palette** (13 target, see v3.0 §8.7.3 — 13 delivered).
- ADRs gated in `docs/adr/` with template; one ADR per non-trivial decision in this roadmap.
- All doc files `UPPER_SNAKE_CASE.md` except `README.md` / tool configs.
- Dev container (`.devcontainer/`) must produce passing build in < 15 min from clone.

### 11.3 Standards modernization (carry from v3.0 §8.7)

Status fully tracked in v3.0 checklist; items still open:
- Create `.markdownlint.json`
- VS Code settings schema validation
- Pin all GitHub Actions to commit SHA
- Dev container full clone-to-build test

---

## 12. CI/CD, Packaging & Distribution

### 12.1 Workflows (22 — full audit complete in v38.2)

| Area | Workflow | Status |
|------|----------|--------|
| Build | `build.yml`, `ci-matrix.yml`, `reusable-build.yml` | ✅ |
| Quality | `code-quality.yml`, `coverage.yml`, `docs-validation.yml` | ✅ |
| Perf | `performance-regression-gate.yml` | ✅ |
| Security | `codeql.yml`, `dependency-review.yml`, `sbom.yml` | ✅ |
| Corpus | `corpus-validation.yml` | ✅ (fixed S38) |
| Release | `release.yml`, `publish-packages.yml` | ✅ |
| Pages | `pages.yml` | ✅ |
| **New Phase 1** | `binary-size.yml`, `nightly.yml` | planned |
| **New Phase 4** | `sanitizer-ci.yml` (ASAN/UBSAN), `fuzz-ci.yml` (libFuzzer) | planned |

### 12.2 Package registries (simplify — §2 R5)

| Registry | Current | New |
|----------|---------|-----|
| GitHub Releases | ✅ | Keep (primary) |
| NuGet | ✅ | Keep (SDK) |
| winget | in-progress | **Submit Phase 1** |
| Scoop | manifest exists | **Submit Phase 1** |
| Chocolatey | packaging exists | **Submit Phase 2** |
| Microsoft Store (MSIX) | planned | Phase 5 |
| ghcr.io container | ✅ | Activate Phase 4 (when `lens-server` ships) |
| npm | ✅ | Keep dormant until WASM Phase 6 |
| Maven | ✅ | **Drop** |
| RubyGems | ✅ | **Drop** |

### 12.3 Release pipeline

`Bump-Version.ps1` (idempotent, 20 files) → tag → `release.yml`:
1. Build MSVC v145
2. Package MSI + portable ZIP
3. Attach SBOM (SPDX + CycloneDX)
4. SHA-256 sums
5. Authenticode-sign (Phase 4)
6. Publish to winget / Scoop / NuGet / ghcr / MS Store

---

## 13. AI Tooling Surface & MCP

Current baseline (v38.3.0): 6 agents, 15 scoped instructions, 14 prompts, 7 skills, 3 MCP servers, 22 workflows.

### 13.1 Remaining open items (carry from v3.0 §8.8.8)

| Item | Priority | Phase |
|------|----------|-------|
| Pin all 22 workflows to action commit SHA | P2 | 1 |
| Evaluate SQLite MCP server (cache debugging) | P2 | 2 |
| Evaluate fetch MCP server (format spec lookup for decoder authoring) | P2 | 2 |
| Verify GitHub PAT scopes include `actions:read` | P1 | 1 |
| Dev container end-to-end test | P2 | 1 |

### 13.2 New additions for v4.0

- `corpus.agent.md` — dedicated corpus ingest / SSIM validation / MANIFEST drift checker (split from TestCorpus).
- `roadmap-guardian` prompt — validate every PR description against a roadmap phase.
- `spec-fetch.prompt.md` — pull format specs (AV1, JXL ISO, HEIF ISO/IEC 23008-12) via fetch MCP.

---

## 14. Database & Persistent Storage

### 14.1 Decision

**SQLite 3 for L2 cache index only. No other database.** Everything else: registry, JSON manifest, or in-memory.

| Data | Store | Tech |
|------|-------|------|
| L1 cache | in-process | Robin-Hood hashmap + LRU |
| L2 cache index | `cache.db` | SQLite 3 WAL, read-concurrent |
| L2 cache blobs | `Cache/*.thumb` | memory-mapped files |
| User settings | Registry | `HKCU\Software\ExplorerLens` |
| Enterprise policy | Registry | `HKLM\Software\Policies\ExplorerLens` (ADMX/ADML Phase 4) |
| Corpus baselines | `data/baselines/*.json` | version-controlled JSON |
| Benchmark history | `data/benchmarks/history.jsonl` | append-only JSONL |
| Plugin trust store | `plugins/trust.db` | SQLite 3 (Phase 3) |
| Telemetry (opt-in) | ETW + local `telemetry.db` | SQLite rolling window |

### 14.2 Rejected alternatives

| Alternative | Why rejected |
|-------------|--------------|
| LevelDB | Process-exclusive lock breaks multi-Explorer scenarios |
| LMDB | Good, but SQLite is more battle-tested and humans can `sqlite3` it for debugging |
| RocksDB | Heavyweight; designed for servers |
| Custom file format | NIH; SQLite is crash-safe, atomic, and free |

---

## 15. Infrastructure, Security & Observability

### 15.1 Security hardening roadmap

| Item | Priority | Status / Phase |
|------|----------|----------------|
| ASLR / DEP / CFG | P0 | ✅ |
| `/GS` stack canaries | P0 | ✅ |
| Input validation (magic, size, dims) before decode | P0 | harden Phase 1 |
| `SafeInt<>` for dimension math | P0 | Phase 1 |
| Authenticode signing | P1 | Phase 4 |
| libFuzzer / WinAFL per decoder | P1 | Phase 4 |
| ASAN + UBSAN CI lane | P1 | Phase 4 |
| Out-of-process decode host (AppContainer) | P1 | Phase 4 — [H10 harvest] |
| Supply-chain: SHA-pinned actions | P2 | Phase 1 |
| SBOM (SPDX + CycloneDX) | ✅ | shipped v37 |
| Private vulnerability reporting | ✅ | `.github/SECURITY.md` |
| Dependabot: pip + actions + docker | ✅ | v37 |
| Code signing certificate | P1 | Phase 4 (EV cert) |
| Rust sandbox sidecar for untrusted decoders | P3 | Phase 4 evaluation |

### 15.2 Observability

| Signal | Mechanism | Consumer |
|--------|-----------|----------|
| Decode latency | ETW (`ExplorerLens-Engine`) | WPA, `lens benchmark` |
| Cache hit/miss | shared-memory perf counters | LENSManager dashboard |
| Errors | Windows Event Log (`ExplorerLens` source) | SIEM, `lens doctor` |
| Crashes | WER + minidump (opt-in) | Phase 4 developer-side aggregator |
| OTLP export (enterprise) | OpenTelemetry exporter | Phase 4 |

### 15.3 Crash reporting

- `SetUnhandledExceptionFilter` in `DllMain` attach (COM-safe).
- MiniDump with `MiniDumpWithDataSegs | WithThreadInfo | WithHandleData`.
- Optional HTTPS upload behind opt-in consent + PII scrub.

### 15.4 Auto-update

- `winget upgrade ExplorerLens.ExplorerLens` primary path.
- LENSManager polls GitHub Releases API daily (opt-in).
- No auto-install: notification only; user runs `winget upgrade`.

---

## 16. Cross-Platform, AI/ML & Advanced Features

### 16.1 Cross-platform honest timeline

| Platform | Phase | Mechanism |
|----------|-------|-----------|
| Windows 10/11 | 1–4 | COM IThumbnailProvider (core product) |
| macOS | 5 | `QLThumbnailProvider`, Metal backend, Homebrew |
| Linux | 6 | GNOME Tumbler + KDE KIO thumbnailer, Vulkan, Flatpak/AppImage |
| Web/WASM | 6+ | Emscripten module + `lens-server` REST |

### 16.2 GPU pipeline

| Phase | What | How | Target |
|-------|------|-----|--------|
| 2 | WIC + D3D11 hints | `IWICImagingFactory2` with D3D device | 1.5–2× JPEG/PNG |
| 2 | D3D11 compute resize | `resize_bilinear.hlsl` | < 0.5 ms 4K→256 |
| 3 | DXVA2 video decode | Hardware keyframe extraction | 10× video thumbs |
| 3 | HDR tone-mapping | `tonemap_pq_to_srgb.hlsl` | < 0.5 ms HDR→SDR |
| 4 | D3D12 Video | Unified with compute queue | GPU-vendor-agnostic |
| 5 | Vulkan Video | macOS/Linux parity | cross-platform |

### 16.3 AI/ML — gated behind `[ai]` feature flag

| Feature | Phase | Runtime | Size impact |
|---------|-------|---------|-------------|
| Smart crop (saliency) | 5 | ONNX Runtime + DirectML | +6 MB (quantized MobileSaliency) |
| Aesthetic score (sort thumbs) | 5 | same | +8 MB (NIMA model) |
| Scene understanding / CLIP | 6 | same | +40 MB (distilled CLIP) |
| Generative thumbnails | rejected | — | not core mission |

Philosophy: **AI never runs in the shell DLL hot path.** It runs in `LENSManager` or `lens-server`, produces offline augmented thumbnails, cached.

---

## 17. Refactor / Rewrite / Delete Register

Everything concrete the engineering org must do, grouped by action verb.

### 17.1 Delete

| Target | Reason | Phase |
|--------|--------|-------|
| `Engine/AI/` | Speculative stubs | 1 (fold to Core) |
| `Engine/Enterprise/` | Fold to Core | 1 |
| `Engine/Media/` | Fold to Decoders | 1 |
| `Engine/Memory/` | Fold to Cache | 1 |
| `Engine/Pipeline/` | Fold to Core | 1 |
| `Engine/Plugin/`, `Engine/PluginHost/` | Fold to Core | 1 |
| `Engine/CLI/` | Already moved to `src/Tools.CLI/` | 1 |
| `Engine/Codec/` | Fold to Decoders | 1 |
| Dead headers (6 superseded stubs, GLTFModelDecoderTests.cpp) | ✅ done v36.4 | — |
| Stale Maven + RubyGems publish workflows | ⏳ | 1 |
| Warning suppressions `/wdXXXX` if any remain | ⏳ | 1 |

### 17.2 Refactor

| Target | Action | Phase |
|--------|--------|-------|
| Custom test macros → Catch2 v3 | Gradual migration | 1 |
| Header-heavy stubs → real .cpp | Implement-before-declare rule | 1 |
| `LensServer` Winsock2 → cpp-httplib + thread pool | Rewrite | 4 |
| `LENSShell` error returns → `std::expected` internal, HRESULT at boundary | — | 2 |
| `CacheProvider` → SQLite-backed L2 | — | 2 |
| Format detection split from decoder routing (pure `FormatDetector` lib) | — | 2 |
| External library scripts → vcpkg manifest | 13 scripts retired | 1–3 |

### 17.3 Rewrite

| Target | Action | Phase |
|--------|--------|-------|
| `LensServer` | Full rewrite on cpp-httplib | 4 |
| MuPDF integration → PDFium | Swap behind feature flag | 3 |
| GPU renderer shaders | Write HLSL compute shaders | 2–3 |
| Plugin host | Out-of-process AppContainer | 4 |
| CLI | Finish all 8 commands | 1 |

### 17.4 Add

| Target | Phase |
|--------|-------|
| `IPropertyStore` metadata provider | 3 |
| `IContextMenu` right-click preview | 3 |
| `IPreviewHandler` preview pane | 3 |
| WER crash reporting | 4 |
| Authenticode code signing | 4 |
| winget / Scoop / Choco manifests | 1–2 |
| libjpeg-turbo for JPEG | 1 |
| Catch2 v3 as primary framework | 1 |
| Google Benchmark | 1 |
| PDFium | 3 |
| ONNX Runtime + DirectML | 5 |
| `binary-size.yml`, `sanitizer-ci.yml`, `fuzz-ci.yml` | 1/4 |
| Rust sandbox sidecar (evaluation) | 4 |

---

## 18. 7-Phase Plan

### Phase 1 — Foundation (current focus)

**Goal:** Working, validated, installable product for top 20 formats with real corpus validation.

Foundation cleanup (most already done in v36–v38):
- ✅ Dead code removed, `EngineTests_Late.cpp` split, stale version refs fixed, mkdocs strict, Docker VS 2026, SVG diagram set, MCP config hygiene, 6 agents, 15 instructions, 14 prompts, 7 skills.

Open in Phase 1 (progress as of v38.5.0 / S161–S169):
- Consolidate Engine subdirectories 16 → 7
- Implement-before-declare sweep; header ratio ≤ 1.6:1
- Source 100+ real corpus files via `Fetch-Corpus.ps1`
- Migrate 500 meaningful tests to Catch2 — 10 Catch2 files, 180+ tests implemented; `InputValidationTests.cpp` + `DecoderRegistryTests.cpp` added (S165–S166)
- libjpeg-turbo integrated; top-20 decoders P50 budget met
- `lens.exe` commands (generate/info/register/doctor) working
- ✅ winget + Scoop manifests updated to v38.5.0 (S161, §12.2)
- ✅ Pin GitHub Actions to SHA — `pin-actions.yml` workflow created (S163, D40); `Pin-Actions.ps1` utility available
- ✅ `StatelessFormatDetector.h` pure-library format detection (D43, S167) — `ExplorerLens::Core` namespace, thread-safe, no decoder deps
- ✅ SQLite + fetch MCP server evaluation documented (S164, `mcp-server-evaluation.md`)
- Dev container clone-to-build test

**Exit:** Clean Windows 10 VM + MSI install → every file in Pictures gets a correct thumbnail.

### Phase 2 — Performance

- L1 + L2 cache (SQLite WAL + mmap)
- `ReadDirectoryChangesW` invalidation
- WIC + D3D11 hints; `resize_bilinear.hlsl` compute
- Google Benchmark in CI with regression gate (>10% P95 blocks PR)
- `lens benchmark` JSON report
- Targets: 5 ms JPEG, 8 ms WebP, 20 ms PDF, < 5 MB DLL, < 30 MB idle

### Phase 3 — Breadth & Integration

- PDFium migration (MuPDF kept for enterprise)
- `IPropertyStore`, `IContextMenu`, `IPreviewHandler`
- Media Foundation video keyframes + DXVA2
- Plugin SDK v1 (C ABI) + reference plugin (DICOM or FITS)
- Formats: +EXR, PSD, HDR, QOI, TGA, ICO, DDS, SVG, TAR, ISO, EPUB, MOBI, MP4, MKV, WebM, glTF, STL, OBJ, TTF, OTF, WOFF
- Chocolatey submission
- Corpus → 300 files

### Phase 4 — Enterprise, Server & Hardening

- `lens-server` rewrite (cpp-httplib + thread pool), ghcr.io container
- WER crash reporting + minidump
- Authenticode signing (EV cert)
- ADMX/ADML GPO templates
- Out-of-process decode host (AppContainer)
- Rust sandbox sidecar evaluation + prototype
- ASAN + UBSAN + libFuzzer CI lanes
- OpenTelemetry OTLP exporter (enterprise SKU)
- MSIX package
- In-app update check

### Phase 5 — Cross-Platform macOS + AI

- macOS `QLThumbnailProvider` + Metal backend
- Homebrew formula
- ONNX Runtime + DirectML; smart crop + aesthetic score (opt-in `[ai]` feature)
- MS Store (MSIX) publish
- i18n + accessibility (UIA)

### Phase 6 — Linux + WebAssembly + Advanced AI

- GNOME Tumbler + KDE KIO thumbnailer
- Vulkan Video backend
- Flatpak / AppImage
- Emscripten WASM build; npm publish
- CLIP semantic search (opt-in)

### Phase 7 — Horizon

- D3D12 Video unified with compute queue
- Predictive pre-generation based on navigation patterns
- 3D-photo / stereoscopic thumbnail support [H9]
- HDR gainmap / Ultra HDR / Apple ProRAW full fidelity
- Post-quantum Authenticode (if shipped by Windows)

---

## 19. Success Metrics

### Phase 1 — Foundation

| Metric | Target |
|--------|--------|
| Format families validated with corpus | ≥ 20 |
| Corpus files | ≥ 100 |
| Catch2 tests passing | ≥ 500 |
| Header:source ratio | ≤ 1.6:1 |
| Engine subdirectories | 7 |
| Zero errors / zero warnings | ✅ |
| Clean-VM MSI install | ✅ |
| winget + Scoop submitted | ✅ |

### Phase 2 — Performance

| Metric | Target |
|--------|--------|
| JPEG 6MP P50 | < 5 ms |
| PNG 4K P50 | < 5 ms |
| WebP P50 | < 8 ms |
| PDF first-page P50 | < 20 ms |
| Cache hit P50 | < 1 ms |
| `LENSShell.dll` size | < 5 MB |
| Idle RSS | < 30 MB |
| Perf regression gate | active |

### Phase 3 — Breadth

| Metric | Target |
|--------|--------|
| Format families validated | ≥ 80 |
| Supported extensions | ≥ 200 (all validated) |
| Plugin SDK | v1 released + reference plugin |
| PDFium migration | complete |
| `IPropertyStore` in Details | top 20 formats |
| Corpus files | ≥ 300 |

### Phase 4 — Enterprise

| Metric | Target |
|--------|--------|
| WER + minidump | ✅ |
| Authenticode signing | all binaries + MSI |
| ASAN/UBSAN CI | green |
| Fuzz targets | ≥ 20 |
| Out-of-process decode host | ✅ |
| GPO ADMX/ADML | shipped |
| `lens-server` container | published to ghcr.io |

### Best-in-Class definition (measurable)

| Dimension | Verdict criterion |
|-----------|-------------------|
| Speed | Faster than Windows built-in for every P0/P1 format (CI-measured) |
| Coverage | More CI-validated format families than any competitor |
| Correctness | SSIM ≥ 0.98 vs. reference; EXIF rotation + color management + HDR correct |
| Reliability | Zero crashes on 10K-file fuzz corpus |
| Size | DLL ≤ 5 MB; full install ≤ 20 MB |
| Memory | < 50 MB under load; < 10 MB idle |
| Install | One command (`winget install ExplorerLens.ExplorerLens`) |
| Extensibility | Plugin SDK v1 published; ≥1 third-party plugin in the wild |
| Cross-platform | Windows + macOS shipping; Linux beta |
| Observable | ETW + Event Log + OTLP exporter + `lens doctor` |
| License hygiene | No copyleft library in default distribution (after PDFium migration) |

---

## 20. Decision Log (v4.0)

All v3.0 decisions (D1–D29) preserved; v4.0 adds:

| # | Decision | Rationale |
|---|----------|-----------|
| **D30** | PDFium replaces MuPDF | AGPL-3.0 blocks network-service use; PDFium is BSD-3 |
| **D31** | `std::expected<T,E>` for new Engine APIs | C++23 error handling; MSVC 19.50 supports it |
| **D32** | Consolidate Engine 16 → 7 subdirectories | Remove premature subdivision |
| **D33** | `IPropertyStore` + `IContextMenu` + `IPreviewHandler` | Harvest from Icaros, SageThumbs, macOS preview [H2, H3, H6] |
| **D34** | Out-of-process decode host (AppContainer) Phase 4 | Harvest from GNOME Tumbler isolation model [H10] |
| **D35** | Evaluate Rust sandbox sidecar Phase 4 | Memory safety for untrusted plugins |
| **D36** | ONNX Runtime + DirectML for optional AI | Vendor-agnostic NPU/GPU inference |
| **D37** | `binary-size.yml` + `sanitizer-ci.yml` + `fuzz-ci.yml` | Hardening gates |
| **D38** | `IStreamingDecoder` with `ProbeHeader`/`DecodeAtSize` | Harvest from XnView MP streaming [H4] |
| **D39** | `LibRaw::unpack_thumb()` fast path for RAW | Harvest from IrfanView [H7]; 100× faster |
| **D40** | SHA-pinned GitHub Actions | Supply-chain security |
| **D41** | Drop Maven + RubyGems publish jobs | No consumers |
| **D42** | SQLite L2 cache with WAL | Crash-safe, concurrent, portable [H5] |
| **D43** | `FormatDetector` as pure library (no decoder deps) | Harvest from Apache Tika layering [H12] |
| **D44** | Static HTML / no-SPA web page | Zero dep, zero runtime cost |
| **D45** | No WinUI 3 in Shell DLL ever | `explorer.exe` trust + size budget |
| **D46** | Corpus-driven `Fetch-Corpus.ps1` ingest | Removes 100+ manual file management |
| **D47** | Nightly CI build channel | Harvest from QuickLook [H1] |
| **D48** | Enterprise SKU for signed-plugin + commercial MuPDF | Revenue path preserved without polluting MIT core |

---

## 21. Consolidated Legacy

### From ROADMAP v3.0 "Antares" (April 2026) → archived at `docs/archive/ROADMAP_V3.md`

**Carried forward into v4.0:**
- 9-competitor matrix (expanded to 12 in §3)
- 29-decision log D1–D29 (preserved + extended to D30–D48)
- Engine 16→7 consolidation plan
- Implement-before-declare rule
- Catch2 migration plan + ADR-010
- Shared tooling architecture (v3.0 §11) — still authoritative
- Standards modernization (v3.0 §8.7) — open items re-listed in §11.3
- AI tooling surface plan (v3.0 §8.8) — open items re-listed in §13.1
- 13-SVG diagram target (all 13 delivered by v38.2)

**Archived (kept as historical reference only):**
- v3.0 §8.7/§8.8 detailed checklists — now superseded by the summary in §11/§13
- ROADMAP_V30 "Deneb" CLIP/HNSW/LevelDB — LevelDB rejected (§14), CLIP deferred to Phase 6
- ROADMAP_V34 "Arcturus" 350+ extension target in Phase 1 — re-scoped to 200 validated
- ROADMAP_V35 "Vega" real-time collab / zero-trust / SDXL-Turbo / post-quantum — deferred to Phase 7+ only if justified

### From original architecture (v1.0+) — still valid

| Decision | Why it survives |
|----------|-----------------|
| Static linking of all externals | COM DLL in `explorer.exe` cannot tolerate loader conflicts |
| `/MD` CRT | Eliminates `/MT` vs `/MD` library conflicts |
| Zero-warnings policy | Compile-time quality gate |
| COM CLSID fixed at `9E6ECB90-5A61-42BD-B851-D3297D9C7F39` | Changing it breaks upgrades |
| LENSTYPE enum for routing | Simple, effective, audit-visible |
| Shell / Engine / Manager three-layer split | Clean layering survives every rethink |
| CMake + Ninja + vcpkg | Remains best-in-class for C++ in 2026 |
| MSVC v145 primary | Only toolset with full Windows 26100 SDK + v145 ABI |

---

## How to use this roadmap

1. **Phase 1 is the priority.** Every other phase waits for foundation green.
2. **Measure progress by §19 metrics,** not by header count or version number.
3. **This document supersedes ROADMAP v3.0 "Antares"** (archived at `docs/archive/ROADMAP_V3.md`).
4. **Update the Decision Log (§20)** whenever a significant choice changes; archive the prior version by major number (v4 → v5).
5. **Re-run the competitor matrix (§3) quarterly.** The landscape shifts fastest in AVIF/JXL/HEIC and GPU decode.
6. **No new feature lands without:** (a) a corpus test, (b) a perf budget entry, (c) a §17 register row, (d) an ADR if decision-altering.

---

*ExplorerLens v4.0 roadmap — April 2026. Next revision target: after Phase 1 exit (v39.x "Rigel").*

---

## Sprint v39.0.0 "Rigel" — Production-Readiness Sprint (20 tasks)

> Adapted from a web-project sprint template to this C++20 Windows Shell Extension.
> Tasks marked **N/A** are either web-only or already satisfied by the repo baseline.
> Tasks marked **DONE** were already in place at sprint start (v38.3.0).

| # | Task | Status | Evidence / Issue |
|---|------|--------|------------------|
| 1 | Inventory & delete non-web code paths | **N/A** (repo is native C++; Engine 16→7 consolidation scheduled separately in §7.2) | ROADMAP §7.2 |
| 2 | Remove Python scripts/steps | **N/A** (scope re-interpretation — Python used only in 2 offline utilities and inline CI snippets; not a build dependency) | `build-scripts/utilities/fix_duplicates.py`, `Engine/Tests/dashboard/generate_report.py` |
| 3 | Architecture documentation | **DONE** | [`ARCHITECTURE.md`](ARCHITECTURE.md), [`docs/architecture/README.md`](docs/architecture/README.md) |
| 4 | Standardize build system | **DONE** | CMake 4.3 + vcpkg manifest + MSBuild; `CMakePresets.json`, `vcpkg.json` |
| 5 | Clean project structure | **Deferred** (Engine 16→7 consolidation, high-risk) | ROADMAP §7.2 |
| 6 | Deduplicate utilities | **Deferred** (part of Engine consolidation) | ROADMAP §7.2 |
| 7 | Warnings-as-errors | **DONE** | `/W4 /WX` everywhere; `.clang-tidy` enforces naming; no `/wdXXXX` allowed |
| 8 | Fix all warnings | **DONE** | 0 errors / 0 warnings baseline per `copilot-instructions.md` rule 1 |
| 9 | Formatting & linting standards | **DONE** | `.clang-format`, `.clang-tidy`, `.editorconfig`, `.markdownlint.json` |
| 10 | GitHub Actions CI | **DONE** | 22 workflows: build, ci-matrix, code-quality, codeql, coverage, pr-checks, … |
| 11 | Release workflow | **DONE** | `.github/workflows/release.yml`, `release-drafter.yml`, `publish-packages.yml` |
| 12 | `.vscode` workspace standards | **DONE** | `settings.json`, `extensions.json`, `tasks.json`, `launch.json`, `c_cpp_properties.json`, `mcp.json`, `profiles/` |
| 13 | `.github` hygiene | **DONE** | CODEOWNERS, CONTRIBUTING, SECURITY, SUPPORT, CoC, PR template, 2 issue templates, FUNDING, 22 workflows |
| 14 | Dependabot | **DONE** | `.github/dependabot.yml` covers npm (wrapper), github-actions, vcpkg (manual-monitored) |
| 15 | Update README | **DONE** (v38.4 — test count 4978, ARCHITECTURE.md + ROADMAP v4 links) | `README.md` |
| 16 | Update CHANGELOG | **DONE** (keep-a-changelog format, 8 versions backfilled, v38.4.0 entry finalized) | `CHANGELOG.md`, `CHANGELOG-archive.md` |
| 17 | Mermaid diagrams | **DONE** | New `ARCHITECTURE.md` adds 5 Mermaid diagrams (system / decode / subsystems / build / CI); 11 SVGs exist in `docs/assets/` |
| 18 | Remove redundant configs | **DONE** | Single source of truth verified |
| 19 | Consolidate docs | **Deferred** (~130 md → ~60, scheduled in ROADMAP §11) | ROADMAP §11 |
| 20 | Footprint reduction + v39.0.0 bump | **Deferred** (requires sprint exit — Phase 1 test consolidation lands first) | Use `Bump-Version.ps1 -Version 39.0.0 -Codename Rigel` |

### Sprint exit criteria

- [ ] All **Deferred** rows have linked GitHub Issues
- [ ] `ctest --parallel 8` passes 100% after Phase 1 test consolidation
- [ ] Engine header:source ratio ≤ 1.6 : 1 after §7.2 consolidation
- [ ] v39.0.0 "Rigel" tag pushed with MSI + portable ZIP + SHA256 + SBOM on Release

### Sprint outputs captured this session

- `ARCHITECTURE.md` (root) — new, with 5 Mermaid diagrams
- `ROADMAP.md` v4.0 "Betelgeuse → Rigel" — published
- `docs/archive/ROADMAP_V3.md` — prior roadmap archived
- `build-scripts/Audit-Repo.ps1` — repo-state audit script
- `build-logs/audit.txt` — Phase-0 audit snapshot

### Sprint outputs captured (S141–S148 session)

- `CHANGELOG.md` — 8 versions (v37.0–v38.3) backfilled from git log; v38.4.0 entry finalized
- `build-scripts/Serve-Docs.ps1` — local doc server (`python -m http.server` wrapper)
- `docs/LOCAL_VERIFICATION.md` — SVG diagram verification checklist
- `.vscode/tasks.json` — "Serve Local Site" VS Code task added
- `.github/workflows/nightly.yml` — nightly build + corpus validation + benchmark check
- `build-scripts/corpus/Fetch-Corpus.ps1` — CC0/public-domain corpus ingest with SHA-256 verification
- `build-scripts/utilities/Pin-Actions.ps1` — action tag-to-SHA hardening utility
- `README.md` — test count updated to 4,978; ARCHITECTURE.md + ROADMAP v4 links added
- `.github/CONTRIBUTING.md` — toolchain table updated to CMake 4.3.1/Ninja 1.13.2/WiX 6.0.2/PS 7.5; corpus guide added

### Sprint outputs captured (S151–S169 session — v38.5.0 → v38.6.0)

**S151** — `.github/agents/corpus.agent.md` — dedicated corpus ingest / SSIM validation agent
**S152** — `.github/prompts/spec-fetch.prompt.md` + `roadmap-guardian.prompt.md` — 2 new prompts
**S153** — `docs/adr/ADR-011-streaming-decoder-contract.md` — IStreamingDecoder contract ADR
**S154** — `docs/adr/ADR-012-cache-architecture.md` + `ADR-013-cross-platform-pal.md` + `docs/adr/README.md` updated
**S155** — `.github/workflows/sanitizer-ci.yml` + `fuzz-ci.yml` — ASAN/UBSAN + libFuzzer workflow stubs
**S156** — `data/benchmarks/history.jsonl` — benchmark history log initialized
**S157** — `.devcontainer/post-create-validate.ps1` — devcontainer build/test validation script
**S158** — `.github/standards/ai-tooling-capabilities.md` — capability reference synced to v38.4 additions
**S159** — `ROADMAP.md` header updated to v38.4.0 / 4,978 tests
**S160** — v38.5.0 Bump-Version + gh release (tag `v38.5.0`)
**S161** — `packaging/winget/ExplorerLens.yaml` + `packaging/scoop/explorerlens.json` → updated to v38.5.0 (§12.2 Phase 1)
**S162** — `data/baselines/PerFormatBaselines.json` → version synced to v38.5.0 (§14.1)
**S163** — `.github/workflows/pin-actions.yml` — SHA-pin automation with PR creation (D40, §13.1)
**S164** — `.github/standards/mcp-server-evaluation.md` — SQLite + fetch MCP evaluation (§13.1 P2)
**S165** — `Engine/Tests/Catch2Tests/InputValidationTests.cpp` — 30+ security/input-validation Catch2 tests (§10.4, §15.1)
**S166** — `Engine/Tests/Catch2Tests/DecoderRegistryTests.cpp` — 17 DecoderRegistryV2 Catch2 tests (registration, priority, thread-safety; §10.4, D43)
**S167** — `Engine/Core/StatelessFormatDetector.h` — pure-library format detection (D43, §7.1, H12 Apache Tika harvest); registered in ENGINE_HEADERS
**S168** — `.github/CONTRIBUTING.md` — Catch2 migration guide, devcontainer notes, 10-file test inventory added (§10.4, §11.2)
**S169** — `ROADMAP.md` Phase 1 tracker + header updated to v38.5.0; ai-tooling-capabilities sync


---
