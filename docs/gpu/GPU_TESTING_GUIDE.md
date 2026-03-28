# GPU Testing & Optimization Guide — ExplorerLens v23.7.0

**Version:** 23.7.0 "Vega-X" 
**Last Updated:** February 2026 
**Status:** GPU Testing Operational (D3D11 + D3D12 + Vulkan Compute)

---

## Overview

ExplorerLens uses GPU-accelerated thumbnail generation via DirectX 11/12 compute shaders with CPU fallback. This guide covers testing, benchmarking, and GPU-specific optimization across Intel, NVIDIA, and AMD hardware.

---

## 1. Test Tools

### GPUThumbnailTest.exe

Validates GPU thumbnail generation functionality and quality.

```cmd
GPUThumbnailTest.exe [options]

Options:
 -i <folder> Input folder with test images (required)
 -o <folder> Output folder for thumbnails (optional)
 -s <size> Thumbnail size in pixels (default: 256)
 -v Verbose output
```

### LENSBench.exe

Performance benchmarking against CPU baseline.

```cmd
LENSBench.exe [options]

Options:
 -i <folder> Input folder with test images (required)
 -o <file> Output CSV file (optional)
 -s <size> Thumbnail size in pixels (default: 256)
 -n <count> Iterations per file (default: 10)
 -v Verbose output
```

### GPUValidator.exe

Detects and validates all GPUs in the system.

```cmd
tests\build\GPUValidator.exe
```

Reports vendor, feature level, VRAM, compute shader support, and ExplorerLens compatibility for each detected GPU.

---

## 2. Test Scenarios

### Scenario 1: Intel Integrated GPU Only (Laptop)

- Hardware: Intel Core i7-1165G7 + Intel Iris Xe
- Expected: DirectX 11.1, 6–8x speedup, shared system memory

### Scenario 2: NVIDIA Discrete GPU (Desktop)

- Hardware: NVIDIA GeForce RTX 3070
- Expected: DirectX 12.1, 8–10x speedup, dedicated VRAM

### Scenario 3: AMD Discrete GPU

- Hardware: AMD Radeon RX 6700 XT
- Expected: DirectX 12.0+, 7–9x speedup, dedicated VRAM

### Scenario 4: Dual GPU (Intel iGPU + NVIDIA dGPU)

- Windows automatically selects dGPU for ExplorerLens
- Test both GPUs by forcing via Intel Graphics Command Center

### Scenario 5: Old GPU (DirectX 10.x — CPU Fallback)

- Intel HD Graphics 2000 or similar (DirectX 10.1)
- Falls back to CPU rendering (no compute shader support)

### Scenario 6: No GPU (WARP Software Renderer)

- Virtual machines or cloud instances with no physical GPU
- DirectX 11.1 via software emulation, ~1.0–1.5x speedup

**Validation for all scenarios:**
```cmd
tests\build\GPUValidator.exe
tests\build\LENSBench.exe -i C:\TestImages -n 100 -s 256
```

Monitor DebugView (Sysinternals) for GPU initialization logs:
```
[GPU] Device created successfully (Hardware)
[GPU] Vendor: Intel (0x8086) / NVIDIA (0x10de) / AMD (0x1002)
[GPU] Feature Level: DirectX 11.x / 12.x
[GPU] Compute shader support enabled
```

---

## 3. Performance Expectations by GPU

### Decode Time (256×256 thumbnail)

| GPU Type | Speedup | 300 Images Total Time | Pool Hit Rate |
|----------|---------|----------------------|---------------|
| Intel Arc A770 | 8–10x | 3.5–4.0s | 95%+ |
| NVIDIA RTX 3070 | 8–10x | 3.2–3.8s | 95%+ |
| AMD RX 6700 XT | 7–9x | 3.8–4.5s | 95%+ |
| Intel Iris Xe | 6–8x | 4.5–5.5s | 95%+ |
| Intel UHD 630 | 4–6x | 6.0–8.0s | 95%+ |
| Intel HD 4000 | 3–5x | 8.0–10.0s | 95%+ |
| WARP | 1.0–1.5x | 28–32s | 95%+ |
| CPU baseline | 1.0x | 30–35s | N/A |

### Validation Criteria

- **Pass:** Average speedup ≥ 6.0x
- **Warning:** Average speedup 4.0–6.0x (acceptable, investigate)
- **Fail:** Average speedup < 4.0x (needs optimization)

---

## 4. Intel GPU Support

### Supported Intel GPUs

| Tier | GPU Family | Generations | DirectX | Expected Speedup |
|------|-----------|-------------|---------|-----------------|
| Excellent | Intel Arc | A770, A750, A580, A380, A310 | 12.1 | 8–10x |
| Excellent | Iris Xe | 11th–13th gen | 12.1 | 6–8x |
| Very Good | Iris Plus | 10th gen | 12.1 | 5–7x |
| Good | Iris/Iris Pro | 4th–9th gen | 11.0–11.2 | 4–6x |
| Good | UHD Graphics | 8th–10th gen | 12.0 | 4–6x |
| Good | HD Graphics | 2nd–7th gen (2011+) | 11.0 | 3–5x |
| CPU Fallback | HD Graphics | 1st gen and older | 10.x | N/A |

### Intel-Specific Optimizations

- **Shared memory optimization:** Zero-copy texture uploads, efficient shared allocation
- **Compute shader tuning:** 8×8 thread groups optimal for Intel execution units
- **DirectX 11.1 fast semantics:** Faster resource creation on HD 4000+

### Intel GPU Benchmark Data

