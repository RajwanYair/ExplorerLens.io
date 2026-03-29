# ExplorerLens Sprint Plan — 1000 Series (Sprints 961–1060)
# Versions v30.0.0 "Deneb" through v31.1.0 "Achernar-R"

**Baseline Tests at Sprint 960:** 3,509  
**Projected Tests at Sprint 1060:** 4,229 (+720)  
**Codename Theme:** Deneb (v30.x — Gen-6 Unification) → Achernar (v31.x — Generative Era)  
**Previous Plan:** [SPRINT_PLAN_900.md](SPRINT_PLAN_900.md) (Sprints 861–960, v28.6.0–v29.7.0)

---

## Overview: The Gen-6 Era

The **Deneb** major series (v30.x) represents the Gen-6 architectural leap for ExplorerLens:
**Platform Unification** across Windows / macOS / Linux, **DirectStorage GPU Decompression**
for sub-millisecond large-file preview, **CLIP Semantic Search** enabling natural-language file
discovery, and the **Universal Format Decoder Library** — an open-source, standalone decoder
facade that can be embedded in any application.

The **Achernar** major series (v31.x) opens the **Generative Era**: text-to-thumbnail synthesis,
AI-assisted content inpainting, multi-platform shell integration, and the foundation for
intelligent autonomous workflows that anticipate user needs before they are expressed.

### Design Philosophy: "Invisible Excellence"

Gen-6 targets zero perceivable latency — thumbnails appear before Explorer's window renders.
Every architectural decision in this plan is evaluated against three invariants:

1. **Sub-frame (< 8 ms P50)** for cache-warm thumbnails at 256×256
2. **Zero cold-start CPU stall** — DirectStorage + pre-gen means first show is from GPU cache
3. **Cross-platform behavioral parity** — same thumbnail output on Windows / macOS / Linux

---

## Release Map

| Version  | Codename    | Sprints   | Theme                                         | Tests  |
|----------|-------------|-----------|-----------------------------------------------|--------|
| v30.0.0  | Deneb       | 961–970   | Gen-6 Platform Unification (MAJOR)            | 3,581  |
| v30.1.0  | Deneb-R     | 971–980   | DirectStorage & GPU Decompression             | 3,653  |
| v30.2.0  | Deneb-S     | 981–990   | CLIP Semantic Search & Discovery              | 3,725  |
| v30.3.0  | Deneb-T     | 991–1000  | Live Preview Scrubber & Rich Media            | 3,797  |
| v30.4.0  | Deneb-U     | 1001–1010 | Geospatial, Medical & Scientific Formats      | 3,869  |
| v30.5.0  | Deneb-V     | 1011–1020 | Universal Format Decoder Library              | 3,941  |
| v30.6.0  | Deneb-W     | 1021–1030 | Plugin Marketplace v4 & Commerce              | 4,013  |
| v30.7.0  | Deneb-X     | 1031–1040 | Enterprise Console v3 & Fleet Management      | 4,085  |
| v31.0.0  | Achernar    | 1041–1050 | Generative AI Thumbnails (MAJOR)              | 4,157  |
| v31.1.0  | Achernar-R  | 1051–1060 | Cross-Platform Shell (Linux + macOS)          | 4,229  |

---

## Sprint 961–970 — Gen-6 Platform Unification (v30.0.0 "Deneb") ★★ MAJOR

**Theme:** Unified rendering and shell-extension layer across Windows 11, macOS Sequoia,
and Linux (GTK4/Wayland). Introduces a platform-abstraction layer (PAL) that virtualises
GPU surface creation, file-system access, and shell-provider registration behind a single
cross-platform API, ending the Windows-only architectural constraint.

**Strategic Motivation:**
- macOS Quick Look and Linux Nautilus/Dolphin currently have zero ExplorerLens coverage
- 34% of professional users (survey data) work across Windows + macOS simultaneously
- Unified PAL reduces per-platform maintenance burden from O(N) to O(1)

**Deliverables:**

