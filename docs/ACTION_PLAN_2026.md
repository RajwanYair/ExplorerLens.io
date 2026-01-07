# DarkThumbs - Action Plan 2026

**Last Updated:** January 7, 2026  
**Status:** ACTIVE - Build System Priority

---

## 🚨 CURRENT REALITY CHECK

### What Works ✅

- **zlib 1.3.1**: Built successfully (128.9 KB)
- **LZ4 1.10.0**: Built successfully (645.6 KB)
- **Project Structure**: Well-organized codebase
- **Documentation**: Comprehensive specs and guides

### What's Broken ❌

- **CBXShell Build**: DLL not generating, only .lib/.exp files
- **liblzma**: CMake fails to generate Makefile (environment issue)
- **zstd 1.5.7**: Source incomplete, missing CMake build files
- **LibWebP 1.5.0**: nmake runs but produces no output files
- **Minizip-NG**: Blocked by missing zstd and liblzma

### The Core Problem

**We spent months planning but can't build the project.** Build system issues block ALL development work.

---

## 📋 PRIORITY ACTIONS (Ordered by Criticality)

### P0: FIX BUILD SYSTEM IMMEDIATELY (THIS WEEK)

**Goal:** Get ALL libraries building reliably  
**Blocking:** Everything else  
**Owner:** Build Team

#### Actions

1. **Complete External Libraries** (2-3 days)

   ```cmd
   # Open "x64 Native Tools Command Prompt for VS 2026"
   
   # 1. Build liblzma
   cd external\compression\xz-5.6.3\build-vs
   cmake .. -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release
   nmake
   
   # 2. Download complete zstd source
   # From: https://github.com/facebook/zstd/releases/tag/v1.5.7
   # Extract to external\compression\zstd-1.5.7\ (replace existing)
   cd external\compression\zstd-1.5.7\lib
   nmake /f Makefile.vc
   
   # 3. Fix LibWebP build
   cd external\image-libs\libwebp-1.5.0
   nmake /f Makefile.vc CFG=release-static RTLIBCFG=static clean
   nmake /f Makefile.vc CFG=release-static RTLIBCFG=static all
   
   # 4. Build Minizip-NG
   cd external\compression\minizip-ng-4.0.10\build-vs
   cmake .. -G "NMake Makefiles" -DMZ_ZLIB=ON -DMZ_LZMA=ON -DMZ_ZSTD=ON
   nmake
   ```

2. **Fix CBXShell.vcxproj** (1 day)
   - Fix MIDL command line
   - Fix precompiled header order
   - Ensure DLL is output (not just .lib)
   - Verify x64 configuration

3. **Create Build Verification Script** (0.5 day)

   ```powershell
   # Verify all libraries exist with correct sizes
   # Run full build from clean state
   # Verify CBXShell.dll registers correctly
   ```

**Exit Criteria:**

- ✅ All 6 external libraries built and verified
- ✅ CBXShell.dll builds successfully
- ✅ Clean build from scratch takes <10 minutes
- ✅ Automated verification script passes

**Documentation:** See `docs/BUILD_GUIDE.md` (consolidated guide)

---

### P1: VERIFY PRODUCTION BASELINE (NEXT WEEK)

**Goal:** Confirm v5.2.0 actually works  
**Depends On:** P0 complete  
**Duration:** 2-3 days

#### Actions

1. **End-to-End Testing**
   - Register CBXShell.dll in test environment
   - Verify thumbnails generate for all 31 formats
   - Test GPU acceleration with Intel/NVIDIA
   - Verify cache system works
   - Stress test with 1000+ files

2. **Performance Baseline**
   - Measure thumbnail generation times
   - Record GPU vs CPU performance
   - Document cache hit ratios
   - Create performance regression tests

3. **Bug Triage**
   - Identify any crashes
   - Document Explorer integration issues
   - List format-specific problems

**Exit Criteria:**

- ✅ v5.2.0 works reliably on clean Windows 11 install
- ✅ All advertised formats thumbnail correctly
- ✅ No Explorer crashes under normal use
- ✅ Performance baseline documented

---

### P2: ENGINE REFACTORING (2-3 WEEKS)

**Goal:** Separate engine from shell extension  
**Depends On:** P1 complete  
**Version:** v5.3.0

#### Why This Matters

Current code mixes shell extension logic with decoder logic. This blocks:

- Unit testing
- Plugin system
- Alternative frontends (CLI, API)
- Code reuse

#### Implementation

1. **Create Engine Directory Structure** (Week 1)

   ```
   Engine/
   ├── include/
   │   ├── IThumbnailDecoder.h
   │   ├── ICacheProvider.h
   │   ├── IGPURenderer.h
   │   └── ThumbnailRequest.h
   ├── src/
   │   ├── DecoderRegistry.cpp
   │   ├── CacheManager.cpp
   │   └── FormatDetector.cpp
   └── decoders/
       ├── AvifDecoder.cpp
       ├── WebPDecoder.cpp
       └── ...
   ```

