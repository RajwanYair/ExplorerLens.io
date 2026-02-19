# Sprint 158: ARM64 Performance Baseline

**Block:** v8.3.0 — Phase P2: ARM64 Foundation  
**Status:** ✅ Done  
**Sprint Count:** 158 / 174

---

## Overview

Establishes quantitative performance baseline targets for DarkThumbs running on Windows ARM64.
Defines acceptable degradation thresholds relative to x64, measurement methodology, and
acceptance gate constants.

---

## Deliverables

| Artifact | Path | Notes |
|---|---|---|
| Header | `Engine/Utils/ARM64PerformanceBaseline.h` | Baseline targets, threshold constants, measurement types |
| GTest | `Engine/Tests/Sprint158_ARM64PerfBaseline.cpp` | 12 test cases |
| Sprint doc | `docs/development/sprints-v8/SPRINT_158.md` | This document |

---

## Baseline Targets

| Metric | x64 | ARM64 Target | Tolerance |
|---|---|---|---|
| Single thumbnail decode | 17 ms | ≤ 25 ms | +47% |
| Batch throughput | 235 img/s | ≥ 160 img/s | −32% |
| Cache hit latency | < 5 ms | < 8 ms | +60% |
| Peak memory / decode | < 50 MB | < 50 MB | Same |

---

## Tests (12)

- `ARM64PerfBaseline_SingleThumbMaxMs` — ≤ 25ms constant
- `ARM64PerfBaseline_BatchMinImgPerSec` — ≥ 160 img/s constant
- `ARM64PerfBaseline_CacheHitMaxMs` — < 8ms constant
- `ARM64PerfBaseline_PeakMemMaxMB` — < 50MB constant
- `ARM64PerfBaseline_ThroughputTolerancePct` — -32% max degradation
- `ARM64PerfBaseline_LatencyTolerancePct` — +47% max degradation
- `ARM64PerfBaseline_X64BaselineDefined` — x64 values present for comparison
- `ARM64PerfBaseline_MeasurementConfig` — config struct well-formed
- `ARM64PerfBaseline_ThresholdCheck_Pass` — value within threshold passes
- `ARM64PerfBaseline_ThresholdCheck_Fail` — value over threshold fails
- `ARM64PerfBaseline_ReportGenerated` — report struct non-empty
- `ARM64PerfBaseline_BaselineVersion` — baseline version string defined

---

## Acceptance Criteria

- [x] Header compiles with `/W4` zero warnings
- [x] All threshold constants defined
- [x] Baseline/target table documented
- [x] All 12 GTest cases pass
- [x] Sprint doc created
