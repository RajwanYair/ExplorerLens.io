---
applyTo: "**/CMakeLists.txt,**/build-scripts/**"
---

# Build System Standards — ExplorerLens

## Canonical Build Command

```powershell
# Always use this — sources vcvars64 automatically
.\build-scripts\Build-MSVC.ps1

# With tests
.\build-scripts\Build-MSVC.ps1 -Clean -Test
```

**Never run `cmake` without sourcing vcvars64.bat first.** Without it, CMake picks Clang from PATH.

## Toolchain Requirements

- **Local:** MSVC v145 (cl.exe 19.50), VS 18 2026 BuildTools
- **CI:** No toolset pin — let GitHub runner use whatever VS version ships on `windows-latest`
- **Never** add `toolset: "14.50"` or similar in CI YAML — fails on GitHub-hosted runners

## CMakeLists.txt Registration Rules

Every new file must be registered in BOTH places:

```cmake
# Engine/CMakeLists.txt
set(ENGINE_HEADERS
    ...
    Core/MyNewHeader.h      # ← add here for headers
    ...
)

set(ENGINE_SOURCES
    ...
    Core/MyNewSource.cpp    # ← add here for sources
    ...
)
```

**Insertion points:**
- Core headers: before `# Pipeline`
- Core sources: before `# Pipeline implementations`
- Utils headers: before the trailing `# ` comment in Utils section
- Utils sources: before closing `)`

## CMake Style Rules

- Linker flags (`/NODEFAULTLIB`, `/IGNORE`) are MSVC-only — always guard:
  ```cmake
  if(MSVC)
      target_link_options(... PRIVATE /NODEFAULTLIB:LIBCMT)
  endif()
  ```
- CRT linkage: all targets use `/MD` (`MultiThreadedDLL`) — never `/MT`
- No `/wdXXXX` warning suppressions in CMake or code — fix the root cause

## External Libraries

All external libraries live in `external/`:
```
external/
  compression-libs/  — zlib, lz4, zstd, minizip-ng, lzma, unrar, bzip2, libarchive, xz
  image-libs/        — libwebp, libjxl, libavif, libheif, libde265, dav1d
  camera-libs/       — libraw
  pdf-libs/          — mupdf
  ui-libs/           — wtl
```

Build scripts are in `build-scripts/external-libs/`. All use `Build-Library-Core.ps1`:

```powershell
# Pattern for external lib build scripts
$rootDir = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
. "$PSScriptRoot/../core/Build-Library-Core.ps1"
```

**Never duplicate utility functions** — use `Build-Library-Core.ps1` shared functions.

## Preset Reference

| Preset | Generator | Use Case |
|--------|-----------|----------|
| `default-release` | Ninja | Normal development |
| `default-debug` | Ninja | Debugging |
| `vcpkg-release` | Ninja | vcpkg dependency test |
| `vs2026` | VS 18 | IDE integration |

## Build Script Rules

- `Bump-Version.ps1` is the ONLY way to update version-bearing files
- `build-scripts/core/Build-Library-Core.ps1` must be sourced by all lib build scripts
- Build output logs go to `build-logs/` (`build-latest.log`, `test-latest.log`)
- Stale `CMakeCache.txt` from directory renames: auto-detected by `Build-Library-Core.ps1`

## gitignore Considerations

- `.gitignore` has `Release/` pattern — use `git add -f` for files in `Engine/Release/`
- `build-vcpkg/` is gitignored — never track generated CMake cache/API reply files
- `.vscode/c_cpp_properties.json` is gitignored — machine-local IntelliSense paths

## Validation

Before merging:
1. `.\build-scripts\Build-MSVC.ps1 -Clean -Test` → 0 errors, 0 warnings
2. `ctest --test-dir build -C Release --output-on-failure` → 100% pass
3. Verify `ENGINE_HEADERS` and `ENGINE_SOURCES` contain new files

## Build Caching — sccache

### Why sccache

ExplorerLens has 500+ headers; full rebuilds take 3–5 minutes even on fast machines.
[sccache](https://github.com/mozilla/sccache) is a shared compilation cache that wraps MSVC
`cl.exe` and caches object files by content hash. Cache hits skip compilation entirely.

### Local Setup

```powershell
# Install via scoop (already in scoopfile.json)
scoop install sccache

# Set environment variables (user-level, persistent)
[Environment]::SetEnvironmentVariable('CMAKE_C_COMPILER_LAUNCHER',   'sccache', 'User')
[Environment]::SetEnvironmentVariable('CMAKE_CXX_COMPILER_LAUNCHER', 'sccache', 'User')

# Or pass to cmake directly:
cmake --preset default-release -DCMAKE_C_COMPILER_LAUNCHER=sccache -DCMAKE_CXX_COMPILER_LAUNCHER=sccache
```

### CI Setup (GitHub Actions)

```yaml
- name: Setup sccache
  uses: mozilla-actions/sccache-action@v0.0.7
  with:
    version: "v0.10.0"

- name: Configure with sccache
  shell: pwsh
  env:
    CMAKE_C_COMPILER_LAUNCHER: sccache
    CMAKE_CXX_COMPILER_LAUNCHER: sccache
  run: cmake --preset ci-release
```

### Cache Locations

| Storage | Config | Use Case |
|---------|--------|----------|
| Local disk | `SCCACHE_DIR` (default `~/.cache/sccache`) | Developer machine |
| GitHub Actions cache | `SCCACHE_GHA_ENABLED=true` | CI pipeline |
| S3/Azure Blob | `SCCACHE_BUCKET`, `SCCACHE_AZURE_*` | Shared team cache |

### Checking Cache Hit Rate

```powershell
sccache --show-stats
# Target: >70% cache hit rate on incremental builds
```

### Known Limitations

- `/Zi` debug info: sccache supports MSVC PDB via `/Z7` (embedded) or `/Zi` with recent versions
- PCH: precompiled headers are NOT cached — sccache skips them. Keep `pch.h` stable to minimize rebuilds
- `/MP` (parallel compile): not compatible with sccache; cmake Ninja generator already parallelizes

## CMake Build Cache Patterns

### Ninja + ccache/sccache Integration

The `ci-release` preset already uses Ninja generator. Ninja natively supports `CMAKE_*_COMPILER_LAUNCHER`:

```json
// CMakePresets.json — add to cacheVariables
{
    "CMAKE_C_COMPILER_LAUNCHER": "sccache",
    "CMAKE_CXX_COMPILER_LAUNCHER": "sccache"
}
```

### GitHub Actions Cache for build/

Use `actions/cache` to persist the CMake build directory across runs:

```yaml
- name: Cache CMake build
  uses: actions/cache@v4
  with:
    path: build/
    key: cmake-${{ runner.os }}-${{ hashFiles('CMakeLists.txt', 'Engine/CMakeLists.txt', 'CMakePresets.json') }}
    restore-keys: |
      cmake-${{ runner.os }}-
```

### External Libraries Cache

Already in CI workflows. Key hashes external lib build scripts:

```yaml
- name: Cache external libraries
  uses: actions/cache@v4
  with:
    path: external/
    key: external-libs-${{ hashFiles('build-scripts/external-libs/**') }}
    restore-keys: |
      external-libs-
```
