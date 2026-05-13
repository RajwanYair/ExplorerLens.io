# ExplorerLens v35.x "Vega" — Streaming, Cloud & Real-Time Collaboration Roadmap

**Status:** Active planning
**Current Version:** v35.0.0 "Vega" (2026-04-10)
**Next Major:** v36.0.0 "Altair" (planned)
**Goal:** Make ExplorerLens the premier thumbnail engine for cloud-hosted,
collaboratively edited, and remotely accessed files.

---

## Executive Summary

v34.x "Arcturus" established format breadth and GPU-first decode performance.
v35.x "Vega" builds the **cloud-native layer** — streaming hydration, real-time
collaboration, zero-trust security, and cross-device sync — so ExplorerLens
works as well for a SharePoint library of 100000 files as it does for a local
NVMe drive.

**Three pillars of Vega:**

1. **Cloud-First Pipeline** — Progressive hydration, ETag invalidation, adaptive
   fidelity, and partial-decode resumption mean thumbnails appear instantly even
   when files are partially downloaded from the cloud.

2. **Real-Time Collaboration** — When multiple users have the same cloud folder
   open, thumbnail cache invalidation propagates immediately.  Collaborative-edit
   sessions never show stale thumbnails.

3. **Secure Remote Access** — Zero-trust session tokens, signed thumbnail manifests,
   and sandboxed remote-decode workers ensure enterprise deployments meet SOC 2
   requirements without sacrificing speed.

---

## Version Timeline

```
v35.0.0  Vega         Streaming & Cloud-Native Thumbnails ★★        (2026-04-10) ✅
v35.1.0  Vega-R       Real-Time Collaboration & Live Edit Sync       (2026-04-10) ✅
v35.2.0  Vega-S       Network-Aware Streaming Cache                  (2026-04-10) ✅
v35.3.0  Vega-T       Zero-Trust Thumbnail Security                  (2026-04-10) ✅
v35.4.0  Vega-U       WebAssembly / Browser Extension Pipeline       (2026-04-10) ✅
v35.5.0  Vega-V       Cross-Device Preview Sync                      (planned)
v35.6.0  Vega-W       REST API + Remote Decode Service               (planned)
v35.7.0  Vega-X       LTS Hardening + Cloud Perf Gate               (planned)
   │
   └── v36.0.0  Altair      AI-Powered Semantic Thumbnails ★★         (future)

★★ = Landmark MAJOR release
```

---

## Current Baseline (v35.0.0 "Vega")

| Metric | Value |
|--------|-------|
| File extensions supported | ~220+ |
| Unit tests | 4,674 |
| Single decode P50 (JPEG 6MP) | 4.2 ms |
| Batch throughput | 235 img/sec |
| Cache hit P50 | 0.8 ms |
| Idle memory | ~24 MB |
| Platforms | Windows + macOS (stub) + Linux (stub) |

---

## T1 — Streaming & Cloud-Native Thumbnails (v35.0.0 "Vega") ★★ ✅

**Status: DELIVERED**

| Module | Directory | Purpose |
|--------|-----------|---------|
| `MultiStageThumbnailEmitter` | `Engine/Pipeline/` | Progressive placeholder → low-res → full-res |
| `CloudHydrationMonitor` | `Engine/Core/` | Windows Cloud Files API hydration state probe |
| `PartialDecodeStateCache` | `Engine/Cache/` | Resume decode from cached mid-state blobs |
| `ThumbnailETagValidator` | `Engine/Core/` | Cloud ETag + mtime cache invalidation |
| `AdaptiveFidelitySelector` | `Engine/Pipeline/` | Bandwidth/GPU/budget-driven fidelity selection |

---

## T2 — Real-Time Collaboration & Live Edit Sync (v35.1.0 "Vega-R")

**Theme:** Multiple users have the same cloud folder open.  When one user saves
a file, every other user's thumbnail cache invalidates automatically and the new
thumbnail appears within one polling cycle.

### Features

| Feature | Description |
|---------|-------------|
| **Live-Sync Session Tokens** | Per-user, per-file edit session tokens; tokens carry an opaque version vector |
| **Collaborative Cache Coordinator** | Broadcast cache-invalidation signals to all active sessions sharing a cloud folder |
| **Thumbnail Delta Encoder** | Encode only the changed pixel regions between two thumbnail versions (LZMA delta) |
| **Conflict Resolution Engine** | When two editors save concurrently, pick the lexicographically latest ETag or prompt the shell |
| **Real-Time Preview Pipeline** | Debounced, back-pressure-aware subscriber pipeline that pushes fresh thumbnails to Explorer |

### Architecture

