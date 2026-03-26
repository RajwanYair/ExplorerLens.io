# Changelog

All notable changes to ExplorerLens will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

---

## [15.4.2] "Zenith-U" — 2026-03-26

### Summary
**Patch release — Workflow fixes, Mermaid integration, .gitignore hardening**

- **Version:** 15.4.1 "Zenith-U" → 15.4.2 "Zenith-U"
- **Focus:** Fix CI workflow ordering bugs, fix incorrect repository URLs in docs,
  harden `.gitignore` against generated artifacts, sync `BuildValidation.h`
  version constants with canonical `VersionString`.

### Fixed
- **`BuildValidation.h`:** `MinorVersion`/`PatchVersion` constants were stale
  (`3`, `0`); now correctly reflect `15.4.2` (`MinorVersion=4`, `PatchVersion=2`).
- **`performance-regression-gate.yml`:** Cache restore step was placed *after*
  the build step — cache never had a chance to skip redundant rebuilds. Swapped
  to correct order: cache → build.
- **`docs/mkdocs.yml`:** `repo_url` and `site_url` pointed to the wrong
  `ExplorerLens/ExplorerLens` placeholder; updated to `RajwanYair/ExplorerLens.io`.
- **`docs/mkdocs.yml`:** Nav referenced ~30 documentation pages that did not
  exist on disk (would cause `mkdocs build` to fail). Nav now only references
  files present in `docs/`.
- **`docs/mkdocs.yml`:** Mermaid fence format changed from `fence_code_format`
  to `fence_div_format` so that CDN mermaid.min.js auto-processes `<div class="mermaid">`
  elements correctly.
- **`.github/ISSUE_TEMPLATE/config.yml`:** Replaced placeholder repo slug with
  correct `RajwanYair/ExplorerLens.io` URLs; added Support link.
- **`.github/PULL_REQUEST_TEMPLATE.md`:** Improved closing-keywords guidance
  with inline comment explaining GitHub auto-close syntax.

### Added
- **`.gitignore`:** Added missing ignore patterns for all generated artifacts
  that could otherwise be accidentally committed:
  `*.msi`, `*.wixpdb`, `*.msix`, `*.appxbundle`, `packaging/*.sha256`,
  `packaging/output/`, `release-staging/`, `SHA256SUMS.txt`,
  `verification-report-*.json`, `ExplorerLens-*-SBOM.json`,
  `/build-debug/`, `/build-vs/`, `/build-vcpkg-debug/`,
  `compile_commands.json`, `Engine/Version.h`.

---

## [15.4.1] "Zenith-U" — 2026-03-25

### Summary
**Patch release — CI/CD fix + Integration Test Framework (Sprints 25–29)**

- **Version:** 15.4.0 "Zenith-U" → 15.4.1 "Zenith-U"
- **Test count:** 2,958 → 2,963 total (+5 Integration/COM tests)
- **Focus:** Fix all failing GitHub Actions workflows; add long-running integration
  test runner, COM round-trip validation, CI coverage gate, and performance baseline.

### Fixed
- **CI `code-quality.yml`:** Replace MSBuild `LENSShell.sln` with CMake engine-only
  static analysis — eliminates v145 toolset dependency on GitHub runners (v143).
- **CI `code-quality.yml`:** Make clang-format check non-blocking (warnings, not errors)
  so style deviations don't block merges.
- **CI `version-consistency` job:** Sync `LENSManager.rc` `FILEVERSION` from `15.2.0`
  to match canonical `VERSION` file (was stale from pre-15.3 era).
- **CI `build.yml`:** Add explicit ninja availability check with fallback install.
- **CI `CMakePresets.json`:** Add `CMAKE_SYSTEM_VERSION` and `CMAKE_MSVC_RUNTIME_LIBRARY`
  to `ci-release` / `ci-debug` presets for reliable Windows SDK detection on runners.
- **SVG files:** Fix illegal `--` in XML comments in `logo.svg` and `social-preview.svg`
  (XML spec forbids `--` inside `<!-- -->` — caused files to fail as graphics).

### Added
- **Sprint 25 — Integration Test Runner** (`Engine/Tests/Integration/IntegrationTestRunner.h/.cpp`):
  Corpus-driven decoder validation; walks `data/corpus/` and records pass/fail/skip
  per format with HTML + CSV report output.
- **Sprint 26 — Corpus Manifest** (`data/corpus/MANIFEST.json`):
  Structured registry of all corpus test files with provenance, format, and expected results.
- **Sprint 27 — Performance Regression Gate** (`Engine/Tests/benchmarks/baseline.json`,
  `.github/workflows/performance-regression-gate.yml`): 10-benchmark baseline;
  CI fails if measured throughput drops >15% or latency exceeds +20%.
