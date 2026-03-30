# ExplorerLens Sprint Plan — 900 Series (Sprints 861–960)

**Version Range:** v28.6.0 "Polaris-W" → v29.7.0 "Capella-X"
**Baseline Tests at Sprint 860:** 3,429
**Projected Tests at Sprint 960:** 3,509 (+80)
**Codename Theme:** Polaris (wrap-up) → Capella (v29.x — Gen-5 Platform)

---

## Sprint 861–870 — v28.6.0 "Polaris-W"
**Theme: Post-Quantum Cryptography & Signature Verification**

ExplorerLens plugin trust chain and code-signing infrastructure must become resistant to
quantum computing attacks. This sprint upgrades the trust chain with CRYSTALS-Dilithium
signatures, adds quantum-resistant plugin manifests, and introduces a hybrid
classical-plus-PQC signature verification pipeline.

**Deliverables:**
- `Engine/Core/PQCSignatureVerifier.h`: Post-quantum CRYSTALS-Dilithium signature verification
- `Engine/Core/HybridTrustChainV2.h`: Hybrid classical + PQC dual-signature trust chain
- `Engine/Core/QuantumSafeKeyExchange.h`: ML-KEM (Kyber-1024) key exchange for plugin channels
- `Engine/Utils/DilithiumCertificateStore.h`: PQC certificate store (DER/PEM parsing)
- `Engine/Plugin/PQCPluginManifest.h`: PQC-signed plugin manifest with hybrid verification
- `Engine/Utils/SignatureAuditLogger.h`: Immutable audit log for all signature verification events
- `Engine/Core/CryptoAgilityBroker.h`: Crypto-agility framework (runtime algorithm selection)
- `Engine/Utils/KeyRotationScheduler.h`: Automated key rotation and certificate lifecycle manager

**Test Count:** 3,429 + 8 = **3,437**

---

## Sprint 871–880 — v28.7.0 "Polaris-X"
**Theme: Cross-Platform Preview Engine (macOS + Linux)**

Port the rendering pipeline to macOS (Metal) and Linux (Vulkan/OpenGL ES). Introduces
platform-neutral buffer types, a Metal command queue abstraction, and GTK4/Wayland
integration hooks.

**Deliverables:**
- `Engine/GPU/MetalRenderBridge.h`: macOS Metal rendering bridge (MTLTexture ↔ BGRA32)
- `Engine/GPU/LinuxVulkanPreview.h`: Linux Vulkan swapchain-less preview backend
- `Engine/Core/PlatformNeutralBuffer.h`: Cross-platform pixel buffer (Metal/D3D/Vulkan/CPU)
- `Engine/Utils/GTK4ThumbnailWidget.h`: GTK4 thumbnail display widget (Wayland + X11)
- `Engine/Core/MacOSShellBridge.h`: macOS Quick Look thumbnail provider bridge
- `Engine/Core/XDGThumbnailProvider.h`: Linux XDG thumbnail specification provider
- `Engine/GPU/MetalShaderCompiler.h`: Runtime Metal shader compilation + caching
- `Engine/Utils/PlatformCapabilityProbe.h`: Runtime platform feature detection (Metal/Vulkan/D3D)

**Test Count:** 3,437 + 8 = **3,445**

---

## Sprint 881–890 — v29.0.0 "Capella"  *(MAJOR — Gen-5 Platform)*
**Theme: Universal File System Integration & WinUI 4**

Major version bump to v29.0.0. Introduces the Gen-5 platform architecture: WinUI 4
IFilePreviewHandler integration, Universal File System (UFS) virtual provider, and
a new async preview broker that decouples thumbnail generation from the shell thread.

**Deliverables:**
- `Engine/Core/AsyncPreviewBroker.h`: Gen-5 async preview broker (COM out-of-proc)
- `Engine/Core/UniversalFileProvider.h`: Virtual file system provider (UFS abstraction)
- `Engine/Core/WinUI4PreviewHandler.h`: WinUI 4 IFilePreviewHandler integration
- `Engine/Core/ShellPropertyHandlerV2.h`: Extended property handler v2 (IPropertyStoreFactory)
- `Engine/Pipeline/PreviewPipelineV5.h`: Gen-5 preview pipeline (async stages, backpressure)
- `Engine/Cache/PersistentL3Cache.h`: Persistent L3 cross-session thumbnail cache (SQLite)
- `Engine/Core/LivePreviewUpdater.h`: Live preview update channel (file-watcher → thumbnail push)
- `Engine/Utils/ShellExtensionHealthMonitor.h`: Shell extension health watchdog + auto-recovery

**Test Count:** 3,445 + 8 = **3,453**

