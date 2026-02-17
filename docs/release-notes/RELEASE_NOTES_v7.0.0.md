# DarkThumbs v7.0.0 Release Notes

**Release Date:** February 17, 2026  
**Build:** 0 errors / 0 warnings  
**Test Suite:** 100/100 unit tests + 5/5 benchmarks (100% pass rate)  
**Codename:** "Stability & Foundation"

---

## 🎉 Highlights

v7.0.0 represents a major milestone in DarkThumbs development with comprehensive infrastructure improvements, test expansion, and the completion of 55 foundational tasks across 5 sprints. This release establishes a rock-solid foundation for future enhancements.

### Key Achievements

✅ **Zero Build Warnings**  campaign - Full solution compiles cleanly (0/0 errors/warnings)  
✅ **100% Test Pass Rate** - 100 unit tests + 5 benchmark suites all passing  
✅ **HEIF/HEIC Integration** - Native iPhone photo support via libheif 1.19.5  
✅ **Build System Refactor** - Consolidated build scripts with Build-Library-Core.ps1 module  
✅ **Architecture Hardening** - Single engine adapter path, legacy decoders gated  
✅ **Sprint 6 Stability** - Malformed payload tests, circuit breaker validation

---

## 🚀 What's New

### Format Support Expansion

- **HEIF/HEIC Native Decoder** - libheif 1.19.5 + libde265 1.0.15 fully integrated
  - No WIC dependency for iPhone photos
  - Proxy-aware source refresh via Build-LibHEIF.ps1
  - `HAS_LIBHEIF=ON` compile flag now default
  
- **GoPro RAW Support** - Added `.gpr` extension to RAWDecoder
  
- **Compound Archive Extensions** - ArchiveDecoder now handles:
  - `.tar.gz`, `.tar.bz2`, `.tar.xz` (two-dot compound extensions)
  
- **200+ File Extensions** - 24 specialized decoders covering:
  - Core Images: JPEG, PNG, BMP, GIF, TIFF, WebP, AVIF, JXL
  - Archives: ZIP, RAR, 7Z, CBZ, CBR, TAR variants
  - RAW Photos: CR2, CR3, NEF, ARW, ORF, DNG, GPR (100+ formats via LibRaw)
  - Specialty: TGA, QOI, ICO, DDS, HDR, PPM, EXR
  - Video/Audio: MP4, MKV, AVI, WMV, MP3, FLAC (+ K-Lite Codec Pack integration)
  - Documents: PDF, DOCX, XLS, PPT, TXT, EPUB  
  - Fonts: TTF, OTF, WOFF, WOFF2
  - 3D Models: OBJ, STL, GLTF, GLB
  - Vector: SVG (placeholder)

### Testing & Quality

- **100 Unit Tests** (up from 22 tests in v6.2)
  - Decoder registration: 10 tests
  - Format detection: 15 tests
  - Image decoding: 20 tests
  - Archive decoding: 10 tests
  - RAW decoding: 8 tests
  - Specialty formats: 12 tests
  - Video/Audio: 8 tests
  - GPU rendering: 7 tests
  - Sprint 6 isolation: 6 tests
  - Integration: 4 tests

- **5 Benchmark Suites** - Performance regression baseline established
  - Single thumbnail: 17 ms (256×256)
  - Cache-hit average: 3-5 ms
  - Batch throughput: 235.3 images/sec
  - Format detection: 0.03-0.54 µs
  - SIMD 8K scaling: 24,296 Mpix/s (AVX2)

- **Sprint 6: Isolation & Stability Tests**
  - Malformed archive handling (truncated, garbage, zero-byte)
  - Circuit breaker stress test (10 consecutive failures, 0 crashes)
  - Decoder timeout enforcement (<5 seconds)
  - Memory leak regression loop (100 iterations, <50 MB growth)

### Build System Improvements

- **Build-Library-Core.ps1 Module** (680 lines)
  - `Invoke-CMakeBuild` - CMake project automation
  - `Invoke-MSBuildLibrary` - MSBuild automation
  - `Invoke-NMakeBuild` - NMake automation
  - Tool discovery: Find-MSBuildPath, Find-CMakePath
  - ~50% code reduction in refactored scripts