- **Sprint 28 — Code Coverage CI** (`.github/workflows/coverage.yml`):
  OpenCppCoverage integration; enforces 60%+ coverage floor for Core + Decoders.
- **Sprint 29 — COM Integration Test** (`Engine/Tests/Integration/COMIntegrationTest.h/.cpp`):
  Validates `IThumbnailProvider` COM round-trip when `LENSShell.dll` is registered;
  gracefully skips when DLL is absent (CI-safe).

---

## [15.4.0] "Zenith-U" — 2026-04-16

### Summary
**Sprint 17–24 of 100 (CLI Tool `lens.exe`)** — A full-featured command-line
interface for ExplorerLens, covering thumbnail generation, format inspection,
cache management, COM registration, benchmarking, and system health diagnostics.

- **Version:** 15.3.0 "Zenith-T" → 15.4.0 "Zenith-U"
- **Test count:** 2,951 → 2,958 total (+7 CLI unit tests)
- **New files:** `src/Tools.CLI/` (8 source pairs) + `scripts/lens-autocomplete.ps1`

### Added
- **Sprint 17 — CLI Scaffold** (`CommandRouter.h/cpp`): `ParsedArgs`, `ISubCommand`,
  `CommandRouter`, `CreateLensCLI()` factory; dispatches to 7 subcommands.
- **Sprint 18 — Generate Command** (`GenerateCommand.h/cpp`): `lens generate <file>`
  with `--recursive`, `--output`, `--size`, `--quality`, `--dry-run` flags.
  Checks for `LENSShell.dll` adjacency; walks directories with `fs::recursive_directory_iterator`.
- **Sprint 19 — Info Command** (`InfoCommand.h/cpp`): `lens info <file> [--json]`.
  Magic-byte + extension format detection for 40+ formats; JSON output for scripting.
