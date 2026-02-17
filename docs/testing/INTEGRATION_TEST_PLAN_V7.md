# Integration Testing Plan - DarkThumbs v7.0.0

**Created:** February 16, 2026  
**Test Scope:** Full system validation of 24 decoders  
**Estimated Duration:** 2-3 hours (complete suite)

---

## Test Strategy

### Test Levels
1. **Unit Tests** - Individual decoder logic (Engine/Tests/)
2. **Integration Tests** - Decoder → Pipeline → Output chain
3. **System Tests** - Shell extension → Explorer integration
4. **Performance Tests** - Throughput and memory benchmarks
5. **Stress Tests** - High-load scenarios (1000+ files)

### Test Coverage Goals
- ✅ **Code Coverage:** 80%+ (decoders, pipeline, cache)
- ✅ **Format Coverage:** 100% (all 80+ extensions)
- ✅ **Platform Coverage:** Windows 10 22H2+, Windows 11 24H2
- ✅ **GPU Coverage:** Intel, AMD, NVIDIA (DirectX 11)

---

## Phase 8A: Unit Tests

### Location
```
Engine/Tests/
├── EngineTests.cpp           # Core engine tests
├── IntegrationTests.cpp      # Pipeline integration
├── PerformanceTests.cpp      # Benchmark suite
└── test_heif_jxl_decoders.cpp # Format-specific tests
```

### Run Unit Tests
```powershell
# Build tests
cmake --build build --config Release --target EngineTests

# Run all tests
.\build\bin\Release\EngineTests.exe --gtest_output=xml:test-results.xml

# Run specific test suite
.\build\bin\Release\EngineTests.exe --gtest_filter=DecoderTests.*
```

### Test Cases (Sample)

#### ImageDecoder Tests
```cpp
TEST(ImageDecoderTests, DecodeJPEG) {
    ImageDecoder decoder;
    ThumbnailRequest request;
    request.filePath = L"test-archives/images/sample.jpg";
    request.width = 256;
    request.height = 256;
    
    ThumbnailResult result;
    HRESULT hr = decoder.Decode(request, result);
    
    EXPECT_EQ(hr, S_OK);
    EXPECT_NE(result.hBitmap, nullptr);
    EXPECT_GT(result.width, 0);
    EXPECT_GT(result.height, 0);
    
    if (result.hBitmap) DeleteObject(result.hBitmap);
}
```

#### VideoDecoder Tests
```cpp
TEST(VideoDecoderTests, ExtractMP4Frame) {
    VideoDecoder decoder;
    EXPECT_TRUE(decoder.CanDecode(L"sample.mp4"));
    
    ThumbnailRequest request;
    request.filePath = L"test-archives/video/sample.mp4";
    request.width = 512;
    request.height = 512;
    
    ThumbnailResult result;
    HRESULT hr = decoder.Decode(request, result);
    
    EXPECT_EQ(hr, S_OK);
    EXPECT_NE(result.hBitmap, nullptr);
}
```

### Expected Results
- **Total Tests:** 150+ test cases
- **Pass Rate:** 100% (all tests must pass)
- **Duration:** ~30 seconds (full suite)

---

## Phase 8B: Integration Tests

### Test Script
```powershell
# scripts/test/Test-DarkThumbs.ps1
# Automated integration testing

.\scripts\test\Test-DarkThumbs.ps1 -All -Report
```

### Test Scenarios

#### 1. Format Validation Test
**Purpose:** Verify all 80+ extensions are recognized

```powershell
$testExtensions = @(
    ".jpg", ".png", ".webp", ".avif", ".jxl",      # Standard images
    ".heif", ".heic",                               # iPhone photos
    ".cr2", ".nef", ".arw", ".dng", ".raf",        # RAW photos
    ".zip", ".rar", ".7z", ".cbz", ".cbr",         # Archives
    ".mp4", ".mkv", ".avi", ".webm", ".mov",       # Video
    ".mp3", ".flac", ".m4a", ".ogg",               # Audio
    ".pdf", ".svg", ".qoi",                         # Documents/Vector
    ".obj", ".stl", ".gltf",                        # 3D models
    ".ttf", ".otf"                                  # Fonts
)

foreach ($ext in $testExtensions) {
    $decoder = Find-Decoder -Extension $ext
    if ($decoder) {
        Write-Pass "$ext → $($decoder.Name)"
    } else {
        Write-Fail "$ext → NO DECODER"
    }
}
```

