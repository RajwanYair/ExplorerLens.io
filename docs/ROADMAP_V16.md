# ExplorerLens v16.0 Development Roadmap — "Horizon"

**Created:** 2026-03-24  
**Current Version:** 15.1.0 "Zenith-R"  
**Target Version:** 16.0.0 "Horizon"  
**Audience:** Engineering team, tech lead, product owner  
**Estimated Duration:** 12 sprints (~24 weeks)

---

## Executive Summary

ExplorerLens v15.x is a mature, zero-warning Windows Shell Extension with 200+ format support, GPU acceleration, multi-tier caching, 2900+ unit tests, and a cross-platform Python layer. This roadmap defines the next evolution across **7 major themes**, broken into **46 concrete tasks** organized into **12 sprints** spanning ~6 months. The goal is to transform ExplorerLens from a feature-rich decoder into a fully hardened, user-friendly, enterprise-grade product with a modern management GUI, a powerful CLI, robust error handling, and a mature release pipeline.

---

## Current State Assessment

### Strengths to Build On
| Area | Current State |
|------|---------------|
| Format coverage | 200+ extensions, 25+ native decoders, plugin SDK |
| Build quality | 0 errors, 0 warnings, MSVC v145, zero-warnings policy |
| GPU pipeline | D3D11 → D3D12 → Vulkan → GDI fallback chain |
| Caching | Sub-ms hot cache, USN invalidation, adaptive budget |
| Testing | 2938 unit tests, 100% pass rate, 5 benchmarks |
| Security | SecureAllocator, InputValidator, decoder hardening |

### Gaps to Address
| Gap | Impact | Theme |
|-----|--------|-------|
| LENSManager is WTL C++ — dated UI, hard to extend | Users can't easily configure the product | T1: GUI |
| No real CLI tool for automation/scripting | Power users and CI pipelines can't integrate | T2: CLI |
| Edge-case handling varies across decoders | Crashes or hangs on malformed files in the wild | T4: Resilience |
| Plugin host is blocked on ATL dependency | Plugin ecosystem is unusable | T3: Architecture |
| Release workflow uses windows-2022 runner, not 2026 toolset | CI doesn't match local build environment | T6: Release |
| No code signing in CI | SmartScreen blocks every install | T6: Release |
| No integration test execution in CI | Only unit tests run; no real-file decode validation | T5: Testing |
| IPropertyStore COM registration is incomplete | Details pane metadata doesn't work | T3: Architecture |
| No auto-update mechanism wired end-to-end | Users must manually download new versions | T7: UX |

---

## Theme Overview

| # | Theme | Focus | Sprints |
|---|-------|-------|---------|
| **T1** | Modern GUI (LENSManager WinUI 3) | User experience, configuration, diagnostics | S1–S4 |
| **T2** | CLI Tool (`lens` command) | Automation, scripting, power-user workflow | S3–S5 |
| **T3** | Architecture Hardening | Internal module gaps, plugin host, IPropertyStore | S2–S4 |
| **T4** | Edge-Case Resilience & Error Handling | Decoder robustness, graceful degradation, fuzz testing | S4–S6 |
| **T5** | Test Infrastructure & Coverage | Integration tests, corpus expansion, CI test gates | S5–S7 |
| **T6** | Release Engineering & CI/CD | Signing, auto-update, release automation, ARM64 CI | S7–S10 |
| **T7** | User Experience Polish | Notifications, onboarding, accessibility, telemetry | S9–S12 |

---

## Theme T1: Modern GUI — LENSManager WinUI 3 Migration

### Rationale
The current LENSManager is a WTL/ATL C++ dialog with ~20 source files. It's functional but looks dated, doesn't scale for new features, and has limited dark mode support (manually patched WM_CTLCOLOR). v15.1 began a WinUI 3 migration in C#. This theme completes it.

### Tasks

