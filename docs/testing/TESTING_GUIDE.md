# Testing Guide — ExplorerLens v24.1.0

**Version:** 24.1.0 "Altair-R" 
**Framework:** Custom C++ test harness (`TEST()`, `RUN_TEST()`, `ASSERT()` macros) 
**Test Count:** 3,269 unit tests + 5 benchmarks 
**Pass Rate:** 100%

---

## Running Tests

```powershell
# Via CTest (recommended)
ctest --test-dir build -C Release --output-on-failure

# Direct execution
.\build\bin\EngineTests.exe

# Build + test in one command
.\build-scripts\Build-MSVC.ps1 -Test
```

---

## Test Suite Overview

| Test Suite | Tests | Coverage |
|-----------|-------|----------|
| ArchiveDecoder | 12 | ZIP, RAR, 7z, TAR, CBZ/CBR decode |
| ImageDecoder | 15 | JPEG, PNG, BMP, GIF, TIFF |
| WebPDecoder | 6 | WebP lossy/lossless |
| AVIFDecoder | 5 | AVIF + ICC profiles |
| JXLDecoder | 5 | JPEG XL |
| HEIFDecoder | 6 | HEIF/HEIC via libde265 |
| RAWDecoder | 8 | CR2, NEF, ARW, DNG via LibRaw |
| HDRDecoder | 4 | HDR/EXR tone mapping |
| DDSDecoder | 4 | DDS/DXT compressed textures |
| PSDDecoder | 3 | Photoshop composite layer |
| SVGDecoder | 3 | SVG rasterization |
| QOIDecoder | 3 | QOI fast decode |
| ICODecoder | 3 | ICO/CUR multi-resolution |
| TGADecoder | 3 | TGA RLE/uncompressed |
| PPMDecoder | 6 | Netpbm formats |
| VideoDecoder | 8 | MP4, MKV, AVI, MOV, WebM |
| AudioDecoder | 8 | MP3, FLAC, OGG, WAV |
| GPURenderer | 8 | D3D11/D3D12 thumbnail scaling |
| CacheSystem | 6 | Multi-tier cache hit/miss/eviction |
| BuildValidation | 8 | Version, feature flags, compile-time |
| FormatDetection | 15 | Extension + magic byte detection |
| Pipeline | 4 | End-to-end integration |
| Isolation | 6 | Malformed payloads, circuit breaker |

All suites: **✅ Pass**

---

## Benchmarks

| Benchmark | Target | Actual | Status |
|-----------|--------|--------|--------|
| Single Thumbnail | <20ms | 17ms | ✅ |
| Batch Throughput | >200 img/sec | 235 img/sec | ✅ |
| Cache Hit | <5ms | <5ms | ✅ |
| GPU vs CPU | GPU 2x faster | 2.1x | ✅ |
| Memory Peak | <512MB | ~380MB | ✅ |

---

## Format Coverage

| Category | Formats Tested | Decoder |
|----------|---------------|---------|
| Archives | ZIP, RAR, 7z, TAR, GZ, BZ2, XZ, ZST, LZMA | ArchiveDecoder |
| Comics | CBZ, CBR, CB7, CBT | ArchiveDecoder |
| Standard Images | JPEG, PNG, BMP, GIF, TIFF, ICO | ImageDecoder |
| Modern Images | WebP, AVIF, JXL, HEIF, HEIC, QOI | Specialized |
| HDR Images | HDR, EXR, DDS | HDRDecoder/DDSDecoder |
| RAW Photos | CR2, CR3, NEF, ARW, DNG, ORF, RW2 | RAWDecoder |
| Design | PSD, TGA, SVG, PPM | Specialized |
| Video | MP4, MKV, AVI, MOV, WMV, WebM | VideoDecoder |
| Audio | MP3, FLAC, OGG, WAV, OPUS | AudioDecoder |
| Documents | PDF, DOCX, PPTX, XLSX | DocumentDecoder |
| Fonts | TTF, OTF, WOFF, WOFF2 | FontDecoder |
| 3D Models | OBJ, STL, GLTF, FBX | ModelDecoder |
| eBooks | EPUB, MOBI, AZW3, FB2 | EBookDecoder |

---

## Platform Compatibility

| OS | Architecture | GPU | Status |
|----|-------------|-----|--------|
| Windows 11 24H2 | x64 | D3D12 | ✅ Primary |
| Windows 11 23H2 | x64 | D3D11 | ✅ Tested |
| Windows 10 22H2 | x64 | D3D11 | ✅ Tested |
| Windows 10 22H2 | x64 | CPU only | ✅ Fallback |

---

## Build Configurations

| Configuration | Generator | Result | Time |
|--------------|-----------|--------|------|
| Release x64 | Ninja | ✅ 0 errors, 0 warnings | ~55s |
| Release x64 | MSBuild | ✅ 0 errors, 0 warnings | ~75s |
| Debug x64 | Ninja | ✅ 0 errors | ~45s |

---

## Adding New Tests

The project uses custom test macros, NOT Google Test.

```cpp
// In Engine/Tests/EngineTests.cpp

TEST(MyNewFeatureTest) {
 // Arrange
 auto component = CreateMyComponent();
 
 // Act
 auto result = component->Process();
 
 // Assert
 ASSERT(result != nullptr);
 ASSERT(result->IsValid());
}

// Add to main():
RUN_TEST(MyNewFeatureTest);
```

**Registration requirements:**
1. New test source files must be registered in `Engine/Tests/CMakeLists.txt`
2. New headers must be registered in `Engine/CMakeLists.txt` ENGINE_HEADERS
3. Tests use `g_testsRun`, `g_testsPassed`, `g_testsFailed` counters

---

## Test Data

| Location | Contents |
|----------|----------|
| `tests/test-images/` | JPEG, PNG, WebP, AVIF test images |
| `test-archives/` | ZIP, CBZ, TAR test archives |
| `data/corpus/` | Extended test corpus |

---

## Performance Thresholds

| Metric | Pass | Warning | Fail |
|--------|------|---------|------|
| Image decode (256px) | <100ms | 100–200ms | >200ms |
| Archive extract | <200ms | 200–500ms | >500ms |
| GPU init | <50ms | 50–100ms | >100ms |

---

## Pre-Deployment Checklist

- [ ] All ~1,187 tests pass (`ctest --test-dir build -C Release`)
- [ ] Zero compiler warnings (`0 warnings` in build output)
- [ ] All 5 benchmarks within targets
- [ ] No memory leaks (stable VRAM/RAM under stress)
- [ ] GPU fallback works (disable GPU, verify CPU path)
- [ ] Test with 1000+ thumbnails consecutively (stress test)

---

*Consolidates TESTING_GUIDE.md, TEST_VALIDATION_CHECKLIST.md, and INTEGRATION_TEST_MATRIX.md.*
