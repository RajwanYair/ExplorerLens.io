# ExplorerLens — ROADMAP v8.0 "Vega"

> **Zero-illusions edition.** Every decision stress-tested against shipping reality.
> Archived: `docs/archive/ROADMAP_V7.md` (v7.0 "Sirius", ~45 KB, 24 sections, ADRs A1–A28).
> Current version: **39.2.0 "Betelgeuse"** · Last updated: **2026-05-12**

---

## Table of Contents

1.  [Executive Summary — What Changed from v7.0](#1-executive-summary)
2.  [Brutal Reality Check](#2-brutal-reality-check)
3.  [North Star & Anti-Goals](#3-north-star--anti-goals)
4.  [Competitor & Reference Matrix (25 Products × 50 Dimensions)](#4-competitor--reference-matrix)
5.  [Harvested Best Practices (H1–H48)](#5-harvested-best-practices)
6.  [Language, Compiler & Standards — Final Verdicts](#6-language-compiler--standards)
7.  [Frontend Architecture Rethink](#7-frontend-architecture-rethink)
8.  [Backend Architecture Rethink](#8-backend-architecture-rethink)
9.  [API Design — COM, REST, SDK, CLI](#9-api-design)
10. [External Libraries & Third-Party APIs](#10-external-libraries--third-party-apis)
11. [Database & Persistence Strategy](#11-database--persistence-strategy)
12. [Infrastructure & Distribution](#12-infrastructure--distribution)
13. [CI/CD Pipeline (31 Workflows)](#13-cicd-pipeline)
14. [Testing & Quality Strategy](#14-testing--quality-strategy)
15. [Security Stack](#15-security-stack)
16. [Observability Stack](#16-observability-stack)
17. [Documentation Strategy](#17-documentation-strategy)
18. [Tools & Versions Matrix](#18-tools--versions-matrix)
19. [Refactor / Rewrite / Delete / Add Register](#19-refactor--rewrite--delete--add-register)
20. [10-Phase Plan to Best-in-Class](#20-10-phase-plan-to-best-in-class)
21. [Success Metrics & Exit Criteria](#21-success-metrics--exit-criteria)
22. [ADR Log v8.0 (ADRs A1–A34)](#22-adr-log-v80)
23. [Decisions Reversed from v7.0](#23-decisions-reversed-from-v70)
24. [Sprint Delivery Pipeline S301+](#24-sprint-delivery-pipeline-s301)

---

## 1. Executive Summary

v8.0 is a zero-illusions rewrite. v7.0 was ambitious but still contained marketing language that masked engineering reality. v8.0 forces confrontation with what is actually compiled, tested, and shipping — versus what exists only as header declarations.

### What is materially different in v8.0

| Dimension | v7.0 Decision | v8.0 Verdict |
|---|---|---|
| Zero-warnings claim | "0 errors, 0 warnings" | **FALSE** — `/WX-` is set in `Engine/CMakeLists.txt`; restore `/WX` before any new feature work |
| 600+ headers | Counted as "features" | **Reality: ~80% are contract stubs** — no `.cpp`, no tests, no runtime behavior. Count only compiled+tested code |
| GPU pipeline (95 files) | "Phase 2–6 plan" | **All stubs** — not a single GPU pixel rendered. Radically simplify: deliver D3D11 blit in 1 sprint, not 95 headers |
| AI modules (45 files) | "Research phase" | **DELETE** — no model weights, no inference runtime, no user value. Reintroduce only when ONNX Runtime ships with real model |
| Plugin ecosystem (53 files) | "V5 marketplace" | **Suspend** — zero real plugins exist; marketplace without plugins is dead code. Build 1 real plugin first |
| Test count "5,045" | Claimed in README | **Verify** — `BuildValidation.h` says 4,664; badge says 5,045. Reconcile before any claim |
| Documentation "45 docs" target | Quality over quantity | **Reduce to 30** actually-maintained docs. 45 is still too many for a 1-person team |
| C++23 modules (`import std;`) | "Reduce PCH time 40%" | **DEFER** — MSVC C++23 module support has IntelliSense regressions; use `std::expected` and `std::stacktrace` but not modules |
| LENSManager WinUI 3 rewrite | "Phase 3" | **DEFER to Phase 5** — WinUI 3 XAML Islands + COM = complex; fix dark mode in WTL first (already has `DarkModeController.h`) |
| "200+ formats" claim | Marketing | **Audit to actual tested count** — corpus has ~106 CC0 files; real validated coverage is ~50 formats |
| stb_image removal | "Phase 2 replacement" | **Keep as emergency fallback** with `[FALLBACK]` log tag; add libjpeg-turbo + libspng as primary |

---

## 2. Brutal Reality Check

### What actually compiles and runs today (v39.2.0)

| Component | Status | Evidence |
|---|---|---|
| LENSShell.dll COM registration | ✅ Ships | regsvr32 tested |
| LENSManager.exe WTL GUI | ✅ Ships | Dialog + dark mode controller present |
| Custom test harness (TEST/RUN_TEST) | ✅ Runs | 4,664–5,045 tests (count needs reconciliation) |
| Catch2 test suite | ✅ Runs | 42+ Catch2 test files |
| Camera RAW decode (LibRaw) | ✅ Works | Embedded JPEG extraction + full demosaic |
| HEIF/HEIC decode (libheif) | ✅ Works | libheif 1.19.5 + libde265 |
| AVIF decode (libavif + dav1d) | ✅ Works | libavif 1.3.0 + dav1d 1.5.1 |
| JPEG XL decode (libjxl) | ✅ Works | libjxl 0.11.1 |
| WebP decode | ✅ Works | libwebp 1.5.0 |
| PDF render (MuPDF) | ✅ Works | MuPDF 1.24.11 |
| Archive handling | ✅ Works | libarchive 3.7.6 + minizip-ng |
| Video keyframe (Media Foundation) | ✅ Works | Windows MF API |
| ETW telemetry | ✅ Wired | GUID registered |
| SQLite cache | ⚠️ Contract | Schema designed, implementation partial |
| SSIM validation CI | ✅ Active | ssim-validation.yml |
| Google Benchmark | ✅ Active | baseline.json maintained |

### What exists only as headers (no compiled behavior)

| Component | File Count | Reality |
|---|---|---|
| GPU pipeline | 95 headers | Zero GPU pixels ever rendered. D3D11/D3D12/Vulkan/WebGPU/Metal all stubs |
| AI modules | 45 headers | Zero model weights. CLIP, diffusion, inpainting — all empty structs |
| Plugin marketplace | 53 headers | Zero published plugins. V1–V5 marketplace iterations — all contracts, no server |
| Enterprise features | ~30 headers | ADMX, Group Policy, multi-tenant — all contracts |
| REST API | ~10 headers | 7 endpoint contracts, zero HTTP server |
| Platform PAL (macOS/Linux) | 10 stubs | Zero function bodies for macOS Quick Look or Linux Nautilus |

### Honest gap assessment

**Header-to-implementation ratio**: ~600+ headers registered in CMakeLists, but ~250 have matching `.cpp` files with real logic. The remaining ~350 are type declarations, contract interfaces, or empty stubs. This is not a crisis — contract-first development is the project's architecture pattern — but it means "feature count" claims must distinguish between *contracted* and *implemented*.

**The `/WX-` contradiction**: The project claims "0 errors, 0 warnings" and says "never use `/wdXXXX` warning suppressions." But the actual CMake file uses `/WX-` (warnings NOT as errors). This means warnings may exist silently. **Fix this first.**

---

## 3. North Star & Anti-Goals

### North Star

> ExplorerLens is the **fastest, most format-complete Windows thumbnail provider** that ships as a lightweight, zero-trust shell extension with reliable decode quality.

Simplified from v7.0. Removed "GPU-accelerated" from the north star statement — GPU is a means, not the end. The end is *fast, correct thumbnails*.

### What "Best-in-Class" Actually Means

ExplorerLens is **not** competing with Adobe Bridge or Lightroom. Those are full-featured photo management applications. ExplorerLens is competing with:

1. **The built-in Windows Shell thumbnail system** — our direct replacement target
2. **Apple Quick Look** — the gold standard for "zero-config file preview"
3. **SageThumbs** — the most popular Windows thumbnail shell extension (free)
4. **File Converter** — popular context-menu + thumbnail shell extension
5. **QuickLook for Windows** — spacebar-preview shell extension

The right comparison is: **shell-integrated, zero-config, lightweight thumbnail/preview tools** — not full photo editors.

### Explicit Anti-Goals

| Anti-Goal | Rationale |
|---|---|
| Full image editor | We are a thumbnail provider, not Lightroom |
| AI model inference at user endpoint | Unacceptable attack surface + DLL size impact |
| Plugin marketplace server | Build real plugins first; marketplace is premature |
| Video/audio player | Shell preview pane is sufficient |
| Cloud sync | Network I/O in a shell extension = security violation |
| macOS/Linux production before Windows is complete | Platform stubs are fine; do not split focus |
| Replacing Windows Explorer | We extend it |
| 32-bit (x86) support | Modern Explorer is 64-bit only |
| Windows 7/8/8.1 | COM APIs require Windows 10 1903+ |

---

## 4. Competitor & Reference Matrix

### Category A: Shell Extension / Thumbnail Providers (Direct Competitors)

These are the products ExplorerLens directly competes with — lightweight shell integrations that add thumbnail/preview capabilities to file managers.

Scoring: ✅ = strong · ⚠️ = partial/limited · ❌ = absent · `–` = N/A

| Dimension | ExplorerLens v39 | Windows Built-in | SageThumbs 2.0 | QuickLook (Win) | File Converter | Apple Quick Look | GNOME Thumbnailer | KDE Dolphin | ffmpegthumbnailer |
|---|---|---|---|---|---|---|---|---|---|
| **Format Coverage** | | | | | | | | | |
| JPEG/PNG/BMP/GIF/TIFF | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ❌ |
| WebP | ✅ | ⚠️ | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ | ❌ |
| HEIC/AVIF/JXL | ✅ | ⚠️ | ⚠️ | ⚠️ | ❌ | ✅ | ❌ | ❌ | ❌ |
| Camera RAW (DNG/CR3/ARW) | ✅ | ❌ | ✅ | ⚠️ | ❌ | ✅ | ❌ | ❌ | ❌ |
| PSD/PSB | ⚠️ | ❌ | ✅ | ⚠️ | ❌ | ✅ | ❌ | ❌ | ❌ |
| EXR/HDR | ⚠️ | ❌ | ⚠️ | ❌ | ❌ | ✅ | ❌ | ❌ | ❌ |
| PDF page 1 | ✅ | ❌ | ❌ | ✅ | ❌ | ✅ | ❌ | ❌ | ❌ |
| 3D (glTF/OBJ/STL) | ⚠️ | ❌ | ❌ | ❌ | ❌ | ⚠️ | ❌ | ❌ | ❌ |
| Archives (ZIP/RAR/7z) | ⚠️ | ❌ | ❌ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ |
| Video keyframe | ⚠️ | ❌ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| Font preview (TTF/OTF) | ⚠️ | ❌ | ❌ | ✅ | ❌ | ✅ | ❌ | ❌ | ❌ |
| SVG | ⚠️ | ❌ | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ | ❌ |
| **Performance** | | | | | | | | | |
| Sub-5ms cache hit | ✅ | ⚠️ | ⚠️ | ✅ | ❌ | ✅ | ✅ | ✅ | ✅ |
| Batch > 200 img/sec | ⚠️ | ⚠️ | ⚠️ | ❌ | ❌ | ✅ | ✅ | ✅ | ✅ |
| Async non-blocking | ✅ | ⚠️ | ⚠️ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| GPU-accelerated decode | ❌ | ❌ | ❌ | ❌ | ❌ | ✅ | ❌ | ❌ | ❌ |
| Cancel-aware decode | ❌ | ✅ | ❌ | ❌ | ❌ | ✅ | ❌ | ❌ | ❌ |
| **Quality** | | | | | | | | | |
| ICC color management | ❌ | ❌ | ❌ | ❌ | ❌ | ✅ | ❌ | ❌ | ❌ |
| HDR tone mapping | ❌ | ❌ | ❌ | ❌ | ❌ | ✅ | ❌ | ❌ | ❌ |
| High-DPI aware | ✅ | ✅ | ⚠️ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| SSIM-validated output | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **Platform & Install** | | | | | | | | | |
| Install footprint < 10 MB | ✅ | – | ✅ | ✅ | ✅ | – | ✅ | ✅ | ✅ |
| Silent install (MSI/MSIX) | ⚠️ | – | ✅ | ✅ | ✅ | – | ✅ | ✅ | ✅ |
| WinGet / Scoop / Choco | ⚠️ | – | ❌ | ✅ | ✅ | – | – | – | – |
| Arm64 support | ❌ | ✅ | ❌ | ❌ | ❌ | ✅ | ✅ | ✅ | ✅ |
| MSIX Store-ready | ❌ | – | ❌ | ✅ | ❌ | – | – | – | – |
| Portable (no install) | ✅ | – | ❌ | ✅ | ✅ | – | ✅ | ✅ | ✅ |
| **Security & Trust** | | | | | | | | | |
| Code-signed binaries | ⚠️ | ✅ | ❌ | ✅ | ✅ | ✅ | – | – | – |
| SBOM on release | ✅ | ❌ | ❌ | ❌ | ❌ | ⚠️ | ⚠️ | ⚠️ | ❌ |
| Fuzz-tested decoders | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ✅ |
| Crash telemetry opt-in | ✅ | ⚠️ | ❌ | ❌ | ❌ | ✅ | ✅ | ✅ | ❌ |
| **Developer Experience** | | | | | | | | | |
| Plugin API / SDK | ✅ | ❌ | ❌ | ✅ | ❌ | ✅ | ✅ | ✅ | ❌ |
| CLI headless decode | ⚠️ | ❌ | ❌ | ❌ | ✅ | ❌ | ❌ | ❌ | ✅ |
| Open source | ✅ | ❌ | ✅ | ✅ | ✅ | ❌ | ✅ | ✅ | ✅ |
| **UX** | | | | | | | | | |
| Dark mode in settings UI | ⚠️ | ✅ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ | – |
| Spacebar instant preview | ⚠️ | ❌ | ❌ | ✅ | ❌ | ✅ | ❌ | ❌ | – |
| Context menu integration | ✅ | ✅ | ✅ | ❌ | ✅ | ✅ | ✅ | ✅ | ❌ |

### Category A Scorecard (Shell Extensions Only)

| Product | ✅ Count / 36 | Tier |
|---|---|---|
| Apple Quick Look | 29 | A |
| QuickLook (Windows) | 21 | B |
| **ExplorerLens v39** | **19** | **B (target A)** |
| SageThumbs 2.0 | 14 | C |
| File Converter | 14 | C |
| KDE Dolphin | 14 | C |
| GNOME Thumbnailer | 12 | C |
| Windows Built-in | 10 | C |
| ffmpegthumbnailer | 10 | C |

**ExplorerLens scores 19/36 in the correct competitive category. Target is 30+/36 (Tier A).**

### Category B: Full Photo Management (Aspirational Reference)

These are NOT direct competitors but provide best-practice patterns to harvest:

| Product | Key Strength to Harvest |
|---|---|
| Adobe Bridge 2026 | ICC color pipeline (AGM), folder watch ingest, smart preview proxy |
| Adobe Lightroom | GPU-accelerated decode, catalog DB with inode tracking, progressive rendering |
| Capture One 24 | Color science pipeline (custom ICC engine), tethered camera integration |
| darktable 4.8 | Reproducible builds, SSIM-gated CI, FreeDesktop compliance, OpenCL GPU path |
| digiKam 8 | Plugin architecture (DImg), face detection pipeline, geolocation index |
| Photo Mechanic 6 | Embedded JPEG extraction speed (sub-1ms), parallel I/O readahead N=8 |
| IrfanView 4.7 | Install footprint < 3 MB, community plugin ecosystem, 40-year format compat |
| XnView MP 1.8 | Cross-platform C++ (Qt), batch processing, wide format matrix |
| FastStone 5.9 | Memory efficiency, portable mode, raw decode speed |

### Key Patterns Harvested from Competition

| Pattern | Source | ExplorerLens Impact |
|---|---|---|
| **Embedded JPEG fast-path** | Photo Mechanic | Sub-1ms RAW thumbnail via EXIF tag extraction |
| **ICC color passthrough** | Bridge, Capture One | Decoder emits `(pixels, icc_bytes)` pair; renderer applies lcms2 |
| **Progressive decode** | Lightroom, Google Photos | Show low-res scan immediately; refine in background |
| **Folder readahead** | Photo Mechanic | Pre-decode next N=8 files while displaying current |
| **Cancel-aware decode** | Windows Shell, Quick Look | Honor `IBindStatusCallback` cancellation within 50ms |
| **WIC passthrough first** | Microsoft Photos | Use WIC for natively-supported formats; custom decoder only for gaps |
| **Minimal install footprint** | IrfanView, SageThumbs | Target < 5 MB for shell extension only |
| **Reproducible builds** | darktable | Bit-identical binaries across CI runs |
| **Plugin ABI versioning** | darktable (`dt_iop_module_so_t`) | Explicit uint32 API version in plugin header |
| **SSIM-gated CI** | digiKam | Automated visual regression on every PR |
| **AppContainer sandbox** | QuickLook, Edge | Run decoders in restricted process with minimal capabilities |
| **FreeDesktop thumb spec** | GNOME, KDE | Standard thumbnail storage for Linux cross-app reuse |

---

## 5. Harvested Best Practices

### From Apple Quick Look (Gold Standard Shell Preview)

**H1 — Async placeholder thumbnail**: Return the last cached (possibly stale) bitmap immediately. Decode the fresh version in background. Explorer never shows a blank white square. [Phase 2]

**H2 — Crash telemetry opt-in at first run**: Single consent dialog. No silent telemetry. [Done — S293]

**H3 — ICC color profile passthrough**: Embed source ICC profile in decoded buffer; let display pipeline apply color management. Do not tone-map inside the decoder. [Phase 3]

**H4 — Spacebar instant preview**: Single key activates floating preview panel. [Done — S281]

**H5 — Cancel-aware decode**: Honor cancellation via `IBindStatusCallback`. Return `E_ABORT` cleanly within 50ms. [Phase 2]

**H6 — Format negotiation**: Query the file's actual format via magic bytes before dispatching. Reject corrupt files before allocating decode buffers. [Phase 2]

### From Photo Mechanic 6 (Fastest Ingest Tool)

**H7 — Embedded JPEG extraction**: For camera RAW, extract EXIF embedded JPEG (tag 0x0201/0x0202) before invoking the full raw decoder. Sub-1ms for most camera files. [Phase 2]

**H8 — Parallel I/O readahead**: Pre-read next N=8 files from folder while displaying current. `ParallelIoManagerContract`. [Phase 2]

**H9 — Ingest progress ETW events**: Surface decode progress as ETW events for debugging and monitoring. [Phase 3]

### From Adobe Bridge / Lightroom (Color & Quality)

**H10 — Smart preview (offline proxy)**: Store 2560px JPEG proxy in cache DB. Serve proxy when source file unavailable. [Phase 5]

**H11 — Catalog database with inode tracking**: SQLite keyed on file inode + mtime, not just path. Detect renames without re-decode. [Phase 3]

**H12 — End-to-end ICC pipeline**: Source profile → lcms2 transform → sRGB D65 display. Single biggest quality gap vs A-tier. [Phase 3]

**H13 — Video keyframe extraction**: Extract first non-black keyframe (not frame 0). [Done — S265]

### From darktable / digiKam (Open Source Quality)

**H14 — Reproducible builds**: Controlled CFLAGS + timestamp stripping → bit-identical binaries. [Phase 7]

**H15 — SSIM-gated CI**: Validate thumbnail output against reference images. SSIM ≥ 0.95. [Done — S300]

**H16 — Plugin ABI with explicit versioning**: `dt_iop_module_so_t` pattern. Mirror in `plugin_api.h` v0x00010000. [Done — S296]

**H17 — OpenCL GPU fallback path**: darktable uses OpenCL (not vendor-locked). Consider OpenCL 1.2 as universal GPU fallback before D3D12/Vulkan vendor paths. [Phase 4]

### From IrfanView / FastStone (Efficiency)

**H18 — < 5 MB shell extension footprint**: IrfanView installer < 3 MB. Target: LENSShell.dll < 3 MB. Total MSI < 8 MB. [Phase 3]

**H19 — Plugin community page (not marketplace)**: IrfanView hosts a simple download page. Don't build a marketplace server — build a static JSON catalog first. [Phase 6]

**H20 — Fallback chain with user notification**: When a decoder fails, show a fallback icon with a tooltip explaining why, not a blank square. [Phase 2]

### From QuickLook for Windows (Modern Shell UX)

**H21 — MSIX sparse package**: QuickLook ships as MSIX. Register COM servers without full-trust registry pollution. [Phase 5]

**H22 — Plugin hot-reload**: QuickLook plugins can be added at runtime without restarting Explorer. [Phase 6]

**H23 — WinUI 3 preview panel**: Modern XAML rendering for the preview window. [Phase 5]

### From Microsoft Photos / WIC (Platform Integration)

**H24 — WIC codec passthrough first**: Use Windows Imaging Component for all natively-supported formats. Custom decoders only for formats WIC cannot handle. Reduces code surface and DLL size. [Phase 2]

**H25 — MSIX app model for shell extensions**: Microsoft's recommended path for new shell extensions on Windows 11. [Phase 5]

### From SageThumbs (Proven Shell Extension)

**H26 — Registry-light installation**: SageThumbs uses minimal registry entries. Audit ExplorerLens registry footprint. [Phase 2]

**H27 — GFL SDK for format breadth**: SageThumbs uses the GFL library for 400+ formats. Consider using stb_image + specialized libs rather than shipping 25 independent decoders. [Research]

### From ffmpegthumbnailer (Unix Simplicity)

**H28 — Single-binary CLI**: ffmpegthumbnailer is one binary, one job. `lens.exe` should be equally simple for headless use. [Phase 3]

**H29 — DBus thumbnailer protocol**: FreeDesktop.org standard for Linux thumbnailers. [Phase 9]

### Cross-cutting (Synthesized from All Comparisons)

**H30 — ICC display pipeline**: Decoder emits `(pixels, icc_profile_bytes)`. Renderer applies lcms2 transform to sRGB D65. Biggest single quality gap. [Phase 3]

**H31 — Native Arm64 EC build**: Windows on Arm is fastest-growing segment. `/arm64EC` for COM host interop. [Phase 6]

**H32 — SLSA Level 2 provenance**: `actions/attest-build-provenance@v2` on every release artifact. [Phase 3]

**H33 — OOM kill protection**: `SetProcessWorkingSetSizeEx` + heap trim on low-memory. Prevent shell host crash. [Phase 2]

**H34 — Explorer cancel-aware batch**: `IBindStatusCallback::OnProgress`. Cancel in-flight decodes within 50ms. [Phase 2]

**H35 — Thumbnail overlay branding**: Use Windows `TypeOverlay` registry mechanism (not custom drawing) for format badges. [Phase 2]

**H36 — Process isolation opt-in**: Offer out-of-process decode via `DisableProcessIsolation=0` for crash isolation. [Phase 4]

**H37 — Fallback to Windows thumbnail cache**: If our decode fails, delegate to the Windows built-in thumbnail cache rather than showing a blank icon. Graceful degradation. [Phase 2]

**H38 — Thumbnail adornments via Shell API**: Use `Treatment` registry values (drop shadow, photo border) rather than custom rendering. [Phase 2]

**H39 — Decode timeout with fallback**: Set a 500ms hard timeout on any single decode. If exceeded, return cached/fallback and queue background retry. [Phase 2]

**H40 — Memory-mapped file I/O**: Use `CreateFileMapping` + `MapViewOfFile` for large files instead of `IStream::Read`. Avoids kernel buffer copies. [Phase 3]

**H41 — SIMD-optimized resize**: Use AVX2 Lanczos resize for thumbnail scaling instead of GDI+ `DrawImage`. [Phase 3]

**H42 — Per-format decode budget**: Limit memory allocation per format. RAW files get 128 MB; ICO files get 1 MB. Prevents OOM from malicious files. [Phase 2]

**H43 — Lazy library loading**: `LoadLibrary` external codecs (libheif, libjxl, etc.) only when needed. Reduces cold-start memory. [Phase 3]

**H44 — WIC metadata reader**: Use WIC `IWICMetadataQueryReader` for EXIF/XMP instead of custom parsing. Less code, fewer bugs. [Phase 2]

**H45 — Shared memory thumbnail transfer**: For out-of-process decode, use shared memory (`CreateFileMapping`) instead of serializing bitmaps over pipes. [Phase 4]

**H46 — Windows Property System integration**: Implement `IPropertyStore` to expose EXIF data in Explorer details pane. [Phase 4]

**H47 — Thumbnail cache warming**: On first install, pre-decode common file types in Desktop/Documents/Pictures. [Phase 5]

**H48 — Error telemetry per decoder**: Track which decoders fail most. Prioritize stability fixes by real-world failure rate. [Phase 2]

---

## 6. Language, Compiler & Standards

### C++ Standard — Verdict: C++23 (selective features)

**Decision (ADR A23 — refined)**: Use C++23 features selectively. Do NOT enable C++ modules.

| Feature | Status | Use Case |
|---|---|---|
| `std::expected<T,E>` | ✅ Use now | Replace `HRESULT` error returns in Engine-internal paths |
| `std::stacktrace` | ✅ Use now | Crash reporter — structured stack in ETW events |
| `std::flat_map` | ✅ Use now | LENSTYPE → decoder dispatch (cache-friendly) |
| `std::print` / `std::println` | ✅ Use now | Replace `printf` in CLI and test harness |
| `[[assume(expr)]]` | ✅ Use now | Decoder hot-path branch hints |
| Deducing `this` | ⚠️ Cautious | CRTP-free decoder base only if MSVC stable |
| C++ modules (`import std;`) | ❌ Defer | IntelliSense regressions in VS 2026; revisit at v42.0 |
| `constexpr std::string` | ⚠️ Cautious | Magic-byte tables — profile for compile-time benefit |

### Rust — Verdict: KILL (retained from v7.0)

No Rust in production path. C++23 + ASAN + fuzzing is sufficient for the use case. If a Rust decoder exists (e.g., `zune-jpeg`), consume it as a C-ABI plugin only.

### Clang — Verdict: CI-only (retained)

Clang 18 for ASan, fuzzer builds, and clang-tidy only. Production DLL is always MSVC v145.

### WARNING: Restore `/WX` (ADR A29)

**The `/WX-` flag in `Engine/CMakeLists.txt` line 2559 must be changed to `/WX` before any Phase 2 work begins.** The project's core discipline is zero-warnings builds. This cannot be aspirational.

### Compiler Flags (authoritative, v8.0)

```
/std:c++23 /W4 /WX /permissive- /Zc:__cplusplus /Zc:preprocessor
/fp:fast /GL /Gy /GR- /EHsc /MP /arch:AVX2
/D NOMINMAX /D WIN32_LEAN_AND_MEAN /D UNICODE
/diagnostics:caret
```

---

## 7. Frontend Architecture Rethink

### 7.1 Shell Extension (`LENSShell.dll`) — Keep, Harden, Shrink

**Goal**: LENSShell.dll is the *only* artifact most users interact with. It must be rock-solid.

| Work Item | Priority | Phase | Rationale |
|---|---|---|---|
| Restore `/WX` — fix all warnings | P0 | 1 | Core discipline violated |
| Cancel-aware decode (H5, H34) | P0 | 2 | Prevents shell thread stalling |
| OOM kill protection (H33) | P0 | 2 | Prevents Explorer crash |
| Decode timeout 500ms (H39) | P0 | 2 | Hard budget for shell responsiveness |
| Async placeholder thumbnail (H1) | P1 | 2 | Eliminate blank squares |
| WIC passthrough first (H24) | P1 | 2 | Reduce custom code for standard formats |
| Per-format memory budget (H42) | P1 | 2 | Security: prevent OOM from malicious files |
| Fallback to Windows cache (H37) | P1 | 2 | Graceful degradation |
| Error return `E_FAIL` (not blank bitmap) | P1 | 2 | Current behavior returns `S_OK` with blank |
| DLL size < 3 MB target (H18) | P2 | 3 | Audit and trim linked symbols |
| Arm64 EC build (H31) | P2 | 6 | Windows on Arm support |
| MSIX sparse package (H21, H25) | P3 | 5 | Modern installation model |

### 7.2 LENSManager GUI — Fix WTL First, WinUI 3 Later

**Decision reversal from v7.0 (ADR A30)**: Do NOT rewrite LENSManager in WinUI 3 in Phase 3.

**Rationale**: The project already has `DarkModeController.h` for WTL dark mode. WinUI 3 XAML Islands + COM interop is extremely complex (see Microsoft's own documentation on limitations). The effort-to-value ratio is poor for a settings utility.

**New plan**:
1. **Phase 2**: Complete WTL dark mode using existing `DarkModeController.h`
2. **Phase 2**: Add high-DPI dynamic awareness via `WM_DPICHANGED`
3. **Phase 3**: Add basic accessibility (UIA providers for main controls)
4. **Phase 5**: Evaluate WinUI 3 migration if WTL limitations are blocking

### 7.3 CLI (`lens.exe`) — Simplify

The CLI should be **one binary, one job** (H28). Not a REST server.

| Command | Status | Phase |
|---|---|---|
| `lens decode <file> [--size N] [--output out.png]` | ⚠️ Contract | 2 |
| `lens validate <dir>` | ❌ New | 3 |
| `lens benchmark [--iterations N]` | ✅ Done | Done |
| `lens info <file>` | ❌ New | 3 |
| `lens cache clear` | ❌ New | 3 |

**Decision (ADR A31)**: Remove REST server from `lens.exe`. If a REST/headless API is needed, build it as a separate `lens-server.exe` in Phase 6+. Combining a CLI tool and HTTP server in one binary creates confused responsibility.

### 7.4 Static HTML (`index.html`) — Delete

Root `index.html` is stale. GitHub Pages should serve from `docs/` via MkDocs. Delete `index.html`. [Phase 1]

---

## 8. Backend Architecture Rethink

### 8.1 Header Hygiene — The Contract Debt Problem

**The core issue**: 600+ headers, ~250 with real implementations. The remaining ~350 are "contract headers" — type declarations for future features. This is architecturally sound (API-first) but creates:

1. **Build time bloat**: `EngineTestsIncludes.h` includes ALL Engine headers. Compiling tests touches 600+ files.
2. **Size confusion**: Marketing claims "25 decoders" but many are thin wrappers around external libs.
3. **Maintenance overhead**: Every sprint adds ~10 contract headers. At this rate, v45.0 would have 1,000+ headers.

**Decision (ADR A32)**: Freeze contract-header creation. From v40.0 forward:
- New contracts require a matching `.cpp` stub within the same sprint
- Contracts without implementation after 2 major versions are deleted
- Header audit script (`Audit-Headers.ps1`) must track contract vs. implemented ratio

### 8.2 Decoder Architecture — Simplify to 5 Families

Current: 8 directories with 200+ decoder headers.

v8.0 target: **5 decoder families**, each with a clear owner library:

| Family | Directory | Primary Library | Formats |
|---|---|---|---|
| **Image** | `Decoders/Image/` | WIC + libjpeg-turbo + libspng + libwebp + tinyexr | JPEG, PNG, BMP, GIF, TIFF, WebP, ICO, TGA, PPM, QOI, EXR, HDR, DDS |
| **Modern** | `Decoders/Modern/` | libheif + libjxl + libavif | HEIC, AVIF, JXL, JPEG 2000 |
| **Camera** | `Decoders/Camera/` | LibRaw | All camera RAW formats (100+ extensions) |
| **Document** | `Decoders/Document/` | MuPDF + GDI+ + FreeType | PDF, SVG, Font, Office docs |
| **Media** | `Decoders/Media/` | Media Foundation + libarchive | Video, Audio, Archives, Comic books |

**Removed from v7.0**: Separate `Vector/` and `Specialized/` families (merged into Document and Image).

### 8.3 Pipeline — 8-Stage (Simplified from 9)

| Stage | Action | Owner |
|---|---|---|
| 1 | **Format Detection** — magic bytes + extension | `FormatDetector` |
| 2 | **Cache Check** — L1 memory, L2 disk | `CacheManager` |
| 3 | **Decoder Selection** — registry dispatch by LENSTYPE | `DecoderRegistry` |
| 4 | **Decode** — format-specific decoder | `ILensDecoder` |
| 5 | **Color Management** — lcms2 ICC profile application | `IccProfileManager` |
| 6 | **Resize** — Lanczos downscale to requested `cx` | `ThumbnailResizer` |
| 7 | **Cache Write** — async background write to L1/L2 | `CacheWriter` |
| 8 | **Bitmap Delivery** — HBITMAP ARGB return | COM layer |

**Change from v7.0**: Removed separate "Header Validation" stage. Validation happens inside Format Detection (stage 1). The "Fallback" stage was not a real pipeline stage — it's error handling, not a stage.

### 8.4 GPU Pipeline — Radical Simplification (ADR A33)

**v7.0 had 95 GPU headers. v8.0 kills 85 of them.**

The GPU pipeline should start with ONE thing: **D3D11 texture blit for resize**. This replaces GDI+ `DrawImage` for the resize step and delivers measurable performance improvement.

| Phase | What Ships | Files Needed |
|---|---|---|
| 2 | D3D11 device init + texture blit resize | 3 files: `D3D11Device.h`, `D3D11Resizer.h`, `D3D11Resizer.cpp` |
| 4 | D3D11 JPEG hardware decode (DXVA2) | +2 files |
| 6 | Vulkan compute resize (if D3D11 insufficient) | +3 files |

**All other GPU headers (90+ files)** — NVDEC, AMF, QuickSync, WebGPU, Metal, tensor acceleration, neural codec, path tracing, ray tracing — are **deleted** or archived to `docs/archive/gpu-research/`. They can be re-created when there is an actual implementation plan with a build target.

### 8.5 AI Modules — Delete (ADR A34)

**v7.0 had 45 AI module headers. v8.0 deletes all of them.**

Rationale:
- Zero model weights exist in the repo or any distribution artifact
- No ONNX Runtime is linked or shipped
- AI inference in a shell extension DLL is a security anti-pattern
- Every AI header adds compile time without any runtime value
- These headers have been "research phase" for 10+ major versions

If AI features are ever needed, they should be:
1. In a separate process (`lens-ai.exe`)
2. With explicit user opt-in
3. With real model weights downloaded separately
4. Not linked into `LENSShell.dll`

Archive AI headers to `docs/archive/ai-research/` for reference.

### 8.6 Concurrency Model

| Component | Current | Target |
|---|---|---|
| Decoder dispatch | Thread pool | `std::jthread` + `std::stop_token` (C++20/23) |
| Cache reads | SRWLOCK | `std::shared_mutex` + `std::expected` error path |
| Cache writes | Synchronous (blocks decode thread) | Async writer thread with bounded queue |
| Parallel I/O | Single file at a time | Directory readahead N=8 (H8) |
| GPU upload | N/A | Zero-copy: decode directly into D3D11 staging texture |

### 8.7 Cache Architecture

| Tier | Storage | Max Size | Eviction | Status |
|---|---|---|---|---|
| L1 | `std::unordered_map<pHash, HBITMAP>` in-process | 64 MB | LRU | ⚠️ Partial |
| L2 | SQLite BLOB + PNG in `%LOCALAPPDATA%\ExplorerLens\cache\` | 512 MB | LRU + mtime | ⚠️ Contract |

**Decision**: Remove L3 "smart preview" tier from plan. Smart previews (2560px JPEG proxies) are a photo management feature, not a shell extension feature. If needed later, add to `lens-server.exe` scope.

---

## 9. API Design

### 9.1 COM Interface (Shell ↔ Engine)

```cpp
// Stable ABI, version 0x00010000
interface ILensDecoder : IUnknown {
    HRESULT Decode(IStream* pStream, UINT cx, HBITMAP* phbmp);
    HRESULT GetFormatInfo(LENSFORMAT_INFO* pInfo);
    HRESULT Cancel();  // NEW in v8.0 — bound to std::stop_token
};
```

### 9.2 REST API — Moved to Separate Binary (Phase 6+)

Removed from `lens.exe`. If needed, `lens-server.exe` will expose:

| Method | Path | Description |
|---|---|---|
| GET | `/v1/thumbnail?path={}&size={}` | Decode + return PNG |
| GET | `/v1/formats` | Supported format list |
| GET | `/v1/health` | Liveness + version |

Transport: HTTP/2 (WinHTTP). Auth: mTLS only.

### 9.3 Plugin SDK (C ABI)

```c
// plugin_api.h — stable C ABI
typedef struct {
    uint32_t api_version;       // 0x00010000
    const char* format_id;      // "image/avif"
    const char* display_name;   // "AVIF Decoder"
    HRESULT (*probe)(const uint8_t* header, uint32_t len); // NEW — magic byte check
    HRESULT (*decode)(IStream*, UINT cx, HBITMAP* out);
    void    (*on_unload)(void);
} LensPluginV1;
```

Added `probe()` function for format detection without full decode.

### 9.4 Error Design (C++23)

```cpp
// Engine-internal: use std::expected
std::expected<DecodedBitmap, EngineError> Decode(IStream* pStream, UINT cx);

// COM boundary: convert to HRESULT
HRESULT IThumbnailProvider::GetThumbnail(UINT cx, HBITMAP* phbmp, WTS_ALPHATYPE* pdwAlpha);
```

`EngineError` carries: `HRESULT hr`, `std::string_view context`, `std::source_location loc`.

---

## 10. External Libraries & Third-Party APIs

### Remove List

| Library | Reason | When |
|---|---|---|
| stb (vcpkg) | stb_image remains as emergency fallback only; remove from primary decode paths | Phase 2 |

### Keep List (with version pins)

| Library | Version | Purpose | License | Status |
|---|---|---|---|---|
| zlib | 1.3.1 | Deflate decompression | zlib | ✅ Linked |
| LZ4 | 1.10.0 | Fast compression for L1 cache | BSD-2 | ✅ Linked |
| zstd | 1.5.7 | High-ratio compression for L2 cache | BSD-3 | ✅ Linked |
| libwebp | 1.5.0 | WebP decode | BSD-3 | ✅ Linked |
| minizip-ng | 4.0.10 | ZIP archive handling | zlib | ✅ Linked |
| LibRaw | 0.21.2 | Camera RAW decode | LGPL-2.1 | ✅ Linked |
| dav1d | 1.5.1 | AV1 video decode | BSD-2 | ✅ Linked |
| libde265 | 1.0.15 | HEVC decode (HEIF) | LGPL-3 | ✅ Linked |
| libheif | 1.19.5 | HEIF/HEIC container | LGPL-3 | ✅ Linked |
| libjxl | 0.11.1 | JPEG XL decode | BSD-3 | ✅ Linked |
| libavif | 1.3.0 | AVIF container | BSD-2 | ✅ Linked |
| libarchive | 3.7.6 | Multi-format archives | BSD-2 | ✅ Linked |
| MuPDF | 1.24.11 | PDF page rasterization | AGPL-3 | ✅ Linked |
| tinyexr | 1.0.4 | OpenEXR decode | BSD-3 | ✅ Bundled |
| pugixml | 1.14 | XML parsing | MIT | ✅ Linked |
| xxhash | latest | Fast hashing for cache keys | BSD-2 | ✅ Linked |
| nlohmann-json | latest | JSON parsing | MIT | ✅ Linked |
| Catch2 | 3.7.1 | Unit test framework | BSL-1 | ✅ Test only |
| Google Benchmark | 1.9.1 | Performance benchmarks | Apache-2 | ✅ Test only |

### Add List (Phase-gated)

| Library | Purpose | Phase | License |
|---|---|---|---|
| libjpeg-turbo 3.1 | Replace stb for JPEG; SIMD-accelerated | 2 | BSD-3 |
| libspng 0.7 | Replace stb for PNG; ASAN-clean | 2 | BSD-2 |
| lcms2 2.16 | ICC color management | 3 | MIT |
| FreeType 2.13.3 | Font rasterization (TTF/OTF preview) | 3 | FTL |
| DirectXTex | DDS texture loading (already in vcpkg) | 3 | MIT |

### Evaluate List (Research, No Commitment)

| Library | Purpose | Decision Gate |
|---|---|---|
| resvg 0.44 | SVG rasterization (Rust, C-ABI) | Only if GDI+ SVG is insufficient |
| OpenColorIO 2.4 | ACES/DCI-P3 wide gamut | Only if lcms2 insufficient |
| nghttp2 1.62 | HTTP/2 for REST server | Only if lens-server.exe proceeds |

### Windows APIs (No External Dependency)

| API | Purpose | Status |
|---|---|---|
| Windows Imaging Component (WIC) | Native codec passthrough for supported formats | ✅ Active |
| Windows Media Foundation | Video keyframe extraction | ✅ Active |
| GDI+ | Bitmap manipulation, fallback resize | ✅ Active |
| DirectWrite | Font enumeration + rendering | ⚠️ Partial |
| ETW | Structured telemetry | ✅ Active |
| D3D11 | GPU texture blit (resize acceleration) | ❌ Phase 2 |
| DXVA2 | Hardware JPEG/HEIC decode | ❌ Phase 4 |

---

## 11. Database & Persistence Strategy

### Primary Store: SQLite (retained)

**Rationale**: SQLite is correct for an in-process Shell Extension. No network database. No file-based key-value stores. WAL mode for concurrent reads.

### Schema v8.0 (simplified from v7.0)

```sql
-- Thumbnail cache index (core table)
CREATE TABLE thumbnail_cache (
    id          INTEGER PRIMARY KEY,
    file_path   TEXT NOT NULL,
    file_mtime  INTEGER NOT NULL,
    file_size   INTEGER NOT NULL,
    phash       BLOB,              -- 8-byte perceptual hash
    thumb_path  TEXT,              -- path to .png in cache dir
    width       INTEGER,
    height      INTEGER,
    decoder_id  TEXT,
    decode_ms   INTEGER,
    created_at  INTEGER DEFAULT (unixepoch()),
    last_hit    INTEGER DEFAULT (unixepoch())
);
CREATE INDEX idx_path  ON thumbnail_cache(file_path);
CREATE INDEX idx_mtime ON thumbnail_cache(file_mtime);

-- Format error tracking (H48 — decode failure analytics)
CREATE TABLE decode_errors (
    id          INTEGER PRIMARY KEY,
    file_ext    TEXT NOT NULL,
    decoder_id  TEXT NOT NULL,
    error_code  INTEGER,
    error_msg   TEXT,
    created_at  INTEGER DEFAULT (unixepoch())
);
CREATE INDEX idx_ext ON decode_errors(file_ext);
```

**Removed from v7.0 schema**:
- `smart_previews` table — moved to `lens-server.exe` scope
- `plugins` table — premature without real plugins
- `format_stats` table — merged into `decode_errors` for simplicity
- `file_inode` column — Windows NTFS inode (`BY_HANDLE_FILE_INFORMATION.nFileIndex`) is unreliable across volumes

### Database Location

`%LOCALAPPDATA%\ExplorerLens\catalog.db` — per-user. Configurable via registry key `HKCU\SOFTWARE\ExplorerLens\CacheDir`.

### Pragmas

```sql
PRAGMA journal_mode = WAL;
PRAGMA synchronous = NORMAL;
PRAGMA cache_size = -8000;  -- 8 MB page cache
PRAGMA mmap_size = 67108864; -- 64 MB memory-mapped I/O
```

---

## 12. Infrastructure & Distribution

### Build Infrastructure

| Component | Current | Target |
|---|---|---|
| Build host | GitHub Actions `windows-latest` | Same + self-hosted Arm64 runner (Phase 6) |
| Compiler | MSVC cl.exe 19.50 v145 | Same |
| Build system | CMake 4.3 + Ninja 1.13 | Same |
| Package manager | vcpkg (manifest mode) | Same; audit unused packages |
| Artifact storage | GitHub Releases | Same |
| Build cache | sccache (optional) | sccache (required in CI) |

### Distribution Channels

| Channel | Format | Status | Phase |
|---|---|---|---|
| GitHub Releases | `.msi` + `.zip` (portable) | ✅ Done | Done |
| Scoop bucket | `scoopfile.json` | ✅ Done | Done |
| WinGet manifest | `winget` YAML | ⚠️ Exists, not published | 3 |
| Chocolatey | `.nupkg` | ❌ Not started | 4 |
| MSIX / Microsoft Store | Sparse package | ❌ Not started | 5 |

**Removed from plan**: NuGet (Engine SDK — premature), Container/Docker (COM + Docker = incompatible), Maven/RubyGems (no consumers).

### Install Footprint Target

| Component | Current | Target v42.0 |
|---|---|---|
| LENSShell.dll | ~2,940 KB | < 2,500 KB |
| LENSManager.exe | ~400 KB | < 350 KB |
| External libs | ~8 MB | < 5 MB (prune unused, lazy-load) |
| **Total MSI** | **~12 MB** | **< 8 MB** |

Footprint reduction levers:
1. Remove AI headers → faster compile, no dead symbols
2. Remove unused GPU stubs → smaller static lib
3. Lazy-load libheif/libjxl/libavif (H43) → smaller working set
4. Strip debug info in release (`/DEBUG:NONE` for DLL)
5. Enable ThinLTO (`/LTCG:incremental`) for dead-code elimination

---

## 13. CI/CD Pipeline

31 workflows exist. Honest status assessment:

### Active & Tested (13 workflows — must pass for merge)

| # | Workflow | Trigger | Purpose |
|---|---|---|---|
| 1 | `build.yml` | push/PR | MSVC x64 Release build |
| 2 | `catch2-tests.yml` | push | Catch2 test suite |
| 3 | `codeql.yml` | push/PR + weekly | SAST security analysis |
| 4 | `code-quality.yml` | push | Code metrics |
| 5 | `dependency-review.yml` | PR | License & vulnerability scan |
| 6 | `performance-regression-gate.yml` | push | Benchmark regression gate |
| 7 | `ssim-validation.yml` | weekly | Thumbnail visual quality gate |
| 8 | `release.yml` | tag push | GitHub Release automation |
| 9 | `release-drafter.yml` | push | Auto release notes |
| 10 | `publish-packages.yml` | release | Package publishing |
| 11 | `pr-checks.yml` | PR | Style + lint |
| 12 | `pages.yml` | push | GitHub Pages deploy |
| 13 | `auto-label.yml` | PR | PR auto-labeling |

### Active but Not Critical (7 workflows)

| # | Workflow | Status | Notes |
|---|---|---|---|
| 14 | `ci-matrix.yml` | ⚠️ Active | Multi-config matrix |
| 15 | `nightly.yml` | ⚠️ Active | Full nightly test suite |
| 16 | `asan-nightly.yml` | ⚠️ Active | ASAN full suite |
| 17 | `oss-fuzz-nightly.yml` | ⚠️ Active | Fuzzer integration |
| 18 | `binary-size.yml` | ⚠️ Active | Size tracking |
| 19 | `stale.yml` | ⚠️ Active | Issue hygiene |
| 20 | `sync-labels.yml` | ⚠️ Active | Label management |

### Stubs / Planned (11 workflows — not active)

| # | Workflow | Phase | Notes |
|---|---|---|---|
| 21 | `coverage.yml` | 2 | Manual trigger only |
| 22 | `sanitizer-ci.yml` | 4 | ASAN + UBSAN stub |
| 23 | `fuzz-ci.yml` | 4 | libFuzzer stub |
| 24 | `corpus-validation.yml` | 3 | Weekly corpus check |
| 25 | `docs-validation.yml` | 2 | MkDocs build check |
| 26 | `toolchain-verify.yml` | 2 | Weekly toolchain audit |
| 27 | `sbom-attestation.yml` | 3 | SBOM generation |
| 28 | `pin-actions.yml` | 2 | SHA-pin automation |
| 29 | `screenshot-regression.yml` | 4 | Visual regression |
| 30 | `devcontainer-test.yml` | 3 | Devcontainer CI |
| 31 | `notify-failure.yml` | 2 | Failure notifications |

### Pipeline Quality Gates (must-pass for merge to main)

1. `build.yml` — 0 errors, 0 warnings (requires `/WX` restoration)
2. `catch2-tests.yml` — 100% pass rate
3. `codeql.yml` — 0 high/critical
4. `performance-regression-gate.yml` — no metric > 10% regression
5. `ssim-validation.yml` — SSIM ≥ 0.95

---

## 14. Testing & Quality Strategy

### Test Count — Reconcile First

| Source | Claims |
|---|---|
| README badge | 5,045 |
| `BuildValidation.h` `UnitTestCount` | 4,664 |
| copilot-instructions.md | ~5,045 |

**Action**: Audit actual `RUN_TEST()` count in `EngineTests.cpp`. Update all sources to match reality.

### Test Count Targets

| Version | Test Count | Corpus Files | SSIM Gate |
|---|---|---|---|
| v40.0 | 5,500 (verified) | 150 CC0 files | 0.95 |
| v42.0 | 6,500 | 300 CC0 files | 0.95 |
| v44.0 | 7,500 | 500 CC0 files | 0.97 |
| v46.0 | 8,500 | 750 CC0 files | 0.97 |

### Test Layers (7 — simplified from 9)

| Layer | Framework | What It Tests |
|---|---|---|
| 1. Unit | Custom `TEST()` macros | Pure functions, zero I/O |
| 2. Property | Catch2 | Invariants, edge cases, generated inputs |
| 3. Integration | Custom + real corpus files | End-to-end decode with real file I/O |
| 4. Visual regression | SSIM validation CI | Thumbnail quality vs reference images |
| 5. Performance | Google Benchmark | Latency/throughput vs `baseline.json` |
| 6. Fuzz | OSS-Fuzz + libFuzzer | Decoder crash/hang detection |
| 7. Sanitizer | ASAN + UBSAN | Memory safety, undefined behavior |

**Removed from v7.0**: Separate TSAN/MSAN layers (impractical on MSVC; Clang-only). "Corpus real-decode" merged with Integration layer.

### Test File Placement (mandatory — Rule #18)

- New `TEST()` bodies → `Engine/Tests/EngineTests_Platform.cpp`
- New `extern void Runner()` → `Engine/Tests/EngineTestsExterns.h`
- New `RUN_TEST()` → `Engine/Tests/EngineTests.cpp`
- New `#include` → `Engine/Tests/EngineTestsIncludes.h`

---

## 15. Security Stack

### 15 Security Controls (prioritized)

| ID | Control | Status | Phase | Priority |
|---|---|---|---|---|
| S1 | `/WX` restored — zero warnings enforced | ❌ Broken | 1 | P0 |
| S2 | Per-format decode memory budget (H42) | ❌ | 2 | P0 |
| S3 | Decode timeout 500ms (H39) | ❌ | 2 | P0 |
| S4 | Magic-byte validation before decode (H6) | ⚠️ Partial | 2 | P0 |
| S5 | ASAN + fuzzer coverage on all active decoders | ✅ Done | Done | P0 |
| S6 | CodeQL SAST on every push | ✅ Done | Done | P0 |
| S7 | Dependency review on every PR | ✅ Done | Done | P0 |
| S8 | CycloneDX SBOM on every release | ✅ Done | Done | P1 |
| S9 | Crash telemetry opt-in (no silent telemetry) | ✅ Done | Done | P1 |
| S10 | Plugin trust chain validator (code signing) | ✅ Done | Done | P1 |
| S11 | EV code signing pipeline | ⚠️ Partial | 3 | P1 |
| S12 | SLSA Level 2 provenance (H32) | ❌ | 3 | P1 |
| S13 | OSSF Scorecard > 7.0 | ⚠️ Active | 2 | P2 |
| S14 | OOM kill protection (H33) | ❌ | 2 | P2 |
| S15 | AppContainer sandbox for plugins | ⚠️ Contract | 5 | P2 |

---

## 16. Observability Stack

### ETW Events (structured, typed)

| Provider | Events |
|---|---|
| ExplorerLens-Engine | Decode start/end, cache hit/miss, error, format detected |
| ExplorerLens-Shell | COM activate, IThumbnailProvider call, cancellation |

### Diagnostic CLI

`lens.exe info <file>` outputs:
- Detected format (magic bytes + extension)
- Decoder that would be selected
- Estimated decode time
- Cache status (hit/miss/stale)
- ICC profile present (yes/no)

### Metrics (Phase 6+ — lens-server.exe only)

Prometheus-format counters if REST API is implemented. Not in shell extension scope.

---

## 17. Documentation Strategy

### Reduce to 30 Maintained Documents

v7.0 targeted 45. Still too many. A 1-person project cannot maintain 45 documents at publication quality.

### 3 Tiers (simplified from 4)

| Tier | Audience | Files | Count |
|---|---|---|---|
| T1 — User | End users | README, USER_GUIDE, QUICK_START, TROUBLESHOOTING, CHANGELOG | 5 |
| T2 — Developer | Contributors | ARCHITECTURE, DEVELOPER_GUIDE, CONTRIBUTING, PERFORMANCE, RELEASE_PROCESS, BUILD_QUICK_REFERENCE, TESTING_GUIDE, DECODER_IMPLEMENTATION_STATUS | 8 |
| T3 — Design | Deep contributors | ROADMAP, 15 active ADRs (retire stale ones), TOOLING | 17 |
| **Total** | | | **30** |

### Docs to Retire

- ADRs from v1–v10 era that describe decisions since superseded → archive
- Per-format documentation pages → consolidate into `DECODER_IMPLEMENTATION_STATUS.md`
- `docs/archive/` directory contents → keep as-is but remove from nav
- `TOOLING.md` → merge into `DEVELOPER_GUIDE.md`
- `LOCAL_VERIFICATION.md` → merge into `TESTING_GUIDE.md`

### Documentation Rules (enforced)

1. No `TODO`, `planned`, or `coming soon` in T1 docs
2. Version number in max 3 docs (README, USER_GUIDE, CHANGELOG) — not sprayed everywhere
3. Every doc has `Last reviewed: YYYY-MM-DD` footer
4. `Audit-Headers.ps1` checks doc version references

---

## 18. Tools & Versions Matrix

| Tool | Version | Min Required | Notes |
|---|---|---|---|
| MSVC cl.exe | 19.50 (v145) | 19.40 | C++23 requires 19.38+ |
| CMake | 4.3.1 | 3.25 | Presets v6 format |
| Ninja | 1.13.2 | 1.11 | |
| vcpkg | 2025-01 | 2024-06 | manifest mode only |
| Windows SDK | 10.0.26100.0 | 10.0.19041.0 | |
| Clang | 18.1.8 | 18.0 | CI-only |
| Python | 3.12.4 | 3.10 | build scripts only |
| PowerShell | 7.4.3 | 7.2 | |
| Git | 2.45.2 | 2.30 | |
| WiX Toolset | 6.0.2 | 4.0 | MSI packaging |
| NASM | 3.01 | 2.15 | libjpeg-turbo assembly |
| Meson | 1.10.2 | 1.0 | dav1d build |
| NuGet | 7.3.0 | 6.0 | Package restore |
| Catch2 | 3.7.1 | 3.5 | Test framework |
| Google Benchmark | 1.9.1 | 1.8 | Perf framework |

### Build Scripts Audit

| Script | Status | Action |
|---|---|---|
| `Build-MSVC.ps1` | ✅ Active | Keep — primary build driver |
| `Bump-Version.ps1` | ✅ Active | Keep — version automation |
| `Build-All-And-Package.ps1` | ✅ Active | Keep — release packaging |
| `Build-LENSShell-MSBuild.ps1` | ✅ Active | Keep — shell DLL build |
| `Audit-Headers.ps1` | ✅ Active | Keep — header hygiene |
| `Audit-Repo.ps1` | ✅ Active | Keep — repo health check |
| `Check-Build-Status.ps1` | ✅ Active | Keep |
| `Test-Build-Environment.ps1` | ✅ Active | Keep — toolchain verification |
| `Generate-SBOM.ps1` | ✅ Active | Keep |
| `Generate-Changelog.ps1` | ✅ Active | Keep |
| `Monitor-Build-Logs.ps1` | ✅ Active | Keep |
| `Serve-Docs.ps1` | ✅ Active | Keep |
| `Setup-Vcpkg.ps1` | ✅ Active | Keep |
| `Build-With-Monitoring.ps1` | ⚠️ Evaluate | Merge into `Build-MSVC.ps1` if redundant |
| `Verify-Complete-Build.ps1` | ⚠️ Evaluate | Merge into `Check-Build-Status.ps1` |
| `ExplorerLens-Profile.ps1` | ⚠️ Evaluate | Verify usage |

### Files to Delete (Phase 1)

| File/Dir | Reason |
|---|---|
| `index.html` (root) | Stale GitHub Pages; migrated to `docs/` |
| `scripts/` directory | Empty or unused stubs |
| `tools/` directory | Verify contents; likely empty |
| `src/` directory | Verify contents; may be empty |

---

## 19. Refactor / Rewrite / Delete / Add Register

### DELETE (confirmed dead code)

| Item | File Count | Reason |
|---|---|---|
| AI module headers | 45 files | No model weights, no inference runtime, no user value |
| GPU research stubs | ~85 files | Keep only D3D11Device + D3D11Resizer |
| Plugin marketplace V1–V4 | ~20 files | Zero plugins exist; keep V5 contract only |
| Enterprise contract headers without `.cpp` | ~30 files | Contract debt — reintroduce when implementing |
| `index.html` (root) | 1 file | Stale, replaced by docs/ |
| `scripts/` directory | ? files | Verify empty, then delete |
| `tools/` directory | ? files | Verify empty, then delete |

### REWRITE (fundamentally broken)

| Item | Reason | Phase |
|---|---|---|
| `IThumbnailProvider` error path | Returns `S_OK` with blank bitmap on failure; should return `E_FAIL` | 2 |
| CMakeLists.txt `/WX-` flag | Must be `/WX` | 1 |
| stb_image JPEG/PNG paths | Replace primary paths with libjpeg-turbo + libspng | 2 |

### REFACTOR (correct but needs improvement)

| Item | Reason | Phase |
|---|---|---|
| `EngineTestsIncludes.h` | Includes ALL 600+ headers; split to per-test-file includes | 3 |
| Error returns in decoders | Mix of HRESULT, bool, exceptions; standardize on `std::expected` | 3 |
| Cache write path | Synchronous write on decode thread; move to async background writer | 2 |
| `PerfRegressionGate.h` namespace | Uses `ExplorerLens` not `ExplorerLens::Engine` | 2 |

### ADD (new capabilities)

| Item | Description | Phase |
|---|---|---|
| `Engine/Core/IccProfileManager.h/.cpp` | lcms2 wrapper for ICC color management | 3 |
| `Engine/Core/AsyncDecodeToken.h` | `std::stop_token` wrapper for cancel-aware decode | 2 |
| `Engine/Core/DecodeTimeout.h` | 500ms hard timeout with fallback | 2 |
| `Engine/GPU/D3D11Device.h/.cpp` | D3D11 device initialization | 2 |
| `Engine/GPU/D3D11Resizer.h/.cpp` | D3D11 texture blit resize | 2 |
| `packaging/winget/ExplorerLens.yaml` | WinGet manifest (publish) | 3 |

---

## 20. 10-Phase Plan to Best-in-Class

### Phase 1 — Discipline Restoration (v39.x → v40.0)

**Theme**: Fix the foundation before building on it.

**Exit criteria**:
- [ ] `/WX` restored in `Engine/CMakeLists.txt` — zero warnings confirmed
- [ ] Test count reconciled across README, BuildValidation.h, copilot-instructions.md
- [ ] `index.html` deleted, GitHub Pages serves from `docs/`
- [ ] `scripts/`, `tools/`, `src/` directories verified and cleaned
- [ ] AI module headers archived (45 files → `docs/archive/ai-research/`)
- [ ] Unused GPU stubs archived (85 files → `docs/archive/gpu-research/`)
- [ ] C++23 `/std:c++23` confirmed working (already in CMakeLists)
- [ ] vcpkg.json audited for unused packages (remove `stb` from primary deps)

**Sprints**: S301–S310

### Phase 2 — Correctness & Robustness (v40.x → v41.0)

**Theme**: Make every decode correct, safe, and cancellable.

**Exit criteria**:
- [ ] Cancel-aware decode via `std::stop_token` (H5, H34)
- [ ] OOM kill protection (H33)
- [ ] Decode timeout 500ms with fallback (H39)
- [ ] Per-format memory budget (H42)
- [ ] Async placeholder thumbnail (H1)
- [ ] libjpeg-turbo replaces stb_image for JPEG
- [ ] libspng replaces stb_image for PNG
- [ ] WIC passthrough for standard formats (H24)
- [ ] Error return `E_FAIL` (not blank bitmap) on decode failure
- [ ] D3D11 texture blit resize (first GPU-accelerated operation)
- [ ] Embedded JPEG fast-path for RAW files (H7)
- [ ] Corpus expanded to 150 CC0 files
- [ ] Parallel I/O readahead N=8 (H8)
- [ ] Cache write async (background thread)
- [ ] Decode error telemetry per decoder (H48)

**Sprints**: S311–S340

### Phase 3 — Quality & Polish (v41.x → v42.0)

**Theme**: Color-correct, fast, beautiful thumbnails.

**Exit criteria**:
- [ ] ICC color management end-to-end via lcms2 (H12, H30)
- [ ] LENSManager WTL dark mode completed (using existing `DarkModeController.h`)
- [ ] LENSManager high-DPI dynamic awareness
- [ ] EV code signing in CI
- [ ] SLSA L2 provenance on releases (H32)
- [ ] WinGet manifest published
- [ ] SSIM gate raised to 0.97
- [ ] Corpus expanded to 300 CC0 files
- [ ] FreeType font preview for TTF/OTF
- [ ] DLL size < 2,500 KB
- [ ] Total MSI < 8 MB
- [ ] SIMD (AVX2) Lanczos resize (H41)
- [ ] Lazy library loading for libheif/libjxl/libavif (H43)

**Sprints**: S341–S380

### Phase 4 — Depth & Safety (v42.x → v43.0)

**Theme**: Deep format support, sanitizer-clean.

**Exit criteria**:
- [ ] DXVA2 hardware JPEG decode
- [ ] Process isolation option for decoders (H36)
- [ ] IPropertyStore for EXIF in Explorer details pane (H46)
- [ ] UBSAN clean in CI
- [ ] Corpus expanded to 500 CC0 files
- [ ] 7,500 tests
- [ ] Chocolatey package published

**Sprints**: S381–S420

### Phase 5 — Distribution & Enterprise (v43.x → v44.0)

**Theme**: Ship everywhere, serve enterprises.

**Exit criteria**:
- [ ] MSIX sparse package for Windows 11 (H21, H25)
- [ ] LENSManager WinUI 3 evaluation (migrate if justified)
- [ ] `lens-server.exe` REST API (HTTP/2, mTLS) if demand exists
- [ ] ADMX Group Policy schema for enterprise deployment
- [ ] Thumbnail cache warming on first install (H47)
- [ ] Plugin SDK with 1 real community plugin

**Sprints**: S421–S460

### Phase 6 — Platform Expansion (v44.x → v45.0)

**Theme**: Arm64 + broad package ecosystem.

**Exit criteria**:
- [ ] Native Arm64 EC build green (H31)
- [ ] Vulkan compute resize (if D3D11 insufficient)
- [ ] Reproducible builds (H14)
- [ ] Static plugin catalog (`plugins.explorerlens.io`)
- [ ] 8,500 tests
- [ ] Competitor matrix score ≥ 30/36 in Category A

**Sprints**: S461–S500

### Phase 7 — Plugin Ecosystem (v45.x → v46.0)

**Theme**: Third-party decoder plugins.

**Exit criteria**:
- [ ] `.lenspkg` format signed + installable via LENSManager
- [ ] AppContainer sandbox for plugins
- [ ] 3 community plugins published
- [ ] Plugin SDK documentation complete

**Sprints**: S501–S540

### Phase 8 — Advanced Features (v46.x → v47.0)

**Theme**: Smart features with clear user value.

**Exit criteria**:
- [ ] Smart crop for thumbnail composition (server-side, opt-in)
- [ ] Progressive JPEG/HEIC streaming decode (H27)
- [ ] Archive cover art extraction (H30)

**Sprints**: S541–S580

### Phase 9 — Linux & FreeDesktop (v47.x → v48.0)

**Theme**: Cross-platform thumbnails.

**Exit criteria**:
- [ ] DBus thumbnailer protocol (H29)
- [ ] FreeDesktop thumbnail spec compliance
- [ ] Linux Nautilus integration CI green
- [ ] 750 corpus files

**Sprints**: S581–S620

### Phase 10 — macOS Quick Look (v48.x → v50.0)

**Theme**: Apple platform parity.

**Exit criteria**:
- [ ] macOS Quick Look extension with real decode
- [ ] Native Apple Silicon (not Rosetta)
- [ ] macOS CI green
- [ ] v50.0 = "Best-in-Class" milestone

**Sprints**: S621–S680

---

## 21. Success Metrics & Exit Criteria

### Performance KPIs

| Metric | Current (v39.2) | Target v42.0 | Target v45.0 |
|---|---|---|---|
| Single thumbnail p95 | ~17 ms | < 12 ms | < 8 ms |
| Batch throughput | ~235 img/sec | > 350 img/sec | > 500 img/sec |
| Cache hit latency | < 5 ms | < 3 ms | < 2 ms |
| Peak memory per decode | ~40 MB | < 25 MB | < 15 MB |
| Install footprint | ~12 MB | < 8 MB | < 6 MB |
| Shell host crash rate | Unknown | < 1/10,000 | < 1/100,000 |
| DLL cold load time | Unknown | < 50 ms | < 30 ms |

### Quality KPIs

| Metric | Current | Target v42.0 | Target v45.0 |
|---|---|---|---|
| Test count | ~5,045 (unverified) | 6,500 (verified) | 8,500 |
| Corpus files | ~106 | 300 | 750 |
| SSIM threshold | 0.95 | 0.97 | 0.97 |
| Sanitizer coverage | ASAN only | + UBSAN | + MSAN (Clang) |
| CI pipeline active | 13/31 | 20/31 | 25/31 |
| OSSF Scorecard | Unknown | > 7.0 | > 8.0 |
| Category A matrix score | 19/36 | 26/36 | 30/36 |

### Category A Score Gains by Phase

| Phase | Key Additions | Score Gain | Projected Total |
|---|---|---|---|
| 1 | (cleanup only) | +0 | 19 |
| 2 | Cancel-aware, GPU resize, OOM protect, WIC passthrough | +3 | 22 |
| 3 | ICC color, dark mode, code signing, SLSA L2 | +4 | 26 |
| 4 | DXVA2 decode, UBSAN clean, IPropertyStore | +2 | 28 |
| 5 | MSIX Store, enterprise deploy | +1 | 29 |
| 6 | Arm64, reproducible builds | +2 | 31 |

---

## 22. ADR Log v8.0

### Retained from v7.0 (ADRs A1–A28)

| ID | Title | Status |
|---|---|---|
| A1 | COM IThumbnailProvider as primary Windows interface | Accepted |
| A2 | C++20 as language standard | Superseded by A23 |
| A3 | MSVC v143/v145 as sole production compiler | Accepted |
| A4 | CMake + Ninja as build system | Accepted |
| A5 | vcpkg in manifest mode | Accepted |
| A6 | ETW as observability transport | Accepted |
| A7 | Contract-header model for API-first development | **Amended by A32** |
| A8 | Custom TEST/RUN_TEST/ASSERT macros | Accepted |
| A9 | Google Benchmark for performance gates | Accepted |
| A10 | Catch2 v3 for property-based tests | Accepted |
| A11 | LibRaw for camera RAW decode | Accepted |
| A12 | PDFium for PDF page rendering | Accepted |
| A13 | resvg (Rust, C-ABI) for SVG rasterization | Accepted (evaluate) |
| A14 | SQLite as sole persistence store | Accepted |
| A15 | 8-stage decode pipeline (simplified from 9) | **Amended** |
| A16 | SSIM as thumbnail quality gate | Accepted |
| A17 | OSS-Fuzz for decoder fuzzing | Accepted |
| A18 | ASAN as memory safety gate | Accepted |
| A19 | CycloneDX SBOM on every release | Accepted |
| A20 | AppContainer for plugin sandboxing | Accepted (Phase 5) |
| A21 | mTLS for REST API (lens-server.exe only) | Accepted (Phase 6) |
| A22 | ADMX Group Policy for enterprise | Accepted (Phase 5) |
| A23 | C++23 selective adoption (no modules) | **Amended** |
| A24 | Rust research lane terminated | Accepted |
| A25 | WinUI 3 for LENSManager | **Deferred to Phase 5** by A30 |
| A26 | WinGet manifest in Phase 3 | Accepted |
| A27 | lcms2 as ICC engine | Accepted |
| A28 | UnRAR SDK: feature-flag only | Accepted |

### New in v8.0 (ADRs A29–A34)

| ID | Title | Status | Date |
|---|---|---|---|
| A29 | Restore `/WX` — zero-warnings is non-negotiable | **Accepted** | 2026-05-12 |
| A30 | Defer WinUI 3; fix WTL dark mode first | **Accepted** | 2026-05-12 |
| A31 | Remove REST server from `lens.exe`; separate binary if needed | **Accepted** | 2026-05-12 |
| A32 | Freeze contract-header creation; require `.cpp` within same sprint | **Accepted** | 2026-05-12 |
| A33 | Radical GPU simplification: D3D11 resize only; delete 85 stub headers | **Accepted** | 2026-05-12 |
| A34 | Delete AI module headers; reintroduce only with real model + separate process | **Accepted** | 2026-05-12 |

---

## 23. Decisions Reversed from v7.0

| v7.0 Decision | v8.0 Reversal | Rationale |
|---|---|---|
| WinUI 3 for LENSManager in Phase 3 | **Defer to Phase 5; fix WTL dark mode first** | WinUI 3 XAML Islands + COM = high complexity for settings UI; WTL already has `DarkModeController.h` |
| C++23 modules (`import std;`) | **Defer modules; use other C++23 features** | MSVC IntelliSense regressions; modules not stable enough |
| REST server inside `lens.exe` | **Separate `lens-server.exe` if needed** | CLI ≠ server; confused responsibility |
| 45 documentation targets | **30 maintained docs** | 1-person team cannot maintain 45 docs at quality |
| 95 GPU header files | **Keep 3, delete/archive 92** | Zero GPU pixels rendered in 39 versions |
| 45 AI module headers | **Delete all, archive** | Zero model weights, zero inference, zero user value |
| Remove stb_image entirely | **Keep as emergency fallback with `[FALLBACK]` tag** | Removing all fallback paths is risky |
| L3 smart preview cache tier | **Remove from shell extension scope** | Smart previews = photo management, not shell extension |
| Plugin marketplace V5 | **Suspend; build 1 real plugin first** | Marketplace without plugins is dead code |
| 7 decoder families | **5 families** | Simpler architecture, clearer ownership |
| 9-stage pipeline | **8 stages** | Removed redundant "Header Validation" and "Fallback" pseudo-stages |
| `file_inode` in cache DB | **Removed** | NTFS inode unreliable across volumes |

---

## 24. Sprint Delivery Pipeline S301+

### Phase 1 Sprints (S301–S310): Discipline Restoration

| Sprint | Title | Deliverable |
|---|---|---|
| S301 | Restore `/WX` in Engine/CMakeLists.txt | Fix all existing warnings; `/WX` enabled |
| S302 | Reconcile test count across all docs | Single source of truth for test count |
| S303 | Delete root `index.html`; verify `scripts/`, `tools/`, `src/` dirs | Clean workspace |
| S304 | Archive AI module headers (45 files) | Move to `docs/archive/ai-research/` |
| S305 | Archive unused GPU stubs (85 files) | Move to `docs/archive/gpu-research/`; keep D3D11 core |
| S306 | Audit vcpkg.json — remove unused packages | Leaner dependency manifest |
| S307 | Freeze contract-header creation policy | Update CONTRIBUTING.md with A32 rule |
| S308 | Plugin ecosystem cleanup — keep V5 only | Remove V1–V4 superseded headers |
| S309 | Enterprise contract audit — archive unimplemented | Reduce header count to compiled+tested only |
| S310 | Update ROADMAP, README, BuildValidation.h to v40.0 | Version consistency |

### Phase 2 Preview (S311–S320): Correctness

| Sprint | Title |
|---|---|
| S311 | `AsyncDecodeToken.h` — `std::stop_token` wrapper |
| S312 | `DecodeTimeout.h` — 500ms hard timeout with fallback |
| S313 | Per-format memory budget enforcement |
| S314 | Cancel-aware decode in COM server (`IBindStatusCallback`) |
| S315 | OOM kill protection (`SetProcessWorkingSetSizeEx`) |
| S316 | Async placeholder thumbnail (return last-cached immediately) |
| S317 | Add libjpeg-turbo to build; replace stb JPEG paths |
| S318 | Add libspng to build; replace stb PNG paths |
| S319 | D3D11 device init + texture blit resize |
| S320 | Parallel I/O readahead N=8 |

---

## Appendix A: v7.0 Survivor Registry

Decisions from v7.0 retained unchanged in v8.0:

- **COM CLSID** `9E6ECB90-5A61-42BD-B851-D3297D9C7F39` — immutable
- **Contract-header model** (A7) — retained but amended (A32: requires `.cpp` within sprint)
- **ETW observability** (A6)
- **SQLite persistence** (A14) — schema simplified
- **LibRaw, MuPDF, dav1d, libheif, libjxl, libavif** — all retained
- **Custom TEST/RUN_TEST macros** (A8) — test placement rules unchanged
- **Google Benchmark baseline gates** (A9)
- **SSIM quality gate** (A16) — threshold stays at 0.95 for Phase 1–2
- **OSS-Fuzz + ASAN** (A17, A18)
- **CycloneDX SBOM** (A19)
- **Rust terminated** (A24)
- **Sprint cadence: 10 sprints/session**
- **Zero-warnings build discipline** — NOW ENFORCED (A29)

## Appendix B: Header Debt Metrics

| Category | v39.2 Header Count | Estimated with `.cpp` | Action |
|---|---|---|---|
| Core/ | ~150 | ~80 | Archive contracts without implementation |
| Pipeline/ | ~50 | ~25 | Keep all — pipeline is core |
| Decoders/ | ~200 | ~100 | Keep active decoders; archive planned-only |
| Cache/ | ~50 | ~15 | Simplify to essential cache headers |
| Memory/ | ~40 | ~10 | Archive compactor/optimizer stubs |
| GPU/ | ~100 | ~5 | **Keep 3-5, archive 95** |
| Platform/ | ~15 | ~8 | Keep Win32; archive macOS/Linux stubs |
| AI/ | ~45 | ~0 | **Archive all** |
| Plugin/ | ~60 | ~15 | Keep loader + trust chain; archive marketplace |
| Enterprise/ | ~30 | ~5 | Archive unimplemented contracts |
| **Total** | **~740** | **~263** | **Archive ~477 headers** |

This debt reduction would:
- Reduce compile time ~40%
- Reduce `EngineTestsIncludes.h` from ~233 KB to ~100 KB
- Make the codebase honest about what is implemented vs. planned
- Enable `/WX` restoration without fighting 400 warnings from stub headers

---

*ROADMAP v8.0 "Vega" — ExplorerLens 39.2.0 → 50.0*
*Archived predecessor: `docs/archive/ROADMAP_V7.md`*
*Next review: v41.0 milestone*