| ID | Task | Description | Estimate | Depends On |
|----|------|-------------|----------|------------|
| T1.1 | **WinUI 3 project scaffold** | .NET 10 + WinUI 3 project with NavigationView shell, Mica backdrop, dark/light theme. Build with MSBuild alongside existing vcxproj. | 3 days | — |
| T1.2 | **Format Registration page** | TreeView of all 200+ formats grouped by category (Image, Archive, Video, Audio, Document, 3D, Scientific). Checkboxes to enable/disable per-format COM registration. Bulk select/deselect per category. | 3 days | T1.1 |
| T1.3 | **Settings page** | Controls for: thumbnail quality (Fast/Balanced/HQ), cache size, GPU selection, network drive policy, portable mode toggle. Backed by registry + JSON export/import. | 2 days | T1.1 |
| T1.4 | **Diagnostics page** | Live decode latency histogram (per-format), cache hit ratio gauge, memory usage chart, GPU utilization. Data sourced from ETW provider or shared memory. | 3 days | T1.1, T3.3 |
| T1.5 | **COM Interop bridge** | C++/WinRT or P/Invoke layer to call `regsvr32`, read shell extension status, invoke cache clear, and query engine metrics from the C# GUI. | 2 days | T1.1 |
| T1.6 | **Installer integration** | WiX MSI updated to install Manager.WinUI alongside (or replacing) legacy LENSManager. Start Menu shortcut, optional auto-launch on install. | 1 day | T1.1, T6.2 |
| T1.7 | **Accessibility audit** | WCAG 2.1 AA compliance: keyboard navigation, screen reader labels, contrast ratios, focus indicators for all pages. | 2 days | T1.2, T1.3, T1.4 |

**Risks:**
- WinUI 3 requires .NET runtime (adds ~15 MB if not present). Mitigate: target .NET 10 (broadly installed) or ship self-contained.
- COM interop for admin-elevation (regsvr32) needs UAC handling. Mitigate: use a named-pipe helper elevated via ShellExecute.

**Stretch Goals:**
- Plugin management page (install/remove/enable/disable plugins from marketplace)
- Real-time thumbnail preview pane (drag a file → see live decode)
- Localized UI strings (resource files for 12 languages from v15.1 framework)

---

## Theme T2: CLI Tool — `lens` Command

### Rationale
There is no scriptable CLI for ExplorerLens. Power users, CI pipelines, and integration scenarios need a command-line tool for batch thumbnail generation, cache management, format queries, and diagnostics.

### Tasks

| ID | Task | Description | Estimate | Depends On |
|----|------|-------------|----------|------------|
| T2.1 | **CLI project scaffold** | Native C++ console app linking `ExplorerLensEngine.lib`. Subcommand routing via argv parser. Ships as `lens.exe` (~500 KB). | 2 days | — |
| T2.2 | **`lens generate` command** | Generate thumbnail(s) for one or more files. Options: `--size`, `--quality`, `--output`, `--format` (png/bmp/jpg), `--recursive`. Parallel via ThreadPool. | 3 days | T2.1 |
| T2.3 | **`lens info` command** | Print format detection, decoder name, metadata (dimensions, color space, page count) for a file. JSON output with `--json`. | 1 day | T2.1 |
| T2.4 | **`lens cache` command** | Subcommands: `clear`, `stats`, `warm <directory>`. Interacts with PersistentDiskCache. | 1 day | T2.1 |
| T2.5 | **`lens register / unregister`** | COM registration management (requires admin). Status query without admin. | 1 day | T2.1 |
| T2.6 | **`lens benchmark`** | Decode performance benchmark for a given file or directory. Reports: p50/p95/p99 latency, throughput, cache hit ratio. | 2 days | T2.1, T2.2 |
| T2.7 | **`lens doctor`** | System health check: verify COM registration, GPU availability, decoder health, library versions, cache integrity. Outputs pass/fail table. | 2 days | T2.1 |
| T2.8 | **Man page & `--help` polish** | Comprehensive `--help` for every subcommand. Consistent formatting. Exit codes documented. | 1 day | T2.2–T2.7 |

**Risks:**
- Linking ExplorerLensEngine.lib (311 MB static) makes the CLI large. Mitigate: dynamic link via DLL export layer, or accept the binary size for a standalone tool.
- Admin-elevation for `register` is awkward in CLI. Mitigate: detect and prompt, or use `gsudo`.

