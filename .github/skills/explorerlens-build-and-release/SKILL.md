# ExplorerLens — Build and Release Skill

## Purpose

Use this skill when the task involves building, testing, packaging, version bumps, or
release preparation for ExplorerLens. Read it fully before starting any build or release
task — each section contains non-obvious constraints specific to this repo.

---

## When to Use This Skill

- Running the canonical MSVC build pipeline
- Performing clean builds or build-plus-test runs
- Validating whether release prerequisites are satisfied
- Bumping version across all 20 version-bearing files
- Checking whether workflow-facing release docs still match repo behavior
- Diagnosing build failures (stale cache, missing vcvars, PCH corruption)

---

## Step-by-Step: Normal Build

```powershell
# 1. Always use the canonical script — it sources vcvars64 automatically
.\build-scripts\Build-MSVC.ps1

# 2. Clean build (after CMakeLists.txt changes or toolset changes)
.\build-scripts\Build-MSVC.ps1 -Clean

# 3. Build + run tests
.\build-scripts\Build-MSVC.ps1 -Clean -Test

# 4. Run tests only (no rebuild)
ctest --test-dir build -C Release --output-on-failure
```

**NEVER** run `cmake` directly without sourcing vcvars64 first — CMake will pick up
Clang from PATH instead of MSVC, silently building with the wrong compiler.

---

## Step-by-Step: Version Bump and Release

```powershell
# Single command — updates ALL 20 version-bearing files, commits, tags, pushes
.\build-scripts\Bump-Version.ps1 -Version "X.Y.Z" -Codename "Name" -TestCount NNNN `
    -ChangelogEntry "Short summary" -TagAndPush
```

**What `-TagAndPush` triggers automatically:**
1. Patches all 20 files (VERSION, CHANGELOG.md, .rc files, SVGs, docs, manifests)
2. `git add -A && git commit`
3. `git tag vX.Y.Z && git push origin main --tags`
4. `release.yml` fires → builds all artifacts → creates GitHub Release
5. `publish-packages.yml` fires → publishes NuGet SDK

**Idempotency guard:** Safe to re-run with the same version — script detects existing
tag and skips. If `SBOMGenerator.h` is locked, re-run with identical parameters.

---

## Step-by-Step: Diagnose Build Failures

| Symptom | Likely Cause | Fix |
|---------|-------------|-----|
| `cl.exe not found` | vcvars not sourced | Use `Build-MSVC.ps1`, never raw cmake |
| `LNK2038: mismatch detected for RuntimeLibrary` | `/MT` vs `/MD` conflict | Rebuild offending external lib with `/MD` |
| `PCH: fatal error C1853` | PCH corruption | `Remove-Item build -Recurse -Force` |
| Stale `CMakeCache.txt` after directory rename | Old paths cached | Delete `build/CMakeCache.txt` |
| Test count unexpectedly unchanged | Stale `.obj` after adding `RUN_TEST()` | Always do clean build after test additions |
| `__builtin_*` errors | GCC intrinsic in MSVC build | Replace with `memcpy()`/`memset()` etc. |

---

## Required Constraints

1. **MSVC v145 only** — never use Clang or GCC for production builds.
2. **Never hand-edit version-bearing files** — use `Bump-Version.ps1` exclusively.
3. **Zero-warnings requirement** — fix root causes; never use `/wd` suppressions.
4. **New headers** require registration in `Engine/CMakeLists.txt` ENGINE_HEADERS.
5. **New sources** require registration in ENGINE_SOURCES.
6. **New test files** require registration in `Engine/Tests/CMakeLists.txt`.
7. **Linker flags** (`/NODEFAULTLIB`, `/IGNORE`) must be wrapped in `if(MSVC)`.
8. **`std::min`/`std::max`** must use `(std::min)(a,b)` form — Windows macro collision.

---

## Canonical File Paths

| Purpose | Path |
|---------|------|
| Main build script | `build-scripts/Build-MSVC.ps1` |
| Full package build | `build-scripts/Build-All-And-Package.ps1` |
| Version bump | `build-scripts/Bump-Version.ps1` |
| Build log | `build-logs/build-latest.log` |
| Test log | `build-logs/test-latest.log` |
| Toolchain file | `cmake/msvc-v145.cmake` |
| CMake presets | `CMakePresets.json` |
| Engine build def | `Engine/CMakeLists.txt` |
| Test registration | `Engine/Tests/CMakeLists.txt` |

---

## Pre-Release Checklist (run before every version push)

```powershell
# 1. Scrub corporate artifacts
git grep -rn "intel.com" -- "*.ps1" "*.yml" "*.yaml" "*.md" "*.json" "*.h" "*.cpp"
git grep -rn "proxy"     -- "*.ps1" "*.yml" "*.yaml" "*.md" "*.json"

