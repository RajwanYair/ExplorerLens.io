# P2: Engine Refactoring - Implementation Plan

**Phase:** Sprint 11 - Platform Foundation  
**Version:** v5.3.0  
**Duration:** 3-4 weeks  
**Status:** 🎯 Ready to Start  
**Date:** January 7, 2026

---

## Overview

Extract the thumbnail generation engine from the COM shell extension into a standalone, reusable library with clean interfaces. This enables future plugin support, testing, and multi-application usage.

---

## Prerequisites ✅

Before starting P2, ensure P1 is complete:

- ✅ v5.2.0 production baseline verified
- ✅ All libraries built successfully (zlib, zstd, lz4, lzma, libwebp, minizip-ng)
- ✅ CBXShell.dll compiles cleanly (0 warnings, 0 errors)
- ✅ GPU acceleration operational (DirectX 11)
- ✅ Test results documented

**Status:** ✅ All prerequisites met (January 7, 2026)

---

## Phase Breakdown

### Week 1: Interface Definition & Structure

**Days 1-2: Directory Structure & Core Interfaces**

1. Create `Engine/` directory structure:
   ```
   Engine/
   ├── Core/
   │   ├── IThumbnailDecoder.h
   │   ├── ICacheProvider.h
   │   ├── IGPURenderer.h
   │   ├── IFormatDetector.h
   │   └── Types.h              # Common types
   ├── Decoders/
   ├── Pipeline/
   ├── CMakeLists.txt
   └── Engine.h                 # Public API
   ```

2. Define `IThumbnailDecoder` interface:
   ```cpp
   class IThumbnailDecoder {
   public:
       virtual ~IThumbnailDecoder() = default;
       
       virtual bool CanDecode(const wchar_t* filePath) = 0;
       virtual HRESULT Decode(const ThumbnailRequest& request, 
                             ThumbnailResult& result) = 0;
       virtual const wchar_t* GetName() = 0;
       virtual const wchar_t** GetSupportedExtensions() = 0;
   };
   ```

3. Define `ThumbnailRequest` and `ThumbnailResult` structures:
   ```cpp
   struct ThumbnailRequest {
       const wchar_t* filePath;
       uint32_t width;
       uint32_t height;
       uint32_t flags;           // Fast mode, GPU acceleration, etc.
   };
   
   struct ThumbnailResult {
       HBITMAP hBitmap;
       uint32_t width;
       uint32_t height;
       HRESULT status;
   };
   ```

**Days 3-5: Format Detection & Decoder Registry**

4. Implement `IFormatDetector`:
   ```cpp
   class IFormatDetector {
   public:
       virtual ~IFormatDetector() = default;
       
       virtual FormatType DetectFormat(const wchar_t* filePath) = 0;
       virtual bool IsImageFormat(const wchar_t* extension) = 0;
       virtual bool IsArchiveFormat(const wchar_t* extension) = 0;
   };
   ```

5. Create decoder registry:
   ```cpp
   class DecoderRegistry {
   public:
       void RegisterDecoder(IThumbnailDecoder* decoder);
       IThumbnailDecoder* FindDecoder(const wchar_t* filePath);
       size_t GetDecoderCount();
   };
   ```

**Deliverables:**
- ✅ `Engine/` directory created
- ✅ Core interfaces defined (4 interfaces)
- ✅ Request/Result structures defined
- ✅ Decoder registry implemented

---

### Week 2: Decoder Extraction

**Days 6-8: Image Decoder Extraction**

6. Create `Engine/Decoders/ImageDecoder.h/cpp`:
   - Extract GDI+ image loading from CBXShell
   - Support: PNG, JPEG, BMP, GIF, TIFF, ICO
   - Implement IThumbnailDecoder interface

7. Create `Engine/Decoders/WebPDecoder.h/cpp`:
   - Extract WebP decoding logic
   - Use libwebp 1.5.0
   - Implement IThumbnailDecoder interface

