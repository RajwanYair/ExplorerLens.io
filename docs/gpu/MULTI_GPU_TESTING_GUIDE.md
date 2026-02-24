# Multi-GPU Testing Guide

**ExplorerLens v5.2.0 - GPU Validation and Testing**

---

## Overview

This guide covers testing ExplorerLens GPU acceleration across different hardware configurations: Intel integrated GPUs, NVIDIA discrete GPUs, AMD GPUs, and WARP software renderer.

---

## Testing Tools

### 1. GPUValidator.exe

**Purpose:** Detect and validate all GPUs in the system.

**Features:**
- Enumerates all DirectX 11 capable GPUs
- Tests WARP software renderer availability
- Validates compute shader support
- Reports ExplorerLens compatibility
- Intel GPU type detection (Arc, Iris Xe, HD, UHD)

**Build:**
```cmd
build-scripts\build-gpu-validator.cmd
```

**Usage:**
```cmd
tests\build\GPUValidator.exe
```

**Example Output:**
```
=================================================
ExplorerLens GPU Validator v5.2.0
Multi-GPU Detection and Validation Tool
=================================================

=================================================
GPU #1: Intel(R) Iris(R) Xe Graphics
=================================================
Vendor:                Intel (0x8086)
Feature Level:         DirectX 11.1
Device Type:           Hardware
Dedicated VRAM:        128 MB
Shared System:         8.0 GB
Compute Shaders:       ✓ Supported
ExplorerLens GPU Mode:   ✓ ENABLED
Intel GPU Type:        Iris Xe (High-Performance)

=================================================
GPU #2: NVIDIA GeForce RTX 3060
=================================================
Vendor:                NVIDIA (0x10de)
Feature Level:         DirectX 12.1
Device Type:           Hardware
Dedicated VRAM:        12.0 GB
Compute Shaders:       ✓ Supported
ExplorerLens GPU Mode:   ✓ ENABLED

=================================================
GPU #3: WARP Software Renderer
=================================================
Vendor:                Microsoft (0x1414)
Feature Level:         DirectX 11.1
Device Type:           Software
Compute Shaders:       ✓ Supported
ExplorerLens GPU Mode:   ✓ ENABLED

=================================================
SUMMARY
=================================================
Total GPUs Found:      2
ExplorerLens Compatible: 2 GPU(s)
Intel GPUs:            1
NVIDIA GPUs:           1
WARP Fallback:         ✓ Available (DirectX 11.1)

✓ ExplorerLens will use GPU acceleration
✓ Expected speedup: 4-10x depending on GPU
```

---

## Test Scenarios

### Scenario 1: Intel Integrated GPU Only (Laptop)

**Hardware:**
- CPU: Intel Core i7-1165G7 (11th gen)
- GPU: Intel Iris Xe Graphics

**Expected Behavior:**
1. GPUValidator detects Intel Iris Xe
2. Feature Level: DirectX 11.1
3. Shared memory: 4-8 GB
4. ExplorerLens uses GPU acceleration
5. Expected speedup: 6-8x vs CPU

**Validation Steps:**
```cmd
# 1. Run GPU validator
tests\build\GPUValidator.exe

# 2. Install ExplorerLens
install-x64-fixed.cmd

# 3. Run DebugView (as Admin)
# Download: https://learn.microsoft.com/en-us/sysinternals/downloads/debugview

# 4. Navigate to folder with images
# Look for DebugView output:
[GPU] Device created successfully (Hardware)
[GPU] Vendor: Intel (0x8086)
[GPU] Intel high-performance GPU detected (Iris/Xe/Arc)
[GPU] Feature Level: DirectX 11.1
[GPU] Compute shader support enabled

# 5. Run benchmark
tests\build\LENSBench.exe -i C:\TestImages -n 100 -s 256

# Expected: 6-8x speedup over CPU
```

### Scenario 2: NVIDIA Discrete GPU (Desktop)

**Hardware:**
- CPU: AMD Ryzen 7 5800X
- GPU: NVIDIA GeForce RTX 3070

**Expected Behavior:**
1. GPUValidator detects NVIDIA RTX 3070
2. Feature Level: DirectX 12.1
3. Dedicated VRAM: 8 GB
4. ExplorerLens uses GPU acceleration
5. Expected speedup: 8-10x vs CPU

