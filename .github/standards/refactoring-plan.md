# ExplorerLens Refactoring Plan

**Created:** February 2026
**Version:** 15.0.0 → 16.0.0
**Scope:** Comprehensive project-wide modernization

---

## Executive Summary

This refactoring plan covers all project dimensions: tooling, engine architecture,
utilities consolidation, AI modules, UI/UX modernization, plugin system,
memory/performance optimization, VS Code integration, and documentation standards.

Each section includes priority (P0 = critical, P1 = high, P2 = medium, P3 = low),
estimated effort, and concrete action items.

---

## 1. VS Code Integration (P0) ✅ COMPLETED

### What Was Done

| Item | Status | Description |
|------|--------|-------------|
| **extensions.json** | ✅ Done | 16 recommended extensions (C++, CMake, Git, quality, productivity) |
| **settings.json** | ✅ Done | Modernized with sections, Developer PowerShell profile, CMake parallel jobs, C++ formatting, spell checker dictionary, markdownlint config, bracket colorization, sticky scroll, rulers |
| **launch.json** | ✅ Done | Added C++ debug configs (Engine Tests, LENSManager, Attach to Explorer.exe), organized PS scripts, added preLaunchTask |
| **tasks.json** | ✅ Done | Reorganized with section headers, default build task = Build Engine, default test = Run Engine Tests, Run CTest, dependsOn chains, showReuseMessage:false |
| **c_cpp_properties.json** | ✅ OK | Already well-configured (MSVC v145, C++20, compile_commands.json) |
| **.editorconfig** | ✅ Done | Consistent formatting rules across all file types |

---

## 2. Documentation Standards (P0) ✅ COMPLETED

### What Was Done

| Item | Status | Description |
|------|--------|-------------|
| **tool-versions.md** | ✅ Done | Complete tool/library version matrix with upgrade procedures |
| **Markdown lint fixes** | ✅ Done | Fixed README.md, CHANGELOG.md, docs/development/README.md, Engine/README.md |
| **coding-standards.md** | ✅ OK | Already comprehensive (684 lines) |
| **build-method.md** | ✅ OK | Already comprehensive (559 lines) |
| **build-troubleshooting.md** | ✅ OK | Already comprehensive |
| **copilot-instructions.md** | ✅ OK | Already comprehensive |

### Remaining Documentation Actions (P2)

- [x] Create `.github/standards/architecture-decisions.md` — ADR log for major design decisions ✅
- [x] Create `.github/standards/performance-benchmarks.md` — baseline performance targets ✅
- [x] Update `DEVELOPMENT_LEARNINGS.md` with latest patterns discovered ✅ (Section 9 added — architecture patterns, build timing, singleton tests)
- [x] Add Mermaid architecture diagrams to `docs/architecture/` ✅ (3 diagrams: system-overview, decode-pipeline, plugin-architecture)

---

## 3. Engine Architecture Refactoring (P1)

### Current State

The `Engine/Core/` directory contains **170+ header files** spanning decode pipeline,
GPU rendering, UI bridge, telemetry, cloud integration, and more. Many are single-use
headers with minimal code. The architecture would benefit from:

### Proposed Actions

#### 3.1 Interface Consolidation (P1) ✅ COMPLETED

```
Engine/Core/Interfaces/
  IThumbnailDecoder.h    ← canonical location (forwarding wrapper in Core/)
  IFormatDetector.h      ← canonical location (forwarding wrapper in Core/)
  IGPURenderer.h         ← canonical location (forwarding wrapper in Core/)
  ICacheProvider.h       ← canonical location (forwarding wrapper in Core/)
```

- **Action:** Created `Engine/Core/Interfaces/` subdirectory, moved all `I*.h` interfaces there.
  Original files in `Core/` converted to thin `#include "Interfaces/X.h"` forwarding headers.
  All 8 headers (4 canonical + 4 forwarding) registered in CMakeLists.txt.
- **Impact:** Clear separation of contracts vs. implementations

#### 3.2 Telemetry Consolidation (P1) ✅ COMPLETED

Consolidated 5 files → `Engine/Core/Telemetry.h` (~700 lines). Old headers
(`TelemetryEngine.h`, `TelemetryPipeline.h`, `TelemetryPipelineV2.h`,
`TelemetryDashboard.h`, `TelemetryHooks.h`) converted to thin `#include "Telemetry.h"` wrappers.
Renamed `enum class TelemetryEvent` → `TelemetryEventType` to resolve name collision.
`TelemetryEngine.cpp` updated to include `Telemetry.h`.

#### 3.3 Version/Drift Management (P2) ✅ COMPLETED

