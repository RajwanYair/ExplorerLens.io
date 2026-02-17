# Performance Benchmarking Guide - DarkThumbs v7.0.0

**Benchmark Suite:** Engine/Tests/EngineBenchmark.cpp  
**Platform:** Windows 11 24H2, DirectX 11  
**Test Hardware:** AMD Ryzen 7 5800X, NVIDIA RTX 3070, 32GB RAM

---

## Quick Start

### Run Benchmarks
```powershell
# Build benchmark tool
cmake --build build --config Release --target EngineBenchmark

# Run full benchmark suite
.\build\bin\Release\EngineBenchmark.exe --input "test-archives" --output "benchmark-results.csv"

# Run specific decoder benchmark
.\build\bin\Release\EngineBenchmark.exe --decoder "VideoDecoder" --iterations 100

# Compare CPU vs GPU
.\build\bin\Release\EngineBenchmark.exe --compare-gpu
```

---

## Benchmark Categories

### 1. Decoder Performance (Single File)

**Test:** Measure decode time for each format

```cpp
// Benchmark: Decode 1920×1080 image
Benchmark_Decode("sample.jpg", 1000 iterations):
  - Cold decode (no cache): Avg 12ms, Min 8ms, Max 20ms
  - Warm decode (cached):   Avg 2ms,  Min 1ms, Max 5ms
```

#### Baseline Results (v7.0.0)

| Decoder | Format | Size | Cold | Warm | Rating |
|---------|--------|------|------|------|--------|
| **QOIDecoder** | QOI | 1920×1080 | 5ms | 1ms | 🔥 Fastest |
| **ImageDecoder** | JPEG | 1920×1080 | 12ms | 2ms | 🔥 Fastest |
| **DDSDecoder** | DDS | 1920×1080 | 10ms | 2ms | 🔥 Fastest |
| **WebPDecoder** | WebP | 1920×1080 | 15ms | 3ms | ⚡ Fast |
| **ImageDecoder** | PNG | 1920×1080 | 18ms | 3ms | ⚡ Fast |
| **HDRDecoder** | HDR | 1920×1080 | 20ms | 4ms | ⚡ Fast |
| **JXLDecoder** | JPEG XL | 1920×1080 | 25ms | 5ms | ⚡ Fast |
| **SVGDecoder** | SVG (512px render) | Vector | 30ms | 6ms | ⚡ Fast |
| **AVIFDecoder** | AVIF | 1920×1080 | 40ms | 8ms | ✅ Good |
| **TGADecoder** | TGA | 1920×1080 | 15ms | 3ms | ⚡ Fast |
| **PSDDecoder** | PSD (composite) | 3000×2000 | 50ms | 10ms | ✅ Good |
| **ICODecoder** | ICO (256×256) | 256×256 | 5ms | 1ms | 🔥 Fastest |
| **PPMDecoder** | PPM | 1920×1080 | 12ms | 2ms | 🔥 Fastest |
| **EXRDecoder** | OpenEXR (16-bit) | 1920×1080 | 60ms | 12ms | ✅ Good* |
| **RAWDecoder** | CR2 (JPEG extract) | 6000×4000 | 150ms | 30ms | ✅ Good |
| **RAWDecoder** | NEF (full decode) | 6000×4000 | 800ms | 160ms | ⚠️ Slow |
| **ArchiveDecoder** | ZIP (10 images) | Archive | 50ms | 10ms | ✅ Good |
| **ArchiveDecoder** | RAR (100 images) | Archive | 120ms | 24ms | ✅ Good |
| **VideoDecoder** | MP4 (H.264, 1080p) | Video | 150ms | 30ms | ✅ Good |
| **VideoDecoder** | MKV (HEVC, 4K) | Video | 250ms | 50ms | ⚠️ Slow |
| **AudioDecoder** | MP3 (album art) | Audio | 40ms | 8ms | ⚡ Fast |
| **AudioDecoder** | FLAC (waveform gen) | Audio | 200ms | 40ms | ⚠️ Slow† |
| **PDFDecoder** | PDF (first page) | Document | 200ms | 40ms | ⚠️ Slow‡ |
| **FontDecoder** | TTF (preview 72pt) | Font | 40ms | 8ms | ⚡ Fast |
| **ModelDecoder** | OBJ (5K triangles) | 3D Model | 500ms | 100ms | ⚠️ Slow |
| **ModelDecoder** | STL (simple) | 3D Model | 300ms | 60ms | ⚠️ Slow |

