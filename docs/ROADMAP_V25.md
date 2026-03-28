# ExplorerLens v25.x "Rigel" — Development Roadmap

**Status:** Planning  
**Current Version:** v23.5.0 "Vega-V"  
**Next Major Version:** v25.0.0 "Rigel"  
**Previous Roadmap:** [ROADMAP_V16.md](ROADMAP_V16.md) *(archived — covers v15.x/v16.x work now completed)*  
**Sprint Detail:** [SPRINT_PLAN_600.md](SPRINT_PLAN_600.md) — Sprints 561–660

---

## Executive Summary

ExplorerLens v25.x "Rigel" represents the **third major evolution** of the shell extension,
following the architectural foundations of v22.x "Sirius" (enterprise & testing maturity),
v23.x "Vega" (reactive pipeline + GPU v3 + AI-native thumbnailing), and v24.x "Altair"
(AI thumbnailing v2 + cross-process architecture + cloud integration + security hardening).

The Rigel series targets three overarching goals:

1. **Open the plugin ecosystem** — WebAssembly plugin sandbox removes DLL trust barriers,
   allowing any developer to ship a decoder as a `.wasm` bundle with full memory safety.
2. **Expand compute heterogeneity** — Route AI inference to NPUs (Intel, Qualcomm), improve
   ARM64 native paths, and power-optimize across battery/plugged scenarios.
3. **Integrate with the developer workflow** — VCS overlays surface Git/Perforce state
   directly in thumbnails; the new gRPC/REST/GraphQL protocol surface lets CI/CD pipelines
   call the thumbnail engine programmatically without COM.

The **Canopus** major release (v26.0.0) closes the series with post-quantum cryptography —
future-proofing plugin signing and IPC channels ahead of NIST FIPS 203/205 mandates.

---

## Version Timeline

```
v23.5.0  Vega-V    ── NOW (released)
   │
   ├── v23.6.0  Vega-W     Security Hardening v2        (Sprint 461–470)
   ├── v23.7.0  Vega-X     Format Expansion V            (Sprint 471–480)
   │
   ├── v24.0.0  Altair     AI-Native Thumbnailing v2 ★  (Sprint 481–490)
   ├── v24.1.0  Altair-R   Cross-Process Architecture    (Sprint 491–500)
   ├── v24.2.0  Altair-S   Cloud Integration v2          (Sprint 501–510)
   ├── v24.3.0  Altair-T   Enterprise Policy v3          (Sprint 511–520)
   ├── v24.4.0  Altair-U   Performance Profiling v2      (Sprint 521–530)
   ├── v24.5.0  Altair-V   Accessibility & HiDPI v2      (Sprint 531–540)
   ├── v24.6.0  Altair-W   Store & Distribution           (Sprint 541–550)
   ├── v24.7.0  Altair-X   Developer Experience v2       (Sprint 551–560)
   │
   ├── v25.0.0  Rigel      WASM Plugin Sandbox ★★       (Sprint 561–570)  ◄ THIS ROADMAP
   ├── v25.1.0  Rigel-R    Neural Format Intelligence    (Sprint 571–580)
   ├── v25.2.0  Rigel-S    NPU & Heterogeneous Compute   (Sprint 581–590)
   ├── v25.3.0  Rigel-T    VCS & Source Control          (Sprint 591–600)
   ├── v25.4.0  Rigel-U    Self-Healing Recovery         (Sprint 601–610)
   ├── v25.5.0  Rigel-V    Multi-Instance & VDesktop     (Sprint 611–620)
   ├── v25.6.0  Rigel-W    Collaborative Annotations     (Sprint 621–630)
   ├── v25.7.0  Rigel-X    Protocol Surface & APIs       (Sprint 631–640)
   │
   ├── v26.0.0  Canopus    Post-Quantum Security ★★     (Sprint 641–650)
   └── v26.1.0  Canopus-R  Windows Next-Gen Shell        (Sprint 651–660)

★  = Major version (MAJOR theme block)
★★ = Landmark MAJOR release
```

---

## Completed Foundation (v22.x – v24.x)

The Altair series will complete these capabilities before Rigel begins:

