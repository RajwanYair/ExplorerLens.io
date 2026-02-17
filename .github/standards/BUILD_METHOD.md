# DarkThumbs Build Method - Standard Operating Procedure

**Last Updated:** January 8, 2026  
**Policy:** 64-bit only, warnings-as-errors in Release, VS Code monitoring

---

## Build Architecture

### Supported Configurations
- **Platform**: x64 ONLY (32-bit builds are not supported)
- **Configurations**: Debug, Release
- **Toolchain**: Visual Studio 2026 Build Tools (v145), MSVC 19.3+
- **Build Systems**: 
  - MSBuild for shell extension (CBXShell.sln)
  - CMake + Ninja for Engine library

### Tool Paths (Auto-detected on this machine)
```powershell
# Visual Studio Build Tools 18.3.0 (2026)
$VSPath = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools"
$MSBuild = "$VSPath\MSBuild\Current\Bin\amd64\MSBuild.exe"
$vcvarsall = "$VSPath\VC\Auxiliary\Build\vcvarsall.bat"

# CMake 4.2.3 (via Scoop)
$CMake = "C:\Users\ryair\scoop\shims\cmake.exe"

# Ninja 1.13.2 (via Scoop)
$Ninja = "C:\Users\ryair\scoop\shims\ninja.exe"
```

**Note:** Tools are auto-detected by `build-scripts\Find-All-Tools.ps1`. If not in PATH, the script searches standard installation locations.

### Compiler Flags
**Release configuration:**
- `/W4` - Warning level 4
- `/WX` - Treat warnings as errors (enforced)
- `/O2` - Optimize for speed
- `/GL` - Whole program optimization
- `/arch:AVX2` - AVX2 instruction set
- `/std:c++20` - C++20 standard
- `/MT` - Static runtime (or `/MD` for DLL runtime)

**Debug configuration:**
- `/W4` - Warning level 4
- `/Od` - No optimization
- `/Zi` - Full debug info
- `/MTd` or `/MDd` - Debug runtime

---

## Build Process

### Clean Build (Recommended)
```powershell
# Clean all build artifacts
Remove-Item -Recurse -Force build, x64, CBXShell\x64, CBXManager\x64, Engine\Release -ErrorAction SilentlyContinue

# Build Engine library first
cd Engine
cmake -S . -B Release -G "Visual Studio 18 2026" -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build Release --config Release

# Build shell extension
cd ..
msbuild CBXShell.sln /p:Configuration=Release /p:Platform=x64 /m /v:minimal
```

### Incremental Build
```powershell
msbuild CBXShell.sln /p:Configuration=Release /p:Platform=x64 /m /v:minimal
```

### With Logging (REQUIRED for slow machines)
```powershell
$LogDir = "build-logs"
New-Item -ItemType Directory -Force -Path $LogDir | Out-Null
$LogFile = "$LogDir\build_$(Get-Date -Format 'yyyyMMdd_HHmmss').log"

msbuild CBXShell.sln /p:Configuration=Release /p:Platform=x64 /m /v:minimal 2>&1 | Tee-Object -FilePath $LogFile
```

---

## Monitoring (CRITICAL)

### ✅ DO: Monitor via VS Code
- All build scripts pipe output to `/build-logs/*.log`
- Open logs in VS Code explorer to monitor progress
- Use VS Code tasks for automated builds
- Tail log files with VS Code extensions

### ❌ DON'T: Monitor in shell
- Never `Ctrl-C` a running build to check status
- Never use `Start-Sleep` followed by re-run
- Don't watch shell output directly (unreliable on slow machines)

### Log File Location
```
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
- [ ] Output files exist:
  - `x64\Release\CBXShell.dll` (~1.3 MB)
  - `x64\Release\CBXManager.exe` (~300 KB)
  - `Engine\Release\DarkThumbsEngine.lib` (~1.9 MB)
- [ ] Log file saved to `/build-logs`
- [ ] No build artifacts committed to Git

---

## Troubleshooting

### Build fails with LNK2001/LNK2019 (unresolved externals)
- Verify Engine library built first: `Engine\Release\DarkThumbsEngine.lib`
- Check AdditionalLibraryDirectories in CBXShell.vcxproj
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

**Important:** All libraries MUST be built with `-DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreadedDLL"` (dynamic CRT `/MD`) 
to prevent LIBCMT conflicts. See [REFACTOR_PLAN.md](../../REFACTOR_PLAN.md) for rebuild instructions.

---

## Recent Build Fixes (February 2026)

### Issue #1: LNK4098 - LIBCMT Conflict in Test Executables

**Symptom:**
```
LINK : warning LNK4098: defaultlib 'LIBCMT' conflicts with use of other libs; 
use /NODEFAULTLIB:library [EngineBenchmark.vcxproj]
```

**Root Cause:**
- Main Engine library had `/NODEFAULTLIB:LIBCMT` linker option
- Test executables (EngineTests, EngineBenchmark) did NOT have the flag
- External libraries (zlib, webp, zstd, minizip-ng) built with static CRT (`/MT`)
- Tests link both Engine (using `/MD`) and external libs (using `/MT`) causing conflict

**Fix Applied:**
Added `/NODEFAULTLIB:LIBCMT` and `/IGNORE:4099` to test executables in `Engine/Tests/CMakeLists.txt`:
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
```
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
```
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
- [ ] Clean build: `cmake --build build --target clean`  
- [ ] Rebuild all: `cmake --build build --config Release -j 8`
- [ ] Check for LNK4098 warnings (LIBCMT conflicts)
- [ ] Check for C4456 warnings (variable shadowing)
- [ ] Check for C4702 warnings (unreachable code)
- [ ] Run tests: `ctest --test-dir build -C Release --output-on-failure`
- [ ] Verify zero warnings in Release builds
