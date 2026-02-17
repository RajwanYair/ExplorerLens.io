# DarkThumbs v7.x - Complete Refactoring Plan
## Best-in-Class Windows 11 Application

**Date:** February 16, 2026  
**Status:** Active Execution  
**Priority:** Critical Path to v7.0 Release  
**Dependencies:** MASTER_PLAN.md (strategic), this doc (tactical execution)

---

## Executive Summary

This refactoring plan addresses the comprehensive analysis findings:
1. **Code Duplication**: Duplicate decoder implementations (Engine vs CBXShell)
2. **Build Script Redundancy**: 15+ similar build scripts with duplicate logic
3. **Documentation Inconsistencies**: Version drift and stale references
4. **Windows 11 Optimization**: GPU acceleration, DPI handling, dark mode
5. **Performance Bottlenecks**: Cold start, large archives, first-thumbnail latency

**Success Metrics:**
- 0 duplicate decoder implementations
- 0 build warnings (Release x64)
- <100ms p95 thumbnail latency for cached thumbnails
- 100% Windows 11 23H2/24H2 compatibility
- WinUI 3 GUI parity with existing features

---

## Phase 1: Code Consolidation (Sprint 1-3, ~2 weeks)

### 1.1 Decoder Duplication Elimination

**CRITICAL FINDING**: Duplicate decoder implementations found:

| Decoder | Engine Location | CBXShell Location | Status |
|---------|----------------|-------------------|--------|
| AVIF | Engine/Decoders/AVIFDecoder.cpp/.h | CBXShell/avif_decoder.cpp/.h | 🔴 Duplicate |
| JXL | Engine/Decoders/JXLDecoder.cpp/.h | CBXShell/jxl_decoder.cpp/.h | 🔴 Duplicate |
| WebP | Engine/Decoders/WebPDecoder.cpp/.h | CBXShell/webp_decoder.cpp/.h | 🔴 Duplicate |
| HEIF | Engine/Decoders/HEIFDecoder.cpp/.h | CBXShell/heif_decoder_native.cpp/.h | 🔴 Duplicate |
| RAW | Engine/Decoders/RAWDecoder.cpp/.h | CBXShell/raw_decoder.cpp/.h | 🔴 Duplicate |

**Action Plan:**
1. **Task 1**: Remove CBXShell decoder files (avif_decoder.*, jxl_decoder.*, webp_decoder.*, heif_decoder_native.*, raw_decoder.*)
2. **Task 2**: Update CBXShellClass.cpp to use EngineAdapter exclusively
3. **Task 3**: Remove legacy decoder includes from StdAfx.h
4. **Task 4**: Update CBXShell.vcxproj to remove decoder compilation units
5. **Task 5**: Verify all decoder calls route through Engine API

**Benefits:**
- Single decoder maintenance path
- Consistent behavior across all consumers
- ~50% reduction in CBXShell.dll size
- Easier testing and validation

### 1.2 Build Script Consolidation

**FINDING**: 15+ build scripts with similar patterns

**Current State:**
```
build-scripts/
├── build-image-libs.ps1
├── Build-ImageLibs-CMake.ps1
├── external-libs/
│   ├── Build-LibWebP-NMake.ps1
│   ├── Build-MinizipNG.ps1
│   ├── build-libjxl.ps1
│   ├── build-libavif.ps1
│   ├── Build-Zlib.ps1
│   ├── Build-Zstd.ps1
│   ├── Build-LZ4.ps1
│   └── build-lzma-sdk-26.00.ps1
├── library-builders/
│   ├── Build-All-Libraries.ps1
│   ├── Build-All-External-Libraries.ps1
│   └── Build-Libraries-Simple.ps1
└── production/
    └── Rebuild-External-Libs-Correct-Runtime.ps1
```

**Action Plan:**
1. **Task 6**: Create unified `build-scripts/core/Build-Library-Core.ps1` with common functions:
   - `Invoke-CMakeBuild`
   - `Invoke-MSBuildLibrary`
   - `Invoke-MesonBuild`
   - `Test-BuildOutput`
   - `Copy-LibraryArtifacts`

2. **Task 7**: Refactor individual library scripts to use core functions
   - Reduce each script from 100+ lines to ~20 lines
   - Standard parameter set: `-Configuration`, `-Clean`, `-Verbose`

3. **Task 8**: Consolidate orchestration scripts:
   - Keep: `build-scripts/Build-All-Libraries.ps1` (main entry point)
   - Deprecate: `Build-Libraries-Simple.ps1`, `Build-All-External-Libraries.ps1`

