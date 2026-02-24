# ExplorerLens Test Suite Documentation

## Overview

Comprehensive testing framework for ExplorerLens Windows 11 shell extension. The test suite ensures all core functionality remains stable across code changes, refactoring, and platform migrations.

## Test Structure

```
tests/
├── UnitTests.cpp              # Core logic unit tests (format detection, string utilities)
├── IntegrationTests.cpp       # File system & Windows API integration tests
├── generate_test_data.py      # Test data generator (creates sample archives)
├── build-tests.cmd            # Windows build script (MSVC)
├── run-all-tests.cmd          # Master test runner
└── test_data/                 # Generated test files (CBZ, ZIP, EPUB, etc.)
    ├── test_comic.cbz
    ├── test_archive.zip
    ├── test_ebook.epub
    ├── test_archive.tar
    ├── test_comic.cbt
    └── test_photos.phz
```

## Test Suites

### 1. Unit Tests (`UnitTests.cpp`)

**Purpose:** Validate core logic without external dependencies

**Test Coverage:**
- ✅ **Format Detection Tests** (6 tests, 30+ assertions)
  - Comic archives: CBZ, CBR, CB7, CBT
  - Generic archives: ZIP, RAR, 7Z, TAR
  - Ebooks: EPUB, MOBI, AZW, AZW3, FB2
  - Photo archives: PHZ
  - Unsupported formats
  - Edge cases (case sensitivity, multiple dots, etc.)

- ✅ **Image Format Tests** (6 tests, 40+ assertions)
  - Traditional: BMP, GIF, JPG, JPEG, PNG, TIF, TIFF, ICO
  - Modern (Windows 11): WebP, AVIF, JXL, JXR, HEIF, HEIC
  - Path handling (full paths, UNC paths, relative paths)
  - Non-image file rejection

- ✅ **String Utility Tests** (3 tests, 15+ assertions)
  - Basic string comparison
  - Case-insensitive matching
  - Special characters

- ✅ **Format Coverage Tests** (3 tests, 20+ assertions)
  - All 12 archive formats supported
  - All 16 image formats supported
  - No type collisions

- ✅ **Regression Tests** (4 tests, 25+ assertions)
  - Backward compatibility (original 6 formats)
  - Enhancement validation (6 new formats)

- ✅ **Path Handling Tests** (5 tests, 15+ assertions)
  - Windows paths (C:\, D:\)
  - Network paths (\\\\server\\share)
  - Relative paths (.\\, ..\\)
  - Long paths (260+ characters)
  - Special characters in paths

- ✅ **Boundary & Stress Tests** (5 tests, 20+ assertions)
  - Empty strings
  - Null-terminated strings
  - Very long filenames (500+ chars)
  - Multiple dots
  - Unicode filenames (Japanese, Russian, Arabic)

- ✅ **Integration Validation Tests** (4 tests, 15+ assertions)
  - Real-world comic book names
  - Real-world ebook names
  - Real-world image names
  - Mixed case extensions

**Total:** 36 test cases, 180+ assertions

### 2. Integration Tests (`IntegrationTests.cpp`)

**Purpose:** Validate Windows API integration and file system operations

**Test Coverage:**
- ✅ **File System Tests** (4 tests)
  - File existence checking
  - File readability
  - File size detection
  - Path extension extraction

- ✅ **Archive Validation Tests** (3 tests)
  - ZIP signature validation
  - EPUB structure validation
  - TAR signature validation

- ✅ **Windows API Tests** (3 tests)
  - COM initialization
  - IStream creation and usage
  - Path API functions (PathFindExtension, PathFileExists, PathIsDirectory)

- ✅ **Performance Tests** (3 tests)
  - Large path handling (long directory chains)
  - Memory allocation patterns (1MB+ buffers)
  - Multiple format processing

**Total:** 13 test cases, 40+ assertions

### 3. Test Data Generator (`generate_test_data.py`)

**Purpose:** Create minimal valid test files for all supported formats

