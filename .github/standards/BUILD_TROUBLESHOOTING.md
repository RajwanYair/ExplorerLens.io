# ExplorerLens Build Troubleshooting Guide

**Last Updated:** June 2026  
**Version:** 14.0.0 (Codename: Apex)

This document captures all build issues encountered and their resolutions, organized as a knowledge base
for future reference. Each issue includes root cause analysis and prevention guidance.

---

## Table of Contents

1. [Windows SDK Header Compatibility](#1-windows-sdk-header-compatibility)
2. [CRT Linkage Conflicts](#2-crt-linkage-conflicts)
3. [CMake Configuration Issues](#3-cmake-configuration-issues)
4. [Compiler Warnings (W4/WX)](#4-compiler-warnings-w4wx)
5. [Git & Source Control](#5-git--source-control)
6. [Build Script Issues](#6-build-script-issues)
7. [Quick Diagnostic Commands](#7-quick-diagnostic-commands)

---

## 1. Windows SDK Header Compatibility

### 1.1 `<versionhelpers.h>` + `WIN32_LEAN_AND_MEAN` (CRITICAL)

**Severity:** Build-breaking  
**First seen:** June 2026  
**Affects:** Any file including `<versionhelpers.h>` when `WIN32_LEAN_AND_MEAN` is globally defined

**Symptom:**
```
error C2065: 'WORD': undeclared identifier
error C2065: 'BOOL': undeclared identifier
error C3861: 'IsWindows10OrGreater': identifier not found
```

**Root Cause:**  
`WIN32_LEAN_AND_MEAN` is defined globally in `Engine/CMakeLists.txt` as a compile definition. 
This macro causes `<windows.h>` to **exclude** many SDK sub-headers that `<versionhelpers.h>` depends on.
The missing types (`WORD`, `DWORD`, `BOOL`, `OSVERSIONINFOEXW`) are defined in those excluded headers.

Attempting to fix include order (putting `<windows.h>` before `<versionhelpers.h>`) does **NOT** work —
the types are excluded at the preprocessor level regardless of order.

**Solution:**  
Use `RtlGetVersion()` from `ntdll.dll` instead:
```cpp
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

**Prevention Rule:**
> **NEVER** include `<versionhelpers.h>` in any file in this project.  
> **NEVER** include any Windows SDK header that depends on types excluded by `WIN32_LEAN_AND_MEAN`.  
> Use `RtlGetVersion()` for OS version checks — it's more accurate than deprecated `GetVersionEx()` anyway.

**Affected headers to avoid (non-exhaustive):**
- `<versionhelpers.h>` — OS version checks
- `<mmsystem.h>` — Multimedia (include manually if needed)
- `<winsock2.h>` — Must be included BEFORE `<windows.h>` 
- `<ddeml.h>` — DDE messaging

### 1.2 Include Order for Windows Headers

**Rule:** When manually including Windows SDK sub-headers alongside `<windows.h>`:
1. `<winsock2.h>` (if needed) — MUST come before `<windows.h>`  
2. `<windows.h>` — Main header
3. `<winternl.h>` — If using NT internals
4. Other SDK headers (`<shlobj.h>`, `<shobjidl.h>`, etc.)

The project already defines `WIN32_LEAN_AND_MEAN`, `NOMINMAX`, `UNICODE`, `_UNICODE` globally.

---

## 2. CRT Linkage Conflicts

### 2.1 LNK4098 — LIBCMT Conflicts

**Severity:** Warning (becomes error with /WX)  
**First seen:** February 2026

**Symptom:**
```
LINK : warning LNK4098: defaultlib 'LIBCMT' conflicts with use of other libs
```

**Root Cause:**  
External libraries (especially libwebp) built with `/MT` (static CRT) while project uses `/MD` (dynamic CRT).
Mixing `/MT` and `/MD` in the same link produces this warning.

**Current Workaround:**
```cmake
if(MSVC)
    target_link_options(ExplorerLensEngine PRIVATE /NODEFAULTLIB:LIBCMT /IGNORE:4099)
    target_link_options(EngineTests PRIVATE /NODEFAULTLIB:LIBCMT /IGNORE:4099)
    target_link_options(EngineBenchmark PRIVATE /NODEFAULTLIB:LIBCMT /IGNORE:4099)
endif()
```

**Proper Fix:** Rebuild ALL external libraries with `/MD`:
```powershell
.\build-scripts\Rebuild-All-With-MD.ps1
```

**Prevention Rule:**
> All external library build scripts MUST use `-DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreadedDLL"`

### 2.2 Linker Flags Are MSVC-Only

**Rule:** Always guard linker flags with `if(MSVC)`:
```cmake
# CORRECT:
if(MSVC)
    target_link_options(MyTarget PRIVATE /NODEFAULTLIB:LIBCMT)
endif()

# WRONG (breaks Clang/GCC):
target_link_options(MyTarget PRIVATE /NODEFAULTLIB:LIBCMT)
```

---

## 3. CMake Configuration Issues

### 3.1 CMAKE_C_COMPILER Not Needed

**Severity:** Warning  
**Symptom:** `CMake Warning: CMAKE_C_COMPILER is not used by this project`

**Root Cause:** Project is C++ only — no C sources. Setting `CMAKE_C_COMPILER` in CMakePresets.json triggers warning.  
**Fix:** Remove `CMAKE_C_COMPILER` from presets. Only set `CMAKE_CXX_COMPILER`.

### 3.2 CMake Picks Clang Instead of MSVC

**Severity:** Build-breaking (different ABI, missing MSVC features)

**Symptom:** Build fails with Clang-specific errors or produces incompatible binaries.

**Root Cause:** Clang is in PATH (e.g., from LLVM install) and CMake finds it before MSVC.

**Fix:** Always source `vcvars64.bat` before running CMake:
```powershell
# Build-MSVC.ps1 does this automatically
# For manual builds:
cmd /c '"C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat" -vcvars_ver=14.50.35717 && powershell'
```

**Prevention Rule:**
> **NEVER** run `cmake` without sourcing vcvars64 first.  
> **ALWAYS** use `Build-MSVC.ps1` which handles this automatically.

### 3.3 Stale CMakeCache.txt After Directory Renames

**Severity:** Configuration failure

**Symptom:** CMake configure fails after renaming directories, with errors about missing paths.

**Fix:** `Build-Library-Core.ps1` auto-detects stale caches. For manual fix:
```powershell
Remove-Item build/CMakeCache.txt -Force
Remove-Item -Recurse build/CMakeFiles -Force
cmake --preset default-release  # Re-configure from scratch
```

### 3.4 vcpkg Manifest Name Must Be Lowercase

**Severity:** Warning/validation error

**Symptom:** vcpkg warns about non-lowercase package name.

**Fix:** In `vcpkg.json`, use lowercase: `"name": "explorerlens"` (not `"ExplorerLens"`).

---

## 4. Compiler Warnings (W4/WX)

### 4.1 C4456 — Variable Shadowing

**Symptom:** `warning C4456: declaration of 'x' hides previous local declaration`

**Prevention:**
- Use descriptive variable names: `formatIterations`, `scaleIterations` (not generic `iterations`)
- Avoid reusing names in nested scopes
- Prefix loop variables when nesting: `outerIdx`, `innerIdx`

### 4.2 C4702 — Unreachable Code

**Symptom:** `warning C4702: unreachable code`

**Common causes in this project:**
- Code after `ASSERT()` macros (which may throw/abort)
- Code after unconditional `return` statements
- Dead branches in `switch` statements after `default:` with return

**Fix:** Remove the unreachable code. Use `[[maybe_unused]]` for intentionally unused paths.

### 4.3 C4100 — Unused Parameters

**Fix:** Use `[[maybe_unused]]` or `(void)paramName;` to suppress.

### 4.4 C4189 — Unused Local Variables

**Fix:** Remove the variable or mark with `[[maybe_unused]]` if intentionally retained for debugging.

---

## 5. Git & Source Control

### 5.1 build-vcpkg/ Tracked Accidentally

**Symptom:** `build-vcpkg/.cmake/api/v1/reply/error-*.json` files (25+ files) show up in git status.

**Root Cause:** `build-vcpkg/` was not in `.gitignore`.

**Fix:** Added `build-vcpkg/` to `.gitignore` and removed tracked files:
```powershell
git rm -r --cached build-vcpkg/
echo "build-vcpkg/" >> .gitignore
```

### 5.2 .vscode/c_cpp_properties.json Is Machine-Local

**Rule:** `.vscode/c_cpp_properties.json` contains machine-specific IntelliSense paths.
It MUST be gitignored. Only `.vscode/tasks.json` and `.vscode/launch.json` should be tracked.

### 5.3 Engine/Release/ Blocked by .gitignore

The pattern `Release/` in `.gitignore` matches `Engine/Release/`. If you need to add files there:
```powershell
git add -f Engine/Release/needed-file.txt
```

---

## 6. Build Script Issues

### 6.1 NMake Build Scripts — Makefile Parameter

**Context:** Some external libraries (e.g., libwebp) use NMake with `Makefile.vc` instead of CMake.

**Pattern:** Use the `-Makefile` parameter in `Build-Library-Core.ps1`:
```powershell
# Build-LibWebP-NMake.ps1 uses:
Invoke-BuildStep "Build" {
    Build-WithNMake -SourceDir $srcDir -Makefile "Makefile.vc" -Args @("CFG=release-static")
}
```

### 6.2 Build-Library-Core.ps1 Root Dir Resolution

**Pattern:** All build scripts resolve the project root via:
```powershell
$rootDir = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
```
This assumes the script is two levels deep under the project root.

---

## 7. Quick Diagnostic Commands

### Check for Build Errors
```powershell
# In latest build log
$log = Get-ChildItem build-logs\*.txt | Sort-Object LastWriteTime | Select-Object -Last 1
Select-String -Path $log -Pattern "FAILED|error C|fatal error" -CaseSensitive
```

### Check for Build Warnings
```powershell
Select-String -Path $log -Pattern "warning C" -CaseSensitive
```

### Verify Build Artifacts
```powershell
@("build/lib/ExplorerLensEngine.lib",
  "build/lib/ExplorerLensModernRuntime.lib",
  "build/bin/EngineTests.exe",
  "build/bin/EngineBenchmark.exe",
  "build/bin/IntegrationTests.exe"
) | ForEach-Object {
    $exists = Test-Path $_
    Write-Host "$_ : $exists" -ForegroundColor $(if($exists){"Green"}else{"Red"})
}
```

### Check Compiler Version
```powershell
# After sourcing vcvars:
cl.exe 2>&1 | Select-String "Version"
# Expected: Microsoft (R) C/C++ Optimizing Compiler Version 19.50.35720
```

### Check CMake Configuration
```powershell
cmake --preset default-release 2>&1 | Select-String "Compiler|Version|Build type"
```

### Full Clean + Rebuild
```powershell
Remove-Item -Recurse -Force build, x64, LENSShell\x64, LENSManager\x64 -ErrorAction SilentlyContinue
.\build-scripts\Build-MSVC.ps1 -Clean
```

---

## Appendix: Header Registrations

Every new `.h` or `.cpp` file must be registered in CMake. Here's the mapping:

| File Location | Register In | Section |
|---|---|---|
| `Engine/Core/*.h` | `Engine/CMakeLists.txt` | `ENGINE_HEADERS` |
| `Engine/Core/*.cpp` | `Engine/CMakeLists.txt` | `ENGINE_SOURCES` |
| `Engine/Utils/*.h` | `Engine/CMakeLists.txt` | `ENGINE_HEADERS` |
| `Engine/Utils/*.cpp` | `Engine/CMakeLists.txt` | `ENGINE_SOURCES` |
| `Engine/Pipeline/*.h` | `Engine/CMakeLists.txt` | `ENGINE_HEADERS` |
| `Engine/Cache/*.h` | `Engine/CMakeLists.txt` | `ENGINE_HEADERS` |
| `Engine/Memory/*.h` | `Engine/CMakeLists.txt` | `ENGINE_HEADERS` |
| `Engine/Plugin/*.h` | `Engine/CMakeLists.txt` | `ENGINE_HEADERS` |
| `Engine/AI/*.h` | `Engine/CMakeLists.txt` | `ENGINE_HEADERS` |
| `Engine/GPU/*.h` | `Engine/CMakeLists.txt` | `ENGINE_HEADERS` |
| `Engine/Tests/*.cpp` | `Engine/Tests/CMakeLists.txt` | `EngineTests target` |

**Failure to register = files missing from IDE and install targets.**
