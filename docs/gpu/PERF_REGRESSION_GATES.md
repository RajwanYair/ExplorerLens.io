# Performance Regression Gates (Sprint 14)

**Date:** January 6, 2026
**Status:** Draft / Policy

## 1. Philosophy

"Fast is a feature. Regressions are bugs."

DarkThumbs v6.0 introduces strict automated gates. If a PR makes the engine slower than the baseline (without justification), the build fails.

## 2. Metrics Baseline Database

We maintain a JSON database of expected performance characteristics for canonical hardware profiles (High-end Desktop, Ultrabook, VM).

**File:** `tests/data/perf_baseline.v1.json`

```json
{
  "profiles": {
    "dev_workstation_generic": {
      "avif_4k_decode_ms": { "target": 12.0, "tolerance": 1.5 },
      "psd_100mb_load_ms": { "target": 85.0, "tolerance": 5.0 },
      "cold_start_time_ms": { "target": 150.0, "tolerance": 10.0 },
      "gpu_resident_memory_mb": { "target": 32.0, "tolerance": 5.0 }
    }
  }
}
```

## 3. The Performance Budget

We define "budgets" for critical user interactions.

| Interaction | Budget (P95) | Hard Limit (Blocker) |
|---|---|---|
| **Thumbnail Visiblity (Cache Hit)** | 16ms (60fps) | 50ms |
| **Thumbnail Generation (Small Image)** | 50ms | 200ms |
| **Thumbnail Generation (Large RAW)** | 500ms | 2000ms |
| **Worker Process Startup** | 100ms | 300ms |

## 4. Regression Gate Logic

The CI pipeline runs a `BenchmarkJob`:

1. Run GoogleBenchmark suite.
2. Load `perf_baseline.v1.json`.
3. Compare `Actual vs Target`.
4. If `Actual > Target + Tolerance`: **BUILD FAILED**.
   - Output: `[PERF FAILURE] AVX2 Blur Filter took 4.5ms (Limit: 3.0ms)`

## 5. Updating Baselines

When we legitimately add heavy features or optimize code:

1. Run the benchmark tool locally: `DarkThumbs.Perf.exe --update-baseline`
2. Commit the updated JSON.
3. PR Reviewer must explicitly approve the "performance cost".

## 6. Telemetry Monitoring

In production (Manager App Dashboard), we track:

- `Mean Time To Texture (MTTT)`
- `Cache Hit Ratio`
- `Scheduler Queue Depth`

If global MTTT spikes > 20% in a new release, a "Performance Incident" is raised.
