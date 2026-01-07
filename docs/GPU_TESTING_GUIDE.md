# DarkThumbs GPU Testing Guide

**Version:** 5.2.0 Phase 2  
**Date:** November 24, 2025  
**Status:** Testing Tools Ready

---

## Overview

This guide covers testing and benchmarking the GPU-accelerated thumbnail generation in DarkThumbs v5.2.0.

**GPU Vendor-Specific Guides:**
- **[Intel GPU Optimization Guide](INTEL_GPU_GUIDE.md)** - Intel HD Graphics, Iris, Xe, Arc (2nd gen+ / 2011+)
- NVIDIA GPU Guide (coming soon)
- AMD GPU Guide (coming soon)

## Test Tools

### 1. GPUThumbnailTest.exe

**Purpose:** Validate GPU thumbnail generation functionality and quality.

**Features:**
- Loads test images from a folder
- Generates thumbnails using CPU baseline (WIC Fant scaler)
- Saves output thumbnails for visual inspection
- Reports timing and success/failure for each file
- Supports JPEG, PNG, WebP, BMP, TIFF formats

**Usage:**
```cmd
GPUThumbnailTest.exe [options]

Options:
  -i <folder>    Input folder with test images (required)
  -o <folder>    Output folder for thumbnails (optional)
  -s <size>      Thumbnail size in pixels (default: 256)
  -v             Verbose output
  -h, --help     Show help
```

**Examples:**
```cmd
# Basic test - generate thumbnails from test images
GPUThumbnailTest.exe -i C:\TestImages -o C:\Thumbnails

# Specify custom size and verbose output
GPUThumbnailTest.exe -i C:\TestImages -o C:\Thumbnails -s 512 -v

# Test without saving outputs (timing only)
GPUThumbnailTest.exe -i C:\TestImages -s 256
```

**Output:**
```
=== GPU Thumbnail Test Suite ===
Input folder: C:\TestImages
Output folder: C:\Thumbnails
Thumbnail size: 256px

Testing: sample1.jpg ... OK (256x192, 45.2 ms)
Testing: sample2.png ... OK (256x256, 78.5 ms)
Testing: sample3.webp ... OK (256x144, 52.1 ms)

=== Test Summary ===
Total tests: 3
Passed: 3
Failed: 0
Average time: 58.6 ms
Total time: 175.8 ms
```

### 2. CBXBench.exe

**Purpose:** Performance benchmarking and validation of 6.5x speedup target.

**Features:**
- Measures thumbnail generation performance
- Runs multiple iterations per file for statistical accuracy
- Calculates min/max/avg/stddev timing
- Groups results by format (JPEG, PNG, WebP, etc.)
- Exports detailed CSV results

**Usage:**
```cmd
CBXBench.exe [options]

Options:
  -i <folder>    Input folder with test images (required)
  -o <file>      Output CSV file (optional)
  -s <size>      Thumbnail size in pixels (default: 256)
  -n <count>     Iterations per file (default: 10)
  -v             Verbose output
  -h, --help     Show help
```

**Examples:**
```cmd
# Basic benchmark - 10 iterations per file
CBXBench.exe -i C:\TestImages -o results.csv

# High-precision benchmark - 50 iterations
CBXBench.exe -i C:\TestImages -o results.csv -n 50

# Quick benchmark - 5 iterations, verbose
CBXBench.exe -i C:\TestImages -o results.csv -n 5 -v
```

**Output:**
```
=== CBXShell Performance Benchmark ===
Test folder: C:\TestImages
Thumbnail size: 256px
Iterations per file: 10

[1] Benchmarking: test1.jpg ... 42.5 ms (min: 40.2, max: 45.8)
[2] Benchmarking: test2.png ... 75.3 ms (min: 72.1, max: 79.5)
[3] Benchmarking: test3.webp ... 48.9 ms (min: 46.5, max: 52.3)

=== Format Summary ===
Format          Files      Avg Time (ms)
----------------------------------------
JPEG            10         43.2
PNG             5          76.8
WebP            3          49.5

Results saved to: results.csv
```

**CSV Output Format:**
```csv
Filename,Format,SourceWidth,SourceHeight,AvgTime(ms),MinTime(ms),MaxTime(ms),StdDev(ms),Samples
test1.jpg,JPEG,3840,2160,42.500,40.200,45.800,1.750,10
test2.png,PNG,1920,1080,75.300,72.100,79.500,2.340,10
test3.webp,WebP,2560,1440,48.900,46.500,52.300,1.890,10
```

