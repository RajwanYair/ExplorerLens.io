# DarkThumbs - 25 Sprint Task Plan
**Created:** February 15, 2026  
**Goal:** Production-ready build with all features, proper library linkage, and complete test coverage

---

## Priority Classification
- **P0 (Critical):** Blocks production deployment
- **P1 (High):** Core functionality gaps
- **P2 (Medium):** Quality & performance improvements
- **P3 (Low):** Nice-to-have enhancements

---

## Sprint Tasks (Execution Order)

### **Sprint 1: External Libraries - Rebuild all with /MD flag** [P0]
**Goal:** Eliminate LIBCMT conflicts by rebuilding all external libraries with dynamic CRT  
**Tasks:**
1.1. Create master rebuild script `build-scripts/Rebuild-All-With-MD.ps1`  
1.2. Rebuild zlib 1.3.1 with `/MD`  
1.3. Rebuild zstd 1.5.7 with `/MD`  
1.4. Rebuild lz4 1.10.0 with `/MD`  
1.5. Rebuild minizip-ng 4.0.10 with `/MD`  
1.6. Rebuild lzma 25.00 with `/MD`  
1.7. Rebuild xz 5.6.3 with `/MD`  
1.8. Rebuild bzip2 1.0.8 with `/MD`  
1.9. Rebuild libwebp 1.5.0 with `/MD`  
1.10. Rebuild libavif 1.3.0 + dav1d 1.5.1 with `/MD`  
1.11. Update Engine CMakeLists.txt to remove LIBCMT workaround  
1.12. Verify clean build with zero linker warnings

**Estimated Time:** 4-6 hours  
**Dependencies:** CMake, Ninja, Visual Studio Build Tools  
**Success Criteria:** Build succeeds without `/NODEFAULTLIB:LIBCMT`

---

### **Sprint 2: JPEG XL Integration** [P1]
**Goal:** Enable JPEG XL (.jxl) thumbnail support  
**Tasks:**
2.1. Build libjxl 0.11.1 with brotli and highway dependencies  
2.2. Copy libs to `SDK/lib/` and headers to `SDK/include/jxl/`  
2.3. Enable `HAS_LIBJXL=ON` in Engine CMakeLists.txt  
2.4. Link `jxl.lib`, `jxl_threads.lib`, `brotlienc.lib`, `brotlidec.lib`, `hwy.lib`  
2.5. Implement `JXLDecoder::Decode()` using libjxl C API  
2.6. Handle 8-bit, 16-bit, and animated JXL (first frame only)  
2.7. Add ICC profile support via `JxlDecoderGetColorAsEncodedProfile`  
2.8. Add 50 MP decode limit for memory safety  
2.9. Create unit tests for JXL decoder (5+ test files)  
2.10. Update Shell registration to unbrick .jxl extension

**Estimated Time:** 1-2 days  
**Dependencies:** Sprint 1 (clean library build)  
**Success Criteria:** .jxl files show thumbnails in Explorer

---

### **Sprint 3: HEIF/HEIC Integration** [P1]
**Goal:** Enable HEIF/HEIC (.heif, .heic) thumbnail support for iPhone photos  
**Tasks:**
3.1. Build libheif 1.19+ with libde265 (HEVC decoder)  
3.2. Copy libs to `SDK/lib/` and headers to `SDK/include/libheif/`  
3.3. Enable `HAS_LIBHEIF=ON` in Engine CMakeLists.txt  
3.4. Link `heif.lib` and `de265.lib`  
3.5. Implement `HEIFDecoder::Decode()` using libheif C API  
3.6. Prefer embedded thumbnails via `heif_image_handle_get_thumbnail()`  
3.7. Handle multi-image containers (use primary image)  
3.8. Handle EXIF orientation flags  
3.9. Test with iPhone HEIC photos (portrait, HDR, live photos)  
3.10. Update Shell registration for .heic/.heif

**Estimated Time:** 1-2 days  
**Dependencies:** Sprint 1  
**Success Criteria:** iPhone photos show correct thumbnails

---