Created `Engine/Core/VersionManagement.h` umbrella header including all 6 version files.
Fixed stale hardcoded version in `VersionDriftDetector.h` (v7.1.0 → v15.0.0).
Registered 4 previously-unregistered headers in CMakeLists.txt
(`LibraryInventoryManager.h`, `LibraryVersionAudit.h`, `VersionManagement.h`, `VersionSynchronizer.h`).
Note: `VersionNormalization.h` does not exist.

#### 3.4 DarkMode Consolidation (P2) ✅ COMPLETED

Created `Engine/Core/DarkMode.h` umbrella header with `#ifdef _WIN32` guards for
Win32-dependent headers (`DarkModeRendererV2.h`, `DarkModeControls.h`).
`DarkModeEngine.h` always included (cross-platform). All 4 headers registered in CMakeLists.txt.
Note: `DarkModeManagerV2.h` does not exist. LENSManager helpers remain separate (different project).

#### 3.5 Dead Code Audit (P2) — ✅ Done

Created `Engine/Core/DeadCodeAnalysis.h` umbrella including both `DeadCodeAudit.h` and
`DeadCodeAuditor.h`. All headers registered in CMakeLists.txt.

- [x] Consolidate headers into umbrella ✅
- [x] Run dead code analysis to identify unused code paths ✅ (377 headers, 63 zero-ref = 16.7%, 31 now registered in CMake)
- **Actual dead code:** 16.7% of headers have zero consumers; 39% are test-only. Report: `docs/DEAD_CODE_AUDIT.md`

---

## 4. Utils Consolidation (P1)

### Current State: 134 files in Engine/Utils/

The Utils directory is the largest single directory in the project. Many files follow
the pattern of adding a new header per feature without merging related functionality.

### Consolidation Targets

| Category | Current Files | Target | Effort | Status |
|----------|--------------|--------|--------|--------|
| ARM64 | 10 files (ARM64*.h) | `ARM64Platform.h` | Medium | ✅ Done (prior session) |
| Accessibility | 4 files | `Accessibility.h` | Low | ✅ Done |
| Enterprise/Compliance | 6 files | `Enterprise.h` | Medium | ✅ Done |
| Fuzzing/Security | 5 files | `SecurityTesting.h` | Low | ✅ Done |
| CI/Pipeline | 4 files | `CIPipeline.h` | Low | ✅ Done (chain exists) |
| Test Infrastructure | 4 files | `TestInfrastructure.h` | Low | ✅ Done (3 files — CodeCoverage.h excluded, type conflicts) |
| Installer | 4 files | `Installer.h` | Low | ✅ Done |
| Documentation Gen | 4 files | `DocGenerator.h` | Low | ✅ Done (added VersionNormalization.h + DiagnosticDashboard.h) |
| Memory Safety | 4 files | `MemorySafety.h` | Low | ✅ Done (chain exists) |
| Observability | 3 files | `Observability.h` (exists) | Low | ✅ Registered |
| Windows Compat | 5 files | `WindowsCompat.h` (exists) | Low | ✅ Done (added MSIXPackageManager.h + PortableModeManager.h — WindowsUI.h excluded, DPIScale conflict) |
| **ReleaseGate** | 26 files → 1 | ✅ **Already done** | — | ✅ Done |

### Priority Order

1. **ARM64** consolidation (10 files → 1) — highest file count
2. **Enterprise** consolidation (6 files → 1) — related concepts
3. **Test Infrastructure** (4 files → 1) — cleanup
4. Remaining categories in order of file count

---

## 5. AI Module Improvements (P2)

### Current State: 6 files in Engine/AI/

| File | Purpose | Status |
|------|---------|--------|
| `SceneUnderstandingEngine.h` | ML-based scene classification | ✅ ClassifyByHeuristics() — color histogram + edge density heuristics |
| `SmartCropV2.h` | Saliency-based cropping | ✅ ComputeCenterOfInterest() + ComputeCropRegion() — Sobel gradient-weighted |
| `ImageQualityAssessor.h` | BRISQUE/NIQE quality scoring | ✅ Laplacian variance blur, exposure defects, sharpness grading, Assess() |
| `AISearchIntegration.h` | CLIP embedding search | ✅ Perceptual hashing (aHash, dHash), Hamming distance, AreSimilar() |
| `AIThumbnailEnhancer.h/.cpp` | Enhancement pipeline | Has implementation |

### Proposed Improvements

1. ~~**P2:** Implement DirectML backend for `SceneUnderstandingEngine` — actual inference~~ CPU heuristics done
2. **P2:** Add ONNX Runtime integration for model loading
3. ~~**P3:** Implement perceptual hashing in `AISearchIntegration` using xxHash + DCT~~ ✅ Done (aHash + dHash)
4. ~~**P3:** Wire `SmartCropV2` to content-aware thumbnail generation~~ ✅ Done (Sobel-based interest detection)

