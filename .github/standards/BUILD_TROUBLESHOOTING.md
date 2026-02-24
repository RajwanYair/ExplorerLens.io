# ExplorerLens Build Troubleshooting Guide

**Last Updated:** February 2026
**Applies To:** v14.0.0 "Apex", VS 18 2026 Build Tools, MSVC v145 toolset

---

## Issue #1: Windows SDK Header Compatibility with WIN32_LEAN_AND_MEAN

### Symptom

```text
fatal error C1083: Cannot open include file: 'versionhelpers.h'
-- OR --
error C2065: 'WORD': undeclared identifier (from versionhelpers.h)
error C2065: 'BOOL': undeclared identifier
error C3861: 'IsWindows10OrGreater': identifier not found
```

### Root Cause

`WIN32_LEAN_AND_MEAN` is globally defined in `Engine/CMakeLists.txt`.
This excludes many Windows SDK sub-headers. The `<versionhelpers.h>` header
depends on types (`WORD`, `BOOL`, `OSVERSIONINFOEXW`) from these excluded headers.

### Headers Incompatible with WIN32_LEAN_AND_MEAN

| Header                | Excluded Types / Subsystem           | Alternative               |
| --------------------- | ------------------------------------ | ------------------------- |
| `<versionhelpers.h>`  | `WORD`, `RTL_OSVERSIONINFOW`         | `RtlGetVersion()` direct  |
| `<mmsystem.h>`        | Multimedia types                     | Include after windows.h   |
| `<winsock2.h>`        | Socket types                         | Include before windows.h  |
| `<shellapi.h>`        | Shell types (partial)                | Usually works, test first |
| `<commdlg.h>`         | Common dialog types                  | Include after windows.h   |

### Solution Applied

Use `RtlGetVersion()` from ntdll.dll instead of `<versionhelpers.h>`:

```cpp
typedef NTSTATUS(WINAPI* RtlGetVersionFunc)(PRTL_OSVERSIONINFOW);
bool IsWindows10OrGreaterViaRtl() {
    HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
    if (!ntdll) return false;
    auto pRtlGetVersion = (RtlGetVersionFunc)GetProcAddress(ntdll, "RtlGetVersion");
    if (!pRtlGetVersion) return false;
    RTL_OSVERSIONINFOW osvi = {};
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    return (pRtlGetVersion(&osvi) == 0 && osvi.dwMajorVersion >= 10);
}
```

### Prevention Rule

> **NEVER** include `<versionhelpers.h>` in any source file. Always verify new
> Windows SDK includes compile under `WIN32_LEAN_AND_MEAN` before committing.

---

## Issue #2: CRT Linkage Conflicts (LNK4098)

### Symptom

```text
LINK : warning LNK4098: defaultlib 'LIBCMT' conflicts with use of other libs;
use /NODEFAULTLIB:library
```

### Root Cause

Project uses `/MD` (dynamic CRT) but some external libraries (libwebp, older
builds) use `/MT` (static CRT). The linker sees both `MSVCRT.lib` (from `/MD`)
and `LIBCMT.lib` (from `/MT`) and warns about the conflict.

### Solution (Workaround)

```cmake
# In CMakeLists.txt for targets that link mixed-CRT libraries:
if(MSVC)
    target_link_options(MyTarget PRIVATE
        /NODEFAULTLIB:LIBCMT    # Suppress static CRT from /MT libs
        /IGNORE:4099            # Suppress missing PDB warnings
    )
endif()
```

### Proper Solution

Rebuild all external libraries with matching CRT:

```powershell
# Rebuild all libs with /MD (dynamic CRT)
.\build-scripts\Rebuild-All-With-MD.ps1
```

### Applied Locations

- `Engine/CMakeLists.txt` — ExplorerLensEngine target
- `Engine/Tests/CMakeLists.txt` — EngineTests, EngineBenchmark targets

### Important Notes

- Linker flags like `/NODEFAULTLIB` and `/IGNORE` are MSVC-specific
- Always guard with `if(MSVC)` in CMake
- Never use these flags with Clang or GCC toolchains

---

## Issue #3: CMake Configuration Issues

### 3a: Wrong Compiler Selected

**Symptom:**

```text
-- The CXX compiler identification is Clang 19.1.7
```

**Expected:** `MSVC 19.50.35720.0`

**Cause:** `vcvars64.bat` not sourced before running CMake. Clang from PATH
takes priority.

**Fix:**

```powershell
# Always source vcvars first:
cmd /c '"C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat" -vcvars_ver=14.50.35717 && powershell'

# Or use the automated script (handles vcvars internally):
.\build-scripts\Build-MSVC.ps1
```

### 3b: Stale CMake Cache

**Symptom:** CMake remembers old compiler/paths after toolchain changes.

**Fix:**

```powershell
# Delete build directory and reconfigure
Remove-Item -Recurse -Force build
cmake --preset default-release
cmake --build --preset default-release -j 8
```

### 3c: CMAKE_C_COMPILER Warning

**Symptom:**

```text
CMake Warning: Manually-specified variables were not used by the project: CMAKE_C_COMPILER
```

**Cause:** Project is C++ only. Setting `CMAKE_C_COMPILER` in presets triggers this.

**Fix:** Remove `CMAKE_C_COMPILER` from `CMakePresets.json`. Only `CMAKE_CXX_COMPILER` is needed.

---

## Issue #4: Compiler Warnings (W4 + WX)

### 4a: C4456 — Variable Shadowing

```text
warning C4456: declaration of 'x' hides previous local declaration
```

**Fix:** Rename inner variable to avoid shadowing:

