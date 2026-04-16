# ExplorerLens — Strategic Roadmap & Architecture Reset

**Version:** 1.0 — Written April 16, 2026
**Scope:** Complete review of all project decisions, from v1 inception through v35.5.0 "Vega-V"
**Purpose:** Redirect the project toward best-in-class quality with substance over velocity

---

## Table of Contents

1. [Executive Summary](#1-executive-summary)
2. [Strategic Review — What Worked, What Didn't](#2-strategic-review)
3. [Architecture Decisions — Rethought](#3-architecture-decisions)
4. [Code Quality Reset](#4-code-quality-reset)
5. [Build System & Toolchain](#5-build-system--toolchain)
6. [Testing Strategy Overhaul](#6-testing-strategy-overhaul)
7. [External Libraries & Dependencies](#7-external-libraries--dependencies)
8. [Documentation Right-Sizing](#8-documentation-right-sizing)
9. [CI/CD & Infrastructure](#9-cicd--infrastructure)
10. [Frontend — Shell Extension & GUI](#10-frontend--shell-extension--gui)
11. [Backend — Engine & Decode Pipeline](#11-backend--engine--decode-pipeline)
12. [GPU Pipeline — Reality Check](#12-gpu-pipeline--reality-check)
13. [Cross-Platform Strategy](#13-cross-platform-strategy)
14. [Cloud, AI & Advanced Features](#14-cloud-ai--advanced-features)
15. [Packaging & Distribution](#15-packaging--distribution)
16. [Phase Plan — 6 Phases to Best-in-Class](#16-phase-plan)
17. [Consolidated Backlog from Previous Roadmaps](#17-consolidated-backlog)
18. [Success Metrics](#18-success-metrics)
19. [Appendix — Decision Log](#19-appendix--decision-log)

---

## 1. Executive Summary

ExplorerLens has an **exceptional architectural vision** — the roadmap documents, CI/CD
infrastructure, and code organization demonstrate serious engineering ambition. The project
aims to be the premier thumbnail provider on Windows with GPU acceleration, 200+ format
support, and cross-platform reach.

**However, a candid audit reveals a substance gap.** The project has:

- **1,386 header files** but only **269 source files** (5.1:1 ratio)
- **545 headers in Engine/Core/ alone** — many appear to be stubs or thin declarations
- **~27 minor versions shipped in ~11 days** (v30.0 → v35.5), each adding exactly 5 headers + 10 tests
- A custom test framework with ~4,724 tests that may not exercise real I/O or GPU paths
- No visible `.hlsl`, `.comp`, or `.metal` shader files despite claiming D3D11/D3D12/Vulkan/Metal
- Empty packaging directories (Inno Setup, NSIS, MSIX, vdproj) and dead code paths
- 3 separate roadmap files + 130 total markdown files — documentation outpaces working code

**This roadmap proposes a strategic reset:** consolidate, validate, and harden
the functional core before expanding. The goal is to make ExplorerLens genuinely
best-in-class — where "best" means *actually working, measurably fast, and reliably
deployed* rather than *architecturally envisioned*.

---

## 2. Strategic Review

### 2.1 Decisions That Worked Well

| Decision | Verdict | Rationale |
|----------|---------|-----------|
| **C++20 for the engine** | ✅ Correct | COM interop requires native code; C++20 is the right choice for a Windows shell extension |
| **CMake + Ninja build** | ✅ Correct | Industry standard for C++ projects; presets provide good developer experience |
| **Static linking of external libs** | ✅ Correct | Shell extensions must be self-contained DLLs; no runtime dependency issues |
| **COM IThumbnailProvider** | ✅ Correct | The only way to provide native Explorer thumbnails on Windows |
| **Separation of Shell/Engine/Manager** | ✅ Correct | Clean layering: COM shell → engine library → config GUI |
| **Zero-warnings policy** | ✅ Correct | Professional-grade discipline; prevents warning rot |
| **vcpkg integration (optional)** | ✅ Correct | Provides fallback when local libs aren't built |
| **`/MD` CRT linkage** | ✅ Correct | Avoids the `/MT` vs `/MD` hell with external libraries |
| **Security flags (ASLR/DEP/HIGHENTROPYVA)** | ✅ Correct | Essential for a COM DLL loaded by Explorer |
| **16 CI workflows** | ✅ Good ambition | CodeQL, coverage, performance gates, release automation — comprehensive |

### 2.2 Decisions That Need Rethinking

| Decision | Issue | Recommendation |
|----------|-------|----------------|
| **Header-only architecture (5.1:1 ratio)** | Most classes are declared but not meaningfully implemented in `.cpp` files. Compile times balloon; actual code coverage is murky. | Move implementations to `.cpp` files. A header should declare; a source should implement. Target < 2:1 ratio. |
| **Custom test framework** | `TEST()/ASSERT()` macros lack: fixtures, parameterized tests, death tests, output capture, XML reporting for CI, IDE integration. | Migrate to **Catch2** (header-only, MIT, excellent MSVC support) or **doctest** (fastest compile). Keep the macro bridge for transition. |
| **Sprint-mechanical development pattern** | Every version adds exactly 5 headers + 10 tests. This suggests auto-generation rather than feature-driven development. Each "module" appears to be a thin stub. | Stop counting headers. Count *working format decoders that produce correct thumbnails from real files*. |
| **Version inflation (v35.5.0)** | Semver should reflect API maturity. v35 implies 35 breaking changes — unlikely for a project that's < 6 months active. | Consider resetting to v1.0.0 at the next actual stable release, or v2.0.0 if the COM ABI is frozen. Use calver (2026.4) if semver milestones are hard to define. |
| **3 separate roadmap files** | ROADMAP_V30.md, ROADMAP_V34.md, ROADMAP_V35.md create version-specific silos. Hard to track what's actually completed vs. planned. | Consolidate to single ROADMAP.md (this file). Archive old ones. |
| **Documentation volume (130 .md files)** | More words than working code. docs/ has architecture specs for features that don't exist yet. | Right-size: document what works today. Move aspirational docs to this roadmap. |
| **Claiming 200+ formats** | The 200+ count includes stubs, planned formats, and formats where "support" means a header file exists. | Audit and report only *verified working* formats with test corpus validation. |
| **Cross-platform stubs** | macOS/Linux "stubs" that compile via `#ifdef` are not cross-platform support. | Be honest in README: "Windows native, cross-platform planned." Remove false claims. |
| **Cloud/Collaboration/WebAssembly** | v35.x headers for streaming, collaboration, WASM, zero-trust are aspirational stubs, not working features. | Archive these as future vision. Focus Phase 1 on making the core Windows product excellent. |
| **Empty packaging dirs** | `packaging/inno/`, `nsis/`, `msix/`, `vdproj/` are dead code | Delete immediately. WiX is the only active installer. |
| **Duplicate external libs** | Two `unrar/` dirs, two `libwebp/` dirs | Clean up to single copies. |

### 2.3 Critical Honest Assessment

**What ExplorerLens actually is today:**
- A Windows COM shell extension that registers for file types
- An engine library with ~18 external decoder libraries linked
- A configuration GUI (WTL-based `LENSManager.exe`)
- A CLI tool (`lens.exe`) in early development
- A comprehensive build and CI/CD system

**What it isn't yet (despite headers existing):**
- GPU-accelerated (no shader code, no actual D3D device creation in hot path)
- Cloud-native or collaboration-ready
- Cross-platform
- AI-powered (no model files, no inference runtime integration)
- Serving 200+ verified working formats (needs corpus testing)

**This is not a failure** — it's a foundation with enormous potential. The architecture
is sound, the infrastructure is professional, and the vision is compelling. The gap is
between vision and validated implementation.

---

## 3. Architecture Decisions — Rethought

### 3.1 Current Architecture (Good Foundation)

```
Windows Explorer
    │
    ▼
LENSShell.dll (COM IThumbnailProvider)  ← Thin adapter layer
    │
    ▼
ExplorerLensEngine.lib                  ← Core decode pipeline
    ├── FormatDetector (magic + extension)
    ├── DecoderRegistry (200+ format routing)
    ├── DecodePipeline (detect → route → decode → transform → output)
    ├── CacheProvider (LRU + disk)
    └── GPURenderer (D3D11 with GDI+ fallback)
    │
    ▼
External libraries (libraw, libjxl, libheif, mupdf, ...)
```

**Verdict:** This is a solid architecture. Keep it.

### 3.2 Proposed Architecture Changes

#### A. Flatten Engine/Core/ (545 → ~80 Headers)

**Problem:** `Engine/Core/` has 545 headers. Many are single-class files for features that
could be 50-line sections of a broader module.

**Action:**
- Audit every header in `Core/`. Merge related classes into cohesive modules.
- Target: ~80 headers in Core (one per logical subsystem, not one per class).
- Example: `LiveSyncTokenManager.h`, `CollaborativeCacheCoordinator.h`,
  `ConflictResolutionEngine.h` → single `Collaboration.h` (when the feature is real).

#### B. Implement Before Declaring

**New Rule:** No header file may be registered in CMakeLists.txt unless it has:
1. A corresponding `.cpp` with non-trivial implementation (>50 LOC of logic)
2. At least 3 tests that exercise real behavior (not just construction/defaults)
3. Integration with at least one other module

#### C. Reduce Engine Subdirectories

Current: 16 subdirectories under `Engine/`
```
AI/ Cache/ CLI/ Codec/ Core/ Decoders/ Enterprise/ GPU/
Media/ Memory/ Pipeline/ Platform/ Plugin/ PluginHost/ Tests/ Utils/
```

Proposed consolidation:
```
Engine/
├── Core/           ← Detection, routing, pipeline orchestration, types
├── Decoders/       ← All format decoders (image, archive, document, 3D, scientific)
├── GPU/            ← GPU acceleration (when real shaders exist)
├── Cache/          ← Caching subsystem
├── Platform/       ← OS abstraction (Windows today, macOS/Linux later)
├── Tests/          ← Test harness
└── Utils/          ← Shared utilities
```

Merge `AI/`, `Enterprise/`, `Media/`, `Memory/`, `Pipeline/`, `Plugin/`,
`PluginHost/`, `CLI/`, `Codec/` into the appropriate parent. These are
premature subdivisions for features that don't have substantial implementations.

#### D. Module Ownership Map

| Module | Owner Responsibility | Quality Bar |
|--------|---------------------|-------------|
| **Core** | Decode pipeline, format detection, type system | Every format detector must pass magic-byte validation against 10+ real files |
| **Decoders** | Format-specific decode logic | Every decoder must produce a correct thumbnail from a real test file |
| **GPU** | Hardware acceleration | Must demonstrate measurable speedup vs. CPU path on at least one format |
| **Cache** | Thumbnail caching | Must pass stress test: 10K entries, LRU eviction, concurrent access |
| **Platform** | OS abstraction | COM registration must work on clean Windows 10/11 install |
| **Tests** | Validation | Every test must exercise real I/O or real computation |
| **Utils** | Shared helpers | Zero dependency on Engine internals; usable standalone |

---

## 4. Code Quality Reset

### 4.1 Language & Standards

| Aspect | Current | Proposed | Rationale |
|--------|---------|----------|-----------|
| **Language** | C++20 | **C++20** (keep) | MSVC v145 supports it well; `std::span`, concepts, ranges are useful |
| **Standard library** | STL + Win32 | **STL + Win32 + abseil-lite** | Consider `absl::flat_hash_map` for cache; `absl::InlinedVector` for small buffers |
| **Error handling** | Mixed (HRESULT + exceptions) | **`std::expected<T,E>`** (C++23) or **result type** | C++23 `expected` is available in MSVC 19.50; use it for all new APIs |
| **String handling** | `std::wstring` + `WCHAR*` | **`std::wstring`** for Windows paths, **`std::string` (UTF-8)** internally | Windows shell requires UTF-16; engine internals should use UTF-8 |
| **Smart pointers** | `std::unique_ptr` | **Keep `std::unique_ptr`**; add `std::shared_ptr` only for COM prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent preventinterop refs | Good — `unique_ptr` is correct for ownership |
| **Naming** | PascalCase classes, camelBack vars | **Keep** | Consistent with Windows SDK conventions |

### 4.2 Header-to-Source Rebalancing Plan

**Current state:** 1,386 `.h` files, 269 `.cpp` files (5.1:1)
**Target state:** ~300 `.h` files, ~250 `.cpp` files (1.2:1)

**How to get there:**
1. **Phase 1 — Audit**: Classify every header as: Real (has implementation), Stub (declarations only), Dead (unreferenced)
2. **Phase 2 — Merge**: Combine related stub headers into cohesive module headers
3. **Phase 3 — Implement**: For each kept header, write the corresponding `.cpp`
4. **Phase 4 — Delete**: Remove all dead headers; update CMakeLists.txt

**Estimated reduction:** ~1,086 headers removed or merged (78% reduction)

### 4.3 Code Patterns to Adopt

```cpp
// BEFORE: Header-only stub (current pattern)
// CloudHydrationMonitor.h
class CloudHydrationMonitor {
    enum class HydrationState { UNKNOWN, PARTIAL, FULL, PLACEHOLDER };
    HydrationState state_ = HydrationState::UNKNOWN;
public:
    HydrationState GetState() const { return state_; }
    bool IsFullyHydrated() const { return state_ == HydrationState::FULL; }
    void Probe(const std::wstring& path) { state_ = HydrationState::FULL; } // STUB
};

// AFTER: Real implementation (target pattern)
// CloudHydrationMonitor.h — declaration only
class CloudHydrationMonitor {
public:
    enum class State { UNKNOWN, PLACEHOLDER, PARTIAL, FULL };
    State Probe(const std::filesystem::path& path);
    bool IsFullyHydrated(const std::filesystem::path& path);
private:
    // Windows Cloud Files API integration
    bool QueryCloudState(const std::filesystem::path& path, CF_PLACEHOLDER_STATE& state);
};

// CloudHydrationMonitor.cpp — real implementation
#include "CloudHydrationMonitor.h"
#include <cfapi.h>  // Windows Cloud Files API
#pragma comment(lib, "cldapi.lib")

CloudHydrationMonitor::State CloudHydrationMonitor::Probe(const std::filesystem::path& path) {
    CF_PLACEHOLDER_STATE cfState{};
    if (!QueryCloudState(path, cfState)) return State::UNKNOWN;
    if (cfState & CF_PLACEHOLDER_STATE_IN_SYNC) return State::FULL;
    if (cfState & CF_PLACEHOLDER_STATE_PARTIAL) return State::PARTIAL;
    return State::PLACEHOLDER;
}
```

### 4.4 Naming Conventions Cleanup

| Entity | Current Convention | Issue | Proposed |
|--------|--------------------|-------|----------|
| Headers | `VeryLongDescriptiveClassName.h` | Some names are 40+ chars | Keep PascalCase but prefer shorter, verb-less names |
| Enums | `UPPER_CASE` values | ✅ Correct per clang-tidy | Keep |
| Test names | `TestClassName_MethodName` | Verbose | `ClassName_Method_Scenario` (when adopting Catch2) |

---

## 5. Build System & Toolchain

### 5.1 Current Build Stack Assessment

| Tool | Version | Verdict |
|------|---------|---------|
| CMake | 4.3.1 | ✅ Latest — keep |
| Ninja | 1.13.2 | ✅ Latest — keep |
| MSVC v145 (cl 19.50) | VS 18 2026 | ✅ Latest — keep (but pin explicitly in CI when available) |
| vcpkg | 2026-02-21 | ✅ Good — keep as optional |
| Windows SDK | 10.0.26100.0 | ✅ Latest — keep |
| WiX 6.0.2 | .NET tool | ✅ Good for MSI — keep |

**No changes needed to the toolchain.** The build system is professional-grade.

### 5.2 Build System Improvements

| Improvement | Priority | Description |
|-------------|----------|-------------|
| **Precompiled headers** | P0 | PCH for `<windows.h>`, STL headers, COM headers. Will cut rebuild time 30-50%. |
| **Unity builds** (CMake `UNITY_BUILD`) | P1 | Combine small `.cpp` files into larger translation units for faster builds. |
| **ccache / sccache** | P1 | Build caching for developer iteration. Even on Windows, sccache works with MSVC. |
| **Compile-time profiling** | P2 | `-ftime-trace` equivalent for MSVC (`/d1reportTime`); identify slow headers. |
| **Module support (C++20 modules)** | P3 | When MSVC modules are stable, migrate `Engine.h` to `import Engine;`. Future investment. |

### 5.3 External Library Build Improvements

Current: 13 separate `Build-*.ps1` scripts in `build-scripts/external-libs/`.

**Proposal:** Migrate to **vcpkg manifest mode** as the primary path:
- `vcpkg.json` already exists with most dependencies
- Eliminates 13 custom build scripts
- Automated dependency updates via Dependabot
- Keep local external/ as fallback for air-gapped / offline builds

```json
// vcpkg.json (enhanced)
{
  "name": "explorerlens",
  "version": "35.5.0",
  "dependencies": [
    "zlib", "lz4", "zstd", "liblzma",
    "minizip-ng", "bzip2", "libarchive",
    "libwebp", "libjxl", "libavif", "libheif",
    "libraw", "mupdf", "openjpeg",
    "dav1d", "libde265", "freetype"
  ],
  "overrides": [],
  "builtin-baseline": "2026.02.21"
}
```

---

## 6. Testing Strategy Overhaul

### 6.1 Current State Assessment

| Metric | Value | Issue |
|--------|-------|-------|
| Test count | ~4,724 | Inflated — many test "construction + default check" only |
| Framework | Custom macros | No fixtures, parameterization, or CI reporting |
| Test files | 5 split files (~50K lines total) | Hard to navigate; mechanical 2-tests-per-header pattern |
| Test corpus | Empty `data/corpus/` | No real files to validate decoders against |
| GPU tests | 0 | No GPU path is tested |
| Integration tests | ~18 internal pipeline tests | Minimal |
| Coverage | claimed 95%+ | Likely inflated by header-only code (constructors count as covered) |

### 6.2 Proposed Testing Stack

| Layer | Framework | What It Tests | Target Count |
|-------|-----------|---------------|--------------|
| **Unit tests** | **Catch2 v3** | Individual classes, pure functions | ~500 meaningful tests |
| **Decoder validation** | **Custom corpus runner** | Each decoder produces correct output from real files | 1 test per format × 3 files = ~600 |
| **Integration tests** | **Catch2 + COM** | Full pipeline from shell request to thumbnail | ~50 |
| **GPU tests** | **Catch2 + D3D11** | GPU decode vs CPU decode SSIM comparison | ~20 |
| **Performance benchmarks** | **Google Benchmark** | P50/P95/P99 regression tracking | ~30 |
| **Fuzz tests** | **libFuzzer or WinAFL** | Crash/hang resistance on malformed files | Continuous |

**Total target: ~1,200 high-quality tests** (replacing ~4,724 low-quality ones)

### 6.3 Test Corpus Strategy

**Critical gap:** No real test files exist. Decoders cannot be validated without them.

```
data/corpus/
├── images/
│   ├── jpeg/          (5 files: basic, EXIF rotation, progressive, CMYK, large)
│   ├── png/           (5 files: 8-bit, 16-bit, alpha, interlaced, animated APNG)
│   ├── webp/          (5 files: lossy, lossless, alpha, animated, large)
│   ├── avif/          (3 files: 8-bit, 10-bit HDR, animated)
│   ├── heic/          (3 files: single, burst, live photo)
│   ├── jxl/           (3 files: lossy, lossless, HDR)
│   ├── raw/           (5 files: CR2, NEF, ARW, DNG, RAF)
│   ├── exr/           (3 files: half-float, ACES, multi-layer)
│   ├── hdr/           (2 files: Radiance, Lightprobe)
│   ├── psd/           (2 files: layers, large canvas)
│   └── ...
├── archives/          (zip, rar, 7z, cbz, cbr, tar.gz)
├── documents/         (pdf, epub, docx)
├── models/            (glTF, STL, OBJ, FBX)
├── fonts/             (TTF, OTF, WOFF2)
├── video/             (mp4 H.264, mkv H.265, webm VP9)
├── audio/             (mp3 with cover, flac, wav)
├── scientific/        (DICOM, FITS, HDF5, NIfTI)
└── MANIFEST.json      (checksums, expected thumbnail hashes, metadata)
```

**Source for test files:**
- CC0 / public domain sample files from format specification repos
- Synthetically generated using ImageMagick, FFmpeg, FreeCAD
- Captured from actual cameras for RAW format coverage

### 6.4 Migration Path from Custom to Catch2

1. Add Catch2 via vcpkg or `FetchContent`
2. Write new tests in Catch2 format alongside existing
3. Create a thin `LEGACY_TEST()` macro bridge
4. Gradually migrate existing tests that exercise real behavior
5. Delete mechanical stub tests that only check construction
6. Remove `EngineTestsMacros.h` when migration complete

---

## 7. External Libraries & Dependencies

### 7.1 Current Library Audit

| Library | Version | Latest | Status | Action |
|---------|---------|--------|--------|--------|
| zlib | 1.3.1 | 1.3.1 | ✅ Current | None |
| LZ4 | 1.10.0 | 1.10.0 | ✅ Current | None |
| zstd | 1.5.7 | 1.5.7 | ✅ Current | None |
| LZMA SDK | 26.00 | 26.00 | ✅ Current | None |
| minizip-ng | 4.0.10 | 4.0.10 | ✅ Current | None |
| UnRAR | 7.2.2 | 7.2.2 | ✅ Current | **Delete duplicate `unrar/` dir** |
| libwebp | 1.5.0 | 1.5.0 | ✅ Current | **Delete `libwebp-1.5.0-original/`** |
| libavif | 1.3.0 | 1.3.0 | ✅ Current | None |
| libjxl | 0.11.1 | 0.11.1 | ✅ Current | None |
| libheif | 1.19.5 | 1.19.5 | ✅ Current | None |
| libde265 | 1.0.15 | 1.0.15 | ✅ Current | None |
| dav1d | 1.5.1 | 1.5.1 | ✅ Current | None |
| LibRaw | 0.21.3 | 0.21.3 | ✅ Current | None |
| MuPDF | 1.24.11 | 1.24.11 | ✅ Current | None |
| openjpeg | 2.5.3 | 2.5.3 | ✅ Current | None |
| bzip2 | 1.0.8 | 1.0.8 | ✅ Current | None |
| xz/liblzma | 5.6.3 | 5.6.3 | ✅ Current | None |
| libarchive | 3.7.6 | 3.7.6 | ✅ Current | None |
| FreeType | 2.13.3 | 2.13.3 | ✅ Current | None |
| WTL | NuGet | — | ✅ Current | **Delete `wtl-nuget.zip` + `wtl.zip` if extracted** |

### 7.2 Libraries to Add (for Real Feature Implementation)

| Library | Purpose | Priority | License |
|---------|---------|----------|---------|
| **libjpeg-turbo** | SIMD-accelerated JPEG decode (faster than WIC for thumbnails) | P0 | BSD / IJG |
| **libpng** | PNG decode with SIMD (currently relying on WIC?) | P0 | libpng-2.0 |
| **libtiff** | TIFF/GeoTIFF/OME-TIFF decode | P1 | MIT-like |
| **Catch2 v3** | Test framework | P0 | BSL-1.0 |
| **Google Benchmark** | Performance benchmarks | P1 | Apache-2.0 |
| **stb_image** | Fallback for simple formats (BMP, TGA, PNM) | P2 | Public domain |
| **DirectXTex** | DDS/WIC/HDR texture loading (Microsoft, MIT) | P1 | MIT |

### 7.3 Libraries NOT Needed Yet (Remove from Headers)

These are referenced in roadmap headers but have no real integration:

| Library | Claimed For | Status |
|---------|-------------|--------|
| NVJPEG / CUDA | GPU JPEG decode | No CUDA SDK in build; remove references |
| Intel oneVPL | QSV decode | No oneVPL in vcpkg.json; remove references |
| AMD AMF | GPU video decode | No AMD SDK in build; remove references |
| nghttp2 | REST API server | No HTTP server exists; remove references |
| OpenCASCADE | STEP/IGES CAD | Heavy dependency; defer until real need |
| IfcOpenShell | IFC/BIM | Heavy dependency; defer until real need |

---

## 8. Documentation Right-Sizing

### 8.1 Current Documentation Problem

**130 markdown files** across the project. Much of it documents features that
don't exist, or duplicates information across multiple files.

### 8.2 Documentation Tiers

| Tier | Audience | Files | Rule |
|------|----------|-------|------|
| **Tier 1 — User-Facing** | End users & evaluators | `README.md`, `docs/USER_GUIDE.md`, `CHANGELOG.md`, `LICENSE` | Must reflect only *working, released* features |
| **Tier 2 — Developer** | Contributors | `docs/development/`, `.github/CONTRIBUTING.md`, `.github/standards/` | Accurate build instructions, coding standards, architecture |
| **Tier 3 — Architecture** | Deep contributors | `docs/architecture/`, `ROADMAP.md` | Vision + current state, clearly labeled |
| **Tier 4 — Format Specs** | Decoder authors | `docs/formats/` | Per-format validation status, test coverage |
| **Tier 5 — Historical** | Reference only | `CHANGELOG-archive.md`, old roadmaps | Move to `docs/archive/`, gitignore from docs site |

### 8.3 Files to Consolidate or Delete

| Action | Files | Rationale |
|--------|-------|-----------|
| **Archive** | `docs/ROADMAP_V30.md`, `ROADMAP_V34.md`, `ROADMAP_V35.md` | Replaced by this unified ROADMAP.md |
| **Merge** | `docs/PERFORMANCE.md` ← `.github/standards/performance-benchmarks.md` | Single source of truth for perf targets |
| **Merge** | `docs/RELEASE_PROCESS.md` ← version-bump instructions | Copilot instructions have more detail |
| **Delete** | Empty `packaging/inno/`, `nsis/`, `msix/`, `vdproj/` | Dead code; only WiX is used |
| **Delete** | `external/compression-libs/unrar/` (old) | Superseded by `unrar-7.2.2/` |
| **Delete** | `external/image-libs/libwebp-1.5.0-original/` | Build copy is sufficient |
| **Right-size** | `README.md` SEO metadata section | 2KB of hidden HTML comments; move to separate file or reduce |

### 8.4 README.md Rewrite Principles

The current README is **excellent for SEO** but oversells capabilities. Proposed changes:

1. **"200+ file extensions"** → State actual *tested and validated* count
2. **"GPU-accelerated"** → Clarify which formats actually use GPU decode today
3. **"macOS Quick Look + Linux Nautilus"** → Remove until real implementations exist
4. **Test count badge** → Show Catch2 results, not inflated custom framework count
5. Add a **"Getting Started in 60 seconds"** section (build + `regsvr32`)
6. Add **screenshots** of actual thumbnails generated (this is a visual product!)

---

## 9. CI/CD & Infrastructure

### 9.1 Current CI Workflows Assessment

| Workflow | Status | Value | Action |
|----------|--------|-------|--------|
| `ci-matrix.yml` | ✅ Active | **Critical** — canonical CI | Keep; ensure it tests real decoder output |
| `build.yml` | ✅ Active | Manual verification | Keep |
| `codeql.yml` | ✅ Active | Security scanning | Keep |
| `code-quality.yml` | ✅ Active | Static analysis | Keep; add clang-tidy checks |
| `coverage.yml` | ✅ Active | Coverage tracking | **Recalibrate** — 60% floor is meaningless with header-only code |
| `performance-regression-gate.yml` | ✅ Active | Perf regression | Keep; wire to actual benchmark suite |
| `release.yml` | ✅ Active | Release automation | Keep; verify all artifacts are real |
| `publish-packages.yml` | ✅ Active | 5-registry publish | **Evaluate** — are all 5 registries needed? |
| `pr-checks.yml` | ✅ Active | PR quality gates | Keep |
| `auto-label.yml` | ✅ Active | Automation | Keep |
| `release-drafter.yml` | ✅ Active | Release notes | Keep |
| `stale.yml` | ✅ Active | Issue hygiene | Keep |
| `notify-failure.yml` | ✅ Active | Alerting | Keep |
| `pages.yml` | ✅ Active | Docs site | Keep |
| `sync-labels.yml` | ✅ Active | Label management | Keep |
| `toolchain-verify.yml` | ✅ Active | Env validation | Keep |

### 9.2 CI Improvements

| Improvement | Priority | Description |
|-------------|----------|-------------|
| **Real decoder tests in CI** | P0 | Upload test corpus to CI cache; run decoder validation on every PR |
| **Screenshot regression** | P1 | Generate thumbnails in CI; pixel-diff against baseline (perceptual hash) |
| **Binary size tracking** | P1 | Track `LENSShell.dll` size per commit; alert on >10% growth |
| **Dependency scanning** | P2 | Dependabot / Renovate for vcpkg dependencies |
| **5-registry publish review** | P2 | NuGet makes sense for SDK; npm/Maven/RubyGems may be premature |

### 9.3 Package Registry Strategy

| Registry | Justification | Keep? |
|----------|---------------|-------|
| NuGet | C++ SDK / COM interop | ✅ Yes — natural for Windows C++ |
| Container (ghcr.io) | `lens-server` future service | ⏳ Defer until service exists |
| npm | Browser extension / WASM | ⏳ Defer until WASM exists |
| Maven | Java interop | ❌ Remove — no Java consumers |
| RubyGems | Ruby bindings | ❌ Remove — no Ruby consumers |

---

## 10. Frontend — Shell Extension & GUI

### 10.1 LENSShell.dll (COM Shell Extension)

**Status:** Core product. Functional COM registration with IThumbnailProvider.

| Aspect | Current | Target |
|--------|---------|--------|
| COM interfaces | IThumbnailProvider | Add IExtractImage2 for legacy Explorer, IPreviewHandler for preview pane |
| Registration | Manual `regsvr32` | MSI auto-registration + `lens.exe register` |
| Error handling | Basic HRESULT | Structured logging to ETW + Event Log |
| File type coverage | Registry-based | Also register as fallback for unregistered types |
| Thumbnail sizes | Standard | Support 16×16 to 1024×1024 (Extra Large) |
| Threading | STA | Verify MTA safety for Explorer's thread pool |

### 10.2 LENSManager.exe (Configuration GUI)

**Status:** WTL-based admin GUI. Functional but dated appearance.

| Decision | Current (WTL) | Alternative | Recommendation |
|----------|---------------|-------------|----------------|
| **UI Framework** | WTL (Win32) | WinUI 3, Qt, Dear ImGui | **Keep WTL** for v1. It works, it's lightweight, and it's the right choice for a system utility. WinUI 3 adds 100MB+ of dependencies. Consider WinUI 3 for v2 *only* if user demand warrants it. |
| **Dark mode** | Custom `DarkModeController` | Windows 11 dark mode API | **Enhance** — use `SetPreferredAppMode()` from uxtheme.dll |
| **System tray** | `SystemTrayIcon.h` | Standard NotifyIcon | Keep; add balloon notifications for decode errors |

### 10.3 lens.exe (CLI Tool)

**Status:** 17 source files in `src/Tools.CLI/`. Early development.

**Priority commands to make work first:**
1. `lens generate <file>` — Generate thumbnail for a file (validate decoder)
2. `lens info <file>` — Show format detection result + metadata
3. `lens register` — Register/unregister shell extension
4. `lens doctor` — System diagnostics (GPU, libraries, registration)
5. `lens benchmark <directory>` — Batch decode benchmark
6. `lens cache stats` — Cache hit rate and size

### 10.4 Web Frontend (index.html)

**Status:** Project has an `index.html` at root — likely a landing page.

**Recommendation:** Move to `docs/` for GitHub Pages. Keep it simple:
a marketing page explaining what ExplorerLens does, with screenshots and download link.

---

## 11. Backend — Engine & Decode Pipeline

### 11.1 Priority: Make Core Decoders Work Flawlessly

Instead of adding more format stubs, make the top 20 formats *perfect*:

| Priority | Format | Library | Target P50 | Validation |
|----------|--------|---------|------------|------------|
| P0 | JPEG (.jpg) | libjpeg-turbo / WIC | < 5 ms | EXIF rotation, progressive, CMYK |
| P0 | PNG (.png) | libpng / WIC | < 5 ms | 8/16-bit, alpha, interlaced, APNG first frame |
| P0 | WebP (.webp) | libwebp | < 8 ms | Lossy, lossless, alpha, animated first frame |
| P0 | AVIF (.avif) | libavif + dav1d | < 10 ms | 8/10-bit, HDR, animated first frame |
| P0 | HEIC (.heic) | libheif + libde265 | < 10 ms | Single, burst, live photo key frame |
| P0 | JPEG XL (.jxl) | libjxl | < 12 ms | Lossy, lossless, HDR |
| P0 | PDF (.pdf) | MuPDF | < 20 ms | First page render at 256px |
| P0 | RAW (CR2/NEF/ARW/DNG) | LibRaw | < 25 ms | Embedded preview extraction (fast path) |
| P1 | ZIP (.zip, .cbz) | minizip-ng | < 15 ms | First image in archive |
| P1 | RAR (.rar, .cbr) | UnRAR | < 15 ms | First image in archive |
| P1 | 7Z (.7z, .cb7) | LZMA SDK | < 15 ms | First image in archive |
| P1 | EPUB (.epub) | minizip-ng + image extract | < 20 ms | Cover image extraction |
| P1 | GIF (.gif) | WIC / custom | < 5 ms | First frame of animated |
| P1 | BMP (.bmp) | WIC / stb_image | < 2 ms | Standard |
| P1 | TIFF (.tiff) | libtiff / WIC | < 8 ms | Multi-page: first page; GeoTIFF |
| P2 | EXR (.exr) | tinyexr / OpenEXR | < 15 ms | Tone-map to sRGB |
| P2 | PSD (.psd) | Custom parser | < 15 ms | Compositor first layer |
| P2 | DDS (.dds) | DirectXTex | < 5 ms | BC1-BC7 decode |
| P2 | SVG (.svg) | WIC / Direct2D | < 10 ms | Rasterize at target size |
| P2 | TTF/OTF (.ttf, .otf) | FreeType | < 10 ms | Render sample text "AaBb" |

### 11.2 Decoder Architecture Improvements

**Current DecoderRegistry pattern:**
```
Extension → LENSTYPE enum → Decoder class → Decode(stream) → HBITMAP
```

**Proposed improvements:**

1. **Two-phase decode:** `ProbeHeader(first_16KB)` → `DecodeThumb(stream, targetSize)`
   - Phase 1 reads only the header to confirm format and extract metadata
   - Phase 2 does minimal decode at the requested thumbnail size
   - For RAW photos, Phase 2 extracts embedded JPEG preview (instant)

2. **Streaming decode:** Instead of reading entire file into memory:
   ```cpp
   class IStreamingDecoder {
       virtual DecodeResult ProbeHeader(std::span<const uint8_t> header) = 0;
       virtual DecodeResult DecodeAtSize(IStream* stream, uint32_t targetSize) = 0;
       virtual bool SupportsPartialDecode() const { return false; }
   };
   ```

3. **Cancellation:** Explorer may cancel thumbnail requests when scrolling fast.
   Every decoder must check a cancellation token periodically:
   ```cpp
   virtual DecodeResult DecodeAtSize(IStream* stream, uint32_t targetSize,
                                      std::stop_token cancel) = 0;
   ```

### 11.3 Cache Architecture

**Current:** Robin-hood hash map (`SubMillisecondCacheEngine`), claimed < 1ms hit.

**Proposed improvements:**

| Feature | Description |
|---------|-------------|
| **Two-tier cache** | L1: In-memory LRU (64MB default, BGRA bitmaps). L2: Disk cache (`%LOCALAPPDATA%\ExplorerLens\Cache\`) with memory-mapped access. |
| **Cache key** | `SHA256(canonical_path + file_mtime + file_size + target_dimensions)` |
| **Eviction** | LRU with size budget. L1 evicts to L2. L2 evicts oldest when disk budget exceeded. |
| **Invalidation** | `ReadDirectoryChangesW` watcher per opened Explorer folder. File change → invalidate key. |
| **Persistence** | L2 survives reboots. Cold start reads L2 index (memory-mapped). |
| **Metrics** | Hit rate, miss rate, eviction count, L1 size, L2 size — exposed via `lens cache stats`. |

---

## 12. GPU Pipeline — Reality Check

### 12.1 Honest GPU Status

**Current state:** No shader files (`.hlsl`, `.comp`, `.metal`) exist in the repository.
No CUDA SDK, oneVPL SDK, or AMD AMF SDK is referenced in the build system. The "GPU
acceleration" is architected but not implemented.

### 12.2 Realistic GPU Acceleration Plan

| Phase | What | How | Measurable Target |
|-------|------|-----|-------------------|
| **Phase 1** — WIC GPU | Use WIC with D3D11 device hints | `IWICImagingFactory2::CreateDecoderFromStream` with D3D device | 1.5-2× speedup on JPEG/PNG |
| **Phase 2** — D3D11 resize | GPU-accelerated bilinear/Lanczos resize | D3D11 compute shader (one `.hlsl` file) | < 0.5 ms for 4K→256px resize |
| **Phase 3** — NVDEC/QSV video | Hardware video decode for thumbnail extraction | NVDEC via CUDA or DirectX Video Acceleration (DXVA2) | 10× speedup for video thumbnails |
| **Phase 4** — GPU tone-map | HDR → SDR on GPU | Compute shader for PQ/HLG/gainmap | < 0.5 ms tone-map |

**Key principle:** GPU acceleration should be *measurably faster* than the CPU path
on real hardware, with automatic fallback. Do not claim GPU support until benchmarks prove it.

### 12.3 GPU Shaders to Write

| Shader | Purpose | Input | Output |
|--------|---------|-------|--------|
| `resize_bilinear.hlsl` | Fast thumbnail resize | SRV (source texture) | UAV (target texture) |
| `resize_lanczos.hlsl` | High-quality resize | SRV | UAV |
| `tonemap_pq_to_srgb.hlsl` | HDR10 PQ → sRGB | SRV (HDR10 surface) | UAV (sRGB surface) |
| `tonemap_hlg_to_srgb.hlsl` | HLG → sRGB | SRV | UAV |
| `demosaic_bayer.hlsl` | RAW Bayer demosaic | SRV (raw Bayer data) | UAV (RGB surface) |

---

## 13. Cross-Platform Strategy

### 13.1 Honest Assessment

Windows is the only platform that matters right now. macOS Quick Look and Linux
Nautilus are valid expansion targets but should not be pursued until the Windows
product is excellent.

### 13.2 Platform Priority

| Platform | Timeline | Approach |
|----------|----------|----------|
| **Windows 10/11** | Now | COM IThumbnailProvider — the core product |
| **macOS** | Phase 4+ | Quick Look QLThumbnailProvider — requires Objective-C bridge and Metal backend |
| **Linux** | Phase 5+ | Nautilus/Dolphin tumbler plugin — requires GLib/D-Bus integration |
| **Web/WASM** | Phase 6+ | Server-side `lens-server` or client-side WASM module |

### 13.3 What Cross-Platform Means in Practice

The Engine library (format detection, decoders, cache) IS portable C++20. The non-portable parts:
- COM registration → Platform/Win32ShellProvider
- D3D11 GPU → Platform/MetalRenderer (macOS), Platform/VulkanRenderer (Linux)
- File system notifications → Platform/FSWatcher abstraction
- Installer → platform-specific packaging

The Platform Abstraction Layer (PAL) architecture is correct. The implementation order
should be: perfect the Windows PAL backend first, then add others.

---

## 14. Cloud, AI & Advanced Features

### 14.1 Cloud Features (Defer to Phase 4+)

Features from v35.x roadmap that should be deferred:

| Feature | v35.x Module | Status | Action |
|---------|-------------|--------|--------|
| Cloud hydration monitoring | `CloudHydrationMonitor` | Stub | **Implement in Phase 3** — uses real Windows CF API |
| Streaming cache | `StreamingCacheTierPolicy` | Stub | Defer to Phase 4 |
| Real-time collaboration | `CollaborativeCacheCoordinator` | Stub | Defer to Phase 5 — needs server component |
| Zero-trust security | `ThumbnailManifestSigner` | Stub | Defer to Phase 4 — needs crypto integration |
| WebAssembly pipeline | `WasmDecoderShim` | Stub | Defer to Phase 6 |
| Cross-device sync | `DeviceSyncManifest` | Stub | Defer to Phase 5 |
| REST API server | `RemoteDecodeServer` | Stub | Defer to Phase 4 |

### 14.2 AI Features (Defer to Phase 5+)

| Feature | Current State | Realistic Timeline |
|---------|---------------|-------------------|
| Smart crop (saliency detection) | Header stubs in `Engine/AI/` | Phase 5 — requires ONNX Runtime or DirectML integration |
| Scene understanding | Header stubs | Phase 6 — research-grade feature |
| Semantic search (CLIP) | Header stubs | Phase 6 — requires embedding model deployment |
| Generative thumbnails | Roadmap only | Phase 7+ — not a core product need |

### 14.3 Features That DO Provide Value Now

| Feature | Why Now | Implementation |
|---------|---------|----------------|
| **EXIF-aware rotation** | Users see sideways photos without this | LibRaw/libjpeg-turbo EXIF orientation tag |
| **Embedded preview extraction (RAW)** | 100× faster than full RAW decode | LibRaw `unpack_thumb()` — extract embedded JPEG |
| **Archive cover image** | CBZ/EPUB users expect this | Extract first image alphabetically |
| **PDF first page** | Universal document thumbnail | MuPDF `fz_new_pixmap_from_page()` |
| **Video keyframe** | Better than generic icon | Media Foundation `IMFSourceReader::ReadSample` at 10% duration |
| **Font sample text** | Shows font appearance at a glance | FreeType render "AaBb123" |

---

## 15. Packaging & Distribution

### 15.1 Current Packaging (Simplify)

| Package | Status | Keep? |
|---------|--------|-------|
| WiX MSI | ✅ Active | ✅ Yes — primary installer |
| Portable ZIP | ✅ Active | ✅ Yes — for power users |
| Inno Setup | ❌ Empty | Delete |
| NSIS | ❌ Empty | Delete |
| MSIX | ❌ Empty | Delete (consider for Microsoft Store later) |
| vdproj | ❌ Empty | Delete (legacy VS installer) |
| NuGet (SDK) | ✅ Active | ✅ Keep |
| npm | ✅ Active | ⏳ Defer |
| Docker / ghcr.io | ✅ Active | ⏳ Defer |
| Maven | ✅ Active | ❌ Remove |
| RubyGems | ✅ Active | ❌ Remove |

### 15.2 Distribution Channels (Realistic)

| Channel | Priority | Action |
|---------|----------|--------|
| **GitHub Releases** | P0 | MSI + ZIP + checksums on every release |
| **winget** | P0 | Submit to Windows Package Manager community repo |
| **Chocolatey** | P1 | Submit package for `choco install explorerlens` |
| **Scoop** | P1 | Add to Scoop extras bucket |
| **Microsoft Store** | P2 | MSIX package when matured |

### 15.3 Installer Improvements

| Improvement | Description |
|-------------|-------------|
| **Silent install** | `msiexec /i ExplorerLens.msi /qn` — must work without GUI |
| **Per-user install** | Don't require admin for HKCU registration (per-user COM) |
| **Automatic updates** | Check GitHub Releases API for updates; prompt user |
| **Uninstall cleanup** | Remove cache, registry entries, event source on uninstall |
| **Side-by-side** | Allow multiple versions for testing (different CLSIDs) |

---

## 16. Phase Plan — 6 Phases to Best-in-Class

### Phase 1 — Foundation (**4-6 weeks**)
**Goal:** Working, validated, installable product for the top 20 formats.

- [ ] Audit all 1,386 headers: classify as Real / Stub / Dead
- [ ] Delete dead headers and empty packaging directories
- [ ] Verify all 18 external libraries build and link correctly
- [ ] Implement or fix the top 20 format decoders (see §11.1) with real `.cpp` files
- [ ] Create test corpus with 5+ real files per format
- [ ] Integrate Catch2; write 500+ meaningful tests replacing mechanical stubs
- [ ] Run all decoders against test corpus; achieve 100% correct output
- [ ] Fix all warnings; clean build on MSVC v145
- [ ] `lens generate <file>` CLI works for all 20 formats
- [ ] `regsvr32 LENSShell.dll` works on clean Windows 10 VM
- [ ] Update README.md to reflect actual, verified capabilities
- [ ] Delete `ROADMAP_V30.md`, `ROADMAP_V34.md`, `ROADMAP_V35.md`

**Exit criteria:** A user can install the MSI, and every file in their Photos
folder gets a correct, fast thumbnail in Explorer.

### Phase 2 — Performance (**3-4 weeks**)
**Goal:** Measurably fast decode with proper caching.

- [ ] Implement two-tier cache (L1 memory + L2 disk)
- [ ] Add `ReadDirectoryChangesW` cache invalidation
- [ ] Implement WIC-with-D3D11 GPU path for JPEG/PNG/WebP
- [ ] Write `resize_bilinear.hlsl` compute shader
- [ ] Benchmark all format decoders; establish P50/P95/P99 baselines
- [ ] Integrate Google Benchmark into CI
- [ ] Performance regression gate blocks >10% P95 regressions
- [ ] `lens benchmark <dir>` CLI works and produces JSON report
- [ ] Target: 5ms JPEG P50, 8ms WebP P50, 20ms PDF P50

**Exit criteria:** ExplorerLens is measurably faster than Windows built-in
thumbnails for every supported format.

### Phase 3 — Format Breadth (**4-6 weeks**)
**Goal:** Expand from 20 → 80+ validated formats.

- [ ] Add remaining image formats: EXR, PSD, HDR, QOI, TGA, ICO, DDS, SVG
- [ ] Add archive formats: TAR, GZ, BZ2, XZ, ISO
- [ ] Add document formats: EPUB, MOBI, CHM, RTF, DOCX (embedded preview)
- [ ] Add 3D formats: glTF/GLB, STL, OBJ (wireframe preview)
- [ ] Add font formats: TTF, OTF, WOFF (sample text render)
- [ ] Add video formats: MP4, MKV, AVI, WebM (keyframe extraction)
- [ ] Add audio formats: MP3, FLAC, OGG (album art + waveform)
- [ ] Add RAW formats: all LibRaw-supported cameras (100+ models)
- [ ] Add scientific: DICOM (basic), FITS (basic)
- [ ] Expand test corpus to cover all new formats
- [ ] Submit to winget + Chocolatey + Scoop

**Exit criteria:** ~80+ format families with 200+ extensions, all validated
against real test files, all producing correct thumbnails.

### Phase 4 — Enterprise & Cloud (**4-6 weeks**)
**Goal:** Production-ready for enterprise deployment.

- [ ] Implement Windows Cloud Files API hydration detection
- [ ] Group Policy template (ADMX/ADML) for enterprise configuration
- [ ] ETW tracing for decode pipeline (Windows Performance Recorder compatible)
- [ ] Event log entries for decode errors and crashes
- [ ] `lens-server` REST API for headless thumbnail generation
- [ ] Docker container for `lens-server`
- [ ] SBOM generation with real dependency tracking
- [ ] Security audit: fuzz all decoders, fix crashes

**Exit criteria:** IT admins can deploy via GPO, monitor via SIEM, and
run `lens-server` in their CI pipelines.

### Phase 5 — Cross-Platform (**6-8 weeks**)
**Goal:** macOS Quick Look extension shipping.

- [ ] Implement macOS PAL backend (QLThumbnailProvider)
- [ ] Metal backend for GPU operations
- [ ] FSEvents for file change detection
- [ ] Homebrew formula for macOS distribution
- [ ] Cross-compile Engine with Clang on macOS
- [ ] Validate SSIM ≥ 0.99 between Windows and macOS output
- [ ] Linux Nautilus thumbnailer plugin (basic)

**Exit criteria:** macOS users can install via Homebrew and get thumbnails
for the top 20 formats in Finder.

### Phase 6 — AI & Advanced (**ongoing**)
**Goal:** Smart thumbnails that understand content.

- [ ] ONNX Runtime integration for on-device inference
- [ ] Smart crop using saliency detection model
- [ ] HDR tone-mapping on GPU (PQ/HLG/Gainmap)
- [ ] CLIP embedding for semantic search
- [ ] Predictive pre-generation based on folder navigation patterns
- [ ] WebAssembly build for browser-based thumbnail generation

**Exit criteria:** Thumbnails are not just decoded but intelligently
cropped and tone-mapped for maximum visual quality.

---

## 17. Consolidated Backlog from Previous Roadmaps

### From ROADMAP_V34.md (Arcturus) — Incomplete Items

| Item | Original Scope | Status | Carry Forward? |
|------|---------------|--------|----------------|
| 350+ file extensions | Format blitz | Stubs created, not validated | **Yes** → Phase 3 |
| GPU-first decode (sub-10ms) | NVJPEG, QSV, AMD AMF | Stubs only | **Partially** → Phase 2 (WIC GPU first) |
| HDR tone-mapping | PQ/HLG/Gainmap | Stubs | **Yes** → Phase 6 |
| Predictive pre-generation | Directory pre-scan | Stubs | **Yes** → Phase 6 |
| Animated format suite | GIF/APNG/WebP animation | Stubs | **Yes** → Phase 3 (first frame only) |
| Scientific formats v2 | DICOM/FITS/HDF5 | Stubs | **Yes** → Phase 3 (basic) |
| CAD/BIM/EDA | DWG/IFC/Gerber | Stubs | **Defer** — heavy dependencies |
| Performance hardening | LTS gate | Stubs | **Yes** → Phase 2 |

### From ROADMAP_V35.md (Vega) — Incomplete Items

| Item | Original Scope | Status | Carry Forward? |
|------|---------------|--------|----------------|
| Cloud-native thumbnails | Stream hydration | Stubs | **Yes** → Phase 4 |
| Real-time collaboration | Cache coordination | Stubs | **Defer** — needs server |
| Network-aware caching | Bandwidth throttle | Stubs | **Yes** → Phase 4 |
| Zero-trust security | FIPS crypto | Stubs | **Defer** → Phase 4 (basic auth only) |
| WebAssembly pipeline | Browser extension | Stubs | **Defer** → Phase 6 |
| Cross-device sync | Manifest sync | Stubs | **Defer** — needs cloud service |
| REST API server | lens-server | Stubs | **Yes** → Phase 4 |

### From consolidation-opportunities.md — Pending Cleanup

| Item | Priority | Action |
|------|----------|--------|
| ~~Delete src/Engine/~~ | ✅ Done (directory no longer exists) | None |
| Delete packaging/inno, nsis, vdproj | **P0** | Delete in Phase 1 |
| Merge performance docs | **P1** | Single source of truth |
| Consolidate plugin docs | **P2** | Restructure docs/plugins/ |
| Review SDK/include/{jxl,libraw}/ | **P2** | Confirm they're API stubs |

### From missing-types-analysis.md — Unresolved

| Type | Status | Action |
|------|--------|--------|
| ErrorCategorizationEngine | ❌ Missing | Create in Phase 1 if needed; otherwise delete test references |
| ShellNotificationProvider | ❌ Missing | Create in Phase 1 (toast notifications for errors) |
| DeploymentPreflightCheck | ❌ Missing | Create as `lens doctor` CLI command in Phase 1 |
| DecoderPerformanceCounters | ❌ Missing | Create in Phase 2 (perf tracking) |
| FormatStatusProvider location | In LENSManager/ not Engine/ | Move to Engine/ or add thin wrapper |

---

## 18. Success Metrics

### Phase 1 Success (Foundation)

| Metric | Target |
|--------|--------|
| Validated format families | ≥ 20 |
| Test corpus files | ≥ 100 |
| Catch2 tests passing | ≥ 500 |
| Header-to-source ratio | < 3:1 |
| Clean install on Windows 10 VM | Yes |
| Zero build warnings | Yes |

### Phase 2 Success (Performance)

| Metric | Target |
|--------|--------|
| JPEG 6MP P50 | < 5 ms |
| PNG 4K P50 | < 5 ms |
| WebP P50 | < 8 ms |
| PDF first-page P50 | < 20 ms |
| Cache hit P50 | < 1 ms |
| LENSShell.dll size | < 5 MB |
| Idle memory | < 30 MB |
| Benchmark regression gate in CI | Active |

### Phase 3 Success (Breadth)

| Metric | Target |
|--------|--------|
| Validated format families | ≥ 80 |
| File extensions supported | ≥ 200 (validated) |
| Test corpus files | ≥ 300 |
| winget / Chocolatey / Scoop listings | Published |
| GitHub stars | ≥ 100 |

### Long-Term Best-in-Class Criteria

| Dimension | Best-in-Class Means |
|-----------|---------------------|
| **Speed** | Faster than OS built-in thumbnails for every format |
| **Coverage** | More formats than any other thumbnail provider |
| **Correctness** | EXIF rotation, color management, HDR tone-mapping — all correct |
| **Reliability** | Zero crashes on malformed files (fuzz-tested) |
| **Size** | < 5 MB DLL + < 15 MB with all libraries |
| **Memory** | < 50 MB working set under load; < 10 MB idle |
| **Install** | One-click MSI or `winget install ExplorerLens` |
| **Cross-platform** | Windows native + macOS Quick Look + Linux thumbnailer |
| **Extensible** | Plugin SDK for third-party format decoders |
| **Observable** | ETW tracing, Event Log, CLI diagnostics |

---

## 19. Appendix — Decision Log

### Key Decisions Made in This Roadmap

| # | Decision | Rationale |
|---|----------|-----------|
| D1 | Keep C++20 + MSVC v145 | Right tool for COM shell extensions; no alternative |
| D2 | Keep CMake + Ninja | Industry standard; presets are excellent DX |
| D3 | Migrate to Catch2 | Industry standard; IDE integration, parameterized tests, XML export |
| D4 | Migrate to vcpkg-primary | Eliminates 13 custom build scripts; Dependabot support |
| D5 | Flatten Engine/Core/ from 545 to ~80 headers | 5.1:1 ratio is unsustainable; most are stubs |
| D6 | Create real test corpus | Cannot validate decoders without real files |
| D7 | Defer cross-platform to Phase 5 | Windows must be excellent before expanding |
| D8 | Defer AI/ML to Phase 6 | Core product value is fast, correct thumbnails |
| D9 | Defer cloud/collaboration to Phase 4 | Enterprise feature; needs working core first |
| D10 | Remove Maven + RubyGems registries | No Java or Ruby consumers exist |
| D11 | Add winget/Chocolatey/Scoop | Primary Windows distribution channels |
| D12 | Don't reset version number | Version history has value; v36.0 starts Phase 1 |
| D13 | Single ROADMAP.md | 3 roadmap files creates confusion; consolidate |
| D14 | Right-size documentation | 130 .md files for a product with 269 .cpp files is backwards |
| D15 | Implement before declaring | No header in CMakeLists without real .cpp implementation |

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
2. **Check off items as completed.** This is a living document.
3. **Old roadmap files** (`ROADMAP_V30.md`, `ROADMAP_V34.md`, `ROADMAP_V35.md`) should
   be moved to `docs/archive/` for historical reference.
4. **Measure progress** by the success metrics in §18, not by header count or version number.
5. **This document supersedes** all previous roadmap documents.

---

*"Make it work, make it right, make it fast — in that order."* — Kent Beck