**Stretch Goals:**
- PowerShell completion script (`lens.ps1` argument completer)
- `lens watch <directory>` — file-system watcher that regenerates thumbnails on change
- `lens plugin install <url>` — plugin management from CLI

---

## Theme T3: Architecture Hardening

### Rationale
Several internal modules are declared but not fully wired. The plugin host is blocked, IPropertyStore registration is incomplete, and the decoder registry could benefit from the data-driven approach started in v15.1.

### Tasks

| ID | Task | Description | Estimate | Depends On |
|----|------|-------------|----------|------------|
| T3.1 | **Unblock PluginHost (remove ATL dependency)** | Replace ATL COM helpers with lightweight COM wrappers (manual `IUnknown`/`IClassFactory`). Enable PluginHost compilation. Wire into ThumbnailPipeline. | 3 days | — |
| T3.2 | **Complete IPropertyStore registration** | Implement COM registration for `IPropertyStore` + `IPropertyStoreCapabilities`. Register for target formats in WiX. Populate: dimensions, format name, color depth, page count. | 2 days | — |
| T3.3 | **Observability pipeline completion** | Wire ETW provider end-to-end: emit decode start/end, cache hit/miss, GPU dispatch, error events. Define manifest (`.man` file). Ship consumer script for WPA/PerfView. | 3 days | — |
| T3.4 | **Data-driven decoder registry** | Complete the constexpr format table from v15.1. Each entry: extension, MIME type, magic bytes, decoder class, GPU eligibility, priority. Remove all `if/else` chains in format routing. | 2 days | — |
| T3.5 | **IPreviewHandler implementation** | Full `IPreviewHandler` COM class for rich file preview in Explorer's preview pane. Reuse decoder pipeline. Register for top-20 formats. | 3 days | T3.2 |
| T3.6 | **Context menu verb** | `IContextMenu` implementation: "Regenerate Thumbnail" right-click action. Invalidates cache, re-decodes. Optional "Copy Thumbnail" to clipboard. | 2 days | — |

**Risks:**
- Removing ATL from PluginHost may break existing plugin sandbox isolation patterns. Mitigate: write COM wrappers that mirror ATL's `CComPtr`/`CComObject` semantics exactly.
- IPreviewHandler runs in `prevhost.exe` (separate process). Test for out-of-process COM scenarios.

**Stretch Goals:**
- `ITransferAdviseSink` integration for progress during batch thumbnail generation
- Decoder hot-swap (replace decoder DLL at runtime without restarting explorer.exe)

---

## Theme T4: Edge-Case Resilience & Error Handling

### Rationale
With 200+ formats and dozens of decoders, the surface area for malformed input is enormous. Current hardening covers 9 decoders. This theme extends it systematically to all decoders and builds a fuzz testing pipeline.

### Tasks