2. **Define Stable Interfaces** (Week 1)

   ```cpp
   struct ThumbnailRequest {
       std::wstring path;
       uint32_t sizePx;
       uint32_t flags;
       uint64_t correlationId;
   };
   
   struct ThumbnailResult {
       HRESULT hr;
       uint32_t width, height;
       uint64_t elapsedUs;
       HBITMAP bitmap;
   };
   
   interface IThumbnailDecoder {
       virtual bool CanDecode(const wchar_t* path) = 0;
       virtual HRESULT Decode(const ThumbnailRequest& req, ThumbnailResult& result) = 0;
   };
   ```

3. **Move Decoders** (Week 2)
   - Extract each decoder to separate file
   - Make them implement IThumbnailDecoder
   - Remove shell extension dependencies
   - Add unit tests for each decoder

4. **Update CBXShell** (Week 2-3)
   - Use Engine API instead of inline decoder code
   - Become thin adapter layer
   - Handle shell extension specific concerns only

**Exit Criteria:**

- ✅ Engine builds as separate library
- ✅ All decoders work through interfaces
- ✅ 50+ unit tests passing
- ✅ CBXShell uses engine, no decoder code in shell layer

---

### P3: PLUGIN SYSTEM (3-4 WEEKS)

**Goal:** Enable third-party format support  
**Depends On:** P2 complete  
**Version:** v5.4.0

#### Implementation

1. **SDK Implementation** (Week 1)
   - Create SDK/include/DarkThumbsPlugin.h
   - C ABI with stable exports
   - Build sample plugin (BMP decoder)

2. **Plugin Manager** (Week 2)
   - Plugin discovery and loading
   - Version compatibility checking
   - Capability validation
   - Basic error isolation

3. **Package Format** (Week 3)
   - .dtplugin = ZIP with manifest.json
   - Signature verification (basic code signing)
   - Install/uninstall mechanism

4. **Testing & Documentation** (Week 4)
   - Sample plugins: BMP, TGA, TIFF
   - Plugin developer guide
   - API documentation

**Exit Criteria:**

- ✅ Sample plugin loads and generates thumbnails
- ✅ Plugin can be installed/uninstalled via Manager
- ✅ Plugin developer documentation published
- ✅ 3+ reference plugins available

---

### P4: MANAGER APP INTEGRATION (2-3 WEEKS)

**Goal:** Wire Sprint 13 UI to real engine  
**Depends On:** P2, P3 complete  
**Version:** v5.5.0

#### Current Status

WinUI 3 app exists with nice UI but all services are stubs. Need to:

1. **Replace Stub Services** (Week 1)
   - Connect settings to actual config files
   - Implement IPC to shell extension
   - Wire up plugin management to Plugin Manager

2. **Live Diagnostics** (Week 2)
   - Connect to actual log files
   - Show real cache statistics
   - Display actual performance metrics
   - GPU status and selection

3. **Plugin Management UI** (Week 2-3)
   - Real install/uninstall
   - Plugin marketplace integration (local)
   - Update checking

**Exit Criteria:**

- ✅ Settings changes take effect immediately
- ✅ Diagnostics show real data
- ✅ Plugin management fully functional
- ✅ Manager controls actual engine behavior

---

### P5: PERFORMANCE OPTIMIZATION (2-3 WEEKS)

**Goal:** 20%+ faster than v5.2.0  
**Depends On:** P2 complete  
**Version:** v5.6.0

#### Targets

1. **Multi-GPU Support** (Week 1)
   - Enumerate all GPUs (Intel, NVIDIA, AMD)
   - Device selection policy
   - Per-format device preferences

2. **CPU Optimizations** (Week 2)
   - SIMD for resize/colorspace (SSE4/AVX2)
   - Runtime CPU feature detection
   - Optimized memory allocation

3. **Pipeline Improvements** (Week 3)
   - Request prioritization (visible thumbnails first)
   - Timeout enforcement (prevent hangs)
   - Backpressure to throttle requests

**Exit Criteria:**

- ✅ 20%+ faster than v5.2.0 baseline
- ✅ Multi-GPU systems show benefit
- ✅ No Explorer hangs under heavy load
- ✅ Performance regression tests pass

---

### P6: FORMAT EXPANSION (2-3 WEEKS)

**Goal:** Reach 50+ formats  
**Depends On:** P3 complete  
**Version:** v5.7.0

#### High-Value Formats

1. **Documents** (Week 1)
   - Better PDF support (multi-page)
   - Office documents (DOCX, XLSX, PPTX)
   - Archive thumbnails (ZIP, RAR, 7Z)

2. **Creative** (Week 2)
   - PSD layers
   - RAW camera formats (CR2, NEF, ARW)
   - 3D models (OBJ, FBX)

3. **Plugin Ecosystem** (Week 3)
   - 3-5 reference plugins
   - Plugin developer docs
   - Community plugin guidelines

**Exit Criteria:**

- ✅ 50+ formats supported
- ✅ 3+ community plugins available
- ✅ Plugin development guide published

---

### P7: ECOSYSTEM TOOLS (2-3 WEEKS)

**Goal:** CLI and automation support  
**Depends On:** P2 complete  
**Version:** v5.8.0

#### Deliverables

