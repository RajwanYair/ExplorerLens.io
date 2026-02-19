# DarkThumbs — Copilot Instructions

## Project Overview

DarkThumbs is a **Windows Shell Extension** (IThumbnailProvider COM DLL) that generates
GPU-accelerated thumbnails for 200+ file formats across 25 specialized decoders.

- **Version:** 8.4.0
- **Language:** C++20 (MSVC v145 toolset, Visual Studio 18 2026)
- **Build System:** CMake 3.20+ (Engine) + MSBuild (Shell/Manager)
- **GPU:** DirectX 11 + DirectX 12 with CPU fallback
- **COM CLSID:** `9E6ECB90-5A61-42BD-B851-D3297D9C7F39`
- **Sprint Count:** 177 completed (v8.4.0 block: Sprints 175–177 ✅)
- **Build Status:** 0 errors, 0 warnings

## Architecture

```
CBXShell.dll (2940 KB)     — COM Shell Extension (IThumbnailProvider)
CBXManager.exe (400 KB)    — GUI Configuration Utility
DarkThumbsEngine.lib       — Core decode + render pipeline
```

### Key Directories

| Directory                      | Purpose                                                       |
| ------------------------------ | ------------------------------------------------------------- |
| `CBXShell/`                    | Shell extension DLL (COM registration, thumbnail provider)    |
| `CBXManager/`                  | WTL-based admin GUI for registration/settings                 |
| `Engine/`                      | Core library — decoders, GPU pipeline, caching, observability |
| `Engine/Core/`                 | Decode pipeline, GPU renderer, resource management            |
| `Engine/Decoders/`             | Format-specific decoders (25+ total, incl. CAD/glTF/Scientific) |
| `Engine/Plugin/`               | Plugin ecosystem (trust chain, sandbox, compat kit, ref pack) |
| `Engine/Memory/`               | Memory management (compactor, hot-mode, pressure controller)  |
| `Engine/Pipeline/`             | Pipeline stages (fallback engine, zero-copy upload)           |
| `Engine/Cache/`                | Cache management (adaptive budget manager)                    |
| `Engine/Utils/`                | Utilities (ARM64 support, matrix validation, installer lifecycle) |
| `Engine/Tests/`                | GTest unit tests + Google Benchmark                           |
| `build-scripts/`               | PowerShell build automation                                   |
| `build-scripts/core/`          | Build-Library-Core.ps1 — unified build module                 |
| `build-scripts/external-libs/` | Per-library build scripts (zlib, LZ4, zstd, etc.)             |
| `cmake/`                       | CMake toolchain files (incl. toolchain-windows-arm64.cmake)   |
| `packaging/`                   | MSI (WiX), Inno Setup, MSIX manifests                         |
| `SDK/`                         | Plugin SDK (C ABI, plugin_api.h)                              |
| `docs/`                        | All documentation                                             |
| `docs/development/sprints-v8/` | Per-sprint markdown docs (SPRINT_1.md … SPRINT_177.md)        |
| `.github/workflows/`           | CI/CD pipelines (incl. arm64.yml)                             |

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

- **Framework:** Google Test + Google Benchmark
- **Test count:** ~437 unit tests (100 original + 337 new Sprints 150–174), 5 benchmarks
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

## Sprint Execution Guidance (v8.4+)

- **Current version:** v8.4.0 (Sprints 175-177 complete)
- **Next roadmap block:** Sprints 178+ per `docs/IMPROVEMENT_PLAN_V9.md`
- **Next block theme (v9.0.0):** Format expansion, async shell extension, D3D12 compute pipeline
- **Execution package docs:** `docs/development/sprints-v8/SPRINT_XX.md`
- **Source of truth:** `MASTER_PLAN.md` + `docs/IMPROVEMENT_PLAN_V9.md`
- **Carry-over closure:** legacy "planned/partial" items from older sections must be explicitly mapped to new sprint tasks
- **Per sprint commit policy:** one clear commit per sprint with objective + impacted areas
- **Sprint deliverables pattern:** header in `Engine/`, GTest in `Engine/Tests/`, doc in `docs/development/sprints-v8/`, CMakeLists.txt registration (BOTH `Engine/CMakeLists.txt` ENGINE_HEADERS AND `Engine/Tests/CMakeLists.txt` EngineTests sources), git commit

## v8.4.0 Block Summary (Sprints 175–177 ✅)

| Sprint | Title | Key Changes |
|---|---|---|
| 175 | Critical Bug Fixes | Fixed djvu→CBXTYPE_DJVU, added model routing, AVIF/HEIF separation, HEIF extensions |
| 176 | Shell Registration Expansion | CBXShell.rgs 47→93 extensions (archives, RAW, docs, models, fonts) |
| 177 | Version Normalization | All docs updated to v8.4.0, CHANGELOG v8.0-8.4 entries added |

## v8.3.0 Block Summary (Sprints 150–174 ✅)

| Phase | Sprints | Title |
|---|---|---|
| P1 | 150–154 | Plugin Ecosystem Hardening (sandbox, trust chain, compat kit, ref pack) |
| P2 | 155–159 | ARM64 Foundation (build config, lib matrix, runtime validator, CI) |
| P3 | 160–164 | Format Expansion (JPEG2000, CAD/glTF/Scientific, fallback engine) |
| P4 | 165–169 | Memory Excellence (compactor, zero-copy, adaptive cache, hot-mode, pressure V2) |
| P5 | 170–174 | v8.3.0 Release (matrix validation, installer lifecycle, release gate V2, doc sync, closure) |

## New Patterns Discovered in v8.3.0 Sprints

- **ARM64 cross-compile:** Use `cmake/toolchain-windows-arm64.cmake` with MSVC `amd64_arm64` arch; CI in `.github/workflows/arm64.yml`
- **Plugin architecture:** Plugin files go in `Engine/Plugin/`; use `ICADDecoderPlugin` / `IThumbnailPlugin` patterns from Sprint 161/153
- **Memory pressure ladder:** 5-tier (None/Low/Medium/High/Critical) with bitmask `PressureAction` flags — see `MemoryPressureControllerV2.h`
- **Format fallback:** `FormatFallbackEngine` routes via `FallbackTrigger` bitmask flags; default chains in `CreateDefault()`
- **Zero-copy pipeline:** `ZeroCopyPipeline` uses scatter-gather DMA descriptors; fallback for non-mappable buffers automatic
- **Cache budget:** `AdaptiveCacheBudgetManager` maintains hot/warm/cold tier budget sum invariant (total = 512MB default)
- **Release gate:** `ReleaseGateV2` requires ALL 9 KPI dimensions to pass; `ReleaseKPIThresholds::ForV83()` has exact thresholds
- **Doc sync audit:** Sprint 173 `DocumentationSyncAudit` checks 7 artifacts — run before declaring any release done