```cpp
// BAD
int iterations = 100;
for (int iterations = 0; iterations < 10; ++iterations) { ... }  // shadows!

// GOOD
int totalIterations = 100;
for (int i = 0; i < 10; ++i) { ... }
```

### 4b: C4702 — Unreachable Code

```text
warning C4702: unreachable code
```

**Fix:** Remove dead code after return/throw/break statements.

### 4c: C4100 — Unreferenced Formal Parameter

```text
warning C4100: 'param': unreferenced formal parameter
```

**Fix:**

```cpp
// Option 1: Comment out parameter name
void MyFunc(int /*unused*/) { }

// Option 2: Use [[maybe_unused]] attribute
void MyFunc([[maybe_unused]] int param) { }
```

### 4d: External Library Warnings

**Fix:** Use MSVC's `/external:` mechanism:

```cmake
# Suppress warnings from external headers
target_compile_options(MyTarget PRIVATE /external:anglebrackets /external:W0)
```

---

## Issue #5: Git & Source Control

### 5a: Release/ Pattern in .gitignore

**Symptom:** Files under `Engine/Release/` are ignored by Git.

**Cause:** `.gitignore` has a `Release/` pattern that matches any `Release/` directory.

**Fix:** Use `git add -f <file>` to force-add files under ignored paths, or make
the `.gitignore` pattern more specific (e.g., `x64/Release/`).

### 5b: Build Artifacts Committed Accidentally

**Prevention Checklist:**

```powershell
# Before committing, verify no build artifacts:
git status
git diff --cached --name-only | Select-String "\.obj$|\.lib$|\.dll$|\.exe$|\.pdb$"
```

---

## Issue #6: Build Script Issues

### 6a: Build-Library-Core.ps1 Cannot Find vcvars

**Symptom:**

```text
ERROR: Could not find vcvars64.bat
```

**Fix:** Verify Visual Studio Build Tools installation path matches the script's search paths. The script checks:

1. `$env:VS_PATH` environment variable
2. Standard VS installation directories
3. vswhere.exe output

### 6b: External Library Build Fails

**Common causes:**

- Missing prerequisite library (e.g., libavif needs zlib + dav1d)
- Wrong CRT linkage (`/MT` vs `/MD`)
- Stale CMake cache in library build directory

**Fix:**

```powershell
# Clean and rebuild individual library
.\build-scripts\external-libs\Build-<Library>.ps1 -Clean
```

---

## Quick Diagnostic Commands

```powershell
# Check compiler version
cl.exe 2>&1 | Select-String "Version"
# Expected: Microsoft (R) C/C++ Optimizing Compiler Version 19.50.35720

# Check CMake version and generator
cmake --version
cmake --preset default-release -N 2>&1 | Select-String "generator|compiler"

# Verify vcvars was sourced (check for MSVC env vars)
$env:VSCMD_VER  # Should show "18.x.y"
$env:VCToolsVersion  # Should show "14.50.35717"

# Find all errors/warnings in build logs
Get-ChildItem build-logs\*.log, build-logs\*.txt |
    Select-String -Pattern "error C|fatal error|warning C|FAILED" -CaseSensitive |
    Select-Object Path, LineNumber, Line

# Check if external libraries are built
@(
    "external/compression-libs/zlib-1.3.1/install/lib/zlibstatic.lib",
    "external/image-libs/libwebp-1.5.0-original/output/x64/Release/webp.lib",
    "external/image-libs/libavif-1.3.0/install/lib/avif.lib",
    "external/image-libs/dav1d-1.5.1/build/src/libdav1d.a"
) | ForEach-Object {
    $ok = Test-Path $_
    Write-Host "$_ : $ok" -ForegroundColor $(if($ok){"Green"}else{"Red"})
}
```

---

## Appendix: Header Registrations

All headers under `Engine/` must be registered in `Engine/CMakeLists.txt` ENGINE_HEADERS.
The following headers were added in the February 2026 cleanup:

| Header                        | Subsystem | Sprint     |
| ----------------------------- | --------- | ---------- |
| `Utils/ReleaseGateV16.h`      | Utils     | Sprint 299 |
| `Utils/ReleaseGateV17.h`      | Utils     | Sprint 301 |
| `Utils/ReleaseGateV18.h`      | Utils     | Sprint 303 |
| `Utils/ReleaseGateV19.h`      | Utils     | Sprint 305 |
| `Utils/ReleaseGateV20.h`      | Utils     | Sprint 307 |
| `Utils/ReleaseGateV21.h`      | Utils     | Sprint 309 |
| `Utils/ReleaseGateV22.h`      | Utils     | Sprint 311 |
| `Utils/ReleaseGateV23.h`      | Utils     | Sprint 313 |
| `Utils/ReleaseGateV24.h`      | Utils     | Sprint 315 |
| `Utils/ReleaseGateV25.h`      | Utils     | Sprint 317 |
| `Utils/ReleaseGateV26.h`      | Utils     | Sprint 319 |
| `Utils/ReleaseGateV27.h`      | Utils     | Sprint 321 |
| `Utils/ReleaseGateV28.h`      | Utils     | Sprint 323 |
| `Utils/ReleaseGateV29.h`      | Utils     | Sprint 325 |
| `Utils/ReleaseGateV30.h`      | Utils     | Sprint 327 |
| `Utils/ReleaseGateV31.h`      | Utils     | Sprint 329 |
| `Utils/ReleaseGateV32.h`      | Utils     | Sprint 331 |

> **Rule:** When creating a new `.h` file under `Engine/`, add it to
> `ENGINE_HEADERS` in the same commit.
