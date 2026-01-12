# Sprint 11: Platform Foundation - Implementation Plan

**Phase:** Phase 2 - Architecture Modernization  
**Sprint:** 11 (Weeks 5-8)  
**Start Date:** January 12, 2026  
**Status:** IN PROGRESS  
**Goal:** Extract thumbnail engine from COM shell extension, establish plugin foundation

---

## Current Architecture Status

### Existing Engine Infrastructure ✅

The Engine foundation has already been established with the following components:

#### Core Interfaces (Completed)
- ✅ `IThumbnailDecoder` - Base decoder interface
- ✅ `IFormatDetector` - Format detection interface
- ✅ `IGPURenderer` - GPU acceleration interface  
- ✅ `ICacheProvider` - Caching interface
- ✅ `Types.h` - Common types (ThumbnailRequest, ThumbnailResult, FormatType, etc.)

#### Existing Decoders
- ✅ `ImageDecoder` - JPEG, PNG, BMP, GIF, TIFF
- ✅ `WebPDecoder` - WebP format support
- ✅ `AVIFDecoder` - AVIF format support
- ✅ `JXLDecoder` - JPEG XL format support
- ✅ `HEIFDecoder` - HEIF/HEIC format support
- ✅ `ArchiveDecoder` - ZIP, RAR, 7Z archives

#### Pipeline Components
- ✅ `ThumbnailPipeline` - Main orchestration
- ✅ `DecoderRegistry` - Decoder management
- ✅ `FormatDetector` - Format detection implementation

#### Integration
- ✅ `EngineAdapter` - Bridge between COM shell and Engine
- ✅ CBXShellClass uses EngineAdapter

---

## Architecture Analysis

### Current Structure

```
┌─────────────────────────────────────────────────────┐
│            Windows Shell (Explorer.exe)             │
└────────────────────┬────────────────────────────────┘
                     │ COM Interface
                     │ (IThumbnailProvider)
                     ↓
┌─────────────────────────────────────────────────────┐
│              CBXShell.dll (COM Extension)           │
│                                                      │
│  ┌──────────────────────────────────────────────┐  │
│  │          CCBXShell (COM Object)              │  │
│  │  • IPersistFile                              │  │
│  │  • IThumbnailProvider                        │  │
│  │  • IExtractImage/IExtractImage2              │  │
│  │  • IInitializeWithStream                     │  │
│  └────────────┬─────────────────────────────────┘  │
│               │                                     │
│               ↓                                     │
│  ┌──────────────────────────────────────────────┐  │
│  │        EngineAdapter (Bridge)                │  │
│  │  • COM → Engine translation                  │  │
│  │  • HBITMAP creation                          │  │
│  └────────────┬─────────────────────────────────┘  │
└───────────────┼─────────────────────────────────────┘
                │
                ↓
┌─────────────────────────────────────────────────────┐
│           DarkThumbs Engine (Library)               │
│                                                      │
│  ┌──────────────────────────────────────────────┐  │
│  │       ThumbnailPipeline                      │  │
│  │  • Request routing                           │  │
│  │  • Decoder selection                         │  │
│  │  • Result aggregation                        │  │
│  └─────┬──────────────────────────────────┬─────┘  │
│        │                                  │         │
│        ↓                                  ↓         │
│  ┌─────────────┐                  ┌──────────────┐ │
│  │   Decoders  │                  │   GPU/Cache  │ │
│  │  • Image    │                  │  • D3D11     │ │
│  │  • WebP     │                  │  • Cache     │ │
│  │  • Archive  │                  │              │ │
│  └─────────────┘                  └──────────────┘ │
└─────────────────────────────────────────────────────┘
```

### Separation of Concerns

| Layer | Responsibility | Status |
|-------|---------------|--------|
| **Shell Extension (CBXShell.dll)** | COM interfaces, Windows integration | ✅ Clean |
| **Adapter (EngineAdapter)** | Translation layer, HBITMAP creation | ✅ Implemented |
| **Engine (Library)** | Core thumbnail logic, decoders | ✅ Mostly complete |
| **Decoders (Plugins)** | Format-specific implementations | ✅ Interface ready |