| ID | Task | Description | Estimate | Depends On |
|----|------|-------------|----------|------------|
| T4.1 | **Decoder audit — dimension/size limits** | Audit all 25+ decoder `.cpp` files. Enforce max dimensions (32768×32768), max file size (2 GB), max decompressed size. Use `InputValidator.h` consistently. | 3 days | — |
| T4.2 | **Structured error propagation** | Replace remaining HRESULT returns with `std::expected<T, DecodeError>`. Define `DecodeError` enum: `CorruptHeader`, `UnsupportedVariant`, `OutOfMemory`, `Timeout`, `IOError`, `SecurityViolation`. | 3 days | — |
| T4.3 | **Timeout enforcement per decoder** | Wrap each decoder invocation in a timeout guard (default: 5 seconds, configurable). Kill and return fallback icon on timeout. Prevents explorer.exe hangs on pathological files. | 2 days | — |
| T4.4 | **Fuzz test harness** | LibFuzzer or custom harness targeting each decoder's entry point. Corpus from `data/corpus/`. Run nightly in CI (GitHub Actions). | 3 days | T5.2 |
| T4.5 | **Crash dump capture in production** | Register `SetUnhandledExceptionFilter` in DLL entry. Write minidump to `%TEMP%\ExplorerLens\crashes\`. Report via `CrashIntelligenceEngine`. | 2 days | — |
| T4.6 | **Graceful degradation catalog** | Document and implement fallback behavior for every failure mode: decoder crash → generic icon, GPU unavailable → GDI, cache corrupt → rebuild, plugin crash → disable plugin. Tested with fault injection. | 2 days | T4.2, T4.3 |
| T4.7 | **Malformed archive hardening** | Specifically harden archive decoders (ZIP, RAR, 7z, TAR) against zip bombs, quine archives, path traversal (`../`), and symlink attacks. | 2 days | T4.1 |

**Risks:**
- Timeout enforcement in STA COM apartment is tricky (can't just kill the thread). Mitigate: use a worker thread + `WaitForSingleObject` with timeout; return E_FAIL to Explorer if exceeded.
- Fuzz testing may find real bugs. Budget time for fixing them.

**Stretch Goals:**
- Integration with Windows Error Reporting (WER) for automatic crash telemetry
- Memory sanitizer (ASan) CI build configuration
- Differential fuzzing: compare ExplorerLens decode output against reference decoders

---

## Theme T5: Test Infrastructure & Coverage

### Rationale
2938 unit tests pass at 100%, but there are no integration tests running real file decodes end-to-end, no performance regression gates enforced in CI, and the test corpus needs expansion for edge-case formats.

### Tasks

| ID | Task | Description | Estimate | Depends On |
|----|------|-------------|----------|------------|
| T5.1 | **Integration test framework** | Standalone test runner that decodes real files from `data/corpus/`, validates output (non-null, correct size, non-black). Reports per-format pass/fail. | 3 days | — |
| T5.2 | **Expand test corpus to 500+ files** | Add real-world samples for all 200+ supported extensions. Include valid files, corrupt files, zero-byte files, maximum-dimension files. Document licensing/provenance. | 3 days | — |
| T5.3 | **Performance regression gate in CI** | `performance-regression-gate.yml` runs benchmarks, compares against baseline stored in repo. Fail PR if p95 decode time regresses >10%. | 2 days | — |
| T5.4 | **Code coverage measurement** | Integrate OpenCppCoverage or MSVC `/coverage` flag. Report via CI artifact. Set floor at 60% line coverage for Engine/Core and Engine/Decoders. | 2 days | — |
| T5.5 | **COM integration test** | Register LENSShell.dll in a test harness, simulate `IThumbnailProvider::GetThumbnail()` calls via COM. Validate HBITMAP output. Run as post-build step. | 2 days | T5.1 |
| T5.6 | **Cross-platform Python test suite hardening** | Expand `ExplorerLens.py` tests: mock platform providers, test CLI argument parsing, test registration/unregistration on Linux (Docker). | 2 days | — |

**Risks:**
- Real-file corpus must not include copyrighted content. Mitigate: generate synthetic test files or use CC0-licensed samples.
- Performance regression gate is noisy on shared CI runners. Mitigate: use relative thresholds and statistical significance (3 runs, compare medians).

**Stretch Goals:**
- Mutation testing (introduce bugs, verify tests catch them)
- Visual regression testing (golden-image comparison for thumbnail output)
- Test dashboard (HTML report with per-format matrix)

---

## Theme T6: Release Engineering & CI/CD

### Rationale
The release workflow (`release.yml`) targets `windows-2022`, doesn't sign binaries, doesn't produce checksums automatically, and doesn't support differential/auto-updates. The gap between local dev build (VS 18 2026, v145 toolset) and CI is significant.

### Tasks

| ID | Task | Description | Estimate | Depends On |
|----|------|-------------|----------|------------|
| T6.1 | **Upgrade CI runner to self-hosted or windows-latest with VS 2026** | Ensure CI builds use MSVC v145 matching local dev. Configure vcvars64 in workflow. Validate zero-warnings parity. | 2 days | — |
| T6.2 | **Code signing pipeline** | Integrate Authenticode signing via Azure Trusted Signing or SignTool + PFX secret. Sign: `LENSShell.dll`, `LENSManager.exe`, `lens.exe`, MSI. Eliminates SmartScreen warnings. | 3 days | — |
| T6.3 | **Automated changelog generation** | Script that extracts commit messages since last tag, groups by conventional commit type (feat/fix/perf/docs), generates CHANGELOG.md section. Runs in release workflow. | 1 day | — |
| T6.4 | **Checksum & SBOM in release artifacts** | Auto-generate SHA256 checksums file and CycloneDX SBOM for every release. Attach to GitHub Release. | 1 day | — |
| T6.5 | **Auto-update mechanism** | Client-side update checker in LENSManager (or system tray agent). Checks GitHub Releases API for newer version. Downloads delta MSP patch or full MSI. Silent install option. | 4 days | T6.2 |
| T6.6 | **ARM64 CI pipeline activation** | Enable `arm64.yml` workflow on real ARM64 runner (or cross-compile). Validate decoder output on ARM64 Windows. | 2 days | T6.1 |
| T6.7 | **MSIX packaging** | Complete `packaging/msix/` manifest. Build `.msix` bundle for Microsoft Store distribution. Side-by-side with MSI for existing users. | 2 days | T6.2 |
| T6.8 | **Release-candidate workflow** | `pre-release.yml`: tags `vX.Y.Z-rc.N`, runs full test suite + perf benchmarks + signing, publishes as GitHub pre-release. Manual promotion to stable. | 2 days | T6.1, T6.2 |

**Risks:**
- Code signing certificate costs money and requires identity verification. Mitigate: use Azure Trusted Signing (free tier for open source) or apply for a signing certificate.
- Auto-update in a shell extension context is complex (DLL is loaded by explorer.exe). Mitigate: update on next reboot or use MsiInstallProduct with restart-manager.
- MSIX has limitations for COM shell extensions. Mitigate: use sparse package registration or desktop bridge.

**Stretch Goals:**
- Winget package manifest (`winget install ExplorerLens`)
- Chocolatey package
- Nightly unstable builds published to a separate channel
- Dependabot alerts for external library CVEs with automated PR creation

---

## Theme T7: User Experience Polish

### Rationale
Even with a modern GUI and CLI, the end-to-end user experience has friction: no first-run wizard, no system tray presence, no notification when thumbnails are regenerating, and limited accessibility.

### Tasks

| ID | Task | Description | Estimate | Depends On |
|----|------|-------------|----------|------------|
| T7.1 | **First-run experience** | On first install: welcome dialog, format selection wizard (quick preset: "All Formats", "Images Only", "Archives Only", "Custom"). Writes initial registry config. | 2 days | T1.1 |
| T7.2 | **System tray agent** | Background process (lightweight, <5 MB RAM) with tray icon. Shows: decode activity indicator, cache stats tooltip, quick actions (clear cache, open manager, check for updates). | 3 days | — |
| T7.3 | **Toast notifications** | Windows toast notifications for: update available, cache cleared, new format support added via plugin, decode error on specific file. Action buttons in toast. | 2 days | T7.2 |
| T7.4 | **High-DPI refinement** | Fix known issue with multi-monitor DPI scaling. Request thumbnails at per-monitor DPI. Provide 2× oversampling option for Retina/HiDPI displays. | 3 days | — |
| T7.5 | **Keyboard accessibility for Manager** | Full keyboard navigation: Tab order, accelerator keys, Escape to close, Enter to confirm. Screen reader announcements for all state changes. | 2 days | T1.7 |
| T7.6 | **Error UX — user-facing messages** | When a decode fails, provide a user-understandable tooltip in Explorer (via IQueryInfo). Example: "This file appears corrupt" instead of silent fallback icon. | 2 days | T4.2, T3.2 |
| T7.7 | **Portable mode completion** | Fully functional portable mode: all config in `.\config\` folder next to DLL, no registry writes except COM registration. USB-drive deployment. | 2 days | — |

**Risks:**
- System tray agent as a separate process adds installation complexity. Mitigate: make it optional, launchable from LENSManager.
- Toast notifications require UWP app identity on Windows 10. Mitigate: use `Microsoft.Toolkit.Uwp.Notifications` NuGet or raw COM toast API.

**Stretch Goals:**
- Theme-aware thumbnails (dark-themed archive/document thumbnails for dark Explorer)
- Thumbnail overlay badges (format icon in corner of thumbnail)
- Folder thumbnail aggregation (show mosaic of folder contents as folder icon)
- File preview tooltip (hover over file → see larger thumbnail in tooltip)

---

## Sprint Plan

### Sprint S1 (Weeks 1–2) — Foundation & GUI Scaffold
**Milestone:** WinUI 3 Manager shell running, COM interop functional

| Task | Theme | Priority |
|------|-------|----------|
| T1.1 | WinUI 3 project scaffold | P0 |
| T1.5 | COM Interop bridge | P0 |
| T3.4 | Data-driven decoder registry | P1 |

**Deliverables:** Manager.WinUI project compiles, NavigationView with placeholder pages, bridge can query COM registration status.

---

### Sprint S2 (Weeks 3–4) — GUI Pages & Architecture
**Milestone:** Format Registration and Settings pages functional

| Task | Theme | Priority |
|------|-------|----------|
| T1.2 | Format Registration page | P0 |
| T1.3 | Settings page | P0 |
| T3.1 | Unblock PluginHost | P1 |
| T3.2 | Complete IPropertyStore | P1 |

**Deliverables:** Users can toggle format associations from the new GUI. Plugin decoder loading works.

---

### Sprint S3 (Weeks 5–6) — CLI & Diagnostics
**Milestone:** `lens.exe` generates thumbnails, Diagnostics page shows live data

| Task | Theme | Priority |
|------|-------|----------|
| T2.1 | CLI scaffold | P0 |
| T2.2 | `lens generate` | P0 |
| T2.3 | `lens info` | P1 |
| T1.4 | Diagnostics page | P1 |
| T3.3 | Observability pipeline | P1 |

**Deliverables:** `lens generate file.cbz --output thumb.png` works. Diagnostics page displays real metrics.

---

### Sprint S4 (Weeks 7–8) — CLI Completion & Resilience Start
**Milestone:** Full CLI tool, decoder audit begun

| Task | Theme | Priority |
|------|-------|----------|
| T2.4 | `lens cache` | P0 |
| T2.5 | `lens register` | P0 |
| T2.6 | `lens benchmark` | P1 |
| T2.7 | `lens doctor` | P1 |
| T4.1 | Decoder audit — limits | P0 |
| T3.6 | Context menu verb | P2 |

**Deliverables:** `lens` CLI feature-complete. All decoders enforce dimension/size limits.

---

### Sprint S5 (Weeks 9–10) — Resilience & Error Handling
**Milestone:** Structured error handling across all decoders

| Task | Theme | Priority |
|------|-------|----------|
| T4.2 | Structured error propagation | P0 |
| T4.3 | Timeout enforcement | P0 |
| T4.7 | Malformed archive hardening | P0 |
| T2.8 | CLI help polish | P2 |
| T5.1 | Integration test framework | P1 |

**Deliverables:** Every decoder returns `std::expected`. Timeouts prevent hangs. Archives resist zip bombs.

---

### Sprint S6 (Weeks 11–12) — Fuzz Testing & Corpus
**Milestone:** Fuzz harness running, corpus at 500+ files

| Task | Theme | Priority |
|------|-------|----------|
| T4.4 | Fuzz test harness | P0 |
| T4.5 | Crash dump capture | P1 |
| T4.6 | Graceful degradation catalog | P1 |
| T5.2 | Expand test corpus | P0 |
| T3.5 | IPreviewHandler | P2 |

**Deliverables:** Nightly fuzz jobs running. Crash dumps captured. Test corpus covers all extensions.

---

### Sprint S7 (Weeks 13–14) — Test Gates & CI/CD Foundation
**Milestone:** Performance gates enforced in CI, coverage measured

| Task | Theme | Priority |
|------|-------|----------|
| T5.3 | Performance regression gate | P0 |
| T5.4 | Code coverage measurement | P1 |
| T5.5 | COM integration test | P1 |
| T6.1 | Upgrade CI runner | P0 |

**Deliverables:** PRs blocked on perf regression. Coverage floor enforced. CI matches local toolset.

---

### Sprint S8 (Weeks 15–16) — Signing & Release Pipeline
**Milestone:** Code-signed binaries, automated release

| Task | Theme | Priority |
|------|-------|----------|
| T6.2 | Code signing pipeline | P0 |
| T6.3 | Automated changelog | P1 |
| T6.4 | Checksum & SBOM | P1 |
| T6.8 | Release-candidate workflow | P0 |
| T5.6 | Python test hardening | P2 |

**Deliverables:** `v16.0.0-rc.1` published as signed GitHub pre-release with checksums and SBOM.

---

### Sprint S9 (Weeks 17–18) — Auto-Update & UX
**Milestone:** Auto-update mechanism functional, first-run experience

| Task | Theme | Priority |
|------|-------|----------|
| T6.5 | Auto-update mechanism | P0 |
| T7.1 | First-run experience | P1 |
| T7.2 | System tray agent | P1 |
| T1.6 | Installer integration | P1 |

**Deliverables:** ExplorerLens checks for updates and can self-update. New installs get a welcome wizard.

---

### Sprint S10 (Weeks 19–20) — Platform & Packaging
**Milestone:** ARM64 validated, MSIX package available

| Task | Theme | Priority |
|------|-------|----------|
| T6.6 | ARM64 CI pipeline | P1 |
| T6.7 | MSIX packaging | P1 |
| T7.3 | Toast notifications | P2 |
| T7.7 | Portable mode completion | P2 |

**Deliverables:** ARM64 builds pass CI. MSIX ready for Store submission. Toast notifications functional.

---

### Sprint S11 (Weeks 21–22) — Accessibility & DPI
**Milestone:** WCAG 2.1 AA compliance verified

| Task | Theme | Priority |
|------|-------|----------|
| T1.7 | Accessibility audit | P0 |
| T7.4 | High-DPI refinement | P1 |
| T7.5 | Keyboard accessibility | P1 |
| T7.6 | Error UX messages | P1 |

**Deliverables:** Accessibility checklist passed. Multi-DPI scenarios tested and fixed.

---

### Sprint S12 (Weeks 23–24) — Stabilization & Release
**Milestone:** v16.0.0 "Horizon" released

| Task | Description | Priority |
|------|-------------|----------|
| Bug fixes | Triage and fix all P0/P1 bugs from S1–S11 | P0 |
| Documentation refresh | Update all docs to v16.0.0 | P0 |
| Release notes | Write comprehensive release notes | P0 |
| Legacy LENSManager deprecation | Mark old WTL Manager as deprecated, keep as fallback | P1 |
| v16.0.0 tag & release | Full signed release with MSI, MSIX, portable ZIP | P0 |

**Deliverables:** `v16.0.0` tagged, signed, published. All documentation current. Zero P0 bugs.

---

## Dependency Graph

```
T3.4 (data-driven registry)
  └─→ T4.1 (decoder audit) ─→ T4.2 (std::expected) ─→ T4.6 (graceful degradation)
                                  └─→ T7.6 (error UX)