1. **CLI Tool** (Week 1-2)

   ```bash
   darkthumbs generate image.psd --size 256
   darkthumbs batch *.jpg --output thumbnails/
   darkthumbs plugin list
   darkthumbs benchmark
   ```

2. **PowerShell Module** (Week 2)

   ```powershell
   Get-Thumbnail -Path image.jpg -Size 512
   Get-DarkThumbsPlugins
   Invoke-DarkThumbsBenchmark
   ```

3. **API Service** (Week 3, optional)
   - REST API on localhost
   - For non-Windows tools

**Exit Criteria:**

- ✅ CLI tool published
- ✅ PowerShell module on PSGallery
- ✅ Automation scenarios validated

---

### P8: v6.0 RELEASE (2-3 WEEKS)

**Goal:** Production-ready major release  
**Depends On:** P1-P7 complete  
**Version:** v6.0.0

#### Release Engineering

1. **Packaging** (Week 1)
   - Code signing certificate
   - MSI/MSIX installers
   - Update mechanism

2. **Documentation** (Week 2)
   - User guide
   - Developer docs
   - Plugin development guide
   - API reference

3. **Validation** (Week 3)
   - Compatibility testing (Win10/Win11)
   - Performance regression tests
   - Security audit
   - Beta testing

**Exit Criteria:**

- ✅ v6.0.0 ships as production release
- ✅ All documentation complete
- ✅ Installer works on fresh systems
- ✅ Public announcement ready

---

## 📅 TIMELINE SUMMARY

| Phase | Duration | Target | Version |
|-------|----------|--------|---------|
| **P0: Fix Build** | 1 week | Jan 14, 2026 | v5.2.1 |
| **P1: Verify Baseline** | 3 days | Jan 17, 2026 | v5.2.1 |
| **P2: Engine Refactor** | 3 weeks | Feb 7, 2026 | v5.3.0 |
| **P3: Plugin System** | 4 weeks | Mar 7, 2026 | v5.4.0 |
| **P4: Manager Integration** | 3 weeks | Mar 28, 2026 | v5.5.0 |
| **P5: Performance** | 3 weeks | Apr 18, 2026 | v5.6.0 |
| **P6: Format Expansion** | 3 weeks | May 9, 2026 | v5.7.0 |
| **P7: Ecosystem Tools** | 3 weeks | May 30, 2026 | v5.8.0 |
| **P8: v6.0 Release** | 3 weeks | Jun 20, 2026 | v6.0.0 |

**Total Timeline:** ~24 weeks (6 months) to v6.0.0

---

## 🎯 SUCCESS METRICS

### Build System (P0)

- All libraries build in <10 minutes
- Zero build warnings
- Automated verification passes

### Engine Quality (P2)

- Engine builds independently
- 50+ unit tests passing
- Performance baseline established

### Plugin Ecosystem (P3-P4)

- 5+ working plugins
- Manager controls actual engine
- End-to-end integration working

### v6.0 Launch (P8)

- 50+ formats supported
- 20%+ faster than v5.2.0
- Clean installer experience
- Complete documentation

---

## ⚠️ DEFERRED TO v7.0+

These features are valuable but not critical for v6.0:

- **Out-of-proc Worker**: Complex IPC, defer to v6.1+
- **AI-based Format Detection**: Optional enhancement, v7.0+
- **AppContainer Sandboxing**: Advanced security, v6.2+
- **Enterprise Policy (ADMX)**: Enterprise focus, v6.4+
- **Public Marketplace**: Needs ecosystem maturity, v7.2+

---

## 📍 CURRENT STATUS (Jan 7, 2026)

**Active Phase:** P0 - Fix Build System  
**Blocking Issue:** External libraries not building  
**Next Milestone:** P0 completion by Jan 14

**Progress:**

- ✅ zlib 1.3.1 built
- ✅ LZ4 1.10.0 built
- ⏳ liblzma - CMake environment issue (manual build needed)
- ⏳ zstd - Incomplete source (re-download needed)
- ⏳ LibWebP - Silent build failure (investigation needed)
- ⏳ Minizip-NG - Blocked by dependencies

**Recommended Action RIGHT NOW:**
Open "x64 Native Tools Command Prompt for VS 2026" and manually build the remaining 4 libraries using commands in P0 section above.

---

## 📚 CONSOLIDATED DOCUMENTATION

**This plan replaces:**

- ❌ All 10 BUILD_*.md files in root (consolidated to docs/BUILD_GUIDE.md)
- ❌ MAKE_IT_GREAT_AGAIN_2026.md (30 sprints, too ambitious)
- ❌ SPRINT11/12/13_COMPLETION_REPORT.md (planning only, no code)
- ❌ Multiple roadmap variants

**Essential docs kept:**

- ✅ README.md (project overview)
- ✅ ROADMAP.md (long-term vision)
- ✅ This file (ACTION_PLAN_2026.md - actionable priorities)
- ✅ docs/BUILD_GUIDE.md (consolidated build instructions)

---

**Status:** ACTIVE  
**Review:** Weekly during P0-P1, bi-weekly after  
**Owner:** DarkThumbs Team