4. **Task 9**: Create `build-scripts/README.md` with script hierarchy documentation

### 1.3 Utility Function Deduplication

**FINDING**: Similar helper functions scattered across scripts

**Action Plan:**
1. **Task 10**: Create `build-scripts/core/Build-Helpers.ps1` module with:
   - `Find-MSBuildPath` (currently duplicated 3+ times)
   - `Find-CMakePath`
   - `Test-VisualStudioTools`
   - `Write-BuildLog`
   - `Invoke-SafeCleanup`

2. Consolidate path discovery into single module
3. Import module in all build scripts

---

## Phase 2: Windows 11 Optimization (Sprint 4-6, ~3 weeks)

### 2.1 Modern GPU Acceleration (DirectX 12)

**Current**: DirectX 11 with basic texture rendering  
**Target**: DirectX 12 with modern best practices

**Implementation:**
1. Create `Engine/GPU/D3D12Renderer.cpp/.h` alongside existing D3D11
2. Implement feature detection and fallback chain: D3D12 → D3D11 → Software
3. Add GPU memory pooling for thumbnails (reduce allocation overhead)
4. Implement async compute queues for decode/resize parallelism

**Benefits:**
- 30-40% better GPU utilization on modern hardware
- Reduced CPU-GPU sync overhead
- Better multi-GPU support (iGPU + dGPU scenarios)

### 2.2 Windows 11 Shell Integration

**Current Issues:**
- No Windows 11 context menu integration (File Explorer "Show more options" required)
- Missing thumbnail live preview in new File Explorer
- No Windows 11 Share target integration

**Action Plan:**
1. Implement `IExplorerCommand` for Windows 11 context menu
2. Add thumbnail preview handler for quick look
3. Register as share target for image sharing scenarios
4. Implement Windows 11 notification integration for errors

### 2.3 High DPI Support

**Current**: Partial DPI awareness  
**Target**: Per-Monitor DPI v2 (full Windows 11 support)

**Implementation:**
1. Update manifest with `<dpiAwareness>PerMonitorV2</dpiAwareness>`
2. Handle `WM_DPICHANGED` messages in CBXManager
3. Scale thumbnail sizes based on monitor DPI
4. Test on mixed DPI setups (125%, 150%, 175%, 200%, 250%)

---

## Phase 3: GUI Modernization (Sprint 7-9, ~3 weeks)

### 3.1 WinUI 3 Migration for CBXManager

**Current**: WTL (Windows Template Library) - Win32 dialogs  
**Target**: WinUI 3 with Windows 11 native controls

**Migration Path:**
1. Keep existing CBXManager.exe as fallback (rename to CBXManager-Legacy.exe)
2. Create new `CBXManager-Modern` project with WinUI 3
3. Implement feature parity:
   - Format enable/disable (CategorySettingsPage.xaml)
   - Cache management (CacheSettingsPage.xaml)
   - GPU selection (PerformancePage.xaml)
   - Diagnostics (DiagnosticsPage.xaml)
   - About/health check (AboutPage.xaml)

4. Package as MSIX for Microsoft Store distribution

**Benefits:**
- Native Windows 11 look and feel
- Fluent Design support (Acrylic, Mica backgrounds)
- Automatic dark mode support
- Touch-friendly UI
- Easier maintenance (XAML vs Win32 messages)

### 3.2 Dark Mode Support

**Current**: Partial support in legacy CBXManager  
**Target**: Full system theme awareness

**Implementation:**
1. WinUI 3 automatically supports dark mode
2. For shell extension, query `HKCU\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize\AppsUseLightTheme`
3. Adjust thumbnail rendering for dark backgrounds (add subtle borders)

---

## Phase 4: Performance Optimization (Sprint 10-12, ~3 weeks)

### 4.1 Cold Start Optimization

**Current Baseline**: First thumbnail 300-500ms (cold cache)  
**Target**: <150ms p95