---

## Automated Testing

### PowerShell Test Runner

**run-tests.ps1** - Automated build and test execution

**Usage:**
```powershell
# Build only
.\run-tests.ps1 -BuildOnly

# Build and run tests
.\run-tests.ps1 -TestFolder C:\TestImages

# Full test suite with output folder
.\run-tests.ps1 -TestFolder C:\TestImages -OutputFolder C:\Thumbnails

# High-precision benchmarking
.\run-tests.ps1 -TestFolder C:\TestImages -Iterations 50 -Verbose

# Quick validation
.\run-tests.ps1 -TestFolder C:\TestImages -Iterations 5
```

**Parameters:**
- `-TestFolder <path>` - Folder containing test images (required)
- `-OutputFolder <path>` - Output folder for thumbnails (optional)
- `-ThumbnailSize <pixels>` - Thumbnail size (default: 256)
- `-Iterations <count>` - Benchmark iterations (default: 10)
- `-BuildOnly` - Only build tools, don't run tests
- `-Verbose` - Verbose output

---

## Test Image Preparation

### Recommended Test Set

Create a test folder with diverse images:

**JPEG (10+ files):**
- Small: 640x480
- Medium: 1920x1080
- Large: 3840x2160 (4K)
- Ultra: 7680x4320 (8K)

**PNG (5+ files):**
- Simple (no transparency)
- Complex (transparency + alpha)
- Indexed color
- Grayscale

**WebP (3+ files):**
- Lossy
- Lossless
- Animated

**AVIF (3+ files):**
- Standard
- HDR
- 10-bit color

**HEIF/HEIC (3+ files):**
- iPhone photos
- HDR
- Burst shots

**PDF (2+ files):**
- Simple document
- Complex graphics

**Video (5+ files):**
- MP4 (H.264)
- MKV (H.265/HEVC)
- AVI
- MOV
- WebM

### Download Test Images

Use free stock photo sites:
- **Unsplash** - https://unsplash.com (JPEG, high quality)
- **Pexels** - https://pexels.com (JPEG, video)
- **Pixabay** - https://pixabay.com (PNG, JPEG)
- **Sample Videos** - https://sample-videos.com (MP4, AVI, MOV)

---

## Performance Validation

### Expected Results (v5.2.0 Phase 2)

Based on 6.5x average speedup target:

| Format | Baseline (CPU) | Target (GPU) | Speedup |
|--------|----------------|--------------|---------|
| JPEG 4K | 100 ms | 15 ms | 6.7x |
| PNG Complex | 150 ms | 30 ms | 5.0x |
| WebP Animated | 80 ms | 11 ms | 7.3x |
| AVIF HDR | 120 ms | 20 ms | 6.0x |
| HEIF/HEIC | 110 ms | 18 ms | 6.1x |
| PDF Rasterized | 200 ms | 44 ms | 4.5x |
| Video Frame | 90 ms | 11 ms | 8.2x |

**Average Speedup:** 6.5x

### Validation Criteria

✅ **Pass:** Average speedup ≥ 6.0x  
⚠️ **Warning:** Average speedup 4.0x - 6.0x (acceptable, investigate)  
❌ **Fail:** Average speedup < 4.0x (needs optimization)

---

## GPU Detection Testing

### Test Scenarios

1. **Intel Integrated GPU**
   - Intel HD Graphics 3000+
   - Intel Iris Xe
   - Expected: Hardware GPU, compute shader active

2. **NVIDIA Discrete GPU**
   - GeForce GTX 400+
   - RTX series
   - Expected: Best performance, full feature support

3. **AMD Discrete GPU**
   - Radeon HD 5000+
   - RX series
   - Expected: Hardware GPU, validate vendor-specific behavior

4. **WARP Software Renderer**
   - Disable GPU in Device Manager
   - Expected: WARP fallback, slower but functional

5. **Multi-GPU Systems**
   - Intel integrated + NVIDIA/AMD discrete
   - Expected: Uses discrete GPU (higher performance)

### DebugView Monitoring

**Download:** https://learn.microsoft.com/sysinternals/downloads/debugview

