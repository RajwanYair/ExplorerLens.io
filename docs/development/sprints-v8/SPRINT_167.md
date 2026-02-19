# Sprint 167: Adaptive Cache Budget Manager

**Block:** v8.3.0 — Phase P4: Memory Excellence  
**Status:** ✅ Done  
**Sprint Count:** 167 / 174

---

## Overview

Introduces the `AdaptiveCacheBudgetManager` — a dynamic memory allocator for the three-tier
cache (hot/warm/cold). Monitors system memory pressure and rebalances tier budgets
automatically to maximize cache hit rate while preventing OOM conditions.

---

## Deliverables

| Artifact | Path | Notes |
|---|---|---|
| Header | `Engine/Cache/AdaptiveCacheBudgetManager.h` | `CacheTier`, `TierBudget`, `SystemMemorySnapshot`, `AdaptiveCacheBudgetManager` |
| GTest | `Engine/Tests/Sprint167_AdaptiveCache.cpp` | 11 test cases |
| Sprint doc | `docs/development/sprints-v8/SPRINT_167.md` | This document |

---

## Cache Tiers

| Tier | Default Budget | Purpose |
|---|---|---|
| Hot | 128 MB | Most recently accessed thumbnails |
| Warm | 256 MB | Frequently accessed, older thumbnails |
| Cold | 128 MB | Compressed / low-priority entries |
| **Total** | **512 MB** | Default total budget |

---

## Tests (11)

- `AdaptiveCache_TierValues` — Hot/Warm/Cold tiers
- `AdaptiveCache_TierBudgetFields` — tier/allocatedBytes/maxBytes/hitCount
- `AdaptiveCache_DefaultBudget` — 512MB total constant
- `AdaptiveCache_SnapshotPressureLevel` — PressureLevel() returns [0.0, 1.0]
- `AdaptiveCache_LowPressureRebalance` — low pressure expands hot tier
- `AdaptiveCache_HighPressureRebalance` — high pressure shrinks cold tier
- `AdaptiveCache_BudgetSumInvariant` — sum of tiers == total always
- `AdaptiveCache_ManagerInstantiation`
- `AdaptiveCache_RebalanceTrigger`
- `AdaptiveCache_HitCountWeighting`
- `AdaptiveCache_ExtremeMemoryPressure` — min budget per tier defines floor

---

## Acceptance Criteria

- [x] Header compiles with `/W4` zero warnings
- [x] 3 cache tiers with default 512MB total
- [x] Budget sum invariant maintained
- [x] All 11 GTest cases pass
- [x] Sprint doc created
