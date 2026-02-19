# Sprint 189: Parallel Batch Decode

**Date:** 2026-03-15
**Version:** v9.1.0
**Phase:** 3 — Performance & Quality
**Status:** ✅ Complete

## Objective

Implement thread pool decoder with per-format parallelism controls for batch thumbnail generation.

## Deliverables

### New Files
- `Engine/Pipeline/ParallelBatchDecoder.h` — Batch decode API
- `Engine/Pipeline/ParallelBatchDecoder.cpp` — Format-aware parallelism

### Key Features
1. **Format Classification** — Automatic parallelism level per format
2. **Parallelism Levels** — FullParallel/LimitedParallel/SerialOnly/Adaptive
3. **Batch Priority** — Immediate/Background/CacheWarm ordering
4. **Progress Callbacks** — Per-item progress and batch completion
5. **Format Grouping** — Optimal I/O ordering by file type
6. **Cancel Support** — Cancel entire batches
7. **Statistics** — Throughput, concurrency, failure rates

### Format Parallelism Rules
- **FullParallel**: JPEG, PNG, BMP, GIF, TIFF, WebP, etc.
- **LimitedParallel**: DDS, KTX, RAW (CR2/NEF/ARW), HEIF, AVIF
- **SerialOnly**: ZIP, RAR, 7z, CBZ, CBR, TAR

## Test Summary (10 tests)
- Create, Initialize, Config, ClassifyFormats
- ParallelismNames, StatusNames, PriorityNames
- SubmitEmpty, SubmitBatch, CancelBatch
