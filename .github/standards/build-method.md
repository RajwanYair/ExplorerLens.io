# ExplorerLens Build Method - Standard Operating Procedure

**Last Updated:** March 2026
**Version:** 32.2.0 (Codename: Fomalhaut-S)
**Policy:** 64-bit only, warnings-as-errors in Release, VS Code monitoring, zero-warnings enforcement

---

## Build Architecture

### Supported Configurations

- **Platform**: x64 ONLY (32-bit builds are not supported)
- **Configurations**: Debug, Release
- **Toolchain**: Visual Studio 18 2026 Build Tools (v145 toolset), MSVC 19.50.35720.0
- **Build Systems**:
  - **CMake + Ninja** for Engine library + tests + benchmarks + ModernRuntime (preferred)
  - MSBuild for shell extension (LENSShell.sln)
- **Compilation Units**: 190+ total (Engine lib ~150 + tests/benchmarks ~40)

### Tool Paths (Auto-detected on this machine)

```powershell
# Visual Studio Build Tools 18.3.0 (2026) — MSVC v145 toolset
$VSPath = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools"
$MSBuild = "$VSPath\MSBuild\Current\Bin\amd64\MSBuild.exe"
$vcvars64 = "$VSPath\VC\Auxiliary\Build\vcvars64.bat" # Preferred over vcvarsall
$cl = "$VSPath\VC\Tools\MSVC\14.50.35717\bin\Hostx64\x64\cl.exe" # v19.50.35720

# CMake 4.3.0 (via Scoop)
$CMake = "C:\Users\ryair\scoop\shims\cmake.exe"

# Ninja 1.13.2 (via Scoop)
$Ninja = "C:\Users\ryair\scoop\shims\ninja.exe"
```

**Note:** Tools are auto-detected by `build-scripts\Find-All-Tools.ps1`.
If not in PATH, the script searches standard installation locations.

### CRITICAL: Always Source vcvars64 Before CMake

CMake must be invoked after sourcing `vcvars64.bat` — otherwise it may pick up
Clang from PATH instead of MSVC.

```powershell
# The Build-MSVC.ps1 script handles this automatically.
# For manual builds:
cmd /c '"C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat" -vcvars_ver=14.50.35717 && powershell'
```

### Compiler Flags

**Release configuration:**

- `/W4` - Warning level 4
- `/WX` - Treat warnings as errors (enforced)
- `/O2` - Optimize for speed
- `/GL` - Whole program optimization
- `/arch:AVX2` - AVX2 instruction set
- `/std:c++20` - C++20 standard
- `/MD` - Dynamic CRT runtime (MultiThreadedDLL) — **all targets use /MD**

**Debug configuration:**

- `/W4` - Warning level 4
- `/Od` - No optimization
- `/Zi` - Full debug info
- `/MDd` - Debug dynamic CRT runtime

### Global Preprocessor Defines (CMakeLists.txt)

```text
WIN32_LEAN_AND_MEAN — Excludes rarely-used Windows headers (impacts header compatibility!)
NOMINMAX — Prevents min/max macro conflicts with STL
_WIN32_WINNT=0x0A00 — Targets Windows 10+
WINVER=0x0A00 — Targets Windows 10+
UNICODE / _UNICODE — Unicode build
```

> **WARNING**: `WIN32_LEAN_AND_MEAN` excludes many Windows SDK sub-headers
> (e.g., `<mmsystem.h>`, `<winsock2.h>`).
> Headers like `<versionhelpers.h>` that depend on types from excluded headers
> will fail to compile.
> See [build-troubleshooting.md](build-troubleshooting.md) Issue #4 for details.

---

## Build Process

### Recommended: One-Command Build Script

```powershell
# Standard clean build (handles vcvars, CMake preset, Ninja, logging)
.\build-scripts\Build-MSVC.ps1 -Clean

# Incremental build (faster, reuses previous build state)
.\build-scripts\Build-MSVC.ps1

# With tests
.\build-scripts\Build-MSVC.ps1 -Clean -Test

# vcpkg preset (uses vcpkg for dependencies instead of local external/)
.\build-scripts\Build-MSVC.ps1 -Preset vcpkg-release
```

### Manual: CMake Presets (requires vcvars64 sourced first)

