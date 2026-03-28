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
**Theme: Edge AI & Continuous On-Device Learning**

Enable progressive model adaptation using on-device few-shot learning. Users' corrections
(e.g. "this crop is bad") feed a lightweight LoRA adapter that personalises thumbnail
ranking and smart-crop selection over time — fully private, no data leaves the device.

**Deliverables:**
- `Engine/AI/OnDeviceFewShotAdapter.h`: LoRA-based few-shot adaptation layer (ONNX runtime)
- `Engine/AI/UserFeedbackIngestor.h`: User feedback collector for implicit preference signals
- `Engine/AI/PersonalisationModelStore.h`: Per-user personalisation model store (encrypted)
- `Engine/AI/AdaptiveCropSelectionV2.h`: Adaptive crop selection v2 with user preference signals
- `Engine/AI/EdgeModelCompressionEngine.h`: Quantisation-aware model compression for edge targets
- `Engine/AI/ContinualLearningScheduler.h`: Continual learning scheduler (replay buffer + EWC)
- `Engine/AI/ModelVersioningManager.h`: On-device model versioning with rollback support
- `Engine/AI/PrivacyPreservingTrainer.h`: Differential-privacy training wrapper (ε-δ accountant)

**Test Count:** 3,461 + 8 = **3,469**

---

## Sprint 911–920 — v29.3.0 "Capella-T"
**Theme: Spatial Computing & Immersive Previews**

Adds stereoscopic and lightfield preview rendering for MR/AR headsets. Extends the GPU
pipeline with depth-aware composition, OpenXR layer submission, and a WebXR preview
bridge for browser-based holographic viewers.

**Deliverables:**
- `Engine/GPU/StereoPreviewRenderer.h`: Side-by-side stereo thumbnail renderer
- `Engine/GPU/LightfieldPreviewEncoder.h`: Lightfield / plenoptic preview encoder
- `Engine/GPU/OpenXRLayerSubmitter.h`: OpenXR composition layer submission for MR headsets
- `Engine/Core/DepthCompositor.h`: Depth-buffer-aware thumbnail compositor
- `Engine/GPU/WebXRPreviewBridge.h`: WebXR / WebGPU bridge for browser holographic previews
- `Engine/Core/SpatialAudioAnnotator.h`: Spatial audio metadata extractor for video thumbnails
- `Engine/GPU/VariableRateShadingController.h`: VRS controller for adaptive-quality preview tiles
- `Engine/Utils/OpenXRCapabilityProbe.h`: OpenXR runtime capability probe (runtime / extension discovery)

**Test Count:** 3,469 + 8 = **3,477**

---

## Sprint 921–930 — v29.4.0 "Capella-U"
**Theme: Telemetry & Observability v3 (OpenTelemetry + OTLP)**

Migrate from ETW-only observability to a fully OpenTelemetry-compatible stack. Introduces
OTLP exporter, distributed trace context propagation across COM boundaries, live metrics
streaming, and a Prometheus scrape endpoint for enterprise monitoring.

**Deliverables:**
- `Engine/Core/OTLPExporter.h`: OpenTelemetry OTLP/gRPC exporter for traces + metrics
- `Engine/Core/DistributedTraceContext.h`: W3C TraceContext propagation across COM boundaries
- `Engine/Core/PrometheusMetricsEndpoint.h`: Prometheus scrape endpoint (HTTP/1.1 pull model)
- `Engine/Core/LiveMetricsStreamer.h`: Sub-second live metrics streaming to dashboard
- `Engine/Core/SamplingPolicyEngine.h`: Adaptive trace sampling policy engine (head/tail sampling)
- `Engine/Utils/StructuredLogSinkV3.h`: Structured log sink v3 with OTLP log bridge
- `Engine/Core/SpanAttributeEnricher.h`: COM span attribute enricher (user agent, session, build)
- `Engine/Utils/HealthCheckHttpHandler.h`: HTTP health-check endpoint (/healthz, /readyz, /livez)

**Test Count:** 3,477 + 8 = **3,485**

---

## Sprint 931–940 — v29.5.0 "Capella-V"
**Theme: Plugin Marketplace Commerce & Monetisation**

Build the full plugin marketplace backend: app-store signing, in-app purchase receipts,
subscription management, usage-based billing, and a developer revenue dashboard SDK.

**Deliverables:**
- `Engine/Plugin/MarketplaceReceiptVerifier.h`: App-store receipt verifier (WinStore + sideload)
- `Engine/Plugin/SubscriptionLicenseManager.h`: Subscription + seat-based license manager
- `Engine/Plugin/UsageBillingMeter.h`: Usage-based billing meter (API calls + decode events)
- `Engine/Plugin/DeveloperRevenueSDK.h`: Developer revenue reporting SDK (webhook push)
- `Engine/Plugin/PluginTrialManager.h`: Trial period enforcement with grace-period logic
- `Engine/Plugin/MarketplaceSearchIndex.h`: Local plugin search index (BM25 + semantic)
- `Engine/Plugin/LicenceHostingProxy.h`: Offline licence hosting proxy for air-gapped envs
- `Engine/Plugin/PluginReviewGateway.h`: Pre-publish review gateway (static analysis + signing)

**Test Count:** 3,485 + 8 = **3,493**

---

## Sprint 941–950 — v29.6.0 "Capella-W"
**Theme: Enterprise SSO & Compliance v2 (SAML 2.0 + SOC-2 Type II)**