**Generated Files:**
- **Images:** 1x1 pixel minimal files (PNG, BMP, GIF, JPEG)
- **Archives:**
  - `test_comic.cbz` - Comic Book ZIP with images
  - `test_archive.zip` - Standard ZIP archive
  - `test_ebook.epub` - Valid EPUB with container.xml and content.opf
  - `test_archive.tar` - TAR archive
  - `test_comic.cbt` - Comic Book TAR
  - `test_photos.phz` - Photo ZIP

**Features:**
- Minimal file sizes (all under 10KB total)
- Valid file signatures
- Proper archive structures
- EPUB follows IDPF specification

## Building & Running Tests

### Prerequisites

- **Windows 10/11**
- **MSVC Compiler** (Visual Studio 18 2026 Build Tools or full VS)
- **Python 3.6+** (for test data generation)

### Quick Start

```cmd
# 1. Generate test data
python tests\generate_test_data.py

# 2. Build and run all tests
cd tests
build-tests.cmd

# Or run individually:

# Build unit tests
cl /std:c++20 /EHsc UnitTests.cpp /link shlwapi.lib /OUT:UnitTests.exe

# Build integration tests
cl /std:c++20 /EHsc IntegrationTests.cpp /link shlwapi.lib ole32.lib /OUT:IntegrationTests.exe

# Run tests
UnitTests.exe
IntegrationTests.exe
```

### Build Output

```
tests\build\
├── UnitTests.exe          # Unit test executable
├── UnitTests.pdb          # Debug symbols
├── IntegrationTests.exe   # Integration test executable
└── IntegrationTests.pdb   # Debug symbols
```

## Test Results Format

### Successful Run
```
=========================================
ExplorerLens Unit Test Suite
Version: 15.0.0 (GPU-Accelerated)
=========================================

[Suite 1/8] Format Detection Tests
Running TestGetLENSType_ComicArchives...
  Testing comic book archive format detection...
  Completed.
...

=========================================
Test Summary
=========================================
Tests Passed: 180
Tests Failed: 0
Total Tests:  180

*** ALL TESTS PASSED ***
=========================================
```

### Failed Run
```
...
Running TestIsImage_ModernFormats...
  Testing modern image format detection (Windows 11)...
FAILED: TestMocks::IsImage(_T("image.webp")) at line 156
  Completed.
...

=========================================
Test Summary
=========================================
Tests Passed: 179
Tests Failed: 1
Total Tests:  180

!!! SOME TESTS FAILED !!!
=========================================
```

## Continuous Integration

### Manual Testing Workflow

1. **Before Code Changes:**
   ```cmd
   # Ensure baseline passes
   cd tests
   build-tests.cmd
   ```

2. **After Code Changes:**
   ```cmd
   # Verify no regressions
   build-tests.cmd
   ```

3. **Before Commit:**
   ```cmd
   # Final validation
   build-tests.cmd
   ```

### Automated Testing (Future)

Can be integrated with:
- **GitHub Actions** - Run on every commit
- **Azure Pipelines** - Run on PR creation
- **AppVeyor** - Windows-specific CI

Example GitHub Actions:
```yaml
name: Tests
on: [push, pull_request]
jobs:
  test:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v2
      - name: Setup MSVC
        uses: microsoft/setup-msbuild@v1
      - name: Generate Test Data
        run: python tests/generate_test_data.py
      - name: Build & Run Tests
        run: tests/build-tests.cmd
```

## Test Coverage

### Current Coverage

| Component | Unit Tests | Integration Tests | Total Coverage |
|-----------|------------|-------------------|----------------|
| Format Detection | ✅ 100% | ✅ Signature validation | **100%** |
| Image Detection | ✅ 100% | ✅ File system ops | **100%** |
| String Utilities | ✅ 100% | ✅ Windows API | **100%** |
| Path Handling | ✅ 100% | ✅ Long paths | **100%** |
| COM Integration | ⚠️ Mocked | ✅ Live tests | **80%** |
| Archive Extraction | ❌ Not tested | ⚠️ Signature only | **20%** |
| Thumbnail Generation | ❌ Not tested | ❌ Not tested | **0%** |

### Coverage Goals