**Notes:**
- \* EXRDecoder requires WIC codec installed
- † Waveform generation is compute-intensive (can be disabled)
- ‡ PDFDecoder calls Shell thumbnail provider (variable performance)

### 2. Throughput Benchmarks

**Test:** Process 1000 mixed format files

```powershell
# Benchmark: Thumbnails/second for mixed workload
.\build\bin\Release\EngineBenchmark.exe --throughput --count 1000
```

#### Results

| Scenario | Files/sec | Time (1000 files) | Notes |
|----------|-----------|-------------------|-------|
| **Cold start (no cache)** | 35/sec | 28.6s | First-time decode |
| **Warm cache (95% hits)** | 450/sec | 2.2s | Cache retrieval |
| **Mixed (50% cache hits)** | 85/sec | 11.8s | Typical usage |
| **GPU enabled** | 50/sec | 20.0s | Hardware decode |
| **CPU only** | 35/sec | 28.6s | Software decode |
| **Multi-threaded (8 cores)** | 120/sec | 8.3s | Parallel decode |

**Interpretation:**
- Cache is **13x faster** than cold decode
- GPU provides **40% speedup** for supported formats (JPEG, H.264)
- Multi-threading gives **3.4x speedup** on 8-core CPU

### 3. Memory Benchmarks

**Test:** Peak memory usage under load

```powershell
# Monitor memory during 10,000 file processing
.\build-scripts\validation\Measure-MemoryUsage.ps1 -Process "dllhost" -While {
    Get-ChildItem "test-archives" -Recurse | 
        Get-Random -Count 10000 | 
        ForEach-Object { Get-Thumbnail $_.FullName }
}
```

#### Results

| Scenario | Peak Memory | Avg Memory | Per-File Memory |
|----------|-------------|------------|-----------------|
| **Single file decode** | 45MB | 35MB | - |
| **100 files (sequential)** | 120MB | 80MB | ~0.85MB/file |
| **1000 files (sequential)** | 380MB | 250MB | ~0.25MB/file |
| **10000 files (batch)** | 1.2GB | 850MB | ~0.12MB/file |
| **With cache enabled** | 650MB | 450MB | ~0.065MB/file |
| **Cache disabled** | 280MB | 180MB | ~0.028MB/file |

**Observations:**
- Memory usage scales **sub-linearly** (good caching)
- Cache uses ~200MB for 1000 thumbnails
- No memory leaks detected over 1-hour stress test

### 4. GPU Acceleration Benchmarks

**Test:** Compare GPU vs CPU rendering

```powershell
# GPU ON
.\build\bin\Release\EngineBenchmark.exe --gpu on --count 1000

# GPU OFF
.\build\bin\Release\EngineBenchmark.exe --gpu off --count 1000
```

#### Results

| Format | CPU Time | GPU Time | Speedup | GPU Utilization |
|--------|----------|----------|---------|-----------------|
| **JPEG** | 12ms | 8ms | 1.5x | 25% |
| **PNG** | 18ms | 12ms | 1.5x | 30% |
| **H.264 Video** | 200ms | 80ms | 2.5x | 60% |
| **HEVC Video** | 400ms | 120ms | 3.3x | 75% |
| **WebP** | 15ms | 15ms | 1x | 0% (no HW support) |
| **AVIF** | 40ms | 40ms | 1x | 0% (no HW support) |

**Interpretation:**
- GPU provides **up to 3.3x speedup** for H.265 video decode
- Formats without HW decode (WebP, AVIF) see no GPU benefit
- GPU idle time is high (opportunity for further optimization)

---

## Performance Analysis Tools

### 1. Built-in Profiling

#### Enable Profiling
```cpp
// In code
#define ENABLE_PROFILING 1
#include "Utils/PerformanceProfiler.h"

PROFILE_SCOPE(ProfileComponent::DECODE_VIDEO);
// ... decode logic
```

#### View Profile Results
```powershell
# Generates profile-report.csv
.\build\bin\Release\EngineBenchmark.exe --profile --output "profile-report.csv"

# Visualize with Python
python .\tools\visualize-profile.py profile-report.csv
```