**Expected:** 80+ extensions mapped to 24 decoders

#### 2. Thumbnail Generation Test
**Purpose:** Generate thumbnails for all format samples

```powershell
.\scripts\test\Test-DarkThumbs.ps1 -InputPath "test-archives" -Recursive -OutputPath "test-output"
```

**Expected Output:**
```
test-output/
├── images/
│   ├── sample.jpg.png      # Generated thumbnail
│   ├── sample.png.png
│   └── sample.webp.png
├── video/
│   ├── sample.mp4.png
│   └── movie.mkv.png
├── archives/
│   ├── photos.zip.png      # First image from archive
│   └── comic.cbz.png
└── test-report.html        # Visual comparison report
```

#### 3. Shell Extension Integration Test
**Purpose:** Verify Explorer integration

```powershell
# Verify COM registration
$clsid = "{9E6ECB90-5A61-42BD-B851-D3297D9C7F39}"
$regPath = "HKCR:\CLSID\$clsid\InprocServer32"

if (Test-Path $regPath) {
    $dllPath = (Get-ItemProperty $regPath).'(default)'
    Write-Pass "DLL registered: $dllPath"
    
    # Verify DLL exists and is valid
    if (Test-Path $dllPath) {
        $version = (Get-Item $dllPath).VersionInfo
        Write-Pass "Version: $($version.FileVersion)"
    }
}

# Trigger Explorer thumbnail refresh
Stop-Process -Name "explorer" -Force
Start-Sleep -Seconds 2
Start-Process "explorer.exe"
```

#### 4. Cache Performance Test
**Purpose:** Verify caching improves performance

```powershell
# First run (cache miss)
Measure-Command {
    Get-ChildItem "test-archives\images" -Filter *.jpg | 
        ForEach-Object { Get-ThumbnailImage $_.FullName }
} | Select-Object TotalMilliseconds

# Second run (cache hit) - should be 90%+ faster
Measure-Command {
    Get-ChildItem "test-archives\images" -Filter *.jpg | 
        ForEach-Object { Get-ThumbnailImage $_.FullName }
} | Select-Object TotalMilliseconds
```

**Expected:**
- Cache miss: ~500ms (10 images)
- Cache hit: ~50ms (10 images) - 10x faster

---

## Phase 8C: System Tests

### Manual Test Plan

#### Test 1: Windows Explorer Integration
1. Open Windows Explorer
2. Navigate to `test-archives\images`
3. Switch to "Large Icons" or "Extra Large Icons" view
4. **Expected:** Thumbnails appear for all image formats within 2 seconds

#### Test 2: Right-Click Context Menu
1. Right-click any supported file (e.g., `.webp`, `.cbz`, `.mp4`)
2. Select "Properties"
3. Check "Details" tab
4. **Expected:** No errors, file opens normally

#### Test 3: Multi-Format Folder
1. Create folder with mix of:
   - 10 JPEG images
   - 5 WebP images
   - 3 AVIF images
   - 2 ZIP archives with images
   - 1 MP4 video
2. Open in Explorer (Extra Large Icons)
3. **Expected:** All thumbnails generate correctly, no crashes

#### Test 4: Large File Handling
1. Copy 5GB video file to test folder
2. Open folder in Explorer
3. **Expected:** 
   - Thumbnail generates (may take 3-5 seconds)
   - No freezing or crashes
   - Progress indicator appears if > 5 second decode

