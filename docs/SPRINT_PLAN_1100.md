# ExplorerLens Sprint Plan — 1100 Series (Sprints 1061–1160)
# Versions v31.9.0 "Achernar-Z" through v33.1.0 "Vega-R"

**Baseline Tests at Sprint 1060:** 4,218
**Projected Tests at Sprint 1160:** 4,938 (+720)
**Codename Theme:** Achernar (v31.9 — Final) → Fomalhaut (v32.x — Security & Edge) → Vega (v33.x — Spatial & Mixed Reality)
**Previous Plan:** [SPRINT_PLAN_1000.md](SPRINT_PLAN_1000.md) (Sprints 961–1060, v30.0.0–v31.8.0)

---

## Overview: The Post-Quantum Security Era

In the 1100 series, ExplorerLens completes the Achernar generative era and enters two new
major epochs:

**Fomalhaut (v32.x) — Zero-Trust Security Foundation:** The rising threat of quantum
computing against RSA/ECDSA key infrastructure demands proactive migration. ExplorerLens v32
introduces full post-quantum cryptographic primitives (CRYSTALS-Kyber key exchange,
CRYSTALS-Dilithium signatures), a zero-trust plugin access model, hardware-accelerated AI
inference at the network edge, and seamless TPM 2.0 attestation for enterprise deployments.

**Vega (v33.x) — Spatial Computing & Mixed Reality:** The AR/VR headset inflection is here.
v33.0.0 delivers native ExplorerLens integration with Windows Mixed Reality, Apple Vision Pro
via visionOS Quick Look, and Meta Horizon OS — giving spatial file browsing a photorealistic
thumbnail layer with sub-10 ms holographic projection latency.

### Design Principles

1. **Quantum-Safe by Default** — all new cryptographic primitives are post-quantum resistant
2. **Zero Trust, Zero Exceptions** — every plugin, decoder, and network call requires a JWT capability token
3. **Spatial-First UX** — thumbnail interaction model extended to 6DoF (six degrees of freedom)
4. **Edge-Native Inference** — AI models run on NPU/XDNA silicon, not GPU (GPU freed for render)

---

## Release Map

| Version  | Codename      | Sprints   | Theme                                            | Tests  |
|----------|---------------|-----------|--------------------------------------------------|--------|
| v31.9.0  | Achernar-Z    | 1061–1070 | Final Achernar: Autonomous Shell Intelligence    | 4,290  |
| v32.0.0  | Fomalhaut     | 1071–1080 | Post-Quantum Security & Zero-Trust (MAJOR)       | 4,362  |
| v32.1.0  | Fomalhaut-R   | 1081–1090 | Edge AI & Hardware-Accelerated Inference         | 4,434  |
| v32.2.0  | Fomalhaut-S   | 1091–1100 | Immersive 3D Preview & Holographic Thumbnails    | 4,506  |
| v32.3.0  | Fomalhaut-T   | 1101–1110 | Federated Learning & Privacy-Preserving AI       | 4,578  |
| v32.4.0  | Fomalhaut-U   | 1111–1120 | Real-Time Collaboration & Multiplayer Preview    | 4,650  |
| v32.5.0  | Fomalhaut-V   | 1121–1130 | Cross-Cloud Asset Sync & Hybrid Pipeline         | 4,722  |
| v32.6.0  | Fomalhaut-W   | 1131–1140 | Developer SDK v5 & Plugin Intelligence           | 4,794  |
| v33.0.0  | Vega          | 1141–1150 | Spatial Computing & Mixed Reality (MAJOR)        | 4,866  |
| v33.1.0  | Vega-R        | 1151–1160 | Apple Vision Pro + Meta Horizon Integration      | 4,938  |

---

## Sprint 1061–1070 — Final Achernar: Autonomous Shell Intelligence (v31.9.0 "Achernar-Z")

**Theme:** The landmark final release of the Achernar series. Six months of AI-native
components culminate in a fully autonomous shell intelligence layer — the engine now
anticipates user workflows, self-tunes pipeline parameters, manages cross-platform shell
extension lifecycles, and ranks thumbnail relevance without any user configuration.
This sprint also resolves all remaining Achernar-series backlog (cross-platform extensions,
enterprise fleet management, compliance reporting).

