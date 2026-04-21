# ExplorerLens — Copilot Instructions

> **Scoped instructions** for C++, build, CI, release, security, and testing live in
> `.github/instructions/*.instructions.md`. Read those files before making changes in their domain.

## Project Overview

ExplorerLens is a **Windows Shell Extension** (IThumbnailProvider COM DLL) that generates
GPU-accelerated thumbnails for 200+ file formats across 25 specialized decoders.

- **Version:** 36.4.0 (Codename: Antares)
- **Language:** C++20 (MSVC v145 toolset, Visual Studio 18 2026)
- **Build System:** CMake 3.25+ with presets (Engine) + MSBuild (Shell/Manager)
- **Preferred Compiler:** MSVC cl.exe 19.50 (v145 toolset) — **never use Clang for production builds**
- **GPU:** CPU decode with GDI+ fallback · DirectX 11/12/Vulkan GPU planned (Phase 2+)
- **Platforms:** Windows (IThumbnailProvider), macOS Quick Look (stub), Linux Nautilus (stub)
- **COM CLSID:** `9E6ECB90-5A61-42BD-B851-D3297D9C7F39`
- **Build Status:** 0 errors, 0 warnings

## Architecture

```
LENSShell.dll (2940 KB) — COM Shell Extension (IThumbnailProvider)
LENSManager.exe (400 KB) — GUI Configuration Utility
ExplorerLensEngine.lib — Core decode + render pipeline
```

## AI Tooling Surface

ExplorerLens is configured for the current 2026 GitHub Copilot and VS Code agent workflow surface.

| Asset | Location | Role |
|-------|----------|------|
| Repository rules | `.github/copilot-instructions.md` | Primary project contract for agents and Copilot |
| Scoped instructions | `.github/instructions/*.instructions.md` | Pattern-specific rules for CI, tests, versions, size policy, and workspace behavior |
| Custom agents | `.github/agents/*.agent.md` | 4 repo-specialized agents (ExplorerLens, Docs, Release, TestCorpus) + Explore |
| Prompt templates | `.github/prompts/*.prompt.md` | 11 reusable prompts for review, tests, scaffolding, release, and debug |
| Repository skills | `.github/skills/*/SKILL.md` | 6 focused task playbooks for build, docs, decoders, corpus, perf, workflows |
| Capability reference | `.github/standards/ai-tooling-capabilities.md` | Canonical inventory for instructions, agents, prompts, skills, MCP servers, and workflow coverage |
| MCP configuration | `.vscode/mcp.json` | Workspace MCP servers for GitHub, filesystem, and docs-scoped editing |

### MCP Servers (Workspace)

- `github` — GitHub API operations through `@modelcontextprotocol/server-github`
- `filesystem` — full workspace filesystem scope through `@modelcontextprotocol/server-filesystem`
- `project-docs` — docs-only filesystem scope for `.github/` and `docs/`

When changing AI-facing repo files, keep `.github/standards/ai-tooling-capabilities.md` synchronized with the actual assets and workflow inventory.

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

## Build Quick Reference

```powershell
.\build-scripts\Build-MSVC.ps1          # build (sources vcvars64 automatically)
.\build-scripts\Build-MSVC.ps1 -Clean -Test   # clean build + tests
ctest --test-dir build -C Release --output-on-failure
```

> Full toolchain tables, presets, and external library inventory → `.github/instructions/build.instructions.md`

## Key Types

- `LENSArchive` — Main archive handler, routes formats to decoders
- `LENSTYPE` — Enum of supported archive/format types (LENSArchive.h)
- `IThumbnailProvider` — Windows COM interface implemented by LENSShell
- `ExplorerLensEngine` — Core engine library linking all decoders
- `ObservabilityIntegration` — ETW + structured logger singleton
- `BuildValidation` — Compile-time version/feature flag validation

## Testing

- **Framework:** Custom macros `TEST(name)`, `RUN_TEST(name)`, `ASSERT(cond)` with counters — NOT GTest
- **Test count:** ~4744 unit tests, 5 benchmarks (v36.4.0 baseline)
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
14. **Never use `__builtin_memcpy` or any `__builtin_*`** — GCC-only intrinsics; use `memcpy()` in MSVC
15. **Never use `/wdXXXX` warning suppressions** — fix the root cause; zero-warnings by fixing code, not hiding it
16. **Enum values must be UPPER_CASE** — enforced by `.clang-tidy`; rename before committing
17. **Always `grep_search` for new type names before committing** — prevent naming collisions in 500+ header codebase
18. **New TEST() bodies go to `EngineTests_Platform.cpp`** — `extern void Runner()` decls go to `EngineTestsExterns.h`; `RUN_TEST()` calls go to `EngineTests.cpp`; `#include` goes to `EngineTestsIncludes.h`
19. **After adding `RUN_TEST()` calls, always do a clean build** — stale `.obj` files can hide missing test bodies (Sprint 1131-1140 incident)
20. **`std::min`/`std::max` must be parenthesized** — use `(std::min)(a, b)` to avoid Windows macro expansion
21. **`PerfRegressionGate.h` uses `namespace ExplorerLens`** (not `ExplorerLens::Engine`) — tests must use `using namespace ExplorerLens` for this header
22. **Bump-Version.ps1 has an idempotency guard** — running it twice on the same version is safe; it will skip if the version is already present
23. **Each `.rc` file has 4 version strings** — `FILEVERSION X,Y,Z,0`, `PRODUCTVERSION X,Y,Z,0`, `"FileVersion"`, and `"ProductVersion"` — all 4 must match
24. **Pre-sprint type collision check (MANDATORY)** — before committing any new sprint header batch, `grep_search` every new `struct`, `enum class`, and `class` name across `Engine/**/*.h`. Zero matches required (excluding the file being created). Sprint 1311-1320 collision crisis required 9 build rounds to resolve 13 collisions across 18 files — see §11.8 in lessons-learned.md.

## Lessons Learned Reference

See `.github/standards/lessons-learned.md` for the full engineering retrospective.

## Scoped Instructions Index

| Domain | File | applyTo |
|--------|------|---------|
| C++ coding | `.github/instructions/cpp-coding.instructions.md` | `**/*.h, **/*.cpp` |
| Build system | `.github/instructions/build.instructions.md` | `**/CMakeLists.txt, **/build-scripts/**` |
| CI/CD | `.github/instructions/cicd.instructions.md` | `**/*.yml, **/*.yaml` |
| Release | `.github/instructions/release.instructions.md` | `**/Bump-Version.ps1, **/CHANGELOG.md` |
| Security | `.github/instructions/security.instructions.md` | `**/*.h, **/*.cpp, **/*.ps1, **/*.yml` |
| Decoder authoring | `.github/instructions/decoder-authoring.instructions.md` | `**/Engine/Decoders/**` |
| Performance | `.github/instructions/performance.instructions.md` | `**/Engine/**, **/benchmarks/**` |
| Documentation | `.github/instructions/documentation.instructions.md` | `**/*.md, docs/**` |
| Testing | `.github/instructions/testing.instructions.md` | `**/tests/**, **/*_test.py` |
| File size | `.github/instructions/file-size-policy.instructions.md` | `**` |
| Version bump | `.github/instructions/version-bump.instructions.md` | `**` |
