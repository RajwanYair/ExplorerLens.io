# ExplorerLens — Copilot Instructions

## Project Overview

ExplorerLens is a **Windows Shell Extension** (IThumbnailProvider COM DLL) that generates
GPU-accelerated thumbnails for 200+ file formats across 25 specialized decoders.

- **Version:** 34.1.0 (Codename: Arcturus-R)
- **Language:** C++20 (MSVC v145 toolset, Visual Studio 18 2026)
- **Build System:** CMake 3.25+ with presets (Engine) + MSBuild (Shell/Manager)
- **Preferred Compiler:** MSVC cl.exe 19.50 (v145 toolset) — **never use Clang for production builds**
- **GPU:** DirectX 11 + DirectX 12 + Vulkan Compute with CPU fallback
- **Platforms:** Windows (IThumbnailProvider), macOS Quick Look (stub), Linux Nautilus (stub)
- **COM CLSID:** `9E6ECB90-5A61-42BD-B851-D3297D9C7F39`
- **Build Status:** 0 errors, 0 warnings

## Architecture

```
LENSShell.dll (2940 KB) — COM Shell Extension (IThumbnailProvider)
LENSManager.exe (400 KB) — GUI Configuration Utility
ExplorerLensEngine.lib — Core decode + render pipeline
```

### Key Directories

| Directory | Purpose |
| ------------------------------ | --------------------------------------------------------------------------------- |
| `LENSShell/` | Shell extension DLL (COM registration, thumbnail provider) |
| `LENSManager/` | WTL-based admin GUI for registration/settings |
| `Engine/` | Core library — decoders, GPU pipeline, caching, observability |
| `Engine/Core/` | Decode pipeline, GPU renderer, resource management |
| `Engine/Decoders/` | Format-specific decoders (25+ total, incl. CAD/glTF/Scientific) |
| `Engine/Plugin/` | Plugin ecosystem (trust chain, sandbox, compat kit, ref pack) |
| `Engine/Memory/` | Memory management (compactor, hot-mode, pressure controller, footprint optimizer) |
| `Engine/Pipeline/` | Pipeline stages (fallback engine, zero-copy upload, parallel I/O) |
| `Engine/Cache/` | Cache management (adaptive budget, PSO cache, sub-ms cache, multi-tenant) |
| `Engine/Utils/` | Utilities (matrix validation, installer lifecycle) |
| `Engine/Tests/` | Unit tests + Google Benchmark |
| `Engine/AI/` | AI/ML modules (scene understanding, smart crop, IQA, search) |
| `Engine/GPU/` | GPU decode acceleration (NVDEC/QuickSync/AMF vendor routing) |
| `Engine/Media/` | Live preview scrubber (video frame extraction, timeline, cache) |
| `Engine/Platform/` | Platform Abstraction Layer — Win32, macOS QL, Linux Nautilus |
| `build-scripts/` | PowerShell build automation |
| `build-scripts/core/` | Build-Library-Core.ps1 — unified build module |
| `build-scripts/external-libs/` | Per-library build scripts (zlib, LZ4, zstd, etc.) |
| `cmake/` | CMake toolchain files |
| `packaging/` | MSI (WiX), Inno Setup, MSIX manifests |
| `SDK/` | Plugin SDK (C ABI, plugin_api.h) |
| `docs/` | All documentation |
| `.github/workflows/` | CI/CD pipelines |

## Toolchain & Build Tools

> **Local vs CI Toolchain:** Always prefer the LATEST available toolchain on each environment.
> - **Local dev:** VS 18 2026 BuildTools — MSVC **v145** (cl.exe 19.50) — hardcoded in `Build-MSVC.ps1`
> - **GitHub CI:** `windows-latest` runner ships **VS 2022 (MSVC v143, cl.exe 19.43)**. Workflows use
>   `ilammy/msvc-dev-cmd@v1` with **no toolset pin** so they auto-upgrade when GitHub provides VS 2026 runners.
> - **Never pin** `toolset: "14.50"` or similar in CI workflows — it fails on GitHub-hosted runners.

### Required: VS 18 2026 BuildTools + MSVC v145 (local)

All builds **must** use MSVC v145 toolset from Visual Studio 18 (2026) BuildTools.

