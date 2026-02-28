# ExplorerLens — Performance Benchmarks Baseline

**Version:** 15.0.0 "Zenith"  
**Last Updated:** July 2025  
**Hardware:** Intel i7-12700K, 32 GB DDR5, NVIDIA RTX 3080, NVMe SSD  
**OS:** Windows 11 24H2 (Build 26100)  
**Compiler:** MSVC 19.50.35720 (v145), /O2 /GL /arch:AVX2  

---

## 1. Thumbnail Generation Latency (Single Image)

### Target: < 17 ms (median)

| Format | File Size | Cold (ms) | Warm Cache (ms) | Target (ms) |
|--------|-----------|-----------|-----------------|-------------|
| JPEG | 5 MB | 20-50 | 1-3 | < 20 |
| PNG | 8 MB | 30-60 | 1-3 | < 30 |
| WebP | 2 MB | 30-60 | 1-3 | < 30 |
| JPEG XL | 1 MB | 40-80 | 1-3 | < 40 |
| AVIF | 1 MB | 50-100 | 1-3 | < 50 |
| HEIF/HEIC | 3 MB | 60-120 | 1-3 | < 60 |
| RAW (CR2) | 25 MB | 100-300 | 1-5 | < 150 |
| PSD | 50 MB | 200-500 | 1-5 | < 250 |
| PDF (1 page) | 100 KB | 50-150 | 1-5 | < 80 |
| CBZ (archive) | 50 MB | 200-500 | 1-5 | < 300 |
| CBR (archive) | 50 MB | 300-800 | 1-5 | < 400 |
| Video (MP4) | 500 MB | 500-2000 | 1-5 | < 1000 |

---

## 2. Batch Throughput

### Target: > 235 images/sec (256×256 thumbnails)

| Scenario | Current (img/s) | Target (img/s) | Method |
|----------|----------------|----------------|--------|
| JPEG batch (100 files) | 235 | 300 | Pipeline + GPU |
| Mixed formats (100 files) | 150 | 200 | Pipeline |
| Archive extraction (50 CBZ) | 80 | 120 | Parallel I/O |
| Network drive (100 files) | 50 | 80 | Prefetch + cache |

---

## 3. Cache Performance

### Target: < 5 ms cache hit, > 85% hit rate

| Metric | Current | Target |
|--------|---------|--------|
| Cache hit latency | < 5 ms | < 1 ms |
| Cache hit rate (browsing) | 85-95% | > 90% |
| Cache miss penalty | 50-500 ms | — |
| Cache warm-up (1000 files) | 2-5 s | < 2 s |
| USN journal invalidation | < 100 ms | < 50 ms |

---

## 4. Memory Usage

### Target: < 45 MB idle, < 500 MB peak

| State | Current (MB) | Target (MB) |
|-------|-------------|-------------|
| Idle (loaded, no activity) | 50-100 | < 45 |
| Active (10 thumbnails) | 150-300 | < 200 |
| Heavy load (100 thumbnails) | 500-1000 | < 500 |
| Peak (concurrent archives) | 800-1500 | < 1000 |

---

## 5. Build Performance

| Metric | Current | Target |
|--------|---------|--------|
| Clean build (152 targets) | ~120 s | < 90 s |
| Incremental build (1 file) | ~5 s | < 3 s |
| Test suite (1243 tests) | ~2 s | < 2 s |
| Engine library size | 293 MB | < 250 MB |
| LENSShell.dll size | 2.9 MB | < 3.0 MB |

---

## 6. GPU Acceleration

| Operation | CPU (ms) | GPU DX11 (ms) | Speedup |
|-----------|----------|--------------|---------|
| Bilinear scale 4K→256 | 8.5 | 0.6 | 14x |
| Lanczos scale 4K→256 | 15.2 | 1.1 | 14x |
| Color space conversion | 3.2 | 0.3 | 11x |
| Alpha composite | 2.1 | 0.2 | 10x |

---

## 7. Cold Start

| Stage | Current (ms) | Target (ms) |
|-------|-------------|-------------|
| DLL load + COM init | 50 | < 30 |
| Decoder registration | 30 | < 20 |
| GPU device creation | 80 | < 50 |
| Cache index load | 40 | < 20 |
| **Total cold start** | **200** | **< 100** |

---

## 8. Benchmark Test Names (CTest)

```
EngineBenchmark:
  BM_JpegDecode256
  BM_PngDecode256
  BM_WebpDecode256
  BM_BatchDecode100
  BM_CacheHitLatency
```

### Running Benchmarks

```powershell
# Run all benchmarks
.\build\bin\EngineBenchmark.exe --benchmark_format=console

# Run specific benchmark
.\build\bin\EngineBenchmark.exe --benchmark_filter=BM_JpegDecode

# JSON output for CI tracking
.\build\bin\EngineBenchmark.exe --benchmark_out=results.json --benchmark_out_format=json
```

---

## 9. Profiling Tools

| Tool | Usage | Best For |
|------|-------|----------|
| ETW + WPA | `xperf -on PROC_THREAD+CSWITCH+LOADER` | System-level bottlenecks |
| VS Profiler | Debug → Performance Profiler | CPU hot paths |
| PIX | GPU capture | Shader/dispatch analysis |
| Intel VTune | vtune -collect hotspots | Microarchitectural analysis |
| `QueryPerformanceCounter` | In-code timing | Per-function measurement |

---

## 10. Regression Thresholds

CI should flag performance regressions exceeding these thresholds:

| Metric | Threshold | Action |
|--------|-----------|--------|
| Median latency | +20% | Warning |
| P99 latency | +50% | Warning |
| Batch throughput | -15% | Error (blocks merge) |
| Memory peak | +30% | Warning |
| Build time | +25% | Warning |

---

**Document Version:** 1.0