---

## Sprint 11 Objectives

### Primary Goals

1. **✅ Complete Interface Definitions** (DONE)
   - All core interfaces defined
   - Types and contracts established

2. **🔄 Decoder Implementation Status** (IN PROGRESS)
   - Review existing decoder completeness
   - Ensure all inherit from IThumbnailDecoder
   - Standardize error handling

3. **🔄 Pipeline Validation** (IN PROGRESS)
   - Verify ThumbnailPipeline orchestration
   - Test decoder selection logic
   - Validate result handling

4. **⏳ Build System Integration** (PENDING)
   - Create Engine.lib build target
   - Separate from CBXShell.dll build
   - Enable independent compilation

5. **⏳ Testing Framework** (PENDING)
   - Engine unit tests
   - Decoder validation tests
   - Pipeline integration tests

### Secondary Goals

6. **⏳ Documentation** (PENDING)
   - Engine API documentation
   - Decoder implementation guide
   - Plugin development guide

7. **⏳ Performance Optimization** (PENDING)
   - Benchmark individual decoders
   - Optimize pipeline overhead
   - Memory pool implementation

---

## Implementation Tasks

### Week 5 Tasks (Current Week)

#### Task 1: Engine Build Configuration ⏳
**Priority:** High  
**Effort:** 2-3 hours

**Objectives:**
- Create separate CMakeLists.txt for Engine library
- Define DarkThumbsEngine.lib as static library
- Configure include directories and dependencies
- Ensure independent compilation from CBXShell

**Deliverables:**
- `Engine/CMakeLists.txt` (updated)
- Engine builds as standalone .lib
- No COM dependencies in Engine code

**Acceptance Criteria:**
- Engine.lib builds successfully
- Can link Engine.lib from test project
- Zero COM dependencies in Engine binary

#### Task 2: Decoder Standardization ⏳
**Priority:** High  
**Effort:** 4-5 hours

**Objectives:**
- Audit all existing decoders
- Ensure IThumbnailDecoder interface compliance
- Standardize error codes (E_FAIL → meaningful HRESULTs)
- Add decoder capability reporting

**Decoders to Review:**
- ImageDecoder.cpp
- WebPDecoder.cpp
- AVIFDecoder.cpp
- JXLDecoder.cpp
- ArchiveDecoder.cpp

**Deliverables:**
- All decoders implement GetInfo()
- Consistent error handling
- Unit tests for each decoder

#### Task 3: Pipeline Testing ⏳
**Priority:** Medium  
**Effort:** 3-4 hours

**Objectives:**
- Create Engine test project
- Test decoder registration
- Test format detection
- Test request routing

**Deliverables:**
- Tests/EngineTests.cpp with 20+ test cases
- All tests passing
- Code coverage > 80%

### Week 6 Tasks

#### Task 4: GPU Abstraction Layer
- Abstract D3D11 behind IGPURenderer
- Implement CPU fallback renderer
- Add GPU availability detection

#### Task 5: Cache Integration
- Implement ICacheProvider
- Integrate with ThumbnailPipeline
- Add cache statistics

#### Task 6: EngineAdapter Optimization
- Reduce COM ↔ Engine marshaling overhead
- Optimize HBITMAP creation
- Add error mapping

### Week 7 Tasks

#### Task 7: Plugin Architecture
- Define plugin loading mechanism
- Create sample external decoder plugin
- Test plugin registration and usage

#### Task 8: Performance Profiling
- Benchmark each decoder
- Profile pipeline overhead
- Identify bottlenecks

### Week 8 Tasks

#### Task 9: Documentation
- Engine API reference
- Decoder developer guide
- Architecture diagrams

#### Task 10: Sprint Review
- Demo plugin system
- Performance comparison
- Integration testing

---

## Technical Specifications

### Engine Library Structure