---

## 6. UI/UX Modernization (P2)

### Current State: WTL-based LENSManager

The LENSManager uses **Windows Template Library (WTL)** with custom dark mode rendering.
Current UI supports: format group toggle, performance dashboard, system tray, diagnostics.

### Modernization Options

| Approach | Effort | Benefit | Risk |
|----------|--------|---------|------|
| **A: Enhance WTL** | Low | Maintain stability, add polish | Limited modernity |
| **B: WinUI 3 migration** | Very High | Modern look, Fluent Design | Major rewrite |
| **C: Hybrid (WTL + WebView2)** | Medium | Modern panels in existing frame | Complexity |

### Recommended: Option A (Enhanced WTL) for v16, Option B for v17+

#### v16 Quick Wins (P2)

- [x] Add Mica/Acrylic backdrop via `DwmSetWindowAttribute` ✅
- [x] Implement `DarkModeController` with system theme detection ✅
- [ ] Add smooth scrolling to format list
- [ ] Improve PerformanceDashboard with real-time charts (GDI+ drawing)
- [x] Add Windows 11 rounded corners via `DwmSetWindowAttribute(DWMWA_WINDOW_CORNER_PREFERENCE)` ✅
- [ ] Implement segmented format category tabs
- [ ] Add search/filter to format list

#### v17+ WinUI 3 Migration (P3)

- Full XAML/C++ rewrite using Windows App SDK
- Fluent Design System 2 (Mica, NavigationView, InfoBar)
- See `docs/WINUI3_MIGRATION_PLAN.md` for detailed plan

---

## 7. Plugin System (P2)

### Current State: 30 files in Engine/Plugin/

The plugin system has evolved through multiple iterations (PluginSDKV2, PluginMarketplaceV3,
PluginCompatibilityKitV2). Key areas:

### Proposed Actions

1. ~~**P2:** Consolidate `PluginMarketplace.h` + `PluginMarketplaceV2.h/.cpp` + `PluginMarketplaceV3.h` → single impl~~ ✅ Done → `PluginMarketplaceUnified.h` (V2+V3; V1 excluded due to PackageType/CertificateInfo collisions)
2. ~~**P2:** Merge `PluginSandboxPolicy.h` + `PluginRuntimeValidation.h` → `PluginSecurity.h`~~ ✅ Done
3. ~~**P2:** Merge `PluginActivation.h` + `PluginHotReload.h` → `PluginLifecycle.h`~~ ✅ Done
4. ~~**P3:** Implement actual plugin loading via LoadLibrary + C ABI bridge~~ ✅ Done → `PluginLoaderV2.h` (LoadLibraryExW, C ABI symbol resolution, ABI version negotiation, extension routing, hot-reload)

---

## 8. Memory & Performance Optimization (P2)

### Current State

- **Memory**: 11 files in `Engine/Memory/` covering pool allocation, compaction, hot mode
- **Pipeline**: 20 files covering async decode, parallel batch, zero-copy, prefetch
- **Cache**: 14 files covering adaptive budget, multi-tier, sub-ms engine, PSO cache, USN invalidation

### Key Performance Targets

| Metric | Current | Target | Action |
|--------|---------|--------|--------|
| Single thumbnail | 17ms | 12ms | GPU pipeline optimization |
| Batch throughput | 235 img/s | 300 img/s | Better parallelism |
| Cache hit latency | <5ms | <1ms | Sub-ms cache tuning |
| Memory footprint | ~45MB | ~30MB | Pool allocator tuning |
| Cold start | ~200ms | ~100ms | Lazy initialization |

### Proposed Actions

1. **P1:** Profile actual hot paths with ETW + performance counters
2. **P2:** Implement `BufferPoolAllocator` with size-class bucketing
3. **P2:** Enable `PredictivePrefetchEngine` for directory browsing patterns
4. **P3:** Optimize `SubMillisecondCacheEngine` hash collision rate

---

## 9. Build System Improvements (P2)

### Current State

- CMake 4.3.0 + Ninja for Engine
- MSBuild for Shell/Manager
- 30+ build scripts in `build-scripts/`

### Proposed Actions

1. **P2:** Migrate LENSShell from MSBuild-only to CMake (unified build) ✅ Done
2. **P2:** Migrate LENSManager from MSBuild-only to CMake ✅ Done
3. **P2:** Add `cmake --install` target for SDK headers ✅ Done
4. **P3:** Implement incremental library builds (skip if .lib is newer than source)
5. ~~**P3:** Add build time tracking to `Build-MSVC.ps1`~~ ✅ Done (Stopwatch timing + JSONL history at build-logs/build-history.jsonl)

