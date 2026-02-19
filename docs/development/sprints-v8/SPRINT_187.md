# Sprint 187: Async Shell Extension

**Date:** 2026-03-15
**Version:** v9.1.0
**Phase:** 3 — Performance & Quality
**Status:** ✅ Complete

## Objective

Implement non-blocking IThumbnailProvider with background decode thread pool to prevent Explorer hangs during thumbnail generation.

## Deliverables

### New Files
- `Engine/Pipeline/AsyncThumbnailProvider.h` — Async provider header with full API
- `Engine/Pipeline/AsyncThumbnailProvider.cpp` — Thread pool implementation

### Key Features
1. **Priority Queue** — 5 levels (Critical/High/Normal/Low/Idle) for decode ordering
2. **Request Deduplication** — Merges duplicate file requests to avoid redundant work
3. **Timeout Enforcement** — Per-request and shell-level timeout to prevent hangs
4. **Synchronous Fallback** — Graceful degradation when thread pool is exhausted
5. **Statistics Dashboard** — Real-time metrics (avg/p99 latency, queue depth, worker count)
6. **Cancel Support** — Cancel by request ID or by file path

### Architecture
- `AsyncThumbnailProvider` manages decode request lifecycle
- `DecodePriority` enum with 5 levels for visible-first ordering
- `RequestState` tracks Queued → InProgress → Completed/Failed/Cancelled/TimedOut
- `AsyncProviderConfig` for tunable thread pool size, queue depth, timeouts
- Thread-safe with `std::mutex` and `std::atomic` counters

### Integration Points
- Registered in `Engine/CMakeLists.txt` (ENGINE_HEADERS + ENGINE_SOURCES)
- 9 unit tests in EngineTests.cpp

## Test Summary

| Test | Purpose |
|------|---------|
| TestAsyncProvider_Create | Default construction, not running |
| TestAsyncProvider_Initialize | Init/shutdown lifecycle |
| TestAsyncProvider_Config | Custom configuration propagation |
| TestAsyncProvider_PriorityNames | Priority enum → string mapping |
| TestAsyncProvider_StateNames | State enum → string mapping |
| TestAsyncProvider_SyncFallback | Synchronous decode for valid files |
| TestAsyncProvider_SyncFallbackEmpty | Empty path returns failure |
| TestAsyncProvider_Stats | Statistics initialization |
| TestAsyncProvider_SubmitNotRunning | Submit before init returns 0 |

## Notes
- Windows Thread Pool (TP_WORK) integration is stubbed for future sprint
- Production integration with CBXShell COM requires STA→MTA marshaling