| Capability | Completed In | Notes |
|------------|-------------|-------|
| Reactive pipeline (CQRS/Saga/event store) | v23.0.0 Vega | ✅ Done |
| GPU v3 (CUDA/HIP/multi-GPU/DMA) | v23.1.0 Vega-R | ✅ Done |
| Plugin ecosystem v3 (DI/A-B/canary) | v23.2.0 Vega-S | ✅ Done |
| Memory optimization v3 (huge pages/NVMe) | v23.3.0 Vega-T | ✅ Done |
| Smart Cache v4 (AI eviction/federated) | v23.4.0 Vega-U | ✅ Done |
| CLI & Automation v2 (watch/diff/profile) | v23.5.0 Vega-V | ✅ Done |
| Security hardening v2 (ZTP/CFG/AES-IPC) | v23.6.0 Vega-W | Planned |
| Format Expansion V (ICNS/CUR/FLIF) | v23.7.0 Vega-X | Planned |
| AI-native thumbnailing v2 (ESRGAN/CLIP) | v24.0.0 Altair | Planned |
| Cross-process COM server + named-pipe hub | v24.1.0 Altair-R | Planned |
| Cloud backend (Azure/S3/Redis/CosmosDB) | v24.2.0 Altair-S | Planned |
| Enterprise policy v3 (FIPS/GDPR/SIEM) | v24.3.0 Altair-T | Planned |
| Performance profiling v2 (PMU/flame graphs) | v24.4.0 Altair-U | Planned |
| Accessibility v2 (alt-text AI/WCAG 2.2) | v24.5.0 Altair-V | Planned |
| Store & distribution (MSIX/WinGet/delta) | v24.6.0 Altair-W | Planned |
| Developer experience v2 (REST playground) | v24.7.0 Altair-X | Planned |

---

## Rigel Series — Theme Deep Dives

### T1 — WebAssembly Plugin Sandbox (v25.0.0 "Rigel")

**Problem:** Native DLL plugins require trust chains, code-signing infrastructure, and
carry the risk of host-process compromise if a plugin misbehaves. The current plugin SDK
requires developers to build against specific MSVC toolchains.

**Solution:** Introduce a WASM runtime adapter (targeting WasmEdge 0.14+) that loads
`.wasm` plugin bundles, maps the IThumbnailPlugin WIT interface, and runs entirely within
the WASM linear-memory sandbox. The host engine bridges zero-copy pixel buffers via
shared memory without copying through the WASM boundary.

**Key deliverables:**
- `WASMRuntimeAdapter.h` — WasmEdge host + WASI implementation
- `WITBindingGenerator.h` — Compile-time WIT → C++ host-side glue
- `WASMHotSwapEngine.h` — Live plugin reload without process restart
- `WASMDebuggerBridge.h` — DAP-compatible debugger for WASM plugins

**Impact:** Any developer can write a decoder in Rust, Go, C, or AssemblyScript and
distribute it as a `.wasmplug` bundle — no MSVC, no code-signing certificate required for
sandboxed operation.

---

### T2 — Neural Format Intelligence (v25.1.0 "Rigel-R")

**Problem:** ExplorerLens currently fails silently on unrecognized formats, showing a blank
thumbnail. With 200+ decoders and thousands of vendor-specific variants, keeping the
decoder registry up to date manually is unsustainable.

**Solution:** Train a binary-classification model on the full format corpus, using the
first 4 KB of each file as input features. When a file triggers no registered decoder,
the neural fingerprinter produces a confidence-scored format guess and optionally generates
a synthetic thumbnail stub or triggers a plugin search.

**Key deliverables:**
- `NeuralFormatFingerprinter.h` — MobileNetV3 on byte-histogram + magic-byte features
- `SelfExpandingFormatRegistry.h` — Persists newly learned formats to user profile
- `LLMMIMEInferenceEngine.h` — LLM call (local GGUF model) for complex header analysis
- `FormatEvolutionTracker.h` — Detects when a "known" format's binary signature drifts

---

### T3 — NPU & Heterogeneous Compute (v25.2.0 "Rigel-S")

