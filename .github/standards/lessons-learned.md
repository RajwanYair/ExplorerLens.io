# ExplorerLens — Engineering Lessons Learned

> Retrospective captured from git history (v15.1.0 through v34.7.0, 300+ commits).  
> Maintained by Copilot — update after every major sprint block.  
> Last updated: 2026-04-10 (v34.7.0 Arcturus-X, commit `314f29a0`)

---

## 1. CI/CD — Windows Runner Pitfalls

### 1.1 `git exit-128` on Windows Runners
**Symptom:** `actions/checkout` POST-STEP fails with exit code 128 after job completes.  
**Root cause:** `GITHUB_WORKSPACE` is owned by a different UID after the job runs; git refuses to operate.  
**Fix (apply to ALL Windows workflows):**
```yaml
- name: Configure git safe directory
  run: |
    git config --global --add safe.directory "$Env:GITHUB_WORKSPACE"
    git config --system --add safe.directory "$Env:GITHUB_WORKSPACE"
```
Add this **before** `actions/checkout`, not after. Required for `build.yml`, `release.yml`, `coverage.yml`, `codeql.yml`, and every other workflow with a Windows runner.  
**Commits:** `d0a02e91`, `8f56cde4`, `6bb8efe8`, `cd82a5ce`, `ad0c6299`

### 1.2 GIT_CONFIG_COUNT Environment Variable
**Issue:** Some actions inject `GIT_CONFIG_COUNT` without a matching `GIT_CONFIG_KEY_x` / `GIT_CONFIG_VALUE_x`, causing git to abort.  
**Fix:** Set `GIT_CONFIG_COUNT: 0` in the `env:` block of any step that calls git on Windows:
```yaml
env:
  GIT_CONFIG_COUNT: 0
```
**Commit:** `6bb8efe8`

### 1.3 Toolset Pinning in CI
**Rule:** **Never** pin `toolset: "14.50"` (or any specific toolset) in GitHub Actions workflows using `ilammy/msvc-dev-cmd@v1`. GitHub-hosted runners ship VS 2022 (v143); pinning a v145-only toolset causes immediate failure.  
**Local builds:** Use `Build-MSVC.ps1` which hard-codes `-vcvars_ver=14.50.35717` for local MSVC v145.  
**Commits:** `91354164`, `753bf8a1`

### 1.4 Node.js 24 Migration
**Issue:** GitHub deprecated Node.js 16/20 actions mid-cycle; `actions/upload-artifact@v3` etc. started emitting warnings then failing.  
**Fix:** Upgrade all action versions to those supporting Node.js 24:
- `actions/checkout@v4`, `actions/upload-artifact@v4`, `actions/download-artifact@v4`
- `github/codeql-action/analyze@v4` (not v3)
**Commit:** `d20bce9c`, `1e68928e`

### 1.5 Don't Remove FORCE_JAVASCRIPT_ACTIONS_TO_NODE24
**Lesson:** Adding `FORCE_JAVASCRIPT_ACTIONS_TO_NODE24: true` breaks `ilammy/msvc-dev-cmd`; remove it and upgrade actions to proper v4+ instead.  
**Commit:** `d20bce9c`

### 1.6 Engine Library Build Must Be Separate from EngineTests
**Issue:** Building EngineTests in the same CI job as the engine lib caused release blocking when tests failed.  
**Fix:** Split engine lib build and EngineTests into two CI jobs; gate releases on the lib build, not tests.  
**Commit:** `99b6ad06`

### 1.7 Continue-on-Error for Soft Failures
- `Build-external-libs` step: `continue-on-error: true` — static libs don't need all externals to produce a valid `.lib` for CI validation.
- `Build MSI installer` step: `continue-on-error: true` — MSI build may fail on missing WiX; DLL/EXE artifacts still valid.
- `Run Tests` step: `continue-on-error: true` with `always()` on artifact upload.  
**Commits:** `91860a93`, `ea747ac9`, `e4728e11`

---

## 2. Build System — MSVC/CMake Patterns

