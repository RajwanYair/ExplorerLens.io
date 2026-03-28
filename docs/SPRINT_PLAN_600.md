# ExplorerLens Sprint Plan — Sprints 561–660
# Versions v25.0.0 "Rigel" through v26.1.0 "Canopus-R"

This document covers the sixth hundred sprints (561–660) of the ExplorerLens roadmap,
advancing the project from v24.7.0 "Altair-X" through v26.1.0 "Canopus-R".

The **Rigel** series (v25.x) focuses on expanding the plugin ecosystem to WebAssembly,
adding neural format intelligence, NPU heterogeneous compute, VCS integration, self-healing
architecture, multi-instance coordination, collaborative features, and a full protocol
surface. **Canopus** (v26.0+) marks the transition to post-quantum cryptography and
next-generation Windows shell integration.

---

## Release Map

| Version  | Codename     | Sprints | Theme                                      | TestCount |
|----------|--------------|---------|--------------------------------------------|-----------|
| v25.0.0  | Rigel        | 561–570 | WebAssembly Plugin Sandbox (MAJOR)         | 3269      |
| v25.1.0  | Rigel-R      | 571–580 | Neural Format Intelligence                 | 3277      |
| v25.2.0  | Rigel-S      | 581–590 | NPU & Heterogeneous Compute                | 3285      |
| v25.3.0  | Rigel-T      | 591–600 | Source Control & VCS Integration           | 3293      |
| v25.4.0  | Rigel-U      | 601–610 | Self-Healing & Adaptive Recovery           | 3301      |
| v25.5.0  | Rigel-V      | 611–620 | Multi-Instance & Virtual Desktop           | 3309      |
| v25.6.0  | Rigel-W      | 621–630 | Collaborative Annotations & Sharing        | 3317      |
| v25.7.0  | Rigel-X      | 631–640 | Protocol Surface & API Ecosystem           | 3325      |
| v26.0.0  | Canopus      | 641–650 | Post-Quantum Security & Crypto (MAJOR)     | 3333      |
| v26.1.0  | Canopus-R    | 651–660 | Windows Next-Gen Shell Integration         | 3341      |

---

## Sprint 561–570 — WebAssembly Plugin Sandbox — MAJOR (v25.0.0 "Rigel")

**Theme:** First-class WebAssembly runtime for plugins — enabling sandboxed, cross-origin
plugins distributed as `.wasm` bundles. Applies the WASI/component-model interface binding
to IThumbnailPlugin, eliminating native DLL trust requirements while preserving full decode
throughput via a zero-copy linear-memory bridge.

| Sprint | Deliverable | File |
|--------|-------------|------|
| 561 | WASM runtime adapter (WasmEdge / WABT host) | `Engine/Plugin/WASMRuntimeAdapter.h` |
| 562 | WASM plugin memory safety model (linear mem + caps) | `Engine/Plugin/WASMMemorySafetyModel.h` |
| 563 | WASM plugin load/link engine (component model) | `Engine/Plugin/WASMPluginLoader.h` |
| 564 | WIT interface binding generator for plugin API | `Engine/Plugin/WITBindingGenerator.h` |
| 565 | Cross-process WASM host with resource limits | `Engine/Plugin/WASMHostController.h` |
| 566 | WASM plugin capability negotiation protocol | `Engine/Plugin/WASMCapabilityNegotiator.h` |
| 567 | WASM plugin hot-swap engine (live reload) | `Engine/Plugin/WASMHotSwapEngine.h` |
| 568–570 | WASM plugin debugger bridge / inspector protocol | `Engine/Plugin/WASMDebuggerBridge.h` |

---

## Sprint 571–580 — Neural Format Intelligence (v25.1.0 "Rigel-R")

**Theme:** ML-powered format detection and handling — train a binary-pattern classifier on
the full 200+ format corpus, synthesize decoder stubs for unknown formats, and track
format evolution to handle version drift without manual decoder updates.