- **Phase 1 (Current):** Core logic - ✅ **100%**
- **Phase 2 (Future):** Archive extraction - ⏳ **Target: 80%**
- **Phase 3 (Future):** Thumbnail rendering - ⏳ **Target: 60%**
- **Phase 4 (Future):** Explorer integration - ⏳ **Target: 40%** (manual)

## Adding New Tests

### Unit Test Template

```cpp
TEST_CASE(TestMyNewFeature) {
    std::cout << "  Testing my new feature..." << std::endl;
    
    // Arrange
    int expected = 42;
    
    // Act
    int actual = MyFunction();
    
    // Assert
    ASSERT_EQUAL(actual, expected);
}

// Register in main():
RUN_TEST(MyTestSuite::TestMyNewFeature);
```

### Integration Test Template

```cpp
INTEGRATION_TEST(TestMyIntegration) {
    std::cout << "  Testing integration..." << std::endl;
    
    // Setup
    CoInitialize(NULL);
    
    // Test
    HRESULT hr = MyWindowsAPICall();
    
    // Verify
    ASSERT_SUCCESS(SUCCEEDED(hr), "API call should succeed");
    
    // Cleanup
    CoUninitialize();
}
```

## Troubleshooting

### Test Data Missing
```
Error: test_data/test_comic.cbz not found
```
**Solution:** Run `python tests\generate_test_data.py`

### Compiler Not Found
```
Error: 'cl.exe' is not recognized
```
**Solution:** Run from Visual Studio Developer Command Prompt or:
```cmd
"C:\Program Files\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64
```

### Link Errors
```
Error: unresolved external symbol PathFindExtensionW
```
**Solution:** Add `shlwapi.lib` to link command:
```cmd
cl ... /link shlwapi.lib
```

### COM Initialization Fails
```
Error: CoInitializeEx failed with 0x80010106
```
**Solution:** COM already initialized (RPC_E_CHANGED_MODE), test should handle this case

## Performance Benchmarks

### Unit Tests
- **Execution Time:** < 1 second
- **Memory Usage:** < 10 MB
- **Disk I/O:** None (pure logic)

### Integration Tests
- **Execution Time:** < 5 seconds
- **Memory Usage:** < 50 MB
- **Disk I/O:** Minimal (reads test files)

### Full Suite
- **Total Time:** < 10 seconds
- **Total Memory:** < 100 MB
- **Build Time:** < 30 seconds (incremental)

## Best Practices

### 1. Test Isolation
- Each test should be independent
- No shared state between tests
- Clean up resources after each test

### 2. Clear Assertions
```cpp
// Bad
ASSERT_TRUE(result);

// Good
ASSERT_SUCCESS(result == expected, "Feature X should return Y when Z");
```

### 3. Comprehensive Edge Cases
- Empty inputs
- Null values
- Maximum values
- Boundary conditions
- Unicode characters

### 4. Regression Protection
- Add test for every bug fix
- Keep original format tests
- Version tests separately

## Future Enhancements

### Planned Test Additions

1. **Archive Extraction Tests**
   - ZIP decompression
   - RAR decompression
   - EPUB parsing
   - Error handling

2. **Thumbnail Generation Tests**
   - Image loading
   - Scaling algorithms
   - Memory management
   - Performance benchmarks

3. **COM Interface Tests**
   - IPersistFile implementation
   - IExtractImage2 implementation
   - IThumbnailProvider implementation
   - IInitializeWithStream implementation

4. **Explorer Integration Tests** (Manual)
   - Shell extension registration
   - Context menu appearance
   - Thumbnail cache behavior
   - Icon overlay rendering

## Contributing

When adding new features:

1. **Write tests first** (TDD approach)
2. **Run existing tests** to ensure no regressions
3. **Add integration tests** for Windows API usage
4. **Update this documentation**
5. **Verify all tests pass** before committing

## License

Same as ExplorerLens project. Tests are MIT-compatible and freely redistributable.

---

**Last Updated:** November 18, 2024  
**Test Suite Version:** 2.0  
**ExplorerLens Version:** 15.0.0 (GPU-Accelerated)