8. Create `Engine/Decoders/AVIFDecoder.h/cpp`:
   - Extract AVIF decoding (from avif_decoder.cpp)
   - Implement IThumbnailDecoder interface

**Days 9-10: Archive Decoder Extraction**

9. Create `Engine/Decoders/ArchiveDecoder.h/cpp`:
   - Extract ZIP/CBZ logic (from unzip_new.cpp)
   - Extract RAR/CBR logic
   - Extract 7z/CB7 logic
   - Implement IThumbnailDecoder interface
   - Handle "extract first image" logic

**Deliverables:**
- ✅ ImageDecoder implemented (core formats)
- ✅ WebPDecoder implemented
- ✅ AVIFDecoder implemented
- ✅ ArchiveDecoder implemented (ZIP, RAR, 7z)

---

### Week 3: GPU Pipeline & Integration

**Days 11-13: GPU Renderer Abstraction**

10. Create `Engine/Core/IGPURenderer.h`:
    ```cpp
    class IGPURenderer {
    public:
        virtual ~IGPURenderer() = default;
        
        virtual bool Initialize() = 0;
        virtual HRESULT RenderThumbnail(
            const uint8_t* imageData, 
            uint32_t width, uint32_t height,
            uint32_t thumbWidth, uint32_t thumbHeight,
            HBITMAP* outBitmap) = 0;
        virtual void Shutdown() = 0;
    };
    ```

11. Extract DirectX 11 GPU rendering:
    - Create `Engine/GPU/D3D11Renderer.h/cpp`
    - Move GPU initialization from CBXShell
    - Move shader compilation
    - Move texture pooling
    - Implement IGPURenderer interface

12. Create CPU fallback renderer:
    - `Engine/GPU/CPURenderer.h/cpp`
    - GDI+ based scaling
    - Implement IGPURenderer interface

**Days 14-15: Pipeline Assembly**

13. Create `Engine/Pipeline/ThumbnailPipeline.h/cpp`:
    ```cpp
    class ThumbnailPipeline {
    public:
        ThumbnailPipeline();
        
        void RegisterDecoder(IThumbnailDecoder* decoder);
        void SetGPURenderer(IGPURenderer* renderer);
        void SetFormatDetector(IFormatDetector* detector);
        
        HRESULT GenerateThumbnail(
            const ThumbnailRequest& request,
            ThumbnailResult& result);
    };
    ```

14. Implement pipeline logic:
    - Format detection
    - Decoder selection
    - Decoding
    - GPU rendering (if enabled)
    - Error handling

**Deliverables:**
- ✅ IGPURenderer interface defined
- ✅ D3D11Renderer extracted from CBXShell
- ✅ CPURenderer implemented
- ✅ ThumbnailPipeline implemented
- ✅ Pipeline orchestration working

---

### Week 4: Integration & Testing

**Days 16-18: CBXShell Integration**

15. Update CBXShell to use Engine:
    - Remove duplicate decoder code
    - Create ThumbnailPipeline instance
    - Register all decoders
    - Forward IThumbnailProvider::GetThumbnail() to pipeline
    - Preserve COM interface

16. Update project files:
    - Add Engine.lib to CBXShell linker dependencies
    - Update include paths
    - Ensure clean build

17. Test integration:
    - Verify all formats still work
    - Check GPU acceleration still functional
    - Validate performance (should be identical)

**Days 19-21: Unit Testing**

18. Create `Engine/Tests/` directory:
    ```
    Engine/Tests/
    ├── FormatDetectionTests.cpp
    ├── DecoderTests.cpp
    ├── PipelineTests.cpp
    └── CMakeLists.txt
    ```

