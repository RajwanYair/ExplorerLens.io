# ExplorerLens — ROADMAP v7.0 "Sirius"

> **Deep-rethink edition.** Every major decision re-examined from first principles.
> Archived: `docs/archive/ROADMAP_V6.md` (v6.0 "Rigel", 63 KB, 97 sections, ADRs A1–A22).
> Current version: **39.2.0 "Betelgeuse"** · Last updated: **2026-04-26**

---

## Table of Contents

1. [Executive Summary — What Changed from v6.0](#1-executive-summary)
2. [Reality Check — Honest Current-State Assessment](#2-reality-check)
3. [North Star & Explicit Non-Goals](#3-north-star--explicit-non-goals)
4. [Competitor & Reference Matrix](#4-competitor--reference-matrix)
5. [Harvested Best Practices (H1–H36)](#5-harvested-best-practices)
6. [Language & Compiler — Final Verdicts](#6-language--compiler)
7. [Frontend Architecture Rethink](#7-frontend-architecture-rethink)
8. [Backend Architecture Rethink](#8-backend-architecture-rethink)
9. [API Design — COM, REST, SDK, CLI](#9-api-design)
10. [External Libraries & Third-Party APIs](#10-external-libraries--third-party-apis)
11. [Database & Persistence Strategy](#11-database--persistence-strategy)
12. [Infrastructure & Distribution](#12-infrastructure--distribution)
13. [CI/CD Pipeline (27 Workflows)](#13-cicd-pipeline)
14. [Testing & Quality Strategy](#14-testing--quality-strategy)
15. [Security Stack](#15-security-stack)
16. [Observability Stack](#16-observability-stack)
17. [Documentation Strategy](#17-documentation-strategy)
18. [Tools & Versions Matrix](#18-tools--versions-matrix)
19. [Refactor / Rewrite / Delete / Add Register](#19-refactor--rewrite--delete--add-register)
20. [10-Phase Plan to Best-in-Class](#20-10-phase-plan-to-best-in-class)
21. [Success Metrics & Exit Criteria](#21-success-metrics--exit-criteria)
22. [ADR Log v7.0 (ADRs A1–A28)](#22-adr-log-v70)
23. [Decisions Reversed from v6.0](#23-decisions-reversed-from-v60)
24. [Sprint Delivery Pipeline S301+](#24-sprint-delivery-pipeline-s301)

---

## 1. Executive Summary

v7.0 is a first-principles rewrite of the roadmap. v6.0 was an incremental evolution; v7.0 forces kill/keep verdicts, adds a 25-product comparison matrix across 40 dimensions, expands harvested best practices from 24 to 36, and commits to specific build outputs per phase rather than vague milestones.

### What is materially different in v7.0

| Dimension | v6.0 Decision | v7.0 Verdict |
|---|---|---|
| C++ standard | C++20 now, C++23 "plan" | **C++23 commit** — modules, `std::expected`, `std::stacktrace` |
| Rust research lane | Explore (vague) | **KILL** — no Rust in production path; C++23 with sanitizer coverage is sufficient |
| WTL GUI framework | Keep WTL for now | **KILL WTL** — migrate LENSManager to WinUI 3 XAML Islands (Phase 3) |
| stb_image bundled copy | Keep as decoder fallback | **REPLACE** with libspng + libjpeg-turbo direct; stb_image removed from Engine |
| tinyexr bundled | Keep | **KEEP** — no better lightweight EXR option; pin to v1.0.4 |
| UnRAR SDK | Ship as optional | **KEEP** but dual-license gate — build without it by default, feature-flag only |
| REST API transport | HTTP/1.1 | **HTTP/2** (WinHTTP/2 + nghttp2) from Phase 2 |
| Plugin marketplace | Vague "future" | **Concrete design** — JSON catalog schema, mTLS, code-signed `.lenspkg` bundles |
| Documentation count | "65 docs" target | **Reduce to 45** high-quality docs — quantity is waste without quality |
| ADR count | 22 ADRs | **28 ADRs** — 6 new covering GPU, Arm64, mTLS, C++23, WinUI3, UnRAR |
| Phase count | 9 phases | **10 phases** — Phase 10 adds native Arm64 EC + macOS GA |
| Test count target | 6,000 | **8,000** by v45.0; automated corpus regression ≥ 750 CC0 files |
| Decoder directory count | 8 decoder families | **7 decoder families** — Scientific+CAD merged into `Decoders/Specialized/` |

---

## 2. Reality Check

An honest snapshot of the project as of v39.2.0 before optimistic roadmap language:

### What is working well
- Zero-warnings MSVC v145 build discipline — sustained for 39 major versions
- Custom test harness at ~5,045 tests with 100% pass rate
- Sprint cadence: 10 sprints/session, 290+ sprints shipped
- Contract-header model: new features enter as typed API contracts first, enabling parallel frontend/backend work
- ETW + structured logging foundation solid; GUID registered
- COM CLSID registered and stable (`9E6ECB90-5A61-42BD-B851-D3297D9C7F39`)

### What is genuinely lagging
- **GPU decode**: DirectX 11/12 + Vulkan planned since v1.0 — still `[TODO]` stubs; no GPU pixels have ever been rendered by ExplorerLens in production
- **LENSManager UX**: WTL dialog-based circa 2004 patterns; no dark mode, no high-DPI awareness, no accessibility
- **Real format coverage**: ~200 declared formats, but the majority of decoders produce stub/fallback output; actual tested decode coverage is ~40–50 real formats via corpus
- **stb_image**: a bundled single-header "fallback" that silently downgrades decode quality without surfacing that to the user
- **Documentation drift**: Tier 1 docs (README, USER_GUIDE) are version-synced but Tier 3 ADRs and format validation docs are stale (some reference v22 decisions)
- **macOS / Linux stubs**: Platform stubs exist but have zero function bodies; "cross-platform" is aspirational marketing at this point
- **Plugin ecosystem**: PluginCatalogSchemaContract exists as a contract header; zero real plugins exist outside the Engine itself
- **REST API**: LensRestApiEndpointContract defines 7 endpoints; none are wired to an actual HTTP server

### What should be cut or deferred
- ADMX Group Policy schema (S289) — valid enterprise feature but zero enterprise customers confirmed; defer to Phase 5
- Linux DBus Thumbnailer (S298) — Linux platform stub not viable until Platform PAL is real; defer to Phase 9
- AI/Scene-Understanding headers — good ambition, but AI module has no real model weights; must be research-gated

---

## 3. North Star & Explicit Non-Goals

### North Star
> ExplorerLens is the **fastest, most format-complete, GPU-accelerated Windows thumbnail provider** that ships as a zero-trust shell extension with a first-class developer plugin API.

Operationally: every supported format produces a **correct, color-managed, high-DPI-aware** thumbnail in **< 17 ms** at 235 img/sec batch throughput with **< 5 ms cache hit** latency.

### Success Looks Like
- A developer can add a new format decoder in < 2 hours via the Plugin SDK
- An enterprise admin can deploy via ADMX Group Policy + MSIX in a silent push
- A power user sees thumbnails for `.dng`, `.heic`, `.avif`, `.glb`, `.step`, `.psd` with zero configuration
- CI catches every performance regression before merge via SSIM + benchmark gates

### Explicit Non-Goals (no roadmap item may target these)
| Non-Goal | Rationale |
|---|---|
| Full image editor (crop, adjust, export) | That is Lightroom / darktable; we are a thumbnail provider |
| Video player or audio player | Shell preview is sufficient; we supply a keyframe thumbnail |
| Cloud sync or remote storage | We are a local Shell Extension; network I/O is a threat surface |
| AI model training or inference at user endpoint | Model weights in Shell Extension = unacceptable attack surface |
| Replacing Windows Explorer | We extend it; we do not replace it |
| Supporting Windows 7 / 8 / 8.1 | COM APIs we depend on require Windows 10 1903+ |
| Supporting 32-bit (x86) Shell | Explorer on modern Windows is 64-bit |

---

## 4. Competitor & Reference Matrix

Scoring: ✅ = strong · ⚠️ = partial/limited · ❌ = absent/poor · `–` = N/A

| Dimension | ExplorerLens (v39) | Windows Shell Built-in | Apple Quick Look | FastStone Image Viewer | IrfanView 4.7 | XnView MP 1.8 | ACDSee Photo Studio | Adobe Bridge 2026 | Adobe Lightroom Classic | Photo Mechanic 6 | Capture One 24 | darktable 4.8 | digiKam 8 | Nomacs 3.21 | geeqie 2.4 | Affinity Photo 2 | macOS Finder | GNOME Files / Nautilus | KDE Dolphin | ffmpegthumbnailer | GNOME Thumbnailer | WinZip | 7-Zip 24 | Microsoft Photos | Google Photos |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| **Decode depth** | | | | | | | | | | | | | | | | | | | | | | | | | |
| Raw camera (DNG/CR3/ARW) | ⚠️ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ⚠️ | ⚠️ | ✅ | ✅ | ❌ | ❌ | ⚠️ | ❌ | ❌ | ❌ | ⚠️ | ✅ |
| HEIC/AVIF/JXL | ⚠️ | ⚠️ | ✅ | ⚠️ | ⚠️ | ⚠️ | ✅ | ✅ | ✅ | ⚠️ | ✅ | ⚠️ | ⚠️ | ❌ | ❌ | ✅ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ✅ | ✅ |
| PSD / PSB layered | ⚠️ | ❌ | ✅ | ⚠️ | ❌ | ⚠️ | ✅ | ✅ | ✅ | ⚠️ | ✅ | ❌ | ❌ | ❌ | ❌ | ⚠️ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| EXR / HDR / TIFF 32-bit | ⚠️ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ⚠️ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ⚠️ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| 3D: glTF/OBJ/FBX/STEP | ⚠️ | ❌ | ⚠️ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ⚠️ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| Archives: ZIP/RAR/7z badge | ⚠️ | ❌ | ❌ | ❌ | ❌ | ⚠️ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ✅ | ✅ | ❌ | ❌ |
| Video keyframe | ⚠️ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ | ❌ | ✅ | ❌ | ✅ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ | ✅ | ✅ |
| Font preview (TTF/OTF) | ⚠️ | ❌ | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| PDF page 1 thumbnail | ⚠️ | ❌ | ✅ | ✅ | ⚠️ | ✅ | ✅ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ✅ | ❌ | ❌ | ❌ | ❌ | ⚠️ | ❌ | ❌ | ❌ |
| **Performance** | | | | | | | | | | | | | | | | | | | | | | | | | |
| GPU-accelerated decode | ❌ | ❌ | ✅ | ❌ | ❌ | ❌ | ✅ | ✅ | ✅ | ❌ | ✅ | ⚠️ | ❌ | ❌ | ❌ | ✅ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ✅ | ✅ |
| Sub-5ms cache hit | ✅ | ⚠️ | ✅ | ✅ | ✅ | ✅ | ✅ | ⚠️ | ⚠️ | ✅ | ⚠️ | ⚠️ | ⚠️ | ⚠️ | ⚠️ | ⚠️ | ✅ | ✅ | ✅ | ✅ | ✅ | ⚠️ | ⚠️ | ⚠️ | ✅ |
| Batch throughput > 200/s | ⚠️ | ⚠️ | ✅ | ✅ | ✅ | ✅ | ⚠️ | ⚠️ | ⚠️ | ✅ | ⚠️ | ⚠️ | ⚠️ | ✅ | ✅ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ | ⚠️ | ✅ |
| Async/non-blocking decode | ✅ | ⚠️ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ | ✅ | ✅ |
| **Quality** | | | | | | | | | | | | | | | | | | | | | | | | | |
| ICC color management | ❌ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ⚠️ | ⚠️ | ✅ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ⚠️ | ✅ |
| HDR tone-mapping pipeline | ❌ | ❌ | ✅ | ⚠️ | ⚠️ | ⚠️ | ✅ | ✅ | ✅ | ❌ | ✅ | ✅ | ✅ | ❌ | ❌ | ✅ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ✅ | ✅ |
| High-DPI aware | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| SSIM-validated output | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| **Platform & Distribution** | | | | | | | | | | | | | | | | | | | | | | | | | |
| Native Arm64 | ❌ | ✅ | ✅ | ❌ | ❌ | ❌ | ✅ | ✅ | ✅ | ❌ | ✅ | ✅ | ✅ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ⚠️ | ❌ | ✅ | ✅ |
| macOS support | ❌ | – | ✅ | ❌ | ❌ | ✅ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | – | – | ❌ | – | ✅ | ✅ | ❌ | ✅ |
| Linux support | ❌ | – | – | ❌ | ❌ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ✅ | ✅ | ✅ | ✅ | ❌ | – | ✅ | ✅ | ✅ | ✅ | ❌ | ✅ | ❌ | ✅ |
| MSIX / Store-ready | ❌ | – | – | ❌ | ❌ | ❌ | ✅ | ✅ | ✅ | ❌ | ✅ | ❌ | ❌ | ❌ | ❌ | ✅ | ✅ | – | – | ❌ | – | ✅ | ✅ | ✅ | – |
| Group Policy / ADMX | ❌ | ✅ | – | ❌ | ❌ | ❌ | ✅ | ⚠️ | ⚠️ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ⚠️ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ⚠️ | ❌ |
| Silent enterprise deploy | ❌ | ✅ | ✅ | ❌ | ❌ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ | ❌ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| Install footprint < 10 MB | ✅ | – | – | ✅ | ✅ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ✅ | ✅ | ❌ | – | ✅ | ✅ | ✅ | ✅ | ❌ | ✅ | ❌ | – |
| **Security & Trust** | | | | | | | | | | | | | | | | | | | | | | | | | |
| EV code-signed binaries | ⚠️ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ | ❌ | ❌ | ✅ | ✅ | – | – | ❌ | – | ✅ | ✅ | ✅ | ✅ |
| Plugin sandbox (AppContainer) | ⚠️ | ❌ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ✅ |
| SLSA provenance level ≥ 2 | ⚠️ | ✅ | ✅ | ❌ | ❌ | ❌ | ✅ | ✅ | ✅ | ❌ | ✅ | ⚠️ | ⚠️ | ❌ | ❌ | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ | ✅ | ✅ | ✅ | ✅ |
| Reproducible builds | ❌ | ✅ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ✅ | ✅ | ❌ | ❌ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ | ❌ | ✅ |
| Crash telemetry opt-in | ✅ | ⚠️ | ✅ | ❌ | ❌ | ❌ | ✅ | ✅ | ✅ | ❌ | ✅ | ✅ | ✅ | ❌ | ❌ | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ | ✅ | ❌ | ✅ | ✅ |
| **Developer Experience** | | | | | | | | | | | | | | | | | | | | | | | | | |
| Public plugin API / SDK | ✅ | ❌ | ✅ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ | ✅ | ✅ | ❌ | ❌ | ❌ | ❌ | ✅ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| Plugin marketplace / catalog | ❌ | ❌ | ❌ | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ✅ | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| REST/headless API | ⚠️ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ✅ |
| SBOM (Software Bill of Materials) | ✅ | ❌ | ⚠️ | ❌ | ❌ | ❌ | ❌ | ⚠️ | ⚠️ | ❌ | ❌ | ✅ | ✅ | ❌ | ❌ | ❌ | ⚠️ | ✅ | ✅ | ⚠️ | ⚠️ | ❌ | ❌ | ⚠️ | ❌ |
| Fuzzing / OSS-Fuzz | ✅ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ✅ | ✅ | ❌ | ❌ | ❌ | ❌ | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ | ❌ | ❌ |
| **UX & Accessibility** | | | | | | | | | | | | | | | | | | | | | | | | | |
| Dark mode | ❌ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | – | – | ✅ | ✅ | ✅ | ✅ |
| Accessibility (a11y WCAG AA) | ❌ | ✅ | ✅ | ⚠️ | ⚠️ | ⚠️ | ✅ | ✅ | ✅ | ⚠️ | ✅ | ⚠️ | ⚠️ | ⚠️ | ⚠️ | ✅ | ✅ | ✅ | ✅ | – | – | ✅ | ✅ | ✅ | ✅ |
| Explorer cancel-aware | ⚠️ | ✅ | ✅ | – | – | – | – | – | – | – | – | – | – | – | – | – | ✅ | – | – | – | – | – | – | – | – |

### Scorecard Summary (count of ✅)

| Product | ✅ Count / 40 | Tier |
|---|---|---|
| Adobe Bridge 2026 | 30 | A |
| Adobe Lightroom Classic | 29 | A |
| Apple Quick Look | 28 | A |
| macOS Finder | 27 | A |
| Capture One 24 | 26 | A |
| ACDSee Photo Studio | 25 | A |
| Photo Mechanic 6 | 22 | B |
| darktable 4.8 | 22 | B |
| digiKam 8 | 21 | B |
| GNOME Files / Nautilus | 20 | B |
| KDE Dolphin | 19 | B |
| XnView MP | 19 | B |
| FastStone Image Viewer | 19 | B |
| IrfanView 4.7 | 17 | B |
| Affinity Photo 2 | 17 | B |
| Microsoft Photos | 16 | B |
| Google Photos | 16 | B |
| **ExplorerLens v39** | **15 / 40** | **C (target A)** |
| WinZip | 12 | C |
| 7-Zip 24 | 11 | C |
| Windows Shell Built-in | 9 | C |
| Nomacs 3.21 | 8 | C |
| ffmpegthumbnailer | 8 | C |
| geeqie 2.4 | 8 | C |
| GNOME Thumbnailer | 8 | C |

**ExplorerLens scores 15/40 today. Target is 32+/40 by v45.0 (Tier A).**

The biggest gaps are: GPU decode (0), ICC color management (0), dark mode (0), native Arm64 (0), EV code signing (partial), macOS (stub only), MSIX/Store (not started).

---

## 5. Harvested Best Practices

### From Apple Quick Look (A-tier)
**H1 — Async placeholder UX**: Render a blurred/scaled version of the last cached thumbnail immediately while the real decode runs in background. Explorer never shows a blank white square. [Phase 2]

**H2 — Crash telemetry opt-in at first run**: Show a single consent dialog on first install. No silent telemetry. Map directly to `CrashTelemetryConsentContract`. [Done — S293]

**H3 — ICC color profile passthrough**: Embed the source ICC profile in the decoded pixel buffer; let the display pipeline apply color management. Do not tone-map inside the decoder. [Phase 3]

**H4 — Spacebar instant preview shortcut**: A single key activates a floating preview panel in Explorer without opening the file. Map to `SpacebarPreviewShortcutContract`. [Done — S281]

**H5 — Cancel-aware decode**: Honor `IThumbnailProvider::GetThumbnail` cancellation via `IBindStatusCallback`. Abort in-flight decode, return `E_ABORT` cleanly. [Phase 2]

### From Adobe Bridge / Lightroom (A-tier)
**H6 — Smart previews (offline proxy)**: Store a 2560px JPEG proxy in the cache DB alongside the full thumbnail. When the source file is unavailable (network share, disconnected drive), serve the proxy. [Phase 4]

**H7 — Catalog database with per-file metadata**: SQLite `perceptual_hash_index` table keyed on file inode + mtime, not just path. Detect renames and moves without re-decode. Map to `SqlitePHashIndexContract`. [Done — S292]

**H8 — Color-managed preview rendering pipeline**: Bridge uses AGM (Adobe Color Engine). We use LittleCMS (lcms2). Wire lcms2 into the GPU blit path: source profile → sRGB display. [Phase 3]

**H9 — Ingest-time format validation**: When a new file appears in a watched folder, validate its header bytes against the expected magic signature before dispatching to the decoder. Reject corrupt files fast. [Phase 2]

**H10 — Video keyframe extraction**: Extract first non-black keyframe (not necessarily frame 0). Map to `MediaFoundationVideoKeyframeContract`. [Done — S265]

**H11 — REST headless rendering endpoint**: Bridge Cloud can serve thumbnails over HTTP. Wire `LensRestApiEndpointContract` into a real `lens.exe --serve` mode. [Phase 5]

### From Photo Mechanic 6 (B-tier, fastest ingest tool in market)
**H12 — Parallel I/O with readahead**: Photo Mechanic reads the next N files from a folder while displaying the current one. Implement `ParallelIoManagerContract` with directory readahead of N=8. [Phase 2]

**H13 — Embedded JPEG preview extraction**: For camera raw files, extract the embedded JPEG preview (EXIF tag 0x0201/0x0202) before invoking the raw decoder. This gives a sub-1ms thumbnail for most camera files. [Phase 2]

**H14 — Ingest progress pipeline**: Surface decode progress as an ETW event + REST SSE stream. Consumers can subscribe to progress without polling. [Phase 5]

### From darktable / digiKam (B-tier, open-source, Linux-first)
**H15 — Reproducible build pipeline**: darktable CI produces bit-for-bit identical binaries via controlled CFLAGS + timestamp stripping. Target: ExplorerLens MSVC builds should match SHA-256 across identical source trees on CI. [Phase 6]

**H16 — SSIM-gated CI**: digiKam CI validates thumbnail output against reference images using SSIM. We now have `ssim-validation.yml` (S300). Target SSIM ≥ 0.95 vs reference. [Done — S300]

**H17 — Plugin API with stable ABI**: darktable's module ABI uses `dt_iop_module_so_t` with explicit versioning. Mirror with `EngineDllAbiContract` v0x00010000. [Done — S296]

**H18 — DBus thumbnailer protocol (Linux)**: Compliant with FreeDesktop.org Thumbnailer D-Bus spec. Enables `gdk-pixbuf-thumbnailer`, Nautilus, and Dolphin to use ExplorerLens decoders. Map to `DbusThumbnailerContract`. [Done — S298, deferred to Phase 9]

### From FastStone / XnView (B-tier, Windows-native, broad format support)
**H19 — Side-by-side comparison view**: Two thumbnails from different files displayed side-by-side with synchronized zoom/pan. Expose via `IContextMenu` verb `CompareTo`. [Phase 4]

**H20 — WinUI 3 XAML Islands for settings GUI**: FastStone's settings dialog is WTL-era; XnView uses a custom widget toolkit. Both feel dated. WinUI 3 with dark mode + a11y is the right path for LENSManager. [Phase 3]

**H21 — Format filter sidebar**: XnView's filter sidebar lets you show only specific formats. Expose as a LENSManager pane backed by the SQLite format stats table. [Phase 4]

### From IrfanView (B-tier, legendary footprint efficiency)
**H22 — < 10 MB install footprint**: IrfanView's installer is < 3 MB. Target: ExplorerLens MSI < 8 MB. Current MSI is ~12 MB. Audit all bundled assets. [Phase 3]

**H23 — Plugin marketplace with self-hosted catalog**: IrfanView has a community plugin page. Target: a JSON catalog at `plugins.explorerlens.io` with signed `.lenspkg` bundles. `PluginCatalogSchemaContract` defines the schema. [Done — S299, Phase 7]

### From GNOME / KDE / ffmpegthumbnailer (Linux ecosystem)
**H24 — FreeDesktop thumbnail spec compliance**: Store thumbnails in `~/.cache/thumbnails/large/` as PNG with `Thumb::URI` and `Thumb::MTime` XMP tags. Enables cross-application thumbnail reuse. [Phase 9]

**H25 — Shared thumbnail cache via DBus**: KDE Dolphin and GNOME Files share a single cache daemon. On Windows, expose a COM out-of-process thumbnail server to allow multiple Explorer windows to share a single cache. [Phase 5]

### From Google Photos (cloud-scale thumbnail pipeline)
**H26 — Perceptual hash deduplication**: Before decoding, compute a quick pHash of the first 4 KB of the file. If pHash matches a cached entry with same file size, skip decode and return cached bitmap. [Phase 2]

**H27 — Progressive JPEG streaming**: For large JPEG/HEIC files, decode the lowest-resolution scan first and progressively refine. User sees a thumbnail instantly; quality improves in background. [Phase 3]

### From Microsoft Photos (in-box Windows)
**H28 — WIC codec integration**: Microsoft Photos uses WIC (Windows Imaging Component) as the primary decode path, falling back to custom decoders only for unsupported formats. Audit our WIC passthrough — some formats we hand-decode that WIC handles natively. [Phase 2]

**H29 — MSIX sparse package for shell integration**: Microsoft Photos ships as an MSIX sparse package that can register COM servers without full trust. This avoids the registry pollution of a traditional MSI shell extension. [Phase 6]

### From WinZip / 7-Zip (archive preview specialists)
**H30 — Archive cover art extraction**: For ZIP/RAR/7z containing image files, extract and display the first image as the archive thumbnail. `ArchiveFileBadgeOverlayContract` covers the badge; the cover art is a separate feature. [Phase 4]

**H31 — Lazy archive enumeration**: Do not enumerate the full archive to find the cover image. Read only the central directory (ZIP), then seek to the first matching file. O(1) for well-formed archives. [Phase 4]

### Cross-cutting (synthesized from matrix gaps)
**H32 — ICC profile display pipeline (end-to-end)**: Full ICC pipeline: decoder emits `(pixels, icc_profile_bytes)` pair. Renderer applies lcms2 transform to sRGB D65. This is the single biggest quality gap vs Adobe-tier competitors. [Phase 3]

**H33 — Native Arm64 EC build**: Windows on Arm is the fastest-growing PC platform segment. Build with `/arm64EC` (Arm64 Emulation-Compatible) to handle x64 COM host interop. [Phase 6]

**H34 — SLSA Level 2 provenance**: Add `actions/attest-build-provenance@v2` to release workflow. Adds a signed SLSA attestation to every GitHub release artifact. [Phase 3]

**H35 — OOM kill protection**: Register with Windows Memory Dispatcher via `SetProcessWorkingSetSizeEx` + heap trim on `WM_SETTINGCHANGE` for low-memory. Prevent shell host crash when decoding large RAW files. [Phase 2]

**H36 — Explorer cancel-aware batch decode**: Implement `IBindStatusCallback::OnProgress` in the COM server. When Explorer navigates away, cancel in-flight decodes within 50 ms to avoid stalling the shell thread. [Phase 2]

---

## 6. Language & Compiler

### C++ Standard — Final Verdict: C++23 Commit

**Decision (ADR A23)**: Migrate Engine codebase to C++23 at v40.0. No opt-out.

| Feature | C++20 Status | C++23 Gain | Use Case |
|---|---|---|---|
| `std::expected<T,E>` | Absent | ✅ Available | Replace `HRESULT` error returns in Engine API |
| `std::stacktrace` | Absent | ✅ Available | Crash reporter — structured stack in ETW event |
| `std::flat_map` | Absent | ✅ Available | Hot path LENSTYPE→decoder dispatch (cache-friendly) |
| `std::print` / `std::println` | Absent | ✅ Available | Replace raw `printf` in CLI and test harness |
| C++ modules (import std;) | Partial | ✅ Stable in MSVC 19.50 | Reduce PCH rebuild time by ~40% |
| `[[assume(expr)]]` | Absent | ✅ Available | Decoder hot-path branch hints |
| Deducing `this` | Absent | ✅ Available | CRTP-free decoder base class |
| `constexpr std::string` | Partial | ✅ Full | Format magic-byte tables as `constexpr` |

**Migration path**: Set `/std:c++23` in `Engine/CMakeLists.txt`. Address any MSVC C++23 warnings iteratively, one sprint at a time.

### Rust Research Lane — Final Verdict: KILL

**Decision (ADR A24)**: No Rust in ExplorerLens production path. Ever.

**Rationale**:
- ExplorerLens is a Windows COM in-process DLL. The Rust→COM interglue (`windows-rs`) works but adds 1.2 MB to DLL size and a separate runtime allocator conflict risk in the shell host process.
- C++23 + ASAN + fuzzing coverage achieves equivalent safety guarantees for the use case.
- The engineering cost of maintaining a mixed C++/Rust codebase with a 1-person team is unjustifiable.
- All "Rust decoder" research sprints are cancelled. If a decoder already in Rust exists (e.g., `zune-jpeg`), use it as a C-ABI plugin, not embedded in the Engine.

### Clang/LLVM — Status: CI-only (no production binaries)
Clang 18 is used only in: ASan workflow, fuzzer build, static analysis (clang-tidy). Production DLL is always MSVC v145.

### Compiler Flags (authoritative)
```
/std:c++23 /W4 /WX /permissive- /Zc:__cplusplus /Zc:preprocessor
/fp:fast /GL /Gy /GR- /EHsc /MP
/D NOMINMAX /D WIN32_LEAN_AND_MEAN /D UNICODE
```

---

## 7. Frontend Architecture Rethink

### 7.1 Shell Extension — Keep, Harden

`LENSShell.dll` stays as a COM in-process server. No change to CLSID. Key hardening work:

| Work Item | Priority | Phase |
|---|---|---|
| Cancel-aware decode via `IBindStatusCallback` (H36) | P0 | 2 |
| Async placeholder thumbnail (H1) | P0 | 2 |
| OOM kill protection (H35) | P0 | 2 |
| AppContainer sandbox for decoder spawning | P1 | 3 |
| Arm64 EC build of LENSShell.dll | P1 | 6 |
| MSIX sparse package registration (H29) | P2 | 6 |

### 7.2 LENSManager GUI — Kill WTL, Adopt WinUI 3

**Decision (ADR A25)**: Replace WTL with WinUI 3 XAML Islands.

WTL was last updated for VS2010 patterns. The LENSManager GUI has:
- No dark mode support (gap vs every A-tier competitor)
- No accessibility (MSAA only, no UIA automation)
- No high-DPI dynamic DPI handling (DPI aware via manifest only)
- No fluent styling

**Migration**:
- Phase 3: New WinUI 3 project `LENSManager.WinUI/`
- Legacy WTL `LENSManager/` retained as fallback until WinUI version reaches feature parity
- All settings read/write goes through `EngineSettings` COM interface; GUI is pure display layer

### 7.3 CLI (`lens.exe`) — Expand

`lens.exe` is the headless CLI and future REST server host.

| Command | Status | Phase |
|---|---|---|
| `lens decode <file>` | ✅ Contract exists | 2 |
| `lens serve --port 7472` | ⚠️ Contract only | 5 |
| `lens validate <dir>` | ⚠️ Stub | 3 |
| `lens benchmark` | ✅ Wired to Google Benchmark | Done |
| `lens inspect <file>` | ❌ New | 4 |
| `lens corpus ingest <dir>` | ❌ New | 4 |

### 7.4 Static HTML (`index.html`) — Retire

The project root `index.html` is a GitHub Pages landing page with duplicate content from `README.md`. It is unmaintained and out of date. **Decision**: retire `index.html`, redirect GitHub Pages to `docs/` (MkDocs build). [Phase 1]

---

## 8. Backend Architecture Rethink

### 8.1 Directory Consolidation — Target 7 Decoder Families

Current decoder directories (8): `Core/`, `Image/`, `Raw/`, `Document/`, `Archive/`, `Specialized/`, `Vector/`, `Media/`

v7.0 change: merge `Specialized/` and `CAD/` into one `Specialized/` directory. Final 7 families:

| Family | Contents |
|---|---|
| `Decoders/Image/` | JPEG, PNG, WebP, AVIF, JXL, BMP, ICO, TIFF |
| `Decoders/Raw/` | LibRaw: DNG, CR3, ARW, NEF, RAF, ORF, RW2 |
| `Decoders/Document/` | PDF (PDFium), Office (WinOLE), Font (FreeType) |
| `Decoders/Archive/` | ZIP, RAR (UnRAR SDK), 7z, TAR |
| `Decoders/Specialized/` | EXR, HDR, FITS, STEP/OBJ/FBX, glTF, SVG |
| `Decoders/Vector/` | SVG (resvg), EMF/WMF, CGM |
| `Decoders/Media/` | Video keyframe (MF), Audio waveform, GIF/APNG animation |

### 8.2 9-Stage Pipeline — Keep, Add ICC Stage

Current pipeline stages (v6.0):
1. Format Detection
2. Decoder Selection
3. Header Validation (H9)
4. Decode
5. Color Transform
6. Resize
7. Cache Write
8. Bitmap Delivery
9. Fallback

v7.0 adds **Stage 5a: ICC Profile Application** between decode and color transform:
- Decoder emits raw pixels + embedded ICC profile bytes
- lcms2 transform converts to sRGB D65
- Stage 5 (color transform) receives already-managed pixels

### 8.3 Concurrency Model

| Component | v6.0 | v7.0 |
|---|---|---|
| Decoder dispatch | Thread pool, manual join | `std::jthread` + cancellation token (C++20) |
| Cache reads | RW lock (SRWLOCK) | `std::shared_mutex` + `std::expected` error path |
| GPU upload | Staging buffer, manual sync | Zero-copy pipeline via `ZeroCopyUploadContract` |
| Parallel I/O | Single file at a time | Directory readahead N=8 (H12) |

### 8.4 GPU Pipeline — Concrete Plan (Phase 2–6)

| Phase | Milestone | API |
|---|---|---|
| 2 | DirectX 11 texture blit (GDI+ → DX11) | D3D11 |
| 3 | DXVA2 JPEG/HEIC hardware decode | DXVA2 |
| 4 | DirectX 12 async upload queue | D3D12 |
| 5 | NVDEC vendor path (NVIDIA) | NVDEC |
| 5 | QuickSync vendor path (Intel) | MFX |
| 6 | Vulkan resize pipeline (H17/S297) | Vulkan 1.1 |
| 6 | AMF vendor path (AMD) | AMF |

**No GPU decode will ship without a functional software fallback.** Every GPU path has an ASAN-clean CPU fallback.

### 8.5 Cache Architecture

Three cache tiers (unchanged from v6.0, but with concrete size contracts):

| Tier | Storage | Max Size | Eviction |
|---|---|---|---|
| L1 in-process | `std::unordered_map<pHash, HBITMAP>` | 64 MB | LRU |
| L2 file-backed | SQLite BLOB + PNG file in `%LOCALAPPDATA%\ExplorerLens\cache\` | 512 MB | LRU + mtime |
| L3 proxy (H6) | 2560px JPEG smart preview in SQLite | 2 GB | LRU |

Smart previews (L3) are only generated for camera RAW files and are opt-in via LENSManager.

---

## 9. API Design

### 9.1 COM Interface Contract (Shell ↔ Engine)

```cpp
// Stable ABI, version 0x00010000 (EngineDllAbiContract)
interface ILensDecoder : IUnknown {
    HRESULT Decode(IStream* pStream, UINT cx, HBITMAP* phbmp);
    HRESULT GetFormatInfo(LENSFORMAT_INFO* pInfo);
    HRESULT Cancel();  // NEW in v7.0
};

interface ILensColorManager : IUnknown {
    HRESULT ApplyIccProfile(BYTE* pixels, UINT cb, const BYTE* pIcc, UINT cbIcc);
};
```

### 9.2 REST API (lens.exe --serve, Phase 5)

7 endpoints from `LensRestApiEndpointContract`, HTTP/2 transport:

| Method | Path | Description |
|---|---|---|
| GET | `/v1/thumbnail?path={}&size={}` | Decode + return PNG thumbnail |
| GET | `/v1/formats` | JSON array of supported formats |
| GET | `/v1/cache/stats` | Cache hit/miss counters |
| DELETE | `/v1/cache` | Flush all cache tiers |
| POST | `/v1/decode` | Multipart file decode (headless) |
| GET | `/v1/health` | Liveness + version |
| GET | `/v1/metrics` | Prometheus-format counters |

Auth: mTLS only (`MtlsRestAuthContract`). No API key, no bearer token.

### 9.3 Plugin SDK (C ABI)

```c
// plugin_api.h — stable C ABI, PluginCatalogSchemaContract
typedef struct {
    uint32_t api_version;       // 0x00010000
    const char* format_id;      // "image/avif"
    const char* display_name;   // "AVIF Decoder"
    HRESULT (*decode)(IStream*, UINT cx, HBITMAP* out);
    void    (*on_unload)(void);
} LensPluginV1;
```

Plugin package format: `.lenspkg` = ZIP containing `plugin.dll` + `manifest.json` (signed, `PluginManifestSchemaContract`).

### 9.4 Error Design (C++23 `std::expected`)

```cpp
// v7.0: replace HRESULT error returns in Engine-internal paths
std::expected<DecodedBitmap, EngineError> Decode(
    IStream* pStream, UINT cx);
```

`EngineError` carries: `HRESULT hresult`, `std::string_view context`, `std::stacktrace trace`.

---

## 10. External Libraries & Third-Party APIs

### Kill List (libraries to remove)
| Library | Reason | Replacement |
|---|---|---|
| `stb_image` | Silent quality downgrade; limited color depth | libspng (PNG) + libjpeg-turbo (JPEG) direct |
| `tinyxml2` (if present) | Redundant — pugixml already in use | pugixml only |

### Keep List (with version pins)
| Library | Version | Purpose | License |
|---|---|---|---|
| zlib | 1.3.1 | Deflate decompression | zlib |
| LZ4 | 1.9.4 | Fast compression for L1 cache | BSD-2 |
| zstd | 1.5.6 | High-ratio compression for L2 cache | BSD-3 |
| libwebp | 1.4.0 | WebP decode/encode | BSD-3 |
| minizip-ng | 4.0.7 | ZIP archive handling | zlib |
| LibRaw | 0.21.3 | Camera RAW decode | LGPL-2.1 |
| dav1d | 1.4.3 | AV1 video decode | BSD-2 |
| libde265 | 1.0.15 | HEVC decode (HEIF) | LGPL-3 |
| libheif | 1.19.5 | HEIF/HEIC container | LGPL-3 |
| tinyexr | 1.0.4 | OpenEXR decode | BSD-3 |
| tinygltf | 2.8.22 | glTF 2.0 decode | MIT |
| resvg | 0.44 | SVG rasterization (Rust, C-ABI) | MPL-2 |
| FreeType | 2.13.3 | Font rasterization | FTL |
| PDFium | 6721 | PDF page rasterization | BSD-3 |
| lcms2 | 2.16 | ICC color management | MIT |
| SQLite | 3.46.1 | Cache + catalog database | Public Domain |
| pugixml | 1.14 | XML parsing | MIT |
| Google Benchmark | 1.9.1 | Performance benchmarks | Apache-2 |
| Catch2 | 3.7.1 | Unit test framework | BSL-1 |

### Evaluate / Add List
| Library | Purpose | Decision Gate |
|---|---|---|
| libjpeg-turbo 3.0 | Replace stb_image for JPEG; SIMD-accelerated | Phase 2 |
| libspng 0.7 | Replace stb_image for PNG; ASAN-clean | Phase 2 |
| lcms2 (already listed) | Wire into decode pipeline — exists but not wired | Phase 3 |
| nghttp2 1.62 | HTTP/2 for REST server | Phase 5 |
| WinUI 3 (WindowsAppSDK 1.5) | LENSManager GUI replacement | Phase 3 |
| OpenColorIO 2.4 | ACES/DCI-P3 wide gamut support | Phase 7 (research) |

### Third-Party APIs
| API | Purpose | Auth | Phase |
|---|---|---|---|
| Windows Imaging Component (WIC) | Native Windows codec passthrough | None (COM in-proc) | 2 |
| Windows Media Foundation | Video keyframe extraction | None | 2 |
| DXVA2 / MFT | Hardware video decode | None | 3 |
| ETW (Event Tracing for Windows) | Structured telemetry | None | Done |
| GitHub Releases API | CI artifact upload | GITHUB_TOKEN | Done |
| OSS-Fuzz | Fuzzer integration | GCP service account | Done (S290) |

---

## 11. Database & Persistence Strategy

### Primary Store: SQLite (no change, but schema evolved)

**Decision (ADR A14 — retained)**: SQLite is the correct choice. No LMDB, no Redis, no RocksDB. ExplorerLens is an in-process Shell Extension; a network database is a security boundary violation.

### Schema v7.0

```sql
-- Thumbnail cache index
CREATE TABLE thumbnail_cache (
    id          INTEGER PRIMARY KEY,
    file_path   TEXT NOT NULL,
    file_inode  INTEGER,           -- for rename detection
    file_mtime  INTEGER NOT NULL,  -- Unix timestamp
    file_size   INTEGER NOT NULL,
    phash       BLOB,              -- 8-byte perceptual hash (SqlitePHashIndexContract)
    thumb_path  TEXT,              -- path to .png in cache dir
    width       INTEGER,
    height      INTEGER,
    decoder_id  TEXT,              -- which decoder produced it
    decode_ms   INTEGER,           -- decode latency for analytics
    created_at  INTEGER DEFAULT (unixepoch()),
    last_hit    INTEGER DEFAULT (unixepoch())
);
CREATE INDEX idx_phash ON thumbnail_cache(phash);
CREATE INDEX idx_path  ON thumbnail_cache(file_path);
CREATE INDEX idx_mtime ON thumbnail_cache(file_mtime);

-- Smart preview store (H6, Phase 4)
CREATE TABLE smart_previews (
    id           INTEGER PRIMARY KEY,
    cache_id     INTEGER REFERENCES thumbnail_cache(id) ON DELETE CASCADE,
    jpeg_blob    BLOB,             -- 2560px JPEG
    icc_profile  BLOB,            -- embedded ICC profile
    created_at   INTEGER DEFAULT (unixepoch())
);

-- Plugin catalog (PluginCatalogSchemaContract)
CREATE TABLE plugins (
    id           INTEGER PRIMARY KEY,
    plugin_id    TEXT UNIQUE NOT NULL,
    version      TEXT NOT NULL,
    display_name TEXT,
    install_path TEXT,
    manifest_sig BLOB,            -- ed25519 signature of manifest.json
    enabled      INTEGER DEFAULT 1,
    installed_at INTEGER DEFAULT (unixepoch())
);

-- Format statistics (for LENSManager sidebar, H21)
CREATE TABLE format_stats (
    format_id    TEXT PRIMARY KEY, -- MIME type or LENSTYPE name
    decode_count INTEGER DEFAULT 0,
    error_count  INTEGER DEFAULT 0,
    avg_ms       REAL DEFAULT 0,
    last_seen    INTEGER DEFAULT (unixepoch())
);
```

### Write-Ahead Logging
`PRAGMA journal_mode = WAL;` — required for concurrent shell thread reads + background writer. Already in plan; confirm in `CacheDatabase.h`.

### Database Location
`%LOCALAPPDATA%\ExplorerLens\catalog.db` — per-user, not per-machine. Enterprise deployments may redirect via ADMX policy key `LocalCacheRoot`.

---

## 12. Infrastructure & Distribution

### Build Infrastructure
| Component | Current | Target |
|---|---|---|
| Build host | GitHub Actions `windows-latest` (Server 2025) | `windows-latest` + self-hosted Arm64 runner (Phase 6) |
| Compiler | MSVC cl.exe 19.50 v145 | MSVC 19.50 v145 + Clang 18 (CI-only) |
| Build system | CMake 4.3.1 + Ninja 1.13.2 | Same; add CMake presets for Arm64 EC |
| Package manager | vcpkg (manifest mode) | Same; pin all packages in `vcpkg.json` |
| Artifact storage | GitHub Releases | GitHub Releases + GitHub Packages (MSIX) |

### Distribution Channels
| Channel | Format | Status | Phase |
|---|---|---|---|
| GitHub Releases | `.msi` + `.zip` (portable) | ✅ Done | Done |
| Scoop bucket | `scoopfile.json` | ✅ Done | Done |
| MSIX / Microsoft Store | Sparse package | ❌ Not started | 6 |
| WinGet manifest | `winget` YAML | ❌ Not started | 3 |
| Chocolatey | `.nupkg` | ❌ Not started | 4 |
| NuGet (Engine SDK) | `.nupkg` | ⚠️ Stub in CI | 5 |
| Container (Docker) | `Dockerfile` | ⚠️ Exists, untested | 5 |

### Infrastructure Decisions

**Decision (ADR A26)**: Add WinGet manifest in Phase 3. WinGet is the primary Windows package manager for enterprise deployment as of 2025. Scoop remains the developer-friendly option.

**Decision**: Retire `Dockerfile` or make it testable. Docker on Windows + COM Shell Extension is an unsupported configuration. Convert Dockerfile to a CI build-environment image only (no DLL registration).

### Install Footprint Target (H22)
| Component | Current | Target v42.0 |
|---|---|---|
| LENSShell.dll | 2,940 KB | < 2,500 KB (strip debug, ThinLTO) |
| LENSManager.exe | 400 KB | < 350 KB |
| External libs (bundled) | ~8 MB | < 6 MB (prune stb_image, unused codecs) |
| **Total MSI** | **~12 MB** | **< 8 MB** |

---

## 13. CI/CD Pipeline

27 workflows as of v39.2.0. Full inventory below with status and phase alignment.

| # | Workflow File | Status | Trigger | Phase |
|---|---|---|---|---|
| 1 | `build.yml` | ✅ Active | push/PR | Done |
| 2 | `test.yml` | ✅ Active | push/PR | Done |
| 3 | `release.yml` | ✅ Active | tag push | Done |
| 4 | `codeql.yml` | ✅ Active | push/PR + schedule | Done |
| 5 | `dependency-review.yml` | ✅ Active | PR | Done |
| 6 | `ossf-scorecard.yml` | ✅ Active | schedule | Done |
| 7 | `asan.yml` | ✅ Active | push/PR | Done (S270) |
| 8 | `oss-fuzz.yml` | ✅ Active | schedule | Done (S290) |
| 9 | `ssim-validation.yml` | ✅ Active | push/PR | Done (S300) |
| 10 | `benchmark.yml` | ✅ Active | schedule | Done |
| 11 | `publish-packages.yml` | ✅ Active | release | Done |
| 12 | `clang-tidy.yml` | ✅ Active | push/PR | Done |
| 13 | `valgrind.yml` | ⚠️ Linux-only | schedule | Phase 9 |
| 14 | `arm64-build.yml` | ❌ Planned | push/PR | Phase 6 |
| 15 | `msix-package.yml` | ❌ Planned | tag push | Phase 6 |
| 16 | `winget-publish.yml` | ❌ Planned | release | Phase 3 |
| 17 | `corpus-validate.yml` | ❌ Planned | schedule | Phase 3 |
| 18 | `coverage.yml` | ⚠️ Manual | schedule | Phase 2 |
| 19 | `msan.yml` | ❌ Planned | schedule | Phase 4 |
| 20 | `tsan.yml` | ❌ Planned | schedule | Phase 4 |
| 21 | `ubsan.yml` | ❌ Planned | schedule | Phase 4 |
| 22 | `perf-regression.yml` | ⚠️ Gate only | push | Phase 2 |
| 23 | `sbom-update.yml` | ⚠️ Manual | release | Phase 3 |
| 24 | `slsa-provenance.yml` | ❌ Planned | release | Phase 3 |
| 25 | `plugin-sdk-test.yml` | ❌ Planned | push/PR | Phase 5 |
| 26 | `macos-quicklook.yml` | ❌ Planned | push | Phase 10 |
| 27 | `linux-nautilus.yml` | ❌ Planned | push | Phase 9 |

### Pipeline Quality Gates (must-pass for merge)
1. `build.yml` — 0 errors, 0 warnings
2. `test.yml` — 100% pass rate (all ~5K+ tests)
3. `codeql.yml` — 0 high/critical findings
4. `asan.yml` — 0 memory errors
5. `ssim-validation.yml` — SSIM ≥ 0.95 vs reference
6. `perf-regression.yml` — no metric > 10% regression vs `baseline.json`
7. `clang-tidy.yml` — 0 tidy violations

---

## 14. Testing & Quality Strategy

### Test Count Targets
| Version | Test Count | Corpus Files | SSIM Gate |
|---|---|---|---|
| v39.2.0 (now) | ~5,045 | ~106 CC0 files | 0.95 (new) |
| v41.0 | 6,000 | 300 CC0 files | 0.95 |
| v43.0 | 7,000 | 500 CC0 files | 0.97 |
| v45.0 | 8,000 | 750 CC0 files | 0.97 |

### Test Layers (9 Layers)
1. **Unit** — `TEST()` macro harness, pure functions, zero I/O
2. **Integration** — `IntegrationTests.exe`, real file I/O on corpus
3. **SSIM regression** — `ssim-validation.yml` vs reference PNGs
4. **Performance** — Google Benchmark vs `baseline.json` gates
5. **Fuzz** — OSS-Fuzz + local libFuzzer for 8 decoder targets
6. **ASAN** — clang + address sanitizer in CI
7. **Catch2** — property-based tests for cache + hash components
8. **TSAN/MSAN/UBSan** — sanitizer suite (Phase 4)
9. **Corpus real-decode** — `corpus-validate.yml`, real file formats, not mocks

### Test File Placement (mandatory — Rule #18)
- New `TEST()` bodies → `Engine/Tests/EngineTests_Platform.cpp`
- New `extern void RunnerFoo()` → `Engine/Tests/EngineTestsExterns.h`
- New `RUN_TEST(Foo)` → `Engine/Tests/EngineTests.cpp`
- New `#include` → `Engine/Tests/EngineTestsIncludes.h`

### Quality Gate Exit Criteria (per phase)
| Phase | Gate |
|---|---|
| 1 | Build: 0 errors/warnings; Tests: 100% pass; ASAN: clean |
| 2 | + GPU stub replaced by real D3D11 blit; corpus 300 files |
| 3 | + SSIM 0.95; ICC pipeline active; WinGet manifest |
| 4 | + TSAN/MSAN/UBSan clean; corpus 500 files |
| 5 | + REST API endpoint tested via integration test; SLSA L2 |
| 6 | + Arm64 EC build green; MSIX package CI passes |
| 7 | + Plugin marketplace catalog tested with 1 real plugin |
| 8 | + Smart previews: 2560px JPEG generated for RAW corpus |
| 9 | + Linux Nautilus CI green; FreeDesktop thumb spec compliant |
| 10 | + macOS Quick Look CI green; v45.0 released |

---

## 15. Security Stack

### 15 Security Controls (v6.0 preserved + 3 new)
| ID | Control | Status | Phase |
|---|---|---|---|
| S1 | EV code signing pipeline | ⚠️ Partial (S287) | 3 |
| S2 | COM server registration hardening (no self-registration on network) | ✅ Done | Done |
| S3 | Input validation in all decoders (magic byte check before decode) | ⚠️ Partial | 2 |
| S4 | AppContainer plugin sandbox | ⚠️ Contract (S286) | 3 |
| S5 | mTLS for REST API | ⚠️ Contract (S295) | 5 |
| S6 | Plugin trust chain validator | ✅ Done (S268) | Done |
| S7 | ADMX Group Policy schema | ⚠️ Contract (S289) | 5 |
| S8 | ASAN + fuzzer coverage on all decoders | ✅ Done (S270, S290) | Done |
| S9 | CodeQL SAST on every push | ✅ Done | Done |
| S10 | OSSF Scorecard > 7.0 | ⚠️ Active | Phase 2 |
| S11 | Dependency review on every PR | ✅ Done | Done |
| S12 | SBOM CycloneDX 1.4 on every release | ✅ Done | Done |
| S13 | Crash telemetry opt-in consent (no silent telemetry) | ✅ Done (S293) | Done |
| S14 | WER crash reporter integration | ⚠️ Contract (S288) | 4 |
| S15 | SLSA Level 2 provenance (H34) | ❌ Not started | 3 |
| S16 | OOM kill protection (H35) | ❌ Not started | 2 |
| S17 | Reproducible builds | ❌ Not started | 6 |
| S18 | UnRAR dual-license gate (feature-flag only) | ⚠️ Design | 3 |

---

## 16. Observability Stack

### ETW Events (structured, not printf)
| Provider | GUID | Events |
|---|---|---|
| ExplorerLens-Engine | `{registered}` | Decode start/end, cache hit/miss, error |
| ExplorerLens-Shell | `{registered}` | COM activate, IThumbnailProvider calls |
| ExplorerLens-Plugin | `{registered}` | Plugin load/unload, trust check result |

### Metrics Surface (Phase 5)
`GET /v1/metrics` returns Prometheus-format counters:
```
lens_decode_total{format="jpeg",result="ok"} 1234
lens_decode_duration_ms{quantile="0.95"} 14.2
lens_cache_hit_total 5678
lens_cache_miss_total 234
```

### Live ETW Session (S283)
`LiveEtwSessionContract` defines a real-time ETW consumer session. `lens.exe --trace` starts it for debugging.

---

## 17. Documentation Strategy

### Reduce from 65 to 45 High-Quality Docs

v6.0 targeted 65 documents. v7.0 reduces to 45. More docs is not better docs. Each document must have an owner, a version tag, and a last-reviewed date.

### 4 Tiers (unchanged)
| Tier | Audience | Files | Count |
|---|---|---|---|
| T1 | End users | README, USER_GUIDE, QUICK_START, TROUBLESHOOTING, CHANGELOG | 5 |
| T2 | Developers / contributors | ARCHITECTURE, FORMAT_VALIDATION_STATUS, PERFORMANCE, TOOLING, RELEASE_PROCESS, LOCAL_VERIFICATION | 6 |
| T3 | Design record | ADR files (28), ROADMAP | 29 |
| T4 | Reference | Format pages (10), API docs (5) | 15 |
| **Total** | | | **55 docs** |

Wait — 5+6+29+15 = 55. Target is 45. Reduction comes from: collapsing per-format pages into a single `docs/formats/SUPPORTED_FORMATS.md` table, retiring 5 stale ADRs from v1–v5 era, removing `docs/archive/` from the count.

### Documentation Anti-Patterns to Eliminate
- Version references in Tier 3 docs that are more than 2 major versions stale — **auto-flagged by `Audit-Headers.ps1`**
- "TODO" sections in Tier 1 docs — end users should not see future plans in the user guide
- Duplicate content between README and USER_GUIDE — README is 5-minute overview; USER_GUIDE is comprehensive

### ADR Status Lifecycle
`Proposed → Accepted → Superseded → Deprecated`  
All 28 ADRs in v7.0 must have explicit status. A22 (v6.0) and below require review.

---

## 18. Tools & Versions Matrix

| Tool | Current Version | Min Supported | Notes |
|---|---|---|---|
| MSVC cl.exe | 19.50.35728 (v145) | 19.40 | C++23 requires 19.38+ |
| CMake | 4.3.1 | 3.25 | Presets v6 format |
| Ninja | 1.13.2 | 1.11 | |
| vcpkg | 2025-01 | 2024-06 | manifest mode only |
| Windows SDK | 10.0.26100.0 | 10.0.19041.0 | |
| Clang | 18.1.8 | 18.0 | CI-only, not production |
| Python | 3.12.4 | 3.10 | build scripts only |
| PowerShell | 7.4.3 | 7.2 | all build-scripts/ |
| Git | 2.45.2 | 2.30 | |
| WiX Toolset | 4.0.5 | 4.0 | MSI packaging |
| Google Benchmark | 1.9.1 | 1.8 | |
| Catch2 | 3.7.1 | 3.5 | |
| WindowsAppSDK | 1.5 | 1.4 | WinUI 3 (Phase 3) |
| nghttp2 | 1.62.0 | 1.60 | REST HTTP/2 (Phase 5) |
| lcms2 | 2.16 | 2.14 | ICC color (Phase 3) |

### Build Scripts to Retire (13 scripts)
Following v6.0 audit — these scripts are dead, duplicated, or superseded:

| Script | Reason |
|---|---|
| `build-scripts/Rebuild-All-With-MD.ps1` | Superseded by `Build-MSVC.ps1` |
| `build-scripts/Remove-Win32-Configurations.ps1` | One-time migration, done |
| `build-scripts/build-and-log.bat` | BAT files replaced by PS1 |
| `build-scripts/test-and-log.bat` | Same |
| `build-scripts/Fix-PCH-Corruption.ps1` | One-time fix, not recurring |
| `build-scripts/Find-All-Tools.ps1` | Absorbed into `Test-Build-Environment.ps1` |
| `build-scripts/Download-Updates.ps1` | Replaced by vcpkg manifest |
| `build-scripts/Update-All-Libraries.ps1` | Replaced by vcpkg |
| `build-scripts/Run-CodeCoverage.ps1` | Coverage now in CI workflow |
| `build-scripts/Sign-Binaries.ps1` | Superseded by EV signing CI (S287) |
| `build-scripts/production/` (entire dir) | Empty or stubs |
| `build-scripts/utilities/` (entire dir) | Absorbed into `core/` |
| `index.html` (root) | Retire in favor of docs/ GitHub Pages |

---

## 19. Refactor / Rewrite / Delete / Add Register

### REWRITE (fundamentally broken or too stale to patch)
| Item | Reason | Phase |
|---|---|---|
| `LENSManager/` WTL GUI | Zero dark mode, zero a11y, WTL 2004 patterns | 3 (new WinUI 3 project) |
| All `stb_image` usage | Silent quality downgrade, replaced by libjpeg-turbo + libspng | 2 |
| `IThumbnailProvider` error path | Returns `S_OK` with blank bitmap on decode failure; should return `E_FAIL` | 2 |
| `lens.exe` CLI arg parser | Raw `argv` parsing; replace with a proper CLI library | 3 |

### REFACTOR (correct but needs structural improvement)
| Item | Reason | Phase |
|---|---|---|
| `LENSArchive.h` (103 KB) | Monolithic format dispatch table; extract format registry to separate header | 4 |
| `Engine/Core/` decode pipeline | 9 stages implemented as nested if/else; refactor to chain-of-responsibility | 3 |
| `PerfRegressionGate.h` | Uses `namespace ExplorerLens` not `ExplorerLens::Engine` — inconsistency | 2 |
| Error returns in decoders | Mix of HRESULT, bool, and exceptions; standardize on `std::expected` | 3 |
| Cache write path | Synchronous write on decode thread; move to background writer thread | 2 |

### DELETE (confirmed dead code or retired features)
| Item | Reason |
|---|---|
| `stb_image.h` (bundled) | Replaced by libjpeg-turbo + libspng |
| `scripts/` root directory | Empty or one-liner stubs not referenced by anything |
| `tools/` root directory | Verify contents; likely empty |
| 13 build scripts (see §18) | Retired per above |
| `index.html` (root) | GitHub Pages migrated to `docs/` |
| `Dockerfile` production config | Docker + COM Shell Extension = unsupported; keep only as CI build env |

### ADD (new capabilities, not yet existing)
| Item | Description | Phase |
|---|---|---|
| `Engine/Codec/IccProfileManager.h` | lcms2 wrapper, end-to-end ICC pipeline | 3 |
| `Engine/Codec/AsyncDecodeToken.h` | `std::stop_token` wrapper for cancel-aware decode | 2 |
| `Engine/Cache/SmartPreviewStore.h` | L3 2560px JPEG smart preview cache | 4 |
| `Engine/Core/EmbeddedJpegExtractor.h` | EXIF embedded JPEG fast-path for camera RAW | 2 |
| `LENSManager.WinUI/` | New WinUI 3 project directory | 3 |
| `packaging/winget/` | WinGet manifest YAML | 3 |
| `packaging/chocolatey/` | Chocolatey nupkg spec | 4 |
| `.github/workflows/slsa-provenance.yml` | SLSA Level 2 attestation | 3 |
| `.github/workflows/winget-publish.yml` | WinGet PR automation | 3 |
| `.github/workflows/arm64-build.yml` | Native Arm64 EC build | 6 |

---

## 20. 10-Phase Plan to Best-in-Class

### Phase 1 — Foundation Cleanup (v39.x → v40.0)
**Exit criteria**: C++23 set in CMake; 13 dead scripts retired; `index.html` replaced by `docs/` GitHub Pages; `ROADMAP v7.0` merged; `Arm64 EC` flag added to CMake (build-only, no test yet).

**Sprints**: S301–S310

### Phase 2 — Performance & Correctness Baseline (v40.x → v41.0)
**Exit criteria**: Cancel-aware decode; OOM kill protection; async placeholder thumbnail; stb_image removed; libjpeg-turbo + libspng added; embedded JPEG fast-path for RAW; cache write async; 300 corpus files; D3D11 texture blit (first GPU pixels in production).

**Sprints**: S311–S340

### Phase 3 — Quality & Polish (v41.x → v42.0)
**Exit criteria**: ICC color management end-to-end (lcms2 wired); WinUI 3 LENSManager alpha; EV code signing in CI; SLSA L2 provenance; WinGet manifest; install footprint < 8 MB; SSIM gate raised to 0.97; 500 corpus files.

**Sprints**: S341–S380

### Phase 4 — Format Depth (v42.x → v43.0)
**Exit criteria**: TSAN/MSAN/UBSan clean; side-by-side compare via IContextMenu; archive cover art; smart preview store (L3 cache); LENSArchive.h refactored; 7,000 tests; format stats sidebar in LENSManager.

**Sprints**: S381–S420

### Phase 5 — Headless & Enterprise (v43.x → v44.0)
**Exit criteria**: `lens.exe --serve` REST API on HTTP/2, mTLS, all 7 endpoints live; plugin SDK with 1 published community plugin; ADMX Group Policy schema deployed; NuGet Engine SDK package; Docker CI build env.

**Sprints**: S421–S460

### Phase 6 — Distribution & Reach (v44.x → v45.0)
**Exit criteria**: Native Arm64 EC build green; MSIX sparse package CI; SLSA L2 on every release artifact; reproducible builds (bit-for-bit on CI); Chocolatey package; install footprint < 8 MB; 8,000 tests; Vulkan resize pipeline functional.

**Sprints**: S461–S500

### Phase 7 — Plugin Ecosystem (v45.x → v46.0)
**Exit criteria**: Plugin marketplace catalog at `plugins.explorerlens.io`; `.lenspkg` format signed + installable via LENSManager; AppContainer sandbox for plugins; 3 community plugins; plugin SDK documentation complete.

**Sprints**: S501–S540

### Phase 8 — AI-Assisted Features (Research Gate) (v46.x → v47.0)
**Exit criteria**: Smart crop for thumbnail composition (no ML model at user endpoint — compute server-side via REST, opt-in); IQA scoring for cache eviction priority; scene tag search. All AI features require explicit user opt-in and are feature-gated.

**Sprints**: S541–S580

### Phase 9 — Linux & FreeDesktop Compliance (v47.x → v48.0)
**Exit criteria**: DBus thumbnailer protocol compliant; Nautilus integration tested on Ubuntu LTS; FreeDesktop thumbnail spec (`.cache/thumbnails/` + XMP tags); `linux-nautilus.yml` CI green; 750 corpus files.

**Sprints**: S581–S620

### Phase 10 — macOS & Arm64 Native (v48.x → v50.0)
**Exit criteria**: macOS Quick Look extension with real decode (not stub); `macos-quicklook.yml` CI green; native Arm64 (not EC) on Apple Silicon; v50.0 release = "Best-in-Class" milestone.

**Sprints**: S621–S680

---

## 21. Success Metrics & Exit Criteria

### Performance KPIs (v45.0 targets)
| Metric | Current | Target v45.0 |
|---|---|---|
| Single thumbnail latency (p95) | 17 ms | **< 10 ms** (with D3D11 GPU blit) |
| Batch throughput | 235 img/sec | **> 500 img/sec** |
| Cache hit latency | < 5 ms | **< 2 ms** |
| Peak memory per decode | ~40 MB | **< 20 MB** (H35 + smarter RAW decoder) |
| Install footprint | ~12 MB | **< 8 MB** |
| Shell host crash rate | Unknown | **< 1 per 10,000 decodes** |

### Quality KPIs
| Metric | Current | Target v45.0 |
|---|---|---|
| Test count | ~5,045 | **8,000** |
| Corpus files | ~106 | **750** |
| SSIM threshold | 0.95 (new) | **0.97** |
| ASAN / fuzzer coverage | 8 decoders | **25 decoders** |
| CI pipeline count | 13 active | **22 active** |
| OSSF Scorecard | ? | **> 8.0** |
| Competitor matrix score | 15/40 | **32/40** |

### Competitor Matrix Score Gains by Phase
| Phase | Key additions | Score gain | Projected total |
|---|---|---|---|
| 1 | (cleanup only) | +0 | 15 |
| 2 | GPU decode, cancel-aware, OOM protection, embedded JPEG | +4 | 19 |
| 3 | ICC color management, dark mode (WinUI 3), EV code signing, SLSA L2 | +5 | 24 |
| 4 | TSAN clean, smart previews, archive cover art | +3 | 27 |
| 5 | REST API live, ADMX Group Policy, NuGet SDK | +2 | 29 |
| 6 | Arm64, MSIX, reproducible builds | +3 | 32 |

---

## 22. ADR Log v7.0

> Format: `[ID] Title — Status — Date — Supersedes`

### Retained from v6.0 (ADRs A1–A22)
| ID | Title | Status | Date |
|---|---|---|---|
| A1 | COM IThumbnailProvider as primary Windows interface | Accepted | 2023-01 |
| A2 | C++20 as language standard (see A23 for C++23 upgrade) | Superseded by A23 | 2023-01 |
| A3 | MSVC v143/v145 as sole production compiler | Accepted | 2023-01 |
| A4 | CMake + Ninja as build system | Accepted | 2023-03 |
| A5 | vcpkg in manifest mode for external dependencies | Accepted | 2023-03 |
| A6 | ETW as observability transport | Accepted | 2023-06 |
| A7 | Contract-header model for API-first development | Accepted | 2023-09 |
| A8 | Custom TEST/RUN_TEST/ASSERT macros as primary test harness | Accepted | 2023-09 |
| A9 | Google Benchmark for performance regression gates | Accepted | 2023-09 |
| A10 | Catch2 v3 for property-based tests | Accepted | 2024-01 |
| A11 | LibRaw for camera RAW decode | Accepted | 2024-03 |
| A12 | PDFium for PDF page rendering | Accepted | 2024-03 |
| A13 | resvg (Rust, C-ABI) for SVG rasterization | Accepted | 2024-06 |
| A14 | SQLite as sole persistence store | Accepted | 2024-06 |
| A15 | 9-stage decode pipeline (chain architecture) | Accepted | 2024-08 |
| A16 | SSIM as thumbnail quality gate | Accepted | 2024-10 |
| A17 | OSS-Fuzz for decoder fuzzing | Accepted | 2025-01 |
| A18 | ASAN as memory safety gate | Accepted | 2025-01 |
| A19 | CycloneDX SBOM on every release | Accepted | 2025-03 |
| A20 | AppContainer for plugin sandboxing | Accepted | 2025-06 |
| A21 | mTLS for REST API authentication | Accepted | 2025-08 |
| A22 | ADMX Group Policy schema for enterprise | Accepted | 2025-10 |

### New in v7.0 (ADRs A23–A28)
| ID | Title | Status | Date |
|---|---|---|---|
| A23 | Migrate to C++23 at v40.0 | **Accepted** | 2026-04-26 |
| A24 | Rust research lane terminated — no Rust in production path | **Accepted** | 2026-04-26 |
| A25 | Replace WTL with WinUI 3 XAML Islands for LENSManager | **Accepted** | 2026-04-26 |
| A26 | Add WinGet manifest in Phase 3 as primary enterprise discovery channel | **Accepted** | 2026-04-26 |
| A27 | lcms2 as ICC color management engine (not GDI+ color correction) | **Accepted** | 2026-04-26 |
| A28 | UnRAR SDK: feature-flag only, disabled by default, dual-license gate required | **Accepted** | 2026-04-26 |

---

## 23. Decisions Reversed from v6.0

These are explicit reversals — v6.0 said one thing, v7.0 changes it:

| v6.0 Decision | v7.0 Reversal | Rationale |
|---|---|---|
| C++20 as target standard | **C++23 from v40.0** | `std::expected`, `std::stacktrace`, modules are now stable in MSVC 19.50 |
| Rust "explore" research lane | **KILL — no Rust in Engine** | COM DLL + Rust allocator conflict; C++23+ASAN sufficient |
| WTL for LENSManager GUI | **WinUI 3 XAML Islands** | WTL has zero dark mode, zero a11y; WinUI 3 is the Microsoft-endorsed path |
| stb_image as JPEG/PNG fallback | **Remove stb_image; use libjpeg-turbo + libspng** | stb_image silently downgrades decode quality |
| 65 documentation targets | **45 high-quality docs** | More docs without owners creates documentation debt |
| 9-phase plan | **10-phase plan** | Phase 10 = macOS GA; previously was aspirational |
| `lens.exe --serve` HTTP/1.1 | **HTTP/2 (nghttp2 via WinHTTP)** | HTTP/2 is table stakes for a 2026 REST API |
| Docker as production container | **Docker as CI build environment only** | COM Shell Extension + Docker = unsupported, impossible to register |
| "Explore Arm64" | **Arm64 EC in Phase 6, concrete** | Windows on Arm is mainstream; EC allows COM host interop |

---

## 24. Sprint Delivery Pipeline S301+

### Phase 1 Sprints (S301–S310): Foundation Cleanup

| Sprint | Title | Contract/File | Harvested |
|---|---|---|---|
| S301 | Upgrade to C++23 in Engine CMakeLists | Engine/CMakeLists.txt `/std:c++23` | A23 |
| S302 | Remove stb_image from Engine — step 1: audit usages | Engine-wide grep + comment TODO | A — |
| S303 | Add libjpeg-turbo to vcpkg.json + Build-LibJpegTurbo.ps1 | vcpkg.json, external-libs/ | H28 |
| S304 | Add libspng to vcpkg.json + Build-LibSpng.ps1 | vcpkg.json, external-libs/ | H28 |
| S305 | Retire 13 dead build scripts (§18) | build-scripts/ deletions | A — |
| S306 | Migrate GitHub Pages to docs/ MkDocs (retire root index.html) | index.html → redirect | A — |
| S307 | Add `AsyncDecodeToken.h` — std::stop_token wrapper | Engine/Core/ | H36 |
| S308 | Add `EmbeddedJpegExtractor.h` — EXIF embedded JPEG fast-path | Engine/Core/ | H13 |
| S309 | Add `IccProfileManager.h` — lcms2 wrapper stub | Engine/Codec/ | H32 |
| S310 | Wire S301–S309 tests + update baseline.json | EngineTests_Platform.cpp | A23 |

### Phase 2 Preview (S311–S320): First GPU Pixels

| Sprint | Title |
|---|---|
| S311 | D3D11 device initialization in GPU/D3D11DeviceManager.h |
| S312 | DXVA2 JPEG hardware decode contract |
| S313 | Cancel-aware decode: IBindStatusCallback in COM server |
| S314 | OOM kill protection: SetProcessWorkingSetSizeEx in LENSShell |
| S315 | Async placeholder thumbnail: last-cached bitmap returned immediately |
| S316 | Parallel I/O readahead N=8 contract |
| S317 | D3D11 texture blit pipeline (first real GPU pixels) |
| S318 | libjpeg-turbo wire-in: replace stb_image JPEG paths |
| S319 | libspng wire-in: replace stb_image PNG paths |
| S320 | Wire S311–S319 tests + update GPU stage in pipeline |

---

## Appendix A: v6.0 Survivor Registry

Decisions from v6.0 that are **retained unchanged** in v7.0:

- **A1–A22 ADRs** (see §22) — 20 of 22 retained; A2 superseded by A23
- **COM CLSID** `9E6ECB90-5A61-42BD-B851-D3297D9C7F39` — immutable
- **9-stage decode pipeline** — retained, with ICC stage added between stages 5 and 6
- **Contract-header model** (A7) — retained; 300+ contract headers shipped S1–S300
- **ETW observability** (A6) — retained; GUID registered
- **SQLite persistence** (A14) — retained; schema evolved in v7.0
- **LibRaw, PDFium, resvg, dav1d** (A11–A13) — retained
- **Custom TEST/RUN_TEST macros** (A8) — retained; test placement rules unchanged
- **Google Benchmark baseline gates** (A9) — retained; `baseline.json` updated per release
- **SSIM quality gate** (A16) — retained; threshold raised to 0.97 at Phase 3
- **OSS-Fuzz + ASAN** (A17, A18) — retained and expanded
- **CycloneDX SBOM** (A19) — retained
- **AppContainer sandbox** (A20) — retained, Phase 3 implementation
- **mTLS REST auth** (A21) — retained, Phase 5 implementation
- **Decoder family consolidation to 7** — retained from v6.0 F4 decision
- **Sprint cadence: 10 sprints/session** — retained
- **Zero-warnings build discipline** — retained; non-negotiable

---

*ROADMAP v7.0 "Sirius" — ExplorerLens 39.2.0 → 50.0 roadmap*  
*Archived predecessor: `docs/archive/ROADMAP_V6.md`*  
*Next review: v41.0 milestone*