```
Cloud Storage (OneDrive / SharePoint / S3)
  │  [File change notification — webhook / polling]
  ▼
LiveSyncTokenManager — issues/validates per-session version vectors
  │
CollaborativeCacheCoordinator — fan-out cache invalidation to all open sessions
  │
CloudHydrationMonitor (v35.0) — gate decode until bytes available
  │
PartialDecodeStateCache (v35.0) — skip preamble if header already parsed
  │
ThumbnailETagValidator (v35.0) — confirm new ETag before re-decoding
  │
ThumbnailDeltaEncoder — if only small region changed, encode delta only
  │
ConflictResolutionEngine — arbitrate concurrent edits
  │
RealTimePreviewPipeline — debounce + back-pressure → IThumbnailProvider
```

### Modules

| Header | Source | Directory | Purpose |
|--------|--------|-----------|---------|
| `LiveSyncTokenManager.h` | `.cpp` | `Engine/Core/` | Session token lifecycle: issue, refresh, expire, validate |
| `CollaborativeCacheCoordinator.h` | `.cpp` | `Engine/Cache/` | Fan-out cache invalidation across concurrent sessions |
| `ThumbnailDeltaEncoder.h` | `.cpp` | `Engine/Pipeline/` | LZMA-delta encoder for thumbnail patch payloads |
| `ConflictResolutionEngine.h` | `.cpp` | `Engine/Core/` | Concurrent-edit conflict arbitration with audit log |
| `RealTimePreviewPipeline.h` | `.cpp` | `Engine/Pipeline/` | Debounced subscriber pipeline with back-pressure |

### Tests (Sprint 1291-1300)

| Test | What It Verifies |
|------|-----------------|
| `TestLiveSyncTokenManager_Issue` | Token issue, validate, round-trip |
| `TestLiveSyncTokenManager_Expire` | Expired token rejected; TTL enforcement |
| `TestCollaborativeCacheCoordinator_Invalidate` | Invalidation propagates to all registered sessions |
| `TestCollaborativeCacheCoordinator_Sync` | Session registration / deregistration; no dangling refs |
| `TestThumbnailDeltaEncoder_Encode` | Delta encode produces non-empty payload for changed pixels |
| `TestThumbnailDeltaEncoder_Decode` | Round-trip encode→decode restores pixel region exactly |
| `TestConflictResolutionEngine_Merge` | Merge picks highest-version ETag |
| `TestConflictResolutionEngine_PickLatest` | PickLatest selects the largest mtime |
| `TestRealTimePreviewPipeline_Subscribe` | Subscribe + notify fires callback on file change event |
| `TestRealTimePreviewPipeline_Backpressure` | Flood with 1000 events; only last N coalesced events fire |

---

## T3 — Network-Aware Streaming Cache (v35.2.0 "Vega-S")

**Theme:** Cache tiers adapt to the current network topology.  On a fast LAN the
engine pre-fetches aggressively; on a metered mobile connection it conserves bandwidth
and prefers placeholder thumbnails.

### Modules (Sprint 1301-1310)

| Header | Directory | Purpose |
|--------|-----------|---------|
| `NetworkTopologyProbe.h` | `Engine/Core/` | Detect LAN/WiFi/Cell/VPN topology in real-time |
| `StreamingCacheTierPolicy.h` | `Engine/Cache/` | Map topology → cache tier budget + eviction aggressiveness |
| `BandwidthThrottleGuard.h` | `Engine/Pipeline/` | Token-bucket rate limiter for remote thumbnail fetches |
| `RemoteFileManifestCache.h` | `Engine/Cache/` | Cache remote directory manifests to avoid repeat LIST calls |
| `CachePrefetchScheduler.h` | `Engine/Cache/` | Priority-ordered pre-fetch queue with topology-aware throttle |

---

## T4 — Zero-Trust Thumbnail Security (v35.3.0 "Vega-T")

**Theme:** SOC 2 / FIPS-compliant thumbnail pipeline for enterprise tenants.
Thumbnail manifests are signed; decode workers run in isolated processes with
no network access.

### Modules (Sprint 1311-1320)

| Header | Directory | Purpose |
|--------|-----------|---------|
| `ThumbnailManifestSigner.h` | `Engine/Core/` | ECDSA-P256 signature over thumbnail hash + path + mtime |
| `ZeroTrustDecodeWorker.h` | `Engine/Pipeline/` | Sandboxed process with pledge/seccomp for decode isolation |
| `TokenBoundCacheEntry.h` | `Engine/Cache/` | Cache entries bound to a user/tenant token; no cross-tenant reads |
| `ThumbnailAuditLog.h` | `Engine/Core/` | Structured audit trail of every decode/cache hit for SIEM ingestion |
| `FIPSCryptoAdapter.h` | `Engine/Core/` | Route all crypto to CNG (BCryptProvider) for FIPS 140-2 compliance |

---

## T5 — WebAssembly / Browser Extension Pipeline (v35.4.0 "Vega-U")

**Status: DELIVERED**