**Validation Steps:**
```cmd
# 1. Verify GPU detection
tests\build\GPUValidator.exe

# 2. Check NVIDIA GPU is selected
# DebugView should show:
[GPU] Vendor: NVIDIA (0x10de)
[GPU] Feature Level: DirectX 12.1
[GPU] High performance mode enabled (dedicated GPU)

# 3. Benchmark
tests\build\LENSBench.exe -i C:\TestImages -n 100 -s 256

# Expected: 8-10x speedup
```

### Scenario 3: AMD Discrete GPU

**Hardware:**
- CPU: Intel Core i5-10400
- GPU: AMD Radeon RX 6700 XT

**Expected Behavior:**
1. GPUValidator detects AMD Radeon
2. Feature Level: DirectX 12.0+
3. Dedicated VRAM: 12 GB
4. ExplorerLens uses GPU acceleration
5. Expected speedup: 7-9x vs CPU

**Validation Steps:**
```cmd
# 1. Verify GPU detection
tests\build\GPUValidator.exe

# 2. DebugView output:
[GPU] Vendor: AMD (0x1002)
[GPU] Feature Level: DirectX 12.0
[GPU] High performance mode enabled

# 3. Benchmark
tests\build\LENSBench.exe -i C:\TestImages -n 100 -s 256

# Expected: 7-9x speedup
```

### Scenario 4: Dual GPU (Intel iGPU + NVIDIA dGPU)

**Hardware:**
- CPU: Intel Core i7-12700H (12th gen)
- iGPU: Intel Iris Xe Graphics
- dGPU: NVIDIA GeForce RTX 3060 Laptop

**Expected Behavior:**
1. GPUValidator detects both GPUs
2. Windows automatically selects dGPU for ExplorerLens
3. Feature Level: DirectX 12.1
4. ExplorerLens uses NVIDIA GPU by default
5. Expected speedup: 8-10x vs CPU

**Validation Steps:**
```cmd
# 1. Detect both GPUs
tests\build\GPUValidator.exe

# Expected output:
GPU #1: Intel(R) Iris(R) Xe Graphics
GPU #2: NVIDIA GeForce RTX 3060 Laptop GPU

# 2. DebugView should show NVIDIA GPU selected:
[GPU] Vendor: NVIDIA (0x10de)

# 3. Force Intel iGPU (optional test):
# - Open Intel Graphics Command Center
# - Add LENSShell.dll to app list
# - Set to "Integrated Graphics"
# - DebugView should now show:
[GPU] Vendor: Intel (0x8086)

# 4. Benchmark both GPUs:
# NVIDIA dGPU:
tests\build\LENSBench.exe -i C:\TestImages -n 100 -s 256

# Intel iGPU (after forcing):
tests\build\LENSBench.exe -i C:\TestImages -n 100 -s 256

# Compare results
```

### Scenario 5: Old GPU (DirectX 10.x - CPU Fallback)

**Hardware:**
- GPU: Intel HD Graphics 2000 (1st gen Sandy Bridge)
- Feature Level: DirectX 10.1

**Expected Behavior:**
1. GPUValidator detects GPU but marks as incompatible
2. ExplorerLens falls back to CPU rendering
3. No GPU acceleration (DirectX 11.0 required)

**Validation Steps:**
```cmd
# 1. Verify GPU detected but incompatible
tests\build\GPUValidator.exe

# Expected:
Feature Level:         DirectX 10.1
Compute Shaders:       ✗ Not Supported
ExplorerLens GPU Mode:   ✗ CPU Fallback

# 2. DebugView output:
[GPU] Feature level too low for compute shaders
[GPU] GPU acceleration not available, using CPU fallback

# 3. Thumbnails still work, but no speedup
```

### Scenario 6: No GPU (WARP Renderer)

**Hardware:**
- Virtual Machine or cloud instance
- No physical GPU

**Expected Behavior:**
1. GPUValidator detects WARP software renderer
2. ExplorerLens uses WARP (software emulation)
3. Feature Level: DirectX 11.1
4. Performance similar to CPU fallback