| Sprint | Deliverable | File |
|--------|-------------|------|
| 571 | Neural format fingerprinter (binary patterns → class) | `Engine/AI/NeuralFormatFingerprinter.h` |
| 572 | Unknown format decoder request synthesizer | `Engine/AI/UnknownFormatHandler.h` |
| 573 | LLM-powered MIME inference from file headers | `Engine/AI/LLMMIMEInferenceEngine.h` |
| 574 | Self-expanding format registry (persists learned formats) | `Engine/Core/SelfExpandingFormatRegistry.h` |
| 575 | Transfer-learning fine-tuner for enterprise formats | `Engine/AI/FormatTransferLearner.h` |
| 576 | Confidence-scored format detection report | `Engine/Core/FormatDetectionReport.h` |
| 577 | Synthetic decoder stub generator for unsupported formats | `Engine/Core/SyntheticDecoderGenerator.h` |
| 578–580 | Format evolution tracker (version drift in binary sigs) | `Engine/AI/FormatEvolutionTracker.h` |

---

## Sprint 581–590 — NPU & Heterogeneous Compute (v25.2.0 "Rigel-S")

**Theme:** Expand GPU decode acceleration to NPUs — Intel Meteor Lake / Arrow Lake NPU via
OpenVINO, Qualcomm Hexagon DSP via the ONNX Runtime QNN EP, and a unified power-aware
work-item dispatcher that routes AI inference to the highest-TOPS available accelerator.

| Sprint | Deliverable | File |
|--------|-------------|------|
| 581 | Intel NPU (OpenVINO EP) decode acceleration | `Engine/GPU/IntelNPUBackend.h` |
| 582 | Qualcomm Hexagon DSP inference stub (ONNX QNN EP) | `Engine/GPU/HexagonDSPBackend.h` |
| 583 | ONNX Runtime EP router (NPU / GPU / CPU auto-select) | `Engine/GPU/ONNXEPRouter.h` |
| 584 | Hardware capability fingerprinter (TOPS rating) | `Engine/GPU/HardwareCapabilityProfiler.h` |
| 585 | Power-aware work-item scheduler (battery vs. plugged) | `Engine/Pipeline/PowerAwareScheduler.h` |
| 586 | NPU memory pool with zero-copy ONNX tensor input | `Engine/Memory/NPUMemoryPool.h` |
| 587 | NPU model warm-up / cold-start eliminator | `Engine/AI/NPUWarmupEngine.h` |
| 588–590 | ARM64 native decode path (Windows on ARM) | `Engine/GPU/ARM64DecodeBackend.h` |

---

## Sprint 591–600 — Source Control & VCS Integration (v25.3.0 "Rigel-T")

**Theme:** Surface version-control state directly in thumbnail overlays — Git/SVN/Perforce
status badges, branch-aware cache keys, LFS asset resolution, and diff thumbnails that
show before/after states side by side.

| Sprint | Deliverable | File |
|--------|-------------|------|
| 591 | Git status badge overlay (modified/staged/untracked/conflict) | `Engine/Core/GitStatusOverlay.h` |
| 592 | Git blame heatmap overlay (recent-edit recency colouring) | `Engine/Core/GitBlameHeatmapOverlay.h` |
| 593 | Perforce / SVN status badge adapter | `Engine/Core/VCSBadgeAdapter.h` |
| 594 | Branch-aware cache key (repo + branch + commit hash) | `Engine/Cache/BranchAwareCacheKey.h` |
| 595 | Git diff thumbnail (before/after split-view render) | `Engine/Core/GitDiffThumbnail.h` |
| 596 | Git LFS pointer resolver (fetch large asset for preview) | `Engine/Core/GitLFSResolver.h` |
| 597 | Commit hash + author badge compositor | `Engine/Core/CommitBadgeCompositor.h` |
| 598–600 | Merge conflict marker detector and overlay renderer | `Engine/Core/MergeConflictOverlay.h` |

---

## Sprint 601–610 — Self-Healing & Adaptive Recovery (v25.4.0 "Rigel-U")

**Theme:** A resilient, self-repairing engine — decoder crash prediction via ML on error
history, automatic quarantine and recovery, adaptive per-format timeouts, and a boot-time
integrity self-test that validates COM registration and all decoder binaries on startup.