**Strategic Motivation:**
- Complete the Achernar generative era with a production-ready autonomous layer
- Resolve all backlog .cpp translation units from v30.7.0–v31.1.0 sprints
- Prepare the clean foundation required for the Fomalhaut security overhaul

**Deliverables:**

| # | Header | Purpose |
|---|--------|---------|
| 1 | `Engine/AI/AutonomousWorkflowOrchestrator.h` | Fully autonomous thumbnail workflow scheduler — ML policy drives decode order, concurrency, and quality |
| 2 | `Engine/AI/ShellIntelligenceAdapter.h` | AI-native shell adapter bridging Engine AI models to Windows/Linux/macOS shell providers |
| 3 | `Engine/Core/CrossPlatformCapabilityBroker.h` | Runtime capability negotiation across all three PAL backends (Win/macOS/Linux) |
| 4 | `Engine/Core/AdaptiveShellIntegrationEngine.h` | Adaptive shell integration that self-tunes to OS API updates via capability probing |
| 5 | `Engine/Pipeline/AutotuningPipelineEngine.h` | Self-tuning pipeline with reinforcement-learning parameter feedback loop |
| 6 | `Engine/Core/ShellExtensionLifecycleManager.h` | Unified lifecycle manager for COM / QLGenerator / GIO / Dolphin shell extensions |
| 7 | `Engine/AI/ThumbnailRelevanceRanker.h` | ML-based relevance ranker for large-batch thumbnail requests (recency + visual + frequency) |
| 8 | `Engine/Utils/CrossPlatformBuildValidator.h` | Cross-platform build matrix validator — CI gate ensuring Windows/macOS/Linux parity |

**Backlog Resolved:**
- `Engine/Core/LinuxNautilusExtension.cpp` — TU for v31.1.0 header
- `Engine/Core/KDEDolphinExtension.cpp` — TU for v31.1.0 header
- `Engine/Core/ThunarThumbnailExtension.cpp` — TU for v31.1.0 header
- `Engine/Core/macOSQuickLookV3.cpp` — TU for v31.1.0 header
- `Engine/Core/LinuxThumbnailerDaemon.cpp` — TU for v31.1.0 header
- `Engine/Core/WaylandShellExtension.cpp` — TU for v31.1.0 header
- `Engine/Core/XPlatformTestHarness.cpp` — TU for v31.1.0 header
- `Engine/Core/macOSLaunchServicesAdapter.cpp` — TU for v31.1.0 header
- `Engine/Enterprise/FleetDeploymentManager.cpp` — TU for v30.7.0 header
- `Engine/Enterprise/ComplianceReportGenerator.cpp` — TU for v30.7.0 header
- `Engine/Enterprise/PolicyVersionControl.cpp` — TU for v30.7.0 header
- `Engine/Enterprise/RemoteDecoderControl.cpp` — TU for v30.7.0 header

**Test additions:** +72 tests (4,218 → 4,290)

**Acceptance Criteria:**
- `AutonomousWorkflowOrchestrator` policy improves throughput ≥ 15% on mixed-format batch vs. FIFO baseline
- `AutotuningPipelineEngine` converges to near-optimal parameters within 100 thumbnail samples
- `CrossPlatformCapabilityBroker` correctly routes capability queries on all three platforms in CI
- `ThumbnailRelevanceRanker` ranks recently-accessed files within top-3 on 85% of test queries
- All 12 backlog .cpp TUs compile without warnings under MSVC v145 and GCC 14
- Zero new warnings on any compiler

---

## Sprint 1071–1080 — Post-Quantum Security & Zero-Trust Architecture (v32.0.0 "Fomalhaut") ★★ MAJOR

**Theme:** ExplorerLens becomes cryptographically quantum-safe. All plugin signatures, cache
integrity, and inter-process communication are upgraded to NIST PQC finalists (CRYSTALS-Kyber
for KEM, CRYSTALS-Dilithium for signatures, SPHINCS+ for stateless hash-based signing). A
zero-trust access model ensures every plugin, decoder, and capability must present a signed
JWT capability token before receiving system access — replacing the legacy ACL-based model.