| # | Header | Purpose |
|---|--------|---------|
| 1 | `Engine/Core/PlatformAbstractionLayer.h` | Unified cross-platform API shim — GPU surface, filesystem, threading, window management |
| 2 | `Engine/GPU/MetalPipelineV2.h` | macOS Metal rendering pipeline v2 (MDLAsset + MTLTexture ↔ BGRA32 bridge) |
| 3 | `Engine/GPU/LinuxDRMBackend.h` | Linux DRM/KMS thumbnail backend (mesa + EGL offscreen surface) |
| 4 | `Engine/Core/UniversalWindowBroker.h` | Cross-platform window/surface broker (HWND / NSView / GtkWidget abstraction) |
| 5 | `Engine/Core/NativeFilesystemAdapter.h` | Platform-native filesystem adapter (NTFS / APFS / ext4 / btrfs change notifications) |
| 6 | `Engine/Core/CrossPlatformShellProvider.h` | Shell provider entry points: WinShell / Quick Look / Nautilus / Dolphin unified interface |
| 7 | `Engine/Core/PlatformUIScalingEngine.h` | DPI-aware thumbnail scaling for all three platforms (Win HiDPI / Retina / Wayland HiDPI) |
| 8 | `Engine/Utils/PlatformBuildMatrix.h` | Compile-time and runtime platform build matrix validator (CI gate for all three platforms) |

**Test additions:** +72 tests (3,509 → 3,581)

**Acceptance Criteria:**
- All eight headers compile cleanly with MSVC v145 (Windows), AppleClang 16 (macOS), GCC 14 (Linux)
- `PlatformAbstractionLayer` passes a surface-creation roundtrip test on each platform
- `MetalPipelineV2` produces pixel-identical BGRA32 output to the D3D12 path (PSNR ≥ 45 dB)
- `LinuxDRMBackend` renders a 1×1 test thumbnail via EGL offscreen in CI (GitHub Actions ubuntu-latest)
- `CrossPlatformShellProvider` registers and de-registers without leaking COM / NSExtension objects
- `PlatformBuildMatrix` fails the CI gate if any platform-specific header leaks Windows-only types
- Zero new compiler warnings on any target platform

**Performance Targets:** PAL overhead < 0.5 ms per thumbnail on all platforms.

---

## Sprint 971–980 — DirectStorage & GPU Decompression (v30.1.0 "Deneb-R")

**Theme:** Eliminate the CPU decompression bottleneck for large files. DirectStorage 1.2
streams compressed file data directly to the GPU, where GDeflate / Zstandard GPU kernels
decompress in parallel. Combines with an expanded staging-buffer pool and async I/O broker
to achieve near-flash-speed thumbnail generation for large RAW and HDR files.

**Strategic Motivation:**
- Current P99 for a 50 MB camera RAW file: 340 ms (dominated by CPU inflate)
- Target with DirectStorage + GPU decompress: < 60 ms (5× improvement)
- Removes the single largest performance bottleneck for photography workflows

**Deliverables:**

| # | Header | Purpose |
|---|--------|---------|
| 1 | `Engine/GPU/DirectStorageEngine.h` | DirectStorage 1.2 file stream to GPU staging (IDStorageFactory + IDStorageQueue) |
| 2 | `Engine/GPU/GPUDecompressScheduler.h` | GPU-side GDeflate / ZStd decompression scheduler (NV / AMD / Intel compute kernels) |
| 3 | `Engine/Pipeline/DirectStoragePipelineStage.h` | DirectStorage pipeline stage integrating DS into the existing async pipeline |
| 4 | `Engine/GPU/NvGDeflateBackend.h` | NVIDIA GDeflate hardware decompression backend (RTX series) |
| 5 | `Engine/GPU/AMDDecompressBackend.h` | AMD GPU decompression backend (RDNA2+ compute shader path) |
| 6 | `Engine/Cache/DirectStorageCacheTier.h` | DirectStorage-backed L2 cache tier (NVMe SSD → GPU zero-bounce) |
| 7 | `Engine/Core/AsyncFileStreamBroker.h` | Async file streaming broker (overlapped I/O ↔ DirectStorage unified API) |
| 8 | `Engine/GPU/StagingBufferPoolV2.h` | Pooled staging buffers v2 for DirectStorage upload (configurable tier count) |

**Test additions:** +72 tests (3,581 → 3,653)

**Acceptance Criteria:**
- `DirectStorageEngine` loads a 50 MB ZSTD-compressed blob in < 80 ms on NVMe (CI: disk-speed-neutral mock)
- `GPUDecompressScheduler` dispatches correctly to NVIDIA / AMD / Intel paths based on GPU vendor
- `DirectStoragePipelineStage` integrates with existing async pipeline without breaking existing tests
- `StagingBufferPoolV2` achieves zero double-buffering stalls under concurrent decode load
- Fallback to CPU decompression works transparently when DirectStorage is unavailable
- All tests pass with DirectStorage mocked for CI environments without NVMe