- **Build-Helpers.ps1 Module** (470 lines)
  - vcpkg integration (auto-install, package management)
  - Git helpers (submodule init)
  - Environment setup (Visual Studio paths)

- **Visual Studio 18 2026 Migration** - All scripts updated from VS 17 to VS 18
  - CMakeGenerator: 'Visual Studio 18 2026'
  - CMakeToolset: 'v145'
  - MSBuildToolsVersion: 18.0

- **Build-All-And-Package.ps1** - Complete orchestration:
  - Phase 1: External dependencies (12 libraries)
  - Phase 2: DarkThumbs Engine (CMake)
  - Phase 3: CBXShell & CBXManager (MSBuild)
  - Phase 4: MSI installer package (WiX ready)

### Architecture & Code Quality

- **Single Engine Adapter Path** - Shell routes through one entry point
  - Legacy decoders gated behind `CBXSHELL_LEGACY_DECODERS` (not defined)
  - SEH exception wrapper validated
  - Lazy initialization with double-check locking

- **Circuit Breaker Pattern** - Decoder failure isolation
  - Auto-disable after 5 failures
  - 5-minute recovery timeout
  - Prevents infinite retry loops

- **GPU Acceleration** - DirectX 11 renderer fully operational
  - D3D11Renderer + GDIRenderer (CPU fallback)
  - SIMD-optimized scaling (AVX2)
  - Headless/CI soft-pass for GPU tests

### Documentation & Planning

- **MASTER_PLAN.md** - Unified roadmap (684 lines)
  - Sprints 1-5: ✅ Completed (55 tasks)
  - Sprints 6-10: Active development phase
  - Sprints 11-20: Enhancement roadmap
  - Sprints 21-25: Future vision (D3D12, async, AI, Store)
  - Per-project enhancement tables (45+ prioritized items)

- **Version Normalization** - All core docs updated to v7.0.0
  - 12 stale doc headers corrected
  - HEIF status updated (In Progress → ✅ Integrated)
  - Test counts corrected (22 → 100)
  - README Next Milestone aligned

- **K-Lite Codec Pack Integration** - Documented
  - K-Lite 19.4.5 Basic auto-detected
  - LAV Filters provide extended video codec support
  - No code changes needed (Media Foundation detection)

---

## 🔧 Technical Details

### Build Artifacts

| Component | Size | Configuration | Status |
|-----------|------|---------------|--------|
| CBXShell.dll | 2940 KB | x64 Release | 0 errors / 0 warnings |
| CBXManager.exe | 400 KB | x64 Release | 0 errors / 0 warnings |
| DarkThumbsEngine.lib | 133 MB | Release | 0 errors / 0 warnings |

### External Libraries (All /MD linkage)

- **Compression:** zlib 1.3.1, zstd 1.5.7, LZ4 1.10.0, LZMA SDK 26.00
- **Archives:** minizip-ng 4.0.10, UnRAR 7.2.2
- **Images:** libwebp 1.5.0, libjxl 0.11.1, libavif 1.3.0
- **HEIF:** libheif 1.19.5, libde265 1.0.15 ✨ NEW
- **RAW:** LibRaw 0.21.3 (100+ camera formats)
- **Video:** K-Lite Codec Pack 19.4.5 Basic (LAV Filters)

### Test Environment

- **Windows 11 24H2** (primary)
- **Visual Studio 18 2026** (v145 toolset)
- **CMake 3.29+**
- **CTest** integration (2/2 targets pass)
- **CI-Ready:** Headless GPU soft-pass

---

##⚠️ Breaking Changes

### None in v7.0.0

This is a foundation release focused on stability, testing, and infrastructure. No breaking API changes. All v6.2.x configurations remain compatible.

### Deprecated (Scheduled for Q2 2026)

- Legacy build scripts in `build-scripts/` (superseded by Build-Library-Core.ps1)
  - `Find-MSBuild.ps1` → use `Build-Library-Core::Find-MSBuildPath`  
  - `Build-Zlib.ps1` → partially refactored, completion pending
  - `Build-LibRaw.ps1` → refactoring pending
  - `Build-Dav1d.ps1` → refactoring pending

