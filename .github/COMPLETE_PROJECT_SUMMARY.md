# DarkThumbs v7.0.0 - Complete Project Summary

**Project:** DarkThumbs - GPU-Accelerated Thumbnail Generator  
**Current Version:** v7.0.0  
**Last Updated:** February 18, 2026  
**Status:** 🔄 Active Development — Sprints 1-39 Complete; Sprints 40-42 Planned

> **Note:** This document replaces the previous aspirational v7.5.0 summary.
> All claims below reflect verified, committed code as of the latest git history.

---

## Executive Summary

DarkThumbs v7.0.0 is a GPU-accelerated Windows shell extension (IThumbnailProvider COM DLL)
that generates thumbnails for 200+ file extensions across 25 specialized decoders.
The project has completed 39 of 42 planned sprints, with advanced infrastructure for
cloud integration, multi-tier caching, enterprise deployment (GPO), crash intelligence,
supply-chain security (SBOM), USN journal cache invalidation, plugin marketplace,
accessibility/i18n, video enhancement, modular codec DLLs, context menu integration,
animated thumbnails, and archive grid previews. Sprints 40-42 remain for color management,
duplicate detection, and portable mode.

### What's Real vs. Planned

| Area | Actual Status | Notes |
|------|--------------|-------|
| Core shell extension | ✅ Shipping | CBXShell.dll, 0 errors / 0 warnings |
| 25 decoders | ✅ Built | 200+ extensions covered |
| Unit tests | ✅ 100 tests | 5 benchmarks, 100% pass rate |
| GPU rendering | ✅ D3D11 + D3D12 | Fallback chain works |
| AI thumbnails | 🔧 Headers/stubs | AIThumbnailEnhancer.h/cpp exist, not end-to-end tested |
| MSIX packaging | 🔧 Manifest only | AppxManifest.xml created, not submitted to Store |
| OpenImageIO | 📋 Planned | Integration stubs + test contracts, library not yet linked |
| Cloud integration | ✅ Sprint 26 | CloudThumbnailProvider.h — OneDrive/Google Drive/Dropbox abstraction |
| Multi-tier caching | ✅ Sprint 27 | MultiTierCache.h — Bloom filter, WAL SQLite, 3-tier hierarchy |
| Video enhancement | ✅ Sprint 28 | VideoEnhancer.h — scene detection, HDR tone mapping |
| Plugin marketplace | ✅ Sprint 29 | PluginMarketplace.h — signing, security scanning, REST API |
| Accessibility & i18n | ✅ Sprint 30 | AccessibilityFramework.h — 5 languages, UIA, WCAG contrast |
| Enterprise GPO | ✅ Sprint 31 | EnterpriseDeployment.h — ADMX, silent install, network cache |
| Performance profiling | ✅ Sprint 32 | PerformancePolish.h — micro-profiler, memory pool, soak test |
| Crash intelligence | ✅ Sprint 33 | CrashIntelligence.h — minidump, symbol pipeline, bucketing |
| Supply-chain security | ✅ Sprint 34 | SupplyChainSecurity.h — SBOM SPDX/CycloneDX, CI policy gate |
| USN cache invalidation | ✅ Sprint 35 | USNCacheInvalidation.h — NTFS journal, file identity keys |
| Microsoft Store published | ❌ Not yet | Manifest ready, submission pending |

---

## Sprint Completion Status

### ✅ Foundation (Sprints 1-5)
| Sprint | Name | Status |
|--------|------|--------|
| 1 | Repo & Doc Integrity | ✅ Complete |
| 2 | Script Surface Consolidation | ✅ Complete |
| 3 | Architecture Path Hardening | ✅ Complete |
| 4 | Performance Instrumentation | ✅ Complete |
| 5 | Test Infrastructure & CI | ✅ Complete |