| Tool | Version | Location |
| ---------------- | ------------ | --------------------------------------------------------------------------------------------------------------- |
| **cl.exe** | 19.50.35720 | `C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Tools\MSVC\14.50.35717\bin\Hostx64\x64\cl.exe` |
| **link.exe** | 14.50.35717 | Same MSVC bin dir |
| **lib.exe** | 14.50.35717 | Same MSVC bin dir |
| **msbuild.exe** | 18.3 | `...\BuildTools\MSBuild\Current\Bin\amd64\MSBuild.exe` |
| **vcvars64.bat** | — | `...\BuildTools\VC\Auxiliary\Build\vcvars64.bat` |
| **vcpkg** | 2026-02-21 | `...\.BuildTools\VC\vcpkg\vcpkg.exe` (bundled) |
| **Windows SDK** | 10.0.26100.0 | `C:\Program Files (x86)\Windows Kits\10` |

### Additional Tools (Scoop — preferred for latest versions)

| Tool | Version | Path |
| ----- | ------- | ------------------------- |
| cmake | 4.3.1 | `~/scoop/shims/cmake.exe` |
| ninja | 1.13.2 | `~/scoop/shims/ninja.exe` |
| git | 2.53.0 | `~/scoop/shims/git.exe` |
| nasm | 3.01 | `~/scoop/shims/nasm.exe` |
| meson | 1.10.2 | `~/scoop/shims/meson.exe` |
| nuget | 7.3.0 | `~/scoop/shims/nuget.exe` |
| 7zip | 26.00 | `~/scoop/shims/7z.exe` |
| WiX | 6.0.2 | `~/.dotnet/tools/wix.exe` |

### MSVC Toolset Versions Available

| Toolset | cl.exe Version | Status |
| ----------- | -------------- | ----------- |
| 14.50.35717 | 19.50.35720 | **Primary** |
| 14.44.35207 | 19.44.35222 | Fallback |

## Build Commands

### Recommended: CMake Preset + Build Script (sources vcvars64 automatically)

```powershell
# One-command build (recommended — handles vcvars, cmake, ninja)
.\build-scripts\Build-MSVC.ps1

# Clean build
.\build-scripts\Build-MSVC.ps1 -Clean

# With tests
.\build-scripts\Build-MSVC.ps1 -Clean -Test

# vcpkg preset
.\build-scripts\Build-MSVC.ps1 -Preset vcpkg-release
```

### Manual: CMake Presets (requires vcvars64 sourced first)

```powershell
# IMPORTANT: Source vcvars first! Without this, CMake picks Clang from PATH.
cmd /c '"C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat" -vcvars_ver=14.50.35717 && powershell'

# Configure + Build
cmake --preset default-release
cmake --build --preset default-release -j 8

# Test
ctest --test-dir build -C Release --output-on-failure
```

### Available CMake Presets

| Preset | Generator | Compiler | Dependencies |
| ----------------- | --------- | --------- | --------------- |
| `default-release` | Ninja | MSVC v145 | Local external/ |
| `default-debug` | Ninja | MSVC v145 | Local external/ |
| `vcpkg-release` | Ninja | MSVC v145 | vcpkg |
| `vcpkg-debug` | Ninja | MSVC v145 | vcpkg |
| `vs2026` | VS 18 | MSVC v145 | Local external/ |

### MSBuild (Shell + Manager only)

```powershell
msbuild LENSShell.sln /p:Configuration=Release /p:Platform=x64 /m /v:minimal
```

### Production Build (all libraries + packaging)

```powershell
.\build-scripts\Build-All-And-Package.ps1
```

## CRT Linkage

All targets use `/MD` (dynamic CRT — `MultiThreadedDLL`). All external libraries including libwebp
have been rebuilt with `/MD`. No `/NODEFAULTLIB:LIBCMT` workarounds are needed.

## External Libraries (Statically Linked)