**Problem:** AI decode acceleration routes only to GPU (DirectML/CUDA/HIP). Modern
client PCs (Intel Meteor Lake+, Snapdragon X Elite, Qualcomm 8cx Gen 3) have dedicated
NPUs with 10–45 TOPS that sit idle during thumbnail generation.

**Solution:** Expand the GPU decode router to include NPU execution providers via the ONNX
Runtime abstraction. A power-aware scheduler checks battery state and thermal headroom
before routing to the highest-TOPS available accelerator.

**Key deliverables:**
- `IntelNPUBackend.h` — OpenVINO 2025.x EP integration
- `HexagonDSPBackend.h` — Qualcomm ONNX QNN EP integration
- `ONNXEPRouter.h` — Unified EP selection: NPU → GPU → CPU
- `PowerAwareScheduler.h` — Power plan + thermal → compute tier selection
- `ARM64DecodeBackend.h` — Native ARM64 NEON SIMD paths for JPEG/WebP/AVIF

**Performance target:** AI inference on Meteor Lake NPU: 1.5ms (vs 4ms DirectML GPU).

---

### T4 — Source Control & VCS Integration (v25.3.0 "Rigel-T")

**Problem:** Developers browsing source trees in Explorer have no visual indication of
which files are modified, staged, in conflict, or stored in Git LFS. They must context-switch
to a terminal or IDE to see VCS state.

**Solution:** Integrate with the Git object database (libgit2 or git-status cached) and
Perforce/SVN status files to overlay real-time VCS state badges on thumbnails. Branch-aware
cache keys ensure thumbnail validity is scoped to branch context.

**Key deliverables:**
- `GitStatusOverlay.h` — libgit2-based status query + badge compositor
- `BranchAwareCacheKey.h` — Cache key includes repo root + branch + HEAD commit
- `GitDiffThumbnail.h` — Before/after split view for image diffs in Git
- `GitLFSResolver.h` — Resolve LFS pointers to real assets for preview
- `VCSBadgeAdapter.h` — Adapter for Perforce/SVN via `p4 fstat` / `svn status`

---

### T5 — Self-Healing & Adaptive Recovery (v25.4.0 "Rigel-U")

**Problem:** A single malformed file can crash the decoder DLL and cause Explorer to hang
or restart, disrupting user workflow and producing no diagnostic information.

**Solution:** An ML-based crash predictor trained on decoder error history quarantines
decoders showing elevated failure rates before they can crash. An adaptive timeout tuner
learns per-format latency distributions and tightens timeouts proactively.

**Key deliverables:**
- `DecoderCrashPredictor.h` — Random-forest on error count/latency/format features
- `DecoderQuarantineManager.h` — Isolates failing decoders, offers graceful degradation
- `AdaptiveTimeoutTuner.h` — Bayesian timeout learning per file-extension
- `COMSelfRepairValidator.h` — Validates + repairs broken COM registry keys at startup
- `BootIntegritySelfTest.h` — Verifies all decoder binaries and COM state on cold start

---

### T6 — Multi-Instance & Virtual Desktop (v25.5.0 "Rigel-V")

**Problem:** Multiple Explorer windows, virtual desktops, RDS sessions, and Citrix users
on the same machine all share a single COM server instance, causing priority starvation
and incorrect DPI resolution for background-window thumbnails.

**Solution:** Instance-aware routing logic with a virtual-desktop scope for cache keys,
WTS session isolation for multi-user environments, and foreground-window priority
inheritance to ensure the visible folder always renders fastest.

---

### T7 — Collaborative Annotations & Sharing (v25.6.0 "Rigel-W")

**Problem:** No mechanism exists to annotate files within Explorer thumbnails — designers
and content teams must use external tools to attach review comments or star ratings to
individual files.

**Solution:** A lightweight SQLite annotation store attached to each file by path + hash,
with an overlay renderer that projects stars, colour tags, and comment badges directly
onto thumbnails. Optional cloud sync via SharePoint/OneDrive and web hooks to Teams/Slack.

---

### T8 — Protocol Surface & API Ecosystem (v25.7.0 "Rigel-X")

