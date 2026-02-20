# DarkThumbs — Copilot Instructions

## Project Overview

DarkThumbs is a **Windows Shell Extension** (IThumbnailProvider COM DLL) that generates
GPU-accelerated thumbnails for 200+ file formats across 25 specialized decoders.

- **Version:** 13.0.0
- **Language:** C++20 (MSVC v145 toolset, Visual Studio 18 2026)
- **Build System:** CMake 3.20+ (Engine) + MSBuild (Shell/Manager)
- **GPU:** DirectX 11 + DirectX 12 + Vulkan Compute with CPU fallback
- **COM CLSID:** `9E6ECB90-5A61-42BD-B851-D3297D9C7F39`
- **Sprint Count:** 298 completed (v13.0.0 block: Sprints 249–298 ✅)
- **Build Status:** 0 errors, 0 warnings

## Architecture

```
CBXShell.dll (2940 KB)     — COM Shell Extension (IThumbnailProvider)
CBXManager.exe (400 KB)    — GUI Configuration Utility
DarkThumbsEngine.lib       — Core decode + render pipeline
```

### Key Directories

| Directory                      | Purpose                                                           |
| ------------------------------ | ----------------------------------------------------------------- |
| `CBXShell/`                    | Shell extension DLL (COM registration, thumbnail provider)        |
| `CBXManager/`                  | WTL-based admin GUI for registration/settings                     |
| `Engine/`                      | Core library — decoders, GPU pipeline, caching, observability     |
| `Engine/Core/`                 | Decode pipeline, GPU renderer, resource management                |
| `Engine/Decoders/`             | Format-specific decoders (25+ total, incl. CAD/glTF/Scientific)   |
| `Engine/Plugin/`               | Plugin ecosystem (trust chain, sandbox, compat kit, ref pack)     |
| `Engine/Memory/`               | Memory management (compactor, hot-mode, pressure controller)      |
| `Engine/Pipeline/`             | Pipeline stages (fallback engine, zero-copy upload)               |
| `Engine/Cache/`                | Cache management (adaptive budget manager)                        |
| `Engine/Utils/`                | Utilities (ARM64 support, matrix validation, installer lifecycle) |
| `Engine/Tests/`                | GTest unit tests + Google Benchmark                               |
| `build-scripts/`               | PowerShell build automation                                       |
| `build-scripts/core/`          | Build-Library-Core.ps1 — unified build module                     |
| `build-scripts/external-libs/` | Per-library build scripts (zlib, LZ4, zstd, etc.)                 |
| `cmake/`                       | CMake toolchain files (incl. toolchain-windows-arm64.cmake)       |
| `packaging/`                   | MSI (WiX), Inno Setup, MSIX manifests                             |
| `SDK/`                         | Plugin SDK (C ABI, plugin_api.h)                                  |
| `docs/`                        | All documentation                                                 |
| `docs/development/sprints-v8/` | Per-sprint markdown docs (SPRINT_1.md … SPRINT_298.md)            |
| `.github/workflows/`           | CI/CD pipelines (incl. arm64.yml)                                 |

## Build Commands

```powershell
# CMake + Ninja (recommended, ~55 sec)
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j 8

# MSBuild (traditional, ~75 sec)
msbuild CBXShell.sln /p:Configuration=Release /p:Platform=x64 /m

# Run tests
ctest --test-dir build -C Release --output-on-failure

# Production build (all libraries + packaging)
.\build-scripts\Build-All-And-Package.ps1
```

## External Libraries (Statically Linked)

| Library    | Version | Purpose                   |
| ---------- | ------- | ------------------------- |
| zlib       | 1.3.1   | ZIP/deflate compression   |
| LZ4        | 1.10.0  | Fast compression          |
| zstd       | 1.5.7   | Zstandard compression     |
| LZMA SDK   | 26.00   | 7z archives               |
| minizip-ng | 4.0.10  | ZIP archive handling      |
| UnRAR      | 7.2.2   | RAR archive extraction    |
| libwebp    | 1.5.0   | WebP images               |
| libavif    | 1.3.0   | AVIF images               |
| libjxl     | 0.11.1  | JPEG XL images            |
| libheif    | 1.19.5  | HEIF/HEIC images          |
| libde265   | 1.0.15  | HEVC decoding for libheif |
| LibRaw     | 0.21.3  | RAW camera formats        |

## Code Conventions

- **C++20** standard, all headers use `#pragma once`
- Classes: `PascalCase`, functions: `PascalCase`, variables: `camelBack`
- Private members: `m_` prefix, constants: `UPPER_CASE`
- **Zero warnings policy** — build must produce 0 warnings
- See `.clang-tidy` for static analysis config
- See `docs/development/CODE_QUALITY_STANDARDS.md` for full guide
- COM interfaces follow Windows SDK patterns (IUnknown, AddRef/Release)