| Library | Version | Purpose |
| ---------- | ------- | ------------------------- |
| zlib | 1.3.1 | ZIP/deflate compression |
| LZ4 | 1.10.0 | Fast compression |
| zstd | 1.5.7 | Zstandard compression |
| LZMA SDK | 26.00 | 7z archives |
| minizip-ng | 4.0.10 | ZIP archive handling |
| UnRAR | 7.2.2 | RAR archive extraction |
| libwebp | 1.5.0 | WebP images |
| libavif | 1.3.0 | AVIF images |
| libjxl | 0.11.1 | JPEG XL images |
| libheif | 1.19.5 | HEIF/HEIC images |
| libde265 | 1.0.15 | HEVC decoding for libheif |
| dav1d | 1.5.1 | AV1 decoding for libavif |
| LibRaw | 0.21.3 | RAW camera formats |
| MuPDF | 1.24.11 | PDF rendering |
| openjpeg | 2.5.3 | JPEG 2000 images |
| bzip2 | 1.0.8 | BZIP2 compression |
| xz/liblzma | 5.6.3 | XZ compression |
| libarchive | 3.7.6 | Multiformat archive handling |

## Code Conventions

- **C++20** standard, all headers use `#pragma once`
- Classes: `PascalCase`, functions: `PascalCase`, variables: `camelBack`
- Private members: `m_` prefix, constants: `UPPER_CASE`
- **Zero warnings policy** — build must produce 0 warnings
- See `.clang-tidy` for static analysis config
- See `.github/standards/coding-standards.md` for full guide
- COM interfaces follow Windows SDK patterns (IUnknown, AddRef/Release)

### Header Banner Format

All headers use this standardized Copyright doc-block (banner BEFORE `#pragma once`):

```cpp
// FileName.h — Short Title
// Copyright (c) 2026 ExplorerLens Project
//
// Description of what this header provides and its role in the architecture.
//
#pragma once
```

- **No** `=====` decorator lines, version numbers, sprint/batch tags
- Prefer block-level comments over inline comments
- Remove inline comments that just restate the type name (`// Stats` above `struct Stats`)
- Keep comments that explain *why* or *how*, not *what* the next line declares

## Key Types

- `LENSArchive` — Main archive handler, routes formats to decoders
- `LENSTYPE` — Enum of supported archive/format types (LENSArchive.h)
- `IThumbnailProvider` — Windows COM interface implemented by LENSShell
- `ExplorerLensEngine` — Core engine library linking all decoders
- `ObservabilityIntegration` — ETW + structured logger singleton
- `BuildValidation` — Compile-time version/feature flag validation

## Testing

- **Framework:** Custom macros `TEST(name)`, `RUN_TEST(name)`, `ASSERT(cond)` with counters — NOT GTest
- **Test count:** ~4604 unit tests, 5 benchmarks (v34.1.0 baseline)
- **Pass rate:** 100%
- **Performance targets:** 17ms single thumbnail, 235 img/sec batch, <5ms cache hit

## Important Rules for AI Assistants

1. **Never break the zero-warnings build** — all changes must compile cleanly with MSVC v145
2. **Always use MSVC v145** — never configure CMake without vcvars64.bat sourced; use `Build-MSVC.ps1`
3. **New headers must be registered** in `Engine/CMakeLists.txt` ENGINE_HEADERS
4. **New source files must be registered** in ENGINE_SOURCES
5. **New test files must be registered** in `Engine/Tests/CMakeLists.txt` EngineTests target
6. **COM CLSID is fixed** — do not change `9E6ECB90-5A61-42BD-B851-D3297D9C7F39`
7. **LENSTYPE enum values must not collide** — check existing values in LENSArchive.h
8. **Build scripts use Build-Library-Core.ps1** — don't duplicate utility functions
9. **Documentation versions must stay in sync** — update all docs when version changes
10. **Git commits must have descriptive messages**
11. **Build with:** `.\build-scripts\Build-MSVC.ps1` (or `cmake --preset default-release` after vcvars)
12. **Test with:** `ctest --test-dir build -C Release --output-on-failure`
13. **Linker flags** (`/NODEFAULTLIB`, `/IGNORE`) are MSVC link.exe only — guard with `if(MSVC)` in CMake

## External Libraries Directory Structure (Post-Cleanup)

```
external/
 compression-libs/ — zlib, lz4, zstd, minizip-ng, lzma, unrar, bzip2, libarchive, xz
 image-libs/ — libwebp, libjxl, libavif, libheif, libde265, dav1d
 camera-libs/ — libraw, libraw-install
 pdf-libs/ — mupdf
 ui-libs/ — wtl
```