### 2.1 Stale CMakeCache.txt
**Symptom:** Build fails with "generator mismatch" or picks wrong compiler after toolchain change or directory rename.  
**Fix:** `Build-Library-Core.ps1` auto-detects stale cache; also run manually:
```powershell
Remove-Item -Path build\CMakeCache.txt -Force -ErrorAction SilentlyContinue
cmake --preset default-release
```
Always run `cmake --preset` before `build-and-log.bat` after a clean checkout or version bump.  
**Commits:** `d862ca42`, `c77be804`

### 2.2 PCH Order in MSBuild
**Rule:** In `.vcxproj` files, `stdafx.h` / PCH header must be listed **first** in the compiler inputs. WTL/ATL headers must come after the Windows SDK headers.  
**Symptom:** LNK2011 or C1010 on first compile of LENSShell/LENSManager.  
**Commit:** `753bf8a1`

### 2.3 Windows SDK Include Order
**Critical order (do not reorder):**
```cpp
#include <windows.h>     // must be first
#include <objidl.h>      // before gdiplus
#include <gdiplus.h>
```
GDI+, bcrypt.h, and other SDK headers are order-sensitive.  
**Commits:** `45d8ad07`, `8cbf1e23`

### 2.4 WIN32_LEAN_AND_MEAN — Excluded Headers
Because `WIN32_LEAN_AND_MEAN` is globally defined, these headers are **excluded and must never be included**:
- `<versionhelpers.h>` — use `RtlGetVersion()` from ntdll.dll instead
- Headers that pull in winsock/rpc/ole2 transitively if not needed  
**Commit:** `9c9d28f7`

### 2.5 Warning Suppressions Are Not Allowed
**Rule:** Never use `/wdXXXX` flags to suppress compiler warnings.  
Fix the root cause instead. A zero-warning build must be achieved by fixing the code, not hiding the warnings.  
**Key fixes applied:**
- `::tolower` → `static_cast<unsigned char>(c)` lambda in `std::transform`
- `1u << 64` → `1ull << 64` for 64-bit shifts
- `wchar_t` → `char` narrowing → explicit cast
- `GetVersionExW` → `RtlGetVersion()` (deprecated API fix)
- `[[maybe_unused]]` on intentionally-unused parameters  
**Commit:** `981b1387`

### 2.6 `__builtin_memcpy` Is a GCC-ism — Forbidden in MSVC
**Rule:** Never use `__builtin_memcpy`, `__builtin_expect`, or any `__builtin_*` GCC intrinsics. MSVC does not support them and will fail with `C3861: identifier not found`.  
**Fix:** Replace with `memcpy()`, `std::copy()`, or MSVC intrinsics as appropriate.  
**Pre-existing files affected:** `PQToSDRToneMapper.cpp`, `HLGToSDRConverter.cpp`, `ACESODTProcessor.cpp`

### 2.7 std::min / std::max Windows Macro Conflict
**Rule:** Always parenthesize `std::min` and `std::max` to prevent Windows macro expansion:
```cpp
size_t result = (std::min)(a, b);   // safe
size_t result = std::min(a, b);     // unsafe — expands to Windows min() macro
```
Add `#define NOMINMAX` before any Windows header if you need unqualified min/max.

### 2.8 Linker Flags Must Be MSVC-Guarded
**Rule:** `/NODEFAULTLIB`, `/IGNORE`, `/LTCG` and all other link.exe flags must be inside `if(MSVC)` in CMakeLists.txt:
```cmake
if(MSVC)
    target_link_options(target PRIVATE /NODEFAULTLIB:LIBCMT)
endif()
```

### 2.9 CRT Linkage Must Be /MD Throughout
All targets and all external libraries must use `/MD` (MultiThreadedDLL). `/MT` libraries cause LNK2038 CRT mismatch.  
libwebp was rebuilt from `/MT` to `/MD` at `d6dfa3e3`.

---

## 3. Code Quality — Recurring Patterns

### 3.1 IWYU — Include What You Use
**Rule:** Every `.cpp` file must `#include` every header it uses directly. Do not rely on transitive includes.  
**Catches:** Detected by clang-tidy in CI; causes C2065/C3861 when single-file builds are run.  
**Commits:** `cd82a5ce`, `e0898f5a`