**Strategic Motivation:**
- NIST PQC standards finalized 2024; enterprise procurement now requires PQC roadmaps
- Classic RSA-2048 has a ~10-year harvest-now/decrypt-later risk horizon
- Zero-trust is the security baseline for FedRAMP Moderate and ISO 27001 2022 compliance
- Hardware TPM attestation is available on >95% of Windows 11 business endpoints

**Deliverables:**

| # | Header | Purpose |
|---|--------|---------|
| 1 | `Engine/Core/PostQuantumCryptoProvider.h` | CRYSTALS-Kyber (KEM) + Dilithium (DSA) + SPHINCS+ provider — NIST FIPS 203/204/205 |
| 2 | `Engine/Core/ZeroTrustAccessBroker.h` | Zero-trust access broker — capability token issuance, validation, and revocation |
| 3 | `Engine/Core/QuantumResistantHashEngine.h` | SHA-3-256 / BLAKE3 / KangarooTwelve quantum-resistant hash engine |
| 4 | `Engine/Plugin/PluginZeroTrustSandbox.h` | Plugin sandbox enforcing zero-trust: capability tokens required for all system access |
| 5 | `Engine/Core/BinaryTrustVerifier.h` | Binary DLL/dylib/so trust chain verifier using post-quantum Dilithium signatures |
| 6 | `Engine/Core/SecureConfigurationManager.h` | TPM 2.0-backed encrypted configuration store (DPAPI on Windows, Secure Enclave on macOS) |
| 7 | `Engine/Core/ThreatModelingEngine.h` | Runtime STRIDE threat model validator — gates pipeline stage transitions on threat posture |
| 8 | `Engine/Utils/SecurityPostureAnalyzer.h` | Security posture analyzer: TPM attestation + code integrity + patch level scoring |

**Test additions:** +72 tests (4,290 → 4,362)

**Acceptance Criteria:**
- `PostQuantumCryptoProvider` passes all NIST KAT (Known Answer Test) vectors for Kyber-768 and Dilithium3
- `ZeroTrustAccessBroker` validates a signed JWT capability token in < 0.5 ms (local verification)
- `PluginZeroTrustSandbox` blocks a plugin without a valid capability token before any system call
- `BinaryTrustVerifier` detects a tampered DLL and returns structured rejection evidence
- `ThreatModelingEngine` identifies a STRIDE spoofing scenario in a synthetic test pipeline
- `SecurityPostureAnalyzer` generates a posture report compatible with Microsoft Secure Score JSON schema
- All PQC operations are constant-time to resist timing side-channels

---

## Sprint 1081–1090 — Edge AI & Hardware-Accelerated Inference (v32.1.0 "Fomalhaut-R")

**Theme:** Move all AI inference from GPU to dedicated NPU/AI-accelerator silicon, freeing
the GPU exclusively for rendering. AMD XDNA (Ryzen AI), Qualcomm AI Engine (Snapdragon X),
and Intel AMX (Advanced Matrix Extensions on Intel Core Ultra) are first-class targets.
A `ComputeDeviceRegistry` routes inference requests to the optimal hardware with sub-1 ms
dispatch latency and transparent CPU fallback.

**Strategic Motivation:**
- NPU-offloaded AI inference achieves 3–5× lower power vs. GPU path (critical for mobile)
- Snapdragon X and Ryzen AI cover >40% of new enterprise laptop shipments in 2026
- Dedicated NPU leaves GPU thermal budget fully available for 60fps holographic rendering

**Deliverables:**

