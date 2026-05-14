---
applyTo: "**/Engine/**,**/benchmarks/**"
---

# Performance Rules — ExplorerLens

## Decode Performance Targets

| Format | P50 Target | Library | Fast Path |
| -------- | ----------- | --------- | ----------- |
| JPEG | < 5 ms | libjpeg-turbo | WIC HW hint |
| PNG | < 5 ms | WIC / libpng | — |
| WebP | < 8 ms | libwebp | — |
| AVIF | < 10 ms | libavif + dav1d | — |
| HEIC | < 10 ms | libheif + libde265 | — |
| JXL | < 12 ms | libjxl | — |
| PDF | < 20 ms | MuPDF | First page only |
| RAW | < 25 ms | LibRaw | **Embedded JPEG** (100× faster) |
| Archive | < 15 ms | minizip-ng | First image alphabetically |

**NEVER** decode more data than needed for the requested thumbnail size.

## Memory Budgets

| Component | Budget |
| ----------- | -------- |
| L1 cache (in-process) | 64 MB max |
| Peak decode working set | < 128 MB |
| Idle (LENSShell.dll loaded) | < 10 MB |
| LENSShell.dll binary size | < 5 MB |

## Two-Phase Decode (Mandatory Pattern)

```cpp
// Phase 1: ProbeHeader (≤ 16 KB read, metadata only — fast)
DecodeResult probe = decoder->ProbeHeader(headerSlice);
if (!probe.success) return probe;

// Phase 2: DecodeAtSize (minimal decode at target size)
DecodeResult result = decoder->DecodeAtSize(stream, targetSize, cancelToken);
```

Never skip Phase 1. It avoids full I/O for unsupported/corrupt files.

## Cache Usage (Mandatory Before Decode)

```cpp
// Always check L1 then L2 before decoding
auto cacheKey = CacheKey{path, mtime, size, targetSize};
if (auto hit = l1Cache_->Get(cacheKey)) return *hit;
if (auto hit = l2Cache_->Get(cacheKey)) {
    l1Cache_->Put(cacheKey, *hit);  // promote to L1
    return *hit;
}
// Miss — decode and store
auto result = decoder->DecodeAtSize(...);
l1Cache_->Put(cacheKey, result);
l2Cache_->Put(cacheKey, result);
return result;
```

## Benchmark Writing

Use Google Benchmark format in `Engine/Tests/benchmarks/`:

```cpp
static void BM_JpegDecode_6MP(benchmark::State& state) {
    const auto data = LoadCorpusFile("images/jpeg/6mp-standard.jpg");
    for (auto _ : state) {
        auto result = JpegDecoder{}.DecodeAtSize(data, 256, {});
        benchmark::DoNotOptimize(result);
    }
    state.SetBytesProcessed(state.iterations() * data.size());
}
BENCHMARK(BM_JpegDecode_6MP)->Unit(benchmark::kMillisecond);
```

## CI Performance Gate

- P95 regression > 10% → PR blocked
- Benchmark results stored in `Engine/Tests/benchmarks/baseline.json`
- Compare with `--benchmark_out_format=json --benchmark_compare_with_previous`

## Profiling (When Investigating Regressions)

```powershell
# 1. Build with /d1reportTime to find slow-compiling headers
cmake --build --preset default-release -- /p:CL_FLAGS="/d1reportTime" 2>&1 | Select-String "time="

# 2. Profile with ETW (production)
xperf -on PROC_THREAD+LOADER+PROFILE -stackwalk Profile -buffersize 1024
# ... reproduce the slow scenario ...
xperf -d explorerlens.etl
wpa explorerlens.etl

# 3. Quick timing in lens.exe
lens benchmark <directory> --output benchmark.json
```

## Anti-Patterns

- ❌ Decoding full RAW files when an embedded JPEG preview exists
- ❌ Allocating full-resolution bitmaps then scaling down (decode at target size instead)
- ❌ Synchronous file I/O on the COM thread (use async I/O or pre-read cache)
- ❌ Loading entire archive to get first image (seek directly to file entry)
- ❌ Not checking stop_token — long decodes must be cancellable