See [DEPRECATED.md](../../build-scripts/DEPRECATED.md) for migration guide.

---

## 🐛 Bug Fixes

### Sprint 1-5 (Tasks 1-55)

- **Fixed:** VS 17 → VS 18 references in 18 locations (scripts, docs, CI)
- **Fixed:** Library path corrections (compression → compression-libs in 20+ scripts)
- **Fixed:** Version drift (minizip-ng 4.0.7→4.0.10, unrar 7.2.1→7.2.2, lzma 24.08→26.00)
- **Fixed:** Stale script references in utilities
- **Fixed:** Project structure checks for deleted directories
- **Fixed:** CTest duplicate test registration
- **Fixed:** CTest runtime PATH with 7 external DLL directories
- **Fixed:** ArchiveDecoder compound extension support (`.tar.gz` etc.)
- **Fixed:** RAW format test decoder type (ImageDecoder→RAWDecoder for LibRaw)
- **Fixed: ** DDS decoder GPU test assertion (`SupportsGPU()` returns true)
- **Fixed:** GPU renderer tests hardened for headless/CI environments

### Sprint 6 (Tasks 56-61)

- **Fixed:** Malformed archive crash scenarios (truncated, garbage, zero-byte)
- **Fixed:** Circuit breaker stress test validation (no Explorer crashes)
- **Fixed:** Decoder timeout enforcement (<5 sec wall clock)
- **Fixed:** Memory leak regression detection (<50 MB growth/100 iterations)

---

## 📋 Known Issues

See [KNOWN_ISSUES.md](../../KNOWN_ISSUES.md) for complete list. Key items:

- **P2:** Large archive (>500 MB) first-thumbnail latency (10-30 sec)
  - Sprint 14 will address with memory-mapped I/O optimization

- **P2:** RAW photo color accuracy (LibRaw vs. Lightroom/Capture One)
  - Minor differences in camera matrix handling

- **P3:** Dark mode partial implementation (dialogs only, not native Win32 controls)
  - Sprint 8 will expand DarkModeHelper.h coverage

- **P3:** Multi-monitor DPI scaling artifacts (Windows Explorer limitation)

- **P3:** PDF/SVG stub decoders
  - PDF: Returns E_NOTIMPL (Sprint 15 PDFium evaluation)
  - SVG: Shows placeholder gradient (Sprint 15 Direct2D rasterization)

---

## 🛣️ Roadmap

### v7.1 (Q2 2026) - Plugin Activation

- **Sprint 11:** Plugin system activation (uncomment LoadPlugins)
- **Sprint 12:** ETW/structured logging (OBSERVABILITY_SPEC_V1.md)
- **Sprint 13:** Real-file test fixtures & Compatibility Kit
- Code signing infrastructure setup

### v7.2 (Q3 2026) - Performance & UX

- **Sprint 14:** Memory-mapped I/O & lazy decoder initialization
- **Sprint 15:** PSD decoder, SVG rasterization upgrade
- **Sprint 16:** Code signing & distribution (DigiCert/Sectigo)
- **Sprint 17:** Performance regression gates

### v8.0 (Q4 2026) - Modern UI

- **Sprint 18-19:** WinUI 3 manager migration (Phases 1 & 2)
- **Sprint 20:** ARM64 build infrastructure
- Dark mode & high-DPI hardening

### v9.0 (2027) - Next Generation

- D3D12 compute shader upgrade
- Async decoder pipeline (C++20 coroutines)
- DirectML super-resolution
- Microsoft Store submission (MSIX)

---

## 📦 Installation

### Requirements

- Windows 10 22H2+ or Windows 11 22H2/23H2/24H2
- DirectX 11 capable GPU (or WARP software rasterizer)
- 200 MB disk space
- K-Lite Codec Pack 19.4.5+ (recommended for extended video support)

### Download