---

## Sprint 891–900 — v29.1.0 "Capella-R"
**Theme: Accessibility & Inclusive Design v2**

Expand screen reader support to full ARIA live-region thumbnails, WCAG 2.2 AA compliance
for the Manager GUI, high-contrast theme support, and AI-generated alt-text authoring.

**Deliverables:**
- `Engine/AI/AltTextGeneratorV2.h`: AI-based alt-text generation v2 (BLIP-2 multi-modal)
- `Engine/Utils/ARIAThumbnailAnnotator.h`: ARIA live-region thumbnail annotator for shell
- `Engine/Core/AccessibilityAuditEngine.h`: WCAG 2.2 AA automated audit engine
- `Engine/Utils/HighContrastThemeAdapter.h`: Runtime high-contrast theme color adapter
- `Engine/AI/CaptionQualityScorer.h`: Alt-text quality scorer (clarity + informativeness)
- `Engine/Utils/ScreenReaderBridge.h`: Bridge to IA2 / UIA screen reader interfaces
- `Engine/Core/KeyboardNavigationController.h`: Full keyboard-only thumbnail navigation
- `Engine/Utils/A11yTelemetryReporter.h`: Accessibility feature telemetry reporter

**Test Count:** 3,453 + 8 = **3,461**

---

## Sprint 901–910 — v29.2.0 "Capella-S"
**Theme: Project Consolidation Phase 1 — Version Sync & Documentation**

Project-wide consolidation sprint. Fix all stale version references (SBOMGenerator.h,
vcpkg.json, baseline.json stuck at v25.3.0). Archive obsolete docs (SPRINT_PLAN_600/700/800,
ROADMAP_V25). Merge 4 overlapping architecture docs into unified index. Merge 6 overlapping
build/release docs. Enhance Bump-Version.ps1 to handle all 12 version-bearing files.

**Deliverables:**
- Enhanced `build-scripts/Bump-Version.ps1`: Handles all 12 version files (SBOMGenerator.h, vcpkg.json, baseline.json, README.md, tool-versions.md, SBOM.json, architecture-build.svg)
- `docs/archive/SPRINT_PLAN_600.md`: Archived (moved from docs/)
- `docs/archive/SPRINT_PLAN_700.md`: Archived (moved from docs/)
- `docs/archive/SPRINT_PLAN_800.md`: Archived (moved from docs/)
- `docs/archive/ROADMAP_V25.md`: Archived (moved from docs/)
- Merged architecture docs into unified `docs/architecture/` index
- Merged build/release docs into unified `docs/development/` guide
- All 12 version-bearing files synced to v29.2.0

**Test Count:** 3,461 (no new tests — consolidation sprint)

---

## Sprint 911–920 — v29.3.0 "Capella-T"
**Theme: Project Consolidation Phase 2 — Cache Subsystem Deduplication**

Consolidate 77 cache headers into ~15 unified headers. Merge 3 cache migration engines
into single `CacheMigrationEngine`. Merge 4 cache warming strategies into single
`CacheWarmingService`. Merge 3 replication engines into single `CacheReplicator`.
Merge 2 partition managers. Remove deprecated cache schedulers. Consolidate cache
telemetry into main telemetry pipeline.

**Consolidation Actions:**
- Merge `CacheMigrationEngine` + `CacheMigrationManager` + `CacheMigrationTool` → unified `Engine/Cache/CacheMigrationEngine.h`
- Merge `CacheWarmingService` + `IdleCacheWarmer` + `EvictionAwareCachePrimer` + `PredictivePreGenEngine` → unified `Engine/Cache/CacheWarmingService.h`
- Merge `DistributedCacheSync` + `DeltaSyncReplicator` + `DistributedCacheReplicator` → unified `Engine/Cache/CacheReplicator.h`
- Merge `CachePartitionManager` + `ShardedCachePartitionV2` → unified `Engine/Cache/CachePartitionManager.h`
- Merge `CachePrewarmScheduler` + `CacheWarmingScheduler` → unified `Engine/Cache/CacheScheduler.h`
- Remove ~55 superseded cache headers from ENGINE_HEADERS
- Update all cache #include paths across Engine/
- Consolidate cache tests in EngineTests.cpp

**Headers Removed:** ~62 | **Headers Added:** 0 | **Net:** -62

**Test Count:** 3,461 (tests consolidated, not added)

---

## Sprint 921–930 — v29.4.0 "Capella-U"
**Theme: Project Consolidation Phase 3 — Recovery & Telemetry Unification**

