# DarkThumbs Implementation Status

**Last Updated:** February 17, 2026  
**Version:** v7.0.0  
**Status:** 🔄 Active Development — Sprints 1-8, 13-22 Complete; Sprints 9-12 Next

---

## Executive Summary

✅ **Build Status:** 0 errors, 0 warnings (Release x64)  
✅ **Architecture:** 64-bit only (Win32 removed)  
✅ **Engine:** DarkThumbsEngine.lib integrated via EngineAdapter  
🔄 **Plugin System:** Built but `LoadPlugins()` not yet activated (Sprint 11 target)  
✅ **Libraries:** Core dependencies built and linked (HEIF enabled)  
✅ **Decoders:** 25 decoder files covering 200+ extensions  
✅ **Testing:** 100 unit tests, 5 benchmarks — 100% pass rate  
✅ **UI (WTL):** CBXManager with format toggles, tooltips, config snapshots  
🔄 **UI (WinUI 3):** Settings page created, integration ongoing  
🔄 **Observability:** ScopedTimer profiling live, ETW planned (Sprint 12)  
🔄 **Distribution:** MSI infrastructure ready, signing planned (Sprint 10)  
✅ **Performance:** 235.3 img/sec batch, 17ms single thumbnail, <5ms cache hit

---

## Sprint Completion Status

### ✅ Completed (26 of 42 sprints)

| Sprint | Name | Phase | Status |
|--------|------|-------|--------|
| 1 | Repo & Doc Integrity | A - Foundation | ✅ Complete |
| 2 | Script Surface Consolidation | A - Foundation | ✅ Complete |
| 3 | Architecture Path Hardening | B - Engine | ✅ Complete |
| 4 | Performance Instrumentation | C - Performance | ✅ Complete |
| 5 | Test Infrastructure & CI | C - Performance | ✅ Complete |
| 13 | Real-File Test Fixtures | E - Quality | ✅ Complete |
| 14 | Memory-Mapped I/O | C - Performance | ✅ Complete |
| 15 | PSD & Advanced Decoders | B - Engine | ✅ Complete |
| 16 | Code Signing & Distribution | E - Release | ✅ Complete |
| 17 | Performance Regression Gates | C - Performance | ✅ Complete |
| 18 | WinUI 3 Manager Phase 1 | D - GUI | ✅ Complete |
| 19 | WinUI 3 Manager Phase 2 | D - GUI | ✅ Complete |
| 20 | ARM64 & Cross-Platform Prep | D - Compat | ✅ Complete |
| 21 | D3D12 GPU Upgrade | C - Performance | ✅ Complete |
| 22 | Async Pipeline & Streaming | C - Performance | ✅ Complete |

> **Note:** Sprints 13-22 were executed before 6-12 due to dependency alignment.

### 🔄 In Progress / Next (Sprints 6-12)

| Sprint | Name | Objective | Status |
|--------|------|-----------|--------|
| 6 | Worker/Isolation Stabilization | SEH fuzzing, circuit breaker stress, timeout enforcement | ✅ Complete |
| 7 | Windows 11 Compatibility Matrix | 22H2/23H2/24H2 validation, multi-DPI, HDR | ✅ Complete |
| 8 | GUI Hardening | Dark mode fix, high-DPI, Export Diagnostics, missing formats | ✅ Complete |
| 9 | Version Normalization | Update 12 stale docs to v7.0.0, release notes | ⬜ Planned |
| 10 | Release Governance & Packaging | MSI E2E validation, release checklist, CI pipeline | ⬜ Planned |
| 11 | Plugin System Activation | Uncomment LoadPlugins(), end-to-end IPC test | ⬜ Planned |
| 12 | Observability & Logging | ETW provider, JSON logger, diagnostics export | ⬜ Planned |

### 📅 Future (Sprints 23-42)

| Sprint | Name | Category |
|--------|------|----------|
| 23 | AI-Assisted Thumbnails | Advanced Features |
| 24 | Microsoft Store Submission | Distribution |
| 25 | OpenImageIO Integration | Format Expansion |
| 26-27 | Cloud & Caching | Infrastructure |
| 28-29 | Video Enhancement & Marketplace | Ecosystem |
| 30-32 | Accessibility, Enterprise, Polish | Production |
| 33-36 | Crash Intel, Supply Chain, USN, Enterprise | Infrastructure (NEW) |
| 37-42 | Context Menu, Animated, Grid, Color, Hash, Portable | UX Enhancement (NEW) |

---

## What's Complete ✅

### Core Infrastructure
- [x] Engine library (DarkThumbsEngine.lib, 133 MB)
- [x] Engine integration in shell extension (EngineAdapter enabled)
- [x] Plugin SDK built (not yet activated — Sprint 11)
- [x] 64-bit enforcement (Win32 configs removed)
- [x] Warning-free Release builds (/W4)
- [x] Memory-mapped I/O for large files (>100MB)
- [x] C++20 coroutine async pipeline (DecodeAsync)
- [x] D3D12 GPU rendering with D3D11 fallback

### External Libraries (x64 Release)
- [x] zlib 1.3.1, lz4 1.10.0, zstd 1.5.7, minizip-ng 4.0.10
- [x] libwebp 1.5.0, libavif 1.3.0 (dav1d 1.5.1), libjxl 0.11.1
- [x] libheif 1.19.5 (libde265), LibRaw 0.21.3, LZMA SDK 26.00
- [x] UnRAR 7.2.2 (UnRAR64.dll)