| Module | Directory | Purpose |
|--------|-----------|---------|
| `WasmDecoderShim.h` | `Engine/Core/` | Thin C ABI shim for WASM-compilable decoder entry points |
| `BrowserThumbnailBridge.h` | `Engine/Pipeline/` | JS↔native message bridge for async thumbnail delivery |
| `OffscreenCanvasRenderer.h` | `Engine/Pipeline/` | Render thumbnail into OffscreenCanvas (WebGL2) |
| `WasmCacheAdapter.h` | `Engine/Cache/` | Map `PartialDecodeStateCache` API to IndexedDB storage |
| `ProgressiveThumbnailStream.h` | `Engine/Pipeline/` | Server-Sent Events stream of progressive thumbnail frames |

---

## T6 — Cross-Device Preview Sync (v35.5.0 "Vega-V")

**Theme:** Thumbnails generated on one device (desktop) are transparently available
on other devices (phone, tablet) via the cloud cache manifest.

### Modules (Sprint 1331-1340)

| Header | Directory | Purpose |
|--------|-----------|---------|
| `DeviceSyncManifest.h` | `Engine/Core/` | Portable manifest of cached thumbnails (path hash, ETag, dimensions) |
| `CrossDeviceCacheSync.h` | `Engine/Cache/` | Upload/download thumbnail manifests via cloud storage provider |
| `ThumbnailPackFile.h` | `Engine/Pipeline/` | Compact `.tlpk` format: multiple thumbnails bundled with zstd |
| `SyncConflictResolver.h` | `Engine/Core/` | Merge thumb manifests from two devices with diverged caches |
| `DeviceCapabilityAdvertiser.h` | `Engine/Core/` | Publish local GPU/CPU capabilities so peer can offload decode |

---

## T7 — REST API & Remote Decode Service (v35.6.0 "Vega-W")

**Theme:** `lens-server` HTTP service exposes thumbnail generation as a REST API
for CI/CD pipelines, design tools, and enterprise DAM systems.

### Modules (Sprint 1341-1350)

| Header | Directory | Purpose |
|--------|-----------|---------|
| `RemoteDecodeServer.h` | `Engine/Core/` | Lightweight HTTP/2 server front-end (winsock2 + nghttp2) |
| `ThumbnailRequestRouter.h` | `Engine/Pipeline/` | Route REST decode requests to the appropriate decoder |
| `APIRateLimiter.h` | `Engine/Core/` | Per-IP + per-tenant token-bucket rate limiter |
| `RemoteResultSerializer.h` | `Engine/Pipeline/` | Serialize thumbnail to WebP/AVIF/base64 JSON response |
| `ServerHealthMonitor.h` | `Engine/Core/` | Expose `/health` + `/metrics` endpoints (Prometheus format) |

---

## T8 — LTS Hardening + Cloud Perf Gate (v35.7.0 "Vega-X")

**Theme:** Lock down the Vega series with an LTS performance gate that blocks any
regression in cloud-native thumbnail throughput before the next major version.

### Modules (Sprint 1351-1360)

| Header | Directory | Purpose |
|--------|-----------|---------|
| `CloudPerfRegressionGate.h` | `Engine/Core/` | KPI gate for cloud metrics (hydration time, delta encode speed, sync latency) |
| `VegaLTSValidator.h` | `Engine/Core/` | Build-time freeze validator for v35 LTS branch |
| `CollaborativeLoadTester.h` | `Engine/Cache/` | Simulate N-user concurrent cache-invalidation storm |
| `StreamingBenchmarkBaseline.h` | `Engine/Utils/` | JSON baseline for all Vega-series streaming KPIs |
| `CloudIntegrationTestHarness.h` | `Engine/Tests/` | End-to-end harness mocking cloud storage + file notifications |

---

## v36.0.0 "Altair" Preview — AI-Powered Semantic Thumbnails ★★

**Theme:** On-device AI models understand file content and generate semantically
rich thumbnail previews — showing the most visually interesting region of a photo,
the key slide of a deck, or the dominant object in a 3D scene.

### Pillars

1. **Smart Crop 2.0** — YOLO-based saliency detection; crop to the face/object
   the user cares about, not a dumb center crop.

2. **Content-Aware Style** — For code/text files, pick font, color scheme, and
   layout based on content type (Rust source ≠ SQL script).

3. **3D Scene Understanding** — For `.usdz`/`.glb`/`.fbx`, find the "hero shot"
   camera angle automatically via neural rendering.

4. **Semantic Search Integration** — CLIP-based embedding index lets Explorer
   search "photos with blue sky" or "CAD files with gears" via thumbnail embeddings.

5. **Generative Thumbnail Upgrade** — For low-res or zero-byte placeholder files,
   a Stable Diffusion / SDXL-Turbo model generates a plausible preview from the
   filename + metadata.