**Problem:** The thumbnail engine is only accessible via COM, making it impossible to call
from CI/CD pipelines, web services, or non-Windows environments (WSL, .NET build agents,
Python scripts) without writing COM interop boilerplate.

**Solution:** Ship an embedded gRPC server, a REST/HTTP API using cpp-httplib, and a
GraphQL query layer. Auto-generate an OpenAPI 3.1 spec and SDK bindings for C#, Python,
and PowerShell. Rate-limit and OAuth-protect all remote endpoints.

---

### T9 — Post-Quantum Security (v26.0.0 "Canopus" ★★)

**Problem:** All current plugin signing uses RSA-2048 / ECDSA-P256 — both broken by
Shor's algorithm on a cryptographically relevant quantum computer. NIST FIPS 203 (ML-KEM)
and FIPS 205 (SLH-DSA) are finalized and should be adopted ahead of mandate deadlines.

**Solution:** Migrate plugin signing to ML-DSA (FIPS 204), add ML-KEM key encapsulation
for IPC channel setup, and deliver a crypto-agility engine that allows algorithm negotiation
so the transition leaves no compatibility gaps.

**Key deliverables:**
- `MLKEMKeyEncapsulator.h` — FIPS 203 ML-KEM-768 implementation
- `SLHDSASignatureVerifier.h` — FIPS 205 SLH-DSA-SHAKE-256s verifier
- `HybridTLSIPCChannel.h` — Kyber + X25519 hybrid key agreement for IPC
- `CryptoAgilityEngine.h` — Algorithm registry + negotiation + hot-upgrade
- `CertificateMigrationTool.h` — Migrate existing RSA certificates to ML-DSA

---

### T10 — Windows Next-Gen Shell Integration (v26.1.0 "Canopus-R")

**Problem:** ExplorerLens still uses Win32 COM patterns from the Windows Vista-era shell
extension model. Windows App SDK 2.x and Windows 11 24H2+ expose richer WinRT thumbnail
projection, MSIX streaming, and Copilot+ AI Platform APIs that enable tighter shell
integration with lower overhead.

**Solution:** Project the existing IThumbnailProvider implementation through a WinRT
thumbnail bridge, integrate with the Copilot+ AI Platform for on-device model execution,
and use AppContainer isolation for the COM server upgrade path.

---

## Architecture Evolution Diagram

```
                v23.x Vega Foundation
        ┌─────────────────────────────────┐
        │  Reactive Pipeline (CQRS/Saga)  │
        │  GPU v3 (DX11/DX12/Vulkan)      │
        │  AI Pipeline (DirectML/OONX)    │
        │  Plugin Ecosystem v3            │
        │  Smart Cache v4                 │
        └───────────┬─────────────────────┘
                    │
                v24.x Altair Extension
        ┌───────────▼─────────────────────┐
        │  Out-of-Proc COM Server          │
        │  Cloud Backend (Azure/S3)        │
        │  Enterprise Policy v3            │
        │  ESRGAN / CLIP / Auto-Tag AI     │
        │  WCAG 2.2 Accessibility          │
        └───────────┬─────────────────────┘
                    │
            ┌───────▼──────────────────────────────────────┐
            │           v25.x Rigel NEW SURFACE              │
            │                                               │
            │  ┌──────────────┐  ┌────────────────────┐    │
            │  │  WASM Plugin │  │  gRPC / REST /     │    │
            │  │  Sandbox     │  │  GraphQL API       │    │
            │  └──────┬───────┘  └──────────┬─────────┘    │
            │         │                     │               │
            │  ┌──────▼───────┐  ┌──────────▼─────────┐    │
            │  │ Neural Format│  │  VCS Overlays      │    │
            │  │ Intelligence │  │  (Git/P4/SVN)      │    │
            │  └──────┬───────┘  └────────────────────┘    │
            │         │                                     │
            │  ┌──────▼───────┐  ┌──────────────────────┐  │
            │  │  NPU Compute │  │  Self-Healing Engine  │  │
            │  │  (Intel/QC)  │  │  (crash predictor)   │  │
            │  └──────────────┘  └──────────────────────┘  │
            └───────────────┬──────────────────────────────┘
                            │
                    v26.0 Canopus
            ┌───────────────▼──────────────────────────────┐
            │  Post-Quantum Crypto (ML-KEM / SLH-DSA)       │
            │  Windows App SDK 2.x WinRT Bridge             │
            │  Copilot+ AI Platform Integration             │
            └──────────────────────────────────────────────┘
```