### **Sprint 4: SVG Rendering (lunasvg)** [P1]
**Goal:** Replace SVG placeholder with real vector rendering  
**Tasks:**
4.1. Integrate lunasvg library (MIT license, header-only)  
4.2. Add lunasvg to `external/image-libs/lunasvg/`  
4.3. Update Engine CMakeLists.txt to include lunasvg sources  
4.4. Replace `SVGDecoder.cpp` stub with `lunasvg::Document::loadFromFile()`  
4.5. Implement `document->renderToBitmap(width, height)` → BGRA conversion  
4.6. Handle SVGZ (gzip-compressed) via zlib inflate  
4.7. Add viewBox scaling and aspect ratio preservation  
4.8. Test with complex SVGs (gradients, filters, text, CSS)  
4.9. Performance: ensure SVG decode < 30ms for typical icons  
4.10. Handle error cases (malformed SVG, missing fonts)

**Estimated Time:** 1 day  
**Dependencies:** Sprint 1 (zlib)  
**Success Criteria:** SVG files render correctly with colors and shapes

---

### **Sprint 5: PDF Thumbnail Provider Integration** [P1]
**Goal:** Enable PDF thumbnails via Windows PDF renderer or MuPDF  
**Tasks:**
5.1. Test Windows.Data.Pdf API (Windows 10+) for PDF rendering  
5.2. If Windows.Data.Pdf works, implement via WinRT C++/WinRT  
5.3. Otherwise, integrate MuPDF 1.24.11 from external/pdf-libs/  
5.4. Build MuPDF with MSVC (use provided NMake scripts)  
5.5. Implement `PDFDecoder::Decode()` → render first page only  
5.6. Set DPI to 150 for thumbnail clarity  
5.7. Handle encrypted PDFs gracefully (return placeholder)  
5.8. Handle large PDFs (>500 pages) without memory issues  
5.9. Test with various PDF versions (1.4, 1.7, 2.0)  
5.10. Performance target: <100ms for first page render

**Estimated Time:** 1-2 days  
**Dependencies:** Sprint 1  
**Success Criteria:** PDF files show first page as thumbnail

---

### **Sprint 6: Video Decoder Robustness** [P2]
**Goal:** Improve video thumbnail reliability and format coverage  
**Tasks:**
6.1. Audit VideoDecoder for Media Foundation edge cases  
6.2. Handle videos without keyframes at timestamp 0  
6.3. Implement intelligent keyframe search (first 5 seconds)  
6.4. Add hardware decode acceleration flags (DXVA2)  
6.5. Handle corrupt/truncated video files gracefully  
6.6. Add support for VP9, AV1 codecs (if Media Foundation supports)  
6.7. Test with problematic formats (variable framerate, b-frames)  
6.8. Add video metadata extraction (resolution, duration, codec)  
6.9. Performance: ensure video decode < 150ms (hardware path)  
6.10. Add unit tests for 15+ video formats

**Estimated Time:** 1 day  
**Dependencies:** None  
**Success Criteria:** All common video formats work reliably

---

### **Sprint 7: Audio Album Art Extraction** [P2]
**Goal:** Improve audio thumbnail quality and format coverage  
**Tasks:**
7.1. Enhance ID3v2 parser for MP3 (APIC frames)  
7.2. Add FLAC Vorbis comment parsing for cover art  
7.3. Add OGG Vorbis METADATA_BLOCK_PICTURE support  
7.4. Add WMA/ASF file parsing for WM/Picture attribute  
7.5. Add AAC/M4A iTunes metadata parsing  
7.6. Add AIFF/WAV RIFF chunk parsing for artwork  
7.7. Handle embedded PNG, BMP, GIF (in addition to JPEG)  
7.8. Generate waveform visualization if no album art found  
7.9. Add audio metadata extraction (artist, album, duration)  
7.10. Test with 10+ audio formats

**Estimated Time:** 1 day  
**Dependencies:** None  
**Success Criteria:** Most audio files show album art or waveform

---

