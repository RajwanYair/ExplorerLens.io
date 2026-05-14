---
mode: agent
description: "Debug a failed ExplorerLens build. Reads build logs, identifies errors and warnings, suggests root causes, and provides fix strategies."
---

# Debug Build Failure — ExplorerLens

You are debugging a build failure in ExplorerLens. Follow this systematic procedure.

## Step 1: Read the Build Log

```powershell
# Latest build log
Get-Content build-logs\build-latest.log -Tail 100
# Or read specific log
Get-Content build-logs\{{logFile}} | Select-String "error C|error LNK|fatal error|warning C"
```

## Step 2: Classify the Error

| Error Code | Category | Common Cause |
| ------------ | ---------- | ------------- |
| `error C2065` | Undeclared identifier | Missing `#include` or wrong namespace |
| `error C2440` | Type conversion | Implicit narrowing; `std::span` mismatch |
| `error C2039` | Member not found | Typo in member name; wrong header included |
| `error C4996` | Deprecated API | Replace deprecated Windows/MSVC API |
| `error LNK2019` | Unresolved external | Missing `.cpp` in ENGINE_SOURCES; missing `#pragma comment(lib,...)` |
| `error LNK2005` | Multiple definitions | Same symbol defined in two `.cpp` files; LTCG ODR violation |
| `error LNK1169` | One or more multiply defined | Usually paired with LNK2005 — find the duplicate |
| `warning C4267` | size_t → int narrowing | Use explicit cast: `static_cast<int>(size)` |
| `warning C4244` | Narrowing conversion | Use explicit cast or change the type |

## Step 3: Common Root Causes and Fixes

### Missing #include (C2065, C2039)
```powershell
# Find where the missing type is defined
Select-String -Path Engine\**\*.h -Pattern "class MissingType|struct MissingType" -Recurse
# Add the correct #include to the failing file
```

### LNK2019 Unresolved External
```powershell
# Check if .cpp is in ENGINE_SOURCES
Select-String -Path Engine\CMakeLists.txt -Pattern "MissingSource.cpp"
# If missing, add to ENGINE_SOURCES
```

### Stale CMakeCache (after directory rename)
```powershell
Remove-Item build -Recurse -Force
.\build-scripts\Build-MSVC.ps1 -Clean
```

### Stale .obj files (hidden missing test bodies)
```powershell
Remove-Item build\**\*.obj -Force
.\build-scripts\Build-MSVC.ps1 -Clean -Test
```

### WIN32_LEAN_AND_MEAN exclusion
```cpp
// ❌ These headers fail under WIN32_LEAN_AND_MEAN
#include <versionhelpers.h>  // REMOVE — use RtlGetVersion() instead

// ✅ Use alternative
NTSTATUS status = RtlGetVersion(&osVersionInfo);
```

### GCC-only builtins (MSVC error)
```cpp
// ❌ GCC-only — won't compile on MSVC
__builtin_memcpy(dst, src, len);
// ✅ Use standard C
memcpy(dst, src, len);
```

### Windows macro conflict (min/max)
```cpp
// ❌ Expands to Windows macro
std::min(a, b)
// ✅ Use parenthesized form
(std::min)(a, b)
```

## Step 4: After Fix

```powershell
# Verify fix compiles cleanly
.\build-scripts\Build-MSVC.ps1 -Test
# Must produce: 0 errors, 0 warnings
```

## Step 5: Prevent Recurrence

- New headers → register in `ENGINE_HEADERS` in `Engine/CMakeLists.txt`
- New sources → register in `ENGINE_SOURCES`
- New Windows SDK includes → verify under `WIN32_LEAN_AND_MEAN`
- New test bodies → always do a clean build to catch stale `.obj` files
- New types → always `grep_search` for name collisions before adding

## Build Timings (Don't Panic)

| Phase | Normal Time |
| ------- | ------------- |
| CMake configure | 10-30s |
| EngineTests.cpp compile (~22K lines) | 60-90s |
| LTCG linking | 20-40s |
| Full clean build | 8-12 min |

If build appears stuck: check Task Manager for `cl.exe` or `ninja.exe` activity.
Do NOT kill the process unless it's been idle (0% CPU) for > 5 minutes.