> **Note:** Build scripts use `$rootDir = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)` to resolve the project root. All scripts import `Build-Library-Core.ps1` from `build-scripts/core/`.

## .gitignore Considerations

- The `Release/` pattern in `.gitignore` blocks `Engine/Release/` — use `git add -f` for files there
- Stale `CMakeCache.txt` files from directory renames are auto-detected by `Build-Library-Core.ps1`
- `build-vcpkg/` is gitignored — never track generated CMake cache/API reply files
- `.vscode/c_cpp_properties.json` is gitignored — machine-local IntelliSense paths

## Windows SDK Header Rules (CRITICAL)

Because `WIN32_LEAN_AND_MEAN` is globally defined:

- **NEVER** include `<versionhelpers.h>` — use `RtlGetVersion()` from ntdll.dll instead
- **NEVER** include headers that depend on types excluded by `WIN32_LEAN_AND_MEAN`
- Always verify new Windows SDK includes compile under `WIN32_LEAN_AND_MEAN`
- See `.github/standards/build-troubleshooting.md` for the full compatibility list

## Development Guidance (v15.2+)

- **Current version:** v34.1.0 "Arcturus-R"
- **Source of truth:** `CHANGELOG.md`
- **Gen-6 Roadmap:** `docs/ROADMAP_V30.md` — v30.x "Deneb" Platform Unification + v31.x "Achernar" Generative AI
- **Per feature commit policy:** one clear commit per feature with objective + impacted areas
- **Every revision bump MUST:** include full binaries (LENSShell.dll, LENSManager.exe, lens.exe, Manager.WinUI.exe, MSI, ZIP, SBOM) in the GitHub Release AND publish to all 5 package registries (NuGet, npm, Container, Maven, RubyGems) — **this is non-negotiable, no exceptions**
- **Version-bearing file registry:** See `.github/instructions/version-bump.instructions.md` for the complete 21-file checklist and release artifact procedure
- **Graphics must be updated on every bump:** `docs/assets/social-preview.svg` and `docs/assets/architecture-build.svg` must reflect the current version, test count, and platform support — `Bump-Version.ps1` handles this automatically
- **Deliverables pattern:** header in `Engine/`, test in `Engine/Tests/EngineTests.cpp`, CMakeLists.txt registration (BOTH `Engine/CMakeLists.txt` ENGINE_HEADERS/ENGINE_SOURCES), git commit
- **Batch pattern:** Create 5 source files → register in CMakeLists.txt (multi-replace) → add includes + TEST() + RUN_TEST() to EngineTests.cpp → git commit each individually
- **CMakeLists.txt insertion points:** Core headers before `# Pipeline`, Core sources before `# Pipeline implementations`, Utils headers before `# `, Utils sources before closing `)`
- **EngineTests.cpp insertion points:** New includes after last feature include, TEST() functions before `//== ` section, RUN_TEST() calls before `// Isolation & Stability Tests`
- **Test bodies go to EngineTests_Late.cpp:** extern void declarations + RUN_TEST() calls in EngineTests.cpp; TEST() function bodies in EngineTests_Late.cpp (or EngineTests_Mid.cpp for earlier sprints)
- **Sprint 1131-1140 backfill note:** In v33.0.0 the Sprint 1131-1140 test bodies (VideoFrameExtractor/ScrubberTimeline/etc.) were backfilled to EngineTests_Mid.cpp — they were missing despite the RUN_TEST calls existing (stale .obj build artifact).

## Release Procedure (EVERY version bump)

> **Non-negotiable:** Every version bump MUST trigger a GitHub Release with all binaries AND
> publish to all 5 package registries. This applies to EVERY version (X.Y.Z) without exception.
> `Bump-Version.ps1 -TagAndPush` is the ONE command to rule them all:
> - Updates all 21 version-bearing files (including npm/ruby/Dockerfile packaging manifests when present)
> - Commits with a descriptive message
> - Creates the `vX.Y.Z` git tag
> - Pushes tag → fires `release.yml` → builds LENSShell.dll + LENSManager.exe + lens.exe + MSI + ZIP + SBOM
> - The GitHub Release creation (`release: published`) then triggers `publish-packages.yml` →
>   publishes simultaneously to NuGet, npm, Container (ghcr.io), Maven, and RubyGems