---

## Sprint 981–990 — CLIP Semantic Search & Discovery (v30.2.0 "Deneb-S")

**Theme:** Natural-language search over thumbnails. CLIP vision-language embeddings are
computed on-device at index time and stored in a persistent HNSW kNN graph. Users type
"sunset over water" in Explorer's search bar and ExplorerLens returns visually matching
thumbnails with sub-100 ms latency for a 100,000-file corpus.

**Strategic Motivation:**
- File discovery is the #1 workflow friction point (user research: 62% of time spent searching)
- Local CLIP inference (CLIP-ViT-B/32 quantised) runs at 18 ms/image on Intel NPU
- Zero telemetry: all embedding computation and storage is fully on-device

**Deliverables:**

| # | Header | Purpose |
|---|--------|---------|
| 1 | `Engine/AI/CLIPEmbeddingEngine.h` | CLIP ViT-B/32 vision encoder — runs on NPU/DirectML/CPU with INT8 quantisation |
| 2 | `Engine/AI/SemanticSearchIndex.h` | HNSW approximate nearest-neighbour index for embedding similarity search |
| 3 | `Engine/AI/NaturalLanguageQueryParser.h` | NL text → CLIP text embedding pipeline (BERT tokeniser + CLIP text encoder) |
| 4 | `Engine/AI/VisualSimilarityGraph.h` | Per-folder kNN visual similarity graph (offline build + online incremental update) |
| 5 | `Engine/AI/EmbeddingCacheStore.h` | Persistent embedding cache (LevelDB-backed, LZ4-compressed, hot/cold tiering) |
| 6 | `Engine/AI/MultiModalRanker.h` | Multi-modal ranking fusion — CLIP score + filename BM25 + recency + file size |
| 7 | `Engine/AI/SearchResultDeduplicator.h` | Near-duplicate image detection in search results (perceptual hash clustering) |
| 8 | `Engine/AI/IncrementalIndexUpdater.h` | Incremental index update on file-system change events (inotify / ReadDirectoryChanges) |

**Test additions:** +72 tests (3,653 → 3,725)

**Acceptance Criteria:**
- CLIP embedding throughput ≥ 25 images/sec on Intel UHD 770 (DirectML path)
- HNSW index query latency < 15 ms for 100,000-vector corpus at recall@10 ≥ 0.92
- NL query "red sports car" returns automotive images with NDCG@10 ≥ 0.75 on test corpus
- Embedding store survives process restart and resumes incremental updates correctly
- Near-duplicate detection filters > 95% of bit-identical duplicates in test set
- All embedding operations run fully on-device — no network calls

---

## Sprint 991–1000 — Live Preview Scrubber & Rich Media (v30.3.0 "Deneb-T")

**Theme:** Transforms static thumbnails into interactive live previews. Hovering over a
video file scrubs keyframes; hovering over audio shows a waveform; multi-page documents
show a page-flip animation. Shader/code files render with syntax highlighting and a
subtle 3D tilt effect. Font files show a configurable pangram in the font face.

**Strategic Motivation:**
- Static thumbnails carry zero temporal information for video/audio/document files
- Preview hover is the most-requested feature in user feedback (3 years running)
- No performance overhead at rest — all scrubbing is lazy and event-driven

**Deliverables:**

| # | Header | Purpose |
|---|--------|---------|
| 1 | `Engine/Core/LivePreviewScrubber.h` | Event-driven thumbnail scrubber controller (mouse-enter → progressive decode) |
| 2 | `Engine/Core/VideoKeyframeExtractor.h` | Video keyframe extraction pipeline (FFmpeg-lite / MF source reader) |
| 3 | `Engine/Core/AnimatedFrameScrubber.h` | Animated GIF / WEBP / APNG frame scrubber with configurable FPS cap |
| 4 | `Engine/Core/AudioWaveformRenderer.h` | Audio waveform visualisation thumbnail (PCM peak extraction + RMS overlay) |
| 5 | `Engine/Core/DocumentPagePreviewer.h` | Multi-page document preview navigator (PDF / DOCX / PPTX page-flip) |
| 6 | `Engine/Core/ShaderSyntaxHighlighter.h` | Syntax-highlighted GLSL / HLSL / WGSL / Metal shader thumbnail with 3D tilt |
| 7 | `Engine/Core/FontGlyphSampler.h` | Font preview sampler — configurable pangram, weight, script, Unicode range |
| 8 | `Engine/Core/SpreadsheetChartRenderer.h` | Excel / CSV chart thumbnail (auto-detects pivot tables, sparklines, named charts) |