T1.1 (WinUI scaffold)
  ├─→ T1.2 (formats page) ─→ T1.7 (accessibility)
  ├─→ T1.3 (settings page) ─→ T1.7
  ├─→ T1.4 (diagnostics) ─── T3.3 (observability)
  ├─→ T1.5 (COM bridge)
  ├─→ T1.6 (installer) ──── T6.2 (signing)
  └─→ T7.1 (first-run)

T2.1 (CLI scaffold)
  ├─→ T2.2 (generate) ─→ T2.6 (benchmark)
  ├─→ T2.3 (info)
  ├─→ T2.4 (cache)
  ├─→ T2.5 (register)
  ├─→ T2.7 (doctor)
  └─→ T2.8 (help polish)

T3.1 (plugin host) ── independent
T3.2 (IPropertyStore) ─→ T3.5 (IPreviewHandler)
T3.6 (context menu) ── independent

T5.1 (integration tests) ─→ T5.5 (COM test)
T5.2 (corpus) ─→ T4.4 (fuzz testing)
T5.3 (perf gate) ── T6.1 (CI runner)

T6.1 (CI runner) ─→ T6.2 (signing) ─→ T6.5 (auto-update)
                  ─→ T6.6 (ARM64)
T6.2 ─→ T6.7 (MSIX)
T6.2 ─→ T6.8 (RC workflow)