```powershell
# ─── The COMPLETE release command (do this on every version bump, no exceptions) ───

.\build-scripts\Bump-Version.ps1 -Version "X.Y.Z" -Codename "Codename" -TestCount NNNN `
    -ChangelogEntry "Short release summary" -TagAndPush

# What -TagAndPush does automatically:
#  1. Updates ALL 21 version-bearing files:
#       VERSION, CHANGELOG.md, Engine/Core/BuildValidation.h, Engine/Core/SBOMGenerator.h
#       .github/copilot-instructions.md, .github/standards/tool-versions.md
#       .github/standards/build-method.md, docs/assets/social-preview.svg
#       docs/assets/architecture-build.svg, README.md, vcpkg.json
#       docs/SBOM.json, Engine/Tests/benchmarks/baseline.json
#       CMakeLists.txt, Engine/CMakeLists.txt
#       LENSManager/LENSManager.rc, LENSShell/LENSShell.rc
#       docs/USER_GUIDE.md
#       packaging/npm/package.json (if present), packaging/ruby/lib/explorerlens/version.rb (if present)
#       Dockerfile (if present)
#  2. git add -A && git commit -m "chore: bump version to X.Y.Z (Codename)"
#  3. git tag vX.Y.Z
#  4. git push origin main --tags
#
# ── What happens automatically in CI after push ──
#   tag vX.Y.Z pushed → release.yml fires:
#     Build Engine lib + LENSShell.dll + LENSManager.exe + lens.exe CLI + Manager.WinUI.exe
#     Build MSI (WiX) + ZIP + SHA256SUMS.txt + SBOM.json + verification-report.json
#     Create GitHub Release (not draft) with all artifacts attached
#   release: published → publish-packages.yml fires (5 parallel jobs):
#     NuGet   → nuget.pkg.github.com/RajwanYair
#     npm     → npm.pkg.github.com  (@rajwanyair/explorerlens)
#     Container → ghcr.io/rajwanyair/explorerlens
#     Maven   → maven.pkg.github.com/RajwanYair
#     RubyGems → rubygems.pkg.github.com/RajwanYair
#     publish-summary: aggregates all 5 results into step summary, exits 1 on any failure
```

> **Before pushing:** scrub corporate artefacts from ALL tracked files:
> ```powershell
> git grep -rn "intel.com" -- "*.ps1" "*.yml" "*.yaml" "*.md" "*.json" "*.h" "*.cpp"
> git grep -rn "proxy"     -- "*.ps1" "*.yml" "*.yaml" "*.md" "*.json"
> git grep -rn "928\b"     -- "*.ps1" "*.yml" "*.yaml"   # port 928 = Intel proxy
> ```

### Per-revision GitHub Release checklist

| Step | Action |
|------|--------|
| ✅ | CHANGELOG.md has new `[X.Y.Z]` section with all deliverables |
| ✅ | All 18 version-bearing files updated via `Bump-Version.ps1 -TagAndPush` |
| ✅ | Corporate proxy URLs scrubbed from tracked files |
| ✅ | Local build passes: `.\build-scripts\Build-MSVC.ps1 -Test` → 0 errors, 0 warnings |
| ✅ | `git tag vX.Y.Z` pushed to origin → `release.yml` triggered → GitHub Release published |
| ✅ | GitHub Release page shows all artifacts: .dll, .exe, .msi, .zip, SHA256SUMS.txt, SBOM.json |
| ✅ | `publish-packages.yml` summary shows ✅ for all 5 registries (NuGet/npm/Container/Maven/RubyGems) |

## Release Artifact Checklist

| Artifact | Condition | Notes |
|----------|-----------|-------|
| `LENSShell.dll` (x64) | Always | COM shell extension |
| `LENSManager.exe` | Always | WTL config GUI |
| `lens.exe` | Sprint 17+ | CLI tool |
| `Manager.WinUI.exe` | Sprint 52+ | WinUI 3 modern GUI |
| `ExplorerLens-X.Y.Z-x64.msi` | Always | WiX installer |
| `ExplorerLens-X.Y.Z-x64.zip` | Always | Portable archive |
| `SHA256SUMS.txt` | Always | Checksums for all artifacts |
| `ExplorerLens-X.Y.Z-SBOM.json` | Always | CycloneDX SBOM |

## GitHub Packages (published on every release)

| Registry | Package | URL |
|----------|---------|-----|
| NuGet | `ExplorerLens.SDK` | `nuget.pkg.github.com/RajwanYair/index.json` |
| npm | `@rajwanyair/explorerlens` | `npm.pkg.github.com` |
| Container | `explorerlens` | `ghcr.io/rajwanyair/explorerlens` |
| Apache Maven | `io.github.rajwanyair:explorerlens` | `maven.pkg.github.com/RajwanYair` |
| RubyGems | `explorerlens` | `rubygems.pkg.github.com/RajwanYair` |

## Key Architecture Patterns

- **Namespace:** All engine classes use `namespace ExplorerLens { namespace Engine { } }` — use `using namespace ExplorerLens::Engine;` in tests
- **Test framework:** Custom macros `TEST(name)`, `RUN_TEST(name)`, `ASSERT(cond)` with `g_testsRun/g_testsPassed/g_testsFailed` counters — NOT GTest
- **Test file split:** EngineTests.cpp (harness + main, ~8.7K lines) + EngineTests_Core.cpp (~9.7K lines) + EngineTests_Features.cpp (~9.5K lines) + EngineTests_Mid.cpp (~9.4K lines) + EngineTests_Late.cpp (~12.8K lines) — all share EngineTestsIncludes.h + EngineTestsMacros.h
- **Release gates:** Unified `Utils/ReleaseGate.h` — single header covering all gate versions (V2–V33)
- **Plugin architecture:** Plugin files go in `Engine/Plugin/`; use `ICADDecoderPlugin` / `IThumbnailPlugin` patterns
- **Memory pressure ladder:** 5-tier (None/Low/Medium/High/Critical) with bitmask `PressureAction` flags
- **Sub-ms cache:** `SubMillisecondCacheEngine` uses robin-hood open-addressing with `CacheHashAlgo::XXH3` default
- **AI/ML modules:** Classes in `Engine/AI/` use `SceneMLBackend` / `EmbeddingModel` enum for backend selection (DirectML/ONNX/OpenVINO/CPU)
- **GPU decode routing:** `GPUDecodeAccelerationV2` in `Engine/GPU/` routes by `GPUDecodeVendor` at runtime
- **Enterprise pattern:** Policy source hierarchy: `EnterprisePolicyEngineV2` (GPO → Intune → ConfigMgr → Manual)
- **Platform PAL (v33+):** `PlatformShellProvider` abstract base in `Engine/Platform/` — `Win32ShellProvider`, `MacOSQLProvider`, `LinuxNautilusProvider` concrete stubs; `PlatformDetector` factory. All stubs compile on Windows MSVC via `#ifdef _WIN32 / __APPLE__ / __linux__` guards — no platform SDK required.
- **Live Preview (v32.7+):** `VideoFrameExtractor`, `VideoScrubberTimeline`, `ScrubberCacheEngine`, `ThumbnailStripGenerator`, `LivePreviewSession` all in `Engine/Media/`

## Shell & Build Integration Rules

1. **Never send Ctrl-C/Break** to running build commands — builds take 60-120s
2. **Use bat wrappers** for builds: `build-scripts\build-and-log.bat [logfile]`
3. **Monitor via log files** — use `read_file` on `build-logs\*.log` instead of terminal output
4. **If terminal is busy** — open a NEW terminal; don't kill the existing one
5. **Kill only orphaned processes** — check `StartTime` before killing `ninja.exe` or `cl.exe`
6. **Clean .obj locks** before retry: `Remove-Item build\**\*.obj -Force`
7. **Build output is in** `build-logs/` — `build-latest.log`, `test-latest.log`
8. **EngineTests.cpp compilation** takes ~90s (22K lines) — DO NOT assume it's hung
9. **LTCG linking** can take 30s — this is normal for Release/GL builds