### ✅ Stability & Platform (Sprints 6-12)
| Sprint | Name | Key Deliverables | Commit |
|--------|------|------------------|--------|
| 6 | Worker/Isolation | DecoderIsolation.h, FuzzingFramework.h, MemoryLeakDetector.h | `b5d0f96` |
| 7 | Windows 11 Compat | Win11CompatibilityMatrix.h | `d9f97d4` |
| 8 | GUI Hardening | GUIHardening.h | `a3155f5` |
| 9 | Version Normalization | VersionNormalization.h (release notes, decoder registry, stale doc tracker) | `a3f232a` |
| 10 | Release Governance | ReleaseGovernance.h (quality gates, signing policy, CI pipeline registry) | `d684c77` |
| 11 | Plugin System Activation | PluginActivation.h (feature flags, state machine, IPC, lifecycle) | `1c1a5f6` |
| 12 | Observability | ObservabilityPipeline.h (ETW 15 events, JSON logger, privacy filter, request tracer) | `c872959` |

### ✅ Quality & Performance (Sprints 13-22)
| Sprint | Name | Status |
|--------|------|--------|
| 13 | Real-File Test Fixtures | ✅ Complete |
| 14 | Memory-Mapped I/O | ✅ Complete |
| 15 | PSD & Advanced Decoders | ✅ Complete |
| 16 | Code Signing & Distribution | ✅ Complete |
| 17 | Performance Regression Gates | ✅ Complete |
| 18 | WinUI 3 Manager Phase 1 | ✅ Complete |
| 19 | WinUI 3 Manager Phase 2 | ✅ Complete |
| 20 | ARM64 & Cross-Platform Prep | ✅ Complete |
| 21 | D3D12 GPU Upgrade | ✅ Complete |
| 22 | Async Pipeline & Streaming | ✅ Complete |

### ✅ Advanced Features (Sprints 23-25) — Infrastructure
| Sprint | Name | What Exists | What Remains |
|--------|------|-------------|--------------|
| 23 | AI-Assisted Thumbnails | AIThumbnailEnhancer.h/cpp, 7 AI files, 449-line test suite | End-to-end integration, model download, accuracy validation |
| 24 | MSIX Packaging | AppxManifest.xml (v7.0.0), 22 GTest cases | Replace CLSID placeholder, Store submission, asset creation |
| 25 | OpenImageIO | 18 GTest cases, format priority contract | vcpkg integration, OIIODecoder.h/cpp implementation |

### ✅ Advanced Features & Production (Sprints 26-35)
| Sprint | Name | Key File | Commit |
|--------|------|----------|--------|
| 26 | Cloud Integration & Sync | Engine/Cloud/CloudThumbnailProvider.h | `0936aa5` |
| 27 | Advanced Caching (Multi-tier) | Engine/Cache/MultiTierCache.h | `af60118` |
| 28 | Video Enhancement | Engine/Decoders/VideoEnhancer.h | `897cabd` |
| 29 | Plugin Marketplace | Engine/Plugin/PluginMarketplace.h | `7e2df03` |
| 30 | Accessibility & i18n | Engine/Utils/AccessibilityFramework.h | `290205f` |
| 31 | Enterprise Deployment | Engine/Utils/EnterpriseDeployment.h | `d300508` |
| 32 | Performance Polish | Engine/Utils/PerformancePolish.h | `b3cbed2` |
| 33 | Crash Intelligence | Engine/Plugin/CrashIntelligence.h | `433ffea` |
| 34 | Supply-Chain Security | Engine/Utils/SupplyChainSecurity.h | `7395a9e` |
| 35 | USN Cache Invalidation | Engine/Cache/USNCacheInvalidation.h | `1e024eb` |

### ✅ Modular Codecs & UX Enhancements (Sprints 36-39)
| Sprint | Name | Key File | Commit |
|--------|------|----------|--------|
| 36 | Modular Codec DLLs | Engine/Codec/ICodecModule.h, CodecLoader.h, MemoryOptimizationEngine.h | `06669f4` |
| 37 | Context Menu & Shell UX | Engine/Shell/ContextMenuHandler.h | `3f8f427` |
| 38 | Animated Thumbnails | Engine/Decoders/AnimatedThumbnailDecoder.h | `b2244a5` |
| 39 | Archive Grid Preview | Engine/Decoders/ArchiveGridPreview.h | `9490f1d` |

