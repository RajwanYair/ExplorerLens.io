# ExplorerLens Build Method - Standard Operating Procedure

**Last Updated:** February 2026
**Version:** 15.0.0 (Codename: Zenith)
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
- **Compilation Units**: 147 total (Engine lib ~108 + tests/benchmarks ~39)

### Tool Paths (Auto-detected on this machine)

```powershell
# Visual Studio Build Tools 18.3.0 (2026) — MSVC v145 toolset
$VSPath   = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools"
$MSBuild  = "$VSPath\MSBuild\Current\Bin\amd64\MSBuild.exe"
$vcvars64 = "$VSPath\VC\Auxiliary\Build\vcvars64.bat"   # Preferred over vcvarsall
$cl       = "$VSPath\VC\Tools\MSVC\14.50.35717\bin\Hostx64\x64\cl.exe"  # v19.50.35720

# CMake 4.2.3 (via Scoop)
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
WIN32_LEAN_AND_MEAN   — Excludes rarely-used Windows headers (impacts header compatibility!)
NOMINMAX              — Prevents min/max macro conflicts with STL
_WIN32_WINNT=0x0A00   — Targets Windows 10+
WINVER=0x0A00         — Targets Windows 10+
UNICODE / _UNICODE    — Unicode build
```

> **WARNING**: `WIN32_LEAN_AND_MEAN` excludes many Windows SDK sub-headers
> (e.g., `<mmsystem.h>`, `<winsock2.h>`).
> Headers like `<versionhelpers.h>` that depend on types from excluded headers
> will fail to compile.
> See [BUILD_TROUBLESHOOTING.md](BUILD_TROUBLESHOOTING.md) Issue #4 for details.

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

| Preset            | Generator | Compiler  | Dependencies    |
| ----------------- | --------- | --------- | --------------- |
| `default-release` | Ninja     | MSVC v145 | Local external/ |
| `default-debug`   | Ninja     | MSVC v145 | Local external/ |
| `vcpkg-release`   | Ninja     | MSVC v145 | vcpkg           |
| `vcpkg-debug`     | Ninja     | MSVC v145 | vcpkg           |
| `vs2026`          | VS 18     | MSVC v145 | Local external/ |

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
  - `build/lib/ExplorerLensModernRuntime.lib` — Modern runtime static library
  - `build/bin/EngineTests.exe` — Unit test executable
  - `build/bin/EngineBenchmark.exe` — Benchmark executable
  - `build/bin/IntegrationTests.exe` — Integration test executable
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

| Component                 | CRT   | Notes                                    |
| ------------------------- | ----- | ---------------------------------------- |
| ExplorerLensEngine        | `/MD` | Core library                             |
| ExplorerLensModernRuntime | `/MD` | Modern runtime library                   |
| EngineTests               | `/MD` | Test executable                          |
| EngineBenchmark           | `/MD` | Benchmark executable                     |
| IntegrationTests          | `/MD` | Integration tests                        |
| LENSShell.dll             | `/MD` | COM shell extension                      |
| LENSManager.exe           | `/MD` | GUI utility                              |
| libwebp (external)        | `/MT` | **Exception** — needs rebuild with `/MD` |

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
- Utils headers: before `# Sprint 8-12:` comment
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

## Recent Build Fixes (February 2026)

### Issue #4: versionhelpers.h + WIN32_LEAN_AND_MEAN Conflict (CRITICAL)

**Symptom:**

```text
MSIXPackageManager.cpp
fatal error C1083: Cannot open include file: 'versionhelpers.h'
-- OR --
error C2065: 'WORD': undeclared identifier (from versionhelpers.h)
error C2065: 'BOOL': undeclared identifier
error C3861: 'IsWindows10OrGreater': identifier not found
```

**Root Cause:**
`WIN32_LEAN_AND_MEAN` is globally defined in `Engine/CMakeLists.txt`. This
preprocessor define excludes many Windows SDK sub-headers that
`<versionhelpers.h>` depends on for its type definitions (`WORD`, `BOOL`,
`OSVERSIONINFOEXW`, etc.). Even changing include order does not fix this —
the types are excluded at the preprocessor level.

**Fix Applied:**
Removed `<versionhelpers.h>` entirely from `Engine/Utils/MSIXPackageManager.cpp`.
Replaced `IsWindows10OrGreater()` with direct `RtlGetVersion()` calls via
`ntdll.dll`:

```cpp
// Instead of: #include <versionhelpers.h> + IsWindows10OrGreater()
// Use: Direct RtlGetVersion via ntdll.dll (always available on Windows)
typedef NTSTATUS(WINAPI* RtlGetVersionFunc)(PRTL_OSVERSIONINFOW);

bool IsWindows10OrGreaterViaRtl() {
    HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
    if (!ntdll) return false;
    auto pRtlGetVersion = (RtlGetVersionFunc)GetProcAddress(ntdll, "RtlGetVersion");
    if (!pRtlGetVersion) return false;
    RTL_OSVERSIONINFOW osvi = {};
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    if (pRtlGetVersion(&osvi) != 0) return false;
    return (osvi.dwMajorVersion >= 10);
}
```

**Key Learning:**