### 3.2 Enum Values Must Be UPPER_CASE
**Rule (from .clang-tidy):** All enum values must be `UPPER_CASE`, not `CamelCase` or `snake_case`.
```cpp
enum class Status { PENDING, RUNNING, COMPLETE };   // correct
enum class Status { Pending, Running, Complete };   // wrong — clang-tidy will flag
```

### 3.3 Unique Type Names Across the Whole Engine
**Pattern:** In a codebase with 500+ headers, type name collisions are frequent. When adding a new type:
1. `grep_search` for the name across all headers before committing
2. Prefix with the module name if needed (e.g. `ScrubberCacheEngine` not `CacheEngine`)
3. Sprint 1151-1200 had a dedicated fix commit (`da3e482f`) just for naming conflicts — avoid this by grepping first.

### 3.4 `s_instance` Static Singleton Anti-Pattern
**Lesson:** Using `s_instance` as a static member name caused clang-tidy modernize-use-nodiscard and style issues. Prefer `instance` (no prefix) for public/protected static singletons.  
**Commit:** `5ab821e8`

### 3.5 ranges::lower_bound Over std::lower_bound
**Prefer:**
```cpp
auto it = std::ranges::lower_bound(container, value);   // range-based
// vs
auto it = std::lower_bound(container.begin(), container.end(), value);  // iterator-based (older)
```

### 3.6 Use `.at()` for Bounds-Checked Access
**Rule:** Use `container.at(key)` instead of `container[key]` when the key must exist; it throws `std::out_of_range` which is catchable.

### 3.7 clang-format-off for Generated/Protocol Headers
Headers with generated code, packed structs, or wire-protocol layouts must use:
```cpp
// clang-format off
// ... generated code ...
// clang-format on
```
Without this, clang-format rewrites whitespace in ways that break generated byte layouts.  
**Commit:** `3edfd64e`

---

## 4. Versioning & Release Process

### 4.1 Never Edit Version Files Manually
**Rule:** Always use `Bump-Version.ps1`. It updates all 21 version-bearing files atomically.  
Manual edits cause drift between files (e.g. RC files still showing old version while CMakeLists.txt shows new).

### 4.2 Idempotency Guard in Bump-Version.ps1
**Issue:** Running Bump-Version.ps1 twice in one session produced a duplicate CHANGELOG section.  
**Fix:** Added first-run idempotency check — the script now detects if the target version is already present in CHANGELOG.md and skips the prepend.  
**Commit:** `9425757d`

### 4.3 RC File Version Format
Both `LENSManager.rc` and `LENSShell.rc` have **four** version strings that must all match:
```
FILEVERSION X,Y,Z,0
PRODUCTVERSION X,Y,Z,0
VALUE "FileVersion", "X.Y.Z.0"
VALUE "ProductVersion", "X.Y.Z.0"
```
The comma-separated `FILEVERSION`/`PRODUCTVERSION` and quoted string versions are separate. All four must be updated.

### 4.4 CHANGELOG Markdownlint Discipline
**Recurring violations in CHANGELOG.md:**
- MD009: Trailing whitespace
- MD024: Duplicate heading text (e.g. two `### Fixed` at the same level)
- MD036: Emphasis used as a heading

Always run `markdownlint CHANGELOG.md` before committing. Use markdownlint-disable comments sparingly.  
**Commit:** `4d997143`

### 4.5 SVG Graphics Must Be Updated on Every Bump
`docs/assets/social-preview.svg` and `docs/assets/architecture-build.svg` must be updated to show the new version, test count, and codename. `Bump-Version.ps1` handles this automatically via regex substitution.  
**Commit (where this was added):** `3bbf6cef`

---

## 5. Testing Infrastructure

### 5.1 Test File Split Architecture
EngineTests are split across 5 files to keep each under ~500 KB:
| File | Contents |
|---|---|
| `EngineTestsIncludes.h` | All `#include` directives |
| `EngineTestsMacros.h` | `TEST()`, `ASSERT()`, `RUN_TEST()` macros + MockDecoder |
| `EngineTests.cpp` | Harness globals, `extern void` decls, `main()`, all `RUN_TEST()` calls |
| `EngineTests_Core.cpp` | Core/decoder/registry test bodies |
| `EngineTests_Features.cpp` | Feature module test bodies |
| `EngineTests_Mid.cpp` | Settings/memory/plugin/format test bodies |
| `EngineTests_Late.cpp` | CLI/workflow/AI/platform test bodies (latest sprints) |