---

## Performance Targets

| Metric | Current (v23.5) | v25.0 Target | v26.0 Target | Method |
|--------|----------------|-------------|-------------|--------|
| Single thumbnail | 17ms | **12ms** | **9ms** | NPU AI + adaptive timeout |
| Batch throughput | 235 img/sec | **300 img/sec** | **380 img/sec** | Cross-instance LB + WASM parallelism |
| Cache hit latency | 5ms | **3ms** | **2ms** | Zero-copy cross-session pool |
| AI inference | 4ms (GPU) | **1.5ms (NPU)** | **1.2ms (NPU)** | Intel/Qualcomm NPU EP |
| Memory footprint | ~80 MB | **<60 MB** | **<50 MB** | WASM sandbox + self-repair |
| Cold start → first thumbnail | 250ms | **180ms** | **120ms** | Boot self-test parallelism |
| Plugin install to active | 30s (DLL) | **3s (WASM)** | **3s (WASM)** | No restart needed |

---

## Code Quality & Technical Debt Goals

### Mandatory for v25.0.0 Release

- [ ] **Complete fix14/fix15 type-rename cascades** — EngineTests.cpp, ZeroCopyPipelineTests.cpp,
      PluginReferencePackTests.cpp, PluginHostServer.cpp all must compile clean
- [ ] **Zero build warnings** — MSVC v145 Release, MSVC Debug, CI Clang
- [ ] **100% test pass rate** — all 3197 unit tests must pass
- [ ] **Archive stale docs** — ROADMAP_V16.md, FEATURE_FREEZE_16.md, MIGRATION_GUIDE_V16.md
      moved to `docs/archive/`
- [ ] **Update stale doc versions** — PERFORMANCE.md (v20.2→v25.x), ENTERPRISE.md (v19.1→v25.x)

### Fuzz Testing (new for Rigel)

All new decoders and the WASM runtime adapter must have libFuzzer harnesses:

```cpp
// Engine/Tests/fuzz/WASMPluginLoader_fuzz.cpp
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    WASMPluginLoader loader;
    loader.TryLoad(data, size);  // must not crash or leak
    return 0;
}
```

### CI/CD Enhancements

- Add **ARM64 CI job** (GitHub-hosted `windows-arm64` runner when available)
- Add **benchmark regression gate** — fail CI if batch throughput drops >5%
- Add **WASM plugin compliance check** in CI — validate `.wasmplug` bundles against WIT spec
- Publish **OpenAPI spec** to `docs/api/openapi.yml` on every release tag

---

## Security Posture Goals

| Standard | Current | Rigel Target |
|----------|---------|-------------|
| Plugin signing | RSA-2048 / ECDSA | ML-DSA (FIPS 204) hybrid |
| IPC encryption | AES-256-GCM | Hybrid TLS (Kyber + X25519) |
| Plugin sandbox | Job Object + ACL | WASM linear memory + caps |
| Audit trail | AES-256 encrypted log | PQ-resistant Merkle hash chain |
| Compliance | SOC2 Type I | SOC2 Type II (v26.0 target) |
| Crypto compliance | FIPS 140-2 mode | **FIPS 140-3 Level 2+** (v26.0) |

---

## Toolchain & Dependencies (Rigel Series)

| Tool / Library | Current | Rigel Minimum | Notes |
|----------------|---------|--------------|-------|
| MSVC | v145 (19.50) | v145 (19.50)+ | Unchanged |
| Windows SDK | 10.0.26100 | 10.0.26100+ | Unchanged |
| WasmEdge | — | 0.14.x | New — WASM runtime |
| OpenVINO | — | 2025.x | New — Intel NPU EP |
| libgit2 | — | 1.8.x | New — VCS overlays |
| liboqs | — | 0.11.x | New — post-quantum primitives |
| cpp-httplib | — | 0.20.x | New — REST server |
| grpc-cpp | — | 1.65.x | New — gRPC service |
| ONNX Runtime | 1.x | 1.20.x | Updated — QNN NPU EP |
| cmake | 4.3.0 | 4.3.0+ | Unchanged |
| ninja | 1.13.2 | 1.13.2+ | Unchanged |

