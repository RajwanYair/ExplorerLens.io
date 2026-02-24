# ExplorerLens Performance Tuning Guide
**Sprint 177: Version Normalization**  
**Version:** 15.0.0  
**Last Updated:** June 2025

---

## Table of Contents
1. [Performance Overview](#performance-overview)
2. [Hardware Acceleration](#hardware-acceleration)
3. [Cache Optimization](#cache-optimization)
4. [Thread Pool Tuning](#thread-pool-tuning)
5. [Memory Management](#memory-management)
6. [Network Drive Optimization](#network-drive-optimization)
7. [Decoder-Specific Tuning](#decoder-specific-tuning)
8. [Benchmarking](#benchmarking)
9. [Profiling](#profiling)

---

## Performance Overview

### Key Performance Metrics

| Metric | Typical Value | Measurement |
|--------|---------------|-------------|
| **Cache Hit Rate** | 85-95% | Cached thumbnails / Total requests |
| **Decode Latency** | 10-50 ms | Time to generate thumbnail (cached) |
| **Decode Latency** | 100-500 ms | Time to generate thumbnail (first time) |
| **GPU Speedup** | 5-15x | GPU decode time / CPU decode time |
| **Memory Usage** | 50-200 MB | Working set size (varies with cache) |
| **Disk I/O** | <10 MB/s | Cache read/write bandwidth |

---

### Performance Bottlenecks

**Common bottlenecks and solutions:**

| Bottleneck | Symptom | Solution |
|------------|---------|----------|
| **Disk I/O** | Slow folder loading | Enable SSD caching, increase cache size |
| **CPU Decoding** | High CPU usage | Enable GPU acceleration |
| **Memory Pressure** | System slowdown | Reduce cache size, limit max image dimensions |
| **Thread Contention** | Stuttering thumbnails | Adjust thread pool size |
| **Network Latency** | NAS/SMB delays | Pre-cache with `--cache-warmup` |

---

## Hardware Acceleration

### GPU Acceleration (Direct3D 11)

**What it accelerates:**
- Image decompression (JPEG, WebP, AVIF)
- Color space conversion (YUV → RGB, HDR → SDR)
- Image resizing (bilinear/bicubic)
- 3D model rendering

**Performance gain:**
- **5x faster:** Standard images (JPEG, PNG, WebP)
- **10x faster:** Large images (RAW, TIFF, PSD)
- **15x faster:** 3D models (OBJ, STL, GLTF)

---

### Enabling GPU Acceleration

**Default:** Enabled (with WARP fallback)

**Verify current status:**
```powershell
Get-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" -Name UseGPU
# Output: UseGPU : 1
```

**Enable GPU (if disabled):**
```powershell
Set-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" -Name UseGPU -Value 1
```

**Disable GPU (force CPU):**
```powershell
Set-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" -Name UseGPU -Value 0
# Use case: GPU driver issues, remote desktop
```

---

### GPU Hardware Requirements

**Minimum:**
- DirectX 11 Feature Level 11.0
- 512 MB VRAM
- Any NVIDIA/AMD/Intel GPU from 2012+

**Recommended:**
- DirectX 11 Feature Level 11.1
- 2 GB VRAM
- NVIDIA GTX 1050+ / AMD RX 560+ / Intel Iris Xe

**Fallback:** WARP software renderer (D3D11 on CPU)
- Used when no GPU detected
- 2-3x faster than pure CPU decoding
- Automatic, no configuration needed

---

### Testing GPU Acceleration

```powershell
# Run benchmark with GPU
cd "C:\Program Files\ExplorerLens"
.\EngineBenchmark.exe --benchmark_filter="GPU"

# Expected output:
# DecodeWebP_GPU    102 ms  (vs 520 ms CPU)
# DecodeAVIF_GPU     85 ms  (vs 680 ms CPU)
# ResizeLarge_GPU    12 ms  (vs  95 ms CPU)
```

---

### GPU Profiling

**Tools:**
- **NVIDIA Nsight Graphics**: https://developer.nvidia.com/nsight-graphics
- **AMD Radeon GPU Profiler**: https://gpuopen.com/rgp/
- **Intel GPA**: https://www.intel.com/content/www/us/en/developer/tools/graphics-performance-analyzers/overview.html
- **PIX (Microsoft)**: https://devblogs.microsoft.com/pix/

**Example with PIX:**
```powershell
# Capture GPU workload
"C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\WinPixGpuCapturer.exe" `
    -captureGPU -programToCapture "explorer.exe"

# Open .wpix file in PIX for analysis
```

---

## Cache Optimization

### Cache Architecture

**ExplorerLens uses a 2-level cache:**

1. **Memory Cache (L1):**
   - In-process LRU cache
   - Stores decoded bitmaps
   - Size: Dynamic (up to 100 MB)
   - Eviction: Least Recently Used

2. **Disk Cache (L2):**
   - Persistent file-based cache
   - Location: `C:\ProgramData\ExplorerLens\Cache\`
   - Size: Configurable (default 500 MB)
   - Format: PNG thumbnails with metadata

---

### Cache Size Configuration

**View current size:**
```powershell
Get-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" -Name CacheSizeMB
# Output: CacheSizeMB : 500
```

**Adjust cache size:**
```powershell
# Small system (4 GB RAM, 128 GB SSD)
Set-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" -Name CacheSizeMB -Value 250

# Standard system (8 GB RAM, 512 GB SSD)
Set-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" -Name CacheSizeMB -Value 500

# High-performance system (16+ GB RAM, 1+ TB SSD)
Set-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" -Name CacheSizeMB -Value 2048

# Unlimited cache (not recommended)
Set-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" -Name CacheSizeMB -Value 0
```

**Recommendation:**
- **General use:** 500 MB (~5,000 thumbnails)
- **Photographers:** 1-2 GB (~10,000-20,000 thumbnails)
- **Video editors:** 2-4 GB (video thumbnails are larger)

---

### Cache Location

**Default:** `C:\ProgramData\ExplorerLens\Cache\`

**Change location (e.g., to faster SSD):**
```powershell
# Create new cache directory
New-Item "D:\FastSSD\ExplorerLensCache" -ItemType Directory

# Update registry
Set-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" -Name CachePath `
    -Value "D:\FastSSD\ExplorerLensCache"

# Restart Explorer
Stop-Process -Name explorer -Force
Start-Process explorer.exe
```

**Best practices:**
- Use SSD (not HDD) for cache
- Avoid network drives
- Separate partition from OS (if possible)

---

### Cache Eviction Policy

**LRU (Least Recently Used):**
- Thumbnails sorted by last access time
- When cache > Max size → delete oldest entries
- Runs every 24 hours or when 110% full

**Manual cache clearing:**
```powershell
# Clear entire cache
Remove-Item "C:\ProgramData\ExplorerLens\Cache\*" -Recurse -Force

# Clear cache for specific extension
Remove-Item "C:\ProgramData\ExplorerLens\Cache\*.webp.thumb" -Force
```

---

### Cache Hit Rate Analysis

**Check cache statistics:**
```powershell
cd "C:\Program Files\ExplorerLens"
.\EngineBenchmark.exe --cache-stats

# Output:
# Cache Hits: 8,523 (87.2%)
# Cache Misses: 1,251 (12.8%)
# Cache Size: 412 MB / 500 MB (82.4% full)
# Avg Lookup Time: 2.3 ms
```

**Target hit rate:** 85-95%
- **<70%:** Increase cache size
- **>98%:** Cache may be too large (wasted disk space)

---

### Pre-Caching (Cache Warmup)

**Use case:** Pre-generate thumbnails for large folders

```powershell
cd "C:\Program Files\ExplorerLens"

# Pre-cache a photo library
.\EngineBenchmark.exe --cache-warmup "C:\Photos"

# Pre-cache with progress
.\EngineBenchmark.exe --cache-warmup "C:\Photos" --verbose

# Output:
# Scanning folder: C:\Photos (5,234 files)
# Generating thumbnails: [=========>  ] 52% (2,721 / 5,234)
# Elapsed: 3m 42s | Remaining: 3m 18s
# Cache Hit Rate: 12% (first pass expected)
```

**Recommended scenarios:**
- After installing ExplorerLens
- New photo library import
- Network drive browsing
- Scheduled maintenance (weekly task)

---

## Thread Pool Tuning

### Thread Pool Architecture

**ExplorerLens uses a fixed-size thread pool:**
- **Worker threads:** Decode thumbnails in parallel
- **Default size:** CPU core count (detected automatically)
- **Queue depth:** 256 pending requests

---

### Thread Count Configuration

**View current setting:**
```powershell
Get-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" -Name MaxThreads
# Output: MaxThreads : 8
```

**Adjust thread count:**
```powershell
# Low-end system (4 cores)
Set-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" -Name MaxThreads -Value 4

# Mid-range system (8 cores)
Set-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" -Name MaxThreads -Value 8

# High-end system (16+ cores)
Set-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" -Name MaxThreads -Value 16
```

**Guidelines:**

| Storage Type | Recommended Threads |
|--------------|---------------------|
| **HDD (5400 RPM)** | 2-4 (avoid thrashing) |
| **HDD (7200 RPM)** | 4-6 |
| **SATA SSD** | Match CPU cores |
| **NVMe SSD** | 1.5x CPU cores |
| **Network drive** | 2-4 (avoid overwhelming NAS) |

---

### Thread Pool Profiling

**Detect thread contention:**
```powershell
# Run benchmark with thread profiling
.\EngineBenchmark.exe --benchmark_filter="Thread" --benchmark_repetitions=10

# Output shows optimal thread count:
# 2 threads:  2.1 sec
# 4 threads:  1.3 sec
# 8 threads:  0.9 sec (optimal)
# 16 threads: 1.1 sec (diminishing returns)
# 32 threads: 1.4 sec (contention overhead)
```

---

## Memory Management

### Memory Usage Overview

**Typical memory footprint:**
- **Base (idle):** 10-20 MB
- **Active (decoding):** 50-150 MB
- **Cached (100 thumbnails):** +10 MB
- **Large image (50 MP RAW):** +200 MB (temporary)

---

### Max Image Dimensions

**Purpose:** Prevent out-of-memory errors on huge images

**Default:** 50 MP (megapixels)

**View current limit:**
```powershell
Get-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" -Name MaxImagePixels
# Output: MaxImagePixels : 50000000 (50 MP)
```

**Adjust limit:**
```powershell
# Low memory system (4 GB RAM)
Set-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" -Name MaxImagePixels -Value 25000000  # 25 MP

# Standard system (8 GB RAM)
Set-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" -Name MaxImagePixels -Value 50000000  # 50 MP

# High memory system (16+ GB RAM)
Set-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" -Name MaxImagePixels -Value 150000000 # 150 MP

# Unlimited (not recommended)
Set-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" -Name MaxImagePixels -Value 0
```

**Example limits:**
- 25 MP = 5,000 × 5,000 pixels
- 50 MP = 7,071 × 7,071 pixels
- 100 MP = 10,000 × 10,000 pixels
- 150 MP = 12,247 × 12,247 pixels

---

### Max File Size

**Purpose:** Skip extremely large files (slow network transfers)

**Default:** Unlimited

**Set file size limit:**
```powershell
# Skip files > 100 MB
Set-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" -Name MaxFileSizeMB -Value 100

# Skip files > 500 MB
Set-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" -Name MaxFileSizeMB -Value 500

# No limit (default)
Set-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" -Name MaxFileSizeMB -Value 0
```

**Use case:** Network drives with large files (RAW video, gigapixel panoramas)

---

### Memory Pressure Handling

**ExplorerLens monitors system memory:**
- **<20% free RAM** → Reduce cache size dynamically
- **<10% free RAM** → Skip thumbnail generation
- **Critical pressure** → Clear in-memory cache

**Manual memory cleanup:**
```powershell
# Force garbage collection
.\EngineBenchmark.exe --gc

# Clear in-memory cache only (disk cache persists)
.\EngineBenchmark.exe --clear-memory-cache
```

---

## Network Drive Optimization

### Problem: Slow NAS/SMB Thumbnails

**Causes:**
1. Network latency (file read delays)
2. No local caching (repeated network requests)
3. Too many concurrent operations (network congestion)

---

### Solution 1: Pre-Cache Thumbnails

```powershell
# Pre-generate thumbnails for network share
cd "C:\Program Files\ExplorerLens"
.\EngineBenchmark.exe --cache-warmup "\\NAS\Photos"

# Schedule weekly pre-caching
$action = New-ScheduledTaskAction -Execute "PowerShell.exe" `
    -Argument "-File C:\Scripts\PreCacheNAS.ps1"
$trigger = New-ScheduledTaskTrigger -Weekly -At 2am
Register-ScheduledTask -TaskName "ExplorerLens PreCache" -Action $action -Trigger $trigger
```

---

### Solution 2: Reduce Thread Count

```powershell
# Avoid overwhelming NAS
Set-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" -Name MaxThreads -Value 2
```

**Effect:** Fewer concurrent file reads (less network congestion)

---

### Solution 3: Increase Cache Size

```powershell
# Store more thumbnails locally
Set-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" -Name CacheSizeMB -Value 2048
```

**Effect:** Reduces need to re-fetch from network

---

### Solution 4: Skip Large Files

```powershell
# Don't transfer huge files over network
Set-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" -Name MaxFileSizeMB -Value 50
```

---

### Benchmarking Network Performance

```powershell
# Measure network drive performance
.\EngineBenchmark.exe --benchmark_filter="Network" `
    --network-path="\\NAS\Photos"

# Output:
# Local SSD:       92 ms per thumbnail
# NAS (cached):   105 ms per thumbnail
# NAS (uncached): 842 ms per thumbnail (8x slower)
```

---

## Decoder-Specific Tuning

### Video Decoder Settings

#### **Seek Position** (default: 10%)

**Purpose:** Extract thumbnail from video at specific timestamp

**Default:** Seek to 10% of video duration

**Adjust:**
```powershell
# Seek to 25% (skip intro sequences)
Set-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" -Name VideoSeekPercent -Value 25

# Seek to 5% (for short clips)
Set-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" -Name VideoSeekPercent -Value 5
```

---

#### **Hardware Decoding** (experimental)

**Enable GPU video decode (NVDEC/VCE/Quick Sync):**
```powershell
Set-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" -Name VideoHWAccel -Value 1
# Note: Requires FFmpeg with hwaccel support
```

---

### RAW Decoder Settings

#### **Use Embedded Thumbnails** (default: OFF)

**OFF (default):** Decode full RAW image (high quality, slower)  
**ON:** Extract embedded JPEG preview (lower quality, 5x faster)

**Enable embedded thumbnails:**
```powershell
Set-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" -Name RAWUseEmbedded -Value 1
```

**Comparison:**
| Mode | Quality | Speed | File Size |
|------|---------|-------|-----------|
| **Full decode** | Native (14-bit) | 500-1000 ms | 50 MP RAW |
| **Embedded** | JPEG (8-bit) | 100-200 ms | 2 MP preview |

---

### Archive Decoder Settings

#### **Preview Mode** (default: First file)

**Modes:**
- `0`: First image/video in archive
- `1`: Largest image in archive
- `2`: Montage of first 4 images (2×2 grid)

**Set mode:**
```powershell
# Montage mode (comic book archives)
Set-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" -Name ArchivePreviewMode -Value 2
```

---

#### **Max Files to Extract** (default: 1)

**Purpose:** Limit extraction for large archives

**Set limit:**
```powershell
# Extract up to 4 files for montage
Set-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" -Name ArchiveMaxFiles -Value 4
```

---

## Benchmarking

### Running Benchmarks

**Full benchmark suite:**
```powershell
cd "C:\Program Files\ExplorerLens"
.\EngineBenchmark.exe

# Output:
# ==================================================
# ExplorerLens Benchmark Suite - Version 15.0.0
# ==================================================
# System: Intel Core i7-10700K, 16 GB RAM
# GPU: NVIDIA RTX 3060 (12 GB VRAM)
# ==================================================
# 
# DecodeJPEG_Small      12.5 ms   80.0 MB/s
# DecodeJPEG_Large      89.3 ms  112.1 MB/s
# DecodeWebP_Lossy      24.1 ms   41.5 MB/s
# DecodeWebP_Lossless   52.8 ms   18.9 MB/s
# DecodeAVIF_10bit     104.7 ms    9.5 MB/s
# DecodeJXL_Lossy       38.2 ms   26.2 MB/s
# DecodeRAW_CR3        523.1 ms   19.1 MB/s
# DecodeVideo_MP4      215.4 ms    4.6 MB/s
# ...
```

---

### Filtering Benchmarks

```powershell
# Run specific decoder tests
.\EngineBenchmark.exe --benchmark_filter="WebP"

# Run GPU tests only
.\EngineBenchmark.exe --benchmark_filter="GPU"

# Run cache tests
.\EngineBenchmark.exe --benchmark_filter="Cache"
```

---

### Benchmark Output Formats

**JSON export:**
```powershell
.\EngineBenchmark.exe --benchmark_format=json --benchmark_out=results.json
```

**CSV export:**
```powershell
.\EngineBenchmark.exe --benchmark_format=csv --benchmark_out=results.csv
```

---

### Comparing Performance

**Before/after optimization:**
```powershell
# Baseline (CPU only)
.\EngineBenchmark.exe --benchmark_out=baseline.json

# Enable GPU
Set-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" -Name UseGPU -Value 1

# Measure with GPU
.\EngineBenchmark.exe --benchmark_out=gpu_enabled.json

# Compare results
.\EngineBenchmark.exe --benchmark_compare=baseline.json,gpu_enabled.json

# Output:
# DecodeWebP_Large:  520 ms → 102 ms (5.1x speedup)
# DecodeAVIF_10bit:  680 ms →  85 ms (8.0x speedup)
```

---

## Profiling

### CPU Profiling (Visual Studio)

1. **Attach to Explorer:**
   - Debug → Performance Profiler → Attach to Process
   - Select `explorer.exe`
   - Choose "CPU Usage" profiler

2. **Trigger thumbnail generation:**
   - Navigate to folder with images
   - Enable thumbnails view

3. **Analyze results:**
   - Hot Path: `LENSShell.dll!CThumbnailProvider::GetThumbnail`
   - Look for bottlenecks in decoder functions

---

### Memory Profiling (CRT Debug Heap)

**Enable memory leak detection:**
```powershell
# Rebuild with debug CRT
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug

# Run tests with leak detection
.\build\bin\Debug\EngineTests.exe

# Output shows any memory leaks at exit
```

---

### GPU Profiling (PIX)

**Capture D3D11 calls:**
```powershell
# Launch PIX
$pix = "C:\Program Files\Microsoft PIX\PIX.exe"
& $pix

# File → Attach to Process → explorer.exe
# GPU Capture → Start
# Navigate to folder with images
# GPU Capture → Stop

# Analyze:
# - Draw calls per thumbnail
# - Texture upload bandwidth
# - Shader execution time
```

---

### Thread Profiling (ETW)

**Capture ETW trace:**
```powershell
# Start trace (admin required)
xperf -on PROC_THREAD+LOADER+PROFILE -stackwalk Profile

# Trigger thumbnail generation
# (Navigate folders in Explorer)

# Stop trace
xperf -stop -d thumbnail_trace.etl

# Analyze in Windows Performance Analyzer (WPA)
wpa thumbnail_trace.etl
```

---

## Performance Tips Summary

### Quick Wins (5 minutes)

1. **Enable GPU acceleration:**
   ```powershell
   Set-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" -Name UseGPU -Value 1
   ```

2. **Increase cache size:**
   ```powershell
   Set-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" -Name CacheSizeMB -Value 1024
   ```

3. **Pre-cache photo library:**
   ```powershell
   .\EngineBenchmark.exe --cache-warmup "C:\Photos"
   ```

---

### Advanced Optimizations (30 minutes)

1. **Move cache to fast SSD**
2. **Tune thread count** (benchmark different values)
3. **Enable RAW embedded thumbnails** (if quality acceptable)
4. **Profile with PIX/Nsight** (find decoder bottlenecks)

---

### System-Specific Tuning

**Low-end system (4 GB RAM, HDD):**
```powershell
Set-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" -Name UseGPU -Value 0  # Use WARP
Set-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" -Name CacheSizeMB -Value 100
Set-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" -Name MaxThreads -Value 2
Set-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" -Name MaxImagePixels -Value 25000000
```

**High-end system (32 GB RAM, NVMe SSD, RTX 4090):**
```powershell
Set-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" -Name UseGPU -Value 1
Set-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" -Name CacheSizeMB -Value 4096
Set-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" -Name MaxThreads -Value 24
Set-ItemProperty "HKLM:\SOFTWARE\ExplorerLens" -Name MaxImagePixels -Value 150000000
```

---

**Next Steps:**
- [Benchmarking Results](BENCHMARK_RESULTS.md) - Reference performance data
- [User Manual](USER_MANUAL.md) - General configuration guide
- [Troubleshooting](TROUBLESHOOTING.md) - Fix performance issues

---

**Last Updated:** June 2025  
**Sprint:** 177 - Version Normalization  
**Version:** 15.0.0