T7.2 (tray agent) ─→ T7.3 (toasts)
T4.2 (errors) + T3.2 (property store) ─→ T7.6 (error UX)
```

---

## Risk Register

| # | Risk | Likelihood | Impact | Mitigation |
|---|------|-----------|--------|------------|
| R1 | WinUI 3 doesn't support COM shell extension admin operations natively | Medium | High | Build COM interop bridge (T1.5) as isolated helper; test early in S1 |
| R2 | Code signing certificate procurement delays | Medium | High | Start procurement in S1, don't block S1–S6 on it |
| R3 | Fuzz testing discovers critical bugs requiring extensive rework | High | Medium | Budget S12 as stabilization sprint; prioritize security fixes |
| R4 | MSIX packaging has limitations for COM extensions | Medium | Medium | Validate sparse package approach in S10; keep MSI as primary |
| R5 | Auto-update while DLL is loaded by explorer.exe | Medium | High | Use Windows Restart Manager API; install on next reboot fallback |
| R6 | Performance regression gate too noisy on shared CI runners | Medium | Low | Use median of 3 runs; set threshold at 15% rather than 10% |
| R7 | ARM64 cross-compile lacks real hardware validation | Low | Medium | Use ARM64 test machine or Windows DevKit 2025 |
| R8 | .NET 10 runtime dependency for WinUI Manager | Low | Medium | Ship self-contained or detect and prompt for install |

---

## Success Criteria for v16.0.0

| Metric | Target |
|--------|--------|
| All unit tests pass | 100% (3000+ expected) |
| Integration test coverage | 200+ real-file decodes, 100% pass |
| Code coverage (Engine/Core + Decoders) | ≥ 60% line coverage |
| Performance regression vs v15.1 | ≤ 5% slower (p95) |
| Build warnings | 0 |
| Code signed binaries | All shipping binaries |
| WCAG 2.1 AA compliance | Verified (Manager.WinUI) |
| Fuzz testing corpus | 500+ files, 0 unresolved crashes |
| CLI commands | 7 subcommands, all with `--help` |
| Packaging formats | MSI + MSIX + Portable ZIP |
| CI/CD pipelines | Build + Test + Perf gate + Sign + Release |

---

## Appendix A: Task Summary by Priority

### P0 — Must Have (24 tasks)
T1.1, T1.2, T1.3, T1.5, T2.1, T2.2, T2.4, T2.5, T4.1, T4.2, T4.3, T4.4, T4.7, T5.1, T5.2, T5.3, T6.1, T6.2, T6.5, T6.8, T1.7, S12 (bugs + docs + release)

### P1 — Should Have (16 tasks)
T1.4, T2.3, T2.6, T2.7, T3.1, T3.2, T3.3, T3.4, T4.5, T4.6, T5.4, T5.5, T6.3, T6.4, T6.6, T6.7, T7.1, T7.2, T7.4, T7.5, T7.6, T7.7

### P2 — Nice to Have (6 tasks)
T2.8, T3.5, T3.6, T5.6, T7.3, T1.6

---

## Appendix B: Glossary

| Term | Definition |
|------|-----------|
| STA | Single-Threaded Apartment (COM threading model used by Explorer) |
| WiX | Windows Installer XML — MSI authoring toolset |
| MSIX | Modern Windows packaging format (Store-compatible) |
| USN | Update Sequence Number journal (NTFS change tracking) |
| IPropertyStore | COM interface for file metadata in Explorer's Details pane |
| IPreviewHandler | COM interface for file preview in Explorer's Preview pane |
| ETW | Event Tracing for Windows — structured event logging |
| PSO | Pipeline State Object (GPU shader+state bundle cached to disk) |
| WPA | Windows Performance Analyzer — ETW trace viewer |
| STRIDE | Threat modeling framework (Spoofing, Tampering, Repudiation, Info Disclosure, DoS, Elevation) |
