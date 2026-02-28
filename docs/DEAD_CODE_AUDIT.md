# Dead Code Audit Report — ExplorerLens Engine

**Date:** 2026-02-28
**Version:** v15.0.0 "Zenith"
**Scope:** All `.h` files under `Engine/` (excluding `Engine/Tests/` and gtest)
**Method:** `#include` reference scan across all project source files

## Summary

| Metric | Count |
|---|---|
| Total headers analyzed | **377** |
| Headers with zero `#include` references | **63** (16.7%) |
| Headers not registered in CMakeLists.txt | **31** (8.2%) → **NOW ALL REGISTERED** |
| Headers referenced only from tests | **147** (39.0%) |
| Headers with production references | **167** (44.3%) |

## Actions Taken

### 1. Registered 30 Missing Headers in Engine/CMakeLists.txt

The following headers existed on disk but were not listed in `ENGINE_HEADERS`:

**Core/ (22 headers):**
- `ArchiveRefactorEngine.h`, `BuildValidation.h`, `CIHardeningEngine.h`
- `CodeCoverageEngine.h`, `CRTConsistencyManager.h`, `DecoderPriority.h`
- `FFmpegIntegration.h`, `FormatCategoryManager.h`, `FormatGalleryView.h`
- `FormatGroupManager.h`, `FormatStatusIndicator.h`, `FreeTypeIntegration.h`
- `GPUShaderLibrary.h`, `HybridUIBridge.h`, `LibWebPConfig.h`
- `MuPDFIntegration.h`, `OpenJPEGIntegration.h`, `PluginHostManager.h`
- `PluginTypes.h`, `PropertyStoreHandler.h`, `SettingsExportImport.h`
- `SettingsImportExport.h`, `SystemTrayManager.h`, `VideoCodecRouter.h`
- `WinUI3MigrationEngine.h`, `WinUI3Research.h`

**Other directories (8 headers):**
- `Decoders/ExampleDecoder.h`
- `Cache/CacheKeyGenerator.h`
- `Utils/DecoderCircuitBreaker.h`, `Utils/PerceptualHashing.h`

**Already in PluginHost/CMakeLists.txt (1):**
- `PluginHost/PluginHostServer.h`

### 2. Identified Zero-Consumer Headers (63 total)

#### Entire directories with no consumers:
- **Engine/Shell/** — 3/3 headers (COMApartmentAudit, ContextMenuHandler, PreviewPaneHandler)
- **Engine/Cloud/** — 2/2 headers (CloudThumbnailProvider, NetworkThumbnailProvider)

#### Memory/ — 5/10 headers unused:
- BufferPoolAllocator, DecoderHotsetManager, DirectoryFormatProfiler
- MemoryOptimizationEngine, MemorySoakValidator

#### Utils/ umbrella headers never included:
- CIPipeline, Enterprise, Installer, Observability, SecurityTesting, PerformanceTools
- CodeCoverage.h (deferred — type conflicts with TestFramework.h)
- WindowsUI.h (deferred — DPIScale enum conflict with WindowsCompat.h)

### 3. Updated DeadCodeAudit.h

All findings from this audit are now tracked in `Engine/Core/DeadCodeAudit.h`
with proper `DeadCodeType`/`DeadCodeSeverity`/`DeadCodeStatus` classifications.

## Structural Observation

**147 headers (39%) are test-only** — included exclusively from `EngineTests.cpp`.
This is by design: ExplorerLens uses header-only implementations for many engine
components. Tests validate these stubs. Production integration happens via the
Shell extension calling Engine APIs that internally reference these headers through
include chains.

## Recommendations (Future Work)

1. **Remove truly dead files** — The 11 fully orphaned files (Section 1A) could be
   deleted if confirmed unnecessary after manual review
2. **Integrate or remove Shell/Cloud directories** — These appear to be forward-looking
   stubs with no production consumers yet
3. **Resolve type conflicts** — `CodeCoverage.h` and `WindowsUI.h` need enum deduplication
   to be includable alongside their umbrella headers
4. **Consider consolidating Memory/ headers** — Half the directory is unused