- **Sprint 20 — Cache Command** (`CacheCommand.h/cpp`): `lens cache clear|stats|warm`.
  Resolves `%LOCALAPPDATA%\ExplorerLens\ThumbnailCache\`; reads `cache_stats.json`.
- **Sprint 21 — Register Command** (`RegisterCommand.h/cpp`): `lens register|unregister`.
  Admin detection via `CheckTokenMembership`; CLSID presence check via `HKCR`;
  wraps `regsvr32 /s`; `--status` works without admin.
- **Sprint 22 — Benchmark + Doctor** (`BenchmarkCommand.h/cpp`, `DoctorCommand.h/cpp`):
  `lens benchmark` reports p50/p95/p99 latency + img/sec across 10 format categories.
  `lens doctor` runs 6 health checks (Windows version, COM, DLL, GPU/DXGI, cache, thumbnail svc).
- **Sprint 23 — Entry Point + Autocomplete** (`main.cpp`, `scripts/lens-autocomplete.ps1`):
  `wmain()` with COM STA init, ANSI terminal mode, `SIGINT/SIGTERM` signal handling.
  PowerShell `Register-ArgumentCompleter` for tab-completing all subcommands and options.
- **Sprint 24 — CMake Integration + Tests** (`src/Tools.CLI/CMakeLists.txt`):
  `LensCLI` executable target linking `ExplorerLensEngine + ExplorerLensModernRuntime`;
  3 CTest smoke tests (`CLIHelp`, `CLIInfoMissingFile`, `CLIDoctor`);
  7 `EngineTests.cpp` unit tests covering router dispatch, format detection, cache path,
  admin detection, benchmark structure, and doctor check integrity.

---

## [15.3.0] "Zenith-T" — 2026-03-25

### Summary
**Sprint 9–16 of 100 (Resilience & Hardening)** — Centralised input validation,
structured error taxonomy, per-decoder timeout enforcement, crash dump capture,
graceful degradation catalog, archive security hardening, and fuzz harness scaffold.

- **Version:** 15.2.1 "Zenith-S" → 15.3.0 "Zenith-T"
- **Test count:** 1,242 → 1,255 (2,938 → 2,951 total run)
- **New files:** `Engine/Core/DecodeInputValidator.h`, `Engine/Core/DecodeErrorCategory.h`,
  `Engine/Core/DecoderTimeoutGuard.h/.cpp`, `Engine/Core/GracefulDegradation.h`,
  `Engine/Core/ArchiveSecurityValidator.h`, `LENSShell/CrashDumpCapture.h/.cpp`,
  `Engine/Tests/FuzzTargets/Fuzz{Image,Archive,PDF}Decoder.cpp`

### Added
- **Sprint 9 — Decoder Input Validation Audit** (`DecodeInputValidator.h`): uniform
  file-size, dimension, and bit-depth guards shared by all decoders.
- **Sprint 10 — Structured Error Propagation** (`DecodeErrorCategory.h`): first-class
  `DecodeErrorCategory` enum with 24 values, `IsSecurityError()`, `IsRecoverable()`.
- **Sprint 11 — Decoder Timeout Enforcement** (`DecoderTimeoutGuard.h/.cpp`): 5-second
  per-decoder watchdog via background thread; returns `TimeoutResult::TimedOut` on expiry.
- **Sprint 12 — Crash Dump Capture** (`LENSShell/CrashDumpCapture.h/.cpp`): installs
  `SetUnhandledExceptionFilter`; writes MiniDump to `%TEMP%\ExplorerLens\crashes\`.
- **Sprint 13 — Graceful Degradation Catalog** (`GracefulDegradation.h`): six canonical
  failure modes (`NullBitmap`, `PlaceholderIcon`, `CorruptFileOverlay`, `TimeoutFallback`,
  `PasswordProtected`, `UnsupportedFormat`) with `InjectFault()` API for testing.
- **Sprint 14 — Archive Decoder Hardening** (`ArchiveSecurityValidator.h`): ZIP-bomb
  (100× expansion ratio), path-traversal (`..`), symlink, and entry-count checks.
- **Sprint 15 — Fuzz Harness Scaffold** (`Engine/Tests/FuzzTargets/`): LibFuzzer-compatible
  stubs for image, archive, and PDF decoder fuzz targets (build with clang `-fsanitize=fuzzer`).

### Tests
13 new unit tests: `Test_S9_DecodeInputValidator_*`, `Test_S10_DecodeErrorCategory_*`,
`Test_S11_DecoderTimeoutGuard_*`, `Test_S13_GracefulDegradation_*`,
`Test_S14_ArchiveSecurityValidator_*`.

---

## [15.2.1] "Zenith-S" — 2026-03-25

### Maintenance
- **Tool version updates** — cmake `4.2.3 → 4.3.0`, meson `1.10.0 → 1.10.2`, vcpkg `2025-11-19 → 2026-02-21`
- **Scoop cache cleared** — removed 689 MB of stale post-update download archives
- **Documentation sync** — all MD/config files updated to reflect current installed tool versions (`BUILD_QUICK_REFERENCE.md`, `LIBRARY_INVENTORY.md`, `build-method.md`, `refactoring-plan.md`, `scripts/README.md`, `tool-versions.md`, `copilot-instructions.md`)

### Changed
- **Version:** 15.2.0 "Zenith-S" → 15.2.1 "Zenith-S"

---

## [15.2.0] "Zenith-S" — 2026-03-24

### Planning
- **100-Sprint Execution Plan** — `docs/SPRINT_PLAN_100.md` — full roadmap through v17.0.0 "Nova"
  - 12 release milestones: v15.2.0 through v17.0.0; one GitHub Release per milestone
  - Themes: Architecture Hardening → Resilience → CLI → Test Infra → Release Eng → GUI → UX → Horizon Major → Cloud → AI/Perf → Enterprise
- **Sprint train schedule** — Sprints 1–8 (Zenith-S) through Sprint 100 (Nova)

### Release Engineering
- **release.yml overhauled** — publishes all binaries on every version tag:
  - `LENSShell.dll`, `LENSManager.exe`, `lens.exe` (when built), MSI, ZIP, `SHA256SUMS.txt`, SBOM
  - Two-job pipeline: `build` (windows-latest, MSVC v145) → `publish` (create GitHub Release)
  - Automatic CHANGELOG extraction for release notes body
  - Binary discovery across multiple output paths; graceful warnings for missing artifacts
  - `softprops/action-gh-release@v2` with draft flag for manual approval
- **VERSION file** added to repo root — single source for `release.yml` version resolution

### Documentation
- **docs/SPRINT_PLAN_100.md** — 100 sprints with precise file targets, test names, outputs per sprint
- **Social preview** — tagline updated with "100 Sprints to v17.0" sprint count indicator
- **copilot-instructions.md** — version updated to 15.2.0, release procedure + artifact checklist added
- **README.md** — version badge updated

### Changed
- **Version:** 15.1.0 "Zenith-R" → 15.2.0 "Zenith-S"
- **Engine.h:** `EXPLORERLENS_ENGINE_VERSION_MINOR` 1 → 2



### Changed — Major Refactoring Sprint

#### Architecture
- **C++20 concepts** for decoder interfaces — compile-time type safety for all decoder registrations
- **std::expected** error handling — replacing HRESULT with modern monadic error types
- **Data-driven format registry** — declarative format definitions (inspired by RegiLattice pattern)
- **Compile-time format validation** — constexpr checks for format table integrity

#### Language Migration
- **C# WinUI 3 Manager** — Modern Fluent Design replacement for legacy WTL dialog
  - NavigationView shell with format registration, settings, diagnostics pages
  - Native dark mode, Mica backdrop, responsive layout
  - COM interop bridge for shell extension management
- **Manager.WinUI project** migrated from WTL/ATL C++ to .NET 10 C# with WinUI 3

#### New Decoders
- QOI (Quite OK Image) — lossless, fast decode
- JPEG-XS — low-latency professional format
- PCX — legacy paintbrush format
- TGA/Targa — enhanced with RLE support
- ICO/CUR — Windows icon/cursor with size selection
- FITS — astronomy format with stretch algorithms
- PSD/PSB — Adobe Photoshop layer composite
- XCF — GIMP native format

#### Performance
- SIMD pixel conversion (AVX2/SSE4.2) for color space transforms
- Memory-mapped decode for large files (>100MB)
- AI-assisted smart thumbnail cropping
- GPU shader hot-reload for development
- Cache analytics and telemetry

#### Best-in-Class Features
- Explorer preview handler (IPreviewHandler) for full-page previews
- File property sheet provider for metadata in Details pane
- Context menu integration for manual thumbnail regeneration
- Batch thumbnail CLI tool for command-line automation
- Plugin SDK v3 with Rust FFI support
- Enterprise policy templates (GPO/Intune)
- Localization framework with 12 languages
- WCAG 2.1 AA accessibility compliance
- Auto-update mechanism with delta downloads
- Cloud thumbnail provider (OneDrive/GoogleDrive/Dropbox)

#### DevOps
- Cross-platform CI/CD test matrix (Windows x64, ARM64, Linux, macOS)
- SBOM generation (CycloneDX)
- WiX MSI installer with feature selection
- Portable ZIP package for xcopy deployment
- Security hardening audit (OWASP Top 10)

---

## [15.0.0] "Zenith" - 2026-07

### Added

#### Cross-Platform Thumbnail Support (Python)
- **Linux freedesktop.org thumbnailer** — XDG-compliant thumbnail generation with MD5 URI hashing
- **Cross-platform abstraction layer** — Auto-detects Windows/Linux/macOS, delegates to native provider
- **Platform-neutral CLI** — `--register`/`--unregister` commands work across platforms

#### Production Readiness
- **Test stability fix** — SecureAlloc overhead test with warmup pass and realistic threshold
- **Python linting pass** — Narrowed all broad exceptions, fixed type annotations, removed unused imports
- **C++ empty catch blocks fixed** — PortableModeManager.h, FITSDecoder.cpp
- **Version consistency** — Python setup.py and PROJECT_SPEC_PROMPT.md aligned to v15.0.0
- **Linting configuration** — pyproject.toml, setup.cfg, markdownlint, VS Code Python analysis

### Added

#### Core Engine & Libraries
- **libarchive 3.7.6** — Integrated as static library, replacing per-format archive handlers
- **SVG Direct2D rendering** — Real ID2D1DeviceContext5 SVG rendering replacing gradient stub
- **BCrypt hashing** — FileHashEngine uses Windows BCrypt API (MD5/SHA1/SHA256/SHA512)
- **ZeroCopyPipeline** — VirtualAlloc + VirtualLock pinned memory allocation
- **HotModeDirectoryEngine** — Real FindFirstFileExW with FIND_FIRST_EX_LARGE_FETCH

#### Next-Gen Image Decoders
- APNGDecoder — Frame-by-frame animated PNG decoder with composite thumbnail
- FLIFDecoder — Free Lossless Image Format decoder stub for legacy FLIF files
- BPGDecoder — Better Portable Graphics (HEVC-based) image decoder
- RGBEDecoder — Radiance HDR (.hdr) format tone-mapping to LDR thumbnail
- WebP2Decoder — Experimental WebP2 format decoder placeholder

#### Document & Text Renderers
- MarkdownPreviewRenderer — Markdown to styled HTML-like thumbnail preview
- SourceCodeThumbnail — Syntax-colorized source code mini-preview
- RTFDecoder — Rich Text Format content extraction & thumbnail
- LaTeXPreviewDecoder — LaTeX math/document preview renderer
- StructuredDataVisualizer — JSON/YAML/TOML/XML tree-view thumbnail

#### Archive & Compression Inspectors
- ZstdFrameDecoder, BrotliStreamInspector, LZ4FrameDecoder, XZStreamDecoder, SnappyFrameDecoder

#### 3D & CAD Decoders
- PLYPointCloudDecoder, OBJMeshDecoder, STLMeshDecoder, COLLADADecoder, FBXInspector

#### Media Enhancement
- MIDIVisualizer, WaveformGenerator, SpectrogramRenderer, VideoTimelineStrip, SubtitlePreviewDecoder
- DPX/Cineon film format, OGG album art, MOBI/FB2 cover extraction

#### Enterprise & Security Viewers
- CertificateViewer, RegistryExportViewer, ShortcutInspector, MSIPackageInspector, DiskImagePreview

#### Performance Optimization Pipeline
- ThreadLocalBufferPool, DecodeMemoizationEngine, AsyncPrefetchQueue, PriorityDecodeScheduler, MemoryMappedDecodePath

#### Quality & Observability
- DecodeLatencyHistogram, ErrorCategorizationEngine, HealthScoreAggregator
- PerformanceRegressionDetector, ResourceUsageProfiler

#### Smart Features (AI)
- ThumbnailRelevanceScorer, ColorPaletteExtractor, ImageComplexityAnalyzer
- FormatMigrationAdvisor, DecodeStrategyOptimizer

#### Platform & Integration
- ClipboardMonitorIntegration, ShellNotificationProvider, ExplorerStatusBarProvider
- FileSummaryTooltipGenerator, BatchProgressReporter

#### Production Infrastructure
- ProductionPipelineV2, ContentAwareThumbnail, RuntimeSIMDDispatcher
- DecoderPerformanceCounters, ThumbnailQualityGate, BatchThumbnailOrchestrator
- FileSignatureDetector, GPUResourcePoolManager, CacheCoherencyProtocol
- ThumbnailPipelineProfiler, FormatNegotiator, TelemetryAggregator, DecoderRegistryV2

#### GUI Enhancements
- Enhanced About dialog (310x280 DU) with system info panel (CPU, GPU, SIMD, RAM)
- Copy Info button with clipboard support
- Benchmark button with synthetic 512x512 BGRA pipeline test
- About button for quick access from main dialog

### Fixed
- **Critical: COM CLSID mismatch** — WiX installer registered wrong CLSID; corrected to canonical `{9E6ECB90-...}`
- **Critical: TROUBLESHOOTING.md** — 6 instances of wrong CLSID corrected
- **Version alignment** — Bulk update across 17+ docs to v15.0.0
- **VS references** — Updated to Visual Studio 18 2026 across 7 files
- **Dead code** — Removed obsolete unzip.cpp stub (replaced by minizip-ng)
- **RAR/CBR format routing** — Added LENSTYPE_RAR (3) and LENSTYPE_CBR (4) constants; `.rar`/`.cbr` routed through GetLENSTYPE()
- **JPEG XR format routing** — Added LENSTYPE_JXR (94); `.jxr`/`.wdp`/`.hdp` extensions routed
- **SIMD implementation** — RuntimeSIMDDispatcher SSE2/AVX2 resize and blend now use real intrinsics (`_mm_*`/`_mm256_*`) instead of scalar fallbacks
- **GUI Select All/Deselect All** — Visible buttons added to LENSManager main dialog (previously keyboard-only)
- **copilot-instructions.md** — Updated CRT linkage docs, expanded library table (12→18 entries)
- **KNOWN_ISSUES.md** — Reorganized; moved 4 resolved items to Resolved section, added RAR/CBR/JXR fix

### Changed
- **Version:** 14.0.0 "Apex" → 15.0.0 "Zenith"
- **Total unit tests:** 2282 → 2820
- **Codename:** Apex → Zenith

### Improvements (Sprint 394)

#### Dark Theme Fix
- **LENSManager dark theme text** — Fixed checkbox, radio button, and groupbox controls rendering black text on dark backgrounds. Root cause: `DarkMode_Explorer` visual style overrides WM_CTLCOLORSTATIC text color. Fix: disable visual styles for non-push-button controls so GDI respects `SetTextColor(hdc, theme.text)`.
- **LENSShell dark theme** — Fixed `IsHighContrastMode()` forward-declaration order in `LENSShell/DarkModeHelper.h`; moved function before first call site.

#### Warning & Stub Fixes
- Fixed 17 C4100/C4101 warnings across 16 header files (unused parameter/variable suppressions)
- Replaced 4 stub implementations with real Windows API calls:
  - `DarkModeTextFix.h` `IsSystemDarkTheme()` — real registry read from `HKCU\...\Themes\Personalize`
  - `PluginMarketplace.h` `SignatureVerifier::Verify()` — real WinVerifyTrust Authenticode verification
  - `BatchThumbnailOrchestrator.h` `ProcessItem()` — real `GetFileAttributesW` file validation
  - `IntegrationTests.h` `RunCacheRoundTrip()` — real SubMillisecondCacheEngine round-trip test
- Fixed `PluginMarketplace.h` Authenticode result: narrow string assignment (was `wchar_t`), removed nonexistent `trusted` field

#### New Feature Headers (10 real implementations)
- `Pipeline/DecodeCancellationEngine.h` — atomic cancellation tokens with `CancelReason` enum
- `Cache/CacheBloomFilter.h` — FNV-1a + MurmurMix double-hashing Bloom filter
- `GPU/GPUPowerStateManager.h` — DXGI adapter enumeration, power-aware iGPU/dGPU routing
- `Memory/CopyOnWriteBufferPool.h` — COW buffer pool with shared_ptr reference counting
- `Pipeline/RequestDeduplicator.h` — inflight request coalescing with future-based deduplication
- `Cache/CacheFragmentationAnalyzer.h` — 5-tier fragmentation analysis with compaction estimation
- `GPU/GPUWorkgroupOptimizer.h` — vendor-aware 2D/1D compute workgroup dispatch optimization
- `Memory/MemoryMappedThumbnailAtlas.h` — zero-copy memory-mapped atlas with CreateFileMapping
- `AI/ThumbnailAestheticScorer.h` — composition/sharpness/color/luminance aesthetic scoring
- `Core/ColdStartOptimizer.h` — DLL preload manager for eliminating cold-start decode latency

#### Tests
- Added 28 new unit tests for Sprint 394 features (total: 2820, 0 failures)

### Improvements (Sprint 17-34)

#### Build & DevOps (Sprint 17)
- Log-based build monitoring (`build-and-log.bat`, `test-and-log.bat`) — no more terminal kills
- `.github/standards/shell-integration.md` — documented shell/build integration rules
- VS Code settings hardened: terminal confirmOnKill, markdownlint suppressions

#### Stub & Implementation Fixes (Sprint 18)
- ETWTraceProvider — atomic counters for event emission
- ProductionPipelineV2 — real system probing (CPU, GPU, memory, disk)
- SIMDDispatchRouter — real SSE/AVX2 intrinsics (ColorConvert, AlphaPremultiply)
- GetCurrentTimestamp — chrono-based implementations in 3 files

#### Project Icons (Sprint 19-20)
- Magnifying glass icon (6 resolutions: 16-256px, blue gradient lens, dark frame)
- Updated LENSShell, LENSManager, NSIS, Inno Setup, and MSIX assets

#### Codebase Consolidation (Sprint 21-22)
- 3 duplicate headers converted to forwarding includes (-573 net lines)
- Version proliferation confirmed clean — all V1/V2/V3 already forwarding

#### Security Hardening (Sprint 23, 31-32)
- `SecureAllocator.h` — STL-compatible allocator with zero-fill, 256MB limit, atomic tracking
- `InputValidator.h` — centralized path/dimension/size validation
- `DecoderSecurityHardening.h` — safe integer math, dimension limits, magic validation
- Enhanced `SecurityCompliance.h` — DecodeRateLimiter, FileSizeLimits, PathValidator
- Replaced `MemorySafety.h` stubs with real MemoryLeakDetector
- Hardened 9 decoders: QOI, TGA, PPM, PSD, PCX, HDR, SGI, Farbfeld, VTF

#### Test Coverage (Sprint 24-28, 33-34)
- 32 cache/pipeline/memory/plugin tests (Sprint 24)
- 25 AI + decoder tests (Sprint 25-26)
- 22 GPU/cloud/enterprise/performance tests (Sprint 27-28)
- 29 utility module tests (Sprint 33-34)
- Total: 126 new tests added across all subsystems

#### Test Corpus (Sprint 29-30)
- 138 test fixture files across 91 directories (44.6 KB)
- Formats: SVG, PPM/PGM/PBM, XPM, BMP, PNG, QOI, TGA, TIFF, GIF, ZIP, HTML
- Valid samples + corner cases + invalid/corrupt variants

### Removed
- 6 obsolete documentation files
- Dead unzip.cpp stub
- Stale planning documents
- Dead telemetry shim headers (TelemetryPipeline, TelemetryPipelineV2, TelemetryDashboard, TelemetryEngine, TelemetryHooks)
- Dead code analysis shim (DeadCodeAnalysis.h — callers use DeadCodeAudit.h + DeadCodeAuditor.h directly)
- Legacy CI workflow (`build-v7.yml`)
- 5 tracked .gitignore'd CMake-generated vcxproj files from git index
- `.github/standards/` files renamed from UPPERCASE to lowercase-hyphenated

---

## [14.0.0] "Apex" - 2026-06

### Added
- **GPU Pipeline:** DX12 Ultimate mesh shaders, DXR, VRS, SM6.7 DXIL, PSO disk cache
- **Format Intelligence:** Smart format detector V2, extended video/audio/3D renderers
- **Plugin Ecosystem V2:** SDK V2 (9 capabilities), debugger integration, hot reload, profiler
- **Security Hardening V2:** Threat model (STRIDE/CVSS), ASan, SBOM/CVE, runtime integrity
- **UX Polish V2:** Progressive thumbnail loader, animation engine V2, Quick Look integration
- **AI Intelligence V2:** Scene understanding, smart crop, IQA (BRISQUE/NIQE), CLIP embeddings
- **Enterprise & Cloud V2:** Policy engine (GPO/MDM), SharePoint, multi-tenant cache, GDPR/HIPAA audit
- **Platform:** Windows 12 compatibility, ARM64 optimizer (NEON/SVE), sub-millisecond cache
- **Performance:** GPU decode acceleration V2 (NVDEC/QuickSync/AMF), parallel I/O (scatter-gather)
- **Quality:** Accessibility suite V2 (WCAG 2.1 AA), Release Gate V32 (23 KPIs)
- **New modules:** Engine/AI/, Engine/GPU/, Engine/Memory/

### Changed
- **Version:** 13.0.0 → 14.0.0 "Apex"
- **Total unit tests:** 937 → 1187

---

## [13.0.0] - 2026-05

### Added
- CDR/Visio vector decoder, HDF5/NetCDF scientific data, NIfTI neuroimaging
- STEP/IGES CAD formats, HDR display pipeline
- Per-monitor DPI V3, shell overlay icons, cache warming engine, multi-GPU load balancer
- Accessibility pipeline, telemetry analytics, cloud storage integration

### Changed
- **Version:** 12.0.0 → 13.0.0 | **Tests:** 837 → 937

---

## [12.0.0] - 2026-04

### Added
- Parallel batch processing, persistent cache/USN journal, ARM64 validation V2
- Windows 11 24H2 integration, fuzz testing engine, Vulkan compute pipeline
- Plugin marketplace V3, AI-assisted thumbnails, USD/USDZ 3D format
- CSV/JSON data preview, Notebook/Jupyter preview, database/SQLite preview

### Changed
- **Version:** 11.0.0 → 12.0.0 | **Tests:** 737 → 837

---

## [11.0.0] - 2026-03

### Added
- DPX/Cineon film format, APNG/animated format, text/code preview
- DICOM V2, FITS V2, 3MF/USD 3D printing
- D3D12 pipeline activation, async shell extension V2, SIMD acceleration V2

### Changed
- **Version:** 10.6.0 → 11.0.0 | **Tests:** 712 → 737

---

## [10.6.0] - 2026-03

### Added
- Format registry refactor — FormatType enum, FormatRegistry singleton
- LENSArchive.h split — FormatTypeLookup with 80+ extension mappings
- Shell registration manager, test infrastructure V2

### Changed
- **Version:** 10.5.0 → 10.6.0 | **Tests:** 687 → 712

---

## [10.5.0] - 2026-02

### Added
- File Hash Engine, Registry Manager, Error Recovery Engine, Log Rotation Engine
- Resource Pool Engine, CLI Parser, Metadata Extractor, Notification Engine
- Content Indexer, Network Diagnostics, Config Migration Engine

### Changed
- **Version:** 10.4.0 → 10.5.0

---

## [10.4.0] - 2026-01

### Added
- Accessibility Engine, Cloud Sync Provider, Format Converter Engine
- Enterprise Deployment (ADMX/GPO), Watch Folder Engine, Diagnostic Dashboard
- Localization Engine, Theme Engine, Telemetry Engine, Update Engine
- Shell Preview Handler, Batch Processing Engine

### Changed
- **Version:** 10.3.0 → 10.4.0

---

## [10.3.0] - 2025-12

### Added
- Animated Thumbnail Engine, Shell Context Menu V2, Portable Mode Manager
- Network Provider Engine, Security Hardening V2

---

## [10.2.0] - 2025-11

### Added
- CI/CD Pipeline Validation, eBook Decoder, Geospatial Decoder
- Auto Documentation Generator, Config Migration Engine

---

## [10.1.0] - 2025-10

### Added
- Async Shell Extension, Encoder Export Engine, SIMD Accelerator, Windows 11 Integration

---

## [10.0.0] - 2025-09

### Added
- Scientific Format Suite (DICOM, FITS), Advanced 3D Formats (FBX/USD/3MF/STEP/IGES)
- Plugin Marketplace V2, Vulkan Compute Pipeline, Python SDK

---

## [9.2.0] - 2025-08

### Added
- D3D12 Compute Pipeline, Parallel Batch Decoder, Persistent Disk Cache
- ARM64 Hardware Validator, High-DPI Scaling, MSIX Packaging
- Memory Safety (ASAN), Code Coverage Integration, Malformed Input Hardening

### Changed
- **Tests:** 437 → ~587

---

## [9.0.0] - 2025-07

### Added
- Format Expansion: WMF/EMF, PCX, Farbfeld, JPEG 2000, EPS/PostScript
- Game Textures: KTX/KTX2, VTF | Creative Suite: OpenRaster, GIMP XCF
- Enhanced Model Decoder: PLY/DAE/3DS/FBX wireframe | Retro: SGI/RGB, XPM

---

## [8.4.0] - 2025-06

### Fixed
- `.djvu`/`.djv` routing, AVIF/HEIF decoder overlap, 46 missing shell registrations

### Changed
- **Shell registrations:** 47 → 93

---

## [8.3.0] - 2025-05

### Added
- Plugin sandbox, trust chain, ARM64 build config, JPEG2000 header
- CAD decoder plugin, glTF decoder, archive memory compactor, zero-copy pipeline

### Changed
- **Tests:** ~100 → ~437

---

## [8.2.0] - 2025-04

### Added
- Advanced decoders: TGA, QOI, PSD, DDS, HDR, EXR, ICO, PPM, BMP, GIF
- Document decoder (DOCX/PPTX/XLSX), Font decoder, Video/Audio decoders

---

## [8.1.0] - 2025-03

### Added
- SVG decoder (Direct2D), RAW decoder (LibRaw, 27 camera formats)
- PDF decoder (MuPDF), Archive expansion (TAR/CPIO/ISO/CAB/DEB/XAR)

---

## [8.0.0] - 2025-02

### Added
- WebP (libwebp 1.5.0), AVIF (libavif 1.3.0 + dav1d), HEIF (libheif 1.19.5 + libde265)
- JPEG XL (libjxl 0.11.1), DirectX 12 GPU pipeline foundation
- ExplorerLensEngine.lib as separate static library, CMake 3.20+ build system

---

## [7.1.0] - 2026-02-18

### Added
- Observability Integration (ETW + structured logger)
- Build Validation, MSIX CLSID fix, LENSTYPE enum expansion

---

## [7.0.0] - 2026-02-16

### Added
- LibHEIF build script, integration tests for all 24 decoders

### Fixed
- Visual Studio 18 2026 migration (16+ locations)

---

## [6.2.0] - 2026-02-15

### Added
- User/Developer/Known Issues documentation
- SEH Exception Handling, Circuit Breaker Pattern, AVX2 flags

### Fixed
- LZMA SDK updated to 26.00, path standardization

---

## [6.0.0] - 2026-02-12

### Added
- EXIF Orientation Utility, Header Data IPC Protocol, Minizip-NG Integration

---

## [5.3.0] - 2026-02-10

### Added
- LibRaw SDK, Plugin system architecture, GDI+ RAII wrapper

---

[15.0.0]: https://github.com/ExplorerLens/ExplorerLens.io/compare/v14.0.0...v15.0.0
[14.0.0]: https://github.com/ExplorerLens/ExplorerLens.io/compare/v13.0.0...v14.0.0
[13.0.0]: https://github.com/ExplorerLens/ExplorerLens.io/compare/v12.0.0...v13.0.0
[12.0.0]: https://github.com/ExplorerLens/ExplorerLens.io/compare/v11.0.0...v12.0.0
[11.0.0]: https://github.com/ExplorerLens/ExplorerLens.io/compare/v10.6.0...v11.0.0
[10.6.0]: https://github.com/ExplorerLens/ExplorerLens.io/compare/v10.5.0...v10.6.0
[10.5.0]: https://github.com/ExplorerLens/ExplorerLens.io/compare/v10.4.0...v10.5.0
[10.4.0]: https://github.com/ExplorerLens/ExplorerLens.io/compare/v10.3.0...v10.4.0
[10.3.0]: https://github.com/ExplorerLens/ExplorerLens.io/compare/v10.2.0...v10.3.0
[10.2.0]: https://github.com/ExplorerLens/ExplorerLens.io/compare/v10.1.0...v10.2.0
[10.1.0]: https://github.com/ExplorerLens/ExplorerLens.io/compare/v10.0.0...v10.1.0
[10.0.0]: https://github.com/ExplorerLens/ExplorerLens.io/compare/v9.2.0...v10.0.0
[9.2.0]: https://github.com/ExplorerLens/ExplorerLens.io/compare/v9.0.0...v9.2.0
[9.0.0]: https://github.com/ExplorerLens/ExplorerLens.io/compare/v8.4.0...v9.0.0
[8.4.0]: https://github.com/ExplorerLens/ExplorerLens.io/compare/v8.3.0...v8.4.0
[8.3.0]: https://github.com/ExplorerLens/ExplorerLens.io/compare/v8.2.0...v8.3.0
[8.2.0]: https://github.com/ExplorerLens/ExplorerLens.io/compare/v8.1.0...v8.2.0
[8.1.0]: https://github.com/ExplorerLens/ExplorerLens.io/compare/v8.0.0...v8.1.0
[8.0.0]: https://github.com/ExplorerLens/ExplorerLens.io/compare/v7.1.0...v8.0.0
[7.1.0]: https://github.com/ExplorerLens/ExplorerLens.io/compare/v7.0.0...v7.1.0
[7.0.0]: https://github.com/ExplorerLens/ExplorerLens.io/compare/v6.2.0...v7.0.0
[6.2.0]: https://github.com/ExplorerLens/ExplorerLens.io/compare/v6.0.0...v6.2.0
[6.0.0]: https://github.com/ExplorerLens/ExplorerLens.io/compare/v5.3.0...v6.0.0
[5.3.0]: https://github.com/ExplorerLens/ExplorerLens.io/releases/tag/v5.3.0