| Sprint | Deliverable | File |
|--------|-------------|------|
| 601 | Decoder crash predictor (ML on error/latency history) | `Engine/Core/DecoderCrashPredictor.h` |
| 602 | Automatic decoder quarantine & recovery orchestrator | `Engine/Core/DecoderQuarantineManager.h` |
| 603 | Adaptive per-format timeout tuner (latency learning) | `Engine/Core/AdaptiveTimeoutTuner.h` |
| 604 | Heap corruption sentinel (guard-page + canary checks) | `Engine/Core/HeapCorruptionSentinel.h` |
| 605 | Silent retry policy engine (exponential backoff + jitter) | `Engine/Pipeline/RetryPolicyEngine.h` |
| 606 | Decoder health snapshot + automated incident report | `Engine/Core/DecoderIncidentReporter.h` |
| 607 | Self-repair COM registration validator & fixer | `Engine/Utils/COMSelfRepairValidator.h` |
| 608–610 | Boot-time integrity self-test suite | `Engine/Utils/BootIntegritySelfTest.h` |

---

## Sprint 611–620 — Multi-Instance & Virtual Desktop (v25.5.0 "Rigel-V")

**Theme:** Correct and efficient operation across multiple Explorer instances, virtual
desktops, RDS/Citrix multi-user sessions, and tabbed Explorer — with a cross-session
read-only shared thumbnail pool and intelligent foreground-window priority inheritance.

| Sprint | Deliverable | File |
|--------|-------------|------|
| 611 | Virtual Desktop Manager awareness (per-VD cache scope) | `Engine/Core/VirtualDesktopAwareness.h` |
| 612 | Multi-user WTS session isolation (RDS / Citrix) | `Engine/Core/WTSSessionIsolation.h` |
| 613 | Per-monitor DPI thumbnail resolution selector v2 | `Engine/Core/PerMonitorDPISelectorV2.h` |
| 614 | Tabbed Explorer pane synchronizer | `Engine/Core/TabbedExplorerSync.h` |
| 615 | Cross-session read-only shared thumbnail pool (mmap) | `Engine/Memory/CrossSessionThumbnailPool.h` |
| 616 | Instance registry with heartbeat & stale cleanup | `Engine/Core/InstanceRegistry.h` |
| 617 | Foreground-window priority inheritance for decode | `Engine/Pipeline/ForegroundPriorityInheritance.h` |
| 618–620 | Load-balanced decode dispatch across instances | `Engine/Pipeline/CrossInstanceLoadBalancer.h` |

---

## Sprint 621–630 — Collaborative Annotations & Sharing (v25.6.0 "Rigel-W")

**Theme:** Turn thumbnails into a collaboration surface — per-file annotations (stars, tags,
comments) stored in a local SQLite database with optional cloud sync to SharePoint/OneDrive,
plus Teams/Slack webhook bridges for team-level sharing and review workflows.

| Sprint | Deliverable | File |
|--------|-------------|------|
| 621 | Per-file annotation store (SQLite, sync-ready schema) | `Engine/Core/AnnotationStore.h` |
| 622 | Annotation overlay renderer (stars, tags, comments) | `Engine/Core/AnnotationOverlayRenderer.h` |
| 623 | Teams / Slack annotation webhook bridge | `Engine/Utils/CollabWebhookBridge.h` |
| 624 | Shared collection builder (folder-level annotation sets) | `Engine/Core/SharedCollectionBuilder.h` |
| 625 | Annotation diff viewer (provenance & change history) | `Engine/Core/AnnotationDiffViewer.h` |
| 626 | Annotation export engine (JSON / XML / CSV / EXIF sidecar) | `Engine/Utils/AnnotationExporter.h` |
| 627 | SharePoint / OneDrive annotation round-trip sync | `Engine/Utils/CollabCloudSync.h` |
| 628–630 | Annotation schema versioning & migration engine | `Engine/Core/AnnotationSchemaMigrator.h` |

---

## Sprint 631–640 — Protocol Surface & API Ecosystem (v25.7.0 "Rigel-X")

**Theme:** Open the thumbnail engine to external consumers via gRPC, REST, and GraphQL.
Embed a lightweight HTTP server, publish an auto-generated OpenAPI 3.1 spec, and ship SDK
language bindings for C#, Python, and PowerShell so CI/CD pipelines and web services
can generate thumbnails programmatically.