#### Test 5: GPU Acceleration
1. Open Task Manager → Performance → GPU
2. Open folder with 100+ images in Explorer
3. Monitor GPU usage during thumbnail generation
4. **Expected:** 
   - GPU usage increases to 20-40%
   - CPU usage stays < 50%
   - Confirms hardware acceleration active

---

## Phase 8D: Stress Tests

### 1. High-Volume Test
**Test:** 10,000 mixed format files

```powershell
# Generate test files
.\scripts\test\Generate-TestFiles.ps1 -Count 10000 -OutputPath "stress-test"

# Open in Explorer
explorer.exe "stress-test"
```

**Acceptance Criteria:**
- ✅ No crashes or hangs
- ✅ Memory usage < 2GB
- ✅ Thumbnail generation completes within 5 minutes
- ✅ Explorer remains responsive

### 2. Memory Leak Test
**Test:** Continuous thumbnail generation for 1 hour

```powershell
$stopwatch = [System.Diagnostics.Stopwatch]::StartNew()
$iterations = 0

while ($stopwatch.Elapsed.TotalMinutes -lt 60) {
    Get-ChildItem "test-archives" -Recurse -File | 
        Get-Random -Count 100 | 
        ForEach-Object { Get-ThumbnailImage $_.FullName }
    
    $iterations++
    $memoryMB = (Get-Process -Name "dllhost" | 
        Measure-Object -Property WorkingSet64 -Sum).Sum / 1MB
    
    Write-Host "Iteration $iterations - Memory: $([math]::Round($memoryMB, 2)) MB"
    
    if ($memoryMB -gt 500) {
        Write-Error "Memory leak detected! Usage: $memoryMB MB"
        break
    }
}
```

**Acceptance Criteria:**
- ✅ Memory growth < 10MB per 1000 thumbnails
- ✅ No process crashes
- ✅ Thumbnail quality remains consistent

### 3. Concurrent Access Test
**Test:** Multiple Explorer windows accessing same files

```powershell
# Open 5 Explorer windows simultaneously
1..5 | ForEach-Object {
    Start-Process "explorer.exe" -ArgumentList "test-archives\images"
    Start-Sleep -Milliseconds 500
}
```

**Acceptance Criteria:**
- ✅ All windows generate thumbnails correctly
- ✅ No file locking errors
- ✅ Cache sharing works (second window faster)

---

## Phase 8E: Negative Tests

### Error Handling Tests

#### 1. Corrupted File Test
```powershell
# Create corrupted files
Copy-Item "test-archives\images\sample.jpg" "test-corrupted.jpg"
# Overwrite first 100 bytes with zeros
$bytes = [byte[]](0) * 100
[System.IO.File]::WriteAllBytes("test-corrupted.jpg", $bytes)

# Attempt thumbnail generation
# Expected: Graceful failure, no crash, generic icon shown
```

#### 2. Missing File Test
```powershell
# Request thumbnail for non-existent file
Get-ThumbnailImage "C:\nonexistent\file.jpg"

# Expected: ERROR_FILE_NOT_FOUND, no crash
```

#### 3. Permission Denied Test
```powershell
# Create file with no read permissions
$acl = Get-Acl "test-locked.jpg"
$acl.SetAccessRuleProtection($true, $false)
Set-Acl "test-locked.jpg" $acl

# Expected: ERROR_ACCESS_DENIED, fallback to default icon
```

#### 4. Network Path Test
```powershell
# Test with UNC path
Get-ThumbnailImage "\\server\share\image.jpg"

# Expected: Works but slower, network timeout handled
```

#### 5. Unicode Filename Test
```powershell
# Create files with Unicode names
$unicodeFiles = @(
    "测试图片.jpg",
    "Тест.png",
    "テスト.webp",
    "🎨📷.avif"
)

# Expected: All files generate thumbnails correctly
```

---

## Automated Test Execution

### Full Test Suite
```powershell
# Run all automated tests
.\build-scripts\validation\Run-All-Tests.ps1 -Report -OutputPath "test-results"
```