---

## Documentation Plan

New documents to be created alongside Rigel sprints:

| Document | Sprint | Description |
|----------|--------|-------------|
| `docs/WASM_PLUGIN_GUIDE.md` | 561 | WASM plugin authoring + packaging guide |
| `docs/NEURAL_FORMAT_GUIDE.md` | 571 | Neural format detection — tuning & corpus |
| `docs/NPU_ACCELERATION.md` | 581 | NPU backend setup and performance guide |
| `docs/VCS_INTEGRATION.md` | 591 | VCS overlay configuration + libgit2 setup |
| `docs/SELF_HEALING.md` | 601 | Self-healing engine tuning guide |
| `docs/API_REFERENCE.md` | 631 | gRPC + REST + GraphQL API reference |
| `docs/POSTQUANTUM_GUIDE.md` | 641 | Post-quantum migration guide for operators |
| `docs/api/openapi.yml` | 635 | Auto-generated OpenAPI 3.1 spec |
| `SDK/wasm/wit/thumbnail-plugin.wit` | 564 | WIT interface definition for WASM plugins |
| `SDK/proto/thumbnail.proto` | 631 | protobuf3 service definition |
| `docs/archive/ROADMAP_V16.md` | 561 | Archived legacy roadmap |

---

## Risks & Mitigations

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|------------|
| WasmEdge API churn between versions | Medium | High | Pin WasmEdge version in CMake; abstract behind `WASMRuntimeAdapter` |
| libgit2 locking conflicts with actual Git processes | Low | Medium | Use read-only `git_repository_open_ext` with `GIT_REPOSITORY_OPEN_NO_SEARCH` |
| OpenVINO binary size bloat (+200MB) | High | Medium | Ship OpenVINO as optional DLL set; feature-flag gate behind `AllowNPU` policy |
| gRPC adds 8 MB to LENSShell.dll | High | Low | Build grpc as separate `lens-api-server.exe` process; COM server stays lean |
| Post-quantum signature breaking old plugin ecosystem | Medium | High | Keep dual-signing (ECDSA + ML-DSA) for 2 major versions; migrate gradually |
| ARM64 CI not yet available on GitHub-hosted runners | Medium | Low | Use QEMU emulation or self-hosted runner for ARM64 validation |

---

## Dependencies Between Themes

```
T1 (WASM Sandbox)
    └─► T8 (Protocol Surface — WASM plugin distribution endpoint)

T2 (Neural Format)
    └─► T5 (Self-Healing — crash predictor uses format-uncertainty score)

T3 (NPU Compute)
    └─► T2 (Neural Format — NPU runs the format classifier model)

T4 (VCS Integration)
    └─► T6 (Multi-Instance — branch-aware cache keys scoped per VD)

T5 (Self-Healing)
    └─► T1 (WASM Sandbox — quarantine applies to WASM plugins too)

T9 (Post-Quantum)
    └─► T1 (WASM Sandbox — WASM plugin bundles signed with ML-DSA)
    └─► T8 (Protocol Surface — gRPC channel uses hybrid TLS)
```

---

## See Also

- [SPRINT_PLAN_600.md](SPRINT_PLAN_600.md) — Full per-sprint deliverable breakdown (561–660)
- [SPRINT_PLAN_500.md](SPRINT_PLAN_500.md) — Sprints 461–560 (v23.6–v24.7 "Altair-X")
- [AI_ARCHITECTURE.md](AI_ARCHITECTURE.md) — AI pipeline architecture reference
- [ENTERPRISE.md](ENTERPRISE.md) — Enterprise fleet management guide
- [SECURITY_HARDENING.md](SECURITY_HARDENING.md) — Security hardening details
- [PLUGIN_DEVELOPMENT.md](PLUGIN_DEVELOPMENT.md) — Current native plugin guide (see T1 for WASM equivalent)
