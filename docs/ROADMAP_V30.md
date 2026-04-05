# ExplorerLens v30.x "Deneb" — Gen-6 Roadmap

**Status:** Historical — v30.x through v32.x completed; v33.x planning active
**When written:** v25.2.0 "Rigel-S" was current active version
**Current Active Version:** v32.4.0 "Fomalhaut-U" (2026-04-05)
**Next Major:** v33.0.0 "Spica" (see [v33.x Forward Plan](#v33x-spica--forward-plan) below)

---

## v33.x "Spica" — Forward Plan

> New major series starting after v32.x "Fomalhaut" completes.

```
v32.4.0  Fomalhaut-U  ── Release infra: 5-registry packages, manifest sync  (2026-04-05)
v32.5.0  Fomalhaut-V  ── DirectStorage 1.2 zero-copy GPU decompress path     (2026-04-05)
v32.6.0  Fomalhaut-W  ── CLIP semantic search + HNSW index (NPU INT8)        (2026-04-05)
v32.6.1  Fomalhaut-W  ── Patch: clang-tidy fixes + missing test includes      (2026-04-05)
v32.7.0  Fomalhaut-X  ── Live Preview Scrubber (video seek + frame extraction)(planned)
   │
   ├── v33.0.0  Spica        Cross-Platform Shell (macOS Quick Look + Linux) ★★ (planned)
   ├── v33.1.0  Spica-R      Platform Abstraction Layer — Metal + Vulkan EGL   (planned)
   ├── v33.2.0  Spica-S      Enterprise Console v4 + GPO Policy Templates      (planned)
   ├── v33.3.0  Spica-T      Generative AI Thumbnails (on-device NPU)          (planned)
   ├── v33.4.0  Spica-U      Plugin Marketplace v5 + SDK Compat Kit v3         (planned)
   └── v33.5.0  Spica-V      LTS Hardening + Security Audit                   (planned)

★★ = Landmark MAJOR release
```

### v32.5.0 "Fomalhaut-V" — DirectStorage Zero-Copy

**Theme:** Eliminate CPU-side decompression bottleneck for large format files.

- DirectStorage 1.2 streams to GPU staging buffer directly from NVMe
- GDeflate (NVIDIA) + ZStd compute kernel (Intel/AMD) — GPU decompression in ~8 ms
- P99 target: 340 ms (current large RAW) → 55 ms (6×)

### v33.0.0 "Spica" — Cross-Platform Shell ★★

**Theme:** ExplorerLens beyond Windows — macOS Quick Look + Linux Nautilus/Dolphin.

| Platform | Shell Provider | GPU Backend |
|----------|---------------|-------------|
| Windows 11 | IThumbnailProvider / COM | D3D12 |
| macOS Sequoia | QLThumbnailRequest | Metal v2 |
| Linux (GTK4) | Nautilus / Dolphin / Tumbler | Vulkan (EGL) |

Engine internals unchanged across platforms. Platform Abstraction Layer (PAL) isolates
GPU surface creation, shell-provider registration, and file-system notifications.

---

## v30.x–v32.x Completion Summary

| Series | Theme | Status | Key Deliverables |
|--------|-------|--------|-----------------|
| v30.0–v30.7 | Platform Unification (Deneb) | Completed | PAL scaffolding, D3D12 renderer |
| v31.0–v31.7 | Generative AI (Achernar) | Completed | AI scene understanding, NPU routing |
| v32.0–v32.3 | Performance + CI (Fomalhaut-T) | Completed | Sub-ms cache, 5-registry publish, EngineTests split |

---

## Original Gen-6 Vision (written at v25.2.0)

---

## Executive Summary

ExplorerLens v30.x "Deneb" represents the **sixth major generation** of the shell extension.
After the Capella series (v29.x) establishes Gen-5 with WinUI 4, async preview brokering,
OTLP observability, and LTS hardening, the Deneb era tackles the three transformative shifts
defining premium professional workflows in 2027–2029:

**1. Platform Unification** — Windows-only is over. v30.0.0 ships simultaneous native shell
integration on macOS (Quick Look) and Linux (Nautilus/Dolphin/Wayland), backed by a
platform-abstraction layer (PAL) that keeps Engine internals unchanged.

**2. Zero-Latency Preview** — DirectStorage 1.2 + GPU decompression eliminates every CPU
stall on the decode path. Paired with CLIP-powered predictive pre-generation, thumbnails
are on-screen before user focus arrives — sub-1 ms for warm cache, < 20 ms cold on NVMe.

**3. The Generative Era** — v31.0.0 "Achernar" (the first major in this plan's Achernar
continuation) adds on-device text-to-thumbnail synthesis, AI inpainting for damaged files,
and a vision-LLM alt-text synthesiser — all running fully offline on Intel/Qualcomm NPUs.

---

## Version Timeline

```
v29.7.0  Capella-X  ── LTS candidate (Sprint 951-960)
   │
   ├── v30.0.0  Deneb       Platform Unification ★★    (Sprint 961–970)   ◄ THIS ROADMAP
   ├── v30.1.0  Deneb-R     DirectStorage + GPU Decomp  (Sprint 971–980)
   ├── v30.2.0  Deneb-S     CLIP Semantic Search          (Sprint 981–990)
   ├── v30.3.0  Deneb-T     Live Preview Scrubber         (Sprint 991–1000)
   ├── v30.4.0  Deneb-U     Geospatial & Medical Formats  (Sprint 1001–1010)
   ├── v30.5.0  Deneb-V     Universal Decoder Library     (Sprint 1011–1020)
   ├── v30.6.0  Deneb-W     Plugin Marketplace v4         (Sprint 1021–1030)
   ├── v30.7.0  Deneb-X     Enterprise Console v3         (Sprint 1031–1040)
   │
   ├── v31.0.0  Achernar    Generative AI Thumbnails ★★  (Sprint 1041–1050)
   └── v31.1.0  Achernar-R  Cross-Platform Shell          (Sprint 1051–1060)

★★ = Landmark MAJOR release
```

---

## Completed Foundation (v25.x – v29.x)

| Series | Codename | Theme | Status |
|--------|----------|-------|--------|
| v25.0.0–v25.7.0 | Rigel | WASM plugins, NPU compute, VCS overlays, protocol APIs | Active |
| v26.0.0–v26.7.0 | Canopus | Post-quantum crypto, immersive 3D, real-time collab | Planned |
| v27.0.0–v27.7.0 | Sirius | Federated AI, streaming media, distributed rendering | Planned |
| v28.0.0–v28.7.0 | Polaris | Electron shell, generative captions, AR preview, enterprise 2.0 | Planned |
| v29.0.0–v29.7.0 | Capella | Gen-5 WinUI 4, edge AI, spatial computing, OTLP, LTS | Planned |

---

## Gen-6 Theme Deep Dives

### T1 — Platform Unification (v30.0.0 "Deneb") ★★

**Problem:** 34% of professional users work across Windows + macOS. Linux is the primary
development OS for 60% of backend engineers who manage asset libraries. ExplorerLens
having zero presence on non-Windows platforms is a major adoption barrier.

**Solution:** Platform Abstraction Layer (PAL) decouples GPU surface creation, file-system
change notifications, and shell-provider registration from Engine internals. Each platform
implements a thin PAL backend:

| Platform | Shell Provider | GPU Backend | FS Notifications |
|----------|--------------|-------------|-----------------|
| Windows 11 | IThumbnailProvider / COM | D3D12 | ReadDirectoryChanges |
| macOS Sequoia | QLThumbnailRequest | Metal v2 | FSEvents |
| Linux (GTK4) | Nautilus / Dolphin / Tumbler | Vulkan (EGL) | inotify / fanotify |

The Engine itself — all 200+ decoders, AI pipeline, cache, memory management — runs
unchanged on all platforms. Only the PAL backend is platform-specific.

**Key invariant:** Same thumbnail output (SSIM ≥ 0.99) on all three platforms.

---

### T2 — DirectStorage & Zero-Latency Pipeline (v30.1.0 "Deneb-R")

**Problem:** The P99 thumbnail latency for a 50 MB camera RAW file is currently 340 ms,
dominated by CPU inflate (zlib/LZ4/Zstd). This single bottleneck caps throughput at
~3 large files/sec even on a 32-core workstation.

**Solution:** DirectStorage 1.2 streams compressed file data directly to the GPU staging
buffer. GPU compute kernels (GDeflate for NVIDIA, ZStd kernel for Intel/AMD) decompress
in hardware in ~8 ms. The CPU is freed for decode metadata and thumbnail compositing.

```
Old: Disk → OS Cache → CPU malloc → CPU inflate → CPU decode → GPU upload → Composite
New: Disk ──DirectStorage──► GPU staging buffer → GPU decompress → GPU decode → Composite
```

**Expected improvement:** P99 large-file latency: 340 ms → 55 ms (6.2×).

---

### T3 — CLIP Semantic Search (v30.2.0 "Deneb-S")

**Problem:** Users spend 62% of search time on filename-based guesswork. "sunset-final-v3
-COPY.jpg" is indistinguishable from "DSC_00421.jpg" without manually opening files.

**Solution:** CLIP ViT-B/32 (INT8 quantised, 18 ms/image on Intel NPU) embeds every
thumbnail into a 512-dimension semantic vector. An HNSW index answers "sunset over water"
queries in < 10 ms for 100,000-file corpora. All computation is on-device — zero privacy
footprint, zero network requirement.

**Usage flow:** User types query in Explorer search → ExplorerLens intercepts → CLIP text
embed → HNSW query → top-K results highlighted with similarity overlay.

---

### T4 — Live Preview Scrubber (v30.3.0 "Deneb-T")

**Problem:** Static thumbnails carry zero temporal/structural information for videos,
audio, documents, and code files. A 2-hour movie and a 5-second clip look identical.

**Solution:** Mouse-hover triggers lazy progressive decode:
- **Video:** Keyframes extracted at scrub positions (1, 25, 50, 75, 100% elapsed)
- **Audio:** PCM peak analysis → waveform bitmap rendered at 256 px width
- **Documents:** Page-flip animation (PDF, DOCX, PPTX pages 1–5)
- **Code:** Syntax-highlighted first 40 lines with font antialiasing
- **Fonts:** Pangram in the requested face, all available weights

No compute at rest — hover events drive progressive prefetch with priority boost.

---

### T5 — Universal Format Decoder Library (v30.5.0 "Deneb-V")

**Problem:** ExplorerLens's decoder expertise is locked inside the COM shell extension.
Developers building custom viewers, CI/CD thumbnail pipelines, or cloud processors have
no path to reuse these decoders without the full Windows shell registration ceremony.

**Solution:** Package the decoder stack as a standalone `extern "C"` C ABI library
(NuGet + vcpkg, MIT license). Any application gains 200+ format thumbnail generation with
a single `#include "ufdl.h"` and a 3-line init call. Python/Node.js/Rust bindings ship
in the first release. This creates a multiplier effect: every UFDL integrator is a vector
for ExplorerLens decoder improvements flowing back to the wider ecosystem.

---

### T6 — Generative AI (v31.0.0 "Achernar") ★★

**Problem:** Corrupt files, missing thumbnails, and conceptual/placeholder assets have
no visual representation. Users are forced to open the file to identify it — eliminating
the purpose of thumbnails entirely.

**Solution:** On-device generative fallback using SD-Turbo / FLUX.1-schnell (INT4
quantised, 12 ms/step on Intel NPU). When a decoder fails or returns a blank result,
the generative engine synthesises a photorealistic placeholder using filename + metadata
as the text prompt. A `ContentModerationFilter` gates every generated image before
display. A W3C-provenance `GenerativeAuditTrail` records every synthesis event.

**Enterprise policy:** All generative features are off-by-default. GPO locks them off
in regulated environments. This is a hard invariant — not a soft default.

---

## Non-Functional Targets

### Performance

| Metric | v29.7.0 (Sprint 960) | v30.7.0 (Sprint 1040) | v31.1.0 (Sprint 1060) |
|--------|---------------------|----------------------|----------------------|
| Cache-warm P50 | < 3 ms | **< 1.5 ms** | **< 1 ms** |
| Cold NVMe P50 (DS) | ~80 ms | **< 25 ms** | **< 20 ms** |
| CLIP embed throughput | — | **≥ 25 img/s** | **≥ 30 img/s** |
| Semantic search (100K) | — | **< 15 ms** | **< 10 ms** |
| Generative synthesis | — | — | **< 800 ms** |
| Idle memory footprint | 28 MB | **< 24 MB** | **< 22 MB** |
| Peak memory (100 concurrent) | 180 MB | **< 140 MB** | **< 120 MB** |

### Reliability

| Metric | Target |
|--------|--------|
| Crash-free sessions | **≥ 99.999%** |
| Test coverage | **≥ 95%** |
| P99 latency regression gate | **< 5% per PR** |
| Fuzz corpus coverage | **≥ 85% branch on all decoders** |
| Build reproducibility | **100% deterministic** |

### Compatibility

| Requirement | Commitment |
|-------------|------------|
| Windows 10 22H2+ | Full support (no WinUI 4 features) |
| Windows 11 23H2+ | Full feature set |
| macOS Sequoia 15+ | Full Quick Look + Finder integration |
| Ubuntu 24.04 LTS | Full Nautilus + Dolphin + Wayland |
| ARM64 (Snapdragon X Elite) | Native NEON/SVE2 decode paths |
| Apple Silicon (M4+) | Metal v2 GPU pipeline + ANE inference |

---

## Milestones & Checkpoints

| Milestone | Version | Target | Gate |
|-----------|---------|--------|------|
| PAL Alpha | v30.0.0 | Sprint 970 | macOS CI smoke test passes |
| DS Integration | v30.1.0 | Sprint 980 | P99 RAW < 80 ms on CI NVMe mock |
| CLIP Alpha | v30.2.0 | Sprint 990 | Search recall@10 ≥ 0.90 |
| Scrubber GA | v30.3.0 | Sprint 1000 | Video scrub < 100 ms first frame |
| UFDL Public Beta | v30.5.0 | Sprint 1020 | Python binding works on PyPI |
| Marketplace v4 GA | v30.6.0 | Sprint 1030 | 10 community plugins in marketplace |
| Generative Alpha | v31.0.0 | Sprint 1050 | INT4 inference < 1 s on Intel NPU |
| Linux GA | v31.1.0 | Sprint 1060 | Nautilus parity SSIM ≥ 0.99 |

---

## Dependencies & Pre-requisites

| Dependency | Required By | Notes |
|------------|------------|-------|
| DirectStorage SDK 1.2 | v30.1.0 | Windows 11 22H2+ SDK |
| ONNX Runtime 1.20+ (DirectML EP) | v30.2.0 (CLIP) | vcpkg or bundled |
| LevelDB 1.23+ | v30.2.0 (embedding store) | bundled build |
| HNSW (hnswlib 0.8) | v30.2.0 | header-only, bundled |
| FFmpeg 7.x (keyframe) | v30.3.0 | static link, LGPL |
| GDAL 3.9+ (GeoTIFF) | v30.4.0 | optional, autodetected |
| libdicom 2.0+ | v30.4.0 | bundled static |
| cfitsio 4.4 | v30.4.0 (FITS) | bundled static |
| libtorch/ONNX INT4 | v31.0.0 | SD-Turbo / FLUX inference |
| GIO 2.80+ | v31.1.0 (Nautilus) | Linux only |
| KIO 6.0+ | v31.1.0 (Dolphin) | Linux only |

---

## Open Questions & Risks

| # | Risk | Likelihood | Mitigation |
|---|------|-----------|------------|
| 1 | DirectStorage unavailable on CI runners | High | Mock DS layer with identical API + CPU fallback |
| 2 | CLIP INT8 model quality degradation | Medium | Benchmark against FP32 reference; dual-path fallback |
| 3 | macOS sandboxing blocks decoder write paths | Medium | Use App Group shared container; pre-validate entitlements |
| 4 | GDAL/FITS license compliance in closed distribution | Low | Verify LGPL compliance; offer as separate opt-in package |
| 5 | Generative AI regulatory risk (EU AI Act 2026) | Medium | Enterprise default-off + audit trail satisfies GPAI obligations |
| 6 | Linux D-Bus thumbnailer latency under Wayland | Low | Benchmark against X11; portal fallback if > 200 ms |

---

## Relationship to Existing Plans

| Plan | Versions | Sprints | Relationship |
|------|----------|---------|--------------|
| [SPRINT_PLAN_600.md](SPRINT_PLAN_600.md) | v25.0–v26.1 | 561–660 | **Active execution** |
| [SPRINT_PLAN_700.md](SPRINT_PLAN_700.md) | v26.2–v27.3 | 661–760 | Planned (Canopus/Sirius) |
| [SPRINT_PLAN_800.md](SPRINT_PLAN_800.md) | v27.4–v28.5 | 761–860 | Planned (Sirius/Polaris) |
| SPRINT_PLAN_900 | v28.6–v29.7 | 861–960 | Historical (archived) |
| SPRINT_PLAN_1000 | v30.0–v31.1 | 961–1060 | Historical (archived) |