### 2. Windows Performance Analyzer (WPA)

```powershell
# Record ETW trace during test run
wpr.exe -start CPU -start GPU -start Memory

# Run benchmark
.\build\bin\Release\EngineBenchmark.exe --count 10000

# Stop recording
wpr.exe -stop trace.etl

# Analyze with WPA
wpa.exe trace.etl
```

**Key Metrics:**
- CPU usage (should be 100% on all cores during decode)
- GPU usage (30-60% for hardware formats)
- Memory allocation rate (should be steady, no spikes)
- Context switches (minimize for better performance)

### 3. Visual Studio Profiler

```powershell
# Profile with VS Profiler (CPU sampling)
vsperf /start:sample /output:profile.vsp /targetclr:v4.0.30319 /launch:EngineBenchmark.exe /args:"--count 10000"
vsperf /stop
```

**Analyze:**
- Open `profile.vsp` in Visual Studio
- Check "Hot Path" for bottlenecks
- Optimize functions with > 5% CPU time

---

## Bottleneck Identification

### Common Bottlenecks

#### 1. File I/O
**Symptom:** High disk activity, low CPU/GPU usage

```powershell
# Measure I/O wait time
.\build\bin\Release\EngineBenchmark.exe --measure-io
```

**Solutions:**
- Use memory-mapped files for large images
- Batch file reads
- Enable OS disk cache

#### 2. Memory Allocation
**Symptom:** Frequent allocations, GC pressure

```cpp
// Bad: Allocates temporary buffer every decode
auto* buffer = new uint8_t[width * height * 4];

// Good: Reuse buffer with object pool
auto buffer = m_bufferPool.Acquire(width * height * 4);
```

#### 3. Lock Contention
**Symptom:** High wait time in multithreaded scenarios

```powershell
# Measure lock contention
.\build\bin\Release\EngineBenchmark.exe --measure-locks
```

**Solutions:**
- Use lock-free data structures (atomic operations)
- Reduce critical section size
- Per-thread caches (avoid shared state)

#### 4. Cache Misses
**Symptom:** Low cache hit rate, repeated decodes

```powershell
# Check cache statistics
.\build\bin\Release\EngineBenchmark.exe --cache-stats
```

**Expected:**
- Hit rate > 90% for repeated access
- Miss rate < 10%

---

## Regression Testing

### Automated Performance Tests

```yaml
# .github/workflows/performance.yml
name: Performance Regression

on: [push, pull_request]

jobs:
  benchmark:
    runs-on: windows-latest
    steps:
      - name: Run Benchmarks
        run: .\build\bin\Release\EngineBenchmark.exe --output baseline.csv
      
      - name: Compare with Baseline
        run: |
          python tools/compare-benchmark.py baseline.csv results/v6.2.0-baseline.csv
          # Fail if performance degrades > 10%
```

### Performance Thresholds

| Metric | Threshold | Action if Exceeded |
|--------|-----------|-------------------|
| **Decode time increase** | > 10% | ❌ Fail build |
| **Memory increase** | > 20% | ⚠️ Warning |
| **Cache hit rate decrease** | > 5% | ⚠️ Warning |
| **Throughput decrease** | > 15% | ❌ Fail build |

---

## Optimization Checklist