### **Sprint 8: Document Thumbnail Provider** [P2]
**Goal:** Enable Office document thumbnails via Windows Property System  
**Tasks:**
8.1. Test Windows thumbnail cache for .docx/.xlsx/.pptx  
8.2. Use `IShellItemImageFactory` to request system thumbnails  
8.3. If system provider fails, extract embedded thumbnails from OOXML  
8.4. Parse OOXML ZIP structure (`docProps/thumbnail.jpeg`)  
8.5. Handle Office 2003 binary formats (.doc, .xls, .ppt) if possible  
8.6. Add LibreOffice format support (.odt, .ods, .odp)  
8.7. Add RTF thumbnail (render first page as text)  
8.8. Handle password-protected documents gracefully  
8.9. Test with real-world Office files  
8.10. Performance target: <50ms (prefer system thumbnails)

**Estimated Time:** 1 day  
**Dependencies:** None  
**Success Criteria:** Office documents show first page or icon

---

### **Sprint 9: Font Preview Rendering** [P2]
**Goal:** Show actual font glyphs instead of generic icon  
**Tasks:**
9.1. Use DirectWrite `IDWriteFontFile` API  
9.2. Load TTF/OTF/WOFF/WOFF2 via `CreateFontFaceFromFontFile`  
9.3. Render sample text "Aa Bb Cc 123" onto offscreen bitmap  
9.4. Use font's actual style (serif, sans, script, display)  
9.5. Handle non-Latin scripts (show appropriate sample glyphs)  
9.6. Add font metadata extraction (family name, weight, style)  
9.7. Handle installed vs. file fonts correctly  
9.8. Test with Google Fonts collection  
9.9. Performance target: <30ms per font  
9.10. Handle corrupt font files gracefully

**Estimated Time:** 1 day  
**Dependencies:** None  
**Success Criteria:** Font files show recognizable glyph preview

---

### **Sprint 10: Archive Format Expansion** [P2]
**Goal:** Add more archive format support beyond CBZ/CBR/CB7  
**Tasks:**
10.1. Add .tar.gz, .tar.bz2, .tar.xz support via libarchive  
10.2. Add .iso (CD/DVD image) first-file extraction  
10.3. Add .cab (Windows Cabinet) support  
10.4. Add .cpio archive support  
10.5. Add .epub (as ZIP archive with cover.jpg extraction)  
10.6. Add .mobi/.azw Kindle format cover extraction  
10.7. Improve archive first-image search (prefer cover.jpg, 00.jpg)  
10.8. Handle nested archives (archive within archive)  
10.9. Add archive metadata (file count, total size)  
10.10. Performance: ensure archive scan < 100ms

**Estimated Time:** 1 day  
**Dependencies:** Sprint 1 (all compression libs)  
**Success Criteria:** eBook and misc archives show covers

---

### **Sprint 11: RAW Format Expansion** [P2]
**Goal:** Add more camera RAW formats and improve quality  
**Tasks:**
11.1. Test WIC RAW codec pack coverage (current: 24 formats)  
11.2. Add missing Canon formats (.cr3 support verification)  
11.3. Add missing Sony formats (.arw A7 IV verification)  
11.4. Add Fujifilm X-Trans RAW (.raf) verification  
11.5. Add Panasonic .rw2 verification  
11.6. Add Olympus .orf verification  
11.7. Implement EXIF thumbnail extraction for speed  
11.8. Add auto white balance correction option  
11.9. Add RAW metadata extraction (camera model, ISO, exposure)  
11.10. Performance: ensure RAW decode < 50ms (via embedded thumbnail)

**Estimated Time:** 1 day  
**Dependencies:** None (WIC RAW codec built-in)  
**Success Criteria:** All major camera brands work

---

### **Sprint 12: 3D Model Thumbnail Support** [P3]
**Goal:** Enable 3D format thumbnails (.obj, .fbx, .gltf, .stl)  
**Tasks:**
12.1. Research Windows 3D thumbnail provider APIs  
12.2. Test `Microsoft.UI.Composition.Interactions` for 3D rendering  
12.3. Integrate Assimp library for 3D model loading  
12.4. Build Assimp as static lib with `/MD`  
12.5. Implement `Model3DDecoder::Decode()` → render front view  
12.6. Set up basic lighting and camera for thumbnail  
12.7. Support .obj, .fbx, .gltf, .glb, .stl, .dae, .3ds  
12.8. Handle textures if present (load first texture only)  
12.9. Add bounding box calculation for auto-framing  
12.10. Performance target: <200ms per model

