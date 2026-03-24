# ExplorerLens — 100-Sprint Execution Plan

**Created:** 2026-03-24  
**Author:** Engineering Lead  
**Base Version:** 15.1.0 "Zenith-R"  
**Horizon Version:** 17.0.0 "Nova"  

> **Governance:** Every version bump triggers a GitHub Release with full binaries.  
> See `.github/instructions/cicd.instructions.md` for the release-on-tag procedure.

---

## Release Train Schedule

| Milestone | Version | Codename | Sprints | ETA |
|-----------|---------|----------|---------|-----|
| Architecture Hardening | **15.2.0** | Zenith-S | 1–8 | Week 2 |
| Resilience & Hardening | **15.3.0** | Zenith-T | 9–16 | Week 4 |
| CLI Tool | **15.4.0** | Zenith-U | 17–24 | Week 6 |
| Test Infrastructure | **15.5.0** | Zenith-V | 25–32 | Week 8 |
| Release Engineering | **15.6.0** | Zenith-W | 33–40 | Week 10 |
| Modern GUI (WinUI 3) | **15.7.0** | Zenith-X | 41–52 | Week 14 |
| UX Polish | **15.8.0** | Zenith-Y | 53–60 | Week 16 |
| Major Release | **16.0.0** | Horizon | 61–72 | Week 20 |
| Cloud & Auto-Update | **16.1.0** | Horizon-R | 73–80 | Week 22 |
| AI & Performance | **16.2.0** | Horizon-S | 81–88 | Week 24 |
| Enterprise & Policy | **16.3.0** | Horizon-T | 89–96 | Week 26 |
| Grand Milestone | **17.0.0** | Nova | 97–100 | Week 28 |

---

## Sprint Definitions

> Each sprint = 2–3 working days of focused work.  
> Each sprint yields: ≥1 header, ≥1 source, ≥1 test, CMakeLists.txt registration, git commit.  
> Version bump sprints additionally: update VERSION, CHANGELOG, tag, trigger GitHub Release.

---

### ── v15.2.0 "Zenith-S" — Architecture Hardening ──

#### Sprint 1 · Remove ATL from PluginHost (T3.1)
- **Goal:** Enable PluginHost compilation; replace ATL COM helpers with lightweight wrappers
- **Files:** `Engine/Plugin/PluginHost.h`, `Engine/Plugin/PluginHost.cpp`, `Engine/Plugin/ComWrappers.h`
- **Test:** `TEST(PluginHostComWrappers)` — verify IUnknown AddRef/Release counts
- **Output:** PluginHost compiles without ATL; PluginManager links successfully

#### Sprint 2 · IPropertyStore Registration (T3.2)
- **Goal:** Full `IPropertyStore` + `IPropertyStoreCapabilities` COM registration
- **Files:** `LENSShell/PropertyStoreImpl.h`, `LENSShell/PropertyStoreImpl.cpp`
- **Test:** `TEST(PropertyStorePopulation)` — dimensions, format name, color depth
- **Output:** Details pane in Explorer shows format metadata for registered types

#### Sprint 3 · ETW Observability Pipeline (T3.3)
- **Goal:** Wire ETW provider end-to-end; emit decode start/end, cache hit/miss events
- **Files:** `Engine/Core/ETWProvider.h`, `Engine/Core/ETWProvider.cpp`, `Engine/Core/ETWManifest.man`
- **Test:** `TEST(ETWEventEmission)` — start/stop provider; verify GUID registration
- **Output:** `logman query providers ExplorerLensEngine` shows manifest events

#### Sprint 4 · Data-Driven Decoder Registry (T3.4)
- **Goal:** Replace if/else chains with constexpr format table
- **Files:** `Engine/Core/DecoderRegistry.h` — extend with MIME type, magic bytes, priority
- **Test:** `TEST(DecoderRegistryLookup)` — all 200+ extensions resolve in <1µs
- **Output:** Zero if/else in format dispatch; compile-time validation of table

#### Sprint 5 · IPreviewHandler Implementation (T3.5)
- **Goal:** `IPreviewHandler` COM class for rich preview pane
- **Files:** `LENSShell/PreviewHandlerImpl.h`, `LENSShell/PreviewHandlerImpl.cpp`
- **Test:** `TEST(PreviewHandlerLifecycle)` — init/render/unload cycle
- **Output:** Preview pane renders thumbnails for top-20 formats in Explorer

#### Sprint 6 · Context Menu Verb (T3.6)
- **Goal:** "Regenerate Thumbnail" right-click action via `IContextMenu`
- **Files:** `LENSShell/ContextMenuImpl.h`, `LENSShell/ContextMenuImpl.cpp`
- **Test:** `TEST(ContextMenuVerbExecution)` — verb invocation clears cache slot
- **Output:** Right-click on any decoded file shows "Regenerate Thumbnail" option

#### Sprint 7 · IPreviewHandler Out-of-Process Testing
- **Goal:** Validate IPreviewHandler works in `prevhost.exe` (OOP COM scenario)
- **Files:** `Engine/Tests/IntegrationTests/PreviewHandlerOOP.cpp`
- **Test:** `TEST(PreviewHandlerOutOfProc)` — OOP activation via CoCreateInstance
- **Output:** Zero crashes in prevhost.exe during preview for all 20 registered formats