## Key Types

- `cbxArchive` — Main archive handler, routes formats to decoders
- `CBXTYPE` — Enum of supported archive/format types (cbxArchive.h)
- `IThumbnailProvider` — Windows COM interface implemented by CBXShell
- `DarkThumbsEngine` — Core engine library linking all decoders
- `ObservabilityIntegration` — ETW + structured logger singleton
- `BuildValidation` — Compile-time version/feature flag validation

## Testing

- **Framework:** Custom macros `TEST(name)`, `RUN_TEST(name)`, `ASSERT(cond)` with counters — NOT GTest
- **Test count:** ~937 unit tests (100 original + 337 Sprints 150–174 + 250 Sprints 199–248 + 250 Sprints 249–298), 5 benchmarks
- **Pass rate:** 100%
- **Performance targets:** 17ms single thumbnail, 235 img/sec batch, <5ms cache hit

## Important Rules for AI Assistants

1. **Never break the zero-warnings build** — all changes must compile cleanly
2. **New headers must be registered** in `Engine/CMakeLists.txt` ENGINE_HEADERS
3. **New source files must be registered** in ENGINE_SOURCES
4. **New test files must be registered** in `Engine/Tests/CMakeLists.txt` EngineTests target
5. **COM CLSID is fixed** — do not change `9E6ECB90-5A61-42BD-B851-D3297D9C7F39`
6. **CBXTYPE enum values must not collide** — check existing values in cbxArchive.h
7. **Build scripts use Build-Library-Core.ps1** — don't duplicate utility functions
8. **Documentation versions must stay in sync** — update all docs when version changes
9. **Git commits must have descriptive messages** — include sprint number if applicable
10. **Test changes with:** `cmake --build build --config Release -j 8`
11. **Validate with:** `ctest --test-dir build -C Release --output-on-failure`

## External Libraries Directory Structure (Post-Cleanup)

```
external/
  compression-libs/   — zlib, lz4, zstd, minizip-ng, lzma, unrar, bzip2, libarchive, xz
  image-libs/          — libwebp, libjxl, libavif, libheif, libde265, dav1d
  camera-libs/         — libraw, libraw-install
  pdf-libs/            — mupdf
  ui-libs/             — wtl
```

> **Note:** Build scripts use `$rootDir = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)` to resolve the project root. All scripts import `Build-Library-Core.ps1` from `build-scripts/core/`.

## .gitignore Considerations

- The `Release/` pattern in `.gitignore` blocks `Engine/Release/` — use `git add -f` for files there
- Stale `CMakeCache.txt` files from directory renames are auto-detected by `Build-Library-Core.ps1`

## Sprint Execution Guidance (v13.0+)

- **Current version:** v13.0.0 (Sprints 249-298 complete)
- **Next roadmap block:** Sprints 299+ (next improvement plan TBD)
- **Execution package docs:** `docs/development/sprints-v8/SPRINT_XX.md`
- **Source of truth:** `MASTER_PLAN.md`
- **Per sprint commit policy:** one clear commit per sprint with objective + impacted areas
- **Sprint deliverables pattern:** header in `Engine/`, test in `Engine/Tests/EngineTests.cpp`, doc in `docs/development/sprints-v8/`, CMakeLists.txt registration (BOTH `Engine/CMakeLists.txt` ENGINE_HEADERS/ENGINE_SOURCES), git commit
- **Batch pattern:** Create 5 sprints' source files → register in CMakeLists.txt (multi-replace) → add includes + TEST() + RUN_TEST() to EngineTests.cpp → create sprint docs → git commit each individually
- **CMakeLists.txt insertion points:** Core headers before `# Pipeline`, Core sources before `# Pipeline implementations`, Utils headers before `# Sprint 8-12:`, Utils sources before closing `)`
- **EngineTests.cpp insertion points:** New includes after last sprint include, TEST() functions before `//== Sprint 6:` section, RUN_TEST() calls before `// Sprint 6: Isolation & Stability Tests`

## v10.5.0 Block Summary (Sprints 199–248 ✅)

