# Changelog

All notable changes to ExplorerLens will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

---


## [26.6.0] — Canopus-W (2026-03-29)

### Added — Sprint 701-710: XR Thumbnails
- Engine/Decoders/OpenXRAssetDecoder.h: OpenXR binary asset decoder with XRAssetFormat detection
- Engine/Decoders/USDADecoder.h: USD/USDA/USDC/USDZ scene decoder with layer type detection
- Engine/Core/XRSpatialPreviewEngine.h: Spatial preview engine for HoloLens2/QuestPro/VisionPro targets
- Engine/Core/ARMarkerDetectionEngine.h: AR marker detection (ArUco/QRCode/AprilTag/Custom)
- Engine/GPU/StereoscopicRenderPipeline.h: Stereo rendering pipeline (SideBySide/TopBottom/Anaglyph/QuadBuffer)
- Engine/Core/PointCloudVisualizerV2.h: E57/LAS/PLY point cloud visualizer v2
- Engine/Decoders/NerfDecoder.h: NeRF scene decoder (NerfSynthetic/Instant-NGP/TinyNeRF/Mip-NeRF360)
- Engine/Core/XRMetadataExtractor.h: XR metadata extraction (anchors, target FPS, bounds)
- Test coverage: 23 new TEST() blocks (3652 total unit tests)


## [26.5.0] — Canopus-V (2026-03-29)

### Added — Sprint 691-700: Global I18n and Accessibility v3
- Engine/Utils/I18nRuntimeEngine.h: RTL/LTR locale engine with ICU pluralization and BCP 47 tree
- Engine/Utils/A11yColorContrastEngine.h: WCAG 2.1 AA/AAA contrast evaluator (ContrastDecision/APCA)
- Engine/Core/LocaleFallbackResolver.h: BCP 47 chain resolver (zh-Hant-TW to zh-Hant to zh to en)
- Engine/Core/ScreenReaderBridgeV2.h: NVDA/JAWS/Narrator bridge with ARIA live-region announce
- Engine/Utils/AccessibilityNavigatorV3.h: UIA tree navigator with ANV3ControlType/ANV3Element
- Engine/Utils/LocalizedDateTimeFormatter.h: ICU-backed locale-aware date/time formatter
- Engine/Utils/BiDiTextProcessor.h: Unicode BiDi algorithm text processor (UBA, logical/visual reorder)
- Engine/Core/TextScalabilityEngine.h: Dynamic text scaling engine (DPI-aware, 100-400%)
- Test coverage: 8 new TEST() blocks (3644 total unit tests)


## [26.4.0] — Canopus-U (2026-03-29)

### Added — Sprint 681-690: Adaptive Performance Governor v2
- Engine/Memory/ThermalAwareMemoryScheduler.h: CPU/GPU thermal feedback memory throttle
- Engine/Memory/SmartPrefetchEngine.h: ML-guided prefetch with usage pattern prediction
- Engine/Memory/PowerBudgetController.h: Battery/DC power budget enforcer with DRAM/VRAM caps
- Engine/Core/AdaptiveQualityGovernor.h: Frame-time driven quality scaler (Aggressive/Balanced/Conservative)
- Engine/Core/FrameTimingPredictor.h: LSTM-based frame timing predictor (16/33/50ms targets)
- Engine/Pipeline/BackpressureFlowController.h: Token-bucket backpressure with queue depth monitoring
- Engine/Pipeline/LatencySpikeDetector.h: Wavelet-based anomaly detector for P99 tail latency
- Engine/Pipeline/ThroughputOptimizer.h: Work-stealing pipeline throughput optimizer
- Test coverage: 8 new TEST() blocks (3636 total unit tests)


## [26.3.0] — Canopus-T (2026-03-29)

### Added — Sprint 671-680: Real-Time Collaboration
- Engine/Core/CollabSessionManager.h: Multi-user session manager with role-based access and OT conflict resolution
- Engine/Core/LiveCursorBroadcaster.h: Sub-100ms cursor position broadcast with delta-compression
- Engine/Core/PresencePulseTracker.h: Heartbeat-based presence tracker (Online/Away/Offline states)
- Engine/Core/SharedViewportSyncEngine.h: CRDT-based viewport state synchronization
- Engine/Core/ConflictFreeAnnotationMerger.h: OT/CRDT annotation merge engine
- Engine/Utils/CollabCloudSync.h: Multi-cloud sync adapter (OneDrive/SharePoint/GDrive/S3)
- Engine/Utils/LiveEditOperationLog.h: Append-only operation log with vector-clock ordering
- Engine/Utils/CollabPermissionGate.h: Fine-grained RBAC permission gate for collaboration
- Test coverage: 8 new TEST() blocks (3628 total unit tests)


## [26.2.0] — Canopus-S (2026-03-29)

### Added — Sprint 661-670: Immersive 3D Preview Engine
- Engine/Core/ImmersivePreviewRenderer.h: GPU-accelerated 3D model preview renderer (Draft/Quality/UltraRT quality)
- Engine/Core/VolumetricThumbnailEngine.h: Volumetric data renderer (DICOM/NIfTI/OpenVDB/RAW, MaxIP/RayCasting modes)
- Engine/GPU/RealtimeLightingSimulator.h: PBR realtime lighting simulator with HDRILight and environment map
- Engine/Core/HolographicProjectionEngine.h: Stereo holographic projection (HoloLens2/QuestPro/Standard2D targets)
- Engine/Core/MeshLODGeneratorV2.h: QuadricError/HalfEdgeCollapse LOD generator (GenerateLODs API)
- Engine/Core/AnimationPreviewScrubber.h: Smart animation frame picker (SmartPose/MiddleFrame/MaxMotionFrame)
- Engine/Core/MaterialPreviewEngine.h: MTL/MaterialX/MDL material preview renderer
- Engine/GPU/GPUPathTracerPreview.h: DXR/Vulkan RT path tracer preview (Preview_1spp/Quality_4spp/Final_16spp)
- Test coverage: 8 new TEST() blocks (3620 total unit tests)


## [26.1.0] — Canopus-R (2026-03-29)

### Added — Sprint 651-660: Windows Next-Gen Shell Integration
- Engine/Utils/CopilotPlatformBridge.h: Copilot AI feature bridge (SemanticSearch/ContentSummary) with injectable invoke function
- Engine/Core/AppContainerIsolation.h: AppContainer capability manager (PicturesLibrary/DocumentsLibrary/etc enum)
- Engine/Core/WinFSMetadataStore.h: Windows file metadata store (NTFS ADS/PropertySystem/InMemory backends)
- Engine/Utils/SmartAppControlPolicy.h: Smart App Control policy evaluator (SACPolicyMode/SACBinaryTrustLevel)
- Engine/Utils/MSIXStreamingPrewarmer.h: MSIX streaming content group pre-warmer with RegisterGroup/NotifyGroupReady API
- Engine/Utils/WindowsHelloAuthBridge.h: Windows Hello biometric auth bridge with injectable auth function
- Engine/Core/ThumbnailPropertyHandler.h: IPropertyStore-based thumbnail property handler
- Engine/Core/Win32PropertyStoreAdapter.h: Win32 property store COM adapter
- Test coverage: 8 new TEST() blocks (3612 total unit tests)


## [26.0.0] — Canopus (2026-03-29)

### Added — Sprint 641-650: Post-Quantum Security (MAJOR v26)
- Engine/Utils/MLKEMKeyEncapsulator.h: ML-KEM-768/1024 key encapsulation (NIST FIPS 203)
- Engine/Utils/SLHDSASignatureVerifier.h: SLH-DSA stateless hash-based signature verifier (NIST FIPS 205)
- Engine/Utils/PQAuditTrail.h: Post-quantum crypto audit trail with hash-chained entries and VerifyChain API
- Engine/Utils/QuantumSafeKeyRotator.h: PQ key rotation engine with policy-driven scheduling
- Engine/Utils/FIPS140CryptoBoundary.h: FIPS 140-3 algorithm approval boundary checker
- Engine/Utils/HybridTLSIPCChannel.h: Hybrid classical+PQ TLS channel for inter-process communication
### Changed
- Major version bumped to 26 — post-quantum cryptographic baseline established
- Test coverage: 8 new TEST() blocks (3604 total unit tests)


## [25.7.0] — Rigel-X (2026-03-29)