| Sprint | Deliverable | File |
|--------|-------------|------|
| 631 | gRPC thumbnail service (proto3 + server-streaming) | `Engine/Utils/GRPCThumbnailService.h` |
| 632 | Embedded REST/HTTP thumbnail API (cpp-httplib) | `Engine/Utils/RESTThumbnailServer.h` |
| 633 | GraphQL schema for format + thumbnail metadata | `Engine/Utils/GraphQLQueryEngine.h` |
| 634 | WebSocket live-update channel (push thumbnail-ready) | `Engine/Utils/WebSocketPushChannel.h` |
| 635 | OpenAPI 3.1 spec auto-generator from routes | `Engine/Utils/OpenAPISpecGenerator.h` |
| 636 | SDK language bindings generator (C# / Python / PS) | `Engine/Utils/SDKBindingsGenerator.h` |
| 637 | OAuth 2.0 token validation for remote API access | `Engine/Utils/OAuthTokenValidator.h` |
| 638–640 | Rate limiter + API quota & burst-control engine | `Engine/Utils/APIRateLimiter.h` |

---

## Sprint 641–650 — Post-Quantum Security & Crypto — MAJOR (v26.0.0 "Canopus")

**Theme:** Prepare ExplorerLens for the post-quantum era — NIST FIPS 203/205 compliant
key encapsulation (ML-KEM) and signature (SLH-DSA) for plugin signing, hybrid TLS for IPC
channels, and a crypto-agility engine that enables seamless algorithm negotiation and
future-proof upgrades without breaking existing plugins.

| Sprint | Deliverable | File |
|--------|-------------|------|
| 641 | ML-KEM (FIPS 203) key encapsulation for plugin signing | `Engine/Utils/MLKEMKeyEncapsulator.h` |
| 642 | SLH-DSA (FIPS 205) stateless hash-based signature verifier | `Engine/Utils/SLHDSASignatureVerifier.h` |
| 643 | Hybrid TLS (Kyber + X25519) encrypted IPC channel | `Engine/Core/HybridTLSIPCChannel.h` |
| 644 | PQ-resistant audit trail (Merkle-tree hash chain) | `Engine/Utils/PQAuditTrail.h` |
| 645 | Certificate migration tool (RSA / ECDSA → ML-DSA) | `Engine/Utils/CertificateMigrationTool.h` |
| 646 | Quantum-safe key rotation scheduler | `Engine/Utils/QuantumSafeKeyRotator.h` |
| 647 | FIPS 140-3 Level 2+ crypto boundary module | `Engine/Utils/FIPS140CryptoBoundary.h` |
| 648–650 | Crypto agility engine (algorithm negotiation + hot-upgrade) | `Engine/Utils/CryptoAgilityEngine.h` |

---

## Sprint 651–660 — Windows Next-Gen Shell Integration (v26.1.0 "Canopus-R")

**Theme:** Deep integration with the next generation of Windows — Windows App SDK 2.x
WinRT thumbnail projection, Copilot+ AI Platform API hooks, AppContainer isolation for
the COM server, MSIX streaming install thumbnail pre-warming, and Windows Hello biometric
authorization for classified-tier plugin operations.

| Sprint | Deliverable | File |
|--------|-------------|------|
| 651 | Windows App SDK 2.x WinRT thumbnail provider bridge | `Engine/Core/WinRTThumbnailBridge.h` |
| 652 | Windows Copilot+ AI Platform API integration | `Engine/AI/CopilotPlatformBridge.h` |
| 653 | AppContainer isolation for COM thumbnail server | `Engine/Core/AppContainerIsolation.h` |
| 654 | WinFS-style metadata store for thumbnail properties | `Engine/Core/WinFSMetadataStore.h` |
| 655 | Windows Search v3 AQS-powered thumbnail index | `Engine/Core/WindowsSearchV3Bridge.h` |
| 656 | Smart App Control plugin signing compliance | `Engine/Utils/SmartAppControlPolicy.h` |
| 657 | MSIX streaming install thumbnail pre-warm engine | `Engine/Core/MSIXStreamingPrewarmer.h` |
| 658–660 | Windows Hello biometric plugin authorization | `Engine/Utils/WindowsHelloAuthBridge.h` |

---

## Performance Targets — Rigel Series

| Metric | v24.7.0 Target | v25.0.0 Target | v26.0.0 Target |
|--------|---------------|---------------|---------------|
| Single thumbnail latency | <17ms | **<12ms** | **<9ms** |
| Batch throughput | 235 img/sec | **300 img/sec** | **380 img/sec** |
| Cache hit latency | <5ms | **<3ms** | **<2ms** |
| AI inference (NPU path) | 4ms GPU | **1.5ms NPU** | **1.2ms NPU** |
| Memory footprint (idle) | <80 MB | **<60 MB** | **<50 MB** |
| Cold-start to first thumbnail | <250ms | **<180ms** | **<120ms** |

---

## Quality Gates

Each Rigel/Canopus release must pass:

1. **Zero build warnings** on MSVC v145 and Clang 18+ (CI)
2. **100% unit test pass rate** at the published test count
3. **Performance regression gate** — no >5% latency increase from previous version
4. **Fuzz test budget** — 24 h corpus fuzzing on all new decoders (libFuzzer)
5. **WASM plugin interop** — reference WASM plugin generates valid thumbnail on all 10 CI images
6. **Post-quantum signature check** — all plugin bundles signed with ML-DSA; verify on install
7. **API compatibility** — gRPC service protobuf breaks require major version bump
8. **Accessibility audit** — WCAG 2.2 AA pass on LENSManager UI with new features

---

## Technical Debt Retirement (Alongside Rigel Sprint Work)

| Item | Target Version | Action |
|------|----------------|--------|
| Stale `ROADMAP_V16.md` | v25.0.0 | Archive to `docs/archive/` |
| ~~`IPC::ThumbnailRequest` → `IPC::IPCThumbnailRequest` cascade~~ | ~~v25.0.0~~ | ✅ **Done in v24.1.0 cleanup** — fix13–fix16 scripts removed, type renames complete |
| ~~`EngineTests.cpp` type-rename cascade fixes~~ | ~~v25.0.0~~ | ✅ **Done in v24.1.0 cleanup** — fix scripts deleted |
| PERFORMANCE.md version references (v20.2.0) | v25.0.0 | Update to current version |
| ENTERPRISE.md version references (v19.1.0) | v25.0.0 | Update to current version |
| `FEATURE_FREEZE_16.md`, `MIGRATION_GUIDE_V16.md` | v25.0.0 | Archive to `docs/archive/` |
| ~~Duplicate `Find-MSBuild.ps1` in `build-scripts/`~~ | ~~v25.0.0~~ | ✅ **Done in v24.1.0 cleanup** — removed; all callers updated to `Find-MSBuildPath` |

---

## Execution & Release Procedure

Each sprint block follows this pipeline:

```powershell
# 1. Create 8 header files (header-only, fully inlined implementations)
# 2. Register all 8 headers in Engine/CMakeLists.txt ENGINE_HEADERS
# 3. Add includes + TEST() + RUN_TEST() to Engine/Tests/EngineTests.cpp
# 4. Bump version + tag + push:
.\build-scripts\Bump-Version.ps1 -Version "X.Y.Z" -Codename "Name" -TestCount N `
    -ChangelogEntry "..." -TagAndPush
# 5. GitHub Actions release.yml fires on tag — publishes full release artifacts
```

---

## Version History Reference

| Version  | Codename     | Theme                                      | Status    |
|----------|--------------|--------------------------------------------|-----------|
| v24.7.0  | Altair-X     | Developer Experience v2                    | Planned   |
| v25.0.0  | Rigel        | WebAssembly Plugin Sandbox (MAJOR)         | Planned   |
| v25.1.0  | Rigel-R      | Neural Format Intelligence                 | Planned   |
| v25.2.0  | Rigel-S      | NPU & Heterogeneous Compute                | Planned   |
| v25.3.0  | Rigel-T      | Source Control & VCS Integration           | Planned   |
| v25.4.0  | Rigel-U      | Self-Healing & Adaptive Recovery           | Planned   |
| v25.5.0  | Rigel-V      | Multi-Instance & Virtual Desktop           | Planned   |
| v25.6.0  | Rigel-W      | Collaborative Annotations & Sharing        | Planned   |
| v25.7.0  | Rigel-X      | Protocol Surface & API Ecosystem           | Planned   |
| v26.0.0  | Canopus      | Post-Quantum Security & Crypto (MAJOR)     | Planned   |
| v26.1.0  | Canopus-R    | Windows Next-Gen Shell Integration         | Planned   |