Unify 6 competing recovery engines into single `RecoveryEngine` with pluggable strategy
pattern. Unify 7+ telemetry engines into single `TelemetryPipeline` with domain-specific
emitters. Merge duplicate `AuditLogger` / `AuditTrailLogger` into one.

**Consolidation Actions:**
- Merge `ErrorRecoveryEngine` + `ErrorRecoveryEngineV2` + `CrashRecoveryEngine` + `GPUErrorRecovery` + `PluginCrashRecovery` + `ShellExtensionRecovery` + `CacheResilienceManager` → unified `Engine/Core/RecoveryEngine.h` with `IRecoveryStrategy` interface
- Merge `TelemetryEngine` + `UsageTelemetryEngine` + `TelemetrySampler` + `TelemetryDataMinimizer` + `CacheTelemetryCollector` + `CollaborationTelemetryHub` + `PipelineTelemetryEmitter` → unified `Engine/Core/TelemetryPipeline.h` with `ITelemetryEmitter` interface
- Merge `AuditLogger` + `AuditTrailLogger` → unified `Engine/Core/AuditLogger.h`
- Remove ~12 superseded recovery/telemetry headers from ENGINE_HEADERS
- Update all recovery/telemetry #include paths across Engine/

**Headers Removed:** ~12 | **Headers Added:** 0 | **Net:** -12

**Test Count:** 3,461 (tests consolidated, not added)

---

## Sprint 931–940 — v29.5.0 "Capella-V"
**Theme: Project Consolidation Phase 4 — Scheduler & Format Router Consolidation**

Unify 10+ scheduler implementations behind `IScheduler` interface with 3 specializations
(decode, cache, power). Unify 6 format routing/detection classes into single
`DecoderRouter` with fallback chain. Remove deprecated routing oracles.

**Consolidation Actions:**
- Create `Engine/Core/IScheduler.h`: Common scheduler interface (priority, throttle, cancel)
- Merge `BatchDecodeScheduler` + `PriorityDecodeScheduler` + `DecoderPriorityScheduler` + `DeferredDecodeScheduler` + `AdaptivePipelineScheduler` → unified `Engine/Pipeline/DecodeScheduler.h`
- Merge `CacheWarmingScheduler` + `CachePrewarmScheduler` → already done in v29.3.0
- Merge `PowerAwareScheduler` + `ThermalAwareMemoryScheduler` + `PowerBudgetController` → unified `Engine/Core/PowerScheduler.h`
- Merge `AdaptiveDecoderRouter` + `DecoderPriorityRouter` + `DynamicFormatRouter` + `FormatDetectionOracle` + `FormatFallbackEngine` + `FormatNegotiator` → unified `Engine/Core/DecoderRouter.h` with fallback chain
- Remove ~13 superseded scheduler/router headers from ENGINE_HEADERS

**Headers Removed:** ~13 | **Headers Added:** 2 | **Net:** -11

**Test Count:** 3,461 (tests consolidated, not added)

---

## Sprint 941–950 — v29.6.0 "Capella-W"
**Theme: Project Consolidation Phase 5 — Scope Creep Extraction**

Extract out-of-scope modules from core Engine into optional components. Enterprise fleet
management, Docker/K8s/Electron platform adapters, AR/spatial rendering, post-quantum
crypto, UX A/B testing, and Cloud/SharePoint integration are moved to LENSManager or
optional plugin directories. Core engine DLL shrinks significantly.

**Consolidation Actions:**
- Move `Engine/Enterprise/` (17 headers) → `LENSManager/Enterprise/` (optional module)
- Move `Engine/Platform/` (8 headers: Docker, K8s, Electron, PWA) → `tools/platform-adapters/` (deployment tools)
- Move `Engine/AR/` (8 headers) → `Engine/Plugin/optional/AR/` (optional plugin)
- Move `Engine/Security/` PQC headers (9 headers) → `Engine/Plugin/optional/Security/` (optional plugin)
- Move `Engine/UX/` (8 headers: A/B testing, eye tracking) → `LENSManager/UX/` (manager feature)
- Move `Engine/Cloud/` (12 headers: SharePoint/Teams) → `Engine/Plugin/optional/Cloud/` (optional plugin)
- Remove all 62 moved headers from Engine/CMakeLists.txt ENGINE_HEADERS
- Update #include paths for any remaining cross-references

**Headers Removed from Engine:** 62 | **Net Engine reduction:** -62

**Test Count:** 3,461 (tests move with their modules)

---

## Sprint 951–960 — v29.7.0 "Capella-X"  *(Consolidation LTS)*
**Theme: Project Consolidation Phase 6 — Plugin, AI & Build Cleanup**

