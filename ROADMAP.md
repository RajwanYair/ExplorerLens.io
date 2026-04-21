# ExplorerLens — Strategic Roadmap v3.0

**Version:** 3.0 — April 2026
**Current Release:** v36.2.0 "Antares"
**Supersedes:** ROADMAP v2.0, ROADMAP_V35 "Vega", ROADMAP_V34 "Arcturus", ROADMAP_V30 "Deneb"
**Purpose:** Full decision rethink — architecture, language, libraries, APIs, infrastructure,
testing, documentation, CI/CD, distribution, and competitive positioning.

---

## Table of Contents

1. [Executive Summary](#1-executive-summary)
2. [Competitive Landscape & Analysis](#2-competitive-landscape--analysis)
3. [Strategic Decision Rethink](#3-strategic-decision-rethink)
4. [Architecture — Current vs. Target](#4-architecture--current-vs-target)
5. [Code Language, Libraries & APIs](#5-code-language-libraries--apis)
6. [Build System & Toolchain](#6-build-system--toolchain)
7. [Testing & Quality Strategy](#7-testing--quality-strategy)
8. [Documentation & Configuration Standards](#8-documentation--configuration-standards)
9. [CI/CD, Packaging & Distribution](#9-cicd-packaging--distribution)
10. [GitHub AI & Automation Surface](#10-github-ai--automation-surface)
11. [Shared Tooling Architecture](#11-shared-tooling-architecture)
12. [Frontend — Shell, GUI & CLI](#12-frontend--shell-gui--cli)
13. [Backend — Engine & Decode Pipeline](#13-backend--engine--decode-pipeline)
14. [GPU, Cross-Platform & Advanced Features](#14-gpu-cross-platform--advanced-features)
15. [Infrastructure & Operations](#15-infrastructure--operations)
16. [Phase Plan — 6 Phases to Best-in-Class](#16-phase-plan)
17. [Success Metrics](#17-success-metrics)
18. [Decision Log](#18-decision-log)
19. [Consolidated Legacy — What We Kept from V30-V35](#19-consolidated-legacy)

---

## 1. Executive Summary

ExplorerLens is a **Windows Shell Extension** (IThumbnailProvider COM DLL) that generates
thumbnails for 200+ file formats using 18 statically linked decoder libraries, with a
professional-grade CMake/Ninja build system, 20 CI/CD workflows, and a WiX MSI installer.

### What We Have (Strengths)

- Correct architecture: COM shell → engine library → external decoders
- Professional build system: CMake 3.25+ / Ninja / MSVC v145 / vcpkg / sccache
- 18 high-quality external libraries (all current, all `/MD`, all statically linked)
- 20 real CI/CD workflows (~3K lines, 29 jobs)
- Zero-warnings build discipline
- Comprehensive AI tooling surface (4 agents, 13 instructions, 11 prompts, 6 skills, 3 MCP servers)
- WiX MSI installer, Chocolatey, Scoop, WinGet manifests
- CLI tool (`lens.exe`) with subcommands
- REST thumbnail server skeleton (`LensServer`)
- Dockerfile for headless container deployment

### What Needs Work (Honest Gaps)

- **~1,386 headers, ~269 sources** (5.1:1 ratio — target < 2:1)
- **~4,744 tests** in a custom framework — many test only construction/defaults, no parameterization
- **No real test corpus** — decoders cannot be validated against real files in CI
- **No GPU shader code in hot path** despite architecture headers for D3D11/D3D12/Vulkan
- **130+ markdown files** — documentation outpaces working code
- **Cross-platform is stubs only** — macOS/Linux `#ifdef` guards, no real implementations
- **LensServer** at thread-per-connection model, hardcoded version string (35.5.0)
- **MuPDF AGPL license** — potential legal concern for commercial distribution
- **Engine/Core/** has ~530 files — needs sub-organization
- **Dead code** in `src/LensServer/`, `src/PluginHost/`, `src/Tools.PSModule/`, `Engine/Tests/FuzzTargets/`

### Strategic Direction

**Phase 1 goal:** Make the top 20 format decoders produce *correct, fast thumbnails*
from *real files*, validated by a *real test corpus*, with a *one-click MSI install*
that works on any clean Windows 10/11 machine. Everything else waits.

**Competitive differentiator:** No existing tool combines native Explorer integration
(IThumbnailProvider), modern format support (AVIF/JXL/HEIC), GPU acceleration, and
open-source MIT license. We fill this gap by executing on substance over vision.

---

## 2. Competitive Landscape & Analysis

### 2.1 Competitor Comparison Table

| Dimension | **ExplorerLens** (target) | **QuickLook (QL-Win)** | **SageThumbs** | **Icaros Shell Ext.** | **Windows Built-in** | **XnView MP** | **macOS Quick Look** | **IrfanView** | **ImageGlass** |
|-----------|--------------------------|------------------------|----------------|-----------------------|----------------------|---------------|----------------------|---------------|----------------|
| **Type** | Shell ext (IThumbnailProvider) | Space-bar preview app | Shell ext (context menu + thumbnails) | Shell ext (video/audio thumbnails + properties) | WIC-based handlers | Standalone viewer + shell | OS-native preview | Viewer + shell assoc | Standalone viewer |
| **Language** | C++20 | C# (.NET 8 WPF) | C++ (GFL library) | C++ (FFmpeg/Libav) | C++ (OS kernel) | C++/Qt | Objective-C (OS) | C++ (custom) | C# (.NET 8) |
| **GitHub Stars** | New project | **23.1K** | N/A (SourceForge) | N/A (Shark007 site) | N/A | N/A | N/A | N/A | **8.7K** |
| **Active** | ✅ Active | ✅ Active (79 contributors) | ❌ Abandoned (2017) | ✅ Active (2024) | ✅ (Microsoft) | ✅ Active | ✅ (Apple) | ✅ Active | ✅ Active |
| **License** | MIT | GPL-3.0 | GPL-2.0 | GPL-2.0 | Proprietary | Freeware (closed) | Proprietary | Freeware (closed) | GPL-3.0 |
| **Image formats** | 200+ (target) | 100+ via plugins | 162 (GFL) | Limited (focus: video) | ~30 (WIC codecs) | 500+ | ~30 + plugins | 100+ | 80+ |
| **Modern (AVIF/JXL/HEIC)** | ✅ All three | Partial (plugins) | ❌ None | ❌ None | HEIC only (codec) | ✅ All three | ✅ HEIC native | Partial (plugins) | ✅ All three |
| **Video thumbnails** | ✅ Media Foundation | ✅ Built-in player | ❌ No | ✅ **Best-in-class** (FFmpeg) | Partial (WMP codecs) | Limited | ✅ AVFoundation | ❌ No | ❌ No |
| **Archive thumbnails** | ✅ ZIP/RAR/7Z/CBZ/CBR | ✅ Via plugins | ❌ Images only | ❌ No | ❌ No | Limited | ❌ No | ❌ No | ❌ No |
| **Document thumbnails** | ✅ PDF/EPUB | ✅ PDF/Office/EPUB | ❌ Images only | ❌ No | Minimal | Minimal | ✅ PDF/Office | ❌ No | ❌ No |
| **RAW photos** | ✅ LibRaw (100+ cameras) | Partial (plugins) | Partial (GFL) | ❌ No | Limited (MS codecs) | ✅ All major RAW | ✅ Apple RAW | ✅ Plugins | Limited |
| **GPU acceleration** | Planned (D3D11 compute) | HW-accelerated WPF | ❌ None | ❌ None | WIC + DXGI sharing | ❌ None | Metal-backed | ❌ None | Direct2D rendering |
| **Explorer integration** | Native thumbnails | **Separate window** | Native thumbnails + context menu | Native thumbnails + properties | Native thumbnails | Separate app | Native Finder | Separate app | Separate app |
| **Plugin system** | SDK planned (C ABI) | ✅ `.qlplugin` (20+ plugins) | XnView plugins | ❌ No | WIC codec packs | XnView plugins | `.qlgenerator` | Plugin DLLs | ❌ No |
| **Context menu preview** | Planned (IContextMenu) | N/A (space bar) | ✅ Right-click thumb | ❌ No | ❌ No | ❌ No | N/A | ❌ No | ❌ No |
| **Preview pane** | Planned (IPreviewHandler) | N/A | ❌ No | ❌ No | Partial | ❌ No | Integrated | ❌ No | ❌ No |
| **Enterprise/GPO** | Planned (ADMX) | ❌ No | ❌ No | ❌ No | ✅ Group Policy | ❌ No | ✅ MDM profiles | ❌ No | ❌ No |
| **Distribution** | MSI + winget + Scoop + Choco | MS Store + installer + Scoop + nightly | SourceForge | Shark007 installer | OS built-in | Website | OS built-in | Website | MS Store + winget |
| **Install size** | < 5 MB target | ~15 MB (managed) | ~5 MB | ~30 MB (FFmpeg) | N/A | ~80 MB | N/A | ~3 MB | ~15 MB |
| **Cross-platform** | Windows (macOS/Linux planned) | Windows only | Windows only | Windows only | Windows only | Win/Mac/Linux | macOS only | Windows only | Windows only |
| **REST API / headless** | ✅ LensServer (planned) | ❌ No | ❌ No | ❌ No | ❌ No | CLI batch mode | ❌ No | CLI batch mode | ❌ No |
| **Open source** | ✅ MIT | ✅ GPL-3.0 | ✅ GPL-2.0 | ✅ GPL-2.0 | ❌ No | ❌ No | ❌ No | ❌ No | ✅ GPL-3.0 |

### 2.2 Key Lessons from Competitors

| Competitor | Best Practice to Harvest | How We Apply It |
|------------|--------------------------|-----------------|
| **QuickLook** | Plugin ecosystem with `.qlplugin` package format enables 20+ community contributions | Prioritize Plugin SDK with C ABI and simple package format; community extends format support |
| **QuickLook** | Microsoft Store + Scoop + nightly builds = massive reach (23K stars) | Target winget + MS Store (MSIX) as primary channels; offer nightly CI builds |
| **QuickLook** | Fluent Design + acrylic/mica integration looks native on Windows 11 | LENSManager v2 should use WinUI 3 or at minimum dark mode with `SetPreferredAppMode()` |
| **QuickLook** | .NET 8 WPF with hardware-accelerated rendering for smooth previews | Our C++20 approach is leaner (< 5 MB vs 15 MB) — leverage this as a selling point |
| **SageThumbs** | Context menu preview is immediately discoverable (right-click → thumbnail) | Add IContextMenu alongside IThumbnailProvider for preview-on-right-click |
| **SageThumbs** | GFL library handles 162 formats through a single dependency | Validate: fewer, higher-quality decoders beats many thin integrations |
| **Icaros** | FFmpeg-based video thumbnails with configurable keyframe offset | Leverage Media Foundation first; add FFmpeg as optional Phase 3 dependency for exotic codecs |
| **Icaros** | IPropertyStore implementation for audio/video metadata in Explorer columns | Add IPropertyStore to LENSShell for file dimensions, duration, codec info in Details view |
| **XnView MP** | 500+ format support via mature decode pipeline with real I/O | Validate every format against real files; "supported" = "produces correct output" |
| **XnView MP** | Cross-platform via Qt abstraction layer | Keep our PAL approach but defer implementation until Windows is excellent |
| **macOS Quick Look** | Deep OS integration (Finder, Spotlight, Time Machine, preview pane) | Pursue IPreviewHandler (preview pane) + IFilter (Windows Search indexing) |
| **macOS Quick Look** | SQLite-indexed thumbnail cache with file watchers for invalidation | Implement L2 cache with SQLite metadata + `ReadDirectoryChangesW` watcher |
| **IrfanView** | Tiny binary (< 3 MB) with 100+ formats; 180M+ downloads through simplicity | Priority: make it work perfectly for common formats before chasing rare ones |
| **IrfanView** | Plugin architecture for format extension without core binary bloat | Design Plugin SDK so exotic formats (DICOM, FITS, NeRF) don't bloat the core DLL |
| **ImageGlass** | Modern UI with .NET 8 + Direct2D; 8.7K GitHub stars; MS Store presence | Study their release cadence and community engagement model |
| **ImageGlass** | Supports theme packs and custom image processing pipelines | Consider user-configurable thumbnail processing (brightness, contrast, crop rules) |

### 2.3 Competitive Gaps We Fill

No existing tool provides ALL of these simultaneously:

1. **Native Explorer thumbnails** (not a separate viewer window) — eliminates SageThumbs (abandoned), beats QuickLook (separate window)
2. **Modern image formats** (AVIF + JXL + HEIC) without codec packs — beats Windows built-in, Icaros, SageThumbs
3. **Archive cover images** (CBZ/CBR/EPUB) as thumbnails in Explorer — unique among native shell extensions
4. **Open source MIT license** — QuickLook is GPL-3.0; SageThumbs/Icaros are GPL-2.0; MIT enables commercial embedding
5. **GPU-accelerated** decode/resize pipeline — no competitor offers real GPU compute for thumbnails
6. **CLI + REST API** for headless/CI use — unique; enables thumbnail generation in pipelines
7. **Enterprise-ready** (GPO, ETW, Event Log, silent MSI install) — only Windows built-in has GPO today
8. **Plugin SDK** with C ABI for third-party format decoders — harvested from QuickLook's model
9. **Cross-platform architecture** (Windows → macOS → Linux) — only XnView MP spans all three, but isn't a shell extension

This is our moat. Execute on items 1-4 first (Phase 1), then 5-9 (Phases 2-4).

---

## 3. Strategic Decision Rethink

### 3.1 Decisions That Were Right — Keep Them

| # | Decision | Why It's Right |
|---|----------|----------------|
| 1 | **C++20 for the engine** | COM interop requires native code; no managed alternative for IThumbnailProvider |
| 2 | **CMake + Ninja build** | Industry standard; presets provide excellent DX; sccache-compatible |
| 3 | **Static linking of all externals** | Shell extensions MUST be self-contained DLLs; runtime dependencies break Explorer |
| 4 | **COM IThumbnailProvider** | Only way to provide native Explorer thumbnails on Windows |
| 5 | **Shell / Engine / Manager separation** | Clean layering: thin COM adapter → engine library → config GUI |
| 6 | **Zero-warnings policy** | Professional-grade; prevents warning rot; compile-time quality gate |
| 7 | **`/MD` CRT for all targets** | Eliminates `/MT` vs `/MD` conflicts with external libraries |
| 8 | **Security flags (ASLR/DEP/CFG)** | Essential for a COM DLL loaded into `explorer.exe` address space |
| 9 | **vcpkg as optional dependency path** | Fallback for CI and air-gapped builds; Dependabot integration |
| 10 | **WiX for MSI installer** | Only professional MSI toolchain; silent install, GPO deploy |

### 3.2 Decisions to Rethink or Reverse

| # | Decision | Problem | New Decision | Rationale |
|---|----------|---------|-------------|-----------|
| R1 | **Header-only architecture** | 5.1:1 header-to-source ratio; most are stubs | **Implement-before-declare:** no header in CMakeLists without `.cpp` with >50 LOC | Headers declare, sources implement. Reduces compile time, enables real coverage |
| R2 | **Custom test framework** | No fixtures, parameterized tests, XML reporting, IDE integration | **Migrate to Catch2 v3** as primary; keep macro bridge for transition | Industry standard; MSVC integration; CI-friendly JUnit output |
| R3 | **Sprint-mechanical development** | Every version adds exactly N headers + 2N tests | **Feature-driven development:** count working decoders, not header files | One correct decoder with test corpus > 50 stub headers |
| R4 | **16 Engine subdirectories** | Premature subdivision for features without implementations | **Consolidate to 7 directories:** Core, Decoders, GPU, Cache, Platform, Tests, Utils | Merge AI, Enterprise, Media, Memory, Pipeline, Plugin, PluginHost, CLI, Codec into parents |
| R5 | **5 package registries** | NuGet/npm/Maven/RubyGems/Container — no Java or Ruby consumers exist | **NuGet only** (SDK); defer Container until lens-server ships; drop Maven/RubyGems | Publish to registries with actual consumers |
| R6 | **Cross-platform stubs** | `#ifdef` guards ≠ cross-platform support | **Honest README:** "Windows native. macOS/Linux planned for Phase 5." | Don't claim what doesn't work |
| R7 | **3 separate roadmaps** | ROADMAP_V30, V34, V35 create confusion | **Single ROADMAP.md** (this file); archive old ones | One source of truth |
| R8 | **130+ markdown files** | More docs than working code | **Right-size:** document what works; move aspirational content here | Docs should lag code, not lead it |
| R9 | **Claiming GPU acceleration** | No shader files in hot path, no D3D device creation in decode pipeline | **WIC-with-D3D11 first** (Phase 2); real compute shaders in Phase 3 | Prove measurable speedup before claiming GPU support |
| R10 | **MuPDF AGPL-3.0 license** | AGPL requires source disclosure for network services; incompatible with MIT | **Evaluate alternatives:** PDFium (BSD), Poppler (GPL), or MuPDF commercial license | MuPDF is excellent but AGPL is risky for MIT-licensed project |
| R11 | **Cloud/WASM/AI/Collaboration headers** | Aspirational stubs with no runtime integration | **Archive to `docs/archive/vision/`; delete from Engine/** | Focus Phase 1-3 on core thumbnail pipeline |
| R12 | **Dead code in src/** | LensServer/PluginHost/PSModule have no CMake refs | **Delete `src/LensServer/`, `src/PluginHost/`, `src/Tools.PSModule/`** | Dead code confuses contributors |
| R13 | **LensServer thread model** | Thread-per-connection with `std::thread::detach()` won't scale | **Replace with thread pool** or async I/O when LensServer ships (Phase 4) | Production servers need bounded concurrency |
| R14 | **No test corpus** | Cannot validate decoders without real files | **Build corpus of 100+ CC0/public-domain files** covering all 20 priority formats | Single most important gap to close |
| R15 | **Dockerfile uses VS 2022** | Mismatch with VS 2026 v145 toolset requirement | **Update Dockerfile to VS 2026 BuildTools** when available on mcr.microsoft.com | CI/container builds must match dev toolchain |
| R16 | **No auto-update mechanism** | Users must manually download new versions | **Implement winget/Scoop upgrade path** + optional in-app update check | QuickLook users get auto-updates via MS Store |
| R17 | **No crash reporting** | Crashes in explorer.exe context are silent | **Add Windows Error Reporting (WER) + optional telemetry** | Critical for production stability monitoring |
| R18 | **No IPropertyStore** | Explorer Details view shows no metadata for our formats | **Implement IPropertyStore** for image dimensions, camera model, codec info | Icaros does this for video; we should for images |
| R19 | **Engine/Tests/EngineTests_Late.cpp at ~486 KB** | Approaching 500 KB mandatory split threshold | **Split immediately** into `_Late.cpp` + `_Platform.cpp` | File size policy compliance |
| R20 | **LIBRARY_INVENTORY.md header says v15.0.0** | Severely out of date | **Add to Bump-Version.ps1 registry** or remove version from header | Version drift creates confusion |

### 3.3 Honest Assessment — What ExplorerLens Is Today

**What it is:**
- A Windows COM shell extension that registers for file types via IThumbnailProvider
- An engine library with 18 external decoder libraries statically linked
- A WTL configuration GUI (`LENSManager.exe`)
- A CLI tool (`lens.exe`) in early development
- A LensServer REST skeleton (Winsock2-based, thread-per-connection)
- A professional build/CI/CD/packaging pipeline
- A comprehensive AI-assisted development surface (agents, skills, prompts, MCP)

**What it isn't yet:**
- GPU-accelerated (no shader code or D3D device creation in hot path)
- Cloud-native or collaboration-ready
- Cross-platform (macOS/Linux stubs only)
- AI-powered (no model files or inference runtime)
- Validated for 200+ formats (needs corpus testing)
- Crash-aware (no WER integration, silent failures in explorer.exe context)
- Auto-updating (no winget manifest, no in-app update check)
- Metadata-enriching (no IPropertyStore — Explorer Details view shows nothing for our formats)

**What needs attention:**
- ~~MuPDF AGPL-3.0 license~~ → Documented in ADR-009; PDFium evaluation planned Phase 3
- ~~`EngineTests_Late.cpp` at ~486 KB~~ → Split into Late (215 KB) + Platform (256 KB)
- ~~`LIBRARY_INVENTORY.md` version header at v15.0.0~~ → Updated to v36.2.0
- ~~Dead code in `src/`~~ → Deleted LensServer, PluginHost, PSModule
- ~~Dockerfile uses VS 2022~~ → Updated to VS 2026
- ~~CI workflow version comments stale~~ → Updated to v36.1.0
- ~~mkdocs.yml 19 broken nav references~~ → Removed in v36.3.0
- ~~Maven/RubyGems CI publish jobs~~ → Dropped in v36.3.0 (no consumers)
- ~~external/CMakeLists.txt old "DarkThumbs" naming + /MT CRT~~ → Fixed in v36.3.0
- Engine/Core/ has ~530 files (consolidation opportunity — see §4.2)

**This is not a failure.** The architecture is sound, the infrastructure is professional,
and the external library stack is excellent. The gap is between vision and validated
implementation. Phase 1 closes that gap.

---

## 4. Architecture — Current vs. Target

### 4.1 System Architecture (Keep — It's Correct)

```
Windows Explorer
    │
    ▼
LENSShell.dll (COM IThumbnailProvider)  ← Thin adapter (~100 KB)
    │
    ▼
ExplorerLensEngine.lib                  ← Core decode pipeline (~3 MB)
    ├── FormatDetector (magic-byte + extension matching)
    ├── DecoderRegistry (format → decoder routing)
    ├── DecodePipeline (detect → route → decode → transform → output)
    ├── CacheProvider (L1 memory LRU + L2 disk)
    └── GPURenderer (D3D11 compute + GDI+ fallback)
    │
    ▼
External libraries (libraw, libjxl, libheif, mupdf, libwebp, ...)
```

### 4.2 Engine Directory Consolidation

**Current (16 subdirectories):**
```
Engine/
├── AI/  Cache/  CLI/  Codec/  Core/  Decoders/  Enterprise/  GPU/
├── Media/  Memory/  Pipeline/  Platform/  Plugin/  PluginHost/  Tests/  Utils/
```

**Target (7 subdirectories):**
```
Engine/
├── Core/           ← Detection, routing, pipeline, types, observability, enterprise
├── Decoders/       ← All format decoders (image, archive, document, 3D, scientific, media)
├── GPU/            ← GPU acceleration (D3D11 compute, DXVA2 video decode)
├── Cache/          ← Caching subsystem (L1 memory + L2 disk + invalidation)
├── Platform/       ← OS abstraction (Win32 today, macOS/Linux later)
├── Tests/          ← Catch2 unit tests, benchmarks, fuzz harnesses, corpus runner
└── Utils/          ← Shared utilities, release gates, installer lifecycle
```

**Merge map:**
- `AI/` → `Core/` (defer ML to Phase 5; tiny footprint)
- `CLI/` → `src/Tools.CLI/` (already lives there; remove Engine/CLI if empty)
- `Codec/` → `Decoders/`
- `Enterprise/` → `Core/`
- `Media/` → `Decoders/` (video frame extraction IS a decoder)
- `Memory/` → `Cache/` (memory management is cache-adjacent)
- `Pipeline/` → `Core/` (pipeline orchestration IS core)
- `Plugin/` + `PluginHost/` → `Core/` (plugin infrastructure is core; < 10 files)

### 4.3 Header-to-Source Rebalancing

| Metric | Current | Phase 1 Target | Phase 3 Target |
|--------|---------|----------------|----------------|
| Header files | 1,386 | ~400 | ~300 |
| Source files | 269 | ~250 | ~280 |
| Ratio | 5.1:1 | ~1.6:1 | ~1.1:1 |

**How to get there:**
1. **Audit:** Classify every header as Real / Stub / Dead
2. **Delete:** Remove dead headers (unreferenced, no implementation)
3. **Merge:** Combine related stubs into cohesive module headers
4. **Implement:** For each kept header, ensure corresponding `.cpp` with real logic
5. **Enforce:** No new header registered in CMakeLists.txt without `.cpp` > 50 LOC

### 4.4 New Architecture Rule: Implement Before Declaring

```cpp
// ❌ BEFORE: Header-only stub (current pattern in many files)
class CloudHydrationMonitor {
    enum class State { UNKNOWN, PARTIAL, FULL };
    State state_ = State::UNKNOWN;
public:
    State GetState() const { return state_; }
    void Probe(const std::wstring& path) { state_ = State::FULL; } // STUB
};

// ✅ AFTER: Real implementation backed by Windows API
// CloudHydrationMonitor.h — declaration only
class CloudHydrationMonitor {
public:
    enum class State { UNKNOWN, PLACEHOLDER, PARTIAL, FULL };
    State Probe(const std::filesystem::path& path);
private:
    bool QueryCloudState(const std::filesystem::path& path, CF_PLACEHOLDER_STATE& out);
};

// CloudHydrationMonitor.cpp — real implementation
#include "CloudHydrationMonitor.h"
#include <cfapi.h>
#pragma comment(lib, "cldapi.lib")
// ... actual CF API calls ...
```

---

## 5. Code Language, Libraries & APIs

### 5.1 Language Decision: Keep C++20

| Option | Verdict | Rationale |
|--------|---------|-----------|
| **C++20 (MSVC v145)** | ✅ **Keep** | COM interop requires native code; C++20 gives concepts, ranges, `std::span`, `std::jthread` |
| C++23 features | ✅ Adopt selectively | `std::expected<T,E>` for error handling (MSVC 19.50 supports it); `std::print` when stable |
| Rust (for engine) | ❌ Rejected | COM interop from Rust is painful; no MSVC ABI guarantee; learning curve for contributors |
| C# (.NET) | ❌ Rejected | QuickLook proves it works but adds 15+ MB runtime; COM interop overhead; GC pauses |

### 5.2 External Library Audit (18 Libraries — All Current)

| Library | Version | Purpose | Status | Action |
|---------|---------|---------|--------|--------|
| zlib | 1.3.1 | ZIP/deflate | ✅ Current | None |
| LZ4 | 1.10.0 | Fast compression | ✅ Current | None |
| zstd | 1.5.7 | Zstandard compression | ✅ Current | None |
| LZMA SDK | 26.00 | 7z archives | ✅ Current | None |
| minizip-ng | 4.0.10 | ZIP archive handling | ✅ Current | None |
| UnRAR | 7.2.2 | RAR extraction | ✅ Current | Delete duplicate `unrar/` dir |
| libwebp | 1.5.0 | WebP images | ✅ Current | Delete `libwebp-1.5.0-original/` |
| libavif | 1.3.0 | AVIF images | ✅ Current | None |
| libjxl | 0.11.1 | JPEG XL images | ✅ Current | None |
| libheif | 1.19.5 | HEIF/HEIC images | ✅ Current | None |
| libde265 | 1.0.15 | HEVC decoding (for libheif) | ✅ Current | None |
| dav1d | 1.5.1 | AV1 decoding (for libavif) | ✅ Current | None |
| LibRaw | 0.21.3 | RAW camera (100+ models) | ✅ Current | None |
| MuPDF | 1.24.11 | PDF rendering | ⚠️ **AGPL-3.0** | Evaluate PDFium (BSD) or obtain commercial license — see R10 |
| openjpeg | 2.5.3 | JPEG 2000 | ✅ Current | None |
| bzip2 | 1.0.8 | BZIP2 compression | ✅ Current | None |
| xz/liblzma | 5.6.3 | XZ compression | ✅ Current | None |
| libarchive | 3.7.6 | Multi-format archive | ✅ Current | None |

### 5.3 Libraries to Add

| Library | Purpose | Priority | License | Rationale |
|---------|---------|----------|---------|-----------|
| **Catch2 v3** | Test framework | P0 | BSL-1.0 | Industry standard; XML output; parameterized tests; IDE integration |
| **libjpeg-turbo** | SIMD JPEG decode | P0 | BSD/IJG | 2-4× faster than WIC for JPEG thumbnails |
| **Google Benchmark** | Perf benchmarks | P1 | Apache-2.0 | CI regression tracking; JSON output |
| **DirectXTex** | DDS/WIC texture loading | P1 | MIT | Microsoft-maintained; DDS block-compressed textures |
| **stb_image** | Fallback for simple formats | P2 | Public domain | BMP, TGA, PNM, QOI single-file decoder |

### 5.4 Libraries NOT Needed (Remove References from Headers)

| Library | Claimed For | Action |
|---------|-------------|--------|
| NVJPEG / CUDA | GPU JPEG decode | No CUDA SDK in build; remove references |
| Intel oneVPL | QSV decode | No oneVPL in vcpkg.json; remove references |
| AMD AMF | GPU video decode | No AMD SDK in build; remove references |
| nghttp2 | REST API server | No HTTP server exists yet; remove references |
| OpenCASCADE | STEP/IGES CAD | Heavy dependency (200+ MB); defer to plugin |
| IfcOpenShell | IFC/BIM | Heavy dependency; defer to plugin |

### 5.5 API Strategy

| API Surface | Current | Target |
|-------------|---------|--------|
| **COM (IThumbnailProvider)** | Implemented | Add IExtractImage2 (legacy Explorer), IPreviewHandler (preview pane), **IPropertyStore** (metadata columns) |
| **CLI (`lens.exe`)** | 17 source files, early | Make `generate`, `info`, `register`, `doctor`, `benchmark`, `cache` commands work |
| **REST (`lens-server`)** | Winsock2 skeleton | Defer to Phase 4; replace Winsock2 with cpp-httplib when ready |
| **Plugin SDK** | Header stubs | Implement C ABI `plugin_api.h` with decode/probe/metadata functions in Phase 3 |
| **Database** | None needed | SQLite for L2 cache metadata index (like macOS Quick Look) |

### 5.6 Error Handling Evolution

| Pattern | Current | Target |
|---------|---------|--------|
| COM layer | HRESULT ✅ | Keep — COM requires HRESULT |
| Engine internal | Mixed (HRESULT + exceptions) | `std::expected<T, EngineError>` (C++23, available in MSVC 19.50) |
| Decoder results | Ad hoc | `DecodeResult` struct: success/error/partial + diagnostic message |

---

## 6. Build System & Toolchain

### 6.1 Toolchain (No Changes Needed)

| Tool | Version | Verdict |
|------|---------|---------|
| CMake | 4.3.1 | ✅ Latest — keep |
| Ninja | 1.13.2 | ✅ Latest — keep |
| MSVC v145 (cl 19.50) | VS 18 2026 | ✅ Latest — keep |
| vcpkg | 2026-02-21 | ✅ Good — keep as optional |
| Windows SDK | 10.0.26100.0 | ✅ Latest — keep |
| WiX 6.0.2 | .NET tool | ✅ Good for MSI — keep |

### 6.2 Build Improvements

| Improvement | Priority | Impact |
|-------------|----------|--------|
| **PCH for Windows/STL/COM headers** | P0 | 30-50% rebuild time reduction |
| **sccache** (already added v36) | ✅ Done | Build caching for developer iteration |
| **Unity builds** (already added v36) | ✅ Done | Optional batch compilation for CI |
| **Compile-time profiling (`/d1reportTime`)** | P1 | Identify slow-to-compile headers |
| **vcpkg manifest as primary** | P1 | Eliminates 13 custom `Build-*.ps1` scripts over time |
| **C++20 modules** | P3 | Experimental; when MSVC modules are production-stable |

### 6.3 External Library Build Strategy

**Current:** 13 separate PowerShell build scripts in `build-scripts/external-libs/`.
**Target:** vcpkg manifest mode as primary; keep local `external/` as fallback.

```json
// vcpkg.json — enhanced manifest
{
  "name": "explorerlens",
  "version": "36.0.0",
  "dependencies": [
    "zlib", "lz4", "zstd", "liblzma", "minizip-ng", "bzip2", "libarchive",
    "libwebp", "libjxl", "libavif", "libheif", "libraw", "mupdf", "openjpeg",
    "dav1d", "libde265", "libjpeg-turbo", "catch2"
  ]
}
```

---

## 7. Testing & Quality Strategy

### 7.1 Current State (Critical Gap: No Test Corpus)

| Metric | Value | Issue |
|--------|-------|-------|
| Test count | ~4,744 | Many test only construction/defaults — not real I/O |
| Framework | Custom TEST()/ASSERT() | No fixtures, parameterization, CI XML output |
| Test files | 5 split files (~50K lines) | Mechanical 2-tests-per-header pattern |
| Test corpus | Empty `data/corpus/` | **Cannot validate decoders without real files** |
| GPU tests | 0 | No GPU path tested |
| Integration tests | ~18 | Minimal |

### 7.2 Target Testing Stack

| Layer | Framework | What It Tests | Target Count |
|-------|-----------|---------------|--------------|
| **Unit tests** | Catch2 v3 | Individual classes, pure functions | ~500 meaningful tests |
| **Decoder validation** | Custom corpus runner | Each decoder produces correct output from real files | ~600 (1 format × 3 files × 3 sizes) |
| **Integration tests** | Catch2 + COM | Full pipeline: shell request → thumbnail output | ~50 |
| **GPU tests** | Catch2 + D3D11 | GPU decode vs CPU decode SSIM comparison | ~20 |
| **Performance benchmarks** | Google Benchmark | P50/P95/P99 regression tracking | ~30 |
| **Fuzz tests** | libFuzzer / WinAFL | Crash/hang resistance on malformed files | Continuous |

**Total target: ~1,200 high-quality tests** replacing ~4,744 low-quality ones.

### 7.3 Test Corpus (HIGHEST PRIORITY)

```
data/corpus/
├── images/
│   ├── jpeg/     (5 files: basic, EXIF rotation, progressive, CMYK, large 6MP)
│   ├── png/      (5 files: 8-bit, 16-bit, alpha, interlaced, animated APNG)
│   ├── webp/     (5 files: lossy, lossless, alpha, animated, large)
│   ├── avif/     (3 files: 8-bit, 10-bit HDR, animated)
│   ├── heic/     (3 files: single, burst, live photo key frame)
│   ├── jxl/      (3 files: lossy, lossless, HDR)
│   ├── raw/      (5 files: CR2, NEF, ARW, DNG, RAF)
│   └── ...       (EXR, HDR, PSD, TIFF, BMP, GIF, TGA)
├── archives/     (ZIP, RAR, 7Z, CBZ, CBR, TAR.GZ)
├── documents/    (PDF, EPUB)
├── fonts/        (TTF, OTF)
├── video/        (MP4 H.264, MKV H.265 — for keyframe extraction)
└── MANIFEST.json (checksums, expected thumbnail hashes, metadata)
```

**Sourcing:** CC0/public-domain sample files from format specification repos,
synthetically generated with ImageMagick/FFmpeg, real camera RAWs.

### 7.4 Migration from Custom Framework to Catch2

1. Catch2 already added via `FetchContent` in v36.0.0 (Sprint 46)
2. Write all new tests in Catch2 format
3. Create `LEGACY_TEST()` macro bridge for gradual migration
4. Migrate tests that exercise real behavior; delete mechanical stubs
5. Remove `EngineTestsMacros.h` when migration complete
6. Target: Catch2 is primary by end of Phase 1

---

## 8. Documentation & Configuration Standards

### 8.1 Documentation Right-Sizing

**Problem:** 130+ markdown files — documentation outpaces working code.

| Tier | Audience | Files | Rule |
|------|----------|-------|------|
| **Tier 1 — User** | End users | `README.md`, `docs/USER_GUIDE.md`, `CHANGELOG.md`, `LICENSE` | Must reflect ONLY working features |
| **Tier 2 — Developer** | Contributors | `docs/development/`, `.github/CONTRIBUTING.md`, `.github/standards/` | Accurate build instructions |
| **Tier 3 — Architecture** | Deep contributors | `ROADMAP.md`, `docs/architecture/` | Vision + current state, clearly labeled |
| **Tier 4 — Historical** | Reference | `CHANGELOG-archive.md`, `docs/archive/` | Rarely accessed |

**Actions:**
- Archive `ROADMAP_V30.md`, `ROADMAP_V34.md`, `ROADMAP_V35.md` → `docs/archive/`
- Delete empty `packaging/inno/`, `nsis/`, `msix/`, `vdproj/`
- Merge `docs/PERFORMANCE.md` ← `.github/standards/performance-benchmarks.md`
- Update README.md to reflect actual validated capabilities (not aspirational)

### 8.2 GitHub Community Files (Fix Casing)

| Current | Required | Impact |
|---------|----------|--------|
| `.github/contributing.md` | `.github/CONTRIBUTING.md` | GitHub auto-detection for community health |
| `.github/security.md` | `.github/SECURITY.md` | Security tab "Policy" link |
| `.github/codeowners` | `.github/CODEOWNERS` | Auto-assign reviewers on PRs |

### 8.3 MkDocs Navigation (Fix Broken Links)

Run `mkdocs build --strict` — fix or remove 15+ phantom nav entries pointing to
nonexistent files. Add `mkdocs build --strict` as a CI check.

### 8.4 SVG Diagram Expansion

**Existing:** 5 high-quality SVGs in `docs/assets/`
**Target:** 13+ SVGs covering all major architecture flows

| New SVG | Purpose |
|---------|---------|
| `decode-pipeline.svg` | File → detect → route → decode → transform → thumbnail |
| `ci-cd-pipeline.svg` | 20 workflows mapped end-to-end |
| `release-flow.svg` | Bump-Version → 21 files → tag → release → 5 registries |
| `format-matrix.svg` | Visual grid of supported formats with status |
| `cache-architecture.svg` | L1 memory → L2 disk → invalidation |
| `plugin-lifecycle.svg` | Discovery → trust → sandbox → execute |
| `test-architecture.svg` | Corpus → Catch2 → CI → coverage → benchmark |
| `gpu-pipeline.svg` | CPU decode → D3D11 → compute → output |

### 8.5 Documentation Naming Convention

All docs use `UPPER_SNAKE_CASE.md` (except `README.md` and tool configs like `mkdocs.yml`).

### 8.6 Environment Reproducibility

| Item | Current | Target |
|------|---------|--------|
| Dev container | None | `.devcontainer/devcontainer.json` for Codespaces |
| Setup script | `Test-Build-Environment.ps1` | Extended `setup-dev-env.ps1` that installs all tools |
| Scoop manifest | Individual installs | `scoopfile.json` for one-command tool install |
| First-build time | Manual (30-60 min) | < 15 minutes from clone to passing tests |

---

## 9. CI/CD, Packaging & Distribution

### 9.1 CI Workflows (20 — All Real)

All 20 workflows are real implementations (~3K lines, 29 jobs). Key improvements:

| Improvement | Priority | Description |
|-------------|----------|-------------|
| **Corpus-based decoder tests in CI** | P0 | Upload test corpus to CI cache; validate on every PR |
| **Screenshot regression** | P1 | Generate thumbnails in CI; perceptual hash diff against baseline |
| **Binary size tracking** | P1 | Track `LENSShell.dll` size; alert on >10% growth |
| **Dependency scanning** | P2 | Dependabot for vcpkg + npm dependencies |

### 9.2 Package Registry Strategy (Simplify)

| Registry | Current | New Decision | Rationale |
|----------|---------|-------------|-----------|
| **NuGet** | ✅ Active | ✅ Keep | Natural for Windows C++ SDK distribution |
| **Container (ghcr.io)** | ✅ Active | ⏳ Defer to Phase 4 | Until `lens-server` is a real service |
| **npm** | ✅ Active | ⏳ Defer to Phase 6 | Until WASM module exists |
| **Maven** | ✅ Active | ❌ Remove | No Java consumers |
| **RubyGems** | ✅ Active | ❌ Remove | No Ruby consumers |

### 9.3 Distribution Channels (Realistic Priority)

| Channel | Priority | Status |
|---------|----------|--------|
| **GitHub Releases** | P0 | ✅ Active — MSI + ZIP + checksums |
| **winget** | P0 | Submit to Windows Package Manager |
| **Chocolatey** | P1 | `choco install explorerlens` |
| **Scoop** | P1 | Add to Scoop extras bucket |
| **Microsoft Store** | P2 | MSIX package when matured |

### 9.4 Packaging Cleanup

| Action | What |
|--------|------|
| ✅ Keep | `packaging/wix/` (active MSI) |
| ✅ Keep | `packaging/npm/` (if `package.json` exists) |
| ❌ Delete | `packaging/inno/`, `nsis/`, `msix/`, `vdproj/` (all empty) |

---

## 10. GitHub AI & Automation Surface

### 10.1 Current State

| Asset | Count | Assessment |
|-------|-------|------------|
| Copilot instructions | 1 (450 lines) | Monolithic; refactor to ~150 lines + scoped files |
| Scoped instructions | 5 | Missing: C++ coding, build, release, docs, security, performance, PR, decoder |
| Custom agents | 1 | Missing: Docs, TestCorpus, Release agents |
| Prompt templates | 5 | Missing: release-prep, architecture-review, decoder-scaffold, benchmark, PR-desc |
| Skills | 2 (~45 lines each) | Expand to ~150 lines; add decoder-dev, test-corpus, performance, docs skills |
| MCP servers | 3 | Evaluate: GitKraken/GitLens, Pylance |

### 10.2 Enhancement Plan

**A. Refactor `copilot-instructions.md` (P0)**
- Extract C++ standards → `cpp-coding.instructions.md` (`applyTo: "**/*.h,**/*.cpp"`)
- Extract build rules → `build.instructions.md` (`applyTo: "**/CMakeLists.txt,**/build-scripts/**"`)
- Extract release procedure → `release.instructions.md`
- Slim main file to ~150 lines (project overview + architecture + key constraints)

**B. Add 8 Scoped Instructions (P1)**

| File | Scope |
|------|-------|
| `cpp-coding.instructions.md` | `**/*.h,**/*.cpp` |
| `build.instructions.md` | `**/CMakeLists.txt,**/build-scripts/**` |
| `release.instructions.md` | `**/Bump-Version.ps1,**/CHANGELOG.md` |
| `documentation.instructions.md` | `**/*.md,docs/**` |
| `security.instructions.md` | `**/*.h,**/*.cpp,**/*.ps1,**/*.yml` |
| `performance.instructions.md` | `**/Engine/**,**/benchmarks/**` |
| `pr-authoring.instructions.md` | `.github/**` |
| `decoder-authoring.instructions.md` | `**/Engine/Decoders/**` |

**C. Add 3 Agents (P1)**

| Agent | Purpose |
|-------|---------|
| `Docs` | Documentation accuracy checking against actual code |
| `TestCorpus` | Corpus management, decoder validation, SSIM comparison |
| `Release` | Version bumps, release verification, artifact validation |

**D. Add 6 Prompts (P1)**

release-prep, architecture-review, decoder-scaffold, benchmark-analysis,
pr-description, debug-build-failure

**E. Expand Skills to Full Playbooks (P0)**

Both existing skills (~45 lines each) → ~150 lines with step-by-step procedures.
Add 4 new skills: decoder-development, test-corpus, performance, documentation.

**F. MCP Configuration Hygiene (P0)**

Remove `NO_PROXY`/`no_proxy` corporate artifacts from `.vscode/mcp.json`.

---

## 11. Shared Tooling Architecture

**Problem:** Every project under `MyScripts\` duplicates tool configuration. When rules
change, they must be manually propagated to each project.

**Principle:** Common tools live at `MyScripts\` (workspace root). Each project carries
only project-specific overrides.

### 11.1 Target Layout

```
MyScripts\                              ← SHARED (all projects inherit)
├── .editorconfig                       ← Universal editor rules
├── .markdownlint.json                  ← Markdown lint rules
├── .pre-commit-config.yaml             ← Shared pre-commit hooks
├── pyproject.toml                      ← Shared Python tool config
├── pyrightconfig.json                  ← Shared type-checking baseline
├── tooling/
│   ├── clang-tidy/.clang-tidy          ← Shared C++ lint baseline
│   └── cmake/msvc-v145.cmake           ← Shared MSVC toolchain
│
├── ExplorerLens.io/                    ← PROJECT-SPECIFIC ONLY
│   ├── .vscode/ (settings, launch, tasks, mcp)
│   ├── .clang-tidy                     ← Project-specific overrides
│   ├── CMakePresets.json               ← Project-specific presets
│   ├── vcpkg.json                      ← C++ dependency manifest
│   └── .github/                        ← Repo-specific CI, agents, instructions
```

### 11.2 Key Rules

1. **Inherit, don't duplicate.** If `MyScripts\` covers it, don't copy to project.
2. **Override only what differs.** Project config = delta from shared baseline.
3. **`.editorconfig` cascades natively** (spec-defined upward search).
4. **VS Code multi-root** cascades settings from outer to inner `.vscode/`.
5. **Never put secrets or machine-local paths** in shared configs.

### 11.3 Implementation Steps

1. Audit all config files across projects — classify as shared/override/unique
2. Consolidate shared configs at `MyScripts\`; delete project-level duplicates
3. Move shared `.clang-tidy` base to `MyScripts\tooling\clang-tidy\`
4. Create `MyScripts\TOOLING.md` documenting the inheritance architecture
5. Add CI check flagging duplicate configs

---

## 12. Frontend — Shell, GUI & CLI

### 12.1 LENSShell.dll (Core Product)

| Aspect | Current | Target |
|--------|---------|--------|
| COM interfaces | IThumbnailProvider | Add IExtractImage2 (legacy), IPreviewHandler (preview pane), IContextMenu (right-click), **IPropertyStore** (metadata columns in Details view — dimensions, codec, camera model) |
| Registration | Manual `regsvr32` | MSI auto-registration + `lens register` CLI |
| Error handling | Basic HRESULT | Structured logging to ETW + Windows Event Log |
| Thumbnail sizes | Standard | 16×16 to 1024×1024 (Extra Large Icons) |
| Threading | STA | Verify MTA safety for Explorer's thread pool |

### 12.2 LENSManager.exe (Configuration GUI)

| Decision | Current | Target |
|----------|---------|--------|
| Framework | WTL (Win32) | **Keep WTL for v1** — lightweight, works; consider WinUI 3 for v2 |
| Dark mode | Custom controller | Use `SetPreferredAppMode()` from uxtheme.dll |
| System tray | Basic icon | Add balloon notifications for decode errors |

**Why not WinUI 3 now?** It adds 100+ MB of dependencies. WTL is the right choice
for a system utility that ships as < 1 MB. QuickLook uses WPF (15+ MB managed runtime) —
we stay leaner with native Win32.

### 12.3 lens.exe (CLI Tool)

**Priority commands to make work:**

| Command | Purpose |
|---------|---------|
| `lens generate <file> [-o output.png] [-s 256]` | Generate thumbnail — validates decoder end-to-end |
| `lens info <file>` | Format detection result + metadata |
| `lens register [--per-user]` | Register/unregister shell extension |
| `lens doctor` | System diagnostics: GPU, libraries, registration, cache health |
| `lens benchmark <directory>` | Batch decode with P50/P95/P99 output |
| `lens cache stats` | Hit rate, size, eviction count |

### 12.4 Web Frontend (index.html) ✅

~~Move to `docs/` for GitHub Pages.~~ **Done (Sprint 33).** `index.html` relocated to `docs/`;
Pages workflow updated to deploy from `docs/`. Keep it simple: a marketing page explaining what
ExplorerLens does, with screenshots and download link. **Screenshots of actual generated
thumbnails** are the most important marketing asset for a visual product.

---

## 13. Backend — Engine & Decode Pipeline

### 13.1 Priority: Top 20 Formats Must Work Flawlessly

| Priority | Format | Library | Target P50 | Validation Requirements |
|----------|--------|---------|------------|-------------------------|
| P0 | JPEG | libjpeg-turbo / WIC | < 5 ms | EXIF rotation, progressive, CMYK |
| P0 | PNG | WIC / libpng | < 5 ms | 8/16-bit, alpha, interlaced, APNG first frame |
| P0 | WebP | libwebp | < 8 ms | Lossy, lossless, alpha, animated first frame |
| P0 | AVIF | libavif + dav1d | < 10 ms | 8/10-bit, HDR, animated first frame |
| P0 | HEIC | libheif + libde265 | < 10 ms | Single image, burst, live photo key frame |
| P0 | JXL | libjxl | < 12 ms | Lossy, lossless, HDR |
| P0 | PDF | MuPDF | < 20 ms | First page at 256px |
| P0 | RAW | LibRaw | < 25 ms | Embedded preview extraction (fast path) |
| P1 | ZIP/CBZ | minizip-ng | < 15 ms | First image in archive |
| P1 | RAR/CBR | UnRAR | < 15 ms | First image in archive |
| P1 | 7Z/CB7 | LZMA SDK | < 15 ms | First image in archive |
| P1 | EPUB | minizip-ng + extract | < 20 ms | Cover image extraction |
| P1 | GIF | WIC | < 5 ms | First frame of animated |
| P1 | BMP | WIC / stb_image | < 2 ms | Standard |
| P1 | TIFF | WIC / libtiff | < 8 ms | Multi-page: first page |
| P2 | EXR | tinyexr | < 15 ms | Tone-map to sRGB |
| P2 | PSD | Custom parser | < 15 ms | Composite first layer |
| P2 | DDS | DirectXTex | < 5 ms | BC1-BC7 block decode |
| P2 | SVG | WIC / Direct2D | < 10 ms | Rasterize at target size |
| P2 | TTF/OTF | FreeType | < 10 ms | Render sample "AaBb123" |

### 13.2 Decoder Architecture Improvements

**Two-phase decode (harvested from XnView pattern):**
```cpp
class IStreamingDecoder {
    // Phase 1: Read only header (first 16 KB) to confirm format + extract metadata
    virtual DecodeResult ProbeHeader(std::span<const uint8_t> header) = 0;

    // Phase 2: Minimal decode at requested thumbnail size
    virtual DecodeResult DecodeAtSize(IStream* stream, uint32_t targetSize,
                                      std::stop_token cancel) = 0;

    virtual bool SupportsPartialDecode() const { return false; }
};
```

**Key insight from IrfanView:** For RAW photos, Phase 2 extracts the embedded JPEG
preview via `LibRaw::unpack_thumb()` — this is 100× faster than full RAW decode and
is what the user actually wants for a thumbnail.

### 13.3 Cache Architecture (Harvested from macOS Quick Look)

macOS Quick Look uses SQLite-indexed thumbnail cache with file watchers for
invalidation. We adopt the same pattern:

| Feature | Description |
|---------|-------------|
| **L1 (memory)** | LRU map, 64 MB budget, BGRA bitmaps |
| **L2 (disk)** | `%LOCALAPPDATA%\ExplorerLens\Cache\`, SQLite index, memory-mapped blobs |
| **Cache key** | `SHA256(canonical_path + mtime + size + target_dimensions)` |
| **Eviction** | LRU with size budget; L1 → L2 on evict; L2 expires oldest at disk budget |
| **Invalidation** | `ReadDirectoryChangesW` watcher per opened Explorer folder |
| **Persistence** | L2 survives reboots; cold start reads SQLite index |
| **Metrics** | Hit rate, miss rate, eviction count — exposed via `lens cache stats` |

---

## 14. GPU, Cross-Platform & Advanced Features

### 14.1 GPU Pipeline — Realistic Plan

**Current:** No shader files exist. Architecture headers but no implementation.

| Phase | What | How | Measurable Target |
|-------|------|-----|-------------------|
| **Phase 2** | WIC + D3D11 hints | `IWICImagingFactory2` with D3D device | 1.5-2× JPEG/PNG speedup |
| **Phase 2** | D3D11 compute resize | `resize_bilinear.hlsl` | < 0.5 ms for 4K→256px |
| **Phase 3** | DXVA2 video decode | Hardware keyframe extraction | 10× video thumbnail speedup |
| **Phase 4** | GPU tone-mapping | `tonemap_pq_to_srgb.hlsl` | < 0.5 ms HDR→SDR |

**Shaders to write:**
- `resize_bilinear.hlsl` — fast thumbnail resize (SRV → UAV)
- `resize_lanczos.hlsl` — high-quality resize
- `tonemap_pq_to_srgb.hlsl` — HDR10 PQ → sRGB
- `demosaic_bayer.hlsl` — RAW Bayer demosaic

### 14.2 Cross-Platform — Honest Timeline

| Platform | Phase | Approach |
|----------|-------|----------|
| **Windows 10/11** | Phase 1 (now) | COM IThumbnailProvider — the core product |
| **macOS** | Phase 5 | Quick Look QLThumbnailProvider; Metal backend; Homebrew |
| **Linux** | Phase 5 | Nautilus/Dolphin tumbler; Vulkan backend; Flatpak |
| **Web/WASM** | Phase 6 | Server-side `lens-server` or Emscripten module |

### 14.3 AI/ML Features — Defer to Phase 5+

| Feature | Realistic Timeline | Dependency |
|---------|-------------------|------------|
| Smart crop (saliency) | Phase 5 | ONNX Runtime or DirectML |
| Scene understanding | Phase 6 | Research-grade |
| Semantic search (CLIP) | Phase 6 | Embedding model |
| Generative thumbnails | Phase 7+ | Not a core need |

### 14.4 Features That Provide Value NOW

| Feature | Why | Implementation |
|---------|-----|----------------|
| EXIF-aware rotation | Users see sideways photos without this | libjpeg-turbo EXIF orientation |
| Embedded RAW preview | 100× faster than full decode | `LibRaw::unpack_thumb()` |
| Archive cover image | CBZ/EPUB users expect this | First image alphabetically |
| PDF first page | Universal document thumbnail | `fz_new_pixmap_from_page()` |
| Video keyframe | Better than generic icon | `IMFSourceReader` at 10% duration |
| Font sample text | Shows font appearance | FreeType render "AaBb123" |

### 14.5 Cloud Features (Defer to Phase 4)

| Feature | Action |
|---------|--------|
| Windows Cloud Files hydration detection | Implement in Phase 4 (uses real CF API) |
| Enterprise GPO (ADMX/ADML) | Phase 4 |
| ETW tracing | Phase 4 |
| `lens-server` REST API | Phase 4 |
| Streaming cache, zero-trust, collaboration | Phase 5+ |
| WebAssembly, cross-device sync | Phase 6+ |

---

## 15. Infrastructure & Operations

### 15.1 Crash Reporting & Stability

| Component | Current | Target |
|-----------|---------|--------|
| Crash dump collection | None | Windows Error Reporting (WER) with `SetUnhandledExceptionFilter` |
| Telemetry | None | Optional ETW-based telemetry (opt-in, no PII) |
| Diagnostics CLI | `lens doctor` (early) | Full system diagnostics: GPU, libs, registration, cache health, codec availability |
| Error logging | Basic HRESULT | Structured logging to Windows Event Log (`ExplorerLens` source) |

### 15.2 Auto-Update & Distribution Infrastructure

| Channel | Mechanism | Status |
|---------|-----------|--------|
| winget | `winget upgrade ExplorerLens.ExplorerLens` | Priority — submit manifest |
| Scoop | `scoop update explorerlens` | `scoopfile.json` exists; submit to extras bucket |
| Chocolatey | `choco upgrade explorerlens` | `packaging/chocolatey/` exists; submit package |
| In-app check | LENSManager checks GitHub Releases API for newer version | Implement in Phase 3 |
| MS Store | MSIX package for sandboxed install | Phase 4 |

### 15.3 Database Strategy

**No traditional database needed.** The only persistent state is the thumbnail cache:

| Layer | Storage | Technology |
|-------|---------|------------|
| L1 cache | In-process memory | Robin-Hood hash map (XXH3 keys), LRU eviction, 64 MB budget |
| L2 cache index | `%LOCALAPPDATA%\ExplorerLens\cache.db` | SQLite 3 (single-file, crash-safe, read-concurrent) |
| L2 cache blobs | `%LOCALAPPDATA%\ExplorerLens\Cache\*.thumb` | Memory-mapped files, size-budgeted |
| Settings | `HKCU\Software\ExplorerLens` | Windows Registry (via LENSManager) |
| Corpus baselines | `data/baselines/` | JSON (committed to repo; used by CI) |

### 15.4 Security Hardening Roadmap

| Item | Priority | Description |
|------|----------|-------------|
| Fuzz all decoders | P0 | libFuzzer / WinAFL against each decoder with malformed files |
| ASAN builds in CI | P1 | AddressSanitizer for memory safety validation |
| Stack canaries | ✅ Done | `/GS` flag (MSVC default) |
| ASLR + DEP + CFG | ✅ Done | `/DYNAMICBASE`, `/NXCOMPAT`, `/guard:cf` |
| Input validation | P0 | Validate file size, magic bytes, dimensions before decode |
| Integer overflow checks | P1 | `SafeInt<>` for dimension calculations (width × height × bpp) |
| Sandboxed decode | P2 | AppContainer or low-integrity process for untrusted files |
| Code signing | P2 | Authenticode sign `LENSShell.dll`, `LENSManager.exe`, MSI |

### 15.5 Monitoring & Observability

| Signal | Mechanism | Consumer |
|--------|-----------|----------|
| Decode latency | ETW provider (`ExplorerLens-Engine`) | Windows Performance Analyzer, `lens benchmark` |
| Cache hit/miss rate | Performance counters | `lens cache stats`, LENSManager dashboard |
| Error rate | Windows Event Log | SIEM, `lens doctor` |
| Binary size | CI workflow (`binary-size.yml`) | PR gate |
| Memory usage | ETW + `lens doctor` | Support diagnostics |

---

## 16. Phase Plan — 6 Phases to Best-in-Class

### Phase 1 — Foundation (4-6 weeks)
**Goal:** Working, validated, installable product for top 20 formats.

**Infrastructure & cleanup:**
- [x] Delete dead code: `src/LensServer/`, `src/PluginHost/`, `src/Tools.PSModule/`, `Engine/Tests/FuzzTargets/`, `Engine/Tests/gtest/` *(v36.2.0)*
- [x] Split `EngineTests_Late.cpp` at ~486 KB → Late (215 KB) + Platform (256 KB) *(v36.2.0)*
- [x] Fix stale version references: `coverage.yml` (v23.6.0→v36.1.0), `ci-matrix.yml` (v32.1.0→v36.1.0), `LIBRARY_INVENTORY.md` (v15.0.0→v36.1.0) *(v36.2.0)*
- [x] Evaluate MuPDF AGPL license: documented in ADR-009 — keep with compliance docs, evaluate PDFium Phase 3 *(v36.2.0)*
- [x] Fix Dockerfile VS 2022 → VS 2026 BuildTools *(v36.2.0)*
- [x] Honest README: tempered GPU claims, added AGPL notice *(v36.2.0)*
- [x] Drop Maven + RubyGems from CI publish (R5) — 3 registries remain *(v36.3.0)*
- [x] Header audit tooling: `build-scripts/Audit-Headers.ps1` classifies Real/Stub/Dead/Orphan *(v36.3.0)*
- [x] Fix mkdocs.yml: removed 19 broken nav entries referencing non-existent files (R8) *(v36.3.0)*
- [x] Clean external lib references: renamed DarkThumbs→ExplorerLens, fixed /MT→/MD (R8, R10) *(v36.3.0)*
- [x] Document Catch2 migration decision in ADR-010 *(v36.3.0)*
- [ ] Shared tooling architecture (§11): audit configs, consolidate at MyScripts\, establish inheritance
- [x] GitHub AI surface overhaul (§10): 13 instructions, 4 agents, 11 prompts, 6 skills, MCP config *(v36.1.0–v36.3.0)*
- [x] Delete dead headers: 6 superseded stubs + GLTFModelDecoderTests.cpp removed *(v36.4.0)*
- [x] Archive `ROADMAP_V30.md`, `ROADMAP_V34.md`, `ROADMAP_V35.md` → `docs/archive/` *(v36.1.0)*

**Core product:**
- [x] Create test corpus: synthetic corpus with 21 files covering images, docs, archives, 3D models *(v36.3.0)*
- [x] Integrate Catch2 as primary test framework: enabled by default, 9 test files, corpus validation *(v36.3.0)*
- [ ] Verify all 18 external libraries build and link correctly
- [x] Implement or fix top 20 format decoders with real `.cpp` files — all verified *(v36.3.0 audit)*
- [ ] Expand test corpus: 5+ real CC0 files per format (100+ total)
- [ ] Write 500+ meaningful tests replacing mechanical stubs
- [ ] Run all decoders against corpus → 100% correct output
- [ ] `lens generate <file>` works for all 20 formats
- [ ] `regsvr32 LENSShell.dll` works on clean Windows 10 VM
- [x] Update README.md to reflect actual validated capabilities *(v36.3.0)*

**Exit criteria:** A user installs the MSI, and every file in their Photos folder
gets a correct, fast thumbnail in Explorer.

### Phase 2 — Performance (3-4 weeks)
**Goal:** Measurably faster than Windows built-in thumbnails.

- [ ] Implement two-tier cache (L1 memory + L2 disk with SQLite index)
- [ ] Add `ReadDirectoryChangesW` cache invalidation
- [ ] WIC + D3D11 device hints for JPEG/PNG/WebP
- [ ] Write `resize_bilinear.hlsl` compute shader
- [ ] Benchmark all decoders → P50/P95/P99 baselines
- [ ] Google Benchmark integration in CI
- [ ] Performance regression gate: >10% P95 → PR blocked
- [ ] `lens benchmark <dir>` produces JSON report
- [ ] Target: 5ms JPEG, 8ms WebP, 20ms PDF

**Exit criteria:** ExplorerLens is measurably faster than Windows built-in for every
supported format.

### Phase 3 — Format Breadth (4-6 weeks)
**Goal:** 80+ validated format families (200+ extensions).

- [ ] Add remaining image formats: EXR, PSD, HDR, QOI, TGA, ICO, DDS, SVG
- [ ] Add archive: TAR, GZ, BZ2, XZ, ISO
- [ ] Add document: EPUB, MOBI, CHM, RTF, DOCX (embedded preview)
- [ ] Add 3D: glTF/GLB, STL, OBJ (wireframe)
- [ ] Add fonts: TTF, OTF, WOFF (sample text render)
- [ ] Add video: MP4, MKV, AVI, WebM (keyframe extraction)
- [ ] Add RAW: all LibRaw-supported cameras (100+ models)
- [ ] Plugin SDK v1: C ABI with `probe()`, `decode()`, `metadata()` functions
- [ ] Submit to winget + Chocolatey + Scoop
- [ ] Expand test corpus to 300+ files

**Exit criteria:** 80+ format families, all validated against real test files.

### Phase 4 — Enterprise & Cloud (4-6 weeks)
**Goal:** Production-ready for enterprise and headless deployment.

- [ ] Windows Cloud Files API hydration detection
- [ ] GPO template (ADMX/ADML) for enterprise configuration
- [ ] ETW tracing for decode pipeline
- [ ] Event Log entries for decode errors
- [ ] WER crash reporting with `SetUnhandledExceptionFilter`
- [ ] IPropertyStore for image/video metadata in Explorer Details view
- [ ] `lens-server` REST API with cpp-httplib (replace Winsock2 skeleton, thread pool)
- [ ] Update Dockerfile to VS 2026 BuildTools when available
- [ ] Docker container for `lens-server`
- [ ] Code signing (Authenticode) for all binaries and MSI
- [ ] In-app update check (LENSManager → GitHub Releases API)
- [ ] Security audit: fuzz all decoders with libFuzzer/WinAFL, fix all crashes
- [ ] ASAN builds in CI for memory safety
- [ ] SBOM with real dependency graph

**Exit criteria:** IT admins deploy via GPO, monitor via SIEM, run `lens-server`
in CI pipelines.

### Phase 5 — Cross-Platform (6-8 weeks)
**Goal:** macOS Quick Look extension shipping.

- [ ] macOS PAL backend (QLThumbnailProvider)
- [ ] Metal backend for GPU operations
- [ ] FSEvents for file change detection
- [ ] Homebrew formula
- [ ] SSIM ≥ 0.99 between Windows and macOS output
- [ ] ONNX Runtime for on-device AI (smart crop)
- [ ] Linux Nautilus thumbnailer (basic)

**Exit criteria:** macOS users install via Homebrew and get thumbnails for top 20
formats in Finder.

### Phase 6 — AI & Advanced (Ongoing)
**Goal:** Intelligent, content-aware thumbnails.

- [ ] Smart crop using saliency detection
- [ ] HDR tone-mapping on GPU (PQ/HLG/Gainmap)
- [ ] CLIP embedding for semantic search
- [ ] Predictive pre-generation based on navigation patterns
- [ ] WebAssembly build for browser use

---

## 17. Success Metrics

### Phase 1 (Foundation)

| Metric | Target |
|--------|--------|
| Validated format families | ≥ 20 |
| Test corpus files | ≥ 100 |
| Catch2 tests passing | ≥ 500 |
| Header-to-source ratio | < 3:1 |
| Clean install on Windows 10 VM | Yes |
| Build: 0 errors, 0 warnings | Yes |
| MuPDF license resolution | Documented and implemented |
| Stale version references | 0 (all files at current release) |
| `EngineTests_Late.cpp` size | < 400 KB after split |
| Dead code directories removed | `src/LensServer/`, `PluginHost/`, `PSModule/`, `FuzzTargets/` |

### Phase 2 (Performance)

| Metric | Target |
|--------|--------|
| JPEG 6MP P50 | < 5 ms |
| PNG 4K P50 | < 5 ms |
| WebP P50 | < 8 ms |
| PDF first-page P50 | < 20 ms |
| Cache hit P50 | < 1 ms |
| LENSShell.dll size | < 5 MB |
| Idle memory | < 30 MB |

### Phase 3 (Breadth)

| Metric | Target |
|--------|--------|
| Validated format families | ≥ 80 |
| File extensions supported | ≥ 200 (all validated) |
| Test corpus files | ≥ 300 |
| winget / Chocolatey / Scoop | Published |
| GitHub stars | ≥ 100 |

### Phase 4 (Enterprise)

| Metric | Target |
|--------|--------|
| WER crash reporting | Integrated + tested |
| IPropertyStore columns | Dimensions + codec for top 20 formats |
| `lens-server` REST API | Functional with thread pool |
| Decoder fuzz targets | ≥ 20 (one per priority format) |
| ASAN CI builds | Green (zero leaks) |
| Code signing | All binaries Authenticode-signed |

### Best-in-Class Criteria

| Dimension | Definition |
|-----------|-----------|
| **Speed** | Faster than OS built-in for every format |
| **Coverage** | More validated formats than any competitor |
| **Correctness** | EXIF rotation, color management, HDR tone-mapping — all correct |
| **Reliability** | Zero crashes on malformed files (fuzz-tested) |
| **Size** | < 5 MB DLL |
| **Memory** | < 50 MB under load; < 10 MB idle |
| **Install** | `winget install ExplorerLens` — one command |
| **Cross-platform** | Windows + macOS + Linux |
| **Extensible** | Plugin SDK for third-party decoders |
| **Observable** | ETW, Event Log, CLI diagnostics |

---

## 18. Decision Log

| # | Decision | Rationale |
|---|----------|-----------|
| D1 | Keep C++20 + MSVC v145 | COM requires native code; no managed alternative |
| D2 | Keep CMake + Ninja | Industry standard; presets; sccache-compatible |
| D3 | Migrate to Catch2 v3 | Industry standard; XML output; parameterized tests |
| D4 | vcpkg as primary dependency path | Eliminates 13 custom build scripts; Dependabot |
| D5 | Flatten Engine/ from 16 → 7 subdirectories | Remove premature subdivisions |
| D6 | Create real test corpus | Cannot validate decoders without real files |
| D7 | Defer cross-platform to Phase 5 | Windows must be excellent first |
| D8 | Defer AI/ML to Phase 5+ | Core value is fast, correct thumbnails |
| D9 | Remove Maven + RubyGems registries | No consumers exist |
| D10 | Add winget / Chocolatey / Scoop | Primary Windows distribution channels |
| D11 | Keep version at v36+ | History has value; slow velocity to feature-gated bumps |
| D12 | Single ROADMAP.md | 3 files creates confusion |
| D13 | Right-size docs (130 → ~60 files) | Docs should lag code, not lead it |
| D14 | Implement-before-declare rule | No header without .cpp > 50 LOC |
| D15 | Shared tooling at workspace root | Common tools at MyScripts\; projects carry overrides |
| D16 | Full AI tooling surface | 13 instructions, 4 agents, 11 prompts, 6 skills, 5 MCP |
| D17 | Config/docs/env to latest standards | GitHub casing, mkdocs nav, SVG diagrams, dev container |
| D18 | Add libjpeg-turbo | 2-4× faster JPEG decode than WIC |
| D19 | SQLite for L2 cache index | Proven pattern (macOS Quick Look); atomic, crashproof |
| D20 | IContextMenu for right-click preview | Harvested from SageThumbs; immediate discoverability |
| D21 | Plugin SDK with C ABI | Harvested from QuickLook's `.qlplugin`; enables community |
| D22 | `std::expected<T,E>` for new APIs | C++23 error handling; available in MSVC 19.50 |
| D23 | IPropertyStore for Explorer metadata | Harvested from Icaros; show dimensions/codec in Details view |
| D24 | WER crash reporting | Essential for production stability in explorer.exe context |
| D25 | Evaluate MuPDF license alternatives | AGPL-3.0 conflicts with MIT; consider PDFium (BSD) or commercial license |
| D26 | Split EngineTests_Late.cpp at 486 KB | File size policy compliance; prevents git perf issues |
| D27 | Delete dead src/ directories | LensServer, PluginHost, PSModule have no build integration |
| D28 | Add Icaros-style IPropertyStore | Show image/video metadata in Explorer Details columns |
| D29 | Nightly CI builds for early adopters | Harvested from QuickLook's nightly distribution model |

### Decisions Preserved from Original Architecture

| Decision | Origin | Still Valid |
|----------|--------|-------------|
| Static linking of all externals | v1.0 | ✅ Essential for COM DLL |
| `/MD` CRT for all targets | v15.0 | ✅ Eliminates CRT conflicts |
| Zero-warnings policy | v1.0 | ✅ Professional standard |
| COM CLSID immutable | v1.0 | ✅ Changing it breaks upgrades |
| LENSTYPE enum for format routing | v1.0 | ✅ Simple and effective |

---

## How to Use This Roadmap

1. **Phase 1 is the priority.** Everything else waits until the foundation is solid.
2. **Measure progress** by success metrics in §17, not by header count or version number.
3. **This document supersedes** `ROADMAP_V30.md`, `ROADMAP_V34.md`, `ROADMAP_V35.md`.
4. **Archive old roadmaps** to `docs/archive/` for historical reference.
5. **Check off items** as completed. This is a living document.
6. **Compare against competitors** in §2.1 regularly — the landscape shifts.

---

## 19. Consolidated Legacy — What We Kept from V30-V35

This section preserves the valuable strategic decisions from the three superseded roadmaps.
The full original documents are archived at `docs/archive/`.

### From ROADMAP_V30 "Deneb" (v30-v33)

**Preserved decisions:**
- Platform Abstraction Layer (PAL) architecture — validated and implemented in `Engine/Platform/`
- DirectStorage zero-copy GPU decompression concept — deferred to Phase 3 but architecture is sound
- Live Preview Scrubber for video — deferred to Phase 3; Media Foundation approach confirmed
- Risk mitigations for DirectStorage CI unavailability (mock layer) — still applicable

**Archived (not carried forward):**
- CLIP semantic search with HNSW index — deferred to Phase 6 (AI features)
- Generative AI thumbnails (SD-Turbo on-device NPU) — deferred to Phase 6+
- LevelDB dependency — replaced by SQLite for L2 cache

### From ROADMAP_V34 "Arcturus" (v34)

**Preserved decisions:**
- Three pillars: format breadth, sub-10ms decode, zero-regression gates — still the core strategy
- Per-PR automated benchmark gates blocking >5% P95 regression — implemented in CI
- Ultra HDR (Google Gainmap), Apple ProRAW support — in decoder roadmap

**Archived (not carried forward):**
- 20-task consolidation plan at tail — from earlier era; contradicts current architecture
- 350+ extension target as a Phase 1 goal — realistic target is 200+ validated

### From ROADMAP_V35 "Vega" (v35)

**Preserved decisions:**
- LensServer REST API concept — validated by `src/LensServer/` skeleton
- Cloud Files hydration detection via CF API — deferred to Phase 4
- Dockerfile + container deployment — exists and works (needs toolchain update)

**Archived (not carried forward):**
- Real-time collaboration / live-sync session tokens — deferred to Phase 6+
- Zero-trust security with signed thumbnail manifests — deferred to Phase 5
- WebAssembly decoder sandbox — deferred to Phase 6
- SDXL-Turbo generative thumbnails — speculative; depends on upstream stability
- Post-quantum cryptography provider — academic research, not production-relevant

### From ROADMAP v2.0 "Altair" (v36)

**All content carried forward and enhanced in this v3.0.** Key additions:
- Icaros + ImageGlass added to competitor analysis
- Infrastructure & Operations section (§15) — crash reporting, auto-update, security hardening
- MuPDF AGPL license concern identified (R10)
- LensServer scalability concern identified (R13)
- IPropertyStore for Explorer metadata columns (D23, D28)
- Nightly CI build distribution model (D29)
- File size policy compliance for EngineTests_Late.cpp (R19, D26)

---