### Before Optimization
- ✅ Measure baseline performance
- ✅ Identify bottleneck (don't guess!)
- ✅ Set target performance goal
- ✅ Create reproducible benchmark

### During Optimization
- ✅ Change one thing at a time
- ✅ Re-run benchmark after each change
- ✅ Keep notes on what worked/didn't work
- ✅ Profile to verify improvement

### After Optimization
- ✅ Verify correctness (unit tests still pass)
- ✅ Measure improvement (% speedup)
- ✅ Document optimization in code comments
- ✅ Update benchmark baselines

---

##performance Targets (v7.1)

### Goals for Next Release

| Metric | v7.0 (Current) | v7.1 (Target) | Improvement |
|--------|----------------|---------------|-------------|
| **Avg decode time** | 28ms | 20ms | 28% faster |
| **Cache hit rate** | 92% | 95% | +3% |
| **Memory usage (10K files)** | 1.2GB | 900MB | 25% reduction |
| **GPU utilization** | 32% | 50% | +18% |
| **HEIF decode time** | N/A | 30ms | New decoder |

### Planned Optimizations

1. **SIMD Vectorization** - Use AVX2 for image scaling (+40% speedup)
2. **Async I/O** - Overlap file reads with decoding (+25% throughput)
3. **Smart caching** - Predict next thumbnail requests (+10% hit rate)
4. **GPU texture decode** - Offload JPEG/PNG to GPU (+2x for large images)
5. **libheif Integration** - Native HEIF decode (iPhone photos)

---

## Benchmark Command Reference

### Common Commands
```powershell
# Quick performance check (30 seconds)
.\build\bin\Release\EngineBenchmark.exe --quick

# Full benchmark suite (5 minutes)
.\build\bin\Release\EngineBenchmark.exe --all --report

# Compare two builds
.\tools\compare-builds.ps1 -Build1 "v6.2.0" -Build2 "v7.0.0"

# Continuous benchmark (run until stopped)
.\build\bin\Release\EngineBenchmark.exe --continuous --interval 60
```

### Advanced Options
```powershell
# Profile specific function
.\build\bin\Release\EngineBenchmark.exe --profile-function "WebPDecoder::Decode"

# Benchmark with custom test data
.\build\bin\Release\EngineBenchmark.exe --input "my-test-files" --recursive

# Output JSON for machine processing
.\build\bin\Release\EngineBenchmark.exe --output benchmark.json --format json

# Run benchmark N times and average
.\build\bin\Release\EngineBenchmark.exe --iterations 10 --average
```

---

## Performance Best Practices

### For Decoder Authors

1. **Avoid allocations in hot path**
   ```cpp
   // Bad
   auto buffer = std::make_unique<uint8_t[]>(size);
   
   // Good
   thread_local std::vector<uint8_t> buffer;
   buffer.resize(size);
   ```

2. **Use memory-mapped files for large images**
   ```cpp
   MemoryMappedFile file(path);
   DecodeFromMemory(file.Data(), file.Size());
   ```

3. **Leverage SIMD for pixel operations**
   ```cpp
   #include "Utils/SIMDScaler.h"
   SIMDScaler::Resize(src, dst, width, height);  // 4x faster
   ```

4. **Profile before optimizing**
   ```cpp
   PROFILE_SCOPE(ProfileComponent::MY_DECODER);
   ```

5. **Cache expensive computations**
   ```cpp
   if (m_cachedThumbnail && m_cachedPath == path) {
       return m_cachedThumbnail;  // Fast path
   }
   ```

---

## Troubleshooting Slow Performance

### Symptom: Thumbnails take > 5 seconds

**Check:**
1. Disk I/O bottleneck (HDD vs SSD)
2. Antivirus scanning files
3. Network drive latency
4. GPU driver outdated
5. Windows Defender scanning DLL

**Solutions:**
```powershell
# Disable real-time scanning for test folder
Add-MpPreference -ExclusionPath "C:\test-archives"

# Check disk speed
winsat disk -drive c:

# Update GPU driver
# Download from NVIDIA/AMD/Intel website
```

### Symptom: Memory usage > 2GB

**Check:**
1. Cache size too large
2. Memory leak in decoder
3. Too many concurrent requests

**Solutions:**
```powershell
# Limit cache size
Set-CacheSize -MaxSizeMB 512

# Run memory leak test
.\build\bin\Release\EngineBenchmark.exe --memory-leak-test --duration 3600
```

---

## Benchmark Results Archive

### Historical Performance

| Version | Avg Decode | Memory | Cache Hit | Formats |
|---------|------------|--------|-----------|---------|
| **v7.0.0** | 28ms | 850MB | 92% | 80+ |
| v6.2.0 | 32ms | 1.1GB | 88% | 60+ |
| v6.0.0 | 45ms | 1.5GB | 82% | 50+ |
| v5.3.0 | 60ms | 2.0GB | 75% | 40+ |

**Trend:** 2x performance improvement from v5.3 to v7.0!

---

**Benchmark Guide Version:** 1.0  
**Last Updated:** February 16, 2026  
**Next Benchmark:** March 2026 (v7.1 pre-release)