**Estimated Time:** 2 days  
**Dependencies:** Sprint 1  
**Success Criteria:** 3D models show recognizable shape

---

### **Sprint 13: Performance Profiling & Optimization** [P2]
**Goal:** Measure and optimize all decoder performance  
**Tasks:**
13.1. Create comprehensive benchmark suite for all decoders  
13.2. Test each decoder with 100+ sample files  
13.3. Measure p50, p95, p99 decode times  
13.4. Identify slowest decoders (>100ms p95)  
13.5. Profile with Visual Studio profiler  
13.6. Optimize hot paths with SIMD (SSE4.1/AVX2)  
13.7. Add GPU batch processing path for expensive decoders  
13.8. Implement adaptive quality (lower quality for large files)  
13.9. Add decode timeout (250ms max) to prevent hangs  
13.10. Document performance characteristics per decoder

**Estimated Time:** 2 days  
**Dependencies:** All decoder sprints  
**Success Criteria:** 95% of thumbnails generate in <100ms

---

### **Sprint 14: Memory Leak Detection & Fixing** [P0]
**Goal:** Ensure zero memory leaks in production  
**Tasks:**
14.1. Enable CRT debug heap (`_CRTDBG_MAP_ALLOC`) in Debug builds  
14.2. Run all unit tests under CRT leak detection  
14.3. Fix any detected leaks  
14.4. Test with Application Verifier  
14.5. Add RAII wrappers for all COM objects  
14.6. Add RAII wrappers for all Windows handles (HBITMAP, HICON)  
14.7. Audit all `new` calls for matching `delete`  
14.8. Use smart pointers (`unique_ptr`, `shared_ptr`) everywhere  
14.9. Test long-running scenarios (1000+ thumbnail generations)  
14.10. Add continuous memory monitoring in CBXManager

**Estimated Time:** 1-2 days  
**Dependencies:** None  
**Success Criteria:** Zero leaks in 10,000 thumbnail test

---

### **Sprint 15: Unit Test Expansion** [P1]
**Goal:** Increase test coverage from 70 to 150+ tests  
**Tasks:**
15.1. Add tests for PSDDecoder (5 files: layers, 16-bit, CMYK)  
15.2. Add tests for DDSDecoder (5 files: DXT1, BC7, cubemap)  
15.3. Add tests for HDRDecoder (5 files: Radiance RGBE)  
15.4. Add tests for PPMDecoder (5 files: P1-P7 formats)  
15.5. Add tests for EXRDecoder (5 files: float, half, layers)  
15.6. Add tests for SVGDecoder (10 files: gradients, filters, text)  
15.7. Add tests for VideoDecoder (15 files: all codecs)  
15.8. Add tests for AudioDecoder (10 files: all formats)  
15.9. Add tests for archive formats (10 files: all archives)  
15.10. Add negative tests (corrupt files, unsupported formats)

**Estimated Time:** 2 days  
**Dependencies:** Sprints 2-12 (decoder implementations)  
**Success Criteria:** 150+ tests, all passing

---

### **Sprint 16: Integration Testing** [P1]
**Goal:** Test actual Explorer integration end-to-end  
**Tasks:**
16.1. Create PowerShell test harness for Explorer simulation  
16.2. Register CBXShell.dll in test environment  
16.3. Use `IExtractImage2::GetLocation()` + `::Extract()` APIs  
16.4. Test all 55 registered extensions  
16.5. Verify thumbnail appears in Explorer (automated screenshot)  
16.6. Test context menu integration  
16.7. Test thumbnail cache invalidation  
16.8. Test multi-monitor DPI scaling  
16.9. Test Windows 10 vs. Windows 11 behavior  
16.10. Create regression test suite for CI/CD

**Estimated Time:** 2 days  
**Dependencies:** Sprint 15  
**Success Criteria:** All 55 extensions work in Explorer

---