19. Implement test suites:
    - **Format Detection Tests** (15 tests)
      - Extension detection (.jpg, .png, .cbz, etc.)
      - File signature detection
      - Edge cases (no extension, multiple dots)
    
    - **Decoder Tests** (20 tests)
      - Image decoding (valid files)
      - Archive extraction (first image)
      - Error handling (corrupt files)
      - Memory leak detection
    
    - **Pipeline Tests** (15 tests)
      - End-to-end thumbnail generation
      - Decoder selection logic
      - GPU vs CPU rendering
      - Performance benchmarks

20. Run test suites:
    ```powershell
    cd Engine/Tests
    cmake -B build -G "Visual Studio 17 2026"
    cmake --build build --config Release
    cd build/Release
    .\EngineTests.exe
    ```

**Target:** 50+ tests, 100% pass rate

**Deliverables:**
- ✅ CBXShell fully integrated with Engine
- ✅ All formats working via Engine
- ✅ Unit test suite with 50+ tests
- ✅ All tests passing
- ✅ No regressions from v5.2.0

---

## Exit Criteria Checklist

### Code Quality ✅

- [ ] Engine builds independently of CBXShell
- [ ] Zero warnings in Release build
- [ ] Zero errors in Release build
- [ ] CMake build working for Engine
- [ ] All interfaces documented

### Functionality ✅

- [ ] All v5.2.0 formats still work
- [ ] GPU acceleration functional
- [ ] Performance equivalent to v5.2.0
- [ ] No Explorer crashes
- [ ] Memory leaks checked with Valgrind/Dr. Memory

### Testing ✅

- [ ] 50+ unit tests written
- [ ] 100% test pass rate
- [ ] Integration tests pass
- [ ] Manual Explorer testing complete

### Documentation ✅

- [ ] Interface documentation (Doxygen)
- [ ] Architecture diagram
- [ ] Migration guide for CBXShell
- [ ] Developer guide for Engine usage

---

## Success Metrics

| Metric | v5.2.0 Baseline | v5.3.0 Target | Status |
|--------|-----------------|---------------|--------|
| **Formats Supported** | 31+ | 31+ | - |
| **GPU Performance** | 13-61ms | 13-61ms | - |
| **Build Warnings** | 0 | 0 | - |
| **Test Coverage** | Unit tests only | 50+ tests | - |
| **Code Modularity** | Monolithic | Separated | - |

---

## Risk Management

### Technical Risks

1. **Performance Regression**
   - **Risk:** Engine abstraction adds overhead
   - **Mitigation:** Profile before/after, optimize hot paths
   - **Threshold:** <5% performance impact acceptable

2. **COM Interface Complexity**
   - **Risk:** CBXShell COM integration breaks
   - **Mitigation:** Preserve exact external interface, test thoroughly
   - **Fallback:** Keep v5.2.0 as rollback point

3. **Memory Management**
   - **Risk:** Interface boundaries cause leaks
   - **Mitigation:** Use smart pointers, run leak detection
   - **Tool:** Dr. Memory or Application Verifier

### Schedule Risks

1. **Decoder Extraction Complexity**
   - **Risk:** Decoders more coupled than expected
   - **Mitigation:** Start with simplest decoder (ImageDecoder)
   - **Buffer:** Week 4 has testing buffer time

---

## Next Steps After P2

Once P2 (Engine Refactoring) is complete:

1. **P3: Plugin System** (Sprint 12)
   - Use Engine interfaces as plugin API
   - Implement dynamic plugin loading
   - Create sample plugin

2. **P4: Manager Integration** (Sprint 13)
   - Connect CBXManager to Engine
   - Real-time diagnostics
   - Settings management

---

## References

- [PROJECT_STATUS.md](../PROJECT_STATUS.md) - Overall project status
- [ROADMAP.md](../ROADMAP.md) - Long-term development plan
- [tests/TEST_RESULTS_2026-01-07.md](../tests/TEST_RESULTS_2026-01-07.md) - P1 test results
- [docs/architecture/](../docs/architecture/) - Architecture documentation (to be created)

---

**Plan Created:** January 7, 2026  
**Plan Owner:** Development Team  
**Status:** 🎯 Ready to Start
