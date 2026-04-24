# ExplorerLens — Strategic Roadmap v6.0 "Rigel"

**Version:** 6.0 — April 2026  
**Current Release:** v39.2.0 "Betelgeuse" (5,045 tests · 45 Catch2 files · 0 errors · 0 warnings)  
**Supersedes:** ROADMAP v5.0 → archived to `docs/archive/ROADMAP_V5.md`  
**Prior archive chain:** v4.0 → `ROADMAP_V4.md`, v3.0 → `ROADMAP_V3.md`, v1/v2 → `docs/archive/`  
**Scope:** Full re-examination — no decision is exempt. Every "Keep" verdict from v5.0 is challenged again.

> "The moment a decision is declared immune to reconsideration is the moment it starts accumulating technical debt."

---

## Table of Contents

1. [Executive Summary & North Star](#1-executive-summary--north-star)
2. [Full Decision Audit — Every Domain Reopened](#2-full-decision-audit)
3. [Competitor Matrix — 20 Products, 32 Dimensions](#3-competitor-matrix)
4. [Harvested Best Practices — 24 Patterns](#4-harvested-best-practices)
5. [Language, Runtime & Compiler](#5-language-runtime--compiler)
6. [Frontend — Shell, GUI, CLI & Web](#6-frontend)
7. [Backend — Engine, Pipeline, Cache & GPU](#7-backend)
8. [API Design, Error Handling & Versioning](#8-api-design-error-handling--versioning)
9. [External Libraries, APIs & Data Sources](#9-external-libraries-apis--data-sources)
10. [Build System, Toolchain & Developer Experience](#10-build-system-toolchain--developer-experience)
11. [Testing, Quality & Corpus Strategy](#11-testing-quality--corpus-strategy)
12. [Database & Persistent Storage](#12-database--persistent-storage)
13. [Documentation Strategy](#13-documentation-strategy)
14. [CI/CD, Packaging & Distribution](#14-cicd-packaging--distribution)
15. [Security, Sandboxing & Observability](#15-security-sandboxing--observability)
16. [Cross-Platform, AI/ML & Advanced Features](#16-cross-platform-aiml--advanced-features)
17. [Refactor / Rewrite / Delete / Add Register](#17-refactor--rewrite--delete--add-register)
18. [9-Phase Plan to Best-in-Class](#18-9-phase-plan-to-best-in-class)
19. [Success Metrics & Exit Criteria](#19-success-metrics--exit-criteria)
20. [Architecture Decision Log (v6.0)](#20-architecture-decision-log-v60)
21. [AI Tooling Surface & MCP](#21-ai-tooling-surface--mcp)
22. [Consolidated Work — v5.0 Survivors](#22-consolidated-work--v50-survivors)
23. [Sprint Delivery Log](#23-sprint-delivery-log)

---

## 1. Executive Summary & North Star

**North Star:** ExplorerLens is the definitive Windows Shell thumbnail provider — faster than anything built-in, broader than any competitor, safe enough for enterprise, open enough for community.

### What v6.0 changes versus v5.0

| Domain | v5.0 Decision | v6.0 Verdict | Rationale |
|--------|--------------|--------------|-----------|
| C++ standard | C++20, C++23 deferred | **C++23 now** for `std::expected`, `std::mdspan`, `if consteval` | MSVC v145 supports C++23 fully |
| WTL GUI | Keep WTL | **WTL now, WinUI 3 gate at Phase 3** | WTL is productive; WinUI 3 evaluated on merit in Phase 3 |
| Static HTML | Plain HTML | **HTML + vanilla JS format search** | Zero dependency, fast, works offline |
| LZMA SDK | Soft retire | **Hard retire Phase 1** — replace with minizip-ng | 13 dead scripts confirmed |
| Engine layout | 16 subdirectories | **Consolidate to 7 dirs Phase 1** — Phase 1 blocker | 16→7 reduces cognitive load, CMake complexity |
| STA model | Assumed compliant | **STA audit Phase 1** — block host thread, never pump | Defects found in prototype review |
| Static analysis | `/W4` only | **`/analyze` + SAL annotations Phase 2** | CET and bounds violations caught pre-ship |
| Security mitigations | ASLR/DEP/GS | **+CET `/CETCOMPAT` Phase 2** | Win11 hardware enforcement for free |
| COM boundary | Permissive | **Strict — no STL across COM boundary** | ABI stability, no runtime mismatch |
| Cache key | Format+size+mtime | **+decoder version Phase 1** | Stale cache after upgrades fixed |
| PDF backend | MuPDF Phase 3 | **PDFium accelerated to Phase 2** | Better licensing, actively maintained |
| Streaming decoder | Not planned | **IStreamingDecoder for >50 MB Phase 2** | Large PSD/TIF without full load |
| Corpus size | 106 files | **150 Phase 1, 300 Phase 2, 500 Phase 3** | SSIM validation requires real files |
| ADRs | 11 records | **22 ADRs in §20** | Captures v6.0 reopened decisions |
| Packaging | winget optional | **winget mandatory Phase 1** | Discovery is table stakes |

---

## 2. Full Decision Audit

Every decision category from v5.0 is reopened. Verdict: **Execute**, **Keep**, **Accelerate**, **Defer**, or **Kill**.

### 2.1 Architecture (A1–A22)

| # | Decision | v5.0 | v6.0 Verdict | Phase |
|---|----------|------|--------------|-------|
| A1 | COM in-process DLL as delivery vehicle | Keep | **Keep** — fastest IPC, no marshal overhead | — |
| A2 | Single-threaded apartment (STA) host thread; decode on worker pool | Keep | **Keep + Audit** — add `VerifySTA()` assertion at COM entry | P1 |
| A3 | `std::expected<T,E>` for error propagation; no exception throws across COM | Execute | **Execute** ✅ done in Sprint S211-S220 | Done |
| A4 | FormatDetect by magic bytes, not file extension | Keep | **Keep** — extension-only detection is fragile | — |
| A5 | 9-stage decode pipeline: Detect→Probe→CacheCheck→Route→Embedded→Decode→Color→GPUResize→Store | Keep | **Keep** — pipeline is correct; add streaming tap after Probe | — |
| A6 | GDI+ as CPU fallback renderer | Keep | **Keep until Phase 2** — D3D11 replaces for batch mode | P2 |
| A7 | libjpeg-turbo fast path for JPEG | Execute | **Execute** ✅ done in S221 | Done |
| A8 | Engine directory consolidation 16→7 | Defer | **Execute Phase 1 blocker** — 16 dirs is unsustainable | P1 |
| A9 | 5:1 test-to-source ratio maintained | Keep | **Execute** — ratio at 4.8:1; add 200 tests in P1 | P1 |
| A10 | STA compliance audit on all COM entry points | Not listed | **Execute Phase 1** — new in v6.0 | P1 |
| A11 | `std::mdspan` for 2D pixel buffer views | Defer | **Execute Phase 2** — eliminates manual stride arithmetic | P2 |
| A12 | `/analyze` + SAL annotations on Engine/Core | Not listed | **Execute Phase 2** | P2 |
| A13 | `/CETCOMPAT` linker flag for CET enforcement | Not listed | **Execute Phase 2** | P2 |
| A14 | Decoder version embedded in cache key | Defer | **Execute Phase 1** | P1 |
| A15 | `std::jthread` + `std::stop_token` for cancellable decode | Defer | **Execute Phase 2** — enables timeout + Explorer cancel | P2 |
| A16 | Plugin trust chain: signed manifest + hash verification | Keep | **Keep** — plugin sandbox redesigned in §15 | P3 |
| A17 | Structured ETW events per pipeline stage | Execute | **Keep** — ETW provider registered ✅ | Done |
| A18 | Zero warnings policy enforced by CI | Keep | **Keep** — non-negotiable | — |
| A19 | Format probe result cached in memory (ProbeCache) | Not listed | **Execute Phase 1** — avoid re-probe on Explorer refresh | P1 |
| A20 | RAII wrappers for all Win32 handles | Keep | **Keep** — `wil::` adopted for new code | — |
| A21 | Strict COM boundary: no STL containers across boundary | Not listed | **Execute Phase 1** — audit + `static_assert` guards | P1 |
| A22 | Structured cache blob format v1 with version header | Not listed | **Execute Phase 1** — enables zero-copy mmap reads | P1 |

### 2.2 Frontend (F1–F8)

| # | Decision | v5.0 | v6.0 Verdict | Phase |
|---|----------|------|--------------|-------|
| F1 | IThumbnailProvider as primary COM interface | Keep | **Keep** — it IS the product | — |
| F2 | WTL for LENSManager | Keep | **Keep** — battle-tested, zero overhead | — |
| F3 | Dark mode via `DwmSetWindowAttribute` | Keep | **Keep** | — |
| F4 | lens.exe CLI for headless decode + batch | Execute | **Keep + Expand** — add `--profile` and `--compare` flags P2 | P2 |
| F5 | Static HTML format catalogue | Execute | **Keep + vanilla JS search** — Phase 1 add filter/search | P1 |
| F6 | IPreviewHandler (full-resolution in-pane preview) | Defer | **Execute Phase 3 P0** — highest Explorer UX win | P3 |
| F7 | System tray icon for quick settings | Defer | **Execute Phase 3** — complements IPreviewHandler | P3 |
| F8 | First-run wizard / onboarding UX | Not listed | **Execute Phase 3** — reduces support burden | P3 |

### 2.3 Backend (B1–B10)

| # | Decision | v5.0 | v6.0 Verdict | Phase |
|---|----------|------|--------------|-------|
| B1 | ExplorerLensEngine.lib (static) → Engine.dll (Phase 4) | Keep | **Keep schedule** — static is safe during refactor | P4 |
| B2 | SQLite 3.45+ for thumbnail cache | Execute | **Execute Phase 2** — replaces bespoke binary cache | P2 |
| B3 | L1 in-memory LRU (64 MB), L2 SQLite on-disk (512 MB) | Execute | **Execute Phase 2** with cache key v2 | P2 |
| B4 | Multi-tenant cache isolation by SID | Defer | **Execute Phase 2** | P2 |
| B5 | GPU decode pipeline: NVDEC/QuickSync/AMF | Defer | **Execute Phase 2** for D3D11 resize; vendor decode Phase 3 | P2/P3 |
| B6 | SIMD (AVX2) pixel format conversion | Keep | **Keep** — verified in perf tests | — |
| B7 | LibRaw embedded preview fast-path | Execute | **Execute** ✅ done in S222 | Done |
| B8 | PDFium as PDF backend | Phase 3 | **Accelerate to Phase 2** — licensing and maintenance advantage | P2 |
| B9 | IStreamingDecoder interface for large files (>50 MB) | Not listed | **Execute Phase 2** — PSD/TIF/TIFF zero-copy | P2 |
| B10 | Decoder version in cache key | Not listed | **Execute Phase 1** | P1 |

### 2.4 Libraries (L1–L16)

| # | Library | v5.0 | v6.0 Verdict |
|---|---------|------|--------------|
| L1 | zlib 1.3.1 | Keep | **Keep** |
| L2 | LZ4 1.9.6 | Keep | **Keep** |
| L3 | zstd 1.5.6 | Keep | **Keep** |
| L4 | minizip-ng 4.0.x | Keep | **Keep** — LZMA SDK dead scripts retired |
| L5 | libjpeg-turbo 3.0.4 | Execute | **Keep** ✅ |
| L6 | LibWebP 1.4.0 | Keep | **Keep** |
| L7 | LibRaw 0.21.3 | Execute | **Keep** ✅ |
| L8 | libheif 1.19.x + libde265 1.0.15 | Keep | **Keep — LGPL compliance via NOTICE** |
| L9 | dav1d 1.4.x (AV1) | Keep | **Keep** |
| L10 | stb_image (PNG/TGA/BMP/PNM fallback) | Keep | **Demote to fallback only Phase 2** — replace with libpng + libtiff |
| L11 | tinyexr (OpenEXR) | Keep | **Keep until Phase 2** — evaluate OpenEXR 3.x |
| L12 | DirectXTex (DDS/HDR) | Keep | **Keep** |
| L13 | UnRAR 7.x | Keep | **Review** — license restrictive; evaluate libarchive RAR5 P2 |
| L14 | LZMA SDK 26.00 | Soft retire | **Hard retire Phase 1** — 13 scripts dead, minizip-ng covers use cases |
| L15 | MuPDF 1.24 | Phase 3 | **Kill — PDFium replaces** P2 |
| L16 | WTL 10.0 | Keep | **Keep** |

### 2.5 Testing (T1–T9)

| # | Decision | v5.0 | v6.0 Verdict | Phase |
|---|----------|------|--------------|-------|
| T1 | Custom TEST/ASSERT macros as primary harness | Keep | **Keep** — 5,045 tests passing | — |
| T2 | Catch2 v3.7.1 for integration/behavioral tests | Execute | **Keep + Expand** — 45 files, target 60 files P2 | P2 |
| T3 | Google Benchmark for perf regression | Keep | **Keep** — 5 benchmarks in baseline.json | — |
| T4 | SSIM validation in CI for decoder output | Execute | **Execute Phase 1** — threshold ≥0.95 all decoders | P1 |
| T5 | Corpus-driven testing from `data/corpus/` | Execute | **Execute + Grow** — 150 P1, 300 P2, 500 P3 | P1+ |
| T6 | Fuzz testing (libFuzzer) per decoder | Defer | **Execute Phase 4** — OSS-Fuzz integration (H21) | P4 |
| T7 | COM integration tests (host-in-test) | Not listed | **Execute Phase 3** — register DLL in test harness | P3 |
| T8 | Dev container for reproducible CI environment | Execute | **Keep** ✅ | Done |
| T9 | Property-based tests for pixel math | Not listed | **Execute Phase 2** — RapidCheck or custom generator | P2 |

### 2.6 Security (S1–S12)

| # | Decision | v5.0 | v6.0 Verdict | Phase |
|---|----------|------|--------------|-------|
| S1 | ASLR + DEP + GS via MSVC defaults | Keep | **Keep** | — |
| S2 | SafeInt for all pixel dimension arithmetic | Keep | **Keep** | — |
| S3 | No C runtime allocation in STA thread | Keep | **Keep** | — |
| S4 | OOP AppContainer for plugin host | Defer | **Execute Phase 4** | P4 |
| S5 | EV code signing | Defer | **Execute Phase 4** | P4 |
| S6 | SBOM generation in CI | Execute | **Keep** ✅ | Done |
| S7 | CET (`/CETCOMPAT`) | Not listed | **Execute Phase 2** | P2 |
| S8 | SAL annotations on all public Engine headers | Not listed | **Execute Phase 2** | P2 |
| S9 | `dependency-review` GitHub Action | Not listed | **Execute Phase 1** — blocks supply-chain vulns | P1 |
| S10 | Secret scanning + push protection | Keep | **Keep** | — |
| S11 | ASAN build in CI nightly | Defer | **Execute Phase 4** | P4 |
| S12 | mTLS for REST API (Phase 4) | Not listed | **Execute Phase 4** | P4 |

### 2.7 Observability (O1–O5)

| # | Decision | v5.0 | v6.0 Verdict | Phase |
|---|----------|------|--------------|-------|
| O1 | ETW provider `ExplorerLens-Engine` | Execute | **Keep** ✅ | Done |
| O2 | Structured JSON logger (fallback) | Keep | **Keep** | — |
| O3 | WER crash reporting integration | Defer | **Execute Phase 4** — custom WER handler | P4 |
| O4 | Perf regression gate in CI (baseline.json) | Execute | **Keep** ✅ | Done |
| O5 | Live ETW session in LENSManager | Defer | **Execute Phase 3** | P3 |

---

## 3. Competitor Matrix

20 products × 32 dimensions. ✅ = yes/strong, ⚠️ = partial/limited, ❌ = no/absent, — = N/A.

> Competitors studied: PowerToys, QuickLook, SageThumbs, Icaros, Windows built-in, XnView MP,  
> macOS Quick Look, ImageGlass, Nomacs, GNOME Tumbler, libvips, Apache Tika, Eagle,  
> digiKam, Files App (WinUI 3), Kap, RawTherapee, ExplorerLens (current), ExplorerLens (target)

| Dimension | ExplorerLens Now | ExplorerLens Target | PowerToys | QuickLook | SageThumbs | Icaros | Win Built-in | XnView MP | macOS QL | ImageGlass | Nomacs | GNOME Tumbler | libvips | Apache Tika | Eagle | digiKam | Files App | Kap | RawTherapee |
|-----------|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| **Shell integration (native)** | ✅ | ✅ | ✅ | ⚠️ | ✅ | ✅ | ✅ | ❌ | ✅ | ❌ | ❌ | ✅ | ❌ | ❌ | ❌ | ❌ | ✅ | ❌ | ❌ |
| **Language / runtime** | C++20 | C++23 | C++ | C# | C++ | C++ | C++ | C++ | ObjC/Swift | C# | C++ | C | C | Java | Electron | C++ | React | Swift | C++ |
| **License** | MIT | MIT | MIT | GPL3 | Shareware | Freeware | Proprietary | Freeware | Proprietary | MIT | GPL3 | LGPL | LGPL | Apache2 | Proprietary | GPL2 | MIT | GPL3 | GPL3 |
| **Actively maintained** | ✅ | ✅ | ✅ | ⚠️ | ⚠️ | ⚠️ | ✅ | ✅ | ✅ | ✅ | ⚠️ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| **AVIF / JXL / HEIC** | HEIC✅ AVIF✅ JXL❌ | ✅ | ⚠️ | ⚠️ | ❌ | ❌ | HEIC✅ | ✅ | ✅ | ✅ | ⚠️ | ⚠️ | ✅ | ✅ | ✅ | ✅ | ⚠️ | ❌ | ❌ |
| **RAW camera (50+ models)** | ✅ | ✅ | ❌ | ❌ | ⚠️ | ⚠️ | ⚠️ | ✅ | ✅ | ✅ | ✅ | ⚠️ | ❌ | ⚠️ | ✅ | ✅ | ❌ | ❌ | ✅ |
| **Archive covers (ZIP/RAR/7z)** | ✅ | ✅ | ❌ | ❌ | ✅ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **PDF thumbnail** | ❌ | ✅ | ✅ | ✅ | ❌ | ❌ | ✅ | ❌ | ✅ | ❌ | ❌ | ✅ | ❌ | ✅ | ✅ | ❌ | ❌ | ❌ | ❌ |
| **Video keyframe** | ❌ | ✅ | ❌ | ✅ | ❌ | ✅ | ✅ | ❌ | ✅ | ❌ | ❌ | ✅ | ❌ | ✅ | ✅ | ❌ | ✅ | ✅ | ❌ |
| **3D model (glTF/OBJ/STL)** | ❌ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ⚠️ | ✅ | ❌ | ❌ | ❌ | ❌ |
| **Font specimen sheet** | ❌ | ✅ | ❌ | ✅ | ❌ | ❌ | ✅ | ❌ | ✅ | ❌ | ❌ | ✅ | ❌ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **SVG rendering** | ❌ | ✅ | ✅ | ✅ | ❌ | ❌ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ | ❌ | ✅ | ✅ | ✅ | ⚠️ | ❌ | ❌ |
| **GPU-accelerated decode** | ❌ | ✅ | ❌ | ❌ | ❌ | ❌ | ⚠️ | ❌ | ✅ | ❌ | ❌ | ❌ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **IPreviewHandler** | ❌ | ✅ | ✅ | ❌ | ❌ | ❌ | ✅ | ❌ | — | ❌ | ❌ | — | — | — | ❌ | ❌ | ⚠️ | — | — |
| **IPropertyStore** | ❌ | ✅ | ⚠️ | ❌ | ❌ | ❌ | ✅ | ❌ | — | ❌ | ❌ | — | — | — | ❌ | ❌ | ❌ | — | — |
| **IContextMenu** | ❌ | ✅ | ✅ | ❌ | ✅ | ✅ | ✅ | ❌ | — | ❌ | ❌ | — | — | — | ❌ | ❌ | ❌ | — | — |
| **Plugin ecosystem** | ❌ | ✅ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ✅ | ❌ | ❌ | ⚠️ | ✅ | ❌ | ✅ | ❌ |
| **Headless CLI** | ✅ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ✅ | ❌ | ❌ | ✅ | ❌ | ✅ | ✅ | ❌ | ✅ | ❌ | ❌ | ✅ |
| **REST API** | ❌ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ✅ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **SQLite / persistent cache** | ❌ | ✅ | ❌ | ❌ | ❌ | ❌ | ✅ | ❌ | ✅ | ❌ | ❌ | ✅ | ❌ | ❌ | ✅ | ✅ | ❌ | ❌ | ❌ |
| **SSIM-validated CI** | ⚠️ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **SBOM in CI** | ✅ | ✅ | ⚠️ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **Fuzz testing** | ❌ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **Crash reporting (WER)** | ❌ | ✅ | ✅ | ❌ | ❌ | ❌ | ✅ | ❌ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ✅ | ❌ |
| **EV code signed** | ❌ | ✅ | ✅ | ❌ | ❌ | ❌ | ✅ | ✅ | ✅ | ❌ | ❌ | ❌ | — | — | ✅ | ❌ | ❌ | ❌ | ❌ |
| **winget package** | ❌ | ✅ | ✅ | ✅ | ❌ | ❌ | — | ❌ | — | ✅ | ✅ | — | — | — | ❌ | ❌ | ✅ | ✅ | ❌ |
| **Enterprise / GPO** | ❌ | ✅ | ❌ | ❌ | ❌ | ❌ | ✅ | ❌ | — | ❌ | ❌ | — | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **Store / MSIX** | ❌ | ✅ | ✅ | ❌ | ❌ | ❌ | ✅ | ❌ | ✅ | ✅ | ❌ | — | — | — | ❌ | ❌ | ✅ | ❌ | ❌ |
| **EXIF-aware rotation** | ✅ | ✅ | ❌ | ✅ | ✅ | ⚠️ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ❌ | ✅ |
| **16-bit pixel pipeline** | ❌ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ✅ | ✅ | ✅ | ✅ | ❌ | ✅ | ❌ | ❌ | ✅ | ❌ | ❌ | ✅ |
| **Perceptual hash (pHash)** | ❌ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ✅ | ❌ | ✅ | ❌ | ❌ | ✅ | ❌ | ✅ | ✅ | ❌ | ❌ | ❌ |
| **Open source / MIT** | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ | ❌ | ✅ | ✅ | ✅ | ✅ |

**Score (out of 32):** ExplorerLens Now = **14** · ExplorerLens Target = **32** · PowerToys = **16** · macOS QL = **18**

---

## 4. Harvested Best Practices

24 concrete patterns taken from competitors and applied to ExplorerLens.

| # | Source | Practice | Target Phase |
|---|--------|----------|--------------|
| H1 | PowerToys | ADMX Group Policy templates for enterprise config | Phase 4 |
| H2 | PowerToys | Centralized crash telemetry with user opt-in | Phase 4 |
| H3 | QuickLook | Space-bar preview shortcut wired through IPreviewHandler | Phase 3 |
| H4 | QuickLook | Plugin manifest JSON schema — third-party decoders as `.lens` bundles | Phase 3 |
| H5 | SageThumbs | Multi-image archive: render first frame, overlay file count badge | Phase 2 |
| H6 | Icaros | Video keyframe scrubbing: seek to 10% point, not frame 0 | Phase 3 |
| H7 | macOS QL | Async generator: return low-res placeholder immediately, upgrade async | Phase 2 |
| H8 | macOS QL | QLPreviewPanel style: blurred background, true-color display | Phase 3 |
| H9 | libvips | Sequential-access decode for large TIFF/PSD: zero full-load | Phase 2 |
| H10 | libvips | Tile-based decode: read only the thumbnail region from TIFF pyramids | Phase 2 |
| H11 | Apache Tika | REST server mode: stateless, horizontally scalable decode service | Phase 4 |
| H12 | Apache Tika | MIME detection from stream: no extension dependency | Done |
| H13 | XnView MP | EXIF thumbnail extraction as priority path before full decode | Done |
| H14 | ImageGlass | pHash deduplication: detect resized/re-encoded duplicates | Phase 4 |
| H15 | digiKam | SQLite metadata cache: schema with index on phash + file_id | Phase 2 |
| H16 | digiKam | Full LGPL compliance: NOTICE file + link-only usage | Done |
| H17 | Eagle | Folder-level cover image: first image in folder is folder thumbnail | Phase 3 |
| H18 | GNOME Tumbler | D-Bus activation model for background daemon — adapt to COM surrogate | Phase 4 |
| H19 | digiKam | EXIF-aware smart rotation baked into all decoder outputs | Phase 2 |
| H20 | Files App (WinUI 3) | Mica/Acrylic material + XAML components for LENSManager redesign | Phase 3 gate |
| H21 | libvips / OSS-Fuzz | Continuous fuzzing via Google OSS-Fuzz integration per decoder | Phase 4 |
| H22 | Apache Tika REST | Stateless REST service: `POST /decode` returns image + metadata JSON | Phase 4 |
| H23 | Kap | Plugin manifest with typed JSON schema + validator at load time | Phase 3 |
| H24 | RawTherapee | Full 16-bit decode pipeline: no premature 8-bit clamping in Engine | Phase 2 |

---

## 5. Language, Runtime & Compiler

### Primary: C++23 / MSVC v145 (cl.exe 19.50)

| Feature | Status | Notes |
|---------|--------|-------|
| `std::expected<T,E>` | ✅ Done | All Engine APIs migrated (S211-S220) |
| `std::format` | ✅ Done | Replaces `sprintf` in log paths |
| `std::ranges` | ✅ Done | Decoder dispatch tables use ranges |
| `std::jthread` / `std::stop_token` | Phase 2 | Cancellable decode workers |
| `std::mdspan` | Phase 2 | 2D pixel buffer views, stride-safe |
| `if consteval` | Phase 2 | Compile-time magic byte tables |
| `std::flat_map` | Phase 3 | Format registry (ordered, cache-friendly) |
| `[[nodiscard("reason")]]` | Phase 1 | All `std::expected` return sites |

### Language Research Lanes

| Language | Role | Status |
|----------|------|--------|
| C++23 (MSVC v145) | Production — everything | Active |
| Rust 1.80+ | Research lane — parser modules only | Phase 2 eval |
| Python 3.12 | Build scripts, corpus tooling | Active |
| PowerShell 7.4 | Build automation, CI glue | Active |

**Rust de-risk plan:** If Rust modules can link as `extern "C"` static libs with zero overhead in MSVC, adopt for format parsers Phase 3. Otherwise kill the lane and close ADR-019.

### WTL vs. WinUI 3

| Criteria | WTL (now) | WinUI 3 (Phase 3 gate) |
|----------|-----------|------------------------|
| Dependency | None — header-only | WindowsAppSDK 1.6+ |
| Build complexity | Low | High (MSIX, package identity) |
| Runtime cost | ~0 KB | ~25 MB WindowsAppSDK |
| Visual quality | Win32 classic | Mica/Acrylic, XAML |
| Accessibility | Manual | XAML accessibility tree |
| Decision | Keep for v40.x | Evaluate H20 in Phase 3 |

---

## 6. Frontend

### 6.1 COM Shell Interfaces

| Interface | Status | Priority | Description |
|-----------|--------|----------|-------------|
| `IThumbnailProvider` | ✅ Done | P0 | Core product — thumbnail in Explorer |
| `IPreviewHandler` | Phase 3 | P0 | Full-resolution in-pane preview (Space) |
| `IPropertyStore` | Phase 3 | P0 | Expose EXIF/XMP in Details pane |
| `IContextMenu` | Phase 3 | P1 | "Generate full-res thumbnail" right-click |
| `IExtractImage` | Phase 4 | P2 | Compat with pre-Vista hosts |
| `IThumbnailHandlerFactory` | Phase 4 | P2 | Batch generation via factory |

### 6.2 LENSManager GUI (WTL)

| Feature | Status | Phase |
|---------|--------|-------|
| COM registration / unregistration | ✅ Done | — |
| Format enable/disable grid | ✅ Done | — |
| Cache statistics panel | ✅ Done | — |
| Decoder health check (DecoderHealthCheck.h) | ✅ Done | — |
| Export diagnostics (ExportDiagnostics.h) | ✅ Done | — |
| Dark mode (DarkModeController.h) | ✅ Done | — |
| Live ETW trace viewer | Phase 3 | P1 |
| First-run onboarding wizard | Phase 3 | P1 |
| WinUI 3 redesign | Phase 3 gate | P2 |

### 6.3 lens.exe CLI

| Command | Status | Notes |
|---------|--------|-------|
| `lens generate <file>` | ✅ Done | Single thumbnail to stdout / file |
| `lens batch <dir>` | ✅ Done | Batch with progress bar |
| `lens cache stats` | ✅ Done | Cache hit/miss/size report |
| `lens cache purge` | ✅ Done | Invalidate by format or all |
| `lens register` | ✅ Done | COM registration (admin) |
| `lens unregister` | ✅ Done | COM unregistration |
| `lens formats` | Phase 1 | List all supported formats as JSON |
| `lens profile <file>` | Phase 2 | Per-stage timing breakdown |
| `lens compare <f1> <f2>` | Phase 2 | SSIM diff between two renders |

### 6.4 Static HTML Format Catalogue

Located at `index.html` (root) and `docs/index.html`.

| Feature | Status |
|---------|--------|
| Full format table (200+ rows) | ✅ Done |
| Decoder family grouping | ✅ Done |
| Vanilla JS search / filter | Phase 1 |
| MIME type column | Phase 1 |
| Corpus coverage badge per format | Phase 2 |

---

## 7. Backend

### 7.1 9-Stage Decode Pipeline

```
[Explorer IStream] ──▶ FormatDetect ──▶ ProbeHeader ──▶ CacheCheck
                                                              │
                             ┌────────────────────────────── hit ──▶ [return cached]
                             │
                           miss
                             │
                             ▼
                         Route ──▶ EmbeddedPreview? ──▶ Decode ──▶ ColorCorrect
                                                                         │
                                                                    GPUResize ──▶ CacheStore ──▶ [return HBITMAP]
```

**New in v6.0:**
- ProbeCache: memoize probe result in memory — avoid re-probe on Explorer refresh
- Streaming tap after ProbeHeader: IStreamingDecoder for files >50 MB
- Cache key v2: `format + size + mtime + decoder_version`

### 7.2 Engine Directory Consolidation (Phase 1 Blocker)

| Current (16 dirs) | Target (7 dirs) | Rationale |
|-------------------|-----------------|-----------|
| Core/ | Core/ | Keep — decode pipeline, render |
| Decoders/ | Decoders/ | Keep — format decoders |
| Cache/ | Core/Cache/ | Merge into Core |
| Pipeline/ | Core/Pipeline/ | Merge into Core |
| Memory/ | Core/Memory/ | Merge into Core |
| GPU/ | Core/GPU/ | Merge into Core |
| Codec/ | Decoders/Codec/ | Merge into Decoders |
| AI/ | Features/AI/ | New Features/ dir |
| Media/ | Features/Media/ | New Features/ dir |
| Platform/ | Platform/ | Keep — PAL |
| Plugin/ | Plugin/ | Keep |
| PluginHost/ | Plugin/Host/ | Merge into Plugin |
| Enterprise/ | Platform/Enterprise/ | Merge into Platform |
| Utils/ | Utils/ | Keep — shared helpers |
| CLI/ | CLI/ | Keep |
| Tests/ | Tests/ | Keep |

### 7.3 Decoder Priority Tiers

| Tier | Formats | Decoder | Phase |
|------|---------|---------|-------|
| P0 | JPEG, PNG, BMP, GIF, TIFF | libjpeg-turbo, GDI+, stb_image | ✅ Done |
| P0 | HEIC, HEIF, AVIF | libheif + libde265 + dav1d | ✅ Done |
| P0 | WebP | libwebp | ✅ Done |
| P0 | RAW (600+ camera models) | LibRaw (embedded preview fast-path) | ✅ Done |
| P0 | ZIP/7z/RAR archives | minizip-ng + dav1d | ✅ Done |
| P1 | PDF (first page) | PDFium | Phase 2 |
| P1 | DDS/HDR/TGA | DirectXTex + stb_image | ✅ Done |
| P1 | OpenEXR | tinyexr → OpenEXR 3.x eval | Phase 2 |
| P1 | SVG | Direct2D / resvg | Phase 3 |
| P1 | Video keyframe | FFmpeg mini / MediaFoundation | Phase 3 |
| P2 | glTF/OBJ/STL | tiny_gltf / par_shapes | Phase 3 |
| P2 | Font (TTF/OTF) | FreeType 2 | Phase 3 |
| P2 | PSD/PSB large | IStreamingDecoder (libpsd) | Phase 2 |
| P3 | DICOM | dcmtk stub | Phase 4 |
| P3 | HDF5 / scientific | custom stub | Phase 4 |

### 7.4 Cache Architecture

**L1 — In-memory LRU**
- Capacity: 64 MB (configurable via registry)
- Key: `CacheKey_v2 = Blake3(path + size + mtime + decoder_ver)`
- Eviction: LRU with pinned-set for recently opened files
- Thread-safe: `std::shared_mutex`

**L2 — SQLite on-disk**
- Path: `%LOCALAPPDATA%\ExplorerLens\thumbnails.db`
- Capacity: 512 MB (configurable)
- Schema: see §12
- Eviction: LRU by `last_access`; vacuum on startup if >90% full
- Multi-tenant: isolated by user SID

**Cache Key v2 format:**
```cpp
struct CacheKey_v2 {
    uint64_t path_hash;       // Blake3 of normalized path
    uint64_t file_size;
    uint64_t mtime_100ns;
    uint32_t decoder_version; // NEW in v6.0
    uint32_t thumb_size;      // requested thumbnail dimension
};
```

---

## 8. API Design, Error Handling & Versioning

### 8.1 EngineError Enum (v2)

```cpp
enum class EngineError : uint32_t {
    OK                  = 0,
    FORMAT_UNKNOWN      = 1,
    DECODE_FAILED       = 2,
    IO_ERROR            = 3,
    UNSUPPORTED_VARIANT = 4,
    TIMEOUT             = 5,
    PLUGIN_REJECTED     = 6,
    CACHE_MISS          = 7,
    CACHE_CORRUPT       = 8,   // NEW v6.0
    PROBE_INCONCLUSIVE  = 9,   // NEW v6.0
    OUT_OF_MEMORY       = 10,  // NEW v6.0
    CANCELLED           = 11,  // NEW v6.0 (jthread cancel)
};
```

### 8.2 IStreamingDecoder Interface (Phase 2)

```cpp
struct IStreamingDecoder {
    virtual std::expected<ThumbnailResult, EngineError>
        DecodeStream(ISequentialStream* stream, const DecodeParams& p) = 0;

    virtual std::expected<ThumbnailResult, EngineError>
        DecodeEmbeddedPreview(ISequentialStream* stream) = 0;  // NEW v6.0

    virtual uint64_t EstimatedMemoryPeak(uint64_t file_size) const noexcept = 0;
    virtual ~IStreamingDecoder() = default;
};
```

### 8.3 REST API (Phase 4)

Built with `cpp-httplib` (header-only, no Boost).

| Endpoint | Method | Auth | Description |
|----------|--------|------|-------------|
| `/v1/decode` | POST | Bearer | Decode file → thumbnail (multipart) |
| `/v1/decode/batch` | POST | Bearer | Batch decode list of paths |
| `/v1/formats` | GET | None | List all supported formats as JSON |
| `/v1/cache/stats` | GET | Bearer | Cache hit/miss/size/eviction counts |
| `/v1/cache/purge` | DELETE | Bearer | Purge cache by format glob or all |
| `/v1/health` | GET | None | Liveness probe (Kubernetes-compatible) |
| `/v1/version` | GET | None | Version + build info JSON |

Auth model: `Authorization: Bearer <token>` — token is a SHA-256 HMAC over a per-session secret. mTLS in Phase 4 for enterprise deployments.

### 8.4 Versioning Policy

- **COM interface versioning:** New interfaces registered under new ProgID suffix (`LENSShell.v2`)
- **Engine ABI:** Static lib — no ABI compat required until Engine.dll Phase 4
- **Cache format versioning:** Version byte at offset 0 of every blob; migration on open
- **REST API versioning:** URI path (`/v1/`, `/v2/`) — never break `/v1/` once GA

---

## 9. External Libraries, APIs & Data Sources

### 9.1 Library Inventory (30 total)

| Library | Version | Purpose | License | Phase |
|---------|---------|---------|---------|-------|
| zlib | 1.3.1 | Deflate (ZIP, PNG) | zlib | ✅ |
| LZ4 | 1.9.6 | LZ4 frame decode | BSD-2 | ✅ |
| zstd | 1.5.6 | Zstandard decode | BSD-3 | ✅ |
| minizip-ng | 4.0.x | ZIP/7z/LZMA archives | zlib | ✅ |
| libjpeg-turbo | 3.0.4 | JPEG fast decode | IJG+BSD | ✅ |
| libwebp | 1.4.0 | WebP decode | BSD-3 | ✅ |
| LibRaw | 0.21.3 | RAW camera decode | LGPL2.1 | ✅ |
| libheif | 1.19.x | HEIC/HEIF container | LGPL3 | ✅ |
| libde265 | 1.0.15 | HEVC decoder for HEIF | LGPL3 | ✅ |
| dav1d | 1.4.x | AV1 decoder (AVIF) | BSD-2 | ✅ |
| stb_image | 2.30 | PNG/TGA/BMP/PNM fallback | MIT/PD | ✅ |
| tinyexr | 1.0.9 | OpenEXR decode | BSD-3 | ✅ |
| DirectXTex | Jun 2024 | DDS/HDR/TGA | MIT | ✅ |
| WTL | 10.0 | GUI framework | MIT | ✅ |
| WIL | 1.0.240803 | Win32 RAII wrappers | MIT | ✅ |
| Catch2 | 3.7.1 | Integration test harness | BSL-1.0 | ✅ |
| Google Benchmark | 1.8.4 | Microbenchmarks | Apache-2.0 | ✅ |
| nlohmann/json | 3.11.3 | JSON for CLI output, plugin manifests | MIT | ✅ |
| SQLite | 3.46.x | L2 thumbnail cache | PD | Phase 2 |
| PDFium | latest | PDF first-page thumbnail | Apache-2.0 | Phase 2 |
| OpenEXR | 3.x | Full EXR (replaces tinyexr) | BSD-3 | Phase 2 eval |
| libpng | 1.6.x | PNG (replaces stb_image for PNG) | libpng | Phase 2 |
| libtiff | 4.6.x | TIFF streaming | libtiff | Phase 2 |
| FreeType | 2.13.x | Font specimen | FreeType | Phase 3 |
| resvg | 0.43.x | SVG via Rust C API | MIT | Phase 3 |
| tiny_gltf | 2.9.x | glTF parse | MIT | Phase 3 |
| par_shapes | — | 3D mesh → thumbnail | MIT | Phase 3 |
| ONNX Runtime | 1.18.x | AI scene understanding | MIT | Phase 5 |
| cpp-httplib | 0.16.x | REST server | MIT | Phase 4 |
| libarchive | 3.7.x | RAR5 alt to UnRAR | BSD-2 | Phase 2 eval |

### 9.2 APIs & Data Sources

| API / Source | Purpose | Phase |
|--------------|---------|-------|
| Windows ETW | Observability events | ✅ Done |
| Windows WER | Crash report integration | Phase 4 |
| Windows Imaging Component (WIC) | GDI+ fallback encode | ✅ Done |
| Direct2D / Direct3D 11 | GPU-accelerated resize | Phase 2 |
| MediaFoundation | Video keyframe decode | Phase 3 |
| GitHub Actions API | CI status badges | ✅ Done |
| OSS-Fuzz | Continuous fuzz testing | Phase 4 |
| winget-pkgs repo | Package distribution | Phase 1 |
| Chocolatey community | Alt distribution | Phase 2 |
| Microsoft Store | MSIX distribution | Phase 5 |

---

## 10. Build System, Toolchain & Developer Experience

### 10.1 Current Toolchain

| Tool | Version | Role |
|------|---------|------|
| MSVC cl.exe | 19.50 (v145) | C++23 compiler |
| CMake | 4.3.1 | Engine build system |
| Ninja | 1.12.x | Build backend |
| MSBuild | 17.x | LENSShell + LENSManager |
| vcpkg | 2024.x | Package manager |
| WiX | 4.0.x | MSI packaging |
| Inno Setup | 6.3.x | Portable installer |
| PowerShell | 7.4+ | Build scripts |
| CMake presets | — | `default-release`, `default-debug`, `asan` |

### 10.2 Developer Experience Improvements

| Improvement | Status | Phase |
|-------------|--------|-------|
| `Build-MSVC.ps1` one-command build | ✅ Done | — |
| Dev container (`.devcontainer/`) | ✅ Done | — |
| `dependency-review` GH Action | Phase 1 | P1 |
| Retire 13 dead build scripts | Phase 1 | P1 |
| `asan` CMake preset in CI nightly | Phase 4 | P4 |
| `--preset default-debug` with `/fsanitize=address` | Phase 4 | P4 |
| Reproducible builds (Dockerfile) | ✅ Done | — |
| `lens.exe formats --json` for tooling | Phase 1 | P1 |

### 10.3 Scripts to Retire (Phase 1)

| Script | Reason |
|--------|--------|
| `Build-LZMA-SDK-26.00.ps1` | LZMA SDK hard retired |
| `Rebuild-All-With-MD.ps1` | Superseded by Build-MSVC.ps1 |
| `Remove-Win32-Configurations.ps1` | Win32 configs removed |
| `Fix-PCH-Corruption.ps1` | PCH issues resolved |
| `Update-All-Libraries.ps1` | Replaced by vcpkg |
| `Download-Updates.ps1` | Replaced by vcpkg |
| `build-and-log.bat` | Replaced by PowerShell |
| `test-and-log.bat` | Replaced by PowerShell |
| `ExplorerLens-Profile.ps1` | Merged into Build-MSVC.ps1 |
| `Find-All-Tools.ps1` | Merged into Test-Build-Environment.ps1 |
| `Sign-Binaries.ps1` | Placeholder — no EV cert yet |
| `Verify-Complete-Build.ps1` | Superseded by Check-Build-Status.ps1 |
| `Run-CodeCoverage.ps1` | Replaced by coverage preset |

---

## 11. Testing, Quality & Corpus Strategy

### 11.1 Test Framework Stack (9 Layers)

| Layer | Framework | Count | Purpose |
|-------|-----------|-------|---------|
| 1 | Custom TEST/ASSERT macros | ~5,045 | Unit tests — logic, pixel math, format parsing |
| 2 | Catch2 v3.7.1 | ~1,380 | Integration + behavioral (45 files) |
| 3 | Google Benchmark | 5 | Perf regression (baseline.json) |
| 4 | SSIM validation | Per decoder | Image quality gating in CI |
| 5 | Corpus-driven | 106 files now | Real I/O decoder validation |
| 6 | Fuzz (libFuzzer) | Phase 4 | Per-decoder crash resistance |
| 7 | COM integration | Phase 3 | Host DLL in test harness |
| 8 | Property-based | Phase 2 | Pixel math generators |
| 9 | ASAN nightly | Phase 4 | Memory safety |

### 11.2 Quality Gates (CI blocking)

| Gate | Threshold | Status |
|------|-----------|--------|
| Build (0 errors, 0 warnings) | Mandatory | ✅ |
| Unit tests (all pass) | 100% | ✅ |
| Catch2 tests (all pass) | 100% | ✅ |
| Perf regression (baseline.json) | ≤5% regress | ✅ |
| SSIM ≥ 0.95 per decoder | All decoders | Phase 1 |
| Corpus coverage badge | All P0 decoders | Phase 1 |
| `dependency-review` | No critical CVEs | Phase 1 |
| SBOM generated | Every release | ✅ |
| Secret scanning | No secrets in git | ✅ |
| Code coverage (Engine/) | ≥75% line | Phase 2 |
| ASAN nightly | 0 violations | Phase 4 |
| Fuzz CI | No crashes | Phase 4 |
| COM integration | All interfaces | Phase 3 |

### 11.3 Corpus Growth Plan

| Phase | Target Files | Formats | SSIM Gate |
|-------|-------------|---------|-----------|
| Now | 106 | P0 decoders | Manual |
| Phase 1 | 150 | +PDF, +EXR, +DDS | CI automated |
| Phase 2 | 300 | +PSD, +TIFF large, +video | CI + baseline |
| Phase 3 | 500 | +SVG, +font, +3D | Per-format |
| Phase 4 | 750 | All decoders | Fuzz-seeded |

### 11.4 Test File Placement Rules

- New `TEST()` bodies → `Engine/Tests/EngineTests_Platform.cpp`
- New `extern void` decls → `Engine/Tests/EngineTestsExterns.h`
- New `RUN_TEST()` calls → `Engine/Tests/EngineTests.cpp`
- New `#include` directives → `Engine/Tests/EngineTestsIncludes.h`
- New Catch2 test files → `Engine/Tests/Catch2Tests/`
- File size limit: 500 KB per file; split at `//==` boundaries

---

## 12. Database & Persistent Storage

### 12.1 Storage Items

| Item | Storage | Location | Notes |
|------|---------|----------|-------|
| Thumbnail blobs (L2 cache) | SQLite | `%LOCALAPPDATA%\ExplorerLens\thumbnails.db` | Phase 2 |
| User settings | Registry | `HKCU\Software\ExplorerLens` | ✅ Done |
| COM registration | Registry | `HKCR\CLSID\{...}` | ✅ Done |
| Format enable/disable | Registry | `HKCU\Software\ExplorerLens\Formats` | ✅ Done |
| ETW trace sessions | Windows ETW | — | ✅ Done |
| Corpus MANIFEST.json | JSON file | `data/corpus/MANIFEST.json` | ✅ Done |
| Benchmark baseline | JSON file | `data/benchmarks/baseline.json` | ✅ Done |
| Decoder SSIM baselines | JSON file | `data/baselines/` | Phase 1 |
| Plugin manifests | JSON files | `%PROGRAMDATA%\ExplorerLens\Plugins\` | Phase 3 |
| Enterprise ADMX policy | Registry (GPO) | HKLM + ADMX template | Phase 4 |

### 12.2 SQLite Schema (Phase 2)

```sql
CREATE TABLE thumbnails (
    path_hash     BLOB NOT NULL,   -- Blake3(normalized_path), 32 bytes
    file_size     INTEGER NOT NULL,
    mtime_100ns   INTEGER NOT NULL,
    decoder_ver   INTEGER NOT NULL, -- CacheKey_v2.decoder_version
    thumb_size    INTEGER NOT NULL,
    phash         INTEGER,          -- perceptual hash (Phase 4)
    palette       BLOB,             -- dominant colors JSON (Phase 4)
    pixel_data    BLOB NOT NULL,    -- compressed BGRA thumbnail
    created_at    INTEGER NOT NULL,
    last_access   INTEGER NOT NULL,
    PRIMARY KEY (path_hash, thumb_size)
) WITHOUT ROWID, STRICT;

CREATE INDEX idx_last_access ON thumbnails(last_access);
```

### 12.3 Rejected Alternatives

| Alternative | Rejected Because |
|-------------|-----------------|
| LMDB | Complex build; no SQL query for diagnostics |
| RocksDB | 10 MB+ overhead; overkill for thumbnail blobs |
| Redis | Requires daemon; wrong deployment model |
| Custom binary file | No query, no migration path |
| Win32 File Cache API | OS-managed; no eviction control |
| ESENT | Complex API; deprecated in modern Windows |

---

## 13. Documentation Strategy

### 13.1 Documentation Tiers

| Tier | Content | Location | Rule |
|------|---------|----------|------|
| T1 User | Install, use, formats | README.md, USER_GUIDE.md, CHANGELOG.md | Working features only |
| T2 Developer | Build, contribute, debug | docs/development/, CONTRIBUTING.md | Working commands only |
| T3 Architecture | Decisions, roadmap, ADRs | ROADMAP.md, docs/architecture/, docs/adr/ | Vision + current, clearly labeled |
| T4 Historical | Old roadmaps, sprint archive | docs/archive/ | Read-only |

### 13.2 Target: 65 Active Docs

| Category | Count | Examples |
|----------|-------|---------|
| Tier 1 | 5 | README, USER_GUIDE, CHANGELOG, LICENSE, NOTICE |
| Tier 2 | 15 | CONTRIBUTING, TROUBLESHOOTING, QUICK_START, PERFORMANCE, FORMAT_VALIDATION_STATUS, build/ docs |
| Tier 3 | 20 | ROADMAP, ARCHITECTURE, 22 ADRs |
| Tier 4 | 15 | Archive chain, old roadmaps, sprint logs |
| Spec / generated | 10 | SBOM.json, baseline.json, MANIFEST.json |

### 13.3 ADR Registry — 22 Records

See §20 for full ADR log. Template:

```markdown
# ADR-NNN — Decision Title

**Date:** YYYY-MM-DD  
**Status:** Accepted | Superseded by ADR-NNN | Deprecated  
**Deciders:** (author name or "team")

## Context
Why this decision was needed.

## Decision
What was decided.

## Consequences
Trade-offs accepted.
```

### 13.4 Documentation Quality Standards

- ROADMAP is Tier 3 — clearly label **Done**, **Phase N**, **Research Lane**
- README is Tier 1 — only features that work today
- CHANGELOG follows Keep-a-Changelog 1.1.0 format
- ADRs are immutable once `Accepted` — supersede, don't edit
- All doc links checked by CI (markdown-link-check, Phase 1)

---

## 14. CI/CD, Packaging & Distribution

### 14.1 Current Workflows (27)

Major workflows:
- `build.yml` — CMake Release + Debug
- `test.yml` — CTest + custom EngineTests
- `catch2-tests.yml` — Catch2 integration tests
- `perf-regression.yml` — Benchmark vs. baseline.json
- `sbom.yml` — SPDX SBOM generation
- `publish-packages.yml` — NuGet, npm, Container
- `release.yml` — GitHub Release + artifact upload
- `codeql.yml` — CodeQL security scanning
- `dependency-review.yml` — Phase 1

### 14.2 Planned Phase 2 Additions

| Workflow | Purpose |
|----------|---------|
| `ssim-validation.yml` | SSIM gate per decoder against corpus baselines |
| `coverage.yml` | Line coverage report + badge (≥75% gate) |
| `asan-nightly.yml` | ASAN build nightly on `main` |

### 14.3 Distribution Channels

| Channel | Status | Phase |
|---------|--------|-------|
| GitHub Releases (MSI + ZIP + DLL) | ✅ Done | — |
| SHA256SUMS + SBOM per release | ✅ Done | — |
| winget (winget-pkgs PR) | Phase 1 | P1 |
| Chocolatey community | Phase 2 | P2 |
| NuGet (ExplorerLensEngine) | Phase 2 | P2 |
| npm (ExplorerLens CLI wrapper) | Phase 2 | P2 |
| Docker / ghcr.io (REST server) | Phase 4 | P4 |
| Microsoft Store (MSIX) | Phase 5 | P5 |
| Homebrew (macOS stub) | Phase 5 | P5 |
| Flatpak (Linux stub) | Phase 6 | P6 |

### 14.4 Release Cadence

| Type | Trigger | Contents |
|------|---------|----------|
| Sprint release | Every 10 sprints | Changelog, binaries, updated corpus baselines |
| Patch release | Critical bug / security | Binaries + SHA256SUMS only |
| Major release | Phase completion | Full build: MSI + ZIP + DLL + EXE + SBOM |

---

## 15. Security, Sandboxing & Observability

### 15.1 Security Model

| Layer | Control | Status |
|-------|---------|--------|
| ASLR | `/DYNAMICBASE` | ✅ |
| DEP | `/NXCOMPAT` | ✅ |
| Stack protection | `/GS` | ✅ |
| SafeInt | All pixel math | ✅ |
| Control Flow Guard | `/guard:cf` | ✅ |
| CET (hardware shadow stack) | `/CETCOMPAT` | Phase 2 |
| SAL annotations | Engine/Core headers | Phase 2 |
| OOP AppContainer (plugin host) | `CreateProcessAsUser` + low-integrity | Phase 4 |
| EV code signing | Authenticode SHA-256 | Phase 4 |
| ASAN build in CI | `/fsanitize=address` | Phase 4 |
| libFuzzer per decoder | OSS-Fuzz | Phase 4 |
| mTLS REST | client cert + CA pin | Phase 4 |
| Secret scanning | GH push protection | ✅ |
| dependency-review | GH Action on PRs | Phase 1 |
| CodeQL | GH Action on push | ✅ |
| SBOM + NOTICE | Every release | ✅ |

### 15.2 Plugin Sandbox Design (Phase 3)

```
LENSShell.dll (host)
    │
    └─▶ PluginHost.exe (low-integrity, OOP AppContainer — Phase 4)
            │
            └─▶ Plugin.dll (loaded in surrogate process)
                    manifest.json (signed, hash-verified)
                    plugin_api.h (C ABI only, no STL)
```

Phase 3: validate signed manifest before load  
Phase 4: OOP surrogate process with AppContainer  

### 15.3 Crash Reporting (Phase 4)

```cpp
// WER custom handler
HRESULT RegisterWERHandler() {
    return WerRegisterRuntimeExceptionModule(
        L"ExplorerLens.WerHandler.dll", nullptr);
}
```

Crash dumps upload to private Azure Blob (opt-in telemetry, no PII).

### 15.4 Observability Stack

| Layer | Tool | Status |
|-------|------|--------|
| ETW provider | `ExplorerLens-Engine` GUID | ✅ Done |
| Structured JSON log | `ObservabilityIntegration` singleton | ✅ Done |
| Perf counters | ETW manifest + XPERF | ✅ Done |
| Live trace viewer | LENSManager ETW panel | Phase 3 |
| Crash reporting | WER custom handler | Phase 4 |

---

## 16. Cross-Platform, AI/ML & Advanced Features

### 16.1 Cross-Platform Honest Timeline

| Platform | Status | Phase | Notes |
|----------|--------|-------|-------|
| Windows 10 22H2+ | ✅ Full | — | Primary platform |
| Windows 11 | ✅ Full | — | CET enforcement bonus |
| Windows Server 2019+ | ✅ Full | — | COM hosting confirmed |
| macOS 14+ (Quick Look) | Stub | Phase 5 | `QLPreviewProvider` |
| Linux (Nautilus / KIO) | Stub | Phase 6 | Tumbler D-Bus thumbnailer |
| Browser (WASM) | Research | Phase 6 | Emscripten + Wasm decode |

### 16.2 AI/ML Features `[ai]` Feature Flag

All AI features are gated behind `LENS_FEATURE_AI` compile flag.

| Feature | Model | Phase |
|---------|-------|-------|
| Scene understanding (classify foreground) | MobileNet V3 (ONNX) | Phase 5 |
| Smart crop (saliency-based thumbnail center) | U² Net (ONNX) | Phase 5 |
| Image Quality Assessment (reject blurry) | BRISQUE (custom) | Phase 5 |
| Semantic search index (`lens search "sunset"`) | CLIP (ONNX) | Phase 6 |
| Upscale low-res thumbnails | ESRGAN 4x (ONNX) | Phase 5 |
| Face-aware crop | RetinaFace (ONNX) | Phase 6 |

ONNX Runtime 1.18.x — CPU EP by default, DirectML EP if GPU available.

### 16.3 Advanced Features by Phase

| Feature | Phase |
|---------|-------|
| Stereo / panoramic image viewer | Phase 7 |
| Design file formats (Figma export, Sketch) | Phase 7 |
| Third-party plugin catalog (lens-plugins.io) | Phase 7 |
| 1,000+ corpus files | Phase 8 |
| Published benchmarks vs. competitors | Phase 8 |
| D3D12 Video decode pipeline | Phase 9 |
| Neural upscaling (non-ONNX, custom kernel) | Phase 9 |
| WASM plugin sandbox | Phase 9 |

---

## 17. Refactor / Rewrite / Delete / Add Register

### 17.1 Delete (Phase 1)

| Item | Reason |
|------|--------|
| LZMA SDK 26.00 and 13 build scripts | Dead — minizip-ng covers all use cases |
| MuPDF integration code | Killed — PDFium replaces in Phase 2 |
| `stb_image` PNG code path (Phase 2) | Replace with libpng |
| `tinyexr` main path (Phase 2 eval) | Replace with OpenEXR 3.x |
| Win32 MSBuild configurations | Already removed |

### 17.2 Refactor (Phase 1–2)

| Item | Goal |
|------|------|
| Engine 16→7 dir consolidation | Reduce CMake complexity, cognitive load |
| `LENSArchive.h` format dispatch table | Split at 103 KB threshold into families |
| `RegManager.h` registry table | Split at 103 KB threshold |
| `EngineTestsExterns.h` | Split when >400 KB |
| Cache key struct | Add `decoder_version` field |
| All COM entry points | Add `VerifySTA()` assertion |
| COM boundary types | `static_assert` no STL across boundary |

### 17.3 Rewrite (Phase 2–4)

| Item | Rewrite To | Reason |
|------|-----------|--------|
| Binary thumbnail cache | SQLite with schema v1 | Queryable, evictable, multi-tenant |
| GDI+ resize path | Direct3D 11 | GPU acceleration, HDR support |
| PNG decode path | libpng 1.6.x | Full progressive, 16-bit |
| Engine.lib → Engine.dll | DLL + C ABI | Plugin isolation, version independence |

### 17.4 Add Register (by Phase)

**Phase 1:**
- `dependency-review.yml` CI workflow
- `lens formats` CLI command
- Vanilla JS search in `index.html`
- SSIM CI automation (ssim-validation.yml)
- winget package manifest
- `[[nodiscard("reason")]]` on all `std::expected` sites
- COM boundary `static_assert` guards
- ProbeCache in-memory memoization
- Decoder version in cache key

**Phase 2:**
- SQLite L2 cache (thumbnails.db)
- PDFium decoder
- D3D11 resize pipeline
- IStreamingDecoder interface
- `std::jthread` decode workers
- `std::mdspan` pixel views
- 16-bit decode pipeline (H24)
- libpng, libtiff decoders
- EXIF-aware rotation in all decoders (H19)
- Property-based pixel tests

**Phase 3:**
- IPreviewHandler COM interface
- IPropertyStore COM interface
- IContextMenu COM interface
- SVG decoder (resvg)
- Video keyframe decoder (MediaFoundation)
- Font specimen decoder (FreeType 2)
- glTF/OBJ/STL decoder (tiny_gltf + par_shapes)
- Plugin trust chain + signed manifest validator
- System tray icon
- First-run wizard
- Live ETW panel in LENSManager
- Plugin JSON schema + validator (H23)
- Folder cover image (H17)

**Phase 4:**
- OOP AppContainer plugin host
- WER crash handler
- EV code signing
- ASAN + fuzz CI
- REST API (cpp-httplib)
- mTLS auth
- ADMX Group Policy template (H1)
- pHash in cache schema
- Docker / ghcr.io image
- OSS-Fuzz integration (H21)

**Phase 5+:**
- ONNX Runtime AI features (`[ai]` flag)
- macOS Quick Look provider
- Microsoft Store MSIX
- Homebrew formula

---

## 18. 9-Phase Plan to Best-in-Class

### Phase 1 — Foundation Locked (v39.x → v40.0)

**Goal:** Clean slate before adding features. All Phase 1 items are P0 blockers for Phase 2.

| Task | Status | Notes |
|------|--------|-------|
| Engine 16→7 dir consolidation | ❌ Todo | CMake + headers update |
| STA compliance audit + `VerifySTA()` | ❌ Todo | All COM entry points |
| Hard retire LZMA SDK + 13 dead scripts | ❌ Todo | Delete + CMake clean |
| Decoder version in cache key | ❌ Todo | CacheKey_v2 struct |
| COM boundary `static_assert` guards | ❌ Todo | No STL across boundary |
| ProbeCache in-memory memoization | ❌ Todo | Avoid re-probe on refresh |
| `dependency-review` GH Action | ❌ Todo | Block CVE supply chain |
| winget package manifest + PR | ❌ Todo | winget-pkgs repo |
| `lens formats` CLI command (JSON) | ❌ Todo | Format catalogue tooling |
| Vanilla JS search in index.html | ❌ Todo | Format catalogue UX |
| SSIM CI automation | ❌ Todo | ssim-validation.yml |
| Corpus grow to 150 files | ❌ Todo | CC0 files for P1 decoders |
| `[[nodiscard("reason")]]` audit | ❌ Todo | All std::expected sites |
| Split `LENSArchive.h` if >200 KB | ❌ Todo | File size policy |
| Structured cache blob v1 | ❌ Todo | Version header + mmap |

### Phase 2 — Performance, Cache & Quality (v40.x)

| Task | Notes |
|------|-------|
| SQLite L2 cache (thumbnails.db) | Replace bespoke binary cache |
| PDFium decoder | Accelerated from Phase 3 |
| D3D11 GPU resize pipeline | Replace GDI+ batch resize |
| IStreamingDecoder for >50 MB | PSD, TIFF, large TGA |
| `std::jthread` decode workers | Cancellable, Explorer-cancel aware |
| `std::mdspan` pixel buffer views | Eliminate stride math bugs |
| 16-bit decode pipeline (H24) | No premature 8-bit clamping |
| libpng + libtiff (replace stb for these) | Full progressive + streaming |
| EXIF-aware rotation in all decoders (H19) | Smart rotation baked in |
| `/analyze` + SAL Phase 2 | Static analysis hardening |
| `/CETCOMPAT` linker flag | CET hardware enforcement |
| `coverage.yml` + `asan-nightly.yml` | CI quality additions |
| Corpus grow to 300 files | SSIM-validated all P0/P1 |
| OpenEXR 3.x eval (replace tinyexr) | Full EXR support |
| Rust research lane eval | Go/kill decision |
| Async placeholder thumbnail (H7) | Low-res → upgrade async |
| Tile-based large TIFF decode (H10) | Zero full-load for pyramids |

### Phase 3 — Shell Citizen (v41.x)

| Task | Notes |
|------|-------|
| IPreviewHandler (Space-bar preview) | H3 from QuickLook |
| IPropertyStore (Details pane) | EXIF/XMP/IPTC metadata |
| IContextMenu (right-click actions) | Generate, copy, compare |
| SVG decoder (resvg via C API) | Phase 3 P1 |
| Video keyframe decoder (MediaFoundation) | 10% seek point H6 |
| Font specimen decoder (FreeType 2) | Alphabet + numerals sheet |
| 3D model thumbnails (tiny_gltf) | glTF 2.0 + OBJ |
| Plugin trust chain (signed manifest) | H4 + H23 |
| System tray icon | Quick settings |
| First-run wizard | Onboarding UX H8 |
| Live ETW panel in LENSManager | O5 |
| Folder cover image (H17) | Eagle-inspired |
| COM integration tests (T7) | Register DLL in test harness |
| Corpus grow to 500 files | All P0/P1/P2 decoders |
| WinUI 3 gate evaluation (H20) | Go/no-go decision |

### Phase 4 — Enterprise & Hardening (v42.x)

| Task | Notes |
|------|-------|
| OOP AppContainer plugin host (S4) | Low-integrity surrogate |
| Engine.lib → Engine.dll (B1) | C ABI, version-independent |
| REST API (cpp-httplib) | H22 — stateless decode service |
| mTLS for REST | S12 |
| WER crash handler (O3) | Custom dump + opt-in telemetry |
| EV code signing (S5) | Authenticode SHA-256 |
| ASAN build in CI (S11) | `/fsanitize=address` |
| libFuzzer per decoder (T6) | OSS-Fuzz (H21) |
| ADMX Group Policy (H1) | Enterprise config |
| pHash in SQLite schema | Deduplication |
| Docker / ghcr.io image | REST server distribution |
| Corpus grow to 750 files | Fuzz-seeded |

### Phase 5 — AI & macOS (v43.x)

| Task | Notes |
|------|-------|
| ONNX Runtime integration | `[ai]` feature flag |
| Scene understanding (MobileNet V3) | Classify foreground |
| Smart crop (U²Net) | Saliency-based center |
| Neural upscaling (ESRGAN 4x) | Low-res thumbnail upgrade |
| macOS Quick Look provider | Platform/ stub expansion |
| Microsoft Store (MSIX) | Signed package |
| Homebrew formula | macOS distribution |

### Phase 6 — Linux & WASM (v44.x)

| Task | Notes |
|------|-------|
| Linux Nautilus / KIO thumbnailer | Tumbler D-Bus (H18) |
| Flatpak distribution | Linux packaging |
| Vulkan resize (Linux GPU) | Linux GPU path |
| Emscripten WASM decode | Browser demo |
| npm CLI wrapper | Cross-platform tooling |
| CLIP semantic search | `lens search "concept"` |
| Face-aware crop | RetinaFace (ONNX) |

### Phase 7 — Ecosystem

- Stereo / panoramic image viewer
- Design file formats (Figma export, Sketch)
- Third-party plugin catalog (lens-plugins.io)
- 5+ community-contributed decoder plugins

### Phase 8 — Maturity & Scale

- 1,000+ corpus files with full SSIM coverage
- Published benchmarks vs. PowerToys, macOS QL, Icaros
- Community plugin ecosystem established
- Enterprise case studies published

### Phase 9 — Long-term Research

- Direct3D 12 Video decode pipeline
- Neural upscaling custom kernel (non-ONNX)
- WASM plugin sandbox for third-party parsers
- Formal verification of core decode path (limited scope)

---

## 19. Success Metrics & Exit Criteria

### Technical KPIs

| Metric | Now | Phase 1 | Phase 2 | Phase 3 | Phase 4 |
|--------|-----|---------|---------|---------|---------|
| Unit tests | 5,045 | 5,300 | 6,000 | 7,500 | 9,000 |
| Catch2 tests | 1,380 | 1,500 | 2,000 | 2,500 | 3,000 |
| Corpus files | 106 | 150 | 300 | 500 | 750 |
| SSIM ≥ 0.95 decoders | Manual | All P0 | All P0/P1 | All P0/P1/P2 | All |
| Thumbnail P50 latency | 17ms | ≤15ms | ≤12ms | ≤10ms | ≤10ms |
| Cache L2 hit rate | N/A | N/A | ≥80% | ≥85% | ≥90% |
| Install size | 29MB | 30MB | 35MB | 45MB | 50MB |
| Build warnings | 0 | 0 | 0 | 0 | 0 |

### Product KPIs

| Metric | Phase 1 | Phase 2 | Phase 3 | Phase 4 |
|--------|---------|---------|---------|---------|
| Format families supported | 15 | 18 | 24 | 26 |
| COM interfaces | 1 | 1 | 4 | 6 |
| Distribution channels | 1 (GH) | 3 (GH+Choco+NuGet) | 4 | 6 |
| Competitor matrix score | 14/32 | 18/32 | 25/32 | 30/32 |
| CI workflow count | 27 | 30 | 32 | 35 |
| ADRs documented | 22 | 25 | 30 | 35 |

---

## 20. Architecture Decision Log (v6.0)

22 decisions recorded. New in v6.0: ADR-012 through ADR-022.

| ADR | Title | Status | Phase |
|-----|-------|--------|-------|
| ADR-001 | COM IThumbnailProvider as delivery model | Accepted | Done |
| ADR-002 | C++20 → C++23 upgrade | Accepted | Done/P1 |
| ADR-003 | `std::expected` for error propagation (no exceptions across COM) | Accepted | Done |
| ADR-004 | Custom TEST/ASSERT macro harness (not GTest) | Accepted | Done |
| ADR-005 | SQLite for L2 thumbnail cache (replaces bespoke binary) | Accepted | Phase 2 |
| ADR-006 | CMake 4.x + Ninja as Engine build system | Accepted | Done |
| ADR-007 | vcpkg manifest mode for external dependencies | Accepted | Done |
| ADR-008 | LibRaw embedded preview as RAW fast path | Accepted | Done |
| ADR-009 | libjpeg-turbo fast-path decoder for JPEG | Accepted | Done |
| ADR-010 | WTL for LENSManager GUI (WinUI 3 evaluated Phase 3) | Accepted | Done/P3 gate |
| ADR-011 | LZMA SDK hard retire; minizip-ng covers use cases | Accepted | Phase 1 |
| ADR-012 | Engine directory consolidation 16→7 (Phase 1 blocker) | Accepted | Phase 1 |
| ADR-013 | Cache key v2: add decoder_version field | Accepted | Phase 1 |
| ADR-014 | PDFium replaces MuPDF; accelerated to Phase 2 | Accepted | Phase 2 |
| ADR-015 | IStreamingDecoder interface for files >50 MB | Accepted | Phase 2 |
| ADR-016 | `std::mdspan` for 2D pixel buffer views (Phase 2) | Accepted | Phase 2 |
| ADR-017 | Direct3D 11 replaces GDI+ for GPU resize (Phase 2) | Accepted | Phase 2 |
| ADR-018 | OOP AppContainer for plugin host (Phase 4) | Accepted | Phase 4 |
| ADR-019 | Rust research lane for parser modules (go/kill Phase 2) | Proposed | Phase 2 |
| ADR-020 | REST API via cpp-httplib — no Boost (Phase 4) | Accepted | Phase 4 |
| ADR-021 | Strict COM boundary: no STL containers across DLL boundary | Accepted | Phase 1 |
| ADR-022 | Structured cache blob format v1 with version header + mmap | Accepted | Phase 1 |

---

## 21. AI Tooling Surface & MCP

### Asset Inventory

| Asset | Location | Role |
|-------|----------|------|
| Repository rules | `.github/copilot-instructions.md` | Primary project contract |
| Scoped instructions | `.github/instructions/*.instructions.md` | Domain-specific rules (13 files) |
| Custom agents | `.github/agents/*.agent.md` | 5 specialized agents + Explore |
| Prompt templates | `.github/prompts/*.prompt.md` | 14 reusable prompts |
| Repository skills | `.github/skills/*/SKILL.md` | 7 task playbooks |
| Capability reference | `.github/standards/ai-tooling-capabilities.md` | Canonical inventory |
| MCP configuration | `.vscode/mcp.json` | Workspace MCP servers |

### MCP Servers

| Server | Package | Scope |
|--------|---------|-------|
| `github` | `@modelcontextprotocol/server-github` | GitHub API operations |
| `filesystem` | `@modelcontextprotocol/server-filesystem` | Full workspace read/write |
| `project-docs` | `@modelcontextprotocol/server-filesystem` | `.github/` + `docs/` only |

### Custom Agents

| Agent | Specialty |
|-------|-----------|
| ExplorerLens | C++20/23 Engine development, build, COM, GPU |
| Docs | Documentation accuracy, ADR authoring, link checking |
| Release | Version bumps, artifact validation, post-release checks |
| TestCorpus | Corpus management, SSIM, decoder validation |
| CI-Ops | Workflow authoring, action version audit, failure triage |
| Explore | Fast read-only codebase Q&A |

---

## 22. Consolidated Work — v5.0 Survivors

Items from v5.0 that survived re-examination and remain in the backlog.

| Item | Domain | Phase | v6.0 Change |
|------|--------|-------|-------------|
| IPreviewHandler | Frontend | P3 | Unchanged |
| IPropertyStore | Frontend | P3 | Unchanged |
| IContextMenu | Frontend | P3 | Unchanged |
| Plugin ecosystem (signed manifest) | Engine | P3 | Redesigned for OOP surrogate P4 |
| Video keyframe decoder | Decoders | P3 | MediaFoundation preferred |
| Font specimen decoder | Decoders | P3 | FreeType 2 confirmed |
| SVG decoder | Decoders | P3 | resvg via C API confirmed |
| glTF/3D decoder | Decoders | P3 | tiny_gltf + par_shapes |
| Engine.dll (Engine.lib migration) | Backend | P4 | Unchanged |
| OOP AppContainer | Security | P4 | Unchanged |
| EV code signing | Security | P4 | Unchanged |
| ASAN CI | Security | P4 | Unchanged |
| WER crash handler | Observability | P4 | Unchanged |
| REST API (cpp-httplib) | Backend | P4 | No Boost confirmed |
| ADMX Group Policy | Enterprise | P4 | Unchanged |
| pHash in cache schema | Cache | P4 | Added to SQLite schema §12 |
| ONNX Runtime AI | AI | P5 | MobileNet V3 + U²Net + ESRGAN |
| macOS Quick Look | Platform | P5 | Unchanged |
| Microsoft Store MSIX | Distribution | P5 | Unchanged |
| Linux Nautilus/KIO | Platform | P6 | Tumbler D-Bus model |
| WASM Emscripten | Platform | P6 | Research |
| CLIP semantic search | AI | P6 | Unchanged |
| D3D12 Video | GPU | P9 | Long-term research |
| Formal verification (limited) | Quality | P9 | Unchanged |
| SSIM CI automation | Testing | P1 | **Promoted — Phase 1 blocker** |
| winget package | Distribution | P1 | **Promoted — Phase 1 blocker** |
| Corpus 150 files | Corpus | P1 | **Promoted — Phase 1 blocker** |
| SQLite L2 cache | Cache | P2 | Accelerated (was P3 in v4.0) |

---

## 23. Sprint Delivery Log

| Session | Sprints | Key Deliverables |
|---------|---------|-----------------|
| Session 1 | S1–S10 | Engine foundation, COM registration, IThumbnailProvider |
| Session 2 | S11–S30 | Decoder framework, JPEG/PNG/WebP/BMP, LENSManager WTL |
| Session 3 | S31–S80 | HEIC/AVIF/RAW/DDS/EXR decoders, archive support, ETW |
| Session 4 | S81–S130 | GPU pipeline stub, plugin framework, Memory/Cache subsystems |
| Session 5 | S131–S180 | AI modules stub, Media scrubber stub, Platform PAL, CLI |
| Session 6 | S181–S200 | AI tooling surface, Catch2 integration, dev container |
| Session 7 | S201–S210 | Scoped instructions (13 files), 7 skills, 6 agents, 14 prompts |
| Session 8 | S211–S220 | `std::expected` migration, EngineError enum, NOTICE/LGPL compliance |
| Session 9 | S221–S230 | libjpeg-turbo fast path, LibRaw embedded preview, Catch2 test suites, ROADMAP v6.0 |

### Session 9 — Sprint Detail (S221–S230)

| Sprint | File(s) | Description |
|--------|---------|-------------|
| S221 | `Engine/Decoders/JpegTurboDecoder.h` | libjpeg-turbo 3.x fast-path decoder interface |
| S222 | `Engine/Decoders/RawEmbeddedPreviewDecoder.h` | LibRaw::unpack_thumb() pipeline decoder |
| S223 | `Engine/Decoders/ICodecModule.h` + forwarding | Codec module interface (forwarding → Codec/) |
| S224 | `Engine/Decoders/CodecLoader.h` | Dynamic codec loader |
| S225 | `Engine/Decoders/CodecModuleSpecs.h` | Codec spec structs |
| S226 | `Engine/Decoders/FormatConverter.h` | Format conversion utilities |
| S227 | `Engine/Decoders/LazyCodecManager.h` | Lazy initialization codec manager |
| S228 | `Engine/Core/CLICommandParser.h` + `LensCLI.h` | CLI command parsing |
| S229 | `Engine/Tests/Catch2Tests/` (3 files) | 75 Catch2 tests: std::expected, JpegTurbo, EmbeddedPreview |
| S230 | `NOTICE`, `ROADMAP.md` v6.0 | LGPL compliance file; Strategic Roadmap v6.0 |

---

*ExplorerLens ROADMAP v6.0 — April 2026 — targeting best-in-class Windows thumbnail provider*