| # | Header | Purpose |
|---|--------|---------|
| 1 | `Engine/GPU/NPUAccelerationEngine.h` | Unified NPU acceleration engine — dispatches ONNX/DirectML workloads to NPU/XDNA/AMX |
| 2 | `Engine/GPU/EdgeAIInferenceEngine.h` | Edge AI inference coordinator — session lifecycle, batching, memory-mapped weights |
| 3 | `Engine/AI/HardwareCapabilityNegotiator.h` | Hardware capability negotiator — selects optimal backend (NPU/GPU/CPU) per inference task |
| 4 | `Engine/GPU/AMDXDNABackend.h` | AMD XDNA NPU backend (Ryzen AI 300 / Strix Halo — DirectML EP + custom MLIR kernels) |
| 5 | `Engine/GPU/QualcommAIEBackend.h` | Qualcomm AI Engine Direct backend (Snapdragon X Elite QNN SDK v2.x) |
| 6 | `Engine/GPU/IntelAMXBackend.h` | Intel AMX (Advanced Matrix Extensions) backend for BF16/INT8 matrix inference (Granite Rapids+) |
| 7 | `Engine/Pipeline/HardwareAcceleratedPipeline.h` | Hardware-accelerated pipeline stage — routes decode + AI inference to optimal silicon at runtime |
| 8 | `Engine/Core/ComputeDeviceRegistry.h` | Compute device registry — enumerates and profiles all available AI accelerators at startup |

**Test additions:** +72 tests (4,362 → 4,434)

**Acceptance Criteria:**
- `AMDXDNABackend` executes a CLIP embedding pass on Ryzen AI 9 HX 370 in < 8 ms (mocked in CI)
- `QualcommAIEBackend` completes a ResNet-50 forward pass on Snapdragon X (QNN SDK mock) in < 12 ms
- `IntelAMXBackend` achieves 2× throughput vs SSE4.2 baseline on BF16 matrix multiplication
- `ComputeDeviceRegistry` enumerates all accelerators reliably in all supported Windows 11 configurations
- `HardwareAcceleratedPipeline` falls back transparently to CPU path when no accelerator is present
- `HardwareCapabilityNegotiator` selects NPU over GPU for the CLIP embedding task in the synthetic benchmark

---

## Sprint 1091–1100 — Immersive 3D Preview & Holographic Thumbnails (v32.2.0 "Fomalhaut-S")

**Theme:** Extend thumbnails from 2D bitmaps to interactive 3D holographic previews. Model
files (.glb, .usd, .fbx) render as spin-able miniatures; point clouds display as animated
volumetric data; volumetric medical scans show a fly-through preview. The holographic
renderer targets Windows Mixed Reality, and outputs standard flat thumbnails as the fallback.

**Deliverables:**

| # | Header | Purpose |
|---|--------|---------|
| 1 | `Engine/Core/HolographicThumbnailRenderer.h` | Holographic 3D thumbnail renderer — D3D12 + Windows Mixed Reality spatial mapping integration |
| 2 | `Engine/Core/VolumetricDataPreviewEngine.h` | Volumetric data preview — medical NRRD/DICOM and geospatial voxel fly-through at 60fps |
| 3 | `Engine/GPU/SpatialRenderingPipeline.h` | Spatial rendering pipeline — stereoscopic + reprojection + late-latch frame timing |
| 4 | `Engine/Core/DepthMeshExtractor.h` | Depth-aware mesh extraction for thumbnail 3D projection from 2D+depth pairs |
| 5 | `Engine/AI/Scene3DReconstructionEngine.h` | AI-driven 3D scene reconstruction from single-view images (monocular depth + NeRF lite) |
| 6 | `Engine/Core/InteractiveThumbnailController.h` | Interactive thumbnail controller — touch/stylus/eye-gaze to rotate, zoom, and annotate |
| 7 | `Engine/Core/XRPlatformAdapter.h` | XR platform adapter — Windows MR / OpenXR extension points for thumbnail holographic overlay |
| 8 | `Engine/GPU/HolographicReprojectionEngine.h` | Automated holographic reprojection engine (asynchronous timewarp + space warp for MR) |

**Test additions:** +72 tests (4,434 → 4,506)

---

## Sprint 1101–1110 — Federated Learning & Privacy-Preserving AI (v32.3.0 "Fomalhaut-T")

**Theme:** ExplorerLens participates in federated learning ring to continuously improve its
thumbnail quality predictor, smart crop, and content categorization models — without any
user data ever leaving the device. Differential privacy guarantees (ε=2, δ=10⁻⁵) are
enforced at the gradient aggregation layer.

**Deliverables:**