**Rules:**
- New `#include` → `EngineTestsIncludes.h`
- New `extern void TestFoo();` + `RUN_TEST(TestFoo);` → `EngineTests.cpp`
- New `TEST(TestFoo) { ... }` bodies → `EngineTests_Late.cpp`

### 5.2 Stale .obj Artifacts Can Hide Missing Test Bodies
**Issue:** Sprint 1131-1140 test bodies were committed to `EngineTests_Mid.cpp` AFTER the `RUN_TEST()` calls had already been in `EngineTests.cpp` for an entire version. The old `.obj` file satisfied the linker — CI passed, but the tests were never actually running.  
**Prevention:** After adding `RUN_TEST()` entries, always do a clean build (`-Clean` flag) to verify the test bodies exist and link.  
**Commit:** `e0af7d23`

### 5.3 Test Count vs. Actual Running Tests
**Warning:** The `g_testsRun` counter only reflects tests that actually execute. If `RUN_TEST()` calls exist for test bodies not yet compiled, the count will be silently low. Verify by inspecting the `.cpp` split files.

### 5.4 Namespace Discrepancy — PerfRegressionGate
`PerfRegressionGate.h` uses `namespace ExplorerLens` (not `ExplorerLens::Engine`).  
Tests referencing it must use:
```cpp
using namespace ExplorerLens;   // not ExplorerLens::Engine
```
This is the only Engine header with this namespace pattern — document exceptions as they are found.

---

## 6. Architecture Decisions

### 6.1 Platform PAL Stubs Must Compile on All Platforms
`Engine/Platform/` contains stubs for macOS QuickLook and Linux Nautilus. These compile on Windows MSVC using `#ifdef _WIN32 / __APPLE__ / __linux__` guards. Never add macOS/Linux SDK calls without guards.  
**Commit:** `e0af7d23`

### 6.2 Consolidation Commits Are Dangerous
Large "consolidate" commits (removing 50-70 files at once) can:
- Drop working test bodies that look like stubs but actually run
- Delete `LEARNINGS.md` or other docs that have valuable historical content
- Remove `.cpp` stubs that, while stub implementations, are still required for compilation

**Rule:** Before any large consolidation, enumerate what will be deleted and verify nothing is load-bearing.  
**Commits:** `c7592264`, `c4444102`, `112ad659`

### 6.3 ExplorerLens.py Was Removed
The Python wrapper (`ExplorerLens.py`) was removed at commit `c4444102` as part of workspace consolidation. The C++ engine is the sole implementation.

### 6.4 COM CLSID Is Permanently Fixed
```
9E6ECB90-5A61-42BD-B851-D3297D9C7F39
```
This is registered in the Windows Registry and WiX installer. Changing it would break all existing installations. Never change it.  
**Commit (first correct fix):** `e719cba9`

### 6.5 LENSTYPE Enum Values Must Not Collide
LENSArchive.h contains the `LENSTYPE` enum. Before adding any new value, check the file for existing integer assignments. The enum is not sequential — gaps and explicit values are used.

---

## 7. Documentation Patterns

### 7.1 Header Banner Format (Standard)
```cpp
// FileName.h — Short Title
// Copyright (c) 2026 ExplorerLens Project
//
// Description of what this header provides.
//
#pragma once
```
No `=====` decorator lines, no version numbers, no sprint tags in headers.

### 7.2 Version-Bearing File Count
The complete registry is 21 files. The count is tracked in `version-bump.instructions.md`. When adding a new version-bearing file, update:
1. `Bump-Version.ps1` (add the update step)
2. `version-bump.instructions.md` (add to table)
3. The `$details` string at the end of `Bump-Version.ps1`
4. `copilot-instructions.md` (update the count)

### 7.3 LEARNINGS.md History
An early `LEARNINGS.md` was created at `f310d3c3` (v23.0.0→v24.0.0 retrospective) and extended at `ffc04c70` (v24.1.0 sections 11-14). It was later removed during the v33.0.0 consolidation (`c7592264`). Its content has been absorbed into this file.

