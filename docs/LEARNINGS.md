# ExplorerLens — Engineering Learnings Log

**Last Updated:** 2026-03-28
**Covers:** v23.0.0 "Vega" through v24.0.0 "Altair" (last 10 iterations, Sprints 401–490)

This document captures accumulated engineering insights, patterns that worked, pitfalls encountered,
and decisions made across the last 10 release cycles. Every future sprint team should read this
before starting work.

---

## Table of Contents

1. [Test Architecture Learnings](#1-test-architecture-learnings)
2. [C++ Header-Only Implementation Patterns](#2-c-header-only-implementation-patterns)
3. [Build System & Toolchain Learnings](#3-build-system--toolchain-learnings)
4. [Version Bump Discipline](#4-version-bump-discipline)
5. [Sprint Execution Patterns](#5-sprint-execution-patterns)
6. [Documentation & Docs-as-Code](#6-documentation--docs-as-code)
7. [Iteration-by-Iteration Learnings](#7-iteration-by-iteration-learnings)
8. [CI/CD & Release Pipeline](#8-cicd--release-pipeline)
9. [External Library Management](#9-external-library-management)
10. [AI/ML Header Design Patterns](#10-aiml-header-design-patterns)

---

## 1. Test Architecture Learnings

### 1.1 — 3-Test Minimum Is Insufficient for Coverage

**Iteration learned:** v24.0.0 (Sprint 481–490)

Initially, 3 tests per header (defaults, constants, null-input) were added to EngineTests.cpp.
This was expanded to 9 tests per header when the "resolve all stubs" pass revealed that enum
variants, setters, success paths, and error paths were untested.

**Rule established:** Every new header MUST have tests covering:
1. Default construction / all default config values
2. All `enum class` variants (one test asserting each `SetXxx()` + `GetXxx()` round-trip)
3. All `static constexpr` constants
4. All setters (verify stored value)
5. Null/empty input error path → `result.success == false` + non-empty `error`
6. Valid success path → `result.success == true` + correct output dimensions
7. File path error path → empty string → failure
8. Any `static` utility methods (e.g., `HammingDistance`, `EstimateNoiseLevel`)
9. Cross-method state (e.g., `LoadModel("")` fails, then `LoadModel("x")` succeeds)

### 1.2 — Test Count Tracking Must Be Exact

**Iteration learned:** v24.0.0

Test count jumped from 3125 → 3197 (+72). MUST update `BuildValidation.h::UnitTestCount` to match
the exact new total. Discrepancies cause audit mismatches in CI.

**Verification command:**
```powershell
Select-String -Path Engine\Tests\EngineTests.cpp -Pattern "RUN_TEST\(" | Measure-Object | Select-Object -ExpandProperty Count
```

### 1.3 — Custom Test Framework (Not GTest)

Tests use macros `TEST(name)`, `RUN_TEST(name)`, `ASSERT(cond)` that update `g_testsRun`,
`g_testsPassed`, `g_testsFailed`. **Never introduce GTest/Catch2 dependencies.**
All new tests must follow the existing macro pattern.

---

## 2. C++ Header-Only Implementation Patterns

### 2.1 — Forward Reference Bug: Struct Defaults Cannot Reference Class Constants

**Iteration learned:** v24.0.0 (SceneDepthEstimatorV2, StyleTransferEngine, LandmarkDetectionEngine)

**Problem:** A `struct FooConfig` defined BEFORE the `class Foo` used `Foo::SOME_CONSTANT` as a
default value. This compiles as a forward reference error in MSVC v145.

```cpp
// ❌ WRONG — struct is defined before the class
struct DepthConfig {
    int inputSize = SceneDepthEstimatorV2::DEFAULT_INPUT_SIZE; // ERROR: class not yet defined
};
class SceneDepthEstimatorV2 {
    static constexpr int DEFAULT_INPUT_SIZE = 384;
};

// ✅ CORRECT — use literal in struct default, document the constant in the class
struct DepthConfig {
    int inputSize = 384; // matches SceneDepthEstimatorV2::DEFAULT_INPUT_SIZE
};
class SceneDepthEstimatorV2 {
    static constexpr int DEFAULT_INPUT_SIZE = 384;
};
```

**Fix applied:** `SceneDepthEstimatorV2.h`, `StyleTransferEngine.h`, `LandmarkDetectionEngine.h`
all had struct defaults replaced with literal values matching the class constants.

### 2.2 — All Methods Must Be `noexcept`

Every public method in header-only engine types must be declared `noexcept`. This:
- Prevents COM shell extension crashes from propagating exceptions across the DLL boundary
- Allows MSVC to generate better code (no exception table entries)
- Is required by the zero-warnings build policy

### 2.3 — Namespace Pattern

All engine classes use double-namespace:
```cpp
namespace ExplorerLens { namespace AI { class Foo { ... }; }}
```
Tests use `using namespace ExplorerLens::AI;` within `TEST()` scope only.

### 2.4 — Result Struct Pattern

Every operation returns a result struct, never throws:
```cpp
struct FooResult {
    bool                 success = false;
    std::vector<uint8_t> pixels;   // output data
    int                  width  = 0;
    int                  height = 0;
    std::string          error;    // non-empty on failure
};
```
Input validation in the method body returns early with `success=false` and a descriptive error.

### 2.5 — Config Struct + Explicit Constructor Pattern

```cpp
struct FooConfig { /* all configurable fields with defaults */ };

class Foo {
public:
    explicit Foo() = default;
    explicit Foo(const FooConfig& cfg) : m_config(cfg) {}
    // getters, setters, operations
private:
    FooConfig m_config;
};
```
This pattern allows both zero-config construction and full custom configuration.

---

## 3. Build System & Toolchain Learnings

### 3.1 — Always Source vcvars64.bat Before CMake

Without running `vcvars64.bat -vcvars_ver=14.50.35717` first, CMake picks up Clang from PATH
instead of MSVC. This causes:
- Incompatible compiler flags
- Missing Windows-specific intrinsics
- Broken CRT linkage (Clang vs MSVC `/MD`)

**Rule:** Always use `build-scripts\Build-MSVC.ps1` or source vcvars manually before cmake commands.

### 3.2 — MSVC v145 Toolset Is Local-Only (Never Pin in CI)

Local dev uses VS 18 2026 BuildTools (MSVC v145, cl.exe 19.50). GitHub Actions runners only have
VS 2022 (MSVC v143). CI workflows MUST NOT pin `toolset: "14.50"` — this breaks on GitHub runners.
Use `ilammy/msvc-dev-cmd@v1` with no toolset pin.

### 3.3 — CRT Linkage: Always `/MD` (MultiThreadedDLL)

All targets and external libraries use `/MD`. There are NO `/NODEFAULTLIB:LIBCMT` workarounds.
Any new external library MUST be built with:
```
nmake RTLIBCFG=dll OBJDIR=obj
```
or equivalent CMake flag `-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL`.

### 3.4 — New Files Must Be Registered in CMakeLists.txt

Headers → add to `ENGINE_HEADERS` list.
Sources → add to `ENGINE_SOURCES` list.
New test files → must be explicit in `Engine/Tests/CMakeLists.txt`.

**Failure mode:** Header added but not registered → IntelliSense works but build excludes it from
analysis coverage → unit tests may silently link against stale object.

### 3.5 — EngineTests.cpp Compilation Takes ~90s

At 22K+ lines, EngineTests.cpp is a single translation unit. Do NOT assume the build is hung
within the first 90 seconds of compilation. Wait for the full build-and-log output.

---

## 4. Version Bump Discipline

### 4.1 — The 13-File Version Contract

Every version bump MUST update ALL of these files simultaneously:

| # | File | Field(s) |
|---|------|----------|
| 1 | `VERSION` | Bare version string |
| 2 | `Engine/Core/BuildValidation.h` | `MajorVersion`, `MinorVersion`, `VersionString`, `Codename`, `TotalMilestones`, `CompletedMilestones`, `UnitTestCount` |
| 3 | `Engine/Core/SBOMGenerator.h` | Two `"ExplorerLens-X.Y.Z"` strings (name + creator) |
| 4 | `vcpkg.json` | `"version"` field |
| 5 | `docs/SBOM.json` | `serialNumber` (urn:uuid) + `"version"` (component) |
| 6 | `Engine/Tests/benchmarks/baseline.json` | `_comment` + `version` |
| 7 | `.github/standards/tool-versions.md` | Header version line |
| 8 | `README.md` | Tests badge count + Current Version + Codename + Last Updated |
| 9 | `Engine/README.md` | Version line + status test count |
| 10 | `CHANGELOG.md` | Promote `[Unreleased]` → `[X.Y.Z]` section |
| 11 | `.github/copilot-instructions.md` | Version line + test count + sprint plan reference |
| 12 | `docs/assets/social-preview.svg` | Tests chip + version chip |
| 13 | `docs/assets/architecture-build.svg` | MSI artifact filename + comment |

Missing any of these files creates stale metadata that breaks the release pipeline audit.

### 4.2 — Multi-Replace Is More Reliable Than Sequential Edits

Use `multi_replace_string_in_file` for batched version updates. Greatly reduces the chance
of a missed file or off-by-one error on sequential replacements.

### 4.3 — SBOM.json Has Two Stale Version Fields

`docs/SBOM.json` contains:
- Line 6: `serialNumber` (urn:uuid with old version) — updated to current
- Line 20: `"version"` (component) — was still at v23.6.0 even after v23.7.0 bump

**Lesson:** Always search `docs/SBOM.json` for ALL version-string instances, not just the obvious one.

### 4.4 — SBOMGenerator.h Has Two Stale Strings

`Engine/Core/SBOMGenerator.h` contains:
- Line ~100: `"ExplorerLens-X.Y.Z"` in the SPDX document name
- Line ~104: `"ExplorerLens-SBOMGenerator-X.Y.Z"` in the creator tool string

Both must be updated. The second was historically stuck at 23.6.0 even after 23.7.0 bumps.

---

## 5. Sprint Execution Patterns

### 5.1 — The Standard 8-Header Sprint Pattern

Each sprint (10 sprints = 1 version) delivers:
1. 8 new headers in the appropriate `Engine/` subdirectory
2. Each header registered in `Engine/CMakeLists.txt` ENGINE_HEADERS
3. 9 unit tests per header (72 total) in EngineTests.cpp
4. All `RUN_TEST()` calls added before the `// Isolation & Stability Tests` comment
5. All `#include` lines added after the last prior sprint's include group
6. Version bump across all 13 files
7. Git commit + tag + push

### 5.2 — Stub-First Is Acceptable But Must Be Resolved Immediately

In v24.0.0, the initial 8 AI headers were created as stubs (minimal placeholder bodies).
This was acceptable as a first pass, but **all stubs must be replaced with real implementations
in the same sprint before the version commit**. A versioned release must never contain stubs.

Signs a header is a stub:
- Methods that call `UsesDML()` (method that doesn't exist on the class)
- `GetMaxFaces() == 8` when the default config says `maxFaces = 10`
- Tests asserting `engine.UsesDML()` which doesn't exist

### 5.3 — Test Assertions Must Match Actual Implementation Defaults

When the initial stub assumed `default maxFaces = 8` and the real implementation used `10`,
tests like `ASSERT(engine.GetMaxFaces() == 8)` would fail. Always verify test assertions against
the actual field defaults in the struct before committing.

### 5.4 — Sprint Plan Document Convention

Sprint plans go in `docs/SPRINT_PLAN_N00.md` where N covers the next 100 sprints.
The most recent plan must be registered in `docs/mkdocs.yml` nav under the "Sprint Plans" section.
When a plan's sprints pass their execution point, it is moved to `docs/archive/` via `git mv`.

---

## 6. Documentation & Docs-as-Code

### 6.1 — Archives vs. Active Sprint Plans

Sprint plans older than the current execution point are archived to `docs/archive/`.
This keeps the active nav clean and reduces confusion about which plan is current.

**Archive trigger:** Sprint plan covers sprints entirely in the past.

### 6.2 — copilot-instructions.md Must Reflect Reality

`copilot-instructions.md` functions as the AI agent's grounding document. It MUST stay in sync:
- Current version and codename
- Test count
- Sprint plan document list (add SPRINT_PLAN_N00.md references as created)

Stale values in this file cause agents in future sessions to use incorrect version numbers.

### 6.3 — SVG Graphics Use Chip Text for Version + Test Count

`docs/assets/social-preview.svg` uses embedded `<text>` elements for the "Tests" and "Version"
chips. Update these with exact grep-and-replace — do not assume coordinates are stable between
edits.

### 6.4 — ROADMAP_V25.md Needs Manual Status Update

After each release, update `docs/ROADMAP_V25.md`:
- Mark released versions with `✔ Released`
- Mark the current version with `← NOW`
- Use `git mv` not file copy for archive operations

---

## 7. Iteration-by-Iteration Learnings

### v23.0.0 "Vega" — Reactive Pipeline Architecture (Sprints 401–410)

**Theme:** CQRS, event sourcing, saga orchestration, reactive streams.

**Key learnings:**
- `ThumbnailEventStore.h` — Append-only event logs require careful versioning. NEVER change event
  schemas after they've been used in production (add new event types instead).
- Reactive stream engines must have back-pressure from the start. Adding it later breaks API.
- Pipeline orchestrators should use a state machine, not deep call stacks.

### v23.1.0 "Vega-R" — GPU Acceleration v3 (Sprints 411–420)

**Theme:** CUDA, HIP/ROCm, multi-GPU load balancing, async DMA.

**Key learnings:**
- CUDA/HIP stubs with graceful CPU fallback are the correct pattern. GPU hardware may not be
  present. Every GPU code path MUST fall through to CPU.
- GPU texture atlases need deterministic packing to avoid cache fragmentation across sessions.
- `MultiGPULoadBalancerV3.h` — utilization-based routing is better than round-robin. Poll real
  D3D12 queue depth rather than counter-based estimates.

### v23.2.0 "Vega-S" — Plugin Ecosystem v3 (Sprints 421–430)

**Theme:** DI containers, A/B testing, feature flags, canary releases.

**Key learnings:**
- Feature flag engines need a monotonic epoch counter so stale evaluations are detectable.
- Canary controllers should use per-error-type rollback, not just aggregate error rate.
- Plugin compliance auditors need to check SBOM presence, not just signing.

### v23.3.0 "Vega-T" — Memory Optimization v3 (Sprints 431–440)

**Theme:** Huge pages, memory-mapped B-tree, NVMe tier, ECC.

**Key learnings:**
- `MEM_LARGE_PAGES` on Windows requires `SeLockMemoryPrivilege`. Always fall back gracefully
  if privilege is absent — do NOT crash.
- NVMe SCM headers should always be stubs with forward-compatibility comments since the hardware
  is not universally available.
- ECC monitors should use read-only perf counters, never try to "correct" errors in user space.

### v23.4.0 "Vega-U" — Smart Cache v4 (Sprints 441–450)

**Theme:** AI eviction, federated invalidation, content-hash keys, encryption.

**Key learnings:**
- ML-driven eviction needs a shadow mode first (log what it would evict, don't actually evict)
  before it influences production evictions.
- Content-hash cache keys must account for ICC profile and target size, not just raw file hash.
- AES-GCM cache encryption: key rotation must not invalidate existing cached entries —
  use key derivation per session, not per-entry.

### v23.5.0 "Vega-V" — CLI & Automation v2 (Sprints 451–460)

**Theme:** Batch processor, file watcher, perceptual diff, CI/CD webhook.

**Key learnings:**
- File watchers on Windows need `FILE_FLAG_BACKUP_SEMANTICS` for directory handles.
- SSIM/PSNR thresholds should be configurable, not hardcoded — regression thresholds vary
  by use case (photo vs. document vs. CAD).
- CI/CD webhooks must be authenticated (HMAC-SHA256 payload verification). Never expose
  a plain HTTP webhook endpoint.

### v23.6.0 "Vega-W" — Security Hardening v2 (Sprints 461–470)

**Theme:** Zero-trust COM, sandbox isolation, runtime integrity, exploit mitigation.

**Key learnings:**
- Job Object sandbox must allow `NtCreateSection`/`NtMapViewOfSection` for MuPDF font loading —
  overly restrictive job objects break PDF thumbnails.
- CFG enforcement requires all function pointer tables to be const at compile time.
- Anti-tamper detection false-positives are very common under debuggers and profilers.
  Always provide a debug-mode bypass skip.
- Zero-trust policy engines need a policy-miss default action (deny-all vs. allow-known).
  Deny-all causes crashes for unregistered COM callers in test environments.

### v23.7.0 "Vega-X" — Format Expansion V (Sprints 471–480)

**Theme:** ICNS, CUR/ANI, IFF ANIM, MNG, HRZ, PIXAR ptex, JPEG 2000 tiled v2, FLIF v2.

**Key learnings:**
- ICNS bundles can contain near-duplicate resolutions. Always pick the largest PNG variant for
  the thumbnail to avoid upscaling artifacts.
- IFF ANIM (ANIM5/7/8) format requires ILBM chunk parsing. Reuse the existing archive
  framework for chunk routing rather than writing a parallel parser.
- FLIF progressive exit: the decoder supports stopping after a configurable quality level.
  Always expose this as a config option to cap decode time for large files.
- JPEG 2000 tiled decoding v2 must skip sub-resolution levels below the target thumbnail size
  to avoid decoding 8K JPX files fully just to produce a 256×256 thumbnail.

### v24.0.0 "Altair" — AI-Native Thumbnailing v2 (Sprints 481–490) ← current

**Theme:** ESRGAN upscaler, content-aware resize, semantic hash, auto-tagging,
quality restoration, monocular depth, style transfer, face/landmark detection.

**Key learnings:**

1. **Stub headers must be resolved before the version commit.** Initial stubs referenced
   non-existent methods (`UsesDML()`, hardcoded default values inconsistent with config structs).

2. **Forward-reference struct defaults:** Struct field defaults using `ClassName::CONSTANT`
   fail when the struct is declared before the class. Use literal values matching the constant
   instead.

3. **Strength clamping in SetStrength():** User-settable float parameters should clamp to
   valid range in the setter. `StyleTransferEngine::SetStrength(v)` → clamps to `[0.0, 1.0]`.
   Callers should not be expected to validate this.

4. **9-test coverage target confirmed effective:** After expanding from 3 → 9 tests per header,
   two bugs were caught pre-commit:
   - `QualityRestorationEngine` default mode was `Combined` in original stub but `Denoise`
     in the real implementation.
   - `LandmarkDetectionEngine` default `maxFaces` was `8` in stub, `10` in implementation.

5. **HammingDistance for 512-bit hashes:** Computing bit differences across 8×`uint64_t` with a
   manual shift loop is correct but slower than `__builtin_popcountll` or `std::popcount()`
   (C++20). For C++20/MSVC, use `std::popcount()` in future to enable hardware POPCNT instruction.

6. **LoadTaxonomy() returns bool:** Callers must check the return value. Auto-tagging Tag()
   now checks `m_taxonomyLoaded` and returns an error result if taxonomy was never loaded —
   this prevents silent empty-tag results that could look like a success.

---

## 8. CI/CD & Release Pipeline

### 8.1 — release.yml Triggers on Tag Push Only

`release.yml` is triggered by `git push origin main --tags`. It does NOT run on plain branch pushes.
If a release was tagged but the workflow didn't fire, verify the tag was pushed with `--tags`.

### 8.2 — Bypassed Branch Protection Rules Are Expected

On push, git may report:
```
remote: Bypassed rule violations for refs/heads/main:
remote: - Changes must be made through a pull request.
remote: - 5 of 5 required status checks are expected.
```
This is expected for the `main` branch with admin bypass enabled. It is NOT an error.

### 8.3 — SBOM and SHA256SUMS Are Release Artifacts

Every release MUST attach:
- `ExplorerLens-X.Y.Z-SBOM.json` (CycloneDX format)
- `SHA256SUMS.txt`
- `ExplorerLens-X.Y.Z-x64.msi`
- `ExplorerLens-X.Y.Z-x64.zip`
- `LENSShell.dll`, `LENSManager.exe`, `lens.exe`

### 8.4 — Corporate Proxy Scrub Before Public Push

Before any public push, always run:
```powershell
git grep -rn "intel.com" -- "*.ps1" "*.yml" "*.yaml" "*.md" "*.json" "*.h" "*.cpp"
git grep -rn "proxy" -- "*.ps1" "*.yml" "*.yaml" "*.md" "*.json"
git grep -rn "928\b" -- "*.ps1" "*.yml" "*.yaml"
```
Port 928 = Intel internal proxy. Remove any matches before pushing.

---

## 9. External Library Management

### 9.1 — All Libraries Must Use `/MD` CRT

Any new external library MUST be built (or rebuilt) with `/MD` (MultiThreadedDLL). Adding
a library built with `/MT` causes duplicate CRT symbol errors at link time. Non-negotiable.

### 9.2 — Library Auto-Detection From `.lib` Files

`Engine/CMakeLists.txt` auto-detects library features by checking for `.lib` files in `external/`:
- `libwebp.lib` → `HAS_LIBWEBP`
- `libjxl.lib` → `HAS_LIBJXL`
- etc.

When updating a library version, the `.lib` filename must not change, or CMakeLists.txt
detection logic must be updated.

### 9.3 — External Library Directory Structure

```
external/
  compression-libs/  — zlib, lz4, zstd, minizip-ng, lzma, unrar, bzip2, libarchive, xz
  image-libs/        — libwebp, libjxl, libavif, libheif, libde265, dav1d
  camera-libs/       — libraw
  pdf-libs/          — mupdf
  ui-libs/           — wtl
```

Never put new libraries at the project root level. Place them in the appropriate subdirectory.

---

## 10. AI/ML Header Design Patterns

### 10.1 — Backend Enum Pattern

All AI headers with hardware acceleration support use a `Backend` enum:
```cpp
enum class FooBackend : uint8_t {
    DirectML = 0,  // First = default
    CPU      = 1,  // Always available fallback
    ONNX     = 2,  // Optional ONNX runtime
};
```
`DirectML = 0` is always the default (first enum value), `CPU = 1` is the universal fallback.

### 10.2 — All AI Operations Must Have File Path Overloads

Every `Process(pixels, w, h)` method must have a corresponding `ProcessFile(path)`:
```cpp
FooResult Process(const void* pixels, int w, int h) const noexcept;
FooResult ProcessFile(const std::string& path) const noexcept;
```
`ProcessFile()` must return `{ false, ..., "File not found: " + path }` for non-existent paths.
Never throw.

### 10.3 — Static Utility Methods Are Testable Without Construction

Methods like `HammingDistance()`, `EstimateNoiseLevel()`, `EstimateBlurLevel()` are `static`
so they can be tested without constructing the full engine object. Prefer static for pure
mathematical / utility computations.

### 10.4 — Model Loading Is Deferred, Not in Constructor

Model files are never loaded in the constructor. Constructors are `noexcept` and must not do I/O.
Callers call `LoadModel(path)` explicitly, and `IsModelLoaded()` allows checking model state.
This ensures shell extension COM creation is non-blocking.

### 10.5 — Confidence / Score Range Is [0.0, 1.0]

All confidence, strength, threshold, and score values are `float` in `[0.0, 1.0]`.
Setters that accept these values should clamp to range (learned from `StyleTransferEngine`).

---

## Quick Reference — Verification Checklist Before Any Sprint Commit

```powershell
# 1. Count actual RUN_TEST() calls (must equal UnitTestCount in BuildValidation.h)
Select-String Engine\Tests\EngineTests.cpp -Pattern "RUN_TEST\(" | Measure-Object | Select-Object -ExpandProperty Count

# 2. Verify all 13 version files updated
Get-Content VERSION
Select-String Engine\Core\BuildValidation.h -Pattern "VersionString|UnitTestCount"
Select-String vcpkg.json -Pattern '"version"'

# 3. Check for forward-reference bugs (struct defaults using ClassName::CONSTANT)
# This is a manual review step — scan each new header's config struct

# 4. Verify noexcept on all public methods
# Scan with: Select-String Engine\AI\*.h -Pattern "^\s+\w.*\(.*\)\s*(?!const\s*noexcept|noexcept)"

# 5. Scrub corporate artifacts
git grep -rn "intel.com" -- "*.h" "*.cpp" "*.ps1" "*.yml"

# 6. Build and test
.\build-scripts\Build-MSVC.ps1 -Test
```