**Test additions:** +72 tests (3,725 → 3,797)

**Acceptance Criteria:**
- Video keyframe scrubbing delivers first frame within 80 ms of hover (cached: < 5 ms)
- Audio waveform renders for MP3/FLAC/OGG files in < 25 ms at 256 pixels width
- `FontGlyphSampler` renders Latin, Arabic, CJK, Devanagari pangrams without tofu glyphs
- `ShaderSyntaxHighlighter` handles 5,000-line GLSL without stall on main thread
- `DocumentPagePreviewer` page-flip animation stays under 16 ms/frame at 256×256
- Zero memory leaks verified via ASAN for all media decoders at session end

---

## Sprint 1001–1010 — Geospatial, Medical & Scientific Formats (v30.4.0 "Deneb-U")

**Theme:** Expand format coverage into high-value professional domains: satellite imagery
(GeoTIFF, ECW, NITF), medical imaging (DICOM Advanced 4D, NRRD), and scientific data
(HDF5, NetCDF, FITS astronomy). Each decoder produces domain-aware thumbnails — GeoTIFF
shows false-colour band compositing; DICOM shows windowed Hounsfield units; FITS shows
logarithmic stretch.

**Deliverables:**

| # | Header | Purpose |
|---|--------|---------|
| 1 | `Engine/Decoders/GeoTIFFDecoder.h` | GeoTIFF multi-band raster decoder with false-colour compositing and band selection |
| 2 | `Engine/Decoders/NITFDecoder.h` | NITF/NSIF national imagery format decoder (NGA STANAG 4545) |
| 3 | `Engine/Decoders/DICOMAdvancedDecoder.h` | DICOM Advanced 3D/4D multi-frame decoder with Hounsfield windowing |
| 4 | `Engine/Decoders/NRRDDecoder.h` | NRRD (Nearly Raw Raster Data) medical imaging decoder |
| 5 | `Engine/Decoders/HDF5ThumbnailDecoder.h` | HDF5 scientific data thumbnail renderer (dataset → pseudo-colour image) |
| 6 | `Engine/Decoders/NetCDFDecoder.h` | NetCDF-4 climate/oceanographic data visualiser (variable selection + colour map) |
| 7 | `Engine/Decoders/FITSDecoder.h` | FITS astronomy image decoder with logarithmic stretch and false-colour LUT |
| 8 | `Engine/Decoders/ECWDecoder.h` | ECW / JPEG2000 enhanced-compression wavelet decoder (Erdas/OGC) |

**Test additions:** +72 tests (3,797 → 3,869)

**Acceptance Criteria:**
- `GeoTIFFDecoder` renders a 4-band Landsat image with NDVI false-colour in < 50 ms
- `DICOMAdvancedDecoder` applies correct Hounsfield window (brain / lung / bone presets)
- `FITSDecoder` handles a 16-bit/32-bit float FITS image with zscale stretch
- `HDF5ThumbnailDecoder` extracts a 2D slice from a 3D dataset variable
- All decoders reject malformed / truncated files with structured error codes (no crashes)
- New decoders are registered in `LENSArchive.h` LENSTYPE enum without collision

---

## Sprint 1011–1020 — Universal Format Decoder Library (v30.5.0 "Deneb-V")

**Theme:** The Universal Format Decoder Library (UFDL) — a standalone, header-only facade
over all ExplorerLens decoders — packaged as an open-source NuGet/vcpkg library. Any
application can embed UFDL with a single `#include` and get thumbnail generation for 200+
formats without writing COM shell extension code, enabling an entire ecosystem of
independent integrators.

**Deliverables:**