**Validation Steps:**
```cmd
# 1. Verify WARP available
tests\build\GPUValidator.exe

# Expected:
WARP Fallback:         ✓ Available (DirectX 11.1)

# 2. DebugView output:
[GPU] Device created successfully (WARP)
[GPU] Using software renderer

# 3. Benchmark (expect minimal speedup)
tests\build\LENSBench.exe -i C:\TestImages -n 100 -s 256

# Expected: 1.0-1.5x speedup (WARP overhead may be slower)
```

---

## Debug Layer Testing

### Enable DirectX Debug Layer

**Prerequisites:**
- Windows 10 SDK installed
- Graphics Tools feature enabled

**Enable Graphics Tools:**
```powershell
# Windows Settings → Apps → Optional Features → Add a feature
# Search for "Graphics Tools" and install
```

**Enable Debug Layer in Code:**

Modify `gpu_accelerator.cpp` temporarily:

```cpp
// In CreateDevice() method
UINT createDeviceFlags = 0;

#ifdef _DEBUG
createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;  // Enable debug layer
#endif

hr = D3D11CreateDevice(
    adapter.Get(),
    driverType,
    nullptr,
    createDeviceFlags,  // Use debug flags
    featureLevels,
    ...
);
```

**Rebuild:**
```cmd
msbuild LENSShell\LENSShell.vcxproj /p:Configuration=Debug /p:Platform=x64
```

**Run with DebugView:**
```cmd
# Start DebugView as Administrator
# Enable: Capture → Capture Win32

# Install debug build
install-x64-fixed.cmd

# Navigate to images folder
# Check DebugView for debug layer messages:

D3D11 INFO: Device created
D3D11 INFO: CreateTexture2D succeeded
D3D11 WARNING: Possible performance issue detected
D3D11 ERROR: Invalid resource state (if bugs present)
```

**Common Debug Messages:**

✅ **Normal:**
```
D3D11 INFO: ID3D11Device::CreateTexture2D: Texture created
D3D11 INFO: ID3D11DeviceContext::Map: Resource mapped successfully
```

⚠️ **Warnings (review):**
```
D3D11 WARNING: ID3D11DeviceContext::UpdateSubresource: Performance warning
D3D11 WARNING: Resource transition may cause pipeline stall
```

❌ **Errors (must fix):**
```
D3D11 ERROR: ID3D11DeviceContext::Draw: Shader not bound
D3D11 ERROR: Resource still mapped during rendering
```

---

## Performance Testing Matrix

### Test Configuration

**Image Set:**
- 100 JPEG images (4K resolution)
- 100 PNG images (complex transparency)
- 50 WebP images (animated)
- 50 AVIF images (HDR)

**Thumbnail Size:** 256×256

**Metrics:**
- Total time (seconds)
- Avg time per thumbnail (ms)
- GPU utilization (%)
- Peak VRAM usage (MB)
- Texture pool hit rate (%)

### Expected Results by GPU

| GPU Type | Speedup | Total Time | Pool Hit Rate | VRAM |
|----------|---------|------------|---------------|------|
| Intel Arc A770 | 8-10x | 3.5-4.0s | 95%+ | 120 MB |
| NVIDIA RTX 3070 | 8-10x | 3.2-3.8s | 95%+ | 110 MB |
| AMD RX 6700 XT | 7-9x | 3.8-4.5s | 95%+ | 115 MB |
| Intel Iris Xe | 6-8x | 4.5-5.5s | 95%+ | 150 MB |
| Intel UHD 630 | 4-6x | 6.0-8.0s | 95%+ | 180 MB |
| Intel HD 4000 | 3-5x | 8.0-10.0s | 95%+ | 200 MB |
| WARP | 1.0-1.5x | 28-32s | 95%+ | 250 MB |
| CPU (baseline) | 1.0x | 30-35s | N/A | N/A |

---

## Known GPU-Specific Issues

### Intel GPUs

**Issue:** First thumbnail slow on Intel Arc
- **Cause:** Shader compilation on first use
- **Expected:** First call ~500 ms, subsequent ~15 ms
- **Status:** Normal behavior