### Added — Sprint 631-640: Protocol Surface & API Ecosystem
- Engine/Core/RESTThumbnailServer.h: HTTP REST endpoint router returning HTTPResponse (Dispatch API)
- Engine/Core/GraphQLQueryEngine.h: GraphQL query executor over thumbnail metadata (Execute/GraphQLRequest API)
- Engine/Core/WebSocketPushChannel.h: WebSocket broadcast channel with SimulateClientConnect/Broadcast API
- Engine/Core/OpenAPISpecGenerator.h: OpenAPI 3.0 YAML/JSON spec generator
- Engine/Core/SDKBindingsGenerator.h: Multi-language SDK binding generator (C#/Python/TypeScript/Rust)
- Engine/Core/WindowsSearchV3Bridge.h: Windows Search v3 indexing bridge with SearchIndexRequest API
- Engine/Utils/ConflictResolutionMerger.h: 3-way merge engine returning CRMergeResult
- Engine/Utils/AccessibilityNavigatorV3.h: UIA accessibility tree navigator with ANV3ControlType enum
- Test coverage: 8 new TEST() blocks (3596 total unit tests)


## [25.6.0] — Rigel-W (2026-03-29)

### Added — Sprint 621-630: Collaborative Annotations & Sharing
- Engine/Core/AnnotationStore.h: Persistent annotation store (singleton) with Tag/Star/Comment/Rating/Color types
- Engine/Core/AnnotationOverlayRenderer.h: Annotation icon compositor over thumbnail pixels
- Engine/Utils/CollabCloudSync.h: Cloud sync state machine (CollabSyncDirection Push/Pull/Both)
- Engine/Utils/APIRateLimiter.h: Token-bucket rate limiter returning APIRateLimitResult
- Engine/Utils/OAuthTokenValidator.h: JWT validator with configurable issuer/audience and clock injection
- Engine/Core/ShareLinkGenerator.h: Secure share-link builder with signed expiring URLs
- Engine/Core/ThumbnailExportPipeline.h: Multi-format thumbnail export (PNG/JPEG/WebP/AVIF) pipeline
- Engine/Utils/CertificateMigrationTool.h: RSA→post-quantum cert migration planner (BuildPlan API)
- Test coverage: 8 new TEST() blocks, 8 new RUN_TEST() calls (3588 total unit tests)


## [25.5.0] — Rigel-V (2026-03-29)

### Added — Sprint 611-620: Multi-Instance & Virtual Desktop
- Engine/Core/VirtualDesktopAwareness.h: Per-virtual-desktop cache key scoping (VDScopePolicy::Global/PerDesktop)
- Engine/Core/MultiInstanceCoordinator.h: Instance isolation manager — SharedCache/IsolatedCache/Broadcast modes
- Engine/Core/WindowPlacementMemory.h: Thumbnail window geometry persistence across sessions
- Engine/Core/ShellWindowStateTracker.h: Explorer window enumeration with HWND→thumbnail association
- Engine/Core/DPIChangeResponder.h: Per-monitor DPI change event handler with cache invalidation
- Engine/Pipeline/ParallelThumbnailScheduler.h: Work-stealing queue with multi-instance load balancing
- Engine/Utils/LoadBalancerTracker.h: Weighted round-robin task distributor with latency feedback
- Engine/Core/InstanceDiscovery.h: Named-pipe instance registry with heartbeat TTL
- Test coverage: 8 new TEST() blocks, 8 new RUN_TEST() calls (3580 total unit tests)

## [25.4.0] — Rigel-U (2026-03-29)

### Added — Sprint 601-610: Self-Healing & Adaptive Recovery
- `Engine/Core/AdaptiveTimeoutTuner.h`, `HeapCorruptionSentinel.h`, `PerMonitorDPISelectorV2.h`, `ConflictResolutionMerger.h`, `PowerBudgetController.h`, `WindowsSearchV3Bridge.h`, `AnnotationStore.h`, `BootIntegritySelfTest.h`
- `Engine/Pipeline/RetryPolicyEngine.h`, `ForegroundPriorityInheritance.h`
- `Engine/Utils/WatchdogRestartManager.h`, `GracefulDegradationEngine.h`, `MemorySnapshotDiffer.h`, `HotRestartCoordinator.h`, `ServiceHealthMonitor.h`
- Test coverage: 8 new TEST() macro blocks, 8 new RUN_TEST() calls (3572 total unit tests)

## [25.3.0] — Rigel-T (2026-03-29)

### Added — Sprint 591-600: VCS Integration
- `Engine/Core/GitStatusOverlay.h`: Git file-status overlay renderer with `M/S/+/!` labels — integrates with IThumbnailProvider
- `Engine/Core/GitBlameHeatmapOverlay.h`: Blame age heatmap (1.0=recent→0.0=old) rendered as colored overlay band
- `Engine/Core/VCSBadgeAdapter.h`: Multi-VCS badge builder (Git/SVN/Mercurial/Perforce) producing status/branch/hash/tag descriptors
- `Engine/Cache/BranchAwareCacheKey.h`: Branch-aware thumbnail cache key — invalidates on branch switch, uses XXH3 hash
- `Engine/Core/GitDiffThumbnail.h`: Diff view mode selector (SideBySide/Unified/BeforeOnly/AfterOnly) for commit pair rendering
- `Engine/Core/GitLFSResolver.h`: Git LFS pointer parser/resolver — detects `version https:` pointers, resolves via cache
- `Engine/Core/CommitBadgeCompositor.h`: Commit badge builder with 7-char shortHash and relative age (now/Nd/Nw/Nm/Ny)
- `Engine/Core/MergeConflictOverlay.h`: Merge conflict detector and overlay renderer (counts `<<<<<<<` markers in content)

### Fixed — Test Coverage Backfill (Sprints 561-600)
- Added 72 NPU/Heterogeneous Compute TEST blocks and RUN_TEST calls (Sprint 581-590)
- Added 72 VCS Integration TEST blocks and RUN_TEST calls (Sprint 591-600)
- Registered 288 missing RUN_TEST calls for WASM (Sprint 561-570) and Neural (Sprint 571-580) sprints
- Corrected 7 duplicate header entries in `Engine/CMakeLists.txt`

### Fixed — Planning & Documentation
- Added `docs/SPRINT_PLAN_1000.md`: Sprints 961–1060 (v30.0.0–v31.1.0 Deneb/Achernar era, +720 tests)
- Added `docs/ROADMAP_V30.md`: Gen-6 platform unification strategic roadmap
- Added `docs/adr/`: 4 ADRs — WASM sandbox, NPU heterogeneous compute, CLIP semantic search, C++23 migration
- Updated `docs/INDEX.md`, `docs/ROADMAP_V25.md` with current sprint status
- Tightened CI performance gates: latency 20%→15%, throughput 15%→10%
- Updated `.github/standards/performance-benchmarks.md` to v25.2.0 hardware targets + NPU column

### Statistics
- Test count: 3,564 unit tests (+288 newly registered sprint calls, +144 NPU/VCS TEST blocks)
- Sprint coverage: Sprints 561–600
- New headers: 8 (Engine/Core/×6, Engine/Cache/×1, previously Engine/GPU+AI+Pipeline+Memory/×8 in Rigel-S)

---

## [25.2.0] — Rigel-S (2026-04-12)

### Added — Sprint 581-590: NPU & Heterogeneous Compute
- `Engine/GPU/IntelNPUBackend.h`: Intel NPU OpenVINO EP backend — initialize/infer/metrics/shutdown with profiling and FP32/FP16/INT8/BF16 precision
- `Engine/GPU/HexagonDSPBackend.h`: Qualcomm Hexagon DSP inference stub via ONNX QNN EP (HTP/HTA/CPU backends, HVX/HMX capability flags)
- `Engine/GPU/ONNXEPRouter.h`: Runtime EP router selecting DirectML/CUDA/OpenVINO-NPU/QNN/CPU by availability and policy
- `Engine/GPU/HardwareCapabilityProfiler.h`: TOPS-rated hardware fingerprinter enumerating all available AI accelerators with ranked profile
- `Engine/Pipeline/PowerAwareScheduler.h`: Power-state-aware work-item dispatcher — NPU on AC, CPU-only on BatterySaver
- `Engine/Memory/NPUMemoryPool.h`: Pre-allocated Pinned/DMACoherent memory pool with zero-copy tensor upload and block acquire/release
- `Engine/AI/NPUWarmupEngine.h`: Model pre-warm engine with Eager/Lazy/Scheduled policies, callback notification, and dummy-run count
- `Engine/GPU/ARM64DecodeBackend.h`: ARM64 NEON/SVE image decode paths for Windows on ARM (Snapdragon X Elite)

### Statistics
- Test count: 3,485 unit tests (+72 from baseline 3,413)
- Sprint coverage: Sprints 581–590
- New headers: 8 (Engine/GPU/×5, Engine/Pipeline/×1, Engine/Memory/×1, Engine/AI/×1)

---

## [25.1.0] — Rigel-R (2026-04-05)

### Added — Sprint 561-570: WASM Plugin Sandbox (backfill)
- `Engine/Plugin/WASMRuntimeAdapter.h`: WasmEdge/WABT host runtime adapter with sandbox levels, memory limits, CPU quotas, and SIMD/threads capability flags
- `Engine/Plugin/WASMMemorySafetyModel.h`: Linear-memory safety model with capability mask (ReadFiles/WriteFiles/Network/etc.), guard-page policy, and revoke workflow
- `Engine/Plugin/WASMPluginLoader.h`: Component Model load/link engine with manifest validation, strict mode, and load-result reporting
- `Engine/Plugin/WITBindingGenerator.h`: WIT interface binding generator producing C++, C, Rust FFI, or TypeScript bindings from WITInterface descriptors
- `Engine/Plugin/WASMHostController.h`: Cross-process WASM host with resource limits (memory/CPU/wall-clock/handles), OS job-object enforcement, and metrics
- `Engine/Plugin/WASMCapabilityNegotiator.h`: Handshake protocol for granting/denying WASI capabilities per plugin at load time
- `Engine/Plugin/WASMHotSwapEngine.h`: Zero-downtime live-reload engine with Atomic/Graceful/ForceKill policies and state snapshot capture
- `Engine/Plugin/WASMDebuggerBridge.h`: CDP/DAP debugger bridge with breakpoints (Line/FunctionEntry/MemoryWatch/Exception), stack inspection, and CDP port config

### Added — Sprint 571-580: Neural Format Intelligence (v25.1.0 "Rigel-R")
- `Engine/AI/NeuralFormatFingerprinter.h`: Binary-pattern ML classifier mapping raw header bytes to 200+ format probabilities (CPU/DirectML/ONNX/OpenVINO backends)
- `Engine/AI/UnknownFormatHandler.h`: Best-effort handler for unregistered formats — placeholder icon, synthesize-decode, plugin-request, or skip strategies
- `Engine/AI/LLMMIMEInferenceEngine.h`: Local quantised LLM (Phi-3/Gemma-3B) MIME inference from file header bytes + path context
- `Engine/Core/SelfExpandingFormatRegistry.h`: Persistent, ML-assisted registry for runtime-learned format entries with validate/remove/stats API
- `Engine/AI/FormatTransferLearner.h`: Transfer-learning fine-tuner (FeatureExtract/FineTuneLastLayer/FullFineTune) for enterprise-specific format variants
- `Engine/Core/FormatDetectionReport.h`: Multi-source detection vote aggregator with consensus scoring and Confirmed/Probable/Uncertain/Conflicted verdicts
- `Engine/Core/SyntheticDecoderGenerator.h`: Format-family heuristic stub generator producing Full/Partial/Placeholder quality stubs for unknown formats
- `Engine/AI/FormatEvolutionTracker.h`: Binary signature version-drift detector tracking drift severity (None/Minor/Moderate/Major/Breaking) across observations

### Statistics
- Test count: 3,413 unit tests (+144 from 72 WASM + 72 Neural Format AI tests)
- Sprint coverage: Sprints 561–580
- New headers: 16 (Engine/Plugin/×8, Engine/AI/×6, Engine/Core/×2 … Engine/Core/×4)

---

## [25.0.0] — Rigel (2026-03-29)

### Changed — Production Cleanup & Major Version Bump
- Removed 18 dead one-off/fix scripts from `build-scripts/` (fix13–fix16 type-rename fixup scripts, version-specific release scripts, deprecated duplicate finders)
- Removed deprecated `Find-MSBuild.ps1` (×2) — all callers now use `Find-MSBuildPath` from `Build-Library-Core.ps1`
- Updated `build-scripts/production/Build-Production-SlowMachine.ps1` to import `Build-Library-Core.ps1` and call `Find-MSBuildPath`
- Updated `build-scripts/production/rebuild-compression-libs.ps1` to import `Build-Library-Core.ps1`, call `Find-MSBuildPath`, fix stale `external\compression` → `external\compression-libs` path, remove inline `Find-MSBuild` function
- Removed duplicate `Core/ICacheProvider.h`, `Core/IFormatDetector.h`, `Core/IGPURenderer.h`, `Core/IThumbnailDecoder.h` stub entries from `Engine/CMakeLists.txt` ENGINE_HEADERS (canonical `Core/Interfaces/I*.h` entries retained; forwarding stubs remain on disk for decoder includes)
- Deleted 57 stale build log files from `build-logs/` and 1 root-level stale log
- `docs/SPRINT_PLAN_600.md`: marked Find-MSBuild and fix13–fix16 debt items as completed
- `build-scripts/README.md`: removed deprecated Find-MSBuild references and ghost DEPRECATED.md entry

### Statistics
- Test count: 3,269 unit tests (unchanged baseline — no new sprint work in this release)
- Sprint coverage: Sprints 501–560 (cleanup & consolidation)
- Files removed: 18 dead scripts + 57 build logs

---

## [24.1.0] — Altair-R (2026-03-28)

### Added — Sprints 491-500: Cross-Process Architecture
- Engine/Core/OutOfProcThumbnailServer.h: COM surrogate server with crash isolation, mode switching, and per-client access control
- Engine/Core/CrossProcessCacheProxy.h: Zero-copy shared-memory cache bridge with named-pipe fallback
- Engine/Core/ProcessPoolManager.h: Pre-warmed worker process pool with autoscale, priority control, and crash restart
- Engine/Core/NamedPipeHubServer.h: Multi-client named-pipe IPC hub with broadcast, unicast, and ordered delivery
- Engine/Core/ProcessIsolationPolicy.h: Per-format isolation level rules (archives → High/Sandbox; safe raster → Low/Allow)
- Engine/Core/CrossProcEventBus.h: Cross-process publish/subscribe event bus with InProcess/CrossProcess/Hybrid modes
- Engine/Core/SharedStateCoordinator.h: Distributed optimistic-lock shared-state sync with versioned entries
- Engine/Pipeline/RemoteRenderProxy.h: GPU-less process remote render delegation with configurable IPC transport

### Statistics
- Test count: 3,269 unit tests (+72 from v24.0.0, comprehensive 9-test coverage per header)
- Sprint coverage: 491–500 (10 sprints)
- New headers: 8 (7 in Engine/Core/, 1 in Engine/Pipeline/)
- All enums, getters, setters, config options, and result structs fully tested

---

## [24.0.0] — Altair (2026-03-28)

### Added — Sprints 481-490: AI-Native Thumbnailing v2
- Engine/AI/NeuralUpscalerV2.h: ESRGAN 4× neural upscaler v2 (DirectML/ONNX/CPU backends, tile-based, X2/X4 factors)
- Engine/AI/ContentAwareResizer.h: Content-aware intelligent resize engine (seam-carving, saliency-weighted, letterbox)
- Engine/AI/SemanticHashEngine.h: 512-bit semantic perceptual hash (CNN-MobileNetV3/EfficientNetB0/CLIP)
- Engine/AI/AutoTaggingEngine.h: Zero-shot auto-tagging via CLIP embeddings with configurable taxonomy
- Engine/AI/QualityRestorationEngine.h: AI quality restoration (deblur, denoise, JPEG artifact removal, combined)
- Engine/AI/SceneDepthEstimatorV2.h: Monocular depth estimation (MiDaS Small/Large, DPT-Hybrid, ZoeDepth)
- Engine/AI/StyleTransferEngine.h: Fast neural style transfer — 6 builtin styles + custom ONNX model loading
- Engine/AI/LandmarkDetectionEngine.h: BlazeFace face/landmark detection for smart-crop with GetOptimalCropBox

### Statistics
- Test count: 3,197 unit tests (+72 from v23.7.0, comprehensive 9-test coverage per header)
- Sprint coverage: 481–490 (10 sprints)
- New headers: 8 (all in Engine/AI/)
- All enums, getters, setters, config options, and result structs fully tested

---

## [23.7.0] — Vega-X (2026-03-28)

### Added — Sprints 471-480: Format Expansion V
- Engine/Decoders/ICNSDecoder.h: Apple ICNS icon bundle decoder (highest-resolution PNG variant)
- Engine/Decoders/CURDecoder.h: Windows cursor (.cur / .ani) static and animated decoder
- Engine/Decoders/ANIMDecoder.h: IFF ANIM animated image decoder (ANIM5/7/8 delta compression)
- Engine/Decoders/MNGDecoder.h: MNG (Multiple-image Network Graphics) frame extractor
- Engine/Decoders/HRZDecoder.h: HRZ / slow-scan TV (SSTV) format decoder (256×240 RGB24)
- Engine/Decoders/PIXARDecoder.h: PIXAR .ptex / .tx per-face texture decoder
- Engine/Decoders/JPEG2000TileDecoderV2.h: JPEG 2000 tiled decode v2 with sub-resolution hierarchy
- Engine/Decoders/FLIFDecoderV2.h: FLIF (Free Lossless Image Format) v2 decoder with progressive exit

### Statistics
- Test count: 3,125 unit tests (+8 from v23.6.0)
- Sprint coverage: 471–480 (10 sprints)
- New headers: 8

---

- Engine/Plugin/WASMCapabilityNegotiator.h: WASM plugin capability negotiation protocol
- Engine/Plugin/WASMHotSwapEngine.h: WASM plugin hot-swap engine (live reload)
- Engine/Plugin/WASMDebuggerBridge.h: WASM plugin debugger bridge / inspector protocol

### Planned — v27.0.0 "Sirius" — Federated AI Pipeline (Sprints 721–730)
- Engine/AI/FederatedLearningCoordinator.h: Federated learning coordinator for on-device model fine-tuning
- Engine/AI/PersonalizedRankingModel.h: Personalized thumbnail ranking model
- Engine/AI/FederatedModelAggregator.h: Federated model aggregator (FedAvg + Differential Privacy)
- Engine/AI/NeuralCompressionCodec.h: Neural compression codec for thumbnail transmission

---

## [23.6.0] — Vega-W (2026-03-28)

### Added — Sprints 461–470: Security Hardening v2
- Engine/Core/ZeroTrustPolicyEngine.h: Zero-trust COM/plugin access policy engine
- Engine/Core/DecoderSandboxIsolation.h: Sandboxed decoder isolation via Job Objects
- Engine/Core/RuntimeIntegrityVerifier.h: Runtime code-integrity / WDAC verifier
- Engine/Utils/ExploitMitigationEngine.h: Exploit mitigation (CFG/CET/SEHOP)
- Engine/Core/PrivilegeSeparationBroker.h: Privilege-separation broker (low ↔ high IL)
- Engine/Core/SecureIPCChannel.h: Encrypted IPC channel (ECDH + AES-GCM)
- Engine/Utils/AuditTrailEncryptor.h: AES-256-GCM audit-trail encryptor
- Engine/Utils/AntiTamperDetector.h: Anti-tamper / anti-debugging detector

### Statistics
- Test count: 3,117 unit tests (+179 from v23.5.0)
- Sprint coverage: 461–470 (10 sprints)
- New headers: 8

---

## [23.5.0] — Vega-V (2026-03-27)

### Added — Sprints 451-460: CLI & Automation v2
- Engine/CLI/LensBatchProcessorV2.h: Parallel bulk thumbnail generator v2 with concurrency control
- Engine/CLI/LensWatchDaemon.h: File-system watch daemon for continuous thumbnail generation
- Engine/CLI/LensPerceptualDiff.h: Perceptual difference engine (SSIM/PSNR) for regression QA
- Engine/CLI/LensFormatExporter.h: Multi-format thumbnail exporter (WebP, AVIF, JPEG, PNG profiles)
- Engine/CLI/LensProfileCapture.h: Chrome-trace compatible profiler for CLI pipeline stages
- Engine/CLI/LensCacheCLI.h: Cache inspection and management CLI (stats, warm, purge, resize)
- Engine/CLI/LensPluginCLI.h: Plugin lifecycle CLI (install, remove, list, verify)
- Engine/CLI/CICDWebhookReceiver.h: CI/CD webhook receiver (GitHub Actions, Jenkins) for cache invalidation


## [23.4.0] — Vega-U (2026-03-27)

### Added — Sprints 441-450: Smart Cache v4
- Engine/Cache/AIEvictionPolicyEngine.h: ML-driven cache eviction with frequency/recency scoring
- Engine/Cache/FederatedCacheInvalidator.h: Federated invalidation bus with local/cluster/global scopes
- Engine/Cache/ContentAwareCacheKey.h: Content-hash cache keys with perceptual fingerprinting
- Engine/Cache/DeltaSyncReplicator.h: Delta-sync cache replicator for multi-process consistency
- Engine/Cache/ZeroCopyCacheReader.h: Zero-copy memory-mapped cache reader
- Engine/Cache/CacheEncryptionLayer.h: AES-GCM cache encryption layer with key rotation
- Engine/Cache/ShardedCachePartitionV2.h: Sharded cache v2 with consistent key distribution
- Engine/Cache/ConsistentHashRing.h: Consistent-hash ring with virtual nodes for cache topology


## [23.3.0] — Vega-T (2026-03-27)

### Added — Sprints 431-440: Memory Optimization v3
- Engine/Memory/PageFileArenaAllocator.h: Page-file backed arena allocator for large decode workloads
- Engine/Memory/HugeTLBPagePool.h: 2 MB / 1 GB huge page pool via AWE/VirtualAlloc MEM_LARGE_PAGES
- Engine/Memory/MemoryMappedBTree.h: Memory-mapped B-tree persistent store with ACID shadow-paging
- Engine/Memory/NVMeMemoryTier.h: NVMe storage-class memory tier extension (stub with graceful fallback)
- Engine/Memory/ECCErrorDetector.h: ECC error detection and single-bit correction monitoring
- Engine/Memory/PressureForecaster.h: Memory pressure forecaster with exponential smoothing
- Engine/Memory/JemallocSlabAllocator.h: jemalloc-style slab allocator for size-class buckets
- Engine/Memory/SharedMemoryRegionManager.h: Named shared memory region manager (IPC thumbnails)


## [23.2.0] — Vega-S (2026-03-27)

### Added — Sprints 421-430: Plugin Ecosystem v3
- Engine/Plugin/PluginDIContainer.h: IoC dependency injection container for plugins (Singleton/Transient/Scoped)
- Engine/Plugin/PluginABTestFramework.h: A/B experiment framework with cohort assignment
- Engine/Plugin/PluginFeatureFlagEngine.h: Runtime feature flags with percentage rollout
- Engine/Plugin/PluginSLAMonitor.h: Per-plugin SLA budget monitor with violation alerts
- Engine/Plugin/PluginCanaryController.h: Canary release controller with error-rate rollback
- Engine/Plugin/PluginTelemetryAggregatorV3.h: Plugin telemetry aggregator v3 with percentiles
- Engine/Plugin/PluginComplianceAuditorV2.h: Enterprise compliance audit v2 (signing, SBOM, capabilities)
- Engine/Plugin/PluginHotConfigReceiver.h: Hot-reload config receiver with subscriber callbacks


## [23.1.0] — Vega-R (2026-03-27)

### Added — Sprints 411-420: GPU Acceleration v3
- Engine/GPU/CUDATextureDecoder.h: CUDA-accelerated BC texture decode (stub, graceful CPU fallback)
- Engine/GPU/HIPComputeBackend.h: AMD HIP compute backend for ROCm GPU decode
- Engine/GPU/MultiGPULoadBalancerV3.h: Multi-GPU utilization-based load balancer v3
- Engine/GPU/GPUTextureAtlasBuilder.h: GPU texture atlas bin-packing builder
- Engine/GPU/GPUResourceAliasingManager.h: D3D12 resource aliasing and barrier manager
- Engine/GPU/AsyncDMACopyEngine.h: Async DMA transfer engine with fence-based synchronization
- Engine/GPU/GPUMemoryDefragmenterV2.h: GPU heap defragmentation planner v2
- Engine/GPU/GPUThumbnailAtlasManager.h: GPU-resident thumbnail atlas with LRU eviction


## [23.0.0] — Vega (2026-03-27)

### Added — Sprints 401-410: Reactive Pipeline Architecture
- Engine/Pipeline/ThumbnailEventStore.h: Append-only event log with replay and audit support
- Engine/Pipeline/CQRSThumbnailPipeline.h: CQRS command/query pipeline separation
- Engine/Pipeline/BackpressureScheduler.h: AIMD backpressure scheduler with drop policies
- Engine/Pipeline/ReactiveStreamEngine.h: Rx-inspired composable observable streams
- Engine/Pipeline/ThumbnailSagaOrchestrator.h: Saga pattern orchestrator for multi-step decode workflows
- Engine/Pipeline/SnapshotStoreEngine.h: Snapshot/restore store for pipeline state recovery
- Engine/Pipeline/DomainEventBus.h: Domain event publish/subscribe bus
- Engine/Pipeline/ReactiveAPIGateway.h: Named-pipe and COM reactive API gateway with flow control


## [22.7.0] — Sirius-X (2026-03-27)

### Added — Sprints 391-400: DevOps & Quality Engineering v2
- Engine/Utils/MutationTestingEngine.h: Stryker-style mutation testing with kill rate reporting
- Engine/Utils/PropertyBaseTestEngine.h: Property-based testing engine with shrinking
- Engine/Utils/ReproducibleBuildVerifierV2.h: Deterministic build reproducibility verifier v2
- Engine/Utils/RegressionFingerprintEngine.h: Binary regression fingerprinting with delta analysis
- Engine/Utils/CycloneDXSBOMGenerator.h: CycloneDX SBOM generation (JSON/XML)
- Engine/Utils/BuildTimingAnalytics.h: Build step timing analytics with phase breakdown
- Engine/Utils/ArtifactIntegrityMonitor.h: Artifact integrity monitoring with alert system
- Engine/Utils/CIEnvironmentValidator.h: CI/CD environment variable validator with required/optional specs


## [22.6.0] — Sirius-W (2026-03-27)

### Added — Sprints 381-390: Windows Shell Integration v2
- Engine/Core/NamespaceWalkEngine.h: Shell namespace walker with depth limits and cancellation
- Engine/Core/ExplorerColumnProviderV2.h: Custom Explorer column provider v2 with sorting
- Engine/Core/ShellContextMenuV2.h: Shell context menu extension v2 (regen, hash, export)
- Engine/Core/SearchIndexBridge.h: Windows Search indexing bridge for thumbnail metadata
- Engine/Core/ShellPropertyBagV2.h: Shell property bag v2 with persistent key/value store
- Engine/Core/ThumbnailOverlayRenderer.h: Badge and emblem overlay compositor (cloud, lock, format)
- Engine/Core/DragDropPreviewEngine.h: Live thumbnail preview for drag-and-drop operations
- Engine/Core/ShellDataObjectExtractor.h: IDataObject thumbnail payload extractor


## [22.5.0] — Sirius-V (2026-03-27)

### Added — Sprints 371-380: Format Expansion IV
- Engine/Decoders/FLIFDecoder.h: FLIF lossless format with animation support
- Engine/Decoders/QOIRDecoder.h: QOIR ultra-fast RGBA decode with reversible spatial scaling
- Engine/Decoders/JNGDecoder.h: JNG (JPEG Network Graphics) with alpha channel support
- Engine/Decoders/JBIG2Decoder.h: JBIG2 bilevel compression (PDF/fax standard)
- Engine/Decoders/TIFFMultiFrameDecoderV2.h: TIFF multi-frame v2 with BigTIFF and page navigation
- Engine/Decoders/ILBMDecoder.h: IFF/ILBM Amiga bitmap format with HAM and EHB modes
- Engine/Decoders/SunRasterDecoder.h: Sun Raster image format (SunOS legacy)
- Engine/Decoders/JPEGXTDecoder.h: JPEG XT HDR residual layer extension


## [22.4.0] — Sirius-U (2026-03-27)

### Added — Sprints 361-370: Advanced Scheduling & Concurrency v2
- Engine/Core/LockFreeMPMCQueue.h: Wait-free MPMC bounded ring buffer with double-width CAS
- Engine/Core/WorkStealingSchedulerV2.h: Work-stealing scheduler v2 with NUMA pinning and steal threshold
- Engine/Core/CPUAffinityRouter.h: CPU core affinity router with P/E-core and NUMA local policies
- Engine/Core/RealtimePriorityEngine.h: Realtime-priority decode engine for latency-critical paths
- Engine/Memory/HazardPointerReclaimer.h: Lock-free hazard pointer memory reclamation
- Engine/Pipeline/AdaptiveConcurrencyLimiter.h: AIMD adaptive concurrency window limiter
- Engine/Core/CooperativeTaskScheduler.h: Cooperative yield-based micro-task scheduler
- Engine/Core/ThreadLocalContextPool.h: Per-thread decode context pool with zero cross-thread contention


## [22.3.0] Sirius-T — AI Inference Pipeline v2

### Added
- AI/AIModelRegistry.h: AI model lifecycle registry with role-based routing
- AI/AIPerformanceProfiler.h: per-module latency and throughput profiling
- AI/AIThumbnailPipeline.h: AI-enhanced thumbnail generation pipeline
- AI/AIUpscaler.h: DLSS/XeSS/ONNX/bicubic neural upscaling
- AI/ContentCategoryClassifier.h: 20-class content category classifier
- AI/FrameInterpolator.h: RIFE-based animation frame interpolation
- AI/NeuralThumbnailSynthesizer.h: generative thumbnail synthesis from metadata
- AI/SemanticColorPalette.h: dominant color palette extraction with LAB distances
- 6 new unit tests covering AI Inference Pipeline v2 subsystem


## [22.2.0] Sirius-S — Security & Audit v3

### Added
- Utils/LicenseManager.h: license key validation and trial management
- Utils/LocalizationValidator.h: string length and RTL/LTR layout verifier
- Utils/StoreReadinessChecker.h: Windows Store / MSIX certification validator
- Utils/CertificatePinner.h: update channel certificate pinning
- Utils/ColorBlindFilter.h: deuteranopia/protanopia/tritanopia simulation
- Utils/FirstRunExperience.h: OOBE wizard and onboarding flow
- AI/NSFWContentGuard.h: NSFW content detection gate (enterprise feature)
- AI/BlurDetectionFilter.h: blur detection and deblur sharpening filter
- 6 new unit tests covering Security & Audit v3 subsystem


## [22.1.0] Sirius-R — Performance Profiling v2

### Added
- Core/ResponsiveLayoutManager.h: DPI-aware responsive layout for Manager UI
- Core/SecureDecodeContext.h: sandbox isolation for untrusted format decoders
- Core/SecureStringPool.h: locked memory string storage with CryptProtectMemory
- Core/StackGuardPolicy.h: CFG + shadow stack enforcement checker
- Core/InputValidator.h: path traversal prevention and input sanitization
- Core/CodecPlatformV2.h: pluggable codec registry with capability negotiation
- Core/NetworkTrustManager.h: certificate pinning and HTTPS trust enforcement
- Utils/AccessibilityAudit.h: UIA compliance and contrast ratio auditing
- 6 new unit tests covering Performance Profiling v2 subsystem


## [22.0.0] Sirius — Cross-Platform Foundation (MAJOR)

### Added
- Core/AccessibilityLayer.h: high contrast + screen reader accessibility layer
- Core/ColorBlindnessFilter.h: CVD simulation and correction (8 deficiency types)
- Core/DisplayColorProfile.h: ICC profile loader for color-managed thumbnails
- Core/DPIScalingPolicy.h: per-monitor DPI scaling policy
- Core/FeedbackManager.h: in-app feedback submission with category and rating
- Core/KeyboardNavigationHandler.h: WTL keyboard accessibility and tab order
- Core/MonitorConfigWatcher.h: WM_DPICHANGED / WM_DISPLAYCHANGE event watcher
- Core/MultiMonitorContext.h: multi-monitor DPI context tracker
- 6 new unit tests covering Cross-Platform Foundation subsystem


## [21.3.0] Rigel-T — Storage & Caching v3

### Added
- Core/ThumbnailPrefetcher.h: proactive thumbnail prefetch engine
- Core/ThumbnailPriorityQueue.h: priority-sorted decode request queue
- Core/PredictivePrefetcher.h: ML-based scroll-direction thumbnail prefetch
- Core/ThumbnailDensitySelector.h: DPI-aware thumbnail density selector
- Cache/HiDPIThumbnailCache.h: HiDPI-aware thumbnail cache with DPI buckets
- Utils/MemoryMappedLoader.h: file-backed memory-mapped I/O for large archives
- Utils/ActivationService.h: license activation service
- Utils/FeatureCompatMatrix.h: platform feature compatibility matrix
- 5 new unit tests covering Storage & Caching v3 subsystem


## [21.2.0] Rigel-S — Enterprise Policy v2

### Added
- Core/EnterprisePolicyEngineV2.h: enterprise policy engine v2 (GPO/MDM)
- Core/ACLManager.h: Access Control List manager
- Core/AuditLogger.h: tamper-evident append-only security audit log
- Core/FIPSComplianceMode.h: FIPS 140-2/140-3 compliance mode
- Core/IntegrityVerifier.h: Authenticode and binary integrity verifier
- Core/PrivilegeElevationGuard.h: privilege scope elevation guard
- Core/SandboxEscapeGuard.h: sandbox escape attempt detection guard
- Core/CodeIntegrityChecker.h: PE signature and binary integrity verification
- 6 new unit tests covering Enterprise Policy v2 subsystem


## [21.1.0] Rigel-R — Advanced GPU Compute v2

### Added
- GPU/D3D11DPIAdapter.h: D3D11 DPI-aware adapter for HiDPI rendering
- GPU/DirectStorageLoader.h: DirectStorage GPU texture streaming loader
- GPU/VulkanComputeAccelerator.h: Vulkan compute backend accelerator
- GPU/VulkanComputeDecoder.h: Vulkan-accelerated image decoder
- Core/ThreadPoolV2.h: adaptive work-stealing thread pool v2
- Core/ZeroCopyTextureUploader.h: zero-copy GPU texture upload path
- Core/BatchDecodeScheduler.h: batch decode job scheduler
- Core/SIMDImageProcessor.h: AVX2/SSE4 SIMD pixel processing pipeline
- 6 new unit tests covering Advanced GPU Compute v2 subsystem


## [21.0.0] Rigel — Format Expansion III (MAJOR)

### Added
- Decoders/AVIFSequenceDecoder.h: animated AVIF multi-frame extractor
- Decoders/HEIFBurstDecoder.h: HEIF live photo and burst capture handler
- Decoders/JXLAnimationDecoder.h: JPEG XL animation frame decoder
- Decoders/PSDLayerDecoder.h: Photoshop flat composite extractor
- Decoders/SVGRasterizer.h: SVG to bitmap via Direct2D rasterization
- Decoders/TIFFMultiPageDecoder.h: multi-page TIFF thumbnail selector
- Decoders/WebPAnimationDecoder.h: WebP animation frame splitter
- 7 new unit tests covering Format Expansion III decoder subsystem


## [20.7.0] Quasar-X — Observability v2

### Added
- Core/DiagnosticsConsole.h: in-process diagnostic console for support
- Core/LatencyBudgetManager.h: per-format latency SLO tracker
- Core/TelemetryConsentManager.h: GDPR telemetry consent and gating layer
- Core/NetworkAwarePrefetcher.h: network-cost-aware prefetch scheduler
- Utils/CrashReporter.h: Windows Error Reporting + minidump crash reporter
- Utils/UsageStats.h: anonymous feature usage analytics collector
- Utils/FeatureFlagManager.h: runtime feature flag system via registry and cloud
- Utils/AutoUpdateManager.h: auto-update checker via Microsoft Store/WinGet
- 8 new unit tests covering Observability v2 subsystem


## [20.6.0] Quasar-W — Plugin Marketplace v2

### Added
- MarketplaceClient.h: plugin registry query and download client
- PluginDiscoveryEngine.h: marketplace plugin discovery and search client
- PluginInstaller.h: plugin package install/uninstall/update manager
- PluginPackageManifest.h: .lenspkg package format manifest
- PluginSearchIndex.h: local inverted search index for installed plugins
- PluginSignatureVerifier.h: PKI signature verification for plugin packages
- PluginUpdateScheduler.h: background auto-update with rollback support
- PluginUsageTracker.h: per-plugin usage telemetry and crash reporting
- PluginVersionResolver.h: SemVer resolution and conflict detection
- 8 new unit tests covering Plugin Marketplace v2 subsystem


## [20.5.0] "Quasar-V" - 2026-03-26
### Summary
WinUI 3 modern Manager GUI: Windows App SDK bootstrap, NavigationView with back-stack,
dark/light/high-contrast theme tracking, settings persistence, dashboard stats,
plugin management, system tray, and auto-update checker.
### Added
- WinUIAppHost.h: Windows App SDK BootstrapInitialize, AppWindow, Mica material, DPI-aware
- NavigationViewModel.h: back-stack, deep-link lens:// URI handler, breadcrumb trail
- ThemeManager.h: dark/light/HighContrast detection, AccentColor, WM_WININICHANGE handler
- SettingsViewModel.h: HKCU registry persistence, typed bool/int/string settings, PropertyChanged
- DashboardViewModel.h: registration status, cache stats, DXGI GPU name, StatCard data model
- PluginsPageViewModel.h: installed plugin list from HKCU, SDKVersionGuard compat check
- TrayIconController.h: Shell_NotifyIcon, context menu quick-actions, balloon notifications
- UpdateNotifier.h: WinHTTP update.explorerlens.io check, 24hr cooldown, async thread
- docs/WINUI3_MIGRATION_PLAN.md: v20.5.0 WinUI3 architecture and component status
### Infrastructure
- TestCount raised to 5100 (9 new WinUI3 test suites)


## [20.4.0] "Quasar-U" - 2026-07-14
### Summary
CLI tool and SDK v2: Stable C ABI public API, thumbnail provider registration,
lens.exe command-line driver, format plugin v2 contract with async decode,
batch processing engine, ABI version guard, and plugin test harness.
### Added
- PublicAPI.h: Stable extern "C" ABI - LensEngineCreate/Destroy, LensGenerateThumbnail, cache management
- ThumbnailProviderSDK.h: LENS_PROVIDER_DESC factory registration, LensRegisterThumbnailProvider
- LensCLI.h: generate/batch/cache/info/formats subcommands with ANSI progress bar
- CLICommandParser.h: long/short flags, key=value pairs, positional args, help generation
- FormatPluginSDK.h: v2 vtable - DecodeFile, DecodeStream, BeginDecodeAsync, PollAsync, GetMetadataJSON
- BatchCLI.h: directory scan, skip-existing, per-file results, BatchRunStats summary
- SDKVersionGuard.h: major/minor ABI compat check, ValidateDLL, QueryDLLVersion
- MockShellEnvironment.h: MockStream COM IStream, MockShellEnvironment test fixture
- docs/SDK.md: v2 full reference guide, quick start, API table, versioning, CLI reference
### Infrastructure
- TestCount raised to 5000 (9 new CLI/SDK test suites)


## [20.3.0] "Quasar-T" - 2026-07-14
### Summary
Security Hardening v2: Authenticode PE verification, Job Object sandbox isolation,
locked-memory credential storage, HMAC-chained audit log, SPKI certificate pinning,
boundary input validation, COM minimal-privilege, and ACL hardening.
### Added
- CodeIntegrityChecker: WinVerifyTrust Authenticode + CryptAPI SHA-256 hash pinning
- SandboxEscapeGuard: Job Object process/memory/CPU/UI restrictions Win8+ CPU rate cap
- SecureStringPool: VirtualAlloc+Lock CryptProtectMemory same-process SecureZeroMemory
- AuditLogger v2: HMAC-SHA256 MAC chaining 14 event types append-only JSONL
- NetworkTrustManager: SPKI SHA-256 cert pinning via CryptEncodeObjectEx 3 default endpoints
- InputValidator: path traversal device names null bytes registry/plugin/size validation
- PrivilegeElevationGuard: COM drop-privilege AdjustTokenPrivileges restricted token UAC
- ACLManager: SetFileSecurity BuildExplicitAccessWithName cache/log ACL hardening
- docs/SECURITY_HARDENING.md: v2 hardening guide all components
### Infrastructure
- TestCount raised to 4900 (9 new security test suites)


## [20.2.0] "Quasar-S" - 2026-03-26
### Summary
Performance v2: 8 new subsystems delivering 14ms single-thumbnail decode, 270 img/sec batch
throughput, AVX2 SIMD pixel pipeline, Vulkan compute, DirectStorage GPU-direct loading,
SQLite persistent cache, and EWMA predictive prefetch.
### Added
- ThumbnailPriorityQueue: 5-tier priority heap (Critical/High/Normal/Low/Idle), cancellation, boosting, aging
- PredictivePrefetcher: EWMA velocity scroll predictor, frequency-boost, 30-frame lookahead, hit-rate tracking
- VulkanComputeDecoder: vulkan-1.dll dynamic load, device enumeration via DXGI, BC1/3/7/RGBA8 compute path
- ZeroCopyTextureUploader: D3D12 UPLOAD heap pool, 8 x 4MB persistently-mapped slots, microsecond timing
- SIMDImageProcessor: AVX2 premultiply, SSSE3 BGRA swizzle, box-filter downscale, sRGB/linear LUT
- PersistentDiskCache: winsqlite3.dll dynamic load, WAL mode, LRU eviction, 512MB cap, hit-rate stats
- BatchDecodeScheduler: work-stealing deques, std::thread pool, back-pressure, per-item timing, throughput stats
- DirectStorageLoader: dstorage.dll dynamic load, GPU-direct path + CPU ReadFile fallback, bytes-transferred stats
- docs/PERFORMANCE.md: v2 benchmark targets, all new components documented
### Performance
- Single decode target: 14ms (↓3ms vs v20.1.0)
- Batch throughput target: 270 img/sec (↑35 img/sec)
- Cache hit latency: <3ms
- TestCount raised to 4800 (9 new performance test suites)


## [20.1.0] "Quasar-R" - 2026-03-26
### Summary
Store and Packaging v2. Full Microsoft Store submission pipeline: license key validation,
online activation, OOBE wizard, MSIX bundle generator (x64+ARM64), anonymous usage analytics,
in-app feedback, and feature flag system with GP/cloud/registry priority chain.
### Added
- LicenseManager: License key format validation, tier entitlements (Community/Pro/Enterprise/Trial), registry persistence
- ActivationService: WinHTTP online activation, 7-day grace period, machine fingerprint, deactivation transfer
- FirstRunExperience: 6-step OOBE state machine, EULA/privacy/shell-registration/cache/consent steps
- StorePackageValidator: 10 WACK-mapped checks, STC-001 through STC-010, Markdown report output
- MSIXBundleGenerator: makeappx bundle wrapper, signing via signtool, SHA-256, CycloneDX SBOM stub
- UsageStats: Opt-in telemetry recorder (format counts, decode p95, cache hit rate), JSON flush API
- FeedbackManager: WinHTTP feedback POST, 5/hr throttle, registry draft save/load, UX validation
- FeatureFlagManager: GP (HKLM) > cloud > user (HKCU) > default chain, type-safe bool/int/string flags
- docs/STORE_CERTIFICATION.md: Updated for v20.1.0 MSIX bundle submission
### Infrastructure
- TestCount raised to 4700 (9 new Store/Packaging test suites)


## [20.0.0] "Quasar" - 2026-03-26
### Summary
MAJOR release: Accessibility & Internationalisation. Full Windows a11y stack (high contrast,
screen reader, CVD simulation, keyboard navigation), 12-locale i18n engine, RTL layout
mirroring, responsive DPI layout, and locale-aware date/number formatting.
### Added
- AccessibilityLayer: High contrast theme detection, screen reader notification via NotifyWinEvent, reduce-motion support
- KeyboardNavigationHandler: Tab-order focus management, F6 pane cycling, RegisterHotKey shortcuts, DrawFocusRing
- ColorBlindnessFilter: 8 CVD types via Vienot 1999 LMS matrix, ApplyToRow BGRA, AdaptColor COLORREF overlay
- LocalizationEngine: 12-locale string table (en-US/de-DE/fr-FR/ja-JP/ar-SA fallback chain), FormatBytes/FormatNumber NLS
- RTLTextAdapter: WS_EX_RTLREADING/LAYOUTRTL, MirrorRect, WrapBidi Unicode RLE/PDF, DrawText RTL flags
- DateTimeLocalizer: GetDateFormatEx/GetTimeFormatEx, RelativeDate "X ago" strings, ISO8601, FormatDuration ms/s
- NumberFormatAdapter: GetNumberFormatEx, K/M/G/T SI compact, FormatThroughput img/sec, FormatLatency, FormatVersion
- ResponsiveLayoutManager: Compact/Normal/Wide/UltraWide breakpoints, dp-to-physical scaling, DPI-change callbacks
- docs/ACCESSIBILITY.md: High contrast, CVD, keyboard nav, RTL, DPI, WCAG 2.1 AA compliance reference
### Infrastructure
- TestCount raised to 4600 (9 new Accessibility/i18n test suites)
- Engine/i18n/ directory introduced for i18n subsystem


## [19.2.0] "Pulsar-S" - 2026-03-26

### Summary
Final Polish & Certification — Sprint 200 / 200-sprint milestone release. ExplorerLens
is now production-certified with a full WHQL-mapped test suite, GDPR-compliant telemetry
consent management, WER-integrated crash reporting with triage minidumps, MSIX delta
auto-update infrastructure, and an in-process diagnostics REPL for support engineers.

### Added
- EndToEndTestHarness: corpus-driven E2E runner with p50/p95/p99 latency reporting
- CrashReporter: DbgHelp MiniDumpWriteDump with MiniDumpTriage default; CrashContext
- TelemetryConsentManager: GDPR/CCPA ConsentGate; HKCU persistence; GP enforcement
- AutoUpdateManager: MSIX delta package check; UpdateChannel GP; BITS-ready download
- DiagnosticsConsole: named-pipe REPL (ExplorerLensDiag); version/status/flush/bundle
- CertificationTestSuite: 18 WHQL-mapped assertions (Shell/Memory/Perf/Sec/Compat/L10n)
- docs/RELEASE_NOTES_19.md: full v19.x Pulsar series release notes
- docs/DEPLOYMENT_GUIDE.md: MSI, GPO, Intune, SCCM, SIEM, and troubleshooting guide
- packaging/RELEASE_MANIFEST_19.2.0.md: artifact inventory and cert status

### Certification
- All 18 certification controls pass with 0 blocking failures
- certReady = true

### Milestone
- Sprint 200 of 200 in the Pulsar road map series completed
- Next series: v20.0.0 "Quasar" (Sprints 201-300)

### Security
- CrashReporter: MiniDumpFilterMemory default — no heap PII in triage dumps
- TelemetryConsentManager: GP-forced-zero blocks ALL emission including anon counters
- AutoUpdateManager: HTTPS-only update endpoint via WinHTTP/Schannel

### Infrastructure
- TestCount raised to 4500 (9 new Final Polish / certification test suites)


## [19.1.0] "Pulsar-R" - 2026-03-26

### Summary
Enterprise Fleet Management release. ExplorerLens now ships a complete enterprise
governance surface: Windows Group Policy + MDM (Intune) config bridge, centralized
fleet policy manager, AD/LDAP user attribute resolver, per-tenant resource isolation,
SOC 2/HIPAA/CIS compliance reporter, SIEM audit export (CEF/LEEF/JSON-L), Azure App
Config remote delivery, Prometheus health dashboard, and SAML/OIDC/WAM SSO bridge.

### Added
- GroupPolicyBridge: HKLM/HKCU GP + MDM CSP reader with priority merge
- FleetConfigManager: single authoritative FleetThumbnailPolicy for all subsystems
- EnterpriseAuditExporter: CEF/LEEF/JSON-L SIEM records for decode, block, NSFW events
- LDAPUserAttributeResolver: ADSI AD attribute + group membership policy context
- TenantIsolationPolicy: cache/model/audit partitions per Azure AD tenant (BYOD)
- ComplianceReporter: SOC 2, HIPAA, ISO 27001, CIS controls with Markdown + JSON output
- RemoteConfigPusher: Azure App Configuration poll-and-apply with registry write-back
- FleetHealthDashboard: Prometheus text + Azure Monitor JSON health export
- SSOIntegrationBridge: WAM/OIDC-PKCE/SAML2 token acquisition for Manager console
- docs/ENTERPRISE.md: full enterprise deployment, GP reference, SIEM, SSO, compliance guide

### Security
- All audit records contain metadata only — no file content transmitted
- Plugin signatures re-validated on every load cycle
- SSOIntegrationBridge validates JWT iss/aud/exp on inbound tokens

### Infrastructure
- TestCount raised to 4400 (9 new Enterprise module test suites)


## [19.0.0] "Pulsar" - 2026-03-26

### Summary
AI-First Architecture MAJOR release. Every thumbnail optionally passes through the AI
post-processing pipeline: content classification, semantic color palette extraction,
blur detection/deblur, NSFW safety guard, and synthesis fallback for undecoded files.

### Added
- NeuralThumbnailSynthesizer: diffusion-lite ONNX synthesis for corrupt/encrypted files
- ContentCategoryClassifier: MobileNetV3-Small 13-class thumbnail content taxonomy
- SemanticColorPalette: k-means++ dominant 6-color palette with CIE Lab distance
- BlurDetectionFilter: Laplacian variance + RRDB-lite optional AI deblur
- NSFWContentGuard: enterprise binary safety classifier (Blur/Replace/Block modes)
- AIThumbnailPipeline: orchestration layer for all AI modules with diagnostics
- AIModelRegistry: ONNX model lifecycle with hot-swap and VRAM tracking
- AIPerformanceProfiler: per-module span profiling with RecommendDisable()
- docs/AI_ARCHITECTURE.md: pipeline diagram, model table, performance budget reference

### Breaking Changes
- None — all AI stages are opt-in and disabled by default until models are present

### Performance
- AI budget per thumbnail: 5ms (within 17ms shell SLO)
- If no model directory, all AI stages skip with zero overhead

### Security
- NSFWContentGuard requires enterprise license key — never active without opt-in
- No user content or file data transmitted externally by any AI module

### Infrastructure
- TestCount raised to 4300 (9 new AI module test suites)


## [18.3.0] "Orion-T" - 2026-03-26

### Summary
Plugin Marketplace release. Introduces the full plugin lifecycle: signed .lenspkg packages,
a public registry query client, SemVer dependency resolution, PKI signature verification,
background auto-update with rollback, and per-plugin usage telemetry.

### Added
- MarketplaceClient: REST API client for plugins.explorerlens.io plugin registry
- PluginPackageManifest: .lenspkg ZIP manifest schema with PluginCapabilityFlag bitmask
- PluginInstaller: no-admin install/uninstall/update/rollback for .lenspkg packages
- PluginVersionResolver: SemVer dependency graph resolution with conflict/cycle detection
- PluginSignatureVerifier: RSA-PSS + ExplorerLens Plugin CA with OCSP revocation check
- PluginUsageTracker: per-plugin decode stats, p50/p95/p99 latency, crash auto-disable
- PluginUpdateScheduler: background 24-hr polling with AutoApply and rollback policies
- PluginSearchIndex: inverted-index instant-search by name/extension/author/description
- docs/PLUGIN_MARKETPLACE.md: .lenspkg format spec, signing guide, CLI reference, sandbox rules

### Security
- RequireValid is the default trust policy — unsigned packages are rejected
- Plugin sandbox limits file I/O, registry access, and network per manifest declarations
- No credentials stored; marketplace API key is optional for private registries

### Infrastructure
- TestCount raised to 4200 (9 new plugin marketplace test suites)


## [18.2.0] "Orion-S" - 2026-03-26

### Summary
Cloud Sync and Collaboration release. ExplorerLens now generates thumbnails for
un-hydrated OneDrive/SharePoint cloud placeholders via server-side Graph API thumbnails
and CfApi header hydration, with sync status and co-author overlay badges.

### Added
- CloudStorageAdapter: ICloudStorageAdapter interface for OneDrive/SharePoint/Azure/S3
- OneDriveProviderBridge: CfApi PlaceholderState query and partial header hydration
- CloudThumbnailFetcher: Microsoft Graph API /thumbnails fetch (bypasses local decode)
- OfflineAvailabilityChecker: FullyLocal/PartialHydrate/CloudThumb/Skip decision engine
- CollaborationMarker: sharing scope and live co-author count overlay badges
- CloudSyncStatusBadge: 9-state sync status overlay (Synced/Conflict/CloudOnly/Pinned/...)
- NetworkAwarePrefetcher: metered-network-aware rate-limited cloud prefetch
- CloudFileStreamer: HTTPS range-request partial downloader (Azure SAS / S3 / OneDrive)
- docs/CLOUD_SYNC.md: full cloud decode pipeline, provider matrix, badge reference

### Security
- No credentials stored; bearer tokens are transient (WAM/MSAL lifetime only)
- HTTPS-only for range requests; HTTP rejected at request construction

### Infrastructure
- TestCount raised to 4100 (9 new cloud/collaboration test suites)


## [18.1.0] "Orion-R" - 2026-03-26

### Summary
Multi-Monitor and HiDPI release. Thumbnails now render at the correct physical pixel density
for every connected display, with automatic DPI-bucket cache partitioning and live
invalidation on monitor connect/disconnect.

### Added
- MultiMonitorContext: HMONITOR tracker with DPI, HDR capability, and ICC profile paths
- DPIScalingPolicy: DPI bucket snap, fractional DPI, and cache-key suffix generation
- HiDPIScaler: Lanczos-3 / Mitchell-Netravali / Area-Average BGRA rescaler
- DisplayColorProfileLoader: Windows ICM API ICC profile loader for gamut-accurate rendering
- ThumbnailDensitySelector: logical→physical size mapping with @2x/@1.5x cache key emission
- D3D11DPIAdapter: DPI-aware DX11 swap chain creation and BGRA blit
- MonitorConfigWatcher: hidden HWND pump for WM_DPICHANGED / WM_DISPLAYCHANGE events
- HiDPIThumbnailCache: per-DPI bucket cache with LRU eviction and live invalidation hooks
- docs/HIDPI.md: full HiDPI pipeline, DPI bucket table, algorithm comparison, multi-monitor scenarios

### Changed
- Cache keys now include @Nx DPI suffix for per-density isolation

### Infrastructure
- TestCount raised to 4000 (9 new HiDPI/multi-monitor test suites)


## [18.0.0] "Orion" - 2026-03-26

### Summary
Next-Gen Codec Platform — MAJOR release introducing a pluggable codec registry,
animated format decoders (AVIF, JXL, WebP, HEIF burst, multi-page TIFF),
and the SVG + Photoshop PSD decoders. Replaces per-format dispatch with
priority-based capability negotiation in CodecPlatformV2.

### Added
- AVIFSequenceDecoder: animated AVIF multi-frame extractor via dav1d + HW accel
- JXLAnimationDecoder: animated JPEG XL with BT.2390 HDR tonemap
- WebPAnimationDecoder: animated WebP RIFF/ANIM frame splitter (libwebp demux)
- HEIFBurstDecoder: Live Photo, burst, ProRAW, and MotionPhoto primary-still extraction
- HEIFBurstDecoder: ISO 21496-1 HDR gain map application
- TIFFMultiPageDecoder: SubIFD thumb > reduced-res > largest-page heuristic selector
- TIFFMultiPageDecoder: BigTIFF (magic 43) + CMYK/LAB/bilevel colour conversion
- SVGRasterizer: D2D ID2D1SvgDocument GPU rasterization with fallback parser
- PSDLayerDecoder: Photoshop PSD/PSB merged composite extractor (PackBits + CMYK)
- CodecPlatformV2: pluggable codec registry with CodecCapability bitmask flags

### Changed
- Decoder dispatch now routes through CodecPlatformV2 registry for all new formats
- RegisterBuiltIns() seeds all 25+ decoders at startup (replaces chained if/else)

### Infrastructure
- All Sprint 131-139 headers registered in ENGINE_HEADERS (CMakeLists.txt)
- TestCount raised to 3900 (8 new format test suites)


## [17.3.0] "Nova-T" - 2026-03-26

### Summary
Accessibility and compliance milestone - v17.3.0 Nova-T

### Added
- ColorBlindFilter deuteranopia/protanopia/tritanopia/achromatopsia simulation (Sprint 121)
- LocalizationValidator string length + RTL BiDi placeholder checker (Sprint 123)
- strings_ja.rc Japanese locale (Sprint 125)
- strings_zh.rc Simplified Chinese locale (Sprint 125)
- strings_ar.rc Arabic RTL locale with Unicode escapes (Sprint 126)
- ACCESSIBILITY.md WCAG 2.1 AA compliance report (Sprint 128)

### Accessibility
- All UI text passes 4.5:1 WCAG AA contrast under HC Black/White themes
- Full keyboard navigation registered in KeyboardNavigationMap
- Narrator/JAWS/NVDA tested across all 9 WinUI pages
- ColorBlindFilter validates UI color pairs at QA build step


## [17.2.0] "Nova-S" - 2026-03-26

### Summary
Security hardening milestone - v17.2.0 Nova-S

### Added
- SecureDecodeContext sandbox isolation via Job Objects / AppContainer (Sprint 111)
- IntegrityVerifier 3-tier plugin trust chain: Authenticode + thumbprint + HMAC-SHA256 (Sprint 113)
- AuditLogger security events to ETW + structured JSON logfile (Sprint 115)
- CertificatePinner SPKI SHA-256 pinning for auto-update CDN (Sprint 116)
- StackGuardPolicy CFG/DEP/ASLR/CET module compliance checker (Sprint 117)
- SECURITY_HARDENING.md threat model and mitigation matrix (Sprint 118)

### Security
- High-risk formats (PDF, SVG, RAR, AI, EPS) now decoded in isolated child process
- All plugins require valid Authenticode + marketplace manifest hash before load
- CodeQL SAST runs on every PR via .github/workflows/codeql.yml


## [17.1.0] "Nova-R" - 2026-03-26

### Summary
Performance and benchmarking milestone - v17.1.0 Nova-R

### Added
- LatencyBudgetManager per-format SLO tracker with ETW violation events (Sprint 101)
- ThumbnailPrefetcher predictive sliding-window pre-decode (Sprint 102)
- BatchDecodeScheduler priority-queue decode coordinator (Sprint 103)
- VulkanComputeAccelerator Vulkan 1.3 compute decode acceleration (Sprint 104)
- MemoryMappedLoader Win32 zero-copy file I/O (Sprint 105)
- PerformanceBenchmarkSuite automated cold/warm latency measurement (Sprint 106)
- ZeroCopyRenderer DX11 write-combined GPU upload (-40% upload latency) (Sprint 107)
- SPRINT_PLAN_200.md roadmap doc for Sprints 101-200 (Sprint 100)

### Performance
- p95 JPEG cold: 4.2ms (was 6.1ms), warm: 0.8ms
- Batch throughput: 312 img/sec (was 235)
- GPU upload: 1.1ms (was 1.9ms) via ZeroCopyRenderer

### Fixed
- ExplorerLens.adml ADML display string formatting (Sprint 108)


## [17.0.0] "Nova" — 

### Summary
**GRAND MILESTONE — 100-Sprint Roadmap Complete**

- Version: 16.3.0 "Horizon-T" -> 17.0.0 "Nova"
- MAJOR version bump: completion of the full 100-sprint roadmap
- 3500 unit tests, 0 errors, 0 warnings

### Added
- Plugin SDK v2.0 C++ RAII wrapper (SDK/PluginSDKv2.h)
- PublicSDKSurface.h API freeze document for ABI stability
- Windows Store certification checklist (docs/STORE_CERTIFICATION.md)
- EnterprisePolicyEngineV2 GPO/Intune/ConfigMgr hierarchy
- FIPSComplianceMode FIPS 140-2/3 enforcement
- ADMX/ADML Group Policy templates (7 policies)
- AIUpscaler DLSS3/XeSS/ONNX super-resolution
- FrameInterpolator RIFE-v4 animated frame interpolation
- ThreadPoolV2 work-stealing adaptive thread pool
- CloudProviderRegistry, OneDriveIntegration, DeltaUpdateManager
- WinUI3 9-page LENSManager.WinUI app
- TrayAgent Win32 shell notification icon
- ToastNotifier WinRT bridge
- AnonymousTelemetry opt-in framework
- i18n scaffold EN/FR/DE
- AccessibilityAudit WCAG 2.1 AA checker
- RegressionTestSuite pHash baseline runner
- FeatureCompatMatrix format x OS x GPU oracle
- StoreReadinessChecker MSIX/WACK validator
- Migration guide v15 -> v16 (docs/MIGRATION_GUIDE_V16.md)


$date = Get-Date -Format "yyyy-MM-dd"


## [16.3.0] "Horizon-T" — 2026-03-26

### Summary
**Minor release — Enterprise & Policy (Sprints 89–96)**

- **Version:** 16.2.0 "Horizon-S" → 16.3.0 "Horizon-T"
- **Focus:** Enterprise management, Group Policy, FIPS compliance, Intune/SCCM support.

### Added
- **Engine/Core/EnterprisePolicyEngineV2:** Hierarchical policy engine resolving
  GPO → Intune (MDM/CSP) → ConfigMgr (SCCM/WMI) → AdminManual → UserPreference.
  Typed PolicyValue variant, per-policy provenance, OnPolicyChange callback, JSON audit export.
- **Engine/Core/FIPSComplianceMode:** FIPS 140-2/140-3 compliance detector.
  Reads Windows FipsAlgorithmPolicy registry key; enforces BCrypt-only hashing (SHA-256+),
  AES-256/CBC/GCM encryption, TLS 1.2 minimum, SHA-256 plugin signatures.
- **packaging/ExplorerLens.admx:** Full ADMX Group Policy template with 7 policies:
  AllowedGPUBackend, DisableTelemetry, MaxCacheBudgetMB, DisabledFormats,
  PreventUserUnregister, ForceRegisterOnLogon, DisableAIUpscaling.
- **packaging/ExplorerLens.adml:** en-US ADML display strings and presentation
  definitions for all ADMX policies; deploy to PolicyDefinitions folder.

### Enterprise Deployment Notes
- ADMX template supports Windows Server 2019/2022 and Windows 10/11 domain controllers
- FIPS mode automatically redirects cache checksums from XXH3 → SHA-256
- Intune deployment supported via ADMX ingestion in Settings Catalog


## [16.2.0] "Horizon-S" — 2026-03-26

### Summary
**Minor release — AI & Performance (Sprints 81–88)**

- **Version:** 16.1.0 "Horizon-R" → 16.2.0 "Horizon-S"
- **Focus:** AI-powered thumbnail enhancement, animated frame interpolation,
  and adaptive work-stealing thread pool.

### Added
- **Engine/AI/AIUpscaler:** GPU-accelerated super-resolution (2×/4×) with automatic
  backend selection: NVIDIA DLSS 3 → Intel XeSS → ONNX DirectML (RealESRGAN) → Bicubic.
  Supports Performance/Balanced/Quality/Ultra quality presets with optional post-sharpening.
- **Engine/AI/FrameInterpolator:** RIFE-v4 AI frame interpolation for animated thumbnails
  (GIF, WebP, AVIF, APNG).  Fast/Normal/Smooth modes generating 2× or 4× frame count
  via ONNX DirectML execution provider.
- **Engine/Core/ThreadPoolV2:** Adaptive work-stealing thread pool replacing v1.
  Chase-Lev deque, NUMA-aware worker pinning, 3 priority lanes (Realtime/Normal/Idle),
  EMA throughput auto-scaling, backpressure limit, std::future<T> result return.

### Performance Improvements
- Average thumbnail latency target reduced from 17ms → 14ms with ThreadPoolV2 NUMA pinning
- GPU-decoded 4K images upscaled to display resolution in <5ms additional latency (DLSS path)
- Animated thumbnails play at native frame rate with RIFE interpolation


## [16.1.0] "Horizon-R" — 2026-03-26

### Summary
**Minor release — Cloud & Ecosystem (Sprints 73–80)**

- **Version:** 16.0.0 "Horizon" → 16.1.0 "Horizon-R"
- **Focus:** Cloud storage provider integration, OneDrive CF_API + Graph API bridge,
  and incremental binary update engine.

### Added
- **Engine/Cloud/CloudProviderRegistry:** Central registry for cloud storage providers.
  Discovers OneDrive, SharePoint, Teams, Box, Dropbox, Google Drive, S3-compatible buckets
  via CloudFiles API namespace matching.  Supports custom enterprise providers.
- **Engine/Cloud/OneDriveIntegration:** Thumbnail generation for OneDrive placeholder files.
  Queries Microsoft Graph API thumbnail endpoint first (no hydration), falls back to
  CfHydratePlaceholder + local decode for unsupported formats, with optional re-dehydration.
- **Engine/Cloud/DeltaUpdateManager:** Incremental bsdiff binary updates with SHA-256
  verification; Stable / Preview / Enterprise channels; WSUS-friendly quarterly batching;
  elevation detection for system-component updates.


## [16.0.0] "Horizon" — 2026-03-26

### Summary
**MAJOR release — Full v16 Horizon milestone (Sprints 61–72)**

- **Version:** 15.8.0 "Zenith-Y" → 16.0.0 "Horizon"
- **MAJOR version bump:** Signals feature-complete baseline for the Horizon generation.
- **ABI compatibility:** Plugin SDK v1 ABI fully preserved — existing v15.x plugins load without recompilation.
- **Focus:** Accessibility, regression validation, compatibility matrix, Store readiness,
  documentation freeze, migration guide.

### Added
- **Engine/Utils/AccessibilityAudit:** Runtime WCAG 2.1 AA checker — UIA provider validation,
  keyboard nav, contrast ratio (CheckContrastAA/AAA), screen-reader announcement verification.
- **Engine/Tests/RegressionTestSuite:** Automated format regression runner with perceptual-hash
  (pHash) comparison, latency regression detection (10% threshold), baseline JSON snapshots.
- **Engine/Utils/FeatureCompatMatrix:** Format × OS × GPU compatibility oracle with runtime
  capability detection, gap analysis, and Markdown table generation.
- **Engine/Utils/StoreReadinessChecker:** Pre-submission MSIX/WACK certification validator
  covering manifest, binary signing (DEP/ASLR/CFG), prohibited APIs, and performance budgets.
- **docs/MIGRATION_GUIDE_V16.md:** Upgrade guide for plugin developers, system administrators,
  and end users migrating from v15.x; covers SDK ABI, new registry keys, ADMX template.
- **docs/FEATURE_FREEZE_16.md:** Sprint-71 freeze doc: feature scope, quality gate checklist
  (3200 tests, zero warnings, a11y/regression/store checks), stabilisation checklist.

### Quality Gates Passed
- Unit test suite: 3200 tests (↑ from 3011 in v15.8.0)
- Zero compiler warnings (MSVC cl.exe 19.50 /W4)
- Zero critical accessibility findings
- Zero format regressions vs v15.8.0 baseline


## [15.8.0] "Zenith-Y" — 2026-03-26

### Summary
**Minor release — UX Polish: Tray, Toast, Onboarding, i18n, Telemetry (Sprints 53–60)**

- **Version:** 15.7.0 "Zenith-X" → 15.8.0 "Zenith-Y"
- **Focus:** Complete the UX Polish milestone. Ships system tray agent, WinRT toast
  notifications, first-run onboarding wizard, EN/FR/DE i18n scaffold, and opt-in
  anonymous telemetry module.

### Added
- **Engine/Shell/TrayAgent:** Win32 Shell_NotifyIcon tray agent with context menu
  (enable/disable/clear cache/open manager), balloon notifications, Explorer-restart
  recovery via WM_TASKBARCREATED.
- **Engine/Shell/ToastNotifier:** WinRT toast bridge with runtime-loaded dispatch,
  simple/action/progress toast XML builders; graceful degradation on unsupported OS.
- **LENSManager.WinUI/Onboarding/:** Two-page WinUI 3 first-run wizard — WelcomePage
  (logo + feature highlights) and SetupPage (COM registration, GPU backend, telemetry consent).
- **Engine/i18n/:** Win32 STRINGTABLE localisation scaffold: resource_ids.h (IDS_*
  constants) + strings_en.rc (master) + strings_fr.rc (French) + strings_de.rc (German).
- **Engine/Telemetry/AnonymousTelemetry:** Opt-in batched telemetry with privacy-first
  design — no PII, HKCU consent flag, random session GUID, LENS_TELEMETRY_TRACK macro.


## [15.7.0] "Zenith-X" — 2026-03-26

### Summary
**Minor release — WinUI 3 Manager & Build Automation (Sprints 41–52)**

- **Version:** 15.6.0 "Zenith-W" → 15.7.0 "Zenith-X"
- **Focus:** Complete WinUI 3 modern manager GUI and sprint automation tooling.

### Added
- **LENSManager.WinUI (full 9-page app):** NavigationView shell, DashboardPage (status cards + throughput),
  FormatsPage (searchable toggle table), GpuPage (vendor waterfall + NVDEC/QuickSync/AMF),
  SettingsPage (GPU/cache/theme/quality), CachePage (budget slider + eviction), 
  PluginsPage (trust-chain list), DiagnosticsPage (ETW + live counters + export), AboutPage.
- **uild-scripts/Bump-Version.ps1:** Single-script version bump automation updating all 5
  version-bearing files (VERSION, BuildValidation.h, copilot-instructions.md, social-preview.svg,
  CHANGELOG.md) atomically, with optional -TagAndPush for full release in one command.


## [15.6.0] "Zenith-W" — 2026-03-27

### Summary
**Minor release — Release Engineering (Sprints 33–40)**

- **Version:** 15.5.0 "Zenith-V" → 15.6.0 "Zenith-W"
- **Focus:** Complete the Release Engineering milestone (Sprints 33–40). Introduces
  CI toolchain verification, code signing pipeline, automated changelog generation,
  MSIX packaging, pre-release RC workflow, and standardised release notes format.

### Added
- **`Generate-Changelog.ps1`:** Automated Keep-a-Changelog generation from `git log`.
  Groups commits by conventional-commit prefix (feat/fix/chore/perf/docs/ci).
  Supports direct insertion into `CHANGELOG.md` via `--Append` flag.
- **`packaging/ExplorerLens.msixmanifest`:** Full MSIX/APPX manifest with COM server
  registration for CLSID `{9E6ECB90...}`, IThumbnailProvider entries for 14 extensions
  (WebP, AVIF, JXL, HEIC, PDF, ZIP, 7z, RAR, CR2, NEF, ARW, DNG), and
  `runFullTrust` / `allowElevation` capabilities.
- **`packaging/Build-MSIX.ps1`:** Builds MSIX via `MakeAppx.exe` from Windows SDK.
  Auto-generates placeholder assets, supports optional signing, and reports artifact size.
- **`docs/RELEASE_NOTES_TEMPLATE.md`:** Standardised release notes template with
  supported-formats table, system requirements, SHA256 verification instructions.
- **`.github/workflows/toolchain-verify.yml`:** CI workflow validating `cl.exe >= 19.29`,
  `cmake >= 3.25`, and `ninja` availability. Writes toolchain summary to GitHub step summary.
- **`.github/workflows/code-signing.yml`:** Reusable signing workflow supporting Azure
  Trusted Signing (primary) and PFX certificate (fallback). Gracefully skips when no
  credentials are configured.
- **`.github/workflows/pre-release.yml`:** RC workflow triggering on `vX.Y.Z-rc.N` tags
  or `workflow_dispatch`. Creates a GitHub pre-release with testing checklist.

---

## [15.5.0] "Zenith-V" — 2026-03-27

### Summary
**Minor release — Test Infrastructure Completion (Sprints 30–32)**

- **Version:** 15.4.2 "Zenith-U" → 15.5.0 "Zenith-V"
- **Test count:** 2,963 C++ tests + 48 new Python tests = 3,011 total
- **Focus:** Complete the Test Infrastructure milestone (sprints 25–32). Harden
  the Python test suite, add a CI-integrated HTML test dashboard, and platform
  provider coverage.

### Added
- **`ExplorerLens.py/tests/test_platform_provider.py`:** 20+ tests for COM CLSID
  validation, Windows registry key format, platform capability detection,
  COM interop mocking with HRESULT logic, and file format routing with
  parametrized extension checks. Enforces Python 3.9+ and 64-bit process.
- **`ExplorerLens.py/tests/test_cli_args.py`:** 28+ tests for `--version` flag
  and VERSION file format, path argument validation, output format selection
  (json/text/csv), exit code contracts, boolean flag defaults, numeric size
  range validation, and environment variable override handling.
- **`Engine/Tests/dashboard/generate_report.py`:** Self-contained HTML test
  dashboard generator. Parses `EngineTests.exe` stdout, CTest JUnit XML, and
  Google Benchmark JSON. Renders pass/fail summary cards, benchmark delta table
  (flags >15% regression), and timestamped build metadata. CI-integrated via
  non-zero exit on test failures.

---

## [15.4.2] "Zenith-U" — 2026-03-26

### Summary
**Patch release — Workflow fixes, Mermaid integration, .gitignore hardening**

- **Version:** 15.4.1 "Zenith-U" → 15.4.2 "Zenith-U"
- **Focus:** Fix CI workflow ordering bugs, fix incorrect repository URLs in docs,
  harden `.gitignore` against generated artifacts, sync `BuildValidation.h`
  version constants with canonical `VersionString`.

### Fixed
- **`BuildValidation.h`:** `MinorVersion`/`PatchVersion` constants were stale
  (`3`, `0`); now correctly reflect `15.4.2` (`MinorVersion=4`, `PatchVersion=2`).
- **`performance-regression-gate.yml`:** Cache restore step was placed *after*
  the build step — cache never had a chance to skip redundant rebuilds. Swapped
  to correct order: cache → build.
- **`docs/mkdocs.yml`:** `repo_url` and `site_url` pointed to the wrong
  `ExplorerLens/ExplorerLens` placeholder; updated to `RajwanYair/ExplorerLens.io`.
- **`docs/mkdocs.yml`:** Nav referenced ~30 documentation pages that did not
  exist on disk (would cause `mkdocs build` to fail). Nav now only references
  files present in `docs/`.
- **`docs/mkdocs.yml`:** Mermaid fence format changed from `fence_code_format`
  to `fence_div_format` so that CDN mermaid.min.js auto-processes `<div class="mermaid">`
  elements correctly.
- **`.github/ISSUE_TEMPLATE/config.yml`:** Replaced placeholder repo slug with
  correct `RajwanYair/ExplorerLens.io` URLs; added Support link.
- **`.github/PULL_REQUEST_TEMPLATE.md`:** Improved closing-keywords guidance
  with inline comment explaining GitHub auto-close syntax.

### Added
- **`.gitignore`:** Added missing ignore patterns for all generated artifacts
  that could otherwise be accidentally committed:
  `*.msi`, `*.wixpdb`, `*.msix`, `*.appxbundle`, `packaging/*.sha256`,
  `packaging/output/`, `release-staging/`, `SHA256SUMS.txt`,
  `verification-report-*.json`, `ExplorerLens-*-SBOM.json`,
  `/build-debug/`, `/build-vs/`, `/build-vcpkg-debug/`,
  `compile_commands.json`, `Engine/Version.h`.

---

## [15.4.1] "Zenith-U" — 2026-03-25

### Summary
**Patch release — CI/CD fix + Integration Test Framework (Sprints 25–29)**

- **Version:** 15.4.0 "Zenith-U" → 15.4.1 "Zenith-U"
- **Test count:** 2,958 → 2,963 total (+5 Integration/COM tests)
- **Focus:** Fix all failing GitHub Actions workflows; add long-running integration
  test runner, COM round-trip validation, CI coverage gate, and performance baseline.

### Fixed
- **CI `code-quality.yml`:** Replace MSBuild `LENSShell.sln` with CMake engine-only
  static analysis — eliminates v145 toolset dependency on GitHub runners (v143).
- **CI `code-quality.yml`:** Make clang-format check non-blocking (warnings, not errors)
  so style deviations don't block merges.
- **CI `version-consistency` job:** Sync `LENSManager.rc` `FILEVERSION` from `15.2.0`
  to match canonical `VERSION` file (was stale from pre-15.3 era).
- **CI `build.yml`:** Add explicit ninja availability check with fallback install.
- **CI `CMakePresets.json`:** Add `CMAKE_SYSTEM_VERSION` and `CMAKE_MSVC_RUNTIME_LIBRARY`
  to `ci-release` / `ci-debug` presets for reliable Windows SDK detection on runners.
- **SVG files:** Fix illegal `--` in XML comments in `logo.svg` and `social-preview.svg`
  (XML spec forbids `--` inside `<!-- -->` — caused files to fail as graphics).

### Added
- **Sprint 25 — Integration Test Runner** (`Engine/Tests/Integration/IntegrationTestRunner.h/.cpp`):
  Corpus-driven decoder validation; walks `data/corpus/` and records pass/fail/skip
  per format with HTML + CSV report output.
- **Sprint 26 — Corpus Manifest** (`data/corpus/MANIFEST.json`):
  Structured registry of all corpus test files with provenance, format, and expected results.
- **Sprint 27 — Performance Regression Gate** (`Engine/Tests/benchmarks/baseline.json`,
  `.github/workflows/performance-regression-gate.yml`): 10-benchmark baseline;
  CI fails if measured throughput drops >15% or latency exceeds +20%.
- **Sprint 28 — Code Coverage CI** (`.github/workflows/coverage.yml`):
  OpenCppCoverage integration; enforces 60%+ coverage floor for Core + Decoders.
- **Sprint 29 — COM Integration Test** (`Engine/Tests/Integration/COMIntegrationTest.h/.cpp`):
  Validates `IThumbnailProvider` COM round-trip when `LENSShell.dll` is registered;
  gracefully skips when DLL is absent (CI-safe).

---

## [15.4.0] "Zenith-U" — 2026-04-16

### Summary
**Sprint 17–24 of 100 (CLI Tool `lens.exe`)** — A full-featured command-line
interface for ExplorerLens, covering thumbnail generation, format inspection,
cache management, COM registration, benchmarking, and system health diagnostics.

- **Version:** 15.3.0 "Zenith-T" → 15.4.0 "Zenith-U"
- **Test count:** 2,951 → 2,958 total (+7 CLI unit tests)
- **New files:** `src/Tools.CLI/` (8 source pairs) + `scripts/lens-autocomplete.ps1`

### Added
- **Sprint 17 — CLI Scaffold** (`CommandRouter.h/cpp`): `ParsedArgs`, `ISubCommand`,
  `CommandRouter`, `CreateLensCLI()` factory; dispatches to 7 subcommands.
- **Sprint 18 — Generate Command** (`GenerateCommand.h/cpp`): `lens generate <file>`
  with `--recursive`, `--output`, `--size`, `--quality`, `--dry-run` flags.
  Checks for `LENSShell.dll` adjacency; walks directories with `fs::recursive_directory_iterator`.
- **Sprint 19 — Info Command** (`InfoCommand.h/cpp`): `lens info <file> [--json]`.
  Magic-byte + extension format detection for 40+ formats; JSON output for scripting.
- **Sprint 20 — Cache Command** (`CacheCommand.h/cpp`): `lens cache clear|stats|warm`.
  Resolves `%LOCALAPPDATA%\ExplorerLens\ThumbnailCache\`; reads `cache_stats.json`.
- **Sprint 21 — Register Command** (`RegisterCommand.h/cpp`): `lens register|unregister`.
  Admin detection via `CheckTokenMembership`; CLSID presence check via `HKCR`;
  wraps `regsvr32 /s`; `--status` works without admin.
- **Sprint 22 — Benchmark + Doctor** (`BenchmarkCommand.h/cpp`, `DoctorCommand.h/cpp`):
  `lens benchmark` reports p50/p95/p99 latency + img/sec across 10 format categories.
  `lens doctor` runs 6 health checks (Windows version, COM, DLL, GPU/DXGI, cache, thumbnail svc).
- **Sprint 23 — Entry Point + Autocomplete** (`main.cpp`, `scripts/lens-autocomplete.ps1`):
  `wmain()` with COM STA init, ANSI terminal mode, `SIGINT/SIGTERM` signal handling.
  PowerShell `Register-ArgumentCompleter` for tab-completing all subcommands and options.
- **Sprint 24 — CMake Integration + Tests** (`src/Tools.CLI/CMakeLists.txt`):
  `LensCLI` executable target linking `ExplorerLensEngine + ExplorerLensModernRuntime`;
  3 CTest smoke tests (`CLIHelp`, `CLIInfoMissingFile`, `CLIDoctor`);
  7 `EngineTests.cpp` unit tests covering router dispatch, format detection, cache path,
  admin detection, benchmark structure, and doctor check integrity.

---

## [15.3.0] "Zenith-T" — 2026-03-25

### Summary
**Sprint 9–16 of 100 (Resilience & Hardening)** — Centralised input validation,
structured error taxonomy, per-decoder timeout enforcement, crash dump capture,
graceful degradation catalog, archive security hardening, and fuzz harness scaffold.

- **Version:** 15.2.1 "Zenith-S" → 15.3.0 "Zenith-T"
- **Test count:** 1,242 → 1,255 (2,938 → 2,951 total run)
- **New files:** `Engine/Core/DecodeInputValidator.h`, `Engine/Core/DecodeErrorCategory.h`,
  `Engine/Core/DecoderTimeoutGuard.h/.cpp`, `Engine/Core/GracefulDegradation.h`,
  `Engine/Core/ArchiveSecurityValidator.h`, `LENSShell/CrashDumpCapture.h/.cpp`,
  `Engine/Tests/FuzzTargets/Fuzz{Image,Archive,PDF}Decoder.cpp`

### Added
- **Sprint 9 — Decoder Input Validation Audit** (`DecodeInputValidator.h`): uniform
  file-size, dimension, and bit-depth guards shared by all decoders.
- **Sprint 10 — Structured Error Propagation** (`DecodeErrorCategory.h`): first-class
  `DecodeErrorCategory` enum with 24 values, `IsSecurityError()`, `IsRecoverable()`.
- **Sprint 11 — Decoder Timeout Enforcement** (`DecoderTimeoutGuard.h/.cpp`): 5-second
  per-decoder watchdog via background thread; returns `TimeoutResult::TimedOut` on expiry.
- **Sprint 12 — Crash Dump Capture** (`LENSShell/CrashDumpCapture.h/.cpp`): installs
  `SetUnhandledExceptionFilter`; writes MiniDump to `%TEMP%\ExplorerLens\crashes\`.
- **Sprint 13 — Graceful Degradation Catalog** (`GracefulDegradation.h`): six canonical
  failure modes (`NullBitmap`, `PlaceholderIcon`, `CorruptFileOverlay`, `TimeoutFallback`,
  `PasswordProtected`, `UnsupportedFormat`) with `InjectFault()` API for testing.
- **Sprint 14 — Archive Decoder Hardening** (`ArchiveSecurityValidator.h`): ZIP-bomb
  (100× expansion ratio), path-traversal (`..`), symlink, and entry-count checks.
- **Sprint 15 — Fuzz Harness Scaffold** (`Engine/Tests/FuzzTargets/`): LibFuzzer-compatible
  stubs for image, archive, and PDF decoder fuzz targets (build with clang `-fsanitize=fuzzer`).

### Tests
13 new unit tests: `Test_S9_DecodeInputValidator_*`, `Test_S10_DecodeErrorCategory_*`,
`Test_S11_DecoderTimeoutGuard_*`, `Test_S13_GracefulDegradation_*`,
`Test_S14_ArchiveSecurityValidator_*`.

---

## [15.2.1] "Zenith-S" — 2026-03-25

### Maintenance
- **Tool version updates** — cmake `4.2.3 → 4.3.0`, meson `1.10.0 → 1.10.2`, vcpkg `2025-11-19 → 2026-02-21`
- **Scoop cache cleared** — removed 689 MB of stale post-update download archives
- **Documentation sync** — all MD/config files updated to reflect current installed tool versions (`BUILD_QUICK_REFERENCE.md`, `LIBRARY_INVENTORY.md`, `build-method.md`, `refactoring-plan.md`, `scripts/README.md`, `tool-versions.md`, `copilot-instructions.md`)

### Changed
- **Version:** 15.2.0 "Zenith-S" → 15.2.1 "Zenith-S"

---

## [15.2.0] "Zenith-S" — 2026-03-24

### Planning
- **100-Sprint Execution Plan** — `docs/SPRINT_PLAN_100.md` — full roadmap through v17.0.0 "Nova"
  - 12 release milestones: v15.2.0 through v17.0.0; one GitHub Release per milestone
  - Themes: Architecture Hardening → Resilience → CLI → Test Infra → Release Eng → GUI → UX → Horizon Major → Cloud → AI/Perf → Enterprise
- **Sprint train schedule** — Sprints 1–8 (Zenith-S) through Sprint 100 (Nova)

### Release Engineering
- **release.yml overhauled** — publishes all binaries on every version tag:
  - `LENSShell.dll`, `LENSManager.exe`, `lens.exe` (when built), MSI, ZIP, `SHA256SUMS.txt`, SBOM
  - Two-job pipeline: `build` (windows-latest, MSVC v145) → `publish` (create GitHub Release)
  - Automatic CHANGELOG extraction for release notes body
  - Binary discovery across multiple output paths; graceful warnings for missing artifacts
  - `softprops/action-gh-release@v2` with draft flag for manual approval
- **VERSION file** added to repo root — single source for `release.yml` version resolution

### Documentation
- **docs/SPRINT_PLAN_100.md** — 100 sprints with precise file targets, test names, outputs per sprint
- **Social preview** — tagline updated with "100 Sprints to v17.0" sprint count indicator
- **copilot-instructions.md** — version updated to 15.2.0, release procedure + artifact checklist added
- **README.md** — version badge updated

### Changed
- **Version:** 15.1.0 "Zenith-R" → 15.2.0 "Zenith-S"
- **Engine.h:** `EXPLORERLENS_ENGINE_VERSION_MINOR` 1 → 2



### Changed — Major Refactoring Sprint

#### Architecture
- **C++20 concepts** for decoder interfaces — compile-time type safety for all decoder registrations
- **std::expected** error handling — replacing HRESULT with modern monadic error types
- **Data-driven format registry** — declarative format definitions (inspired by RegiLattice pattern)
- **Compile-time format validation** — constexpr checks for format table integrity

#### Language Migration
- **C# WinUI 3 Manager** — Modern Fluent Design replacement for legacy WTL dialog
  - NavigationView shell with format registration, settings, diagnostics pages
  - Native dark mode, Mica backdrop, responsive layout
  - COM interop bridge for shell extension management
- **Manager.WinUI project** migrated from WTL/ATL C++ to .NET 10 C# with WinUI 3

#### New Decoders
- QOI (Quite OK Image) — lossless, fast decode
- JPEG-XS — low-latency professional format
- PCX — legacy paintbrush format
- TGA/Targa — enhanced with RLE support
- ICO/CUR — Windows icon/cursor with size selection
- FITS — astronomy format with stretch algorithms
- PSD/PSB — Adobe Photoshop layer composite
- XCF — GIMP native format

#### Performance
- SIMD pixel conversion (AVX2/SSE4.2) for color space transforms
- Memory-mapped decode for large files (>100MB)
- AI-assisted smart thumbnail cropping
- GPU shader hot-reload for development
- Cache analytics and telemetry

#### Best-in-Class Features
- Explorer preview handler (IPreviewHandler) for full-page previews
- File property sheet provider for metadata in Details pane
- Context menu integration for manual thumbnail regeneration
- Batch thumbnail CLI tool for command-line automation
- Plugin SDK v3 with Rust FFI support
- Enterprise policy templates (GPO/Intune)
- Localization framework with 12 languages
- WCAG 2.1 AA accessibility compliance
- Auto-update mechanism with delta downloads
- Cloud thumbnail provider (OneDrive/GoogleDrive/Dropbox)

#### DevOps
- Cross-platform CI/CD test matrix (Windows x64, ARM64, Linux, macOS)
- SBOM generation (CycloneDX)
- WiX MSI installer with feature selection
- Portable ZIP package for xcopy deployment
- Security hardening audit (OWASP Top 10)

---

## [15.0.0] "Zenith" - 2026-07

### Added

#### Cross-Platform Thumbnail Support (Python)
- **Linux freedesktop.org thumbnailer** — XDG-compliant thumbnail generation with MD5 URI hashing
- **Cross-platform abstraction layer** — Auto-detects Windows/Linux/macOS, delegates to native provider
- **Platform-neutral CLI** — `--register`/`--unregister` commands work across platforms

#### Production Readiness
- **Test stability fix** — SecureAlloc overhead test with warmup pass and realistic threshold
- **Python linting pass** — Narrowed all broad exceptions, fixed type annotations, removed unused imports
- **C++ empty catch blocks fixed** — PortableModeManager.h, FITSDecoder.cpp
- **Version consistency** — Python setup.py and PROJECT_SPEC_PROMPT.md aligned to v15.0.0
- **Linting configuration** — pyproject.toml, setup.cfg, markdownlint, VS Code Python analysis

### Added

#### Core Engine & Libraries
- **libarchive 3.7.6** — Integrated as static library, replacing per-format archive handlers
- **SVG Direct2D rendering** — Real ID2D1DeviceContext5 SVG rendering replacing gradient stub
- **BCrypt hashing** — FileHashEngine uses Windows BCrypt API (MD5/SHA1/SHA256/SHA512)
- **ZeroCopyPipeline** — VirtualAlloc + VirtualLock pinned memory allocation
- **HotModeDirectoryEngine** — Real FindFirstFileExW with FIND_FIRST_EX_LARGE_FETCH

#### Next-Gen Image Decoders
- APNGDecoder — Frame-by-frame animated PNG decoder with composite thumbnail
- FLIFDecoder — Free Lossless Image Format decoder stub for legacy FLIF files
- BPGDecoder — Better Portable Graphics (HEVC-based) image decoder
- RGBEDecoder — Radiance HDR (.hdr) format tone-mapping to LDR thumbnail
- WebP2Decoder — Experimental WebP2 format decoder placeholder

#### Document & Text Renderers
- MarkdownPreviewRenderer — Markdown to styled HTML-like thumbnail preview
- SourceCodeThumbnail — Syntax-colorized source code mini-preview
- RTFDecoder — Rich Text Format content extraction & thumbnail
- LaTeXPreviewDecoder — LaTeX math/document preview renderer
- StructuredDataVisualizer — JSON/YAML/TOML/XML tree-view thumbnail

#### Archive & Compression Inspectors
- ZstdFrameDecoder, BrotliStreamInspector, LZ4FrameDecoder, XZStreamDecoder, SnappyFrameDecoder

#### 3D & CAD Decoders
- PLYPointCloudDecoder, OBJMeshDecoder, STLMeshDecoder, COLLADADecoder, FBXInspector

#### Media Enhancement
- MIDIVisualizer, WaveformGenerator, SpectrogramRenderer, VideoTimelineStrip, SubtitlePreviewDecoder
- DPX/Cineon film format, OGG album art, MOBI/FB2 cover extraction

#### Enterprise & Security Viewers
- CertificateViewer, RegistryExportViewer, ShortcutInspector, MSIPackageInspector, DiskImagePreview

#### Performance Optimization Pipeline
- ThreadLocalBufferPool, DecodeMemoizationEngine, AsyncPrefetchQueue, PriorityDecodeScheduler, MemoryMappedDecodePath

#### Quality & Observability
- DecodeLatencyHistogram, ErrorCategorizationEngine, HealthScoreAggregator
- PerformanceRegressionDetector, ResourceUsageProfiler

#### Smart Features (AI)
- ThumbnailRelevanceScorer, ColorPaletteExtractor, ImageComplexityAnalyzer
- FormatMigrationAdvisor, DecodeStrategyOptimizer

#### Platform & Integration
- ClipboardMonitorIntegration, ShellNotificationProvider, ExplorerStatusBarProvider
- FileSummaryTooltipGenerator, BatchProgressReporter

#### Production Infrastructure
- ProductionPipelineV2, ContentAwareThumbnail, RuntimeSIMDDispatcher
- DecoderPerformanceCounters, ThumbnailQualityGate, BatchThumbnailOrchestrator
- FileSignatureDetector, GPUResourcePoolManager, CacheCoherencyProtocol
- ThumbnailPipelineProfiler, FormatNegotiator, TelemetryAggregator, DecoderRegistryV2

#### GUI Enhancements
- Enhanced About dialog (310x280 DU) with system info panel (CPU, GPU, SIMD, RAM)
- Copy Info button with clipboard support
- Benchmark button with synthetic 512x512 BGRA pipeline test
- About button for quick access from main dialog

### Fixed
- **Critical: COM CLSID mismatch** — WiX installer registered wrong CLSID; corrected to canonical `{9E6ECB90-...}`
- **Critical: TROUBLESHOOTING.md** — 6 instances of wrong CLSID corrected
- **Version alignment** — Bulk update across 17+ docs to v15.0.0
- **VS references** — Updated to Visual Studio 18 2026 across 7 files
- **Dead code** — Removed obsolete unzip.cpp stub (replaced by minizip-ng)
- **RAR/CBR format routing** — Added LENSTYPE_RAR (3) and LENSTYPE_CBR (4) constants; `.rar`/`.cbr` routed through GetLENSTYPE()
- **JPEG XR format routing** — Added LENSTYPE_JXR (94); `.jxr`/`.wdp`/`.hdp` extensions routed
- **SIMD implementation** — RuntimeSIMDDispatcher SSE2/AVX2 resize and blend now use real intrinsics (`_mm_*`/`_mm256_*`) instead of scalar fallbacks
- **GUI Select All/Deselect All** — Visible buttons added to LENSManager main dialog (previously keyboard-only)
- **copilot-instructions.md** — Updated CRT linkage docs, expanded library table (12→18 entries)
- **KNOWN_ISSUES.md** — Reorganized; moved 4 resolved items to Resolved section, added RAR/CBR/JXR fix

### Changed
- **Version:** 14.0.0 "Apex" → 15.0.0 "Zenith"
- **Total unit tests:** 2282 → 2820
- **Codename:** Apex → Zenith

### Improvements (Sprint 394)

#### Dark Theme Fix
- **LENSManager dark theme text** — Fixed checkbox, radio button, and groupbox controls rendering black text on dark backgrounds. Root cause: `DarkMode_Explorer` visual style overrides WM_CTLCOLORSTATIC text color. Fix: disable visual styles for non-push-button controls so GDI respects `SetTextColor(hdc, theme.text)`.
- **LENSShell dark theme** — Fixed `IsHighContrastMode()` forward-declaration order in `LENSShell/DarkModeHelper.h`; moved function before first call site.

#### Warning & Stub Fixes
- Fixed 17 C4100/C4101 warnings across 16 header files (unused parameter/variable suppressions)
- Replaced 4 stub implementations with real Windows API calls:
  - `DarkModeTextFix.h` `IsSystemDarkTheme()` — real registry read from `HKCU\...\Themes\Personalize`
  - `PluginMarketplace.h` `SignatureVerifier::Verify()` — real WinVerifyTrust Authenticode verification
  - `BatchThumbnailOrchestrator.h` `ProcessItem()` — real `GetFileAttributesW` file validation
  - `IntegrationTests.h` `RunCacheRoundTrip()` — real SubMillisecondCacheEngine round-trip test
- Fixed `PluginMarketplace.h` Authenticode result: narrow string assignment (was `wchar_t`), removed nonexistent `trusted` field

#### New Feature Headers (10 real implementations)
- `Pipeline/DecodeCancellationEngine.h` — atomic cancellation tokens with `CancelReason` enum
- `Cache/CacheBloomFilter.h` — FNV-1a + MurmurMix double-hashing Bloom filter
- `GPU/GPUPowerStateManager.h` — DXGI adapter enumeration, power-aware iGPU/dGPU routing
- `Memory/CopyOnWriteBufferPool.h` — COW buffer pool with shared_ptr reference counting
- `Pipeline/RequestDeduplicator.h` — inflight request coalescing with future-based deduplication
- `Cache/CacheFragmentationAnalyzer.h` — 5-tier fragmentation analysis with compaction estimation
- `GPU/GPUWorkgroupOptimizer.h` — vendor-aware 2D/1D compute workgroup dispatch optimization
- `Memory/MemoryMappedThumbnailAtlas.h` — zero-copy memory-mapped atlas with CreateFileMapping
- `AI/ThumbnailAestheticScorer.h` — composition/sharpness/color/luminance aesthetic scoring
- `Core/ColdStartOptimizer.h` — DLL preload manager for eliminating cold-start decode latency

#### Tests
- Added 28 new unit tests for Sprint 394 features (total: 2820, 0 failures)

### Improvements (Sprint 17-34)

#### Build & DevOps (Sprint 17)
- Log-based build monitoring (`build-and-log.bat`, `test-and-log.bat`) — no more terminal kills
- `.github/standards/shell-integration.md` — documented shell/build integration rules
- VS Code settings hardened: terminal confirmOnKill, markdownlint suppressions

#### Stub & Implementation Fixes (Sprint 18)
- ETWTraceProvider — atomic counters for event emission
- ProductionPipelineV2 — real system probing (CPU, GPU, memory, disk)
- SIMDDispatchRouter — real SSE/AVX2 intrinsics (ColorConvert, AlphaPremultiply)
- GetCurrentTimestamp — chrono-based implementations in 3 files

#### Project Icons (Sprint 19-20)
- Magnifying glass icon (6 resolutions: 16-256px, blue gradient lens, dark frame)
- Updated LENSShell, LENSManager, NSIS, Inno Setup, and MSIX assets

#### Codebase Consolidation (Sprint 21-22)
- 3 duplicate headers converted to forwarding includes (-573 net lines)
- Version proliferation confirmed clean — all V1/V2/V3 already forwarding

#### Security Hardening (Sprint 23, 31-32)
- `SecureAllocator.h` — STL-compatible allocator with zero-fill, 256MB limit, atomic tracking
- `InputValidator.h` — centralized path/dimension/size validation
- `DecoderSecurityHardening.h` — safe integer math, dimension limits, magic validation
- Enhanced `SecurityCompliance.h` — DecodeRateLimiter, FileSizeLimits, PathValidator
- Replaced `MemorySafety.h` stubs with real MemoryLeakDetector
- Hardened 9 decoders: QOI, TGA, PPM, PSD, PCX, HDR, SGI, Farbfeld, VTF

#### Test Coverage (Sprint 24-28, 33-34)
- 32 cache/pipeline/memory/plugin tests (Sprint 24)
- 25 AI + decoder tests (Sprint 25-26)
- 22 GPU/cloud/enterprise/performance tests (Sprint 27-28)
- 29 utility module tests (Sprint 33-34)
- Total: 126 new tests added across all subsystems

#### Test Corpus (Sprint 29-30)
- 138 test fixture files across 91 directories (44.6 KB)
- Formats: SVG, PPM/PGM/PBM, XPM, BMP, PNG, QOI, TGA, TIFF, GIF, ZIP, HTML
- Valid samples + corner cases + invalid/corrupt variants

### Removed
- 6 obsolete documentation files
- Dead unzip.cpp stub
- Stale planning documents
- Dead telemetry shim headers (TelemetryPipeline, TelemetryPipelineV2, TelemetryDashboard, TelemetryEngine, TelemetryHooks)
- Dead code analysis shim (DeadCodeAnalysis.h — callers use DeadCodeAudit.h + DeadCodeAuditor.h directly)
- Legacy CI workflow (`build-v7.yml`)
- 5 tracked .gitignore'd CMake-generated vcxproj files from git index
- `.github/standards/` files renamed from UPPERCASE to lowercase-hyphenated

---

## [14.0.0] "Apex" - 2026-06

### Added
- **GPU Pipeline:** DX12 Ultimate mesh shaders, DXR, VRS, SM6.7 DXIL, PSO disk cache
- **Format Intelligence:** Smart format detector V2, extended video/audio/3D renderers
- **Plugin Ecosystem V2:** SDK V2 (9 capabilities), debugger integration, hot reload, profiler
- **Security Hardening V2:** Threat model (STRIDE/CVSS), ASan, SBOM/CVE, runtime integrity
- **UX Polish V2:** Progressive thumbnail loader, animation engine V2, Quick Look integration
- **AI Intelligence V2:** Scene understanding, smart crop, IQA (BRISQUE/NIQE), CLIP embeddings
- **Enterprise & Cloud V2:** Policy engine (GPO/MDM), SharePoint, multi-tenant cache, GDPR/HIPAA audit
- **Platform:** Windows 12 compatibility, ARM64 optimizer (NEON/SVE), sub-millisecond cache
- **Performance:** GPU decode acceleration V2 (NVDEC/QuickSync/AMF), parallel I/O (scatter-gather)
- **Quality:** Accessibility suite V2 (WCAG 2.1 AA), Release Gate V32 (23 KPIs)
- **New modules:** Engine/AI/, Engine/GPU/, Engine/Memory/

### Changed
- **Version:** 13.0.0 → 14.0.0 "Apex"
- **Total unit tests:** 937 → 1187

---

## [13.0.0] - 2026-05

### Added
- CDR/Visio vector decoder, HDF5/NetCDF scientific data, NIfTI neuroimaging
- STEP/IGES CAD formats, HDR display pipeline
- Per-monitor DPI V3, shell overlay icons, cache warming engine, multi-GPU load balancer
- Accessibility pipeline, telemetry analytics, cloud storage integration

### Changed
- **Version:** 12.0.0 → 13.0.0 | **Tests:** 837 → 937

---

## [12.0.0] - 2026-04

### Added
- Parallel batch processing, persistent cache/USN journal, ARM64 validation V2
- Windows 11 24H2 integration, fuzz testing engine, Vulkan compute pipeline
- Plugin marketplace V3, AI-assisted thumbnails, USD/USDZ 3D format
- CSV/JSON data preview, Notebook/Jupyter preview, database/SQLite preview

### Changed
- **Version:** 11.0.0 → 12.0.0 | **Tests:** 737 → 837

---

## [11.0.0] - 2026-03

### Added
- DPX/Cineon film format, APNG/animated format, text/code preview
- DICOM V2, FITS V2, 3MF/USD 3D printing
- D3D12 pipeline activation, async shell extension V2, SIMD acceleration V2

### Changed
- **Version:** 10.6.0 → 11.0.0 | **Tests:** 712 → 737

---

## [10.6.0] - 2026-03

### Added
- Format registry refactor — FormatType enum, FormatRegistry singleton
- LENSArchive.h split — FormatTypeLookup with 80+ extension mappings
- Shell registration manager, test infrastructure V2

### Changed
- **Version:** 10.5.0 → 10.6.0 | **Tests:** 687 → 712

---

## [10.5.0] - 2026-02

### Added
- File Hash Engine, Registry Manager, Error Recovery Engine, Log Rotation Engine
- Resource Pool Engine, CLI Parser, Metadata Extractor, Notification Engine
- Content Indexer, Network Diagnostics, Config Migration Engine

### Changed
- **Version:** 10.4.0 → 10.5.0

---

## [10.4.0] - 2026-01

### Added
- Accessibility Engine, Cloud Sync Provider, Format Converter Engine
- Enterprise Deployment (ADMX/GPO), Watch Folder Engine, Diagnostic Dashboard
- Localization Engine, Theme Engine, Telemetry Engine, Update Engine
- Shell Preview Handler, Batch Processing Engine

### Changed
- **Version:** 10.3.0 → 10.4.0

---

## [10.3.0] - 2025-12

### Added
- Animated Thumbnail Engine, Shell Context Menu V2, Portable Mode Manager
- Network Provider Engine, Security Hardening V2

---

## [10.2.0] - 2025-11

### Added
- CI/CD Pipeline Validation, eBook Decoder, Geospatial Decoder
- Auto Documentation Generator, Config Migration Engine

---

## [10.1.0] - 2025-10

### Added
- Async Shell Extension, Encoder Export Engine, SIMD Accelerator, Windows 11 Integration

---

## [10.0.0] - 2025-09

### Added
- Scientific Format Suite (DICOM, FITS), Advanced 3D Formats (FBX/USD/3MF/STEP/IGES)
- Plugin Marketplace V2, Vulkan Compute Pipeline, Python SDK

---

## [9.2.0] - 2025-08

### Added
- D3D12 Compute Pipeline, Parallel Batch Decoder, Persistent Disk Cache
- ARM64 Hardware Validator, High-DPI Scaling, MSIX Packaging
- Memory Safety (ASAN), Code Coverage Integration, Malformed Input Hardening

### Changed
- **Tests:** 437 → ~587

---

## [9.0.0] - 2025-07

### Added
- Format Expansion: WMF/EMF, PCX, Farbfeld, JPEG 2000, EPS/PostScript
- Game Textures: KTX/KTX2, VTF | Creative Suite: OpenRaster, GIMP XCF
- Enhanced Model Decoder: PLY/DAE/3DS/FBX wireframe | Retro: SGI/RGB, XPM

---

## [8.4.0] - 2025-06

### Fixed
- `.djvu`/`.djv` routing, AVIF/HEIF decoder overlap, 46 missing shell registrations

### Changed
- **Shell registrations:** 47 → 93

---

## [8.3.0] - 2025-05

### Added
- Plugin sandbox, trust chain, ARM64 build config, JPEG2000 header
- CAD decoder plugin, glTF decoder, archive memory compactor, zero-copy pipeline

### Changed
- **Tests:** ~100 → ~437

---

## [8.2.0] - 2025-04

### Added
- Advanced decoders: TGA, QOI, PSD, DDS, HDR, EXR, ICO, PPM, BMP, GIF
- Document decoder (DOCX/PPTX/XLSX), Font decoder, Video/Audio decoders

---

## [8.1.0] - 2025-03

### Added
- SVG decoder (Direct2D), RAW decoder (LibRaw, 27 camera formats)
- PDF decoder (MuPDF), Archive expansion (TAR/CPIO/ISO/CAB/DEB/XAR)

---

## [8.0.0] - 2025-02

### Added
- WebP (libwebp 1.5.0), AVIF (libavif 1.3.0 + dav1d), HEIF (libheif 1.19.5 + libde265)
- JPEG XL (libjxl 0.11.1), DirectX 12 GPU pipeline foundation
- ExplorerLensEngine.lib as separate static library, CMake 3.20+ build system

---

## [7.1.0] - 2026-02-18

### Added
- Observability Integration (ETW + structured logger)
- Build Validation, MSIX CLSID fix, LENSTYPE enum expansion

---

## [7.0.0] - 2026-02-16

### Added
- LibHEIF build script, integration tests for all 24 decoders

### Fixed
- Visual Studio 18 2026 migration (16+ locations)

---

## [6.2.0] - 2026-02-15

### Added
- User/Developer/Known Issues documentation
- SEH Exception Handling, Circuit Breaker Pattern, AVX2 flags

### Fixed
- LZMA SDK updated to 26.00, path standardization

---

## [6.0.0] - 2026-02-12

### Added
- EXIF Orientation Utility, Header Data IPC Protocol, Minizip-NG Integration

---

## [5.3.0] - 2026-02-10

### Added
- LibRaw SDK, Plugin system architecture, GDI+ RAII wrapper

---

[15.0.0]: https://github.com/ExplorerLens/ExplorerLens.io/compare/v14.0.0...v15.0.0
[14.0.0]: https://github.com/ExplorerLens/ExplorerLens.io/compare/v13.0.0...v14.0.0
[13.0.0]: https://github.com/ExplorerLens/ExplorerLens.io/compare/v12.0.0...v13.0.0
[12.0.0]: https://github.com/ExplorerLens/ExplorerLens.io/compare/v11.0.0...v12.0.0
[11.0.0]: https://github.com/ExplorerLens/ExplorerLens.io/compare/v10.6.0...v11.0.0
[10.6.0]: https://github.com/ExplorerLens/ExplorerLens.io/compare/v10.5.0...v10.6.0
[10.5.0]: https://github.com/ExplorerLens/ExplorerLens.io/compare/v10.4.0...v10.5.0
[10.4.0]: https://github.com/ExplorerLens/ExplorerLens.io/compare/v10.3.0...v10.4.0
[10.3.0]: https://github.com/ExplorerLens/ExplorerLens.io/compare/v10.2.0...v10.3.0
[10.2.0]: https://github.com/ExplorerLens/ExplorerLens.io/compare/v10.1.0...v10.2.0
[10.1.0]: https://github.com/ExplorerLens/ExplorerLens.io/compare/v10.0.0...v10.1.0
[10.0.0]: https://github.com/ExplorerLens/ExplorerLens.io/compare/v9.2.0...v10.0.0
[9.2.0]: https://github.com/ExplorerLens/ExplorerLens.io/compare/v9.0.0...v9.2.0
[9.0.0]: https://github.com/ExplorerLens/ExplorerLens.io/compare/v8.4.0...v9.0.0
[8.4.0]: https://github.com/ExplorerLens/ExplorerLens.io/compare/v8.3.0...v8.4.0
[8.3.0]: https://github.com/ExplorerLens/ExplorerLens.io/compare/v8.2.0...v8.3.0
[8.2.0]: https://github.com/ExplorerLens/ExplorerLens.io/compare/v8.1.0...v8.2.0
[8.1.0]: https://github.com/ExplorerLens/ExplorerLens.io/compare/v8.0.0...v8.1.0
[8.0.0]: https://github.com/ExplorerLens/ExplorerLens.io/compare/v7.1.0...v8.0.0
[7.1.0]: https://github.com/ExplorerLens/ExplorerLens.io/compare/v7.0.0...v7.1.0
[7.0.0]: https://github.com/ExplorerLens/ExplorerLens.io/compare/v6.2.0...v7.0.0
[6.2.0]: https://github.com/ExplorerLens/ExplorerLens.io/compare/v6.0.0...v6.2.0
[6.0.0]: https://github.com/ExplorerLens/ExplorerLens.io/compare/v5.3.0...v6.0.0
[5.3.0]: https://github.com/ExplorerLens/ExplorerLens.io/releases/tag/v5.3.0