```powershell
# IMPORTANT: Source vcvars first! Without this, CMake picks Clang from PATH.
cmd /c '"C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat" -vcvars_ver=14.50.35717 && powershell'

# Configure (generates build/build.ninja)
cmake --preset default-release

# Build (all 147 targets)
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

### MSBuild (Shell Extension + Manager only)

```powershell
msbuild LENSShell.sln /p:Configuration=Release /p:Platform=x64 /m /v:minimal
```

### Production Build (all libraries + packaging)

```powershell
.\build-scripts\Build-All-And-Package.ps1
```

### With Logging (REQUIRED for slow machines)

```powershell
$LogDir = "build-logs"
New-Item -ItemType Directory -Force -Path $LogDir | Out-Null
$LogFile = "$LogDir\build_$(Get-Date -Format 'yyyyMMdd_HHmmss').log"

cmake --build --preset default-release -j 8 2>&1 | Tee-Object -FilePath $LogFile
```

---

## Monitoring (CRITICAL)

### DO: Monitor via VS Code

- All build scripts pipe output to `/build-logs/*.log`
- Open logs in VS Code explorer to monitor progress
- Use VS Code tasks for automated builds
- Tail log files with VS Code extensions

### DON'T: Monitor in shell

- Never `Ctrl-C` a running build to check status
- Never use `Start-Sleep` followed by re-run
- Don't watch shell output directly (unreliable on slow machines)

### Log File Location

```text
/build-logs/
 build_20260108_134500.log
 build_20260108_140230.log
 ...
```

*Note: `/build-logs` is gitignored*

---

## Timeout Policy

### Local Builds

- Default timeout: **120 minutes** (slow machine)
- Use PowerShell `-TimeoutSeconds` or `Start-Process -Wait`
- Never terminate builds manually

### CI/CD (GitHub Actions)

```yaml
jobs:
 build:
 timeout-minutes: 120
```

---

## Verification Checklist

After each build:

- [ ] 0 errors in output
- [ ] 0 warnings in Release (enforced by `/WX`)
- [ ] Output files exist (CMake preset build):
  - `build/lib/ExplorerLensEngine.lib` — Core engine static library
  - `build/bin/EngineTests.exe` — Unit test executable
  - `build/bin/EngineBenchmark.exe` — Benchmark executable
- [ ] Output files exist (MSBuild):
  - `x64/Release/LENSShell.dll` (~2940 KB) — COM Shell Extension
  - `x64/Release/LENSManager.exe` (~400 KB) — GUI Configuration Utility
- [ ] Log file saved to `/build-logs`
- [ ] No build artifacts committed to Git

---

## Troubleshooting

### Build fails with LNK2001/LNK2019 (unresolved externals)

- Verify Engine library built first: `Engine\Release\ExplorerLensEngine.lib`
- Check AdditionalLibraryDirectories in LENSShell.vcxproj
- Ensure all external libs are x64: zlib, webp, avif, etc.

### Warning-as-error failures

- Fix the warning; do not suppress with pragmas
- Common: unused variables, narrowing conversions, missing override
- If external library warning: isolate with `/external:W0`

### Slow build times

- Enable `/MP` (multi-processor compilation) - already set
- Use Ninja instead of MSBuild for Engine: `cmake -G Ninja`
- Check disk I/O (SSD recommended)

---

## External Libraries

See [LIBRARY_INVENTORY.md](../../external/LIBRARY_INVENTORY.md) for complete version listing.

**Required x64 libraries:**

- zlib 1.3.1
- lz4 1.10.0
- zstd 1.5.7
- libwebp 1.5.0
- libavif 1.3.0
- dav1d 1.5.1
- libjxl 0.11.1
- minizip-ng 4.0.10
- lzma 25.00 (LZMA SDK)
- xz 5.6.3
- bzip2 1.0.8
- unrar 7.2.2
- libarchive 3.7.6

**Important:** All libraries MUST be built with
`-DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreadedDLL"` (dynamic CRT `/MD`)
to prevent LIBCMT conflicts.
See [REFACTOR_PLAN.md](../../REFACTOR_PLAN.md) for rebuild instructions.

**Exception:** libwebp was historically built with `/MT` (static CRT). The linker
flags `/NODEFAULTLIB:LIBCMT` in `Engine/CMakeLists.txt` handle this automatically
as a workaround.

---

## CRT Linkage Policy

All targets use **`/MD`** (dynamic CRT — `MultiThreadedDLL`). This is enforced via:

```cmake
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
```

| Component | CRT | Notes |
| ------------------------- | ----- | ---------------------------------------- |
| ExplorerLensEngine | `/MD` | Core library |
| ExplorerLensModernRuntime | `/MD` | Modern runtime library |
| EngineTests | `/MD` | Test executable |
| EngineBenchmark | `/MD` | Benchmark executable |
| IntegrationTests | `/MD` | Integration tests |
| LENSShell.dll | `/MD` | COM shell extension |
| LENSManager.exe | `/MD` | GUI utility |
| libwebp (external) | `/MT` | **Exception** — needs rebuild with `/MD` |

When external libs use `/MT` and project uses `/MD`, linker will warn about LIBCMT conflicts.
Current workaround: `/NODEFAULTLIB:LIBCMT` and `/IGNORE:4099` in CMake linker flags.

---

## CMakeLists.txt Maintenance Rules

### Registering New Files (MANDATORY)

1. **New headers** must be added to `ENGINE_HEADERS` in `Engine/CMakeLists.txt`
2. **New source files** must be added to `ENGINE_SOURCES` in `Engine/CMakeLists.txt`
3. **New test files** must be added to `Engine/Tests/CMakeLists.txt` EngineTests target
4. Missing registrations cause IDE IntelliSense issues and may break `install()` targets

### Insertion Points

- Core headers: before `# Pipeline` comment
- Core sources: before `# Pipeline implementations` comment
- Utils headers: before `# <!--` comment
- Utils sources: before closing `)`

### CMakePresets.json Notes

- Do NOT set `CMAKE_C_COMPILER` — project is C++ only, setting it triggers a CMake warning
- The `vcpkg` presets use `VCPKG_ROOT` toolchain file
- All presets use Ninja generator except `vs2026` which uses Visual Studio 18

---

## .gitignore Coverage

The following MUST be gitignored (verified February 2026):

- `build/` — CMake + Ninja build output
- `build-vcpkg/` — vcpkg preset build output (contains generated CMake cache/API files)
- `x64/` — MSBuild output
- `build-logs/` — Build log files
- `.vscode/c_cpp_properties.json` — Machine-local IntelliSense configuration
- `packages/` — NuGet packages
- External library build artifacts

---

## Known Issues & Troubleshooting

Build issue diagnostics (LNK2001, C4456, C4702, versionhelpers.h, CMake header
registration gaps, vcpkg.json naming, CMAKE_C_COMPILER warnings) are documented in full in
[build-troubleshooting.md](build-troubleshooting.md). Refer to that file rather than
repeating the fixes here.

---
## Build Validation Checklist

After CMake configuration changes:

- [ ] Clean build: `.\build-scripts\Build-MSVC.ps1 -Clean`
- [ ] Or manual: `cmake --preset default-release && cmake --build --preset default-release -j 8`
- [ ] Check for LNK4098 warnings (LIBCMT conflicts)
- [ ] Check for C4456 warnings (variable shadowing)
- [ ] Check for C4702 warnings (unreachable code)
- [ ] Grep build log: `Select-String -Path "build-logs\*.txt" -Pattern "FAILED|error C|warning C" -CaseSensitive`
- [ ] Run tests: `ctest --test-dir build -C Release --output-on-failure`
- [ ] Verify zero warnings AND zero errors in Release builds
- [ ] All 147/147 compilation steps complete

### Quick Verification Commands

```powershell
# Check for any errors/warnings in latest build log
$log = Get-ChildItem build-logs\*.txt | Sort-Object LastWriteTime | Select-Object -Last 1
Select-String -Path $log -Pattern "FAILED|error C|fatal error|warning C" -CaseSensitive

# Verify all build artifacts exist
@(
 "build/lib/ExplorerLensEngine.lib",
 "build/lib/ExplorerLensModernRuntime.lib",
 "build/bin/EngineTests.exe",
 "build/bin/EngineBenchmark.exe",
 "build/bin/IntegrationTests.exe"
) | ForEach-Object {
 $exists = Test-Path $_
 Write-Host "$_ : $exists" -ForegroundColor $(if($exists){"Green"}else{"Red"})
}
```