---

## 10. Shell Integration (P2)

### Current State

- `ShellContextMenuV2` — right-click context menu
- `ShellOverlayHandler` — icon overlays
- `ShellPreviewHandler` — preview pane
- `PropertyStoreHandler` — file property sheets

### Proposed Actions

1. **P2:** Implement Windows 11 context menu integration (`IExplorerCommand`) ✅ Done (v14 — LENSShellContextMenu)
2. ~~**P2:** Add thumbnail progress indicator for large files~~ ✅ Done → `ShellProgressIndicator.h` (ThumbnailStage pipeline, ETA, batch tracking, 6 tests)
3. ~~**P3:** Implement shell search protocol handler (`ISearchProtocolHandler`)~~ ✅ Done → `ShellSearchProtocolHandler.h` (index, extension filter, relevance scoring, 4 tests)
4. ~~**P3:** Add Jump List integration for recently browsed folders~~ ✅ Done → `JumpListIntegration.h` (recent/frequent/pinned categories, COM stub, 5 tests)

---

## 11. Error Resolution (P0) ✅ MOSTLY DONE

### Markdown Lint Errors

- **Before:** 877 warnings across project
- **After:** Fixed main offenders (README.md, CHANGELOG.md, docs/development/README.md, Engine/README.md)
- **Remaining:** Minor warnings in other docs (non-breaking, stylistic)
- **Added:** `.editorconfig` + `markdownlint.config` in settings.json to prevent future issues

### C++ Compile Errors

- **Status:** 0 errors, 0 warnings (as of last build)
- **Policy:** `/W4 /WX` in Release — warnings treated as errors

---

## Implementation Priority

### Step 1 (Immediate — v15.0.1)

| # | Item | Effort | Status |
|---|------|--------|--------|
| 1 | VS Code integration | Low | ✅ Done |
| 2 | Documentation standards | Low | ✅ Done |
| 3 | Markdown lint fixes | Low | ✅ Done |
| 4 | .editorconfig | Low | ✅ Done |
| 5 | tool-versions.md | Low | ✅ Done |

### Step 2 (Near-term — v16.0.0)

| # | Item | Effort | Status |
|---|------|--------|--------|
| 6 | Utils consolidation (ARM64) | Medium | ✅ Done (prior session) |
| 7 | Utils consolidation (Enterprise) | Medium | ✅ Done |
| 8 | Telemetry consolidation | Medium | ✅ Done (5→1) |
| 9 | UI/UX quick wins (Mica, rounded corners) | Medium | ✅ Done (Mica/Acrylic/rounded corners + DarkModeController wired) |
| 10 | Performance profiling + ETW | Medium | ✅ Done (ETWTraceProvider.h — real TraceLogging provider) |

### Step 3 (Medium-term — v16.x)

| # | Item | Effort | Status |
|---|------|--------|--------|
| 11 | Plugin system consolidation | Medium | ✅ Done (PluginMarketplaceUnified.h, PluginSecurity.h, PluginLifecycle.h — 3 umbrellas, 11 tests) |
| 12 | AI module implementations | High | ✅ Done (perceptual hashing, blur detection, smart crop, scene classification — 16 tests) |
| 13 | Build system unification (CMake) | High | ✅ Done (LENSShell + LENSManager CMakeLists.txt, root integration, SDK install target) |
| 14 | Shell integration (Win11 context menu) | Medium | ✅ Done (LENSShellContextMenu.h/.cpp — IExplorerCommand, 6 sub-commands, IDL/vcxproj/ObjectMap integrated) |

### Step 4 (Long-term — v17.0.0)

| # | Item | Effort | Status |
|---|------|--------|--------|
| 15 | WinUI 3 migration | Very High | Not started |
| 16 | Full dead code audit + removal | Medium | ✅ Done (audit complete, 30 headers registered, report at docs/DEAD_CODE_AUDIT.md) |
| 17 | Memory footprint optimization | High | Not started |
| 18 | libwebp CRT fix (/MD rebuild) | Low | ✅ Done |

---

## Success Criteria

- [x] All VS Code integration improvements applied ✅
- [x] Zero markdown lint warnings in main docs ✅
- [x] Tool versions documented and audit process defined ✅
- [x] Build passes with 0 errors, 0 warnings ✅ (43 targets, 0 errors, 0 warnings)
- [ ] All 1,187 existing tests pass
- [ ] Performance targets met (< 17ms single, > 235 img/s batch)
- [ ] Utils directory reduced from 134 to < 80 files
- [ ] Engine/Core reduced from 170+ to < 100 files