Final consolidation sprint. Merge 4 plugin marketplace versions into one. Extract
AI scope creep (LLM, federated learning) to optional modules. Consolidate 11 CLI tools
into unified `lens.exe` framework. Simplify build scripts. Full regression test pass
on the consolidated codebase.

**Consolidation Actions:**
- Merge `PluginMarketplace` + `V2` + `V3` → keep only `PluginMarketplaceUnified` in `Engine/Plugin/PluginMarketplace.h`
- Move AI scope creep (`LLMMIMEInferenceEngine`, `OnDeviceFineTuningEngine`, `FederatedLearningCoordinator`, `StyleTransferEngine` + 30 more) → `Engine/Plugin/optional/AI/` (optional plugin)
- Consolidate `Engine/CLI/` 11 separate tools → unified `Engine/CLI/LensCLI.h` command router
- Simplify `build-scripts/` — merge `Test-Builds.ps1` + `Verify-Complete-Build.ps1`, inline `.bat` wrappers
- Archive `docs/SPRINT_PLAN_600.md`, `700.md`, `800.md` to `docs/archive/`
- Full regression test verification on consolidated engine
- Update architecture documentation for new slimmed-down structure

**Headers Removed from Engine:** ~40 (AI) + 3 (Plugin) + 10 (CLI) = ~53 | **Net:** -53

**Test Count:** 3,461 (tests move with modules, consolidated engine verified)

---

## Cumulative Progress Tracker

| Sprint Range | Version                        | Theme                             | Test Δ | Headers Δ |
|-------------|-------------------------------|-----------------------------------|--------|-----------|
| 861–870     | v28.6.0 Polaris-W             | Post-Quantum Cryptography          | +8     | +8        |
| 871–880     | v28.7.0 Polaris-X             | Cross-Platform Preview (macOS/Linux) | +8   | +8        |
| 881–890     | v29.0.0 Capella (MAJOR)       | Gen-5 Platform + WinUI 4           | +8     | +8        |
| 891–900     | v29.1.0 Capella-R             | Accessibility v2                   | +8     | +8        |
| 901–910     | v29.2.0 Capella-S             | **Consolidation: Docs & Version Sync** | 0  | 0         |
| 911–920     | v29.3.0 Capella-T             | **Consolidation: Cache Dedup**     | 0      | **-62**   |
| 921–930     | v29.4.0 Capella-U             | **Consolidation: Recovery & Telemetry** | 0 | **-12**   |
| 931–940     | v29.5.0 Capella-V             | **Consolidation: Schedulers & Routers** | 0 | **-11**   |
| 941–950     | v29.6.0 Capella-W             | **Consolidation: Scope Creep Extraction** | 0 | **-62** |
| 951–960     | v29.7.0 Capella-X (LTS)       | **Consolidation: Plugin, AI & Build** | 0  | **-53**   |
| **861–960** | **Polaris-W through Capella-X** | **Gen-5 Platform + Consolidation** | **+32** | **-200** |

**Total at end of Sprint 960:** 3,429 + 32 = **3,461 unit tests** (consolidated)
**Engine headers at end of Sprint 960:** ~1,316 - 200 = **~1,116 headers** (17% reduction)

---

## Consolidation Metrics (Sprint 900 → 960)

| Metric | v29.1.0 Before | v29.7.0 Target | Change |
|--------|---------------|----------------|--------|
| Engine header files | 1,316 | ~1,116 | -200 (-15%) |
| Cache headers | 77 | ~15 | -62 (-80%) |
| Recovery engines | 6 | 1 | -5 (-83%) |
| Telemetry engines | 7+ | 2 | -5 (-71%) |
| Scheduler implementations | 10+ | 3 | -7 (-70%) |
| Format routers | 6 | 1 | -5 (-83%) |
| Plugin marketplace versions | 4 | 1 | -3 (-75%) |
| Out-of-scope modules in Engine | 6 dirs (62 .h) | 0 (moved) | -100% |
| Active docs (docs/) | 57 | ~30 | -27 (-47%) |
| Stale version files | 6 files at v25.3.0 | 0 | -100% |
| Memory footprint (idle) | 35 MB | 22 MB | -37% |
| Build time (full) | 120 s | 75 s | -38% |

---

## Architecture Pillars for v29.x Consolidation Era

1. **Lean Core Engine** — Only thumbnail-critical code in ExplorerLensEngine.lib
2. **Optional Plugins** — Enterprise, AR, Cloud, PQC Security as loadable plugins
3. **Unified Abstractions** — Single recovery, telemetry, scheduler, and router per concern
4. **Cache Simplification** — 5 core cache classes instead of 77
5. **Documentation Hygiene** — Single source of truth per topic, archived superseded plans