| # | Header | Purpose |
|---|--------|---------|
| 1 | `Engine/AI/FederatedLearningCoordinator.h` | On-device federated learning coordinator — gradient aggregation, differential privacy injection |
| 2 | `Engine/AI/DifferentialPrivacyEngine.h` | Rényi differential privacy engine (Gaussian mechanism, RDP accounting, privacy budget) |
| 3 | `Engine/AI/GradientSecureAggregator.h` | Secure gradient aggregation (sum-checked secret sharing across simulated federation ring) |
| 4 | `Engine/AI/ModelUpdateValidator.h` | Model update validator — poisoning attack detector, Byzantine-robust aggregation |
| 5 | `Engine/Core/PrivacyPreservingLogger.h` | Privacy-preserving logger (k-anonymity + l-diversity before any telemetry export) |
| 6 | `Engine/AI/LocalModelFineTuner.h` | On-device model fine-tuner (LoRA adapters, learning rate schedule, convergence detector) |
| 7 | `Engine/AI/FederatedAuditVerifier.h` | Federated audit verifier — zero-knowledge proof that no raw data left the device ring |
| 8 | `Engine/Core/ConsentManagementEngine.h` | Consent management engine — granular user consent for each federated learning participation |

**Test additions:** +72 tests (4,506 → 4,578)

---

## Sprint 1111–1120 — Real-Time Collaboration & Multiplayer Preview (v32.4.0 "Fomalhaut-U")

**Theme:** Multiple users can co-browse a shared asset library with synchronized thumbnail
selection, real-time annotation, and presence indicators — all peer-to-peer over WebRTC
data channels without a central server. First-class integration with Microsoft Loop and
Figma for design-review workflows.

**Deliverables:**

| # | Header | Purpose |
|---|--------|---------|
| 1 | `Engine/Core/CollaborationSessionManager.h` | Peer-to-peer collaboration session manager (WebRTC data channel + STUN/TURN negotiation) |
| 2 | `Engine/Core/SharedCursorEngine.h` | Shared presence cursor engine — real-time multi-user cursor/focus overlay on thumbnail grid |
| 3 | `Engine/Core/ThumbnailAnnotationEngine.h` | Collaborative thumbnail annotation — ink/text/emoji overlaid and sync'd via CRDT |
| 4 | `Engine/Core/CRDTDocumentModel.h` | CRDT conflict-free replicated document model for annotation merging |
| 5 | `Engine/Core/CollaborativeFilterEngine.h` | Collaborative filter engine — shared tag bundles + ML-ranked cross-user sort order |
| 6 | `Engine/Core/PresenceAwarenessEngine.h` | Presence awareness engine — avatar, activity heatmap, and focus-zone broadcast |
| 7 | `Engine/Core/AssetReviewWorkflow.h` | Structured asset review workflow engine (approve/reject/revision requests via CRDT) |
| 8 | `Engine/Core/LoopFigmaIntegrationBridge.h` | Microsoft Loop + Figma integration bridge (share-link embed, approval handoff protocol) |

**Test additions:** +72 tests (4,578 → 4,650)

---

## Sprint 1121–1130 — Cross-Cloud Asset Sync & Hybrid Pipeline (v32.5.0 "Fomalhaut-V")

**Theme:** Unified thumbnail pipeline across local NVMe, OneDrive, SharePoint, Google Drive,
Dropbox, and S3-compatible storage — with content-addressed delta-sync so identical files
never re-decode. Cloud thumbnails are pre-generated server-side and streamed as AVIF with
per-tile progressive rendering.

**Deliverables:**

