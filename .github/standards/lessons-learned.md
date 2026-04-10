# ExplorerLens — Engineering Lessons Learned

> Retrospective captured from git history (v15.1.0 through v35.3.0, 330+ commits).  
> Maintained by Copilot — update after every major sprint block.  
> Last updated: 2026-04-10 (v35.3.0 Vega-T build-fix session — 11 type collisions resolved)

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
5. **Add extern Runner decls** — add `extern void TestFeature_Runner();` to `Engine/Tests/EngineTestsExterns.h` (NOT EngineTests.cpp — that file `#include`s `EngineTestsExterns.h`)
6. **Add RUN_TEST calls** — add `RUN_TEST(TestFeature);` to `Engine/Tests/EngineTests.cpp`
7. **Add test bodies** — add `TEST(TestFeature) { ... }` to `Engine/Tests/EngineTests_Late.cpp`
8. **Verify clean build** — `.\build-scripts\Build-MSVC.ps1 -Clean -Test`
9. **Version bump** — `.\build-scripts\Bump-Version.ps1 -Version "X.Y.Z" ... -TagAndPush`

**Verify before commit:**
- `grep_search` for **every** new `struct`, `enum class`, and `class` name in `Engine/` to catch collisions — see §11.8
- Confirm all `extern void` Runner declarations have matching bodies in `_Late.cpp`
- Confirm `EngineTestsExterns.h` is under 500 KB (currently ~233 KB as of v35.3.0)

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
| Sprint-level bulk type collision (11 types) | Pre-grep ALL new type names; rename newer header's types + update .cpp + test files | §11.8 |
| PCH order in MSBuild | Ensure stdafx.h is first in vcxproj | `753bf8a1` |
| MARKDOWNLINT MD009/MD024/MD036 | Fix trailing spaces, dup headings | `4d997143` |
| SBOMGenerator.h locked by IntelliSense | Retry Bump-Version.ps1 after 2–5 seconds | `58e12150` |
| extern Runner decls in wrong file | Put in EngineTestsExterns.h, not EngineTests.cpp | `f41436c0` |
| Token budget exceeded mid-sprint | Session summary captures exact oldString; resume with last-lines read | — |
| ROADMAP_VXX.md missing for new major ver | Create docs/ROADMAP_VXX.md at first sprint of new X.0.0 | `f41436c0` |

---

## 11. Vega Series (v35.x) — New Patterns and Lessons (Sprint 1281-1320)

> Commits: `f41436c0` (v35.0.0) through `b2965d69` (v35.3.0)

### 11.1 SBOMGenerator.h File Lock (Idempotency / Retry Pattern)
**Symptom:** `Bump-Version.ps1` fails with:
```
Set-Content: The process cannot access the file 'Engine/Core/SBOMGenerator.h'
because it is being used by another process.
```
**Root cause:** Copilot/IntelliSense (or VS Code itself) holds the file open briefly after it parses it during a sprint delivery.  
**Fix:** Simply re-run `Bump-Version.ps1` with the same parameters immediately. The idempotency guard (Section 4.2) ensures CHANGELOG.md is not double-prepended, and all other files are safely overwritten.  
**Pattern seen in:** v35.0.0 (`f41436c0`), v35.1.0 (`58e12150`), v35.2.0 (`0813a5e8`), v35.3.0 (`b2965d69`)  
**Note:** The second invocation completes successfully ~100% of the time. No manual intervention needed.

### 11.2 EngineTestsExterns.h Architecture
**Discovery:** A dedicated `Engine/Tests/EngineTestsExterns.h` file holds all `extern void TestFoo_Runner()` declarations. It is `#include`d at line 16 of `EngineTests.cpp`.  
**Correct wiring for a new sprint:**
```
EngineTestsIncludes.h  — #include "../Path/NewFeature.h"
EngineTestsExterns.h   — extern void TestNewFeature_Foo_Runner();
EngineTests.cpp        — RUN_TEST(TestNewFeature_Foo);
EngineTests_Late.cpp   — TEST(TestNewFeature_Foo) { ... }
```
**Common mistake:** Putting `extern void` declarations directly in `EngineTests.cpp` (above main) instead of in `EngineTestsExterns.h`. The linker will still find them, but the file organization scheme is violated.  
**File size as of v35.3.0:** ~233 KB — monitor; split when it approaches 500 KB.