```
DarkThumbsEngine.lib
├── Core/
│   ├── IThumbnailDecoder (interface)
│   ├── IFormatDetector (interface)
│   ├── IGPURenderer (interface)
│   ├── ICacheProvider (interface)
│   └── Types (structures)
├── Pipeline/
│   ├── ThumbnailPipeline (orchestration)
│   ├── DecoderRegistry (management)
│   └── FormatDetector (implementation)
├── Decoders/
│   ├── ImageDecoder (JPEG, PNG, BMP, GIF, TIFF)
│   ├── WebPDecoder (WebP)
│   ├── AVIFDecoder (AVIF)
│   ├── JXLDecoder (JPEG XL)
│   └── ArchiveDecoder (ZIP, RAR, 7Z)
└── GPU/
    ├── D3D11Renderer (DirectX 11)
    └── CPURenderer (fallback)
```

### Dependencies

**Engine Library:**
- Windows SDK (minimal)
- C++20 STL
- DirectX 11 (optional, GPU only)
- Image libraries (libjpeg, libpng, libwebp, etc.)

**No Dependencies:**
- ❌ ATL/COM (Engine is COM-free)
- ❌ MFC
- ❌ Shell APIs (Explorer integration)

### Interface Contracts

**ThumbnailRequest:**
```cpp
struct ThumbnailRequest {
    const wchar_t* filePath;     // Full path to file
    uint32_t requestedWidth;     // Desired width
    uint32_t requestedHeight;    // Desired height
    bool useGPU;                 // Enable GPU acceleration
    bool useCache;               // Enable caching
    void* userData;              // Custom data
};
```

**ThumbnailResult:**
```cpp
struct ThumbnailResult {
    HBITMAP hBitmap;            // Output bitmap
    uint32_t width;             // Actual width
    uint32_t height;            // Actual height
    FormatType format;          // Detected format
    HRESULT status;             // Result code
    double generationTimeMs;    // Timing info
    bool usedGPU;               // GPU was used
    bool usedCache;             // Cache hit
};
```

---

## Current Status Summary

### ✅ Completed
- Core interface definitions
- Basic decoder implementations
- Pipeline structure
- EngineAdapter bridge
- COM integration working

### 🔄 In Progress
- Engine build configuration (separate .lib)
- Decoder standardization
- Comprehensive testing

### ⏳ Pending
- Plugin loading mechanism
- Performance optimization
- Complete documentation
- External plugin samples

---

## Success Criteria

Sprint 11 is successful when:

1. ✅ **Interfaces Complete** - All core interfaces defined and documented
2. ⏳ **Engine Builds Independently** - DarkThumbsEngine.lib compiles without CBXShell
3. ⏳ **All Decoders Standardized** - Consistent IThumbnailDecoder implementation
4. ⏳ **Pipeline Validated** - 20+ unit tests passing
5. ⏳ **GPU Abstraction** - D3D11 + CPU fallback working
6. ⏳ **Plugin Architecture** - External decoder loads and executes
7. ⏳ **Documentation** - API reference and developer guide complete
8. ⏳ **Performance Baseline** - < 50ms average thumbnail generation maintained

---

## Risk Assessment

| Risk | Impact | Mitigation |
|------|--------|------------|
| Breaking CBXShell integration | High | Keep EngineAdapter as stable bridge |
| Performance regression | Medium | Continuous benchmarking |
| Decoder inconsistency | Medium | Strict interface enforcement |
| Build complexity | Low | Clear CMake configuration |
| Testing coverage gaps | Low | TDD approach for new code |

---

## Next Steps (Immediate)

1. **Update Engine CMakeLists.txt** - Configure standalone library build
2. **Review Decoder Implementations** - Audit for interface compliance
3. **Create Engine Test Project** - Set up unit testing framework
4. **Document Current State** - This document + code comments

---

## Related Documentation

- [ROADMAP.md](../ROADMAP.md) - Overall project roadmap
- [Engine/README.md](../Engine/README.md) - Engine architecture overview
- [PRIORITY1_TEST_RESULTS.md](PRIORITY1_TEST_RESULTS.md) - Phase 1 validation results

---

**Document Created:** January 12, 2026  
**Sprint Duration:** 4 weeks  
**Next Review:** End of Week 5 (January 19, 2026)
