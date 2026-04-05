# ExplorerLens — Performance Benchmarks Baseline

**Version:** 32.3.1 "Fomalhaut-T"  
**Last Updated:** April 2026  
**Hardware:** Intel Core Ultra 9 285K, 64 GB DDR5-6400, NVIDIA RTX 4090, NVMe SSD (7 GB/s)  
**NPU Target:** Intel AI Boost NPU (Meteor Lake / Arrow Lake, 48 TOPS)  
**OS:** Windows 11 24H2 (Build 26100)  
**Compiler:** MSVC 19.50.35720 (v145), /O2 /GL /arch:AVX512  

---

## 1. Thumbnail Generation Latency (Single Image)

### Target: < 17 ms (P50 median), < 50 ms (P99)

| Format | File Size | Cold CPU (ms) | Cold NPU (ms) | Warm Cache (ms) | Target P50 (ms) |
|--------|-----------|---------------|---------------|-----------------|-----------------|
| JPEG | 5 MB | 15–35 | 8–20 | 1–2 | < 15 |
| PNG | 8 MB | 20–45 | 10–25 | 1–2 | < 20 |
| WebP | 2 MB | 20–45 | 10–22 | 1–2 | < 20 |
| JPEG XL | 1 MB | 30–60 | 15–35 | 1–2 | < 30 |
| AVIF | 1 MB | 35–75 | 18–40 | 1–2 | < 35 |
| HEIF/HEIC | 3 MB | 45–90 | 22–50 | 1–2 | < 45 |
| RAW (CR2/ARW) | 25 MB | 80–220 | 40–120 | 1–4 | < 100 |
| PSD | 50 MB | 150–400 | 70–200 | 1–4 | < 180 |
| PDF (1 page) | 100 KB | 35–100 | 20–60 | 1–4 | < 50 |
| CBZ (archive) | 50 MB | 150–380 | 70–180 | 1–4 | < 200 |
| CBR (archive) | 50 MB | 200–600 | 90–280 | 1–4 | < 280 |
| Video (MP4) | 500 MB | 300–1200 | 150–600 | 1–4 | < 700 |

---

## 2. Batch Throughput

### Target: > 235 images/sec CPU, > 350 img/sec with NPU offload

| Scenario | CPU (img/s) | NPU+GPU (img/s) | Target (img/s) | Method |
|----------|-------------|-----------------|----------------|--------|
| JPEG batch (100 files) | 280 | 420 | 300+ | Pipeline + GPU |
| Mixed formats (100 files) | 180 | 280 | 220+ | Pipeline |
| Archive extraction (50 CBZ) | 100 | 140 | 130+ | Parallel I/O |
| Network drive (100 files) | 65 | 90 | 85+ | Prefetch + cache |
| CLIP embedding (batch 32) | 15 | 32 | 25+ | NPU INT8 |
| Cache hit latency | < 5 ms | < 1 ms | < 1 ms | Cache lookup |
| Cache hit rate (browsing) | 85-95% | > 90% | > 90% | Cache stats |
| Cache miss penalty | 50-500 ms | — | < 50 ms | Cache policy |
| Cache warm-up (1000 files) | 2-5 s | < 2 s | < 2 s | Prefetch |
| USN journal invalidation | < 100 ms | < 50 ms | < 50 ms | File watcher |

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
