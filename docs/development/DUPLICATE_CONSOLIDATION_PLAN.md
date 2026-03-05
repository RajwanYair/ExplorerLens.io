# Duplicate Consolidation Plan

> **Generated:** 2026-01-XX | **Status:** Tracking  
> **Priority:** High — reduces maintenance burden and avoids API confusion

This document tracks confirmed duplicate/redundant code and documentation across the
ExplorerLens codebase. Each group lists the involved files, the overlap, and the
recommended consolidation target.

---

## Priority 1 — Highest Impact (most files, most overlap)

### 1. Crash Handling (7 files → 2)

| File | Lines | Keep? |
|------|-------|-------|
| `Engine/Plugin/CrashHandler.h` | 153 | **Merge into CrashIntelligence.h** |
| `Engine/Plugin/CrashIntelligence.h` | 711 | **KEEP** (canonical) |
| `Engine/Plugin/CrashIntelligenceEngine.h` | 458 | Merge → CrashIntelligence.h |
| `Engine/Plugin/PluginCrashRecovery.h` | 213 | **Merge into PluginCrashIsolation.h** |
| `Engine/Plugin/PluginCrashIsolation.h` | 275 | **KEEP** (canonical) |
| `Engine/Core/CrashRecoveryEngine.h` | 106 | Keep (different layer — engine state) |
| `Engine/Core/CrashAnalyticsCollector.h` | 121 | Keep (different layer — telemetry) |

### 2. Plugin Marketplace (5 files → 2)

| File | Lines | Keep? |
|------|-------|-------|
| `Engine/Plugin/PluginMarketplaceUnified.h` | 364 | **KEEP** (canonical V1+V2+V3) |
| `Engine/Plugin/PluginMarketplace.h` | 661 | Strip to forwarding stub |
| `Engine/Plugin/PluginMarketplaceV2.h` | 8 | Remove (pure stub) |
| `Engine/Plugin/PluginMarketplaceV3.h` | 8 | Remove (pure stub) |
| `Engine/Plugin/PluginMarketplaceClient.h` | 208 | **KEEP** (client-side API) |

### 3. Plugin Hot Reload (3 files → 1)

| File | Lines | Keep? |
|------|-------|-------|
| `Engine/Plugin/PluginHotReload.h` | 120 | Merge → PluginHotReloadManager.h |
| `Engine/Plugin/PluginHotReloader.h` | 99 | Merge → PluginHotReloadManager.h |
| `Engine/Plugin/PluginHotReloadManager.h` | 350 | **KEEP** (most complete) |

### 4. Diagnostics (4 files → 2)

| File | Lines | Keep? |
|------|-------|-------|
| `Engine/Core/DiagnosticsCollector.h` | 465 | **KEEP** (system info) |
| `Engine/Core/DiagnosticCollector.h` | 365 | Merge → DiagnosticsCollector.h |
| `Engine/Core/DiagnosticsExporter.h` | 238 | **KEEP** (export/bundle) |
| `Engine/Core/DiagnosticReportGeneratorV2.h` | 122 | Merge → DiagnosticsExporter.h |

### 5. Color Space (3 files → 1)

| File | Lines | Keep? |
|------|-------|-------|
| `Engine/Core/ColorSpaceEngine.h` | 130 | Merge → ColorSpaceManager.h |
| `Engine/Core/ColorSpaceConverter.h` | 107 | Merge → ColorSpaceManager.h |
| `Engine/Decoders/ColorSpaceManager.h` | 502 | **KEEP** (most complete) |

---

## Priority 2 — Clear Consolidation Targets

### 6. AIThumbnailEnhancer (2 files → 1)

| File | Keep? |
|------|-------|
| `Engine/AI/AIThumbnailEnhancer.h` (203 lines) | **KEEP** (full implementation) |
| `Engine/Core/AIThumbnailEnhancer.h` (88 lines) | Convert to forwarding stub |

### 7. LocalizationEngine (2 files → 1)