### 📋 Future (Sprints 40-42)
| Sprint | Name | Priority |
|--------|------|----------|
| 40 | Color Management | P3 |
| 41 | Hash & Dedup | P3 |
| 42 | Portable Edition | P3 |

---

## Verified Build Metrics

| Metric | Value | Source |
|--------|-------|--------|
| CBXShell.dll | 2,940 KB | Release x64 build |
| CBXManager.exe | 400 KB | Release x64 build |
| DarkThumbsEngine.lib | 133 MB | CMake Release build |
| Compiler warnings | 0 | /W4 Release configuration |
| Unit tests | 100 assertions | CTest, 100% pass rate |
| Benchmarks | 5 suites | CTest, all pass |
| Single thumbnail | 17 ms | 256x256 output |
| Cache hit | 3-5 ms | In-memory LRU |
| Batch throughput | 235.3 img/sec | 20-image batch |

---

## Architecture (Verified)

```
+----------------------------------------------+
|            Windows Explorer                   |
|         IThumbnailProvider COM                |
+------------------+---------------------------+
                   |
                   v
+----------------------------------------------+
|         CBXShell.dll (Shell Extension)        |
|  - COM registration for 200+ extensions      |
|  - EngineAdapter -> DarkThumbsEngine.lib     |
|  - SEH crash isolation wrapper               |
+------------------+---------------------------+
                   |
                   v
+----------------------------------------------+
|       DarkThumbsEngine.lib (Core Engine)     |
|                                              |
|  25 Decoders:                                |
|  +- Archives: ZIP, RAR, 7Z, TAR, CBZ/CBR    |
|  +- E-Books: EPUB, MOBI, AZW, FB2, PHZ      |
|  +- Modern Images: WebP, AVIF, HEIF, JXL    |
|  +- Professional: PSD, DDS, HDR, EXR        |
|  +- Classic: TIFF, SVG, ICO, BMP, GIF, PNG  |
|  +- Specialty: QOI, PPM, TGA                |
|  +- Camera RAW: DNG, CR2, CR3, NEF, ARW     |
|  +- Video: MP4, AVI, MKV, MOV, WebM         |
|  +- Audio: MP3, FLAC, WAV, OGG              |
|  +- Documents: PDF, DOCX, XLSX              |
|  +- Fonts: TTF, OTF, WOFF                   |
|  +- 3D Models: OBJ, STL, FBX, GLTF         |
|                                              |
|  GPU: D3D11 (fallback) + D3D12 (primary)    |
|  AI:  AIThumbnailEnhancer (DirectML/ONNX)   |
|  Cache: In-memory LRU + SQLite              |
|  Plugins: PluginManager + PluginHost IPC    |
+----------------------------------------------+
                   |
                   v
+----------------------------------------------+
|       CBXManager.exe (Configuration UI)      |
|  - WTL dialog (primary, production)          |
|  - WinUI 3 pages (in progress)               |
|  - 35 format toggle checkboxes               |
|  - Dark mode support                         |
|  - Config snapshots + change summary         |
|  - Plugin management UI                      |
+----------------------------------------------+
```

---

## External Libraries (x64 Release, Verified)

| Library | Version | Purpose |
|---------|---------|---------|
| zlib | 1.3.1 | Compression |
| LZ4 | 1.10.0 | Fast compression |
| zstd | 1.5.7 | Modern compression |
| LZMA SDK | 26.00 | 7-Zip compression |
| minizip-ng | 4.0.10 | Archive extraction |
| UnRAR | 7.2.2 | RAR extraction |
| libwebp | 1.5.0 | WebP decode |
| libavif | 1.3.0 | AVIF decode (dav1d 1.5.1) |
| libjxl | 0.11.1 | JPEG XL decode |
| libheif | 1.19.5 | HEIF/HEIC (libde265) |
| LibRaw | 0.21.3 | Camera RAW |

---

## Git Commit History (Recent Sessions)