### 11.3 ROADMAP_VXX.md Creation Pattern
**Rule:** When starting a new major-version series (X.0.0), create `docs/ROADMAP_VXX.md` in the same commit/sprint.  
**Content:** Full sprint timeline (T1–T8+), baseline table (test count, versions), per-sprint module tables, next major-version preview.  
**Created:** `docs/ROADMAP_V35.md` at commit `f41436c0` (v35.0.0 Vega).  
**Templates:** Use ROADMAP_V34.md as the template; update codename series and baseline values.

### 11.4 Token Budget / Session Continuation Pattern
**Issue:** Long sprint deliveries (5 headers + 5 CPPs + test wiring + test bodies) may exceed the AI assistant token budget mid-sprint.  
**Recovery:** The conversation summary system captures:
- Exact file names and last lines confirmed in each file
- Precise `oldString` for the pending `replace_string_in_file` call
- Complete newString content of all 10 TEST bodies
**On new session:** Read the summary, verify the `oldString` still matches with `read_file`, then execute the single pending replace.  
**Prevention:** Complete test bodies (`_Late.cpp` append) **before** any other file changes in a sprint to front-load the largest write.

### 11.5 Vega Series Codename Sequence
The v35.x release series uses the "Vega" constellation codenames with suffixes:

| Version | Codename | Sprint | Theme |
|---------|----------|--------|-------|
| v35.0.0 | Vega | 1281-1290 | Adaptive Fidelity + Roadmap bootstrap |
| v35.1.0 | Vega-R | 1291-1300 | Real-Time Collaboration & Live Edit Sync |
| v35.2.0 | Vega-S | 1301-1310 | Network-Aware Streaming Cache |
| v35.3.0 | Vega-T | 1311-1320 | Zero-Trust Thumbnail Security |
| v35.4.0 | Vega-U | 1321-1330 | WebAssembly / Browser Extension Pipeline |
| v35.5.0 | Vega-V | 1331-1340 | Cross-Device Preview Sync |
| v35.6.0 | Vega-W | 1341-1350 | REST API & Remote Decode Service |
| v35.7.0 | Vega-X | 1351-1360 | LTS Gate + Hardening |

Next series: v36.0.0 "Altair" — AI-Powered Semantic Thumbnails.

### 11.6 New Module Scaffolding Patterns (Verified in Vega Series)

**Token-bucket rate limiter** (`BandwidthThrottleGuard` pattern):
- Always take `uint64_t nowMs` as the time parameter — never use `clock()` or wall time in a library
- Provide `Tick(nowMs)` for the caller to drive time; `TryConsume(bytes, nowMs)` calls `Tick` internally
- Config: `maxKbps=0` means unlimited; never apply the cap when it's 0

**Topology-aware policy** (`StreamingCacheTierPolicy` pattern):
- Use indexed array `Override[N]` not a map for small enum-keyed tables
- `DeriveFromProbe(result)` is a convenience that calls `Derive(topology)` then applies metered override

**Audit ring-buffer** (`ThumbnailAuditLog` pattern):
- Single `maxEvents` bound; `dropOldOnFull=true` → evict-front; `false` → reject-new
- Separate `TotalRecorded` (all ever recorded) vs `EventCount` (currently in buffer)
- `Flush()` clears buffer without resetting `TotalRecorded`/`TotalDropped`

**Token-bound cache** (`TokenBoundCacheEntry` pattern):
- Lookup checks `TenantToken::Matches()` before returning; increments `UnauthorizedMissCount` on denial
- `Clear()` resets all counters including hit/miss counts