**Expected Debug Output:**
```
[GPU] Initializing GPU accelerator...
[GPU] Device created successfully (Hardware)
[GPU] GPU: Intel(R) Iris(R) Xe Graphics
[GPU] Feature Level: DirectX 11.1
[GPU] VRAM: 512 MB dedicated, 8192 MB shared
[GPU] Vendor: Intel (0x8086)
[GPU] Lanczos3 compute shader compiled successfully
[GPU] GPU acceleration ready
```

**Error Scenarios:**
```
[GPU] Failed to create D3D11 device, trying WARP...
[GPU] Device created successfully (WARP)
[GPU] Using software renderer (CPU emulation)
```

---

## Quality Validation

### Visual Inspection

Compare GPU-generated thumbnails to CPU baseline:

1. **Color Accuracy**
   - Check for color shifts (gamma correction working?)
   - Verify sRGB conversion (no washed-out colors)

2. **Sharpness**
   - Lanczos3 should preserve detail better than Fant
   - Check for blurriness or over-sharpening

3. **Artifacts**
   - No ringing/halos around edges
   - No banding in gradients
   - No color fringing

4. **Transparency**
   - PNG alpha channels preserved
   - WebP transparency correct

### Automated Quality Metrics (Future)

Planned for v5.2.1:
- PSNR (Peak Signal-to-Noise Ratio)
- SSIM (Structural Similarity Index)
- MSE (Mean Squared Error)

---

## Troubleshooting

### Build Errors

**Error: MSBuild not found**
```
Solution: Install Visual Studio Build Tools 2022
Download: https://visualstudio.microsoft.com/downloads/
```

**Error: C++ compiler not found**
```
Solution: Install "Desktop development with C++" workload
Run: Visual Studio Installer → Modify → Check "Desktop development with C++"
```

### Runtime Errors

**Error: Failed to initialize tester**
```
Solution: Install Windows Imaging Component (WIC)
Usually pre-installed on Windows 10/11
Check: Windows Features → Enable .NET Framework 3.5
```

**Error: No GPU detected**
```
Solution 1: Update GPU drivers
Solution 2: Check Device Manager → Display adapters
Solution 3: WARP fallback should work (software renderer)
```

**Error: Shader compilation failed**
```
Solution 1: Check for d3dcompiler_47.dll in System32
Solution 2: Install DirectX End-User Runtime
Solution 3: Automatic WIC fallback should activate
```

---

## Performance Tips

### Optimal Test Conditions

1. **Close Background Applications**
   - Web browsers (GPU-intensive)
   - Video players
   - Games

2. **Disable GPU Power Saving**
   - Windows Power Plan: High Performance
   - NVIDIA Control Panel: Prefer Maximum Performance
   - AMD Radeon Settings: High Performance

3. **Use SSD Storage**
   - Test images on SSD (not HDD)
   - Reduces I/O overhead in benchmarks

4. **Sufficient Iterations**
   - Minimum: 10 iterations per file
   - Recommended: 20-50 iterations
   - High precision: 100+ iterations

### Benchmark Best Practices

1. **Warmup Run**
   - First run may be slower (shader compilation)
   - Discard first iteration results
   - Use subsequent runs for measurement

2. **Consistent Environment**
   - Same test images across runs
   - Same system state (no updates running)
   - Same GPU power mode

3. **Statistical Validation**
   - Check standard deviation (should be low)
   - High StdDev = inconsistent performance
   - Filter outliers (>2σ from mean)

---

## Next Steps

After running tests:

1. **Review Results**
   - Check CSV for anomalies
   - Identify slow formats
   - Calculate overall speedup

2. **Visual Validation**
   - Compare thumbnails to originals
   - Check for quality issues
   - Verify color accuracy

3. **Document Findings**
   - Update V5.2.0_SPRINT_SUMMARY.md
   - Add benchmark results table
   - Note any issues found

4. **Optimize if Needed**
   - Texture pooling (if memory overhead high)
   - Shader bytecode embedding (if compile time high)
   - Multi-resolution optimization

---

## Support

**DebugView Logs:** Enable OutputDebugString capture  
**GPU Profilers:** NSight (NVIDIA), Radeon GPU Profiler (AMD), Intel GPA  
**GitHub Issues:** Report bugs with benchmark CSV attached

---

**Last Updated:** November 24, 2025  
**Tools Version:** 1.0.0  
**Target:** DarkThumbs v5.2.0 Phase 2