### **Sprint 17: CBXManager UI Enhancements** [P2]
**Goal:** Improve configuration tool UX and features  
**Tasks:**
17.1. Add decoder enable/disable toggles (persist to registry)  
17.2. Add performance metrics display (avg decode time per format)  
17.3. Add thumbnail preview panel (test any file)  
17.4. Add cache management (clear cache, show size)  
17.5. Add log viewer for troubleshooting  
17.6. Add extension registration status checker  
17.7. Add "Repair Installation" button (re-register DLL)  
17.8. Add "Check for Updates" functionality  
17.9. Improve dark mode integration (all controls)  
17.10. Add tooltips and help text throughout UI

**Estimated Time:** 2 days  
**Dependencies:** None  
**Success Criteria:** CBXManager is user-friendly and polished

---

### **Sprint 18: WinUI 3 Manager (Optional)** [P3]
**Goal:** Modern XAML UI as alternative to WTL  
**Tasks:**
18.1. Create new WinUI 3 project in `src/Manager.WinUI/`  
18.2. Implement MainWindow with NavigationView  
18.3. Add Settings page with toggle switches for decoders  
18.4. Add Performance page with charts (WinUI DataChart)  
18.5. Add About page with hardware detection  
18.6. Add Test page with file picker and preview  
18.7. Implement dark/light theme switching  
18.8. Add Fluent Design (Acrylic, animations)  
18.9. Package as MSIX for Store distribution  
18.10. Add side-by-side installation with WTL version

**Estimated Time:** 3-4 days  
**Dependencies:** None (separate binary)  
**Success Criteria:** Modern UI alternative available

---