#### Sprint 8 · **VERSION BUMP 15.2.0** + Release
- **Goal:** Tag v15.2.0, bump VERSION/CHANGELOG, trigger GitHub Release
- **Files:** `VERSION`, `CHANGELOG.md`, `Engine/Core/BuildValidation.h`
- **Actions:**
  1. Update `VERSION` → `15.2.0`
  2. Update `CHANGELOG.md` — add `[15.2.0] "Zenith-S"` block
  3. `git tag v15.2.0 && git push origin v15.2.0`
  4. GitHub Actions `release.yml` fires → builds + uploads binaries to GitHub Releases
- **Release artifacts:** `LENSShell.dll`, `LENSManager.exe`, `ExplorerLens-15.2.0-x64.msi`, `ExplorerLens-15.2.0-x64.zip`, `SHA256SUMS.txt`

---

### ── v15.3.0 "Zenith-T" — Edge-Case Resilience ──

#### Sprint 9 · Decoder Input Validation Audit (T4.1)
- **Goal:** Audit all 25+ decoders; enforce max dimensions (32768×32768) + max file size
- **Files:** Update all `Engine/Decoders/*.cpp` to call `InputValidator::ValidateDimensions()`
- **Test:** `TEST(InputValidatorDimensionLimits)` — verify rejection of 65536×65536 inputs
- **Output:** Zero out-of-memory crashes from oversized inputs in all decoders

#### Sprint 10 · Structured Error Propagation (T4.2)
- **Goal:** Replace remaining `HRESULT` returns with `DecodeResult<T>` / `std::expected`
- **Files:** `Engine/Core/DecodeResult.h` — add `DecodeErrorCategory` enum expansion
- **Test:** `TEST(DecodeResultPropagation)` — verify error category set correctly per failure mode
- **Output:** All decoder entry points return `DecodeResult<Bitmap>`; no raw HRESULTs

#### Sprint 11 · Decoder Timeout Enforcement (T4.3)
- **Goal:** 5-second per-decoder timeout; return fallback icon on timeout
- **Files:** `Engine/Core/DecoderTimeoutGuard.h`, `Engine/Core/DecoderTimeoutGuard.cpp`
- **Test:** `TEST(DecoderTimeoutFires)` — artificial 6s sleep decoder triggers timeout
- **Output:** explorer.exe never hangs more than 5s on any pathological file