**Issue:** Intel HD Graphics 2500 slower than HD 4000
- **Cause:** Half the execution units (6 vs 16)
- **Expected:** 3-4x speedup instead of 4-5x
- **Status:** Hardware limitation

### NVIDIA GPUs

**Issue:** GTX 900-series slower than expected
- **Cause:** Older architecture (Maxwell vs Turing/Ampere)
- **Expected:** 5-7x speedup instead of 8-10x
- **Status:** Normal for older GPUs

### AMD GPUs

**Issue:** RX 5000-series variable performance
- **Cause:** Driver version dependent
- **Expected:** Update to Adrenalin 23.0+ for best performance
- **Status:** Update drivers

### WARP

**Issue:** WARP slower than CPU
- **Cause:** Software emulation overhead
- **Expected:** Use CPU fallback if WARP is only option
- **Status:** By design

---

## Reporting Results

### Template for GitHub Issues

```markdown
**GPU Configuration:**
- GPU: [e.g., Intel Iris Xe Graphics]
- Driver Version: [e.g., 31.0.101.4953]
- OS: [e.g., Windows 11 23H2]
- System RAM: [e.g., 16 GB]

**ExplorerLens Version:**
- Version: v5.2.0
- DLL Size: 1,476,096 bytes
- Build Date: November 24, 2025

**GPUValidator Output:**
```
[Paste GPUValidator.exe output]
```

**DebugView Logs:**
```
[Paste relevant ExplorerLens GPU logs]
```

**Benchmark Results:**
- Total time: [e.g., 4.2 seconds]
- Speedup: [e.g., 7.1x vs CPU]
- Pool hit rate: [e.g., 97%]
- Peak VRAM: [e.g., 150 MB]

**Expected vs Actual:**
- Expected: 6-8x speedup
- Actual: 7.1x speedup
- Status: ✓ Within expected range

**Screenshots:**
[Optional: Task Manager GPU usage, thumbnails generated]
```

---

## Testing Checklist

### Pre-Testing
- [ ] Update GPU drivers to latest version
- [ ] Install ExplorerLens v5.2.0
- [ ] Download DebugView (Sysinternals)
- [ ] Prepare test image set (100+ images)
- [ ] Build GPUValidator.exe

### GPU Detection
- [ ] Run GPUValidator.exe
- [ ] Verify GPU detected correctly
- [ ] Verify vendor ID correct (Intel=0x8086, NVIDIA=0x10de, AMD=0x1002)
- [ ] Verify feature level DirectX 11.0+
- [ ] Verify compute shader support

### Functionality
- [ ] Run DebugView as Administrator
- [ ] Navigate to folder with images
- [ ] Verify GPU initialization logs appear
- [ ] Verify thumbnails generate correctly
- [ ] Check for error messages in DebugView

### Performance
- [ ] Run LENSBench.exe with 100+ images
- [ ] Record total time
- [ ] Calculate speedup vs CPU baseline
- [ ] Verify texture pool hit rate >90%
- [ ] Monitor GPU usage in Task Manager

### Stress Testing
- [ ] Generate 1000+ thumbnails consecutively
- [ ] Verify no memory leaks (stable VRAM usage)
- [ ] Verify no crashes or hangs
- [ ] Check texture pool cleanup logs

### Debug Layer (Optional)
- [ ] Enable DirectX debug layer
- [ ] Rebuild in Debug mode
- [ ] Run with DebugView
- [ ] Check for errors/warnings
- [ ] Investigate any performance warnings

---

## Support

**Documentation:**
- [GPU Acceleration Overview](GPU_ACCELERATION_OVERVIEW.md)
- [Intel GPU Guide](INTEL_GPU_GUIDE.md)
- [GPU Testing Guide](GPU_TESTING_GUIDE.md)
- [Texture Pooling](TEXTURE_POOLING.md)

**Tools:**
- GPUValidator.exe - GPU detection
- LENSBench.exe - Performance benchmarking
- GPUThumbnailTest.exe - Functional testing
- DebugView - Log capture

**Issues:** Report GPU-specific issues on GitHub with template above

---

**Last Updated:** November 24, 2025  
**Tested Configurations:** Intel Iris Xe, NVIDIA RTX 3060, AMD RX 6700 XT, WARP  
**Status:** Production Ready ✅