# 2. Clean build passes
.\build-scripts\Build-MSVC.ps1 -Clean -Test

# 3. Check for stale version references
$oldVer = "X.Y.Z"  # previous version
git ls-files | Where-Object { $_ -notmatch '^external/' } | ForEach-Object {
    $c = Get-Content $_ -Raw -ErrorAction SilentlyContinue
    if ($c -match [regex]::Escape($oldVer)) { Write-Host "STALE: $_" }
}
```

## Validation Checklist

- [ ] Build succeeds: `Build-MSVC.ps1 -Clean` → 0 errors, 0 warnings
- [ ] Tests pass: `ctest --output-on-failure` → all green
- [ ] Version files updated: `Bump-Version.ps1` ran, not hand-edited
- [ ] No stale old-version references in tracked files
- [ ] No corporate proxy URLs in any tracked file
- [ ] GitHub Release created with all artifacts attached

---

## External Library Build Matrix

All 18 libraries must be built with `/MD` (DLL CRT) before the Engine build.
Build scripts in `build-scripts/external-libs/` handle each one.

| Library | Build Script | Build System | Typical Build Time |
|---------|-------------|--------------|-------------------|
| zlib 1.3.1 | `Build-Zlib.ps1` | CMake | < 30s |
| zstd 1.5.7 | `Build-Zstd.ps1` | CMake | < 60s |
| LZ4 1.10.0 | `Build-LZ4.ps1` | CMake | < 30s |
| minizip-ng 4.0.10 | `Build-MinizipNG.ps1` | CMake | < 60s |
| LZMA SDK 26.00 | `Build-LZMA-SDK-26.00.ps1` | Custom | < 30s |
| UnRAR 7.2.2 | `Build-UnRAR.ps1` | MSBuild | < 60s |
| libarchive 3.7.6 | `Build-Libarchive.ps1` | CMake | < 120s |
| libwebp 1.5.0 | `Build-LibWebP-NMake.ps1` | NMake | < 60s |
| libavif 1.3.0 | `Build-LibAVIF.ps1` | CMake | < 120s |
| dav1d 1.5.1 | `Build-Dav1d.ps1` | Meson | < 120s |
| libjxl 0.11.1 | `Build-LibJXL.ps1` | CMake | < 300s |
| libheif 1.19.5 | `Build-LibHEIF.ps1` | CMake | < 120s |
| libde265 1.0.15 | `Build-LibHEIF.ps1` | CMake | (built with libheif) |
| LibRaw 0.21.2 | `Build-LibRaw.ps1` | MSBuild | < 60s |
| MuPDF 1.24.11 | `Build-MuPDF.ps1` | MSBuild | < 600s |
| OpenJPEG 2.5.3 | `Build-OpenJPEG.ps1` | CMake | Planned |
| FreeType 2.13.3 | `Build-FreeType.ps1` | CMake | Planned |
| FFmpeg 7.1 | `Build-FFmpeg.ps1` | Custom | Planned |

**Rebuild all libraries:**
```powershell
.\build-scripts\Update-All-Libraries.ps1
```

**Common external lib build failures:**

| Symptom | Fix |
|---------|-----|
| `/MT` vs `/MD` link error | Rebuild lib with `-Clean` flag — scripts use `/MD` |
| Missing NASM for dav1d | `scoop install nasm` |
| Missing Meson for dav1d | `scoop install meson` |
| libheif cmake can't find libde265 | Build libde265 first (Build-LibHEIF.ps1 handles both) |
| MuPDF build > 10 minutes | Normal — MuPDF + harfbuzz is huge |

---

## Packaging

### MSI Installer (WiX)
```powershell
pwsh -File packaging\Build-Installer.ps1 -Version "X.Y.Z" -Configuration Release
```

### Portable ZIP
```powershell
pwsh -File packaging\Build-PortableZip.ps1 -Version "X.Y.Z" -Configuration Release
```

### Checksums
```powershell
pwsh -File packaging\Generate-Checksums.ps1
```

### Upload to GitHub Release
```powershell
gh release upload vX.Y.Z packaging\output\*.msi packaging\output\*.zip --clobber
```
