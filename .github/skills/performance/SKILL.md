# ExplorerLens — Performance Skill

## Purpose

Use this skill when profiling decoder performance, writing benchmarks, setting P50/P95
targets, or diagnosing regressions. The performance baseline is defined in
`Engine/Tests/benchmarks/baseline.json` and enforced by `perf-regression.yml`.

---

## When to Use This Skill

- Writing a Google Benchmark test for a new decoder
- Investigating why a P95 benchmark regressed >10%
- Setting performance targets for a new format
- Profiling CPU hotspots in the decode pipeline
- Adding WIC + D3D11 hints to a decoder path
- Tuning the cache hit path

---

## Performance Targets (Phase 1/2)

| Format | Library | P50 Target | P95 Target | Notes |
|--------|---------|-----------|-----------|-------|
| JPEG (6MP) | libjpeg-turbo | < 5 ms | < 10 ms | EXIF orient applied |
| PNG (4K) | WIC / libpng | < 5 ms | < 12 ms | |
| WebP | libwebp | < 8 ms | < 15 ms | |
| AVIF | libavif + dav1d | < 10 ms | < 20 ms | |
| HEIC | libheif | < 10 ms | < 20 ms | |
| JXL | libjxl | < 12 ms | < 25 ms | |
| PDF (first page) | MuPDF | < 20 ms | < 40 ms | |
| RAW (embedded preview) | LibRaw | < 25 ms | < 50 ms | unpack_thumb() path |
| ZIP cover image | minizip-ng | < 15 ms | < 30 ms | |
| Cache hit (L1) | LRU | < 1 ms | < 2 ms | |
| Cache hit (L2 disk) | SQLite + mmap | < 5 ms | < 10 ms | |

---

## Step-by-Step: Write a Benchmark

```cpp
// Engine/Tests/benchmarks/DecoderBenchmarks.cpp
#include <benchmark/benchmark.h>
#include "../Decoders/JpegDecoder.h"
#include "BenchmarkHelper.h"  // LoadCorpusFile()

namespace ExplorerLens { namespace Engine {

static void BM_JpegDecoder_6MP(benchmark::State& state) {
    auto data = LoadCorpusFile("images/jpeg/test-6mp.jpg");
    JpegDecoder decoder;
    for (auto _ : state) {
        auto stream = MakeIStreamFromSpan(data);
        auto result = decoder.DecodeAtSize(stream.Get(), 256, std::stop_token{});
        benchmark::DoNotOptimize(result);
    }
    state.SetLabel("JPEG 6MP → 256px");
    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(data.size()));
}
BENCHMARK(BM_JpegDecoder_6MP)->Unit(benchmark::kMillisecond)->Iterations(100);

}} // namespace

BENCHMARK_MAIN();
```

### Run benchmarks

```powershell
# Build with Release config (required for meaningful numbers)
.\\build-scripts\\Build-MSVC.ps1 -Clean

# Run all benchmarks
.\\build\\bin\\EngineBenchmarks.exe --benchmark_format=json --benchmark_out=baseline.json

# Compare against stored baseline
.\\build\\bin\\EngineBenchmarks.exe --benchmark_format=json | compare.py baseline.json -
```

---

## Step-by-Step: Diagnose a Regression

1. **Reproduce in Release** — Debug builds are 5-20× slower; never benchmark Debug.
2. **Isolate to one decoder** — run single-format benchmark with `--benchmark_filter=BM_Jpeg`.
3. **Profile with VS Profiler:**
   ```powershell
   # Attach VS Profiler to EngineTests.exe during benchmark run
   # Look for: libjpeg internals, libheif decode loops, MuPDF page rendering
   ```
4. **Check for allocation hotspots** — use `_CrtSetAllocHook` or Heap Profiler.
5. **Check cache effectiveness** — run `lens cache stats` before and after.
6. **Common root causes:**

| Symptom | Root Cause | Fix |
|---------|-----------|-----|
| JPEG P95 > 15ms | Full YCbCr transform instead of fast path | Use `jpeg_calc_output_dimensions` + `scale_num/scale_denom` |
| PNG slow on large files | Decompressing at full resolution | Use WIC `IWICBitmapScaler` with `WICBitmapInterpolationModeFant` |
| RAW > 100ms | Full demosaic instead of thumbnail | Call `unpack_thumb()` not `dcraw_process()` |
| Cache miss rate > 50% | Key not matching on re-open | Verify SHA256 includes mtime + file size |
| Heap fragmentation | Many small allocations per decode | Use pool allocator or stack buffer for < 4 KB |

---

## Step-by-Step: Update the Baseline

After a performance improvement is validated:

```powershell
# Generate new baseline
.\\build\\bin\\EngineBenchmarks.exe --benchmark_format=json `
    --benchmark_out=Engine/Tests/benchmarks/baseline.json

# Verify improvements
git diff Engine/Tests/benchmarks/baseline.json