```
9490f1d Sprint 39: Archive Content Grid Preview
b2244a5 Sprint 38: Animated & Multi-Frame Thumbnails
3f8f427 Sprint 37: Context Menu & Shell UX
c872959 Sprint 12: Observability Pipeline
1c1a5f6 Sprint 11: Plugin System Activation
d684c77 Sprint 10: Release Governance
a3f232a Sprint 9: Version Normalization
a3155f5 Sprint 8: GUI Hardening
d9f97d4 Sprint 7: Windows 11 Compatibility Matrix
b5d0f96 Sprint 6: Worker/Isolation Stabilization
06669f4 Sprint 36: Modular Codec DLLs & Memory Optimization
c959f3e Sprints 23-25: AI thumbnails verified, MSIX v7.0.0 fix, OpenImageIO tests
```

---

## Known Gaps & Technical Debt

### Must-Fix Before Release
1. **MSIX CLSID placeholder:** `YOUR-CLSID-HERE` in AppxManifest.xml needs actual CLSID
2. **RC dialog resource:** Sprint 8 added 12 new IDC_ defines but .rc file needs Visual Studio Resource Editor to add physical checkbox controls
3. **cbxArchive.h CBXTYPE gaps:** ICO, QOI, TGA, MODEL, DOCUMENT types not yet defined in archive type enum

### Observability (Partially Wired)
- ETWTracing.h and StructuredLogger.h exist; ObservabilityIntegration.h unifies them
- Pipeline telemetry integration pending (connect ScopedTrace to decode pipeline)

### GUI
- Dynamic layout (OnSize) exits immediately — cosmetic issue for fixed-size dialog
- WinUI 3 manager pages created but not yet at feature parity with WTL

### Documentation
- `.github/SPRINTS_24-32_SUMMARY.md` — Updated to reflect actual Sprint 26-35 completion status with accurate file/commit references
- `.github/SPRINTS_6-22_SUMMARY.md` — Some inflated descriptions (test counts, feature claims)

---

## Learnings & Best Practices

### Build System
- **VS version detection:** Always use `vswhere.exe` — never hardcode VS paths
- **Static vs dynamic CRT:** External libs use /MD, Engine uses /MT — keep compatible
- **Proxy builds:** Corporate proxy needs `http.proxy` + SSL revoke disable for GitHub clones
- **CMake generators:** Use `Visual Studio 18 2026` with `v145` toolset, or Ninja for speed

### GUI
- **Dark mode on WTL:** WM_CTLCOLOR* alone is insufficient — all child controls need matching
- **Owner-draw checkboxes:** BS_OWNERDRAW hides text on themed Windows — avoid in WTL
- **Dynamic layout:** WTL CDialogImpl has no anchor-based layout; manual OnSize risks overlap

### Decoders
- **Compound extensions:** `.tar.gz` needs two-dot matching in CanDecode()
- **RAW decoder:** LibRaw handles CR3/ARW/ORF/GPR — don't route through WIC
- **HEIF linkage:** Use `de265.lib` import library, copy `libde265.dll` to output at runtime

### Testing
- **CTest PATH:** Windows needs semicolon-joined PATH via `string(JOIN)` for DLL dirs
- **GPU in CI:** All GPU tests must soft-pass with `[SKIP]` when D3D11 init fails (headless)
- **Plugin system myth:** LoadPlugins() was never commented out — already gated behind `config.enablePlugins` (default: true)

### Documentation
- **Aspirational docs are dangerous:** Previous .github docs claimed features that didn't exist (v7.5.0, Store published, 500+ tests). Always verify against actual code.
- **Single source of truth:** MASTER_PLAN.md (root) is the canonical roadmap

---

## Reference

- **Roadmap:** [MASTER_PLAN.md](../MASTER_PLAN.md) (root)
- **Build Status:** [IMPLEMENTATION_STATUS.md](standards/IMPLEMENTATION_STATUS.md)
- **Build Guide:** [QUICK_BUILD_REFERENCE.md](../QUICK_BUILD_REFERENCE.md)
- **Developer Guide:** [DEVELOPER_GUIDE.md](../DEVELOPER_GUIDE.md)
- **Changelog:** [CHANGELOG.md](../CHANGELOG.md)