| # | Header | Purpose |
|---|--------|---------|
| 1 | `Engine/Core/UniversalDecoderFacade.h` | Public API facade for all 200+ decoders — single-header library entrypoint |
| 2 | `Engine/Core/FormatCapabilityMatrix.h` | Runtime format × platform × decoder capability matrix (JSON queryable) |
| 3 | `Engine/Core/DecoderVersionRegistry.h` | Decoder version registry with SemVer compatibility tracking and upgrade paths |
| 4 | `Engine/Core/FormatFamilyResolver.h` | Hierarchical format family resolver (image → raster → lossy → JPEG etc.) |
| 5 | `Engine/Core/DecoderHotfixApplicator.h` | Live decoder hotfix applicator — patches running decoders without process restart |
| 6 | `Engine/Core/FormatSchemaValidator.h` | Machine-readable format specification compliance validator (RFC/ISO schema checks) |
| 7 | `Engine/Core/DecoderCompatLayer.h` | Cross-version decoder compatibility shim (UFDL v1 API works against v2 decoders) |
| 8 | `Engine/Core/UniversalLibraryManifest.h` | Library manifest: version, license, capabilities, third-party attributions (SPDX) |

**Test additions:** +72 tests (3,869 → 3,941)

**Acceptance Criteria:**
- `UniversalDecoderFacade` resolves a JPEG, WebP, PSD, and FITS file correctly via single API
- `FormatFamilyResolver` correctly classifies all 200+ registered formats into the hierarchy
- `DecoderHotfixApplicator` applies a test hotfix without restarting the host process
- `UniversalLibraryManifest` produces valid SPDX 3.0 JSON for all bundled third-party libs
- UFDL can be consumed as standalone `extern "C"` C ABI from Python via ctypes (integration test)
- `DecoderVersionRegistry` detects a breaking version mismatch and returns a structured error

---

## Sprint 1021–1030 — Plugin Marketplace v4 & Commerce (v30.6.0 "Deneb-W")

**Theme:** Full marketplace commerce layer for the ExplorerLens plugin ecosystem — plugin
discovery, ratings, subscriptions, usage-based billing, developer revenue dashboards, and
a pre-publish review gateway with automated static analysis and signing enforcement. Makes
ExplorerLens a first-class platform for plugin developers to build sustainable businesses.

**Deliverables:**

| # | Header | Purpose |
|---|--------|---------|
| 1 | `Engine/Plugin/MarketplaceClientV4.h` | Plugin marketplace client v4 (REST + gRPC dual-protocol, offline cache) |
| 2 | `Engine/Plugin/PluginRatingEngine.h` | Plugin rating + review aggregator (Bayesian scoring, fake-review detector) |
| 3 | `Engine/Plugin/PluginDependencyGraph.h` | Plugin dependency graph resolver (topological sort, cycle detection) |
| 4 | `Engine/Plugin/PluginBundleInstaller.h` | Bundle installer for plugin collections (atomic multi-install, rollback) |
| 5 | `Engine/Plugin/SubscriptionLicenseManagerV4.h` | Subscription + seat-based license manager v4 (JWT entitlement tokens) |
| 6 | `Engine/Plugin/PluginReputationScorer.h` | Reputation scorer (download velocity + star rating + security scan + CVE lookup) |
| 7 | `Engine/Plugin/AutoUpdatePolicyV4.h` | Auto-update policy v4 (silent / notify / defer / enterprise-locked) |
| 8 | `Engine/Plugin/PrePublishReviewGateway.h` | Pre-publish review gateway (WASM sandbox static analysis + signing enforcement) |

**Test additions:** +72 tests (3,941 → 4,013)

**Acceptance Criteria:**
- `MarketplaceClientV4` handles network partition gracefully (cached responses + retry)
- `PluginDependencyGraph` detects circular dependency and produces a human-readable error
- `SubscriptionLicenseManagerV4` validates a JWT entitlement token in < 1 ms (local cache)
- `PrePublishReviewGateway` rejects an unsigned WASM plugin with a structured rejection report
- `PluginRatingEngine` Bayesian scorer correctly shrinks towards prior for low-sample plugins
- All marketplace operations complete correctly with no network (air-gapped mode)

---

## Sprint 1031–1040 — Enterprise Console v3 & Fleet Management (v30.7.0 "Deneb-X")

**Theme:** Enterprise Console v3 — a real-time fleet management dashboard for large-scale
ExplorerLens deployments. Administrators can push policy updates, monitor performance
KPIs, remediate security alerts, and generate compliance evidence across thousands of
endpoints via MDM, WinGet, and the SCIM 2.0 provisioning API.

**Deliverables:**

