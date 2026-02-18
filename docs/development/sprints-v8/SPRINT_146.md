# Sprint 146 — Performance Regression CI Gate V2

**Status:** Complete  
**Phase:** N5 — CI & Release Hardening  
**Component:** `Engine/Core/PerfRegressionGate.h`

## Objective
CI-enforceable performance regression gate with KPI thresholds, baseline regression detection, trend analysis, and pass/warn/fail verdict for automated quality gates.

## Deliverables
- `Engine/Core/PerfRegressionGate.h` — Header-only performance gate engine
- `tests/Sprint146_PerfRegressionGate.cpp` — 14 GTest cases

## Key Features
- **PerfKPI** — 8 performance KPIs covering latency, throughput, memory, GPU, and decoder init
- **KpiThreshold** — Per-KPI warn/fail thresholds with direction (LowerIsBetter/HigherIsBetter) and regression percentage tolerance
- **PerfSample** — Timestamped measurement with commit hash linkage
- **GateResult** — Aggregated pass/warn/fail counts with overall verdict
- **TrendStats** — Mean, stddev, min, max, trend slope with degradation detection
- **PerfRegressionGate** — Default thresholds matching DarkThumbs KPIs (17ms single, 235 img/sec batch, <5ms cache), baseline regression detection, linear trend analysis

## Performance Targets (defaults)
| KPI | Warn | Fail |
|-----|------|------|
| SingleThumbnailMs | 15.0 | 25.0 |
| BatchThroughputImgSec | <200 | <150 |
| CacheHitMs | 3.0 | 8.0 |
| ColdStartMs | 100.0 | 200.0 |
| MemoryPeakMB | 150.0 | 250.0 |