### Decoder Coverage (25 decoder files in Engine/Decoders/)
| Category | Formats | Decoder |
|----------|---------|---------|
| Archives | .zip, .rar, .7z, .tar, .tar.gz/.bz2/.xz | ArchiveDecoder |
| Comic Books | .cbz, .cbr, .cb7, .cbt | ArchiveDecoder |
| E-Books | .epub, .mobi, .azw, .azw3, .fb2, .phz | EPUBDecoder |
| Modern Images | .webp, .avif, .heif/.heic, .jxl | WebPDecoder, AVIFDecoder, HEIFDecoder, JXLDecoder |
| Professional | .psd/.psb, .dds, .hdr, .exr | PSDDecoder, DDSDecoder, HDRDecoder, EXRDecoder |
| Classic Images | .tif/.tiff, .svg, .ico, .bmp, .gif, .png, .jpg | ImageDecoder, SVGDecoder, ICODecoder |
| Specialty | .qoi, .ppm/.pgm/.pbm, .tga | QOIDecoder, PPMDecoder, TGADecoder |
| Camera RAW | .dng, .cr2, .cr3, .nef, .arw, .orf, .gpr | RAWDecoder |
| Video | .mp4, .avi, .mkv, .mov, .wmv, .webm, .flv | VideoDecoder |
| Audio | .mp3, .flac, .wav, .aac, .ogg, .wma | AudioDecoder |
| Documents | .pdf, .docx, .xlsx, .pptx | PDFDecoder, DocumentDecoder |
| Fonts | .ttf, .otf, .woff, .woff2 | FontDecoder |
| 3D Models | .obj, .stl, .fbx, .gltf, .glb | ModelDecoder |

### Build System
- [x] MSBuild: CBXShell.sln — VS 18 2026, v145 toolset
- [x] CMake: Engine with Ninja + VS 18 generators
- [x] Build-Library-Core.ps1 unified module (680 lines)
- [x] Build-All-And-Package.ps1 orchestrator
- [x] 22 VS Code tasks for all build targets
- [x] CTest with proper DLL PATH handling

### Testing
- [x] 100 unit test assertions, 5 benchmark suites — 100% pass
- [x] GPU tests with headless/CI soft-pass
- [x] All 22+ decoders registered in test pipeline
- [x] 27 format routing assertions

---

## Known Gaps & Technical Debt

### GUI (Sprint 8 Target)
1. **Missing format checkboxes:** ICO, QOI, PPM, TGA, 3D Models have decoders but no GUI toggles
2. **Incomplete format plumbing:** PSD/DDS/HDR/Audio/Document/Font/EXR have resource IDs (IDC_CB_*) but no CBX_* constants in RegManager.h — InitUI and OnApplyImpl don't handle them
3. **Dark mode disabled:** OnCtlColor handlers all `return FALSE`
4. **Dynamic layout disabled:** OnSize handler exits immediately

### Plugin System (Sprint 11)
- `LoadPlugins()` commented out in ThumbnailPipeline.cpp
- Plugin infrastructure built but inactive

### Version Drift (Sprint 9)
- 12 docs with stale version references (v5.x/v6.2) — see MASTER_PLAN.md §2B

### Observability (Sprint 12)
- ETWTracing.h and StructuredLogger.h exist but not wired into pipeline
- Export Diagnostics button partially implemented

---

## Performance Baselines (Sprint 5)

| Metric | Value |
|--------|-------|
| Single thumbnail (256×256) | 17 ms |
| Cache-hit average | 3-5 ms |
| Batch throughput (20 images) | 235.3 img/sec |
| Format detection | 0.03-0.54 μs/detection |
| SIMD scaling (8K AVX2) | 24,296 Mpix/s |
| CBXShell.dll | 2,940 KB |
| CBXManager.exe | 400 KB |
| DarkThumbsEngine.lib | 133 MB |

---

## Learnings & Best Practices

### Build System Learnings
- **VS version detection:** Always use `vswhere.exe` — never hardcode VS paths
- **Static vs dynamic CRT:** External libs use /MD (minizip-ng), Engine uses /MT — keep compatible
- **Proxy builds:** Corporate proxy needs `http.proxy` + SSL revoke disable for GitHub clones
- **CMake generators:** Use `Visual Studio 18 2026` with `v145` toolset, or Ninja for faster builds

### GUI Learnings
- **Dark mode on WTL:** Setting WM_CTLCOLOR* handlers alone is insufficient — all child controls need matching treatment. WinUI 3 XAML theming is the proper solution.
- **Owner-draw checkboxes:** BS_OWNERDRAW hides text on themed Windows controls — avoid in WTL
- **Dynamic layout:** WTL CDialogImpl doesn't natively support anchor-based layout; manual OnSize risks control overlap

### Decoder Learnings
- **Compound extensions:** `.tar.gz` needs two-dot matching in CanDecode()
- **RAW decoder type:** LibRaw handles CR3/ARW/ORF/GPR — don't route through ImageDecoder (WIC)
- **DDS GPU support:** DDS decoder correctly reports `SupportsGPU() = true` (WIC + D3D11)
- **HEIF linkage:** Use `de265.lib` import library, copy `libde265.dll` to output for runtime

### Testing Learnings
- **CTest PATH:** Windows needs semicolon-joined PATH via `string(JOIN)` for DLL directories
- **GPU in CI:** All GPU tests must soft-pass with `[SKIP]` when D3D11 init fails (headless)
- **Archive extension count:** ArchiveDecoder supports 14+ extensions including compound formats

---

## Reference

- **Single source of truth:** [MASTER_PLAN.md](../../MASTER_PLAN.md) (root)
- **Sprint history:** MASTER_PLAN.md Section 6
- **Build reference:** [QUICK_BUILD_REFERENCE.md](../../QUICK_BUILD_REFERENCE.md)
- **Developer guide:** [DEVELOPER_GUIDE.md](../../DEVELOPER_GUIDE.md)