| Format | Arc A770 | Iris Xe | HD 630 | HD 4000 |
|--------|----------|---------|--------|---------|
| JPEG 4K | 10ms (10.0x) | 14ms (7.1x) | 20ms (5.0x) | 25ms (4.0x) |
| PNG Complex | 18ms (8.3x) | 25ms (6.0x) | 35ms (4.3x) | 42ms (3.6x) |
| WebP Animated | 9ms (8.9x) | 11ms (7.3x) | 15ms (5.3x) | 18ms (4.4x) |
| AVIF HDR | 15ms (8.0x) | 18ms (6.7x) | 25ms (4.8x) | 30ms (4.0x) |

### Intel Processor Quick Reference

| Generation | Codename | Year | GPU | DirectX |
|------------|----------|------|-----|---------|
| 14th Gen | Meteor Lake | 2023 | Intel Arc Graphics | 12.1 |
| 13th Gen | Raptor Lake | 2022 | UHD/Iris Xe | 12.1 |
| 12th Gen | Alder Lake | 2021 | UHD/Iris Xe | 12.1 |
| 11th Gen | Tiger Lake | 2020 | Iris Xe | 12.1 |
| 10th Gen | Ice Lake | 2019 | Iris Plus | 12.1 |
| 8th–9th Gen | Coffee Lake | 2017–18 | UHD 630 | 12.0 |
| 7th Gen | Kaby Lake | 2016 | HD 630 | 12.0 |
| 6th Gen | Skylake | 2015 | HD 530 | 12.0 |
| 4th Gen | Haswell | 2013 | HD 4600 | 11.1 |
| 3rd Gen | Ivy Bridge | 2012 | HD 4000 | **11.0** ✅ |
| 2nd Gen | Sandy Bridge | 2011 | HD 3000 | **11.0** ✅ |

Minimum: 2nd Gen Sandy Bridge (2011) with DirectX 11.0 support.

---

## 5. Quality Validation

Compare GPU-generated thumbnails to CPU baseline:

1. **Color Accuracy** — Check for color shifts, verify sRGB conversion
2. **Sharpness** — Lanczos3 should preserve detail better than Fant scaler
3. **Artifacts** — No ringing/halos, no banding in gradients, no color fringing
4. **Transparency** — PNG alpha channels and WebP transparency preserved

---

## 6. Test Image Preparation

### Recommended Test Set

| Format | Count | Resolutions |
|--------|-------|-------------|
| JPEG | 10+ | 640×480, 1920×1080, 3840×2160, 7680×4320 |
| PNG | 5+ | Simple, alpha, indexed, grayscale |
| WebP | 3+ | Lossy, lossless, animated |
| AVIF | 3+ | Standard, HDR, 10-bit |
| HEIF/HEIC | 3+ | iPhone photos, HDR, burst |
| Video | 5+ | MP4 (H.264), MKV (HEVC), AVI, MOV, WebM |

---

## 7. Troubleshooting

### GPU Not Detected

1. Update GPU drivers (Intel Download Center / NVIDIA GeForce / AMD Adrenalin)
2. Check Device Manager → Display adapters
3. Verify BIOS has integrated graphics enabled (if applicable)
4. WARP fallback activates automatically if no hardware GPU available

### Poor Performance

1. Set Windows Power Plan to High Performance
2. Close GPU-intensive background apps (browsers, video players)
3. Check for thermal throttling (CPU/GPU temps)
4. Ensure 8+ GB RAM for Intel integrated GPUs
5. Use SSD storage for test images

### Intel Arc Issues

1. Update to latest Arc drivers (31.0.101.4953+)
2. Enable Resizable BAR in BIOS
3. Install DirectX End-User Runtime for d3dcompiler_47.dll

### Shader Compilation Errors

1. Verify d3dcompiler_47.dll exists in System32
2. Install DirectX End-User Runtime
3. WIC fallback activates automatically on shader failure

---

## 8. Debug Layer Testing

Enable DirectX debug layer for detailed GPU diagnostics:

1. Install Graphics Tools: Windows Settings → Apps → Optional Features → "Graphics Tools"
2. Rebuild in Debug configuration with `D3D11_CREATE_DEVICE_DEBUG` flag
3. Monitor DebugView for D3D11 INFO/WARNING/ERROR messages

---

## 9. Benchmark Best Practices

- **Warmup:** First run compiles shaders — discard first iteration
- **Iterations:** Minimum 10, recommended 20–50, high precision 100+
- **Environment:** Close background apps, high-performance power plan, SSD storage
- **Statistics:** Check standard deviation; filter outliers >2σ from mean
- **Consistency:** Same test images and system state across runs

---

## 10. Testing Checklist

### Pre-Testing
- [ ] Update GPU drivers to latest
- [ ] Prepare test image set (100+ images)
- [ ] Download DebugView (Sysinternals)

### Validation
- [ ] Run GPUValidator.exe — verify vendor, feature level, compute shaders
- [ ] Run LENSBench.exe — verify speedup meets expectations
- [ ] Monitor DebugView — no GPU errors
- [ ] Visual inspection — thumbnails match CPU baseline quality

### Stress Testing
- [ ] Generate 1000+ thumbnails consecutively
- [ ] Verify stable VRAM usage (no memory leaks)
- [ ] Verify no crashes or hangs
- [ ] Check texture pool cleanup logs

---

*This document consolidates GPU_TESTING_GUIDE.md, MULTI_GPU_TESTING_GUIDE.md, and INTEL_GPU_GUIDE.md.*