| # | Header | Purpose |
|---|--------|---------|
| 1 | `Engine/Enterprise/EnterpriseConsoleV3.h` | Console v3 backend — REST + gRPC dual-protocol admin API surface |
| 2 | `Engine/Enterprise/FleetDeploymentManager.h` | Fleet deployment manager (MDM / WinGet / SCCM push, staged rollout) |
| 3 | `Engine/Enterprise/ComplianceReportGenerator.h` | GDPR / HIPAA / SOC-2 / ISO 27001 compliance evidence report generator |
| 4 | `Engine/Enterprise/EnterpriseMetricsDashboard.h` | Real-time KPI dashboard (P50/P99 latency, error rates, plugin adoption) |
| 5 | `Engine/Enterprise/PolicyVersionControl.h` | GPO policy version control (Git-backed change history + diff + rollback) |
| 6 | `Engine/Enterprise/RemoteDecoderControl.h` | Remote decoder enable / disable / quarantine via admin console (no restart) |
| 7 | `Engine/Enterprise/UsageAnomalyDetector.h` | Statistical usage anomaly detector (Z-score baseline deviation → alert) |
| 8 | `Engine/Enterprise/EnterpriseAuditExporter.h` | Audit log exporter to SIEM endpoints (Splunk HEC / Microsoft Sentinel / QRadar) |

**Test additions:** +72 tests (4,013 → 4,085)

**Acceptance Criteria:**
- `FleetDeploymentManager` correctly stages a rollout in 3 phases (canary / ring1 / full)
- `ComplianceReportGenerator` produces a valid SOC-2 evidence package in PDF + JSON-LD
- `EnterpriseMetricsDashboard` streams P99 latency metrics within 2 s of event
- `RemoteDecoderControl` disables a decoder across all registered endpoints in < 30 s
- `UsageAnomalyDetector` achieves < 5% false-positive rate on 30-day usage simulation
- `EnterpriseAuditExporter` produces CEF-compliant events accepted by Splunk HEC

---

## Sprint 1041–1050 — Generative AI Thumbnails (v31.0.0 "Achernar") ★★ MAJOR

**Theme:** The Generative Era. ExplorerLens gains the ability to *synthesise* thumbnails
rather than merely decode them. For corrupted, missing, or conceptual assets, the engine
uses on-device Stable Diffusion / FLUX (INT4 quantised, 12 ms/step on NPU) to generate
photorealistic placeholders. Alt-text descriptions are synthesised by a local vision LLM.
A content moderation filter ensures all generated output is safe for enterprise environments.

**Strategic Motivation:**
- Generative AI is the defining capability differential of 2026-2028 professional tools
- On-device INT4 models eliminate cloud dependency and privacy concerns
- Content moderation is a compliance requirement for enterprise deployment

**Deliverables:**

| # | Header | Purpose |
|---|--------|---------|
| 1 | `Engine/AI/GenerativeThumbnailEngine.h` | Text-to-thumbnail generative engine (SD-Turbo / FLUX.1-schnell INT4, NPU-accelerated) |
| 2 | `Engine/AI/ContentAwareInpainter.h` | AI inpainting for damaged / partial / corrupt thumbnails (LaMa + FLUX fill) |
| 3 | `Engine/AI/StyleTransferRenderer.h` | Artistic style transfer for brand-consistent thumbnail grid aesthetics |
| 4 | `Engine/AI/ImageDescriptionSynthesizer.h` | Vision LLM alt-text synthesiser (LLaVA-1.6 / Phi-3 Vision, on-device) |
| 5 | `Engine/AI/ThumbnailPersonalisationEngine.h` | DreamBooth-lite fine-tuning for brand / style personalisation (LoRA, < 5 min) |
| 6 | `Engine/AI/GenerativeUpscalerV3.h` | Generative detail-synthesis upscaler v3 (4×/8× ESRGAN + diffusion refinement) |
| 7 | `Engine/AI/ContentModerationFilter.h` | On-device NSFW / safety classifier gating all generated outputs (CLIP-based) |
| 8 | `Engine/AI/GenerativeAuditTrail.h` | Immutable audit trail for generative operations (hash-chain, W3C provenance) |

**Test additions:** +72 tests (4,085 → 4,157)

**Acceptance Criteria:**
- `GenerativeThumbnailEngine` generates a 256×256 thumbnail in < 800 ms on Intel NPU (INT4)
- `ContentAwareInpainter` fills a 25%-masked region with visually plausible content
- `ContentModerationFilter` achieves ≥ 99.5% recall on NSFW test set (zero false negatives)
- `GenerativeAuditTrail` hash chain is tamper-evident (validated by independent verifier)
- `ImageDescriptionSynthesizer` scores ROUGE-L ≥ 0.62 vs human alt-text baseline
- All generative operations are opt-in (default off for enterprise deployments)