| Phase | Sprints | Title                                                                                             |
| ----- | ------- | ------------------------------------------------------------------------------------------------- |
| P1    | 199–204 | GPU Pipeline V2, D3D12 Compute, Shader Compiler, Pipeline Cache, GPU Memory Pool, Release Gate V5 |
| P2    | 205–209 | Async Decode, Thread Pool V2, Priority Queue, Decode Cache V2, Release Gate V6                    |
| P3    | 210–214 | Format Detection V2, MIME Resolver, Codec Registry, Stream Analyzer, Release Gate V7              |
| P4    | 215–219 | ETW Provider V2, Perf Counters, Diagnostic Logger, Health Monitor, Release Gate V8                |
| P5    | 220–224 | Accessibility, Cloud Sync, Format Converter, Enterprise Deploy, Release Gate V11                  |
| P6    | 225–229 | Watch Folder, Diagnostics Dashboard, Benchmark V2, Localization, Theme Engine                     |
| P7    | 230–234 | Telemetry, Update Engine, Shell Preview, Batch Processing, Release Gate V12                       |
| P8    | 235–239 | File Hash, Registry Manager, Error Recovery, Log Rotation, Release Gate V13                       |
| P9    | 240–244 | Resource Pool, CLI Parser, Metadata Extractor, Notifications, Release Gate V14                    |
| P10   | 245–248 | Content Indexer, Network Diagnostics, Config Migration, Release Gate V15 (milestone)              |

## v8.4.0 Block Summary (Sprints 175–177 ✅)

| Sprint | Title                        | Key Changes                                                                         |
| ------ | ---------------------------- | ----------------------------------------------------------------------------------- |
| 175    | Critical Bug Fixes           | Fixed djvu→CBXTYPE_DJVU, added model routing, AVIF/HEIF separation, HEIF extensions |
| 176    | Shell Registration Expansion | CBXShell.rgs 47→93 extensions (archives, RAW, docs, models, fonts)                  |
| 177    | Version Normalization        | All docs updated to v8.4.0, CHANGELOG v8.0-8.4 entries added                        |

## v8.3.0 Block Summary (Sprints 150–174 ✅)

| Phase | Sprints | Title                                                                                       |
| ----- | ------- | ------------------------------------------------------------------------------------------- |
| P1    | 150–154 | Plugin Ecosystem Hardening (sandbox, trust chain, compat kit, ref pack)                     |
| P2    | 155–159 | ARM64 Foundation (build config, lib matrix, runtime validator, CI)                          |
| P3    | 160–164 | Format Expansion (JPEG2000, CAD/glTF/Scientific, fallback engine)                           |
| P4    | 165–169 | Memory Excellence (compactor, zero-copy, adaptive cache, hot-mode, pressure V2)             |
| P5    | 170–174 | v8.3.0 Release (matrix validation, installer lifecycle, release gate V2, doc sync, closure) |

## New Patterns Discovered in v8.3.0 Sprints

- **ARM64 cross-compile:** Use `cmake/toolchain-windows-arm64.cmake` with MSVC `amd64_arm64` arch; CI in `.github/workflows/arm64.yml`
- **Plugin architecture:** Plugin files go in `Engine/Plugin/`; use `ICADDecoderPlugin` / `IThumbnailPlugin` patterns from Sprint 161/153
- **Memory pressure ladder:** 5-tier (None/Low/Medium/High/Critical) with bitmask `PressureAction` flags — see `MemoryPressureControllerV2.h`
- **Format fallback:** `FormatFallbackEngine` routes via `FallbackTrigger` bitmask flags; default chains in `CreateDefault()`
- **Zero-copy pipeline:** `ZeroCopyPipeline` uses scatter-gather DMA descriptors; fallback for non-mappable buffers automatic
- **Cache budget:** `AdaptiveCacheBudgetManager` maintains hot/warm/cold tier budget sum invariant (total = 512MB default)
- **Release gate:** `ReleaseGateV2` requires ALL 9 KPI dimensions to pass; `ReleaseKPIThresholds::ForV83()` has exact thresholds
- **Doc sync audit:** Sprint 173 `DocumentationSyncAudit` checks 7 artifacts — run before declaring any release done

## New Patterns Discovered in v10.5.0 Sprints