**Optimizations:**
1. Lazy decoder initialization (don't load unused decoders)
2. Pre-warm GPU shader cache at shell extension load
3. Memory-mapped file I/O for archives (avoid full extraction)
4. Parallel decoder probing (try multiple decoders simultaneously)

### 4.2 Large Archive Performance

**Current Issue**: 10-30s for first image in >500MB archives  
**Target**: <2s for any archive size

**Implementation:**
1. Implement streaming ZIP decoder (read central directory, seek to first image)
2. Add archive index cache (remember first image location)
3. Use minizip-ng memory-mapped API for zero-copy extraction
4. Parallelize decompression (zstd/lz4 support multi-threading)

### 4.3 Memory Optimization

**Current**: ~100MB peak for typical batch (50 thumbnails)  
**Target**: <50MB sustained, <150MB peak

**Strategies:**
1. Implement texture pooling (reuse HBITMAP handles)
2. Add LRU cache eviction policy
3. Stream large images (tile-based decode for >4K images)
4. Use DirectX 12 placed resources for zero-copy GPU uploads

---

## Phase 5: Testing & Quality (Sprint 13-15, ~2 weeks)

### 5.1 Automated Testing Infrastructure

**Implementation:**
1. Unit tests for all decoders (GoogleTest framework)
2. Integration tests for shell extension (COM automation)
3. Performance regression tests (benchmark suite)
4. Memory leak detection (AddressSanitizer + Valgrind/Dr. Memory)
5. Fuzz testing for decoders (AFL/libFuzzer with malformed files)

### 5.2 Windows 11 Compatibility Matrix

**Test Matrix:**
| OS Version | DPI | GPU | Status |
|------------|-----|-----|--------|
| Win11 22H2 | 100% | Intel iGPU | ⏳ |
| Win11 22H2 | 150% | NVIDIA dGPU | ⏳ |
| Win11 23H2 | 125% | AMD iGPU | ⏳ |
| Win11 23H2 | 200% | Intel + NVIDIA | ⏳ |
| Win11 24H2 | 175% | AMD dGPU | ⏳ |
| Win11 24H2 | 250% | ARM64 Qualcomm | ⏳ |

### 5.3 Real-World Testing

**Test Scenarios:**
1. 1000+ image folder browsing (memory/performance under load)
2. Mixed format folders (all 155+ formats in single folder)
3. Network drives (SMB, OneDrive, Dropbox)
4. Encrypted archives (password-protected ZIP/RAR)
5. Corrupted files (decoder robustness)

---

## Phase 6: Release & Distribution (Sprint 16-17, ~1 week)

### 6.1 Release Packaging

**Deliverables:**
1. `DarkThumbs-7.0.0-x64.msi` (traditional installer)
2. `DarkThumbs-7.0.0-x64.msix` (Microsoft Store/sideload)
3. `DarkThumbs-7.0.0-ARM64.msix` (Windows 11 ARM devices)
4. Portable ZIP package (xcopy deployment)

### 6.2 Documentation Finalization

**Required Docs:**
1. USER_GUIDE.md (updated for WinUI 3 manager)
2. UPGRADE_GUIDE.md (v6.x → v7.0 migration)
3. WINDOWS_11_FEATURES.md (new capabilities showcase)
4. API_REFERENCE.md (for plugin developers)

### 6.3 Microsoft Store Submission

**Requirements:**
1. MSIX packaging
2. Age rating (PEGI 3 / ESRB E)
3. Privacy policy URL
4. Screenshots (light/dark mode, 1920x1080)
5. Store listing localization (English, 中文, Español, Français, Deutsch, 日本語)

---

## Detailed Task List (First 25 Tasks)

### Sprint 1: Code Consolidation (Week 1-2)

| # | Task | Estimate | Dependencies |
|---|------|----------|--------------|
| 1 | Remove CBXShell/avif_decoder.cpp/.h | 2h | None |
| 2 | Remove CBXShell/jxl_decoder.cpp/.h | 2h | None |
| 3 | Remove CBXShell/webp_decoder.cpp/.h | 2h | None |
| 4 | Remove CBXShell/heif_decoder_native.cpp/.h | 2h | None |
| 5 | Remove CBXShell/raw_decoder.cpp/.h | 2h | None |
| 6 | Update CBXShellClass.cpp - remove legacy decoder includes | 4h | 1-5 |
| 7 | Update CBXShell.vcxproj - remove decoder compilation units | 2h | 1-5 |
| 8 | Create build-scripts/core/Build-Library-Core.ps1 | 8h | None |
| 9 | Refactor Build-LibWebP-NMake.ps1 to use core functions | 4h | 8 |
| 10 | Refactor Build-MinizipNG.ps1 to use core functions | 4h | 8 |
| 11 | Refactor build-libjxl.ps1 to use core functions | 4h | 8 |
| 12 | Refactor build-libavif.ps1 to use core functions | 4h | 8 |
| 13 | Create build-scripts/core/Build-Helpers.ps1 | 6h | None |
| 14 | Consolidate Find-MSBuild functions | 3h | 13 |
| 15 | Update all scripts to import Build-Helpers.ps1 | 4h | 13-14 |

### Sprint 2: Build System Cleanup (Week 3)

| # | Task | Estimate | Dependencies |
|---|------|----------|--------------|
| 16 | Deprecate Build-Libraries-Simple.ps1 | 2h | 8-12 |
| 17 | Deprecate Build-All-External-Libraries.ps1 | 2h | 8-12 |
| 18 | Create build-scripts/README.md with hierarchy docs | 4h | 8-17 |
| 19 | Update DEVELOPER_GUIDE.md with new build paths | 4h | 18 |
| 20 | Clean up stale script references in docs | 3h | 18-19 |

### Sprint 3: Testing & Validation (Week 4)

| # | Task | Estimate | Dependencies |
|---|------|----------|--------------|
| 21 | Create decoder validation test suite | 8h | 1-7 |
| 22 | Test all 155+ formats with engine-only decoders | 8h | 21 |
| 23 | Performance baseline measurements (before optimization) | 4h | 21-22 |
| 24 | Memory leak testing with AddressSanitizer | 6h | 21-22 |
| 25 | Update CHANGELOG.md with consolidation results | 2h | 1-24 |

---

## Success Criteria & KPIs

### Code Quality
- ✅ 0 duplicate decoder implementations
- ✅ 0 compiler warnings (Release x64, /W4 /WX)
- ✅ 100% decoder code in Engine (0% in CBXShell)
- ✅ <10 build scripts (down from 25+)

### Performance
- ✅ <100ms p95 thumbnail latency (cached)
- ✅ <200ms p95 thumbnail latency (cold, single file)
- ✅ <2s first image from 500MB+ archive
- ✅ <50MB memory baseline, <150MB peak (50 thumbnails)

### Windows 11 Compatibility
- ✅ 100% pass rate on Windows 11 22H2/23H2/24H2
- ✅ Per-Monitor DPI v2 support
- ✅ Dark mode support (automatic theme detection)
- ✅ Windows 11 context menu integration

### GUI Modernization
- ✅ WinUI 3 manager with 100% feature parity
- ✅ Fluent Design integration (Mica/Acrylic)
- ✅ Touch-friendly UI (min 44x44px touch targets)
- ✅ MSIX packaging for Store distribution

---

## Risk Mitigation

### Technical Risks

| Risk | Probability | Impact | Mitigation |
|------|------------|--------|------------|
| WinUI 3 migration breaks existing functionality | Medium | High | Keep legacy CBXManager as fallback |
| DirectX 12 compatibility issues on older hardware | Medium | Medium | Implement D3D12 → D3D11 → Software fallback |
| Performance regression from consolidation | Low | High | Baseline + continuous benchmarking |
| Windows 11 API changes in 25H2 | Low | Low | Follow Windows Insider builds |

### Schedule Risks

| Risk | Probability | Impact | Mitigation |
|------|------------|--------|------------|
| WinUI 3 learning curve delays Sprint 7-9 | High | Medium | Parallel prototype, early training |
| Build script refactoring breaks production builds | Medium | High | Feature branch, extensive CI testing |
| Testing phase reveals major bugs | Medium | High | Buffer 2 weeks for bug fixes |

---

## Execution Timeline

```
February 2026        Sprint 1-3: Code Consolidation
├─ Week 1-2         ├─ Decoder deduplication
├─ Week 3           └─ Build script consolidation

March 2026          Sprint 4-6: Windows 11 Optimization
├─ Week 1-2         ├─ DirectX 12 migration
├─ Week 3           └─ Shell integration + DPI

April 2026          Sprint 7-9: GUI Modernization
├─ Week 1-2         ├─ WinUI 3 CBXManager
├─ Week 3           └─ Dark mode + polish

May 2026            Sprint 10-12: Performance
├─ Week 1-2         ├─ Cold start optimization
├─ Week 3           └─ Memory optimization

June 2026           Sprint 13-15: Testing & Quality
├─ Week 1           ├─ Automated tests
├─ Week 2           └─ Compatibility matrix

June 2026           Sprint 16-17: Release
├─ Week 3-4         ├─ Packaging + Store submission
└─ Week 4           └─ v7.0 GA Release
```

**Total Duration:** ~18 weeks (4.5 months)  
**Target Release:** June 30, 2026

---

## Next Actions (Immediate)

1. ✅ Review and approve this refactoring plan
2. ⏳ Execute Tasks 1-10 (decoder consolidation + build core module)
3. ⏳ Set up CI pipeline for automated testing
4. ⏳ Create feature branch: `refactor/v7-consolidation`
5. ⏳ Begin WinUI 3 prototype in parallel (de-risk Sprint 7-9)

---

**Document Owner:** Development Team  
**Approved By:** [Pending Review]  
**Next Review:** February 23, 2026 (Sprint 1 completion)