| File | Keep? |
|------|-------|
| `Engine/Core/LocalizationEngine.h` (135 lines) | Merge → Utils version |
| `Engine/Utils/LocalizationEngine.h` (71 lines) | **KEEP** (expand with Core's content) |

### 8. Dead Code Audit (2 files → 1)

| File | Keep? |
|------|-------|
| `Engine/Core/DeadCodeAuditor.h` (96 lines) | Merge → DeadCodeAudit.h |
| `Engine/Core/DeadCodeAudit.h` (304 lines) | **KEEP** |

### 9. Documentation Generator (2 files → 1)

| File | Keep? |
|------|-------|
| `Engine/Utils/DocumentationGenerator.h` (498 lines) | **KEEP** |
| `Engine/Utils/DocGenerator.h` (451 lines) | Merge → DocumentationGenerator.h |

### 10. Enterprise Deployment (2 files → 1)

| File | Keep? |
|------|-------|
| `Engine/Utils/EnterpriseDeploymentManager.h` (74 lines) | Merge → EnterpriseDeployment.h |
| `Engine/Utils/EnterpriseDeployment.h` (569 lines) | **KEEP** |

### 11. Installer (2 files → 1)

| File | Keep? |
|------|-------|
| `Engine/Utils/InstallerManager.h` (540 lines) | **KEEP** (consolidated) |
| `Engine/Utils/InstallerLifecycleManager.h` (477 lines) | Merge → InstallerManager.h |

### 12. Code Coverage (3 files → 1)

| File | Keep? |
|------|-------|
| `Engine/Core/CodeCoverageEngine.h` (135 lines) | Merge → CodeCoverage.h |
| `Engine/Utils/CodeCoverage.h` (171 lines) | **KEEP** |
| `Engine/Utils/CodeCoverageIntegration.h` (160 lines) | Merge → CodeCoverage.h |

### 13. Memory Safety (2 files → 1)

| File | Keep? |
|------|-------|
| `Engine/Utils/MemorySafety.h` (403 lines) | **KEEP** (consolidated) |
| `Engine/Utils/MemorySafetyIntegration.h` (178 lines) | Merge → MemorySafety.h |

---

## Priority 3 — Cache Subsystem

### 14. PSO Cache Persistence (2 files → 1)

| File | Keep? |
|------|-------|
| `Engine/Cache/PSOPersistenceManager.h` (269 lines) | Merge → PSOCachePersistence.h |
| `Engine/Cache/PSOCachePersistence.h` (472 lines) | **KEEP** |

### 15. Cache Pre-warming (4 files → 2)

| File | Keep? |
|------|-------|
| `Engine/Cache/CacheWarmingService.h` (452 lines) | **KEEP** (service) |
| `Engine/Cache/CacheWarmingActivation.h` (238 lines) | Merge → CacheWarmingService.h |
| `Engine/Cache/CachePrewarmScheduler.h` (237 lines) | **KEEP** (scheduler) |
| `Engine/Cache/CachePreloader.h` (96 lines) | Merge → CachePrewarmScheduler.h |

### 16. Persistent Cache (2 files → 1)

| File | Keep? |
|------|-------|
| `Engine/Cache/PersistentDiskCache.h` (192 lines) | Merge → PersistentCacheManager.h |
| `Engine/Cache/PersistentCacheManager.h` (117 lines) | **KEEP** (expand) |

---

## Priority 4 — Pipeline & Decoder Routing

### 17. Decoder Priority/Scheduling (4 files → 2)

| File | Keep? |
|------|-------|
| `Engine/Core/DecoderPriority.h` | **KEEP** (selection logic) |
| `Engine/Core/DecoderPriorityRouter.h` | Merge → DecoderPriority.h |
| `Engine/Pipeline/DecoderPriorityScheduler.h` | **KEEP** (queue scheduler) |
| `Engine/Pipeline/PriorityDecodeScheduler.h` | Merge → DecoderPriorityScheduler.h |

### 18. Decoder Registry (2 files → 1)

| File | Keep? |
|------|-------|
| `Engine/Pipeline/DecoderRegistry.h` (134 lines) | Convert to forwarding stub |
| `Engine/Core/DecoderRegistryV2.h` (273 lines) | **KEEP** |

### 19. Decoder Health (4 files → 2)

| File | Keep? |
|------|-------|
| `Engine/Core/DecoderHealthMonitor.h` | Merge → DecoderHealthDashboard.h |
| `Engine/Core/DecoderHealthDashboard.h` | **KEEP** |
| `Engine/Core/DecoderPerformanceProfiler.h` | Merge → DecoderPerformanceCounters.h |
| `Engine/Core/DecoderPerformanceCounters.h` | **KEEP** |

### 20. Plugin Performance (2 files → 1)

| File | Keep? |
|------|-------|
| `Engine/Plugin/PluginPerformanceProfiler.h` (358 lines) | **KEEP** |
| `Engine/Plugin/PluginPerformanceMonitor.h` (97 lines) | Merge → PluginPerformanceProfiler.h |

---

## Documentation Duplicates

### 21. Plugin Development (3 docs → 1)

| File | Keep? |
|------|-------|
| `docs/PLUGIN_DEVELOPMENT.md` (860 lines) | **KEEP** (merge all into here) |
| `docs/plugins/PLUGIN_API.md` (798 lines) | Merge → PLUGIN_DEVELOPMENT.md |
| `SDK/docs/PLUGIN_SDK.md` (644 lines) | Merge → PLUGIN_DEVELOPMENT.md |

### 22. Architecture (3 docs → 1)

| File | Keep? |
|------|-------|
| `docs/README_ARCHITECTURE.md` (269 lines) | **KEEP** (top-level reference) |
| `docs/architecture/system-overview.md` (132 lines) | Merge → README_ARCHITECTURE.md |
| `docs/architecture/PROJECT_STRUCTURE.md` (619 lines) | Keep (directory reference — different purpose) |

### 23. Performance (2 docs → 1)

| File | Keep? |
|------|-------|
| `docs/PERFORMANCE.md` (800 lines) | **KEEP** |
| `docs/gpu/PERFORMANCE_METRICS.md` (282 lines) | Merge → PERFORMANCE.md |

---

## Build Script Duplicates

### 24. LibRaw Build (2 scripts → 1)

| File | Keep? |
|------|-------|
| `build-scripts/external-libs/Build-LibRaw.ps1` | **KEEP** (CMake-based) |
| `build-scripts/external-libs/Build-LibRaw-NMake.ps1` | Remove (deprecated) |

### 25. Find-MSBuild Forwarding

| File | Keep? |
|------|-------|
| `build-scripts/Find-MSBuild.ps1` | **KEEP** |
| `build-scripts/external-libs/Find-MSBuild.ps1` | Remove (pure forwarding wrapper) |

---

## Forwarding Stubs (Low Priority Cleanup)

These are harmless `#include` forwarding files for backward compatibility.
Remove only after confirming no external consumers depend on them:

- `Engine/Plugin/PluginMarketplaceV2.h` → PluginMarketplaceUnified.h
- `Engine/Plugin/PluginMarketplaceV3.h` → PluginMarketplaceUnified.h
- `Engine/Plugin/PluginCompatibilityKit.h` → PluginCompatibilityKitV2.h
- `Engine/Decoders/FITSDecoder.h` → FITSDecoderV2.h
- `Engine/Decoders/DICOMDecoder.h` → DICOMDecoderV2.h
- `Engine/AI/SmartCropEngine.h` → SmartCropV2.h
- `Engine/AI/ImageQualityAssessor.h` → ImageQualityAssessorV2.h
- `Engine/Decoders/PluginDecoder.h` → Plugin/PluginDecoder.h
- `Engine/Pipeline/FormatFallbackEngine.h` → Core/FormatFallbackEngine.h
- `Engine/Core/ConfigMigrationEngine.h` → Utils/ConfigMigrationEngine.h

---

## Summary

| Priority | Groups | Files Involved | Target Reduction |
|----------|--------|----------------|-----------------|
| P1 — Highest | 5 | 22 | → 8 files |
| P2 — Clear | 8 | 18 | → 8 files |
| P3 — Cache | 3 | 8 | → 4 files |
| P4 — Pipeline | 4 | 12 | → 6 files |
| Docs | 3 | 8 | → 3 files |
| Scripts | 2 | 4 | → 2 files |
| Stubs | — | 10 | → 0 files |
| **Total** | **25** | **82** | **→ 31 files** |

Net reduction: **~51 files** across all priorities.
