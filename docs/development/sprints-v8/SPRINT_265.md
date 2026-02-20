# Sprint 265: Parallel Batch Decode

**Date:** 2026-02-20  
**Version:** v11.1.0  
**Phase:** Phase 3 — Performance Activation

## Objective
Activate ParallelBatchDecoder with thread pool and per-format concurrency control. 5 scheduling policies (RoundRobin, FormatGrouped, SizeOrdered, PriorityBased, Adaptive) for optimal batch processing.

## Deliverables
- `Engine/Core/ParallelBatchProcessor.h` — Parallel batch decode processor
- BatchPolicy enum with 5 scheduling policies
- BatchDecodeStats for throughput/timing metrics
- OptimalThreadCount() based on CPU core count
- 5 unit tests

## Test Results
All 5 tests passing ✅