---

## 8. External Libraries

### 8.1 Unicode Characters Break PowerShell Build Scripts
Em-dash (U+2014) and arrow (→) characters in PowerShell `.ps1` scripts cause parsing failures on some systems.  
**Fix:** Replace with ASCII `--` and `->`.  
**Commit:** `5bf29710`

### 8.2 All Libraries Must Be Built with /MD
Any external library linked against the engine must be compiled with `/MD` (dynamic CRT). Mixing `/MT` libraries causes `LNK2038` (CRT mismatch).  
Check with: `dumpbin /DIRECTIVES library.lib | findstr DEFAULTLIB`

### 8.3 vcpkg.json Has Two `"version"` Fields
1. The root `"version"` field for the vcpkg package version
2. The `"version"` inside `"overrides"` or `"dependencies"` for specific package pins

Both must be checked during a version bump.

---

## 9. Sprint Delivery Checklist (Copilot Reference)

For each sprint batch (5 headers + 5 sources + tests):

1. **Create files** — 5 `Engine/<Dir>/Feature.h` + 5 `Engine/<Dir>/Feature.cpp` (stub/forward impl)
2. **Register headers** — add to `Engine/CMakeLists.txt` ENGINE_HEADERS list
3. **Register sources** — add to `Engine/CMakeLists.txt` ENGINE_SOURCES list
4. **Add includes** — add `#include` for each new header to `Engine/Tests/EngineTestsIncludes.h`
5. **Add extern decls** — add `extern void TestFeature();` to `Engine/Tests/EngineTests.cpp`
6. **Add RUN_TEST calls** — add `RUN_TEST(TestFeature);` to `Engine/Tests/EngineTests.cpp`
7. **Add test bodies** — add `TEST(TestFeature) { ... }` to `Engine/Tests/EngineTests_Late.cpp`
8. **Verify clean build** — `.\build-scripts\Build-MSVC.ps1 -Clean -Test`
9. **Version bump** — `.\build-scripts\Bump-Version.ps1 -Version "X.Y.Z" ... -TagAndPush`

**Verify before commit:**
- `grep_search` for any new type names to catch collisions
- Confirm all `extern void` declarations have matching bodies in `_Late.cpp`
- Confirm `EngineTests.cpp` is under 500 KB (if over, split to `_Late.cpp`)

---

## 10. Recurring Commit Fix Patterns (Quick Reference)

| Problem | Fix Pattern | Key Commit |
|---------|-------------|------------|
| git exit-128 on Windows CI | `safe.directory` in global + system git config | `d0a02e91` |
| StaleGenerator CMake error | Delete CMakeCache.txt, re-run cmake --preset | `d862ca42` |
| /wdXXXX warning suppression in CMake | Remove /wd flags, fix the underlying warning | `981b1387` |
| __builtin_memcpy in MSVC | Replace with memcpy() | `3edfd64e` |
| std::min/max Windows macro conflict | Use (std::min)(a, b) form | `e0898f5a` |
| Enum CamelCase clang-tidy error | Rename all values to UPPER_CASE | `cd82a5ce` |
| Duplicate CHANGELOG entry | Bump-Version.ps1 idempotency guard | `9425757d` |
| Missing test body (stale .obj) | Clean rebuild, then add body to _Late.cpp | `e0af7d23` |
| RC version mismatch | Update all 4 version strings in .rc file | `753bf8a1` |
| versionhelpers.h + WIN32_LEAN_AND_MEAN | Remove include, use RtlGetVersion() | `9c9d28f7` |
| gdiplus.h include order error | Add objidl.h before gdiplus.h | `45d8ad07` |
| Node.js 16/20 deprecated in CI | Upgrade all actions to v4 | `d20bce9c` |
| Type redefinition from co-inclusion | Rename conflicting type, add prefix | `da3e482f` |
| PCH order in MSBuild | Ensure stdafx.h is first in vcxproj | `753bf8a1` |
| MARKDOWNLINT MD009/MD024/MD036 | Fix trailing spaces, dup headings | `4d997143` |