### **Sprint 19: Plugin System Activation** [P1]
**Goal:** Enable and test plugin loading infrastructure  
**Tasks:**
19.1. Uncomment `LoadPlugins()` in initialization code  
19.2. Implement plugin discovery (scan `Plugins/` directory)  
19.3. Implement plugin validation (version check, signature)  
19.4. Implement shared memory IPC for plugin communication  
19.5. Implement plugin sandbox (separate process isolation)  
19.6. Create sample plugin (BMP decoder as reference)  
19.7. Document plugin SDK API  
19.8. Add plugin crash handling (don't crash Explorer)  
19.9. Add plugin update mechanism  
19.10. Test with 3rd party plugin DLL

**Estimated Time:** 2-3 days  
**Dependencies:** Sprint 14 (stability)  
**Success Criteria:** Sample plugin works in isolated process

---

### **Sprint 20: GPU Acceleration Enhancement** [P2]
**Goal:** Maximize GPU usage for thumbnail generation  
**Tasks:**
20.1. Implement GPU batch resize shader (process 16 at once)  
20.2. Use D3D11 compute shader for color space conversion  
20.3. Add GPU-accelerated tone mapping (HDR → SDR)  
20.4. Implement async GPU rendering (overlap with CPU decode)  
20.5. Add multi-GPU support (use IGP for thumbnails)  
20.6. Profile GPU utilization (aim for >50% usage)  
20.7. Add fallback to CPU if GPU unavailable  
20.8. Test with Intel, NVIDIA, AMD GPUs  
20.9. Measure power consumption (prefer integrated GPU)  
20.10. Document GPU requirements in README

**Estimated Time:** 2 days  
**Dependencies:** Sprint 13 (profiling)  
**Success Criteria:** GPU utilization >50% during batch operations

---

### **Sprint 21: Cache System Optimization** [P2]
**Goal:** Improve thumbnail cache performance and reliability  
**Tasks:**
21.1. Implement cache pre-warming (background thread)  
21.2. Add LRU eviction policy (keep 10,000 most recent)  
21.3. Add disk space monitoring (cap at 1GB)  
21.4. Implement cache integrity verification (CRC32 checksums)  
21.5. Add cache statistics tracking (hit rate, size)  
21.6. Implement cache compression (zstd level 3)  
21.7. Add cache migration for version upgrades  
21.8. Implement cache sharing across user sessions  
21.9. Add cache export/import for troubleshooting  
21.10. Document cache file format

**Estimated Time:** 1-2 days  
**Dependencies:** Sprint 1 (zstd)  
**Success Criteria:** Cache hit rate >80% for repeated access

---

### **Sprint 22: Error Handling Robustness** [P0]
**Goal:** Ensure Explorer never crashes due to thumbnail generation  
**Tasks:**
22.1. Wrap all decoder entry points with SEH (`__try`/`__except`)  
22.2. Add timeout mechanism (250ms max per thumbnail)  
22.3. Implement circuit breaker (disable decoder after 5 failures)  
22.4. Add comprehensive error logging (Windows Event Log)  
22.5. Implement telemetry collection (opt-in, privacy-safe)  
22.6. Add crash dump generation for debugging  
22.7. Test with intentionally corrupt files  
22.8. Test with malicious/fuzzing files  
22.9. Add ASAN/UBSAN builds for continuous testing  
22.10. Document error codes and troubleshooting

**Estimated Time:** 2 days  
**Dependencies:** Sprint 14 (memory safety)  
**Success Criteria:** Zero crashes with 10,000 random files

---

### **Sprint 23: WiX Installer Creation** [P1]
**Goal:** Professional MSI installer for easy deployment  
**Tasks:**
23.1. Create WiX v5 project in `packaging/wix/`  
23.2. Define product information (name, version, GUID)  
23.3. Add file components (CBXShell.dll, CBXManager.exe, etc.)  
23.4. Implement COM registration (regsvr32 equivalent)  
23.5. Add file type associations in MSI  
23.6. Create Start Menu shortcuts  
23.7. Implement upgrade logic (handle existing installations)  
23.8. Add custom actions (clear thumbnail cache on install)  
23.9. Create uninstaller  
23.10. Test install/upgrade/uninstall scenarios

**Estimated Time:** 2 days  
**Dependencies:** Sprint 22 (stability)  
**Success Criteria:** MSI installs cleanly on fresh Windows

---

### **Sprint 24: Code Signing & Release Automation** [P1]
**Goal:** Set up automated release pipeline  
**Tasks:**
24.1. Obtain Authenticode code signing certificate  
24.2. Configure `signtool.exe` for automated signing  
24.3. Sign CBXShell.dll, CBXManager.exe, MSI installer  
24.4. Create release build script (build + test + sign + package)  
24.5. Generate SHA256 checksums for all artifacts  
24.6. Create GitHub Releases workflow  
24.7. Add release notes generation (CHANGELOG.md automation)  
24.8. Implement version bumping strategy (semantic versioning)  
24.9. Add pre-release vs. stable release channels  
24.10. Document release process in CONTRIBUTING.md

**Estimated Time:** 1-2 days  
**Dependencies:** Sprint 23  
**Success Criteria:** One-click release from main branch

---

### **Sprint 25: Documentation & Final Polish** [P1]
**Goal:** Complete all documentation for v6.2.0 release  
**Tasks:**
25.1. Update README.md with all supported formats (155+)  
25.2. Create USER_GUIDE.md for end users  
25.3. Create DEVELOPER_GUIDE.md for contributors  
25.4. Update BUILD_METHOD.md with final tool paths  
25.5. Create PLUGIN_SDK.md for plugin developers  
25.6. Add screenshots to docs/ (Explorer thumbnails, CBXManager UI)  
25.7. Create VIDEO_DEMO (screen recording of features)  
25.8. Update CHANGELOG.md with v6.2.0 release notes  
25.9. Create KNOWN_ISSUES.md with troubleshooting  
25.10. Final code cleanup (remove dead code, TODOs, comments)

**Estimated Time:** 1-2 days  
**Dependencies:** All sprints  
**Success Criteria:** Documentation is complete and accurate

---

## Estimated Total Time
- P0 (Critical): ~12-16 hours
- P1 (High): ~20-25 days
- P2 (Medium): ~15-18 days
- P3 (Low): ~5-7 days
- **Total: 40-50 days for complete implementation**

## Success Metrics
- ✅ Zero build warnings/errors
- ✅ Zero memory leaks
- ✅ Zero crashes with 10,000 test files
- ✅ 155+ file formats supported
- ✅ 95% of thumbnails <100ms
- ✅ 150+ unit tests passing
- ✅ 55 Explorer extensions working
- ✅ Clean MSI installation
- ✅ Code signed and released

---

**Status Tracking:** See `SPRINT_PROGRESS.md` for real-time status updates.