**Generates:**
```
test-results/
├── unit-tests.xml          # Google Test XML
├── integration-tests.html  # HTML report with screenshots
├── performance-report.csv  # Benchmark data
├── coverage-report.html    # Code coverage (lcov)
└── summary.json            # Machine-readable summary
```

### Continuous Integration
```yaml
# .github/workflows/tests.yml
name: Integration Tests

on: [push, pull_request]

jobs:
  test:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v4
      - name: Build
        run: .\RUN-BUILD.bat
      - name: Run Tests
        run: .\build-scripts\validation\Run-All-Tests.ps1
      - name: Upload Results
        uses: actions/upload-artifact@v4
        with:
          name: test-results
          path: test-results/
```

---

## Test Reporting

### Test Summary Template
```markdown
# Test Execution Report - DarkThumbs v7.0.0

**Date:** 2026-02-16  
**Tester:** Automated CI  
**Duration:** 2h 34m

## Results

### Unit Tests
- **Total:** 152 tests
- **Passed:** 152 ✅
- **Failed:** 0
- **Skipped:** 0
- **Coverage:** 84.3%

### Integration Tests
- **Formats Tested:** 80
- **Decoders Tested:** 24
- **Success Rate:** 98.7% (79/80)
- **Failed:** HEIF (libheif not linked)

### Performance Tests
- **Avg Decode Time:** 28ms
- **Cache Hit Rate:** 96.4%
- **Memory Usage:** 180MB (peak)
- **GPU Utilization:** 32%

### System Tests
- **Explorer Integration:** ✅ Pass
- **Shell Extension:** ✅ Pass
- **Large Files:** ✅ Pass
- **Unicode Paths:** ✅ Pass

### Stress Tests
- **10K Files:** ✅ Pass (4m 23s)
- **Memory Leak:** ✅ Pass (< 5MB growth)
- **Concurrent Access:** ✅ Pass

## Issues Found

1. **HEIF Decoder** - Requires libheif library (not built)
   - Severity: Medium
   - Status: Known limitation

2. **OpenEXR** - Requires WIC codec
   - Severity: Low
   - Status: Optional dependency

## Recommendations

- Build libheif library to enable iPhone photo support
- Add more video format test samples (AV1, VP9)
- Consider PDFium integration for native PDF rendering

## Sign-Off

✅ **Approved for Release**

All critical tests passed. Known limitations documented.
```

---

## Test Maintenance

### Adding New Tests

1. **Unit Test:**
   ```cpp
   // Engine/Tests/DecoderTests.cpp
   TEST(MyDecoderTests, TestCase) {
       MyDecoder decoder;
       EXPECT_TRUE(decoder.CanDecode(L"file.ext"));
   }
   ```

2. **Integration Test:**
   ```powershell
   # scripts/test/Test-MyFormat.ps1
   Describe "MyFormat Tests" {
       It "Should decode valid file" {
           $result = Get-Thumbnail "sample.ext"
           $result.Success | Should -Be $true
       }
   }
   ```

3. **Update Test Matrix:**
   - Add sample file to `test-archives/`
   - Update `FORMAT_SUPPORT_MATRIX_V7.md`
   - Re-run `.\scripts\test\Verify-AllDecoders.ps1`

---

## Success Criteria Summary

### Must Pass (Critical)
- ✅ 100% unit test pass rate
- ✅ All 24 decoders compile and link
- ✅ Shell extension registers successfully
- ✅ No crashes with valid files
- ✅ Memory usage < 500MB under normal load

### Should Pass (Important)
- ✅ 95%+ integration test pass rate
- ✅ Cache hit rate > 90%
- ✅ GPU acceleration functional
- ✅ Graceful handling of corrupted files
- ✅ Unicode path support

### Nice to Have (Optional)
- ⚠️ HEIF/HEIC support (requires libheif)
- ⚠️ OpenEXR support (requires WIC codec)
- ⚠️ 3D model loading < 1 second (complex scenes may timeout)

---

**Test Plan Version:** 1.0  
**Last Updated:** February 16, 2026  
**Next Review:** March 2026 (v7.1 release)