Complete enterprise identity integration: SAML 2.0 SP-initiated SSO, OAuth 2.1 + PKCE,
automated SOC-2 Type II evidence collection, and a SCIM 2.0 provisioning adapter for
Entra ID / Okta.

**Deliverables:**
- `Engine/Core/SAML2SPBridge.h`: SAML 2.0 service-provider bridge (AssertionConsumer endpoint)
- `Engine/Core/OAuth21PKCEEngine.h`: OAuth 2.1 + PKCE authorisation code flow engine
- `Engine/Core/SCIM2ProvisioningAdapter.h`: SCIM 2.0 user/group provisioning adapter
- `Engine/Utils/SOC2EvidenceCollector.h`: Automated SOC-2 Type II evidence collection engine
- `Engine/Core/EntraIDGroupPolicySync.h`: Microsoft Entra ID group-policy synchroniser
- `Engine/Core/OktaWorkforceIdentityBridge.h`: Okta Workforce Identity integration bridge
- `Engine/Utils/AuditTrailExporter.h`: Immutable audit trail exporter (CEF / JSON-LD)
- `Engine/Core/ComplianceDashboardAPI.h`: REST compliance dashboard API (SOC-2 / ISO 27001)

**Test Count:** 3,493 + 8 = **3,501**

---

## Sprint 951–960 — v29.7.0 "Capella-X"  *(LTS Candidate)*
**Theme: Long-Term Support Hardening & Stability**

Finalises the v29.x LTS branch. Focus: hardened memory allocator, fuzzing-driven
crash elimination, deterministic build reproducibility, and a regression safety net
that gates releases on 99.99% test-pass rate.

**Deliverables:**
- `Engine/Core/HardenedAllocator.h`: Hardened slab allocator with guard pages + canaries
- `Engine/Core/DeterministicBuildValidator.h`: Deterministic build hash validator (reproducible output)
- `Engine/Core/FuzzCorpusManager.h`: Fuzz corpus manager (LibFuzzer seed corpus + coverage goals)
- `Engine/Core/RegressionSafetyNet.h`: Automated regression gate (bisect + blame attribution)
- `Engine/Utils/MemoryCanaryEngine.h`: Runtime memory canary engine (stack + heap sentinels)
- `Engine/Core/LTSCompatibilityMatrix.h`: LTS backward-compatibility matrix validator
- `Engine/Utils/CrashSignatureDeduplicator.h`: Crash signature deduplication (minidump clustering)
- `Engine/Core/StabilityScorecard.h`: Automated stability scorecard (MTBF + crash-free sessions)

**Test Count:** 3,501 + 8 = **3,509**

---

## Cumulative Progress Tracker

| Sprint Range | Version                        | Theme                             | Test Δ |
|-------------|-------------------------------|-----------------------------------|--------|
| 861–870     | v28.6.0 Polaris-W             | Post-Quantum Cryptography          | +8     |
| 871–880     | v28.7.0 Polaris-X             | Cross-Platform Preview (macOS/Linux) | +8   |
| 881–890     | v29.0.0 Capella (MAJOR)       | Gen-5 Platform + WinUI 4           | +8     |
| 891–900     | v29.1.0 Capella-R             | Accessibility v2                   | +8     |
| 901–910     | v29.2.0 Capella-S             | Edge AI & On-Device Learning       | +8     |
| 911–920     | v29.3.0 Capella-T             | Spatial Computing & AR/VR          | +8     |
| 921–930     | v29.4.0 Capella-U             | Telemetry & Observability v3       | +8     |
| 931–940     | v29.5.0 Capella-V             | Plugin Marketplace Commerce        | +8     |
| 941–950     | v29.6.0 Capella-W             | Enterprise SSO & Compliance v2     | +8     |
| 951–960     | v29.7.0 Capella-X (LTS)       | Long-Term Support Hardening        | +8     |
| **861–960** | **Polaris-W through Capella-X** | **Gen-5 Platform**               | **+80** |

**Total at end of Sprint 960:** 3,429 + 80 = **3,509 unit tests**

---

## Metrics & Performance Targets (Sprint 860 → 960)

| Metric | v28.5.0 Baseline | v29.7.0 Target |
|--------|-----------------|----------------|
| Single thumbnail latency (P50) | 12 ms | 8 ms |
| Batch throughput | 320 img/s | 450 img/s |
| Cache hit latency | < 3 ms | < 1 ms |
| Memory footprint (idle) | 35 MB | 28 MB |
| Alt-text generation latency | < 500 ms | < 200 ms |
| Supported platforms | Windows + macOS | Win + macOS + Linux |
| Plugin ecosystem size | 60+ | 120+ |
| Cross-platform format parity | 85% | 100% |
| Crash-free session rate | 99.9% | 99.99% |

---

## Architecture Pillars for Gen-5 (v29.x)

1. **Out-of-Process Broker** — COM activation → async preview broker (`AsyncPreviewBroker.h`)
2. **Cross-Platform GPU** — D3D12 / Metal / Vulkan via `PlatformNeutralBuffer.h`
3. **OpenTelemetry-Native** — All spans, logs, metrics exported via OTLP
4. **Edge AI by Default** — LoRA personalisation, quantised models, privacy-preserving learning
5. **Compliance-Ready** — SOC-2 Type II, PQC signatures, SCIM 2.0 provisioning out of box