---

## Sprint 1051–1060 — Cross-Platform Shell (Linux + macOS) (v31.1.0 "Achernar-R")

**Theme:** First-class native shell integration on Linux (Nautilus, Dolphin, Thunar)
and macOS (Quick Look, Finder). Delivers ExplorerLens thumbnail generation to the
two largest non-Windows developer platforms, supported by a D-Bus thumbnail daemon
(XDG spec), an Exclusive macOS Quick Look generator, and a cross-platform test harness
that validates pixel parity with the Windows reference decoder.

**Deliverables:**

| # | Header | Purpose |
|---|--------|---------|
| 1 | `Engine/Core/LinuxNautilusExtension.h` | GIO/Nautilus thumbnail extension (GnomeDesktopThumbnail API integration) |
| 2 | `Engine/Core/KDEDolphinExtension.h` | KDE Dolphin thumbnail plugin via KIO ThumbCreator interface |
| 3 | `Engine/Core/ThunarThumbnailExtension.h` | Xfce Thunar file manager thumbnail extension (tumbler scheduler D-Bus) |
| 4 | `Engine/Core/macOSQuickLookV3.h` | macOS Quick Look generator v3 (QLThumbnailRequest / Sequoia API, async) |
| 5 | `Engine/Core/LinuxThumbnailerDaemon.h` | XDG thumbnail daemon (FreeDesktop.org thumbnail spec, D-Bus activation) |
| 6 | `Engine/Core/WaylandShellExtension.h` | Wayland shell extension (xdg-portal + thumbnailer portal D-Bus interface) |
| 7 | `Engine/Core/macOSLaunchServicesAdapter.h` | macOS Launch Services UTI adapter (UTType → LENSTYPE routing) |
| 8 | `Engine/Core/XPlatformTestHarness.h` | Cross-platform decode parity test harness (pixel diff against Windows reference) |

**Test additions:** +72 tests (4,157 → 4,229)

**Acceptance Criteria:**
- `LinuxNautilusExtension` thumbnail appears in Nautilus within 2 s for a 10 MB JPEG (Ubuntu 24.04 CI)
- `macOSQuickLookV3` passes Apple Quick Look sandbox validation (net.prohibited entitlement free)
- `LinuxThumbnailerDaemon` registers correctly with `~/.thumbnails/` XDG base directory
- `XPlatformTestHarness` achieves SSIM ≥ 0.99 between Linux/macOS and Windows reference outputs
- `WaylandShellExtension` integrates with GNOME 47 via xdg-portal without X11 dependency
- Shell extensions on all platforms exhibit zero stalls on the file manager's main thread

---

## Cumulative Progress Tracker

| Sprint Range | Version                           | Theme                                  | Test Δ |
|-------------|----------------------------------|----------------------------------------|--------|
| 961–970     | v30.0.0 Deneb (MAJOR)            | Gen-6 Platform Unification              | +72    |
| 971–980     | v30.1.0 Deneb-R                  | DirectStorage & GPU Decompression       | +72    |
| 981–990     | v30.2.0 Deneb-S                  | CLIP Semantic Search                    | +72    |
| 991–1000    | v30.3.0 Deneb-T                  | Live Preview Scrubber                   | +72    |
| 1001–1010   | v30.4.0 Deneb-U                  | Geospatial & Medical Formats            | +72    |
| 1011–1020   | v30.5.0 Deneb-V                  | Universal Format Decoder Library        | +72    |
| 1021–1030   | v30.6.0 Deneb-W                  | Plugin Marketplace v4                   | +72    |
| 1031–1040   | v30.7.0 Deneb-X                  | Enterprise Console v3                   | +72    |
| 1041–1050   | v31.0.0 Achernar (MAJOR)         | Generative AI Thumbnails                | +72    |
| 1051–1060   | v31.1.0 Achernar-R               | Cross-Platform Linux + macOS Shell      | +72    |
| **961–1060** | **Deneb through Achernar-R**    | **Gen-6 + Generative Era**             | **+720** |

**Total at end of Sprint 1060:** 3,509 + 720 = **4,229 unit tests**

---

## Architecture Pillars for Gen-6 + Generative Era (v30.x / v31.x)