- **Resource pooling:** `ResourcePoolEngine` manages checkout/return lifecycle with TTL eviction and prewarming (Sprint 240)
- **CLI parsing:** `CommandLineInterface` supports Flag/String/Int/FilePath/Enum arg types with validation and help generation (Sprint 241)
- **Metadata extraction:** `MetadataExtractor` handles 5 standards (EXIF/IPTC/XMP/ICC/GPS) with 16 fields and formatting utilities (Sprint 242)
- **Notification system:** `NotificationEngine` manages 7 event types with priority-scaled duration (Critical=3x, High=2x) and max-cap overflow (Sprint 243)
- **Content indexing:** `ContentIndexer` classifies 40+ extensions into 8 content types with batch indexing and multi-axis search (Sprint 245)
- **Network diagnostics:** `NetworkDiagnostics` tests 5 connectivity types (Ping/DNS/HTTP/Proxy/TLS) with proxy config support (Sprint 246)
- **Config migration:** `ConfigMigrationEngine` supports 5 migration actions (Copy/Rename/Transform/Delete/SetDefault) with backup-based rollback (Sprint 247)
- **Release gates:** Gates evolve from V5 (12 KPIs) through V15 (20 KPIs) — each adds KPIs validating new sprint deliverables
- **Test framework:** Custom macros `TEST(name)`, `RUN_TEST(name)`, `ASSERT(cond)` with `g_testsRun/g_testsPassed/g_testsFailed` counters — NOT GTest
- **Batch sprint execution:** 5 sprints per batch — create source files → register CMakeLists.txt → add tests → create sprint docs → git commit each individually
- **Namespace:** All engine classes use `namespace DarkThumbs { namespace Engine { } }` — use `using namespace DarkThumbs::Engine;` in tests
- **File collision handling:** If a header name exists from a prior sprint, create a V2/V3 variant (e.g., TestSuiteExpansionV2.h, PluginMarketplaceV3.h)
- **CMakeLists.txt insertion:** After last registered sprint header, before `# Sprint 8-12:` comment
- **EngineTests.cpp insertion:** Includes before `#include <iostream>`, TEST() before `// Main Test Runner`, RUN_TEST() before `// Sprint 6: Isolation`

## v13.0.0 Block Summary (Sprints 249–298 ✅)

| Phase | Sprints | Title                                                                                             |
| ----- | ------- | ------------------------------------------------------------------------------------------------- |
| P1    | 249–254 | Version Sync, Architecture Docs, Quality Foundation, CI Hardening, Perf Baseline, Release Gate V16 |
| P2    | 255–259 | DPX/Cineon, APNG, Text Preview, DICOM V2, FITS V2 (format expansion)                              |
| P3    | 260–264 | 3MF/USD Decoders, Release Gate V17, D3D12 Async Compute, Async Shell Registration, SIMD Pipeline  |
| P4    | 265–269 | Parallel Batch, Persistent Cache, Release Gate V18, ARM64 Detection V2, MSIX Packaging            |
| P5    | 270–274 | Win11 24H2, Test Suite V2, Fuzz Testing, Release Gate V19, Vulkan Compute Activation              |
| P6    | 275–279 | Plugin Marketplace V3, AI Thumbnails, Spreadsheet Preview, USD/USDZ, Auto-Update Engine           |
| P7    | 280–284 | Release Gate V20 (v12.0), Structured Data, Notebook Preview, Database Preview, Legacy Images      |
| P8    | 285–289 | Vector Formats (CDR/Visio), Scientific Data (HDF5/NetCDF), NIfTI, CAD (STEP/IGES), HDR Display    |
| P9    | 290–294 | Per-Monitor DPI V3, Shell Overlay, Cache Warming, Multi-GPU Load Balancer, Release Gate V21       |
| P10   | 295–298 | Accessibility Pipeline, Telemetry Analytics, Cloud Storage, Release Gate V22 (v13.0 Final)        |

## New Patterns Discovered in v13.0.0 Sprints

- **Decoder expansion:** New decoders in `Engine/Decoders/` for spreadsheets (SpreadsheetPreviewDecoder), notebooks (NotebookPreviewDecoder), databases (DatabasePreviewDecoder), legacy images (LegacyImageDecoder), vector formats (VectorFormatDecoder), scientific data (ScientificDataDecoder), NIfTI (NIfTIDecoder), CAD (CADFormatDecoder), structured data (StructuredDataDecoder), USD (USDDecoder)
- **GPU pipeline:** HDRDisplayPipeline (6 tone map operators), MultiGPULoadBalancer (5 strategies), VulkanComputeActivation (7 features), PerMonitorDPIV3 (8 DPI scales)
- **Cache system:** CacheWarmingService (5 warming strategies, battery-aware), AdaptiveCacheBudgetManager updates
- **Shell integration:** ShellOverlayHandler (7 overlay icon types), CloudStorageIntegration (6 providers, hydration strategies)
- **Quality infrastructure:** FuzzTestingManager (4 backends), AccessibilityPipeline (6 features, 5 color blind modes), TelemetryAnalyticsEngine (privacy-first, opt-in)
- **Release gates:** Gates V16–V22, culminating in V22 (23 KPIs) for v13.0 final release
- **Auto-update:** AutoUpdateEngine (4 channels: Stable/Beta/Dev/Canary, semantic versioning)
- **AI integration:** AIThumbnailEnhancer (7 enhancements, 4 backends: DirectML/ONNX/OpenVINO/CPU)
