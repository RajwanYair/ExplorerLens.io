---
mode: ask
description: "Analyze benchmark results from Google Benchmark JSON output. Identifies regressions, compares to baseline.json, and suggests root causes."
---

# Benchmark Analysis — ExplorerLens

Analyze the benchmark results for **{{benchmarkName}}** and compare against the baseline.

## Step 1: Read Baseline

```powershell
Get-Content Engine\Tests\benchmarks\baseline.json | ConvertFrom-Json |
    Select-Object -ExpandProperty benchmarks |
    Where-Object { $_.name -like "*{{benchmarkFilter}}*" } |
    Format-Table name, real_time, cpu_time, iterations
```

## Step 2: Run Current Benchmarks

```powershell
# Build and run benchmarks
.\build-scripts\Build-MSVC.ps1
.\build\bin\EngineTests.exe --benchmark_filter={{benchmarkFilter}} `
    --benchmark_out=benchmark-current.json `
    --benchmark_out_format=json
```

## Step 3: Compare Results

For each benchmark, calculate the regression:

```text
Regression % = ((current_p50 - baseline_p50) / baseline_p50) × 100

Thresholds:
  > +10%  P95 → BLOCKING regression — must fix before merge
  > +5%   P95 → WARNING — investigate before merge
  < 0%           → Improvement — update baseline.json
```

## Step 4: Identify Root Cause

If a regression is found, investigate:

1. **Memory allocation**: Did we add a vector/string copy on the hot path?
2. **Cache misses**: Did struct layout change (check `sizeof` output)?
3. **Lock contention**: Did we add a mutex on a frequently called path?
4. **Extra I/O**: Did we read more bytes from the file than before?
5. **Decoder path change**: Was the fast path (e.g., embedded JPEG in RAW) bypassed?

```powershell
# Quick profile with ETW
xperf -on PROC_THREAD+LOADER+PROFILE -stackwalk Profile -buffersize 2048
.\build\bin\EngineTests.exe --benchmark_filter={{benchmarkFilter}} --benchmark_repetitions=10
xperf -d perf.etl
wpa perf.etl
```

## Step 5: Update Baseline (If Improved)

```powershell
# Copy current results to baseline
Copy-Item benchmark-current.json Engine\Tests\benchmarks\baseline.json
git add Engine\Tests\benchmarks\baseline.json
git commit -m "perf: update benchmark baseline — {{improvement}} improvement in {{benchmarkName}}"
```

## Performance Targets

| Format | P50 Target | P95 Target |
| -------- | ----------- | ----------- |
| JPEG 6MP | < 5 ms | < 10 ms |
| PNG 4K | < 5 ms | < 10 ms |
| WebP | < 8 ms | < 15 ms |
| AVIF | < 10 ms | < 20 ms |
| PDF first page | < 20 ms | < 40 ms |
| Cache hit | < 1 ms | < 2 ms |