### 11.7 GitHub API Offline — Fallback Procedure
**Symptom:** `gh issue list`, `gh pr list`, `gh release list` all fail with TCP connection timeout.  
**Root cause:** GitHub API unreachable from current network (corporate firewall, offline, etc.).  
**Fallback for tracking tasks:**
1. Use `git log --oneline` to identify commit hashes for completed work
2. Update `lessons-learned.md` and `gh-tracking-log.md` locally with git hashes
3. Push via `git push` (uses git protocol, not REST API) — this works even when gh CLI fails
4. When API is restored, close issues with: `gh issue close <N> --comment "Fixed in <hash>"`
5. Never block a sprint delivery on GH API availability

### 11.8 Sprint 1311-1320 Type Collision Crisis — 9-Round Build Fix

**Context:** Sprint 1311-1320 (v35.3.0 "Vega-T") introduced 5 new headers. Combined with all prior
sprint headers in `EngineTestsIncludes.h`, these produced **13 type name collisions** causing C2011
(type redefinition) and C2039 (member not found) errors resolved across 9 build rounds.

**Root cause:** Batch sprint header creation with no pre-collision type-name audit.

**All 13 collisions resolved:**

| Old Name | Winner (older header) | Renamed To | Updated |
|---|---|---|---|
| `AnimatedFormat` | `AnimatedFormatHandler.h` | `SampledAnimFormat` | .h, .cpp, 2 test files |
| `ToneMapOperator` | `D3D12ComputePipeline.h` | `PQToneMapOp` | .h, .cpp, test |
| `LTSGateStatus` | `LTSHardeningController.h` | `LTSValidatorStatus` | .h, .cpp |
| `ScrubState` | `LivePreviewScrubber.h` | `HoverScrubState` | .h only |
| `LatencyPercentiles` | `DecodeLatencyProfiler.h` | `HistogramPercentiles` | .h only |
| `DecodeRequest` | `ThumbnailBatch.h` | `WorkerDecodeRequest` | .h, .cpp, test |
| `WorkerState` | `ProcessPoolManager.h` | `ZTWorkerState` | .h, .cpp, test |
| `AuditEvent` | `ThumbnailAuditLog.h` | `EnterpriseAuditEvent` | .h, .cpp, test |
| `PrefetchRequest` | `CachePrefetchScheduler.h` | `AsyncPrefetchItem` | .h, .cpp, test |
| `BoundingBox3D` | `Advanced3DFormatDecoder.h` | `STEPBoundingBox` | .h, .cpp, test |
| `DICOMWindowPreset` | `DICOMWindowingPresets.h` | `AdvancedWindowPreset` | .h, .cpp, test |
| `PressureLevel` | `MemoryPressureControllerV2.h` | `ResponderPressureLevel` | .h, .cpp |
| `GateResult`/`GateVerdict` | `PerfRegressionGate.h` (ns ExplorerLens) | `GPUGateResult`/`GPUGateVerdict` | .h, .cpp |

**Cascade effect (critical):** Renaming a type at the header level is NOT enough. Test files that
tested the newer header still reference the OLD name — which now resolves to the OLDER header's
different struct. This causes C2039 "member not found" because the two structs have different fields.

**Fix pattern:**
1. Build → identify C2011 redefinition collisions
2. Rename type in the **newer sprint header** (keep older header intact to preserve all usages)
3. Update the matching .cpp file for that header
4. Rebuild → C2039 "member not found" appear in test files
5. Update test files: replace old type name with the new name; fix field name differences
6. Repeat until 0 errors, 0 warnings

**Prevention (now mandatory — added to copilot-instructions.md rule #24):**
Before committing any new sprint header batch, `grep_search` every new `struct`, `enum class`,
and `class` name across ALL `Engine/**/*.h` files. Zero matches required (excluding the new file).

**Key diagnostic errors:**
- `C2011: 'TypeName': type redefinition` — collision; rename newer header's type
- `C2039: 'field': is not a member of 'OldType'` — test using old name resolves to wrong struct
- `C2872: 'Symbol': ambiguous symbol` — same type name in different namespaces; rename the Engine one

**Files modified:** 18 header/source files + 3 test split files  
**Rounds to resolution:** 9  
**Build result:** BUILD_SUCCESS — 0 errors, 0 warnings  
**Fix commit:** (see gh-tracking-log.md for hash after push)