#### Sprint 12 · Crash Dump Capture (T4.5)
- **Goal:** `SetUnhandledExceptionFilter` in DLL entry; write minidumps to `%TEMP%\ExplorerLens\`
- **Files:** `LENSShell/CrashDumpCapture.h`, `LENSShell/CrashDumpCapture.cpp`
- **Test:** `TEST(MiniDumpWritten)` — trigger structured exception; verify .dmp created
- **Output:** Crash dumps appear in `%TEMP%\ExplorerLens\crashes\` on any COM exception

#### Sprint 13 · Graceful Degradation Catalog (T4.6)
- **Goal:** Enumerate + implement fallback for every failure mode
- **Files:** `Engine/Core/GracefulDegradation.h`, `Engine/Core/GracefulDegradation.cpp`
- **Test:** `TEST(GracefulDegradationFaultInjection)` — 6 failure modes each return correct fallback
- **Output:** All failure modes documented in `docs/TROUBLESHOOTING.md` with mitigation

#### Sprint 14 · Archive Decoder Hardening (T4.7)
- **Goal:** Harden ZIP/RAR/7z/TAR against zip bombs, path traversal, symlink attacks
- **Files:** Update `Engine/Decoders/ArchiveDecoder.cpp`, `Engine/Decoders/RARDecoder.cpp`
- **Test:** `TEST(ZipBombRejection)`, `TEST(PathTraversalBlocked)`, `TEST(SymlinkAttackPrevented)`
- **Output:** All 4 archive decoders pass OWASP A01/A03 input validation checklist

#### Sprint 15 · Fuzz Harness Scaffold (T4.4)
- **Goal:** LibFuzzer harness targeting each decoder entry point
- **Files:** `Engine/Tests/FuzzTargets/FuzzImageDecoder.cpp`, `FuzzArchiveDecoder.cpp`, `FuzzPDFDecoder.cpp`
- **Test:** CI nightly job `fuzz.yml` runs all harnesses for 60 seconds each
- **Output:** Zero known crashes in fuzz corpus; corpus stored in `data/corpus/fuzz/`

#### Sprint 16 · **VERSION BUMP 15.3.0** + Release
- Same release procedure as Sprint 8 → tag `v15.3.0`
- **Release artifacts:** `LENSShell.dll`, `LENSManager.exe`, `lens.exe`(stub), MSI, ZIP, SHA256SUMS

---

### ── v15.4.0 "Zenith-U" — CLI Tool ──

#### Sprint 17 · CLI Project Scaffold (T2.1)
- **Goal:** `lens.exe` console app linking `ExplorerLensEngine.lib`; subcommand routing
- **Files:** `src/cli/main.cpp`, `src/cli/CommandRouter.h`, `src/cli/CommandRouter.cpp`
- **CMake:** Add `LensCLI` target to `CMakeLists.txt`
- **Test:** `TEST(CLIRouterDispatch)` — verify all 7 subcommands dispatch correctly
- **Output:** `lens.exe --help` prints structured help text; links cleanly

#### Sprint 18 · `lens generate` Command (T2.2)
- **Goal:** Generate thumbnail(s) for files with `--size`, `--quality`, `--output`, `--recursive`
- **Files:** `src/cli/GenerateCommand.h`, `src/cli/GenerateCommand.cpp`
- **Test:** `TEST(CLIGenerateSingleFile)`, `TEST(CLIGenerateRecursive)`
- **Output:** `lens generate image.heic --output thumb.png --size 256` works end-to-end

#### Sprint 19 · `lens info` Command (T2.3)
- **Goal:** Print format detection, decoder name, metadata; `--json` output
- **Files:** `src/cli/InfoCommand.h`, `src/cli/InfoCommand.cpp`
- **Test:** `TEST(CLIInfoOutput)`, `TEST(CLIInfoJsonOutput)`
- **Output:** `lens info photo.cr3 --json` returns valid JSON with all metadata

#### Sprint 20 · `lens cache` Command (T2.4)
- **Goal:** `clear`, `stats`, `warm <directory>` subcommands
- **Files:** `src/cli/CacheCommand.h`, `src/cli/CacheCommand.cpp`
- **Test:** `TEST(CLICacheClear)`, `TEST(CLICacheStats)`, `TEST(CLICacheWarm)`
- **Output:** `lens cache stats` shows hit ratio, size, entry count from live cache

#### Sprint 21 · `lens register/unregister` (T2.5)
- **Goal:** COM registration management with admin elevation detection
- **Files:** `src/cli/RegisterCommand.h`, `src/cli/RegisterCommand.cpp`
- **Test:** `TEST(CLIRegisterDetectsAdmin)`, `TEST(CLIRegisterStatusQuery)`
- **Output:** `lens register --status` shows registration state without admin

#### Sprint 22 · `lens benchmark` + `lens doctor` (T2.6, T2.7)
- **Goal:** Benchmark (p50/p95/p99) + health check tool
- **Files:** `src/cli/BenchmarkCommand.h`, `src/cli/DoctorCommand.h` (+ .cpp)
- **Test:** `TEST(CLIBenchmarkOutputFormat)`, `TEST(CLIDoctorPassFail)`
- **Output:** `lens doctor` outputs pass/fail table for GPU, COM, decoders, cache

#### Sprint 23 · PowerShell Autocomplete + Man Page (T2.8)
- **Goal:** `lens.ps1` argument completer; comprehensive `--help` for every subcommand
- **Files:** `scripts/lens-autocomplete.ps1`; update all Command*.h with usage strings
- **Test:** `TEST(CLIHelpExitCode)` — `lens --help` exits 0; `lens unknown` exits 1
- **Output:** Tab completion works in PowerShell; exit codes match documented table

#### Sprint 24 · **VERSION BUMP 15.4.0** + Release
- Tag `v15.4.0` → publish `lens.exe` for first time as release artifact
- **Release artifacts:** `LENSShell.dll`, `LENSManager.exe`, **`lens.exe`**, MSI, ZIP, SHA256SUMS

---

### ── v15.5.0 "Zenith-V" — Test Infrastructure ──

#### Sprint 25 · Integration Test Framework (T5.1)
- **Goal:** Standalone runner decoding real corpus files; per-format pass/fail report
- **Files:** `Engine/Tests/Integration/IntegrationTestRunner.h`, `IntegrationTestRunner.cpp`
- **Test:** `TEST(IntegrationRunnerSmoke)` — 5 known-good files decode non-null
- **Output:** `EngineIntegrationTests.exe` runs against `data/corpus/` and produces HTML report

#### Sprint 26 · Corpus Expansion to 500+ Files (T5.2)
- **Goal:** Add CC0/synthetic samples for all 200+ extensions
- **Files:** `data/corpus/images/`, `data/corpus/archives/`, `data/corpus/documents/`, `data/corpus/3d/`
- **Corpus manifest:** `data/corpus/MANIFEST.json` — provenance and license for each file
- **Output:** `data/corpus/` contains ≥500 files covering valid/corrupt/edge-case variants

#### Sprint 27 · Performance Regression Gate in CI (T5.3)
- **Goal:** `performance-regression-gate.yml` runs benchmarks; blocks PR if p95 regresses >10%
- **Files:** `.github/workflows/performance-regression-gate.yml` — update with baseline comparison
- **Baseline:** Store in `Engine/Tests/benchmarks/baseline.json`
- **Output:** PRs show regression report; fails automatically on performance degradation

#### Sprint 28 · Code Coverage Measurement (T5.4)
- **Goal:** OpenCppCoverage integration; floor 60% for Engine/Core + Engine/Decoders
- **Files:** `.github/workflows/coverage.yml`; update CMakeLists.txt with `/coverage` option
- **Output:** Coverage report uploaded as CI artifact; badge in README

#### Sprint 29 · COM Integration Test (T5.5)
- **Goal:** Register DLL, simulate `IThumbnailProvider::GetThumbnail()` via COM harness
- **Files:** `Engine/Tests/Integration/COMIntegrationTest.cpp`
- **Test:** `TEST(COMThumbnailProviderRoundTrip)` — HBITMAP non-null for 10 test files
- **Output:** Post-build step validates COM round-trip for all registered formats

#### Sprint 30 · Python Test Suite Hardening (T5.6)
- **Goal:** Mock platform providers; test CLI; test Linux registration in Docker
- **Files:** `ExplorerLens.py/tests/test_platform_provider.py`, `test_cli_args.py`
- **Test:** Coverage ≥85% for `ExplorerLens.py/src/`
- **Output:** Python CI passes on Ubuntu/macOS/Windows in GitHub Actions matrix

#### Sprint 31 · Test Dashboard + Visual Regression
- **Goal:** HTML test dashboard with per-format matrix; golden-image comparison scaffold
- **Files:** `Engine/Tests/dashboard/generate_report.py`; `data/golden/` directory
- **Output:** `test-dashboard.html` artifact in CI; 5 golden-image comparisons passing

#### Sprint 32 · **VERSION BUMP 15.5.0** + Release
- Tag `v15.5.0` → coverage badge in README; test matrix in release notes
- **Release artifacts:** `LENSShell.dll`, `LENSManager.exe`, `lens.exe`, MSI, ZIP, coverage report

---

### ── v15.6.0 "Zenith-W" — Release Engineering ──

#### Sprint 33 · Upgrade CI to MSVC v145 (T6.1)
- **Goal:** Ensure CI builds use MSVC v145 matching local dev
- **Files:** `.github/workflows/build.yml`, `release.yml` — update runner + vcvars setup
- **Output:** CI `cl.exe --version` matches 19.50; zero new warnings in CI vs local

#### Sprint 34 · Code Signing Pipeline (T6.2)
- **Goal:** Authenticode signing via Azure Trusted Signing (or SignTool + PFX secret)
- **Files:** `.github/workflows/release.yml` — add signing step after build
- **Signed:** `LENSShell.dll`, `LENSManager.exe`, `lens.exe`, MSI
- **Output:** No SmartScreen warning on install; sigcheck.exe shows valid certificate chain

#### Sprint 35 · Automated Changelog Generation (T6.3)
- **Goal:** Script extracts conventional commits → CHANGELOG.md section; runs in release.yml
- **Files:** `build-scripts/utilities/Generate-Changelog.ps1`; update `release.yml`
- **Output:** Every GitHub Release auto-generates formatted release notes from commits

#### Sprint 36 · Checksums + SBOM in Release (T6.4)
- **Goal:** SHA256 for all release artifacts; CycloneDX SBOM in every release
- **Files:** Update `release.yml` — add checksum generation step; update `docs/SBOM.json`
- **Output:** `SHA256SUMS.txt` and `ExplorerLens-SBOM.json` attached to every GitHub Release

#### Sprint 37 · Release Candidate Workflow (T6.8)
- **Goal:** `pre-release.yml`: tags `vX.Y.Z-rc.N`; runs full suite; publishes GitHub pre-release
- **Files:** `.github/workflows/pre-release.yml`
- **Output:** PRs can be promoted to RC with `/rc` comment; manual promotion to stable

#### Sprint 38 · MSIX Packaging (T6.7)
- **Goal:** Complete `packaging/msix/` manifest for Microsoft Store distribution
- **Files:** `packaging/msix/ExplorerLens.msix`, update `release.yml` to build MSIX
- **Output:** `.msix` bundle built in CI; validated with `makeappx.exe` and `certutil`

#### Sprint 39 · ARM64 CI Pipeline Activation (T6.6)
- **Goal:** Enable `arm64.yml` on cross-compile or ARM64 runner; validate decoder output
- **Files:** `.github/workflows/arm64.yml` — add build + test steps
- **Output:** ARM64 builds pass in CI; ARM64 `LENSShell.dll` uploaded to Releases

#### Sprint 40 · **VERSION BUMP 15.6.0** + Release
- Tag `v15.6.0` — first signed release, MSIX included, ARM64 artifact
- **Release artifacts:** `LENSShell.dll` (x64+ARM64), `LENSManager.exe`, `lens.exe`, **signed MSI**, MSIX, SHA256SUMS, SBOM

---

### ── v15.7.0 "Zenith-X" — Modern GUI (WinUI 3) ──

#### Sprint 41 · WinUI 3 Project Scaffold (T1.1)
- **Goal:** .NET 10 + WinUI 3 project; NavigationView shell; Mica backdrop; dark/light theme
- **Files:** `Manager.WinUI/Manager.WinUI.csproj`, `MainWindow.xaml`, `App.xaml`
- **Output:** `Manager.WinUI.exe` launches with NavigationView; Mica on Windows 11

#### Sprint 42 · Format Registration Page (T1.2)
- **Goal:** TreeView of 200+ formats; checkboxes; bulk select by category
- **Files:** `Manager.WinUI/Pages/FormatsPage.xaml`, `FormatsPage.xaml.cs`
- **Output:** All 200+ formats visible; enable/disable persists to registry

#### Sprint 43 · Settings Page (T1.3)
- **Goal:** Quality, cache size, GPU selection, portable mode controls
- **Files:** `Manager.WinUI/Pages/SettingsPage.xaml`, `SettingsPage.xaml.cs`
- **Output:** Settings roundtrip correctly; JSON export/import works

#### Sprint 44 · Diagnostics Page (T1.4)
- **Goal:** Live latency histogram, cache hit ratio gauge, GPU utilization chart
- **Files:** `Manager.WinUI/Pages/DiagnosticsPage.xaml`, `DiagnosticsViewModel.cs`
- **Data:** Reads from ETW shared-memory ring buffer (Sprint 3)
- **Output:** Real-time decode latency updates every 500ms without flickering

#### Sprint 45 · COM Interop Bridge (T1.5)
- **Goal:** C++/WinRT layer to call regsvr32, query engine metrics from C#
- **Files:** `Engine/Interop/EngineInterop.h`, `Engine/Interop/EngineInterop.cpp`
- **Output:** Manager.WinUI can register/unregister shell extension, clear cache, read stats

#### Sprint 46 · Plugin Management Page
- **Goal:** Install/remove/enable/disable plugins; connect to `PluginMarketplaceV2`
- **Files:** `Manager.WinUI/Pages/PluginsPage.xaml`, `PluginsViewModel.cs`
- **Output:** Plugin list populated from local scan; install from marketplace mock

#### Sprint 47 · Real-Time Preview Pane
- **Goal:** Drag a file into Manager.WinUI → live decode → thumbnail displayed
- **Files:** `Manager.WinUI/Controls/PreviewPane.xaml`, `PreviewPaneViewModel.cs`
- **Output:** Preview renders within 20ms for JPEG; graceful fallback for unsupported types

#### Sprint 48 · Installer Integration (T1.6)
- **Goal:** WiX MSI installs Manager.WinUI; Start Menu shortcut; optional auto-launch
- **Files:** `packaging/wix/Product.wxs` — add Manager.WinUI component group
- **Output:** MSI installs both binaries; Start Menu shows "ExplorerLens Manager"

#### Sprint 49 · Accessibility Audit (T1.7)
- **Goal:** WCAG 2.1 AA: keyboard nav, screen reader labels, contrast, focus indicators
- **Files:** Update all `.xaml` files with AutomationProperties; add focus visuals
- **Output:** Accessibility Insights scan shows 0 critical / 0 serious issues

#### Sprint 50 · Localization Framework
- **Goal:** 12 languages via resource files; RTL layout support
- **Files:** `Manager.WinUI/Strings/en-US/Resources.resw` + 11 translations
- **Output:** UI renders correctly in all 12 locales; RTL Hebrew/Arabic correct

#### Sprint 51 · First-Run Wizard (T7.1)
- **Goal:** Welcome dialog + format selection wizard on first install
- **Files:** `Manager.WinUI/Wizard/FirstRunWizard.xaml`, `FirstRunWizardViewModel.cs`
- **Output:** Wizard completes → registry populated with preset → thumbnails work immediately

#### Sprint 52 · **VERSION BUMP 15.7.0** + Release
- Tag `v15.7.0` — first release with `Manager.WinUI.exe`
- **Release artifacts:** `LENSShell.dll`, `LENSManager.exe`, **`Manager.WinUI.exe`**, `lens.exe`, MSI, MSIX

---

### ── v15.8.0 "Zenith-Y" — UX Polish ──

#### Sprint 53 · System Tray Agent (T7.2)
- **Goal:** Lightweight tray agent (<5 MB); decode activity, cache stats, quick actions
- **Files:** `src/tray/TrayAgent.h`, `src/tray/TrayAgent.cpp`; or `Manager.WinUI` tray integration
- **Output:** Tray icon appears; right-click shows cache stats; "Open Manager" works

#### Sprint 54 · Toast Notifications (T7.3)
- **Goal:** Toasts for: update available, cache cleared, decode error, plugin update
- **Files:** `Engine/Core/ToastNotificationProvider.h`, `.cpp`
- **Output:** Update toast fires with "Download" action button

#### Sprint 55 · High-DPI Refinement (T7.4)
- **Goal:** Per-monitor DPI; 2× oversampling option for HiDPI
- **Files:** `LENSShell/DPIHelper.h`, `Engine/Core/DPIScalingManager.h`
- **Test:** `TEST(DPIScalingFactor)` — DPI 96/120/144/168/192 all return correct scale
- **Output:** No blurring on 4K displays; thumbnails crisp at 200% DPI

#### Sprint 56 · Error UX — User-Facing Messages (T7.6)
- **Goal:** IQueryInfo tooltip: "This file appears corrupt" on decode failure
- **Files:** Update `LENSShell/QueryInfoImpl.cpp` — map `DecodeError` → user string
- **Output:** Explorer tooltip shows meaningful text for all 6 `DecodeErrorCategory` values

#### Sprint 57 · Portable Mode Completion (T7.7)
- **Goal:** All config in `.\config\` folder; no registry writes except COM registration
- **Files:** `Engine/Core/PortableModeManager.h` — complete implementation
- **Test:** `TEST(PortableModeConfig)` — verify registry never written in portable mode
- **Output:** xcopy deployment works; `lens.exe --portable` flag activates portable mode

#### Sprint 58 · Keyboard Accessibility (T7.5)
- **Goal:** Full keyboard nav; accelerator keys; screen reader announcements
- **Files:** Update all Manager.WinUI `.xaml` with `KeyboardAccelerator` elements
- **Output:** Manager fully operable without mouse; NVDA reads all state changes

#### Sprint 59 · Theme-Aware Thumbnails + Overlay Badges
- **Goal:** Dark-themed archive/document thumbnails for dark Explorer; format icon overlay
- **Files:** `Engine/Core/ThumbnailThemeAdapter.h`, `Engine/Core/ThumbnailOverlayBadge.h`
- **Test:** `TEST(ThemeAdapterDarkMode)`, `TEST(OverlayBadgeRender)`
- **Output:** Archive thumbnails have subtle dark tint in dark mode; format badge visible

#### Sprint 60 · **VERSION BUMP 15.8.0** + Release
- Tag `v15.8.0` — full UX-polished release
- **Release artifacts:** All binaries signed, MSIX signed, `lens-autocomplete.ps1` included

---

### ── v16.0.0 "Horizon" — Major Release ──

#### Sprint 61 · Auto-Update Mechanism (T6.5)
- **Goal:** Client checks GitHub Releases API; downloads and installs MSP delta or full MSI
- **Files:** `Engine/Utils/AutoUpdateChecker.h`, `Engine/Utils/AutoUpdateChecker.cpp`
- **Output:** "Update available" notification with silent-install option

#### Sprint 62 · Winget Package Manifest
- **Goal:** `winget install ExplorerLens.ExplorerLens`
- **Files:** `packaging/winget/ExplorerLens.ExplorerLens.yaml`
- **Output:** Winget manifest validated against schema; PR submitted to winget-pkgs

#### Sprint 63 · Nightly Unstable Build Channel
- **Goal:** `nightly.yml` workflow pushes unstable builds tagged `vX.Y.Z-nightly.YYYYMMDD`
- **Files:** `.github/workflows/nightly.yml`
- **Output:** Nightly artifacts available for early testers without polluting stable channel

#### Sprint 64 · Dependabot CVE Alerts
- **Goal:** Enable Dependabot for all external libraries; automated PR on CVE detection
- **Files:** `.github/dependabot.yml` — add C++ / GitHub Actions sections
- **Output:** Dependabot PRs appear within 24h of upstream CVE publication

#### Sprint 65 · Plugin SDK v3 (Rust FFI)
- **Goal:** Extend Plugin SDK with Rust FFI support; example plugin in Rust
- **Files:** `SDK/plugin_api_v3.h`, `SDK/rust/explorerlens-plugin/`, `SDK/rust/Cargo.toml`
- **Output:** `example_plugin_rust.dll` compiles; loaded by PluginManager; decodes test format

#### Sprint 66 · Enterprise Policy Templates (GPO)
- **Goal:** ADMX/ADML templates for Group Policy; format enable/disable via GPO
- **Files:** `packaging/gpo/ExplorerLens.admx`, `packaging/gpo/en-US/ExplorerLens.adml`
- **Output:** Templates load in GPMC; format policies applied from domain controller

#### Sprint 67 · Cloud Thumbnail Provider (OneDrive)
- **Goal:** Thumbnail generation for OneDrive placeholder files without full download
- **Files:** `Engine/Decoders/CloudThumbnailProvider.h`, `.cpp`
- **Test:** `TEST(OneDrivePlaceholderDecode)` — extract thumbnail from cloud metadata
- **Output:** OneDrive `.lnk` placeholders show thumbnails without triggering full download

#### Sprint 68 · Font Decoder Enhancement
- **Goal:** Full TrueType/OpenType font preview with character grid and specimen text
- **Files:** `Engine/Decoders/FontDecoder.cpp` — expand with grid layout, kerning preview
- **Test:** `TEST(FontDecoderCharacterGrid)` — verify 96-char grid renders correctly
- **Output:** Font thumbnails show specimen "Aa" + character grid in full 256px size

#### Sprint 69 · Video Timeline Strip Enhancement
- **Goal:** Multi-frame contact strip for video thumbnails (3 keyframes at 20%/50%/80%)
- **Files:** `Engine/Decoders/VideoDecoder.cpp` — add timeline strip mode
- **Test:** `TEST(VideoTimelineStripFrameCount)` — verify 3 keyframes extracted
- **Output:** Video thumbnails show 3 horizontally-stitched keyframes

#### Sprint 70 · Scientific Format Decoders (FITS, NetCDF, HDF5)
- **Goal:** Additional scientific image format support
- **Files:** `Engine/Decoders/NetCDFDecoder.h`, `HDF5Decoder.h` (+ .cpp)
- **Test:** `TEST(NetCDFDecode)`, `TEST(HDF5Decode)`
- **Output:** `.nc` and `.h5` files show data visualization thumbnails

#### Sprint 71 · v16.0.0 Integration Testing & Hardening
- **Goal:** Full integration test run; fix all failures; update test count
- **Files:** All failing integration tests addressed; test count ≥3200
- **Output:** 100% unit + integration pass; zero regressions vs 15.8.0

#### Sprint 72 · **VERSION BUMP 16.0.0** + Release ★ MAJOR RELEASE
- Tag `v16.0.0` — full Horizon release
- **CHANGELOG:** Comprehensive `[16.0.0] "Horizon"` block with all T1-T7 work
- **Release artifacts:**
  - `LENSShell.dll` (x64 + ARM64), signed
  - `LENSManager.exe`, signed
  - `Manager.WinUI.exe`, signed
  - `lens.exe`, signed
  - `LensTrayAgent.exe`, signed
  - `ExplorerLens-16.0.0-x64.msi` (signed)
  - `ExplorerLens-16.0.0-ARM64.msi`
  - `ExplorerLens-16.0.0-x64.msix`
  - `ExplorerLens-16.0.0-x64.zip`
  - `SHA256SUMS.txt`
  - `ExplorerLens-16.0.0-SBOM.json`

---

### ── v16.1.0 "Horizon-R" — Cloud & Ecosystem ──

#### Sprint 73 · Google Drive Cloud Provider
- **Goal:** Thumbnail extraction for Google Drive mounted files
- **Files:** `Engine/Decoders/GoogleDriveThumbnailProvider.h`, `.cpp`

#### Sprint 74 · iCloud Photos Integration
- **Goal:** Extract full-res thumbnail from iCloud `.icloud` placeholder files
- **Files:** `Engine/Decoders/iCloudPlaceholderDecoder.h`, `.cpp`

#### Sprint 75 · Chocolatey Package
- **Goal:** `choco install explorerlens` via Chocolatey Community Repository
- **Files:** `packaging/chocolatey/explorerlens.nuspec`

#### Sprint 76 · WSL2 Integration
- **Goal:** Thumbnail for files accessed via `\\wsl$\` path in Explorer
- **Files:** `Engine/Core/WSLPathAdapter.h`, `Engine/Core/WSLPathAdapter.cpp`

#### Sprint 77 · Network Drive Policy Enforcement
- **Goal:** Configurable policy: skip decode / use cached / full decode for UNC paths
- **Files:** `Engine/Core/NetworkDrivePolicy.h`, `.cpp`

#### Sprint 78 · Delta Update (MSP Patch)
- **Goal:** Generate MSP patch between consecutive MSI versions
- **Files:** `packaging/wix/Patch.wxs`; update `release.yml` with MSP build step

#### Sprint 79 · Plugin Marketplace v2 Backend
- **Goal:** REST API endpoint for plugin discovery; signed plugin manifest
- **Files:** `Engine/Plugin/PluginMarketplaceV2.cpp` — complete implementation

#### Sprint 80 · **VERSION BUMP 16.1.0** + Release
- Tag `v16.1.0` + MSP patch from 16.0.0 → 16.1.0

---

### ── v16.2.0 "Horizon-S" — AI & Performance ──

#### Sprint 81 · Smart Crop v3 (Zero-Copy AI)
- **Goal:** Content-aware crop using ONNX model via DirectML; <2ms overhead
- **Files:** `Engine/AI/SmartCropV3.h`, `.cpp`

#### Sprint 82 · AI Scene Classification
- **Goal:** Classify scene type (indoor/outdoor/document/diagram) → optimize decode strategy
- **Files:** `Engine/AI/SceneClassifierV2.h`, `.cpp`

#### Sprint 83 · Image Quality Assessor v3
- **Goal:** No-reference IQA score; auto-select best frame in multi-frame formats
- **Files:** `Engine/AI/ImageQualityAssessorV3.h`, `.cpp`

#### Sprint 84 · DLSS/XeSS Upscaling for Thumbnails
- **Goal:** Use NVIDIA DLSS or Intel XeSS to upsample low-res source images
- **Files:** `Engine/GPU/DLSSUpscaleAdapter.h`, `Engine/GPU/XeSSUpscaleAdapter.h`

#### Sprint 85 · Memory Footprint Optimizer v2
- **Goal:** Reduce idle DLL footprint to <25 MB; lazy-load decoders
- **Files:** `Engine/Memory/FootprintOptimizerV2.h`, `.cpp`

#### Sprint 86 · Parallel Pipeline v3 (IOCP + Fiber)
- **Goal:** NT fiber-based cooperative pipeline for 500+ img/sec batch throughput
- **Files:** `Engine/Pipeline/FiberPipeline.h`, `Engine/Pipeline/FiberPipeline.cpp`

#### Sprint 87 · GPU Shader Optimization Pass
- **Goal:** Profile and optimize all HLSL shaders (Lanczos, Bicubic, tone-map); target <3ms/frame
- **Files:** `LENSShell/shaders/*.hlsl` — optimize occupancy, reduce register pressure

#### Sprint 88 · **VERSION BUMP 16.2.0** + Release
- Tag `v16.2.0` — AI-enhanced thumbnails; performance benchmarks in release notes

---

### ── v16.3.0 "Horizon-T" — Enterprise & Policy ──

#### Sprint 89 · Intune MDM Policy Support
- **Goal:** OMA-URI policy deployment through Microsoft Intune
- **Files:** `packaging/intune/ExplorerLens-OMA-URI.json`

#### Sprint 90 · SCCM / Configuration Manager Integration
- **Goal:** Detection method scripts + deployment package for SCCM
- **Files:** `packaging/sccm/DetectionMethod.ps1`, `packaging/sccm/InstallScript.ps1`

#### Sprint 91 · Audit Logging (Windows Event Log)
- **Goal:** Write structured events to Windows Event Log for enterprise auditing
- **Files:** `Engine/Core/AuditLogger.h`, `Engine/Core/AuditLogger.cpp`

#### Sprint 92 · Multi-Tenant Isolation
- **Goal:** Per-user cache isolation; process isolation for enterprise multi-user deployments
- **Files:** `Engine/Cache/MultiTenantCacheManager.h` — complete implementation

#### Sprint 93 · FIPS Compliance Mode
- **Goal:** Use only FIPS-140-2 approved algorithms when Windows FIPS mode is active
- **Files:** `Engine/Core/FIPSComplianceAdapter.h`, `.cpp`

#### Sprint 94 · Zero-Trust Network Policy
- **Goal:** Verify file signatures before decode if ZTA policy is active
- **Files:** `Engine/Core/ZeroTrustDecodePolicy.h`, `.cpp`

#### Sprint 95 · Enterprise Deployment Guide
- **Goal:** Complete `docs/ENTERPRISE_DEPLOYMENT.md` with GPO/Intune/SCCM walkthroughs
- **Files:** `docs/ENTERPRISE_DEPLOYMENT.md`

#### Sprint 96 · **VERSION BUMP 16.3.0** + Release
- Tag `v16.3.0` — enterprise-hardened release with ADMX, Intune, SCCM packages

---

### ── v17.0.0 "Nova" — Grand Milestone ──

#### Sprint 97 · Complete Architecture Modernization
- **Goal:** All legacy WTL code removed; full WinUI 3 + C++20 modules codebase
- **Files:** Delete `LENSManager/` legacy; replace with `Manager.WinUI/` fully
- **Output:** No WTL/ATL includes in entire codebase

#### Sprint 98 · Microsoft Store Certification
- **Goal:** Submit ExplorerLens 17.0.0 to Microsoft Store; pass all certification checks
- **Files:** `packaging/msix/` — complete Store manifest; content descriptors; screenshots
- **Output:** App published to Microsoft Store as free download

#### Sprint 99 · Developer SDK Public Release
- **Goal:** Publish Plugin SDK v3 as separate GitHub repository with documentation
- **Files:** `SDK/` publishable as `ExplorerLens-Plugin-SDK-v3.zip`
- **Output:** SDK repository at `github.com/RajwanYair/ExplorerLens-Plugin-SDK`

#### Sprint 100 · **VERSION BUMP 17.0.0** + Release ★ GRAND MILESTONE
- Tag `v17.0.0` — Nova
- **CHANGELOG:** Full `[17.0.0] "Nova"` block covering all v16.x work
- **Release artifacts:**
  - Complete signed binary set (x64 + ARM64)
  - Microsoft Store link in release notes
  - Plugin SDK zip
  - Winget + Chocolatey packages updated
  - Migration guide 16.x → 17.0.0

---

## Release Artifact Checklist (every version bump sprint)

```powershell
# Step 1: Update version files
# VERSION, CHANGELOG.md, Engine/Core/BuildValidation.h, copilot-instructions.md

# Step 2: Commit
git add -A
git commit -m "chore: bump version to X.Y.Z (Codename)"

# Step 3: Tag — this TRIGGERS GitHub Actions release.yml
git tag vX.Y.Z
git push origin main --tags

# GitHub Actions release.yml then automatically:
# → Builds: ExplorerLensEngine.lib, LENSShell.dll, LENSManager.exe, lens.exe, MSI, ZIP
# → Signs: all binaries (when signing cert configured)
# → Generates: SHA256SUMS.txt, SBOM.json
# → Creates: GitHub Release (draft) with all artifacts attached
# → Publishes: on manual approval, or auto-publishes for tagged releases
```

## Sprint Execution Rules

1. **One commit per sprint** — title: `feat(SprintN): description`
2. **Zero new warnings** — CI must show 0 warnings after every sprint
3. **Test count must not decrease** — new code always brings new tests
4. **Version bump sprints** — always update: `VERSION`, `CHANGELOG.md`, `BuildValidation.h`, `copilot-instructions.md`, SVG graphics
5. **Release artifacts** — see `release.yml` — triggered automatically on `git tag vX.Y.Z`
6. **Binaries in release** — every GitHub Release MUST include: DLL, EXE(s), MSI, ZIP, SHA256SUMS
7. **Changelog discipline** — every sprint adds a CHANGELOG subsection organized as: Added / Fixed / Changed / Performance