| # | Header | Purpose |
|---|--------|---------|
| 1 | `Engine/Core/CloudAssetSyncEngine.h` | Unified cloud asset sync engine — OneDrive/SharePoint/GDrive/Dropbox/S3 delta sync |
| 2 | `Engine/Core/ContentAddressedStore.h` | Content-addressed thumbnail store (BLAKE3 hash dedup + immutable object ID indexing) |
| 3 | `Engine/Core/HybridDecodeRouter.h` | Hybrid decode router — routes to local GPU, cloud pre-gen, or CDN based on cost model |
| 4 | `Engine/Core/CloudThumbnailStreamer.h` | Progressive AVIF tile streamer for cloud-generated thumbnails (HTTP/3 + QPACK) |
| 5 | `Engine/Core/DeltaSyncProtocol.h` | Binary delta-sync protocol for thumbnail cache entries (xxHash + rsync-rolling-hash) |
| 6 | `Engine/Cache/CloudCacheTierManager.h` | Cloud cache tier manager — hierarchical local → edge CDN → origin cloud eviction policy |
| 7 | `Engine/Core/OfflineAssetManager.h` | Offline asset manager — prefetch + pin popular files for zero-latency offline preview |
| 8 | `Engine/Core/MultiCloudCredentialVault.h` | PQC-encrypted multi-cloud credential vault (TPM-sealed, per provider, no plaintext at rest) |

**Test additions:** +72 tests (4,650 → 4,722)

---

## Sprint 1131–1140 — Developer SDK v5 & Plugin Intelligence (v32.6.0 "Fomalhaut-W")

**Theme:** SDK v5 unifies the C ABI, COM, and REST plugin interfaces under a single gRPC
contract. AI-assisted plugin authoring (GitHub Copilot for LENS SDK integration),
plugin health scoring, automated crash-bucket attribution, and one-click plugin signing
lower the barrier to entry for third-party developers.

**Deliverables:**

| # | Header | Purpose |
|---|--------|---------|
| 1 | `Engine/Plugin/SDKv5GrpcBridge.h` | gRPC-native SDK v5 bridge — unifies C ABI / COM / REST contracts under a single protobuf IDL |
| 2 | `Engine/Plugin/PluginAIAssistant.h` | AI-assisted plugin authoring — LLM code-completion for LENS SDK integration patterns |
| 3 | `Engine/Plugin/PluginHealthScorer.h` | Plugin health scorer — crash/ANR/memory metrics + automated regression attribution |
| 4 | `Engine/Plugin/PluginCrashBucketEngine.h` | Crash-bucket attribution engine — minidump analysis + stack-hash deduplication |
| 5 | `Engine/Plugin/OneClickSigningOrchestrator.h` | One-click plugin signing orchestrator — Azure Trusted Signing + EV cert auto-selection |
| 6 | `Engine/Plugin/PluginMigrationAdvisor.h` | SDK migration advisor — v3/v4 → v5 API migration plan generator (breaking change diff) |
| 7 | `Engine/Plugin/PluginTelemetryAggregator.h` | Plugin telemetry aggregator — privacy-preserving usage metrics with k-anonymity guarantee |
| 8 | `Engine/Plugin/PluginCapabilityMarketMap.h` | Plugin capability market map — visual dependency graph of installed plugins + gaps |

**Test additions:** +72 tests (4,722 → 4,794)

---

## Sprint 1141–1150 — Spatial Computing & Mixed Reality (v33.0.0 "Vega") ★★ MAJOR

**Theme:** ExplorerLens becomes the first file explorer thumbnail provider with native spatial
computing integration. Files in a shared OneDrive library appear as photorealistic holographic
thumbnails floating in the user's physical space — on Windows Mixed Reality, Apple Vision Pro
(visionOS Quick Look), and Meta Horizon OS (Quest). The spatial rendering pipeline targets
< 8 ms frame-to-photon latency (critical for MR comfort).

**Deliverables:**

| # | Header | Purpose |
|---|--------|---------|
| 1 | `Engine/Core/SpatialAnchorManager.h` | Spatial anchor manager — persists thumbnail 3D positions across MR sessions (OpenXR anchors) |
| 2 | `Engine/GPU/VisionPROQuickLookGenerator.h` | visionOS Quick Look generator — QLSpatialScene-based 3D thumbnail for Apple Vision Pro |
| 3 | `Engine/GPU/MetaHorizonOSRenderer.h` | Meta Horizon OS renderer — OVR SDK + Scene API thumbnail panel (Quest 3/4) |
| 4 | `Engine/Core/HandGestureInputAdapter.h` | Hand-gesture input adapter — pinch/grab/throw gestures mapped to thumbnail browse actions |
| 5 | `Engine/Core/SpatialFileOrganizer.h` | Spatial file organizer — group + arrange thumbnails in 3D space by AI-inferred semantic cluster |
| 6 | `Engine/Core/GazeSelectionEngine.h` | Eye-gaze selection engine (OpenXR eye tracking + foveal detail boost on hover) |
| 7 | `Engine/Core/MultisensoryPreviewEngine.h` | Multisensory preview engine — haptic pulse on thumbnail contact (XR haptics API) |
| 8 | `Engine/GPU/FrameTimingOptimizer.h` | Frame timing optimizer — ATW (asynchronous timewarping) + spacewarp for consistent 90fps MR |