> **NEVER use `<versionhelpers.h>` in any file when `WIN32_LEAN_AND_MEAN` is
> globally defined.** Use `RtlGetVersion()` from `ntdll.dll` instead — it's
> always available and doesn't depend on excluded Windows SDK types. This is
> also more reliable than `GetVersionEx()` which is deprecated and returns
> incorrect results on Windows 10+ without a manifest.

### Issue #5: Unused Includes Waste Compilation Time

**Files cleaned (February 2026):**

- `Engine/Utils/MSIXPackageManager.cpp`: Removed `<sstream>` (unused)
- `Engine/Utils/ConfigMigrationEngine.cpp`: Removed `<fstream>` (unused)
- `Engine/Core/ConfigMigrationEngine.h`: Removed `<functional>` (unused)

**Best Practice:**

- Run periodic include audits; each unused `#include` adds compilation time
- Prefer forward declarations over includes in headers
- Use `#pragma once` consistently (already enforced)

### Issue #6: Missing CMake Header Registrations

**Symptom:** Headers compile fine (found via `#include` paths) but aren't
listed in CMakeLists.txt `ENGINE_HEADERS`. This causes:

- Files missing from IDE project views
- `install()` targets not shipping the headers
- Potential issues with dependency tracking

**Fix Applied:** Registered 17 missing ReleaseGateV16-V32 headers in
`Engine/CMakeLists.txt` ENGINE_HEADERS.

**Prevention Rule:** When creating any new `.h` file under `Engine/`, always
add it to `ENGINE_HEADERS` in the same commit.

### Issue #7: vcpkg.json Name Must Be Lowercase

**Symptom:** vcpkg manifest validation warning when package name contains uppercase.

**Fix:** Changed `"name": "ExplorerLens"` to `"name": "explorerlens"` in `vcpkg.json`.

### Issue #8: CMAKE_C_COMPILER Warning

**Symptom:** CMake warning during configuration: `CMAKE_C_COMPILER is not used by this project`.

**Fix:** Removed `CMAKE_C_COMPILER` from `CMakePresets.json` — project is C++ only.

### Issue #1: LNK4098 - LIBCMT Conflict in Test Executables

**Symptom:**

```text
LINK : warning LNK4098: defaultlib 'LIBCMT' conflicts with use of other libs;
use /NODEFAULTLIB:library [EngineBenchmark.vcxproj]
```

**Root Cause:**

- Main Engine library had `/NODEFAULTLIB:LIBCMT` linker option
- Test executables (EngineTests, EngineBenchmark) did NOT have the flag
- External libraries (zlib, webp, zstd, minizip-ng) built with static CRT (`/MT`)
- Tests link both Engine (using `/MD`) and external libs (using `/MT`) causing conflict

**Fix Applied:**
Added `/NODEFAULTLIB:LIBCMT` and `/IGNORE:4099` to test executables in
`Engine/Tests/CMakeLists.txt`:

```cmake
if(MSVC)
    target_link_options(EngineTests PRIVATE
        /NODEFAULTLIB:LIBCMT    # Ignore static CRT conflicts from external libs
        /IGNORE:4099            # Ignore missing PDB warnings
    )
    target_link_options(EngineBenchmark PRIVATE
        /NODEFAULTLIB:LIBCMT    # Ignore static CRT conflicts from external libs
        /IGNORE:4099            # Ignore missing PDB warnings
    )
endif()
```

**Proper Long-Term Solution:**
Rebuild ALL external libraries with `/MD` dynamic CRT using `Rebuild-All-With-MD.ps1` script.
Current fix is temporary workaround that suppresses warning without fixing root cause.

### Issue #2: C4456 - Variable Shadowing Warning

**Symptom:**

```text
EngineBenchmark.cpp(300,19): warning C4456: declaration of 'iterations' hides
previous local declaration
```

**Root Cause:**

- Outer scope at line 237: `const int iterations = 10000;`
- Inner loop reusing same name could cause shadowing

**Fix Applied:**
Verified code uses distinct variable names:

- Line 237: `const int iterations = 10000;` (for format detection)
- Line 300: `const int scaleIterations = 10;` (for SIMD benchmarks)
- No shadowing in current code - warning from previous version

**Best Practice:**
Use descriptive variable names with prefixes indicating scope:

- `formatIterations`, `scaleIterations`, `cacheIterations` etc.
- Avoid reusing generic names like `i`, `j`, `count`, `iterations` in nested scopes

### Issue #3: C4702 - Unreachable Code Warning

**Symptom:**

```text
EngineTests.cpp(760): warning C4702: unreachable code
```

**Root Cause:**
Compiler optimization detecting code paths that can never execute, typically after:

- Unconditional return/throw statements
- All branches of if/else returning
- Infinite loops

**Fix Applied:**
Could not reproduce in current codebase - likely fixed in recent commits.
Common causes in test code:

- Code after ASSERT macros (which throw on failure)
- Debug-only code after return statements
- Unreachable catch blocks

**Best Practice:**

- Review compiler warnings during code generation phase (not just compilation)
- Remove dead code immediately
- Use `[[maybe_unused]]` attribute for intentionally unused variables

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

