# Sprint 129 — Directory Format Profiler
- **Phase:** N2 (Explorer Memory Optimization) | **Date:** 2026-02-18 | **Status:** Implemented
## Objective
Extension histogram per folder, dominant-format detection (>80% threshold), single-format hot mode activation with per-family memory budgets.
## Deliverables
1. FormatFamily enum (12 families + Unknown).
2. DirectoryProfile with histogram, dominant ratio, single-format mode flag.
3. FamilyMemoryBudget (decoder footprint, per-thumbnail, max working set, concurrency limits).
4. 60+ extension→family mappings.
5. 13 GTest cases.
## Files
- `Engine/Memory/DirectoryFormatProfiler.h` (NEW)
- `tests/Sprint129_DirectoryFormatProfiler.cpp` (NEW)