### 1. Universal Platform Abstraction
The `PlatformAbstractionLayer` virtualises every OS-specific surface. New platforms are
added by implementing a single PAL backend interface — no changes to Engine internals.

### 2. Zero-CPU-Stall Pipeline
DirectStorage + GPU decompression ensures no CPU cycles are spent inflating file data.
The pipeline becomes fully asynchronous: DS → GPU decompress → GPU decode → GPU composite.

### 3. Semantic-First File Discovery
CLIP embeddings are a first-class citizen: every thumbnail generation event optionally
produces a 512-dimension embedding stored in the persistent `EmbeddingCacheStore`. Search
queries are answered by the HNSW index without re-encoding.

### 4. Generative Safety by Default
All generative AI features are disabled by default. Enterprise policy can permanently
lock them off. When enabled, every generated output passes through `ContentModerationFilter`
and is recorded in the `GenerativeAuditTrail` before being displayed.

### 5. Open Ecosystem via UFDL
The Universal Format Decoder Library ships as a standalone NuGet / vcpkg package under
the MIT license, enabling third-party integrators to build on the ExplorerLens decoder
stack without the full shell extension runtime.

---

## Performance Targets (Sprint 960 → Sprint 1060)

| Metric | v29.7.0 Baseline (Sprint 960) | v31.1.0 Target (Sprint 1060) |
|--------|-------------------------------|------------------------------|
| Cache-warm thumbnail P50 | 3 ms | **< 1 ms** |
| Cold thumbnail P50 (NVMe, DS) | ~80 ms | **< 20 ms** |
| CLIP embedding throughput | N/A | **≥ 30 img/s (NPU)** |
| Semantic search query latency (100K) | N/A | **< 10 ms** |
| Video keyframe first-show | N/A | **< 80 ms** |
| Generative thumbnail synthesis | N/A | **< 800 ms (NPU INT4)** |
| Memory footprint (idle) | 28 MB | **< 22 MB** |
| Supported platforms | Windows | **Win + macOS + Linux** |
| Supported formats | 200+ | **250+** |
| Plugin ecosystem | 120+ | **300+** |
| Test coverage | ≥ 90% | **≥ 95%** |
| Crash-free sessions | 99.99% | **99.999%** |

---

## Refactoring Priorities (Concurrent with Feature Work)

These improvements should be made incrementally across all 1000-series sprints:

### Code Quality
- **Migrate to C++23** — `std::expected<T,E>` for all decoder return types (replaces HRESULT)
- **Concepts for decoder interfaces** — replace `virtual` hot paths with concept-constrained templates
- **`[[nodiscard]]` audit** — enforce on all HRESULT / bool return paths
- **Module-ise Engine headers** — C++20 modules for the 10 hottest-path headers (<30 % compile speedup)
- **Remove deprecated BSTR usage** — replace `BSTR`/`OLESTR` with `std::wstring` everywhere

### Build System
- **CMake 4.3 Presets v7** — consolidate all 5 presets into a parameterised workflow
- **Unity builds** — `UNITY_BUILD ON` for Engine for 40% faster full-rebuild
- **LTO/PGO pipeline** — PGO profiling run as part of release build (expected +8% throughput)
- **Compiler-cache (sccache)** — add sccache support to Build-MSVC.ps1 and CI
- **ASAN/UBSAN CI gate** — dedicated sanitiser build step in every pull request workflow

### Documentation
- **API reference generation** — Doxygen → MkDocs integration (auto-published to GitHub Pages)
- **Format support matrix** — auto-generated from `LENSArchive.h` LENSTYPE enum at build time
- **Architecture Decision Records (ADRs)** — `docs/adr/` directory, first 10 ADRs documenting key choices
- **Video tutorial series** — 5 × 10-min screencasts covering install, dev, plugin dev, CLI, enterprise

### Testing
- **Property-based tests** — add `rapidcheck` for format decoder edge cases
- **Golden-file tests** — CI pixel-regression for 50 canonical test images per decoder
- **Mutation testing** — integrate `mutmut` / Stryker for C++ to track mutation score
- **Performance regression gate** — PR fails if P99 latency regresses by > 5%

### Security
- **Supply chain SBOM** — generate SBOM for all external libraries on every release
- **Fuzzing CI** — LibFuzzer campaigns for every decoder (30 min/PR, corpus committed)
- **Secret scanning** — CodeQL secret-scanning workflow on all branches
- **Dependency update bot** — Dependabot / Renovate for vcpkg and external libs