# Include in version bump
.\\build-scripts\\Bump-Version.ps1 -Version "X.Y.Z" ...
# (Bump-Version.ps1 automatically updates baseline.json _comment and version fields)
```

---

## CI Performance Gate

`perf-regression.yml` blocks PRs where any P95 regresses > 10%:

```yaml
- name: Run benchmarks
  run: .\\build\\bin\\EngineBenchmarks.exe --benchmark_format=json --benchmark_out=current.json
- name: Compare against baseline
  run: python scripts/compare-benchmarks.py Engine/Tests/benchmarks/baseline.json current.json --threshold 0.10
```

**Do not bypass this gate** by updating the baseline without a justification comment.

---

## GPU Acceleration Path (Phase 2)

When adding WIC + D3D11 hints:

```cpp
// Use IWICImagingFactory2 with D3D11 device for GPU-assisted decode
ComPtr<IWICImagingFactory2> wicFactory;
CoCreateInstance(CLSID_WICImagingFactory2, nullptr, CLSCTX_INPROC_SERVER,
                 IID_PPV_ARGS(&wicFactory));

ComPtr<ID3D11Device> d3dDevice;
// ... create D3D11 device ...

// IWICImagingFactory2::CreateImageEncoderFromWicBitmap with D3D device hints
// This allows WIC to use DXGI surface sharing for zero-copy GPU decode
```

Target: 1.5-2× speedup for JPEG/PNG compared to CPU path. Measure SSIM ≥ 0.99.

---

## Required Constraints

1. **All benchmarks run Release config only** — never commit Debug benchmark results.
2. **Baseline must be updated** when a new format decoder is added.
3. **P95 regression > 10% blocks merge** — fix before merging.
4. **Benchmark file naming**: `BM_<Format>_<ScenarioSize>` (e.g., `BM_Jpeg_6MP`).
5. **Units in milliseconds** — use `->Unit(benchmark::kMillisecond)`.
6. **No I/O in benchmark hot loop** — pre-load file into memory before `for (auto _ : state)`.

---

## Validation Checklist

- [ ] Benchmark runs in Release config and shows expected P50/P95
- [ ] New baseline.json committed with version bump
- [ ] CI perf gate passing (no >10% P95 regression)
- [ ] Benchmark properly uses `DoNotOptimize` to prevent dead-code elimination
- [ ] GPU path (if added) shows measurable speedup with SSIM ≥ 0.99

---

## Compile-Time Profiling (`/d1reportTime`)

Use MSVC's `/d1reportTime` flag to identify slow-to-compile headers and template instantiations.
This is critical for a 500+ header codebase where compile time directly affects developer productivity.

### Quick Profile a Single TU

```powershell
# Source vcvars and compile one file with timing report
& "$env:VSINSTALLDIR\VC\Auxiliary\Build\vcvars64.bat"
cl.exe /c /EHsc /std:c++20 /O2 /d1reportTime Engine/Core/DecodePipeline.cpp 2>&1 |
    Select-String "time\(" | Sort-Object { [double]($_ -replace '.*time\(([0-9.]+).*','$1') } -Descending |
    Select-Object -First 20
```

### Profile the Entire Engine Build

```powershell
# CMake with /d1reportTime injected via CMAKE_CXX_FLAGS
cmake --preset default-release -DCMAKE_CXX_FLAGS="/d1reportTime"
cmake --build build --config Release 2>&1 | Tee-Object -FilePath build-logs/compile-time-report.txt

# Parse slowest includes
Select-String "time\(" build-logs/compile-time-report.txt |
    Sort-Object { [double]($_ -replace '.*time\(([0-9.]+).*','$1') } -Descending |
    Select-Object -First 30 | Format-Table
```

### What to Look For

| Metric | Threshold | Action |
|--------|-----------|--------|
| Single header > 500ms | Investigate | May need PCH inclusion or forward declarations |
| Template instantiation > 200ms | Investigate | Consider explicit instantiation or PIMPL |
| PCH generation > 5s | Monitor | Normal for large PCH; split if > 15s |
| Full rebuild > 120s | Investigate | Check for unnecessary transitive includes |

### Common Fixes for Slow Compile Times

1. **Move heavy headers to PCH** — add to `Engine/pch.h` if included by >50% of TUs
2. **Forward-declare instead of include** — `class Foo;` instead of `#include "Foo.h"` in headers
3. **Explicit template instantiation** — instantiate in `.cpp` to avoid per-TU instantiation
4. **`#pragma once` on all headers** — already enforced, but verify with:
   ```powershell
   Get-ChildItem Engine/**/*.h -Recurse | Where-Object {
       -not (Get-Content $_.FullName -First 1 | Select-String '#pragma once')
   }
   ```
5. **Include-what-you-use analysis** — remove unnecessary transitive includes

### Tracking Over Time

Store compile-time snapshots in `data/baselines/compile-times.json`:

```json
{
  "_comment": "Compile-time profiling baseline — ExplorerLens v39.9.0",
  "_updated": "2025-01-01T00:00:00Z",
  "full_rebuild_seconds": 95,
  "incremental_seconds": 12,
  "slowest_headers": [
    { "header": "Engine/pch.h", "time_ms": 3200 },
    { "header": "Engine/Decoders/AllDecoders.h", "time_ms": 850 }
  ]
}
```
