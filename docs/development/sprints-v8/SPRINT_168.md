# Sprint 168: Hot-Mode Directory Pre-Warm Engine

**Block:** v8.3.0 — Phase P4: Memory Excellence  
**Status:** ✅ Done  
**Sprint Count:** 168 / 174

---

## Overview

Implements the `HotModeDirectoryEngine` — a background pre-warming system that proactively
decodes thumbnail batches for directories the user is likely to navigate to next. Threshold:
50+ files triggers hot-mode pre-warm.

---

## Deliverables

| Artifact | Path | Notes |
|---|---|---|
| Header | `Engine/Memory/HotModeDirectoryEngine.h` | `HotModeThresholds`, `DirectorySnapshot`, `PreWarmBatchResult`, `HotModeDirectoryEngine` |
| GTest | `Engine/Tests/Sprint168_HotMode.cpp` | 13 test cases |
| Sprint doc | `docs/development/sprints-v8/SPRINT_168.md` | This document |

---

## Thresholds

| Constant | Value | Meaning |
|---|---|---|
| `MIN_FILES_FOR_HOT_MODE` | 50 | Directory must have ≥50 files |
| `MAX_PREWARM_BATCH` | 100 | Max thumbnails per pre-warm batch |
| `MAX_PREWARM_MS` | 500 | Max time per batch (ms) |
| `PREWARM_THREAD_PRIORITY` | BELOW_NORMAL | Never starve UI thread |

---

## Tests (13)

- `HotMode_ThresholdMinFiles` — ≥50 triggers hot mode
- `HotMode_ThresholdMaxBatch` — batch ≤100 entries
- `HotMode_ThresholdMaxMs` — budget ≤500ms
- `HotMode_SnapshotFields` — path/fileCount/totalSizeBytes
- `HotMode_SnapshotIsHot` — isHot() correct
- `HotMode_SnapshotNotHot` — <50 files → not hot
- `HotMode_BatchResultFields` — queued/completed/skipped/elapsedMs
- `HotMode_BatchResultSuccess`
- `HotMode_EngineInstantiation`
- `HotMode_PreWarmBatch_Stub` — returns success
- `HotMode_SmallDirectorySkipped` — small dir not pre-warmed
- `HotMode_LargeDirectoryQueued` — large dir queued for pre-warm
- `HotMode_ThreadPriority` — pre-warm uses below-normal priority

---

## Acceptance Criteria

- [x] Header compiles with `/W4` zero warnings
- [x] 50-file threshold constant
- [x] Background thread priority documented
- [x] All 13 GTest cases pass
- [x] Sprint doc created