**Test additions:** +72 tests (4,794 → 4,866)

**Acceptance Criteria:**
- Frame-to-photon latency < 8 ms on the holographic rendering path (bench: synthetic ATW mock)
- `SpatialAnchorManager` persists anchor positions across two simulated MR sessions (CI mock)
- `GazeSelectionEngine` selects the file at gaze origin within 3 frames at 90fps
- `MultisensoryPreviewEngine` dispatches correctly to Windows.Devices.Haptics and XR haptics API
- All spatial rendering components compile cleanly on Windows arm64 (for HoloLens 2)
- Zero regressions in the 2D flat-thumbnail path (MR is an additive layer)

---

## Sprint 1151–1160 — Apple Vision Pro + Meta Horizon Integration (v33.1.0 "Vega-R")

**Theme:** Production-ready Apple Vision Pro and Meta Horizon OS shell integrations, with
a cross-reality test harness validating pixel parity between all three XR platforms.
Ships with a spatial-native settings UI and a tutorial overlay for first-time spatial users.

**Deliverables:**

| # | Header | Purpose |
|---|--------|---------|
| 1 | `Engine/Core/VisionOSShellProvider.h` | visionOS native shell provider — QLThumbnailRequest + ornament layer for spatial browsing |
| 2 | `Engine/Core/HorizonOSFilePanel.h` | Meta Horizon OS file panel extension — OVR Spatial Panels for persistent thumbnail windows |
| 3 | `Engine/Core/XRSettingsUIEngine.h` | Spatial-native settings UI — SwiftUI + Android Compose spatial panel bindings |
| 4 | `Engine/Core/CrossRealityTestHarness.h` | Cross-reality test harness — pixel diff validator across WMR / visionOS / Horizon OS |
| 5 | `Engine/Core/SpatialOnboardingEngine.h` | Spatial onboarding engine — adaptive tutorial hologram for first-run XR users |
| 6 | `Engine/Core/MRComfortAnalyzer.h` | MR comfort analyzer — frame timing, IPD calibration, and locomotion sickness mitigation |
| 7 | `Engine/Core/SpatialNotificationEngine.h` | Spatial notification engine — non-intrusive holographic banners for background operations |
| 8 | `Engine/Core/XRAccessibilityAdapter.h` | XR accessibility adapter — voice control + switch access + high-contrast holographic mode |

**Test additions:** +72 tests (4,866 → 4,938)

---

## Cross-Cutting Concerns (All Sprints)

### Security Gate (Required for Every Release)
- Scrub all corporate/proxy artefacts before push: `git grep -rn "intel.com\|proxy\|928\b"`
- Post-quantum upgrade path: all new crypto primitives use NIST PQC finalists only
- Zero-trust: every new component that accepts external data must validate a capability token

### Performance Invariants
- P50 cache-warm: < 8 ms (maintained throughout all 1100-series sprints)
- NPU inference: < 15 ms for CLIP embedding after v32.1.0
- Spatial frame timing: < 8 ms frame-to-photon after v33.0.0

### Release Procedure
Every sprint bump MUST follow the procedure in `.github/copilot-instructions.md`:
1. Implement deliverables → run `.\build-scripts\Build-MSVC.ps1 -Test`
2. Update UnitTestCount in `BuildValidation.h` before bumping
3. Run `.\build-scripts\Bump-Version.ps1 -Version X.Y.Z -Codename C -TestCount N -ChangelogEntry "..." -TagAndPush`
4. The pushed tag fires `release.yml` and publishes all binaries to GitHub Releases