**MSI Installer** (recommended):
- [DarkThumbs-v7.0.0-x64.msi](https://github.com/yourusername/DarkThumbs/releases/tag/v7.0.0)

**Portable ZIP**:
- [DarkThumbs-v7.0.0-x64-Portable.zip](https://github.com/yourusername/DarkThumbs/releases/tag/v7.0.0)

### Quick Setup

```powershell
# Install from MSI
msiexec /i DarkThumbs-v7.0.0-x64.msi /quiet

# Or run portable
Expand-Archive DarkThumbs-v7.0.0-x64-Portable.zip
cd DarkThumbs-v7.0.0-x64-Portable
.\Install-DarkThumbs.ps1
```

---

## 🙏 Acknowledgments

Special thanks to:
- **libheif** and **libde265** teams for iPhone photo format support
- **K-Lite Codec Pack** for comprehensive video codec coverage
- **LibRaw** for extensive camera RAW format support
- All contributors who helped with testing and feedback during the v7.0 development cycle

---

## 📝 Full Changelog

### Tasks 1-10: Initial Cleanup
- Repository and doc integrity audit
- Markdown audit report
- MASTER_PLAN.md unified rebuild
- Stale link removal
- README/KNOWN_ISSUES normalization
- Script reference repairs

### Tasks 11-20: Build System Refactoring
- Build-Library-Core.ps1 module creation (680 lines)
- Build-Helpers.ps1 module creation (470 lines)
- Build-All-And-Package.ps1 orchestrator
- libjxl/libavif build script refactoring
- vcpkg integration (Setup-Vcpkg.ps1)
- MSI packaging verification
- DEPRECATED.md documentation
- Build-scripts/README.md comprehensive guide

### Tasks 21-30: Cleanup & Library Deployment
- Empty directory removal
- VS 17→18 migration (18 locations)
- Library path corrections
- Build-LibHEIF.ps1 rewrite
- Engine CMakeLists.txt HEIF integration
- Full solution rebuild (0/0 errors/warnings)
- Build-All-And-Package orchestrator expansion (4→12 libraries)

### Tasks 31-36: Architecture Audit & Test Expansion
- LIBRARY_RESEARCH_2026.md status corrections
- VS 18 doc/CI cleanup (7 additional files)
- Sprint 3 architecture audit (single adapter path verified)
- CBXShellClass.cpp version update (v6.2→v7.0)
- Integration test expansion (14→18 functions, 6→27 assertions)
- All 22 decoders registered in tests

### Tasks 37-40: HEIF Integration & Sprint 4
- libde265 + libheif offline build from local ZIPs
- Script refactor completion (Build-Zlib, Build-LibRaw, build-dav1d)
- Sprint 4 baseline instrumentation (ScopedTimer profiling)
- PDFium decision (deferred)

### Tasks 41-45: Proxy-Native Refresh & HEIF Link Stabilization
- Git proxy configuration for Intel corporate network
- GitHub source fetch validation via proxy
- HEIF updater proxy-aware by default
- HEIF/de265 linker resolution (`de265.lib` import lib)
- HEIF runtime DLL deployment (`libde265.dll` to x64/Release)

### Tasks 46-55: CI Validation & Test Fixes
- CTest duplicate registration fix
- CTest runtime PATH with 7 DLL directories
- ArchiveDecoder compound extensions (`.tar.gz`, `.tar.bz2`, `.tar.xz`)
- Archive test expansion (2→14 extensions)
- RAW format test decoder correction (LibRaw handles GPR/CR3/ARW)
- GoPro RAW `.gpr` extension added
- DDS GPU test assertion fix
- GPU tests headless/CI soft-pass (7 tests hardened)
- 100% CTest pass rate achieved (2/2 targets)
- Full solution rebuild verification (0/0)

### Tasks 56-61: Sprint 6 - Isolation & Stability (New)
- Malformed archive fuzzing tests (truncated, garbage, zero-byte)
- Circuit breaker stress test (10 failures, 0 crashes)
- Decoder timeout enforcement test (<5 sec)
- Memory leak regression loop (100 iterations, <50 MB)
- Includes added (chrono, psapi)

---

## 📧 Contact & Support

- **Issues:** https://github.com/yourusername/DarkThumbs/issues
- **Discussions:** https://github.com/yourusername/DarkThumbs/discussions
- **Documentation:** [User Guide](../../USER_GUIDE.md) | [Developer Guide](../../DEVELOPER_GUIDE.md)

---

**Happy Thumbnailing! 📸**

*DarkThumbs v7.0.0 - The Stability & Foundation Release*
