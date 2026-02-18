# DarkThumbs — Copilot Instructions

## Project Overview

DarkThumbs is a **Windows Shell Extension** (IThumbnailProvider COM DLL) that generates
GPU-accelerated thumbnails for 200+ file formats across 25 specialized decoders.

- **Version:** 7.1.0
- **Language:** C++20 (MSVC v145 toolset, Visual Studio 18 2026)
- **Build System:** CMake 3.20+ (Engine) + MSBuild (Shell/Manager)
- **GPU:** DirectX 11 + DirectX 12 with CPU fallback
- **COM CLSID:** `9E6ECB90-5A61-42BD-B851-D3297D9C7F39`
- **Sprint Count:** 74 completed
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
| `Engine/Decoders/`             | Format-specific decoders (25 total)                           |
| `Engine/Tests/`                | GTest unit tests + Google Benchmark                           |
| `build-scripts/`               | PowerShell build automation                                   |
| `build-scripts/core/`          | Build-Library-Core.ps1 — unified build module                 |
| `build-scripts/external-libs/` | Per-library build scripts (zlib, LZ4, zstd, etc.)             |
| `packaging/`                   | MSI (WiX), Inno Setup, MSIX manifests                         |
| `SDK/`                         | Plugin SDK (C ABI, plugin_api.h)                              |
| `docs/`                        | All documentation                                             |
| `.github/workflows/`           | CI/CD pipelines                                               |

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
- **Test count:** 100 unit tests, 5 benchmarks
- **Pass rate:** 100%
- **Performance targets:** 17ms single thumbnail, 235 img/sec batch, <5ms cache hit

## Important Rules for AI Assistants

1. **Never break the zero-warnings build** — all changes must compile cleanly
2. **New headers must be registered** in `Engine/CMakeLists.txt` ENGINE_HEADERS
3. **New source files must be registered** in ENGINE_SOURCES
4. **COM CLSID is fixed** — do not change `9E6ECB90-5A61-42BD-B851-D3297D9C7F39`
5. **CBXTYPE enum values must not collide** — check existing values in cbxArchive.h
6. **Build scripts use Build-Library-Core.ps1** — don't duplicate utility functions
7. **Documentation versions must stay in sync** — update all docs when version changes
8. **Git commits must have descriptive messages** — include sprint number if applicable
9. **Test changes with:** `cmake --build build --config Release -j 8`
10. **Validate with:** `ctest --test-dir build -C Release --output-on-failure`

## Sprint Execution Guidance (v7.2+)

- **Next roadmap block:** Sprints 75-110 (v7.2.0 -> v8.0.0 refactor program)
- **Execution package docs:** `docs/development/sprints-v8/SPRINT_XX.md`
- **Source of truth:** `MASTER_PLAN.md` must be updated before creating new sprint work
- **Carry-over closure:** legacy "planned/partial" items from older sections must be explicitly mapped to new sprint tasks
- **Per sprint commit policy:** one clear commit per sprint with objective + impacted areas
