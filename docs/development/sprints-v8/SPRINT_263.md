# Sprint 263: Async Shell Extension

**Date:** 2026-02-20
**Version:** v11.1.0
**Phase:** Phase 3 — Performance Activation

## Objective
Activate AsyncThumbnailProvider for non-blocking Explorer integration. Priority-based decode scheduling (Critical/High/Normal/Low/Idle) with timeout management and cancel-on-scroll support.

## Deliverables
- `Engine/Core/AsyncShellActivation.h` — Async shell extension activation
- AsyncDecodeState enum (8 states: Queued through Cancelled)
- DecodePriority enum (5 priorities: Critical through Idle)
- AsyncProviderConfig with concurrent limits, queue capacity, timeouts
- EffectiveTimeout() — priority-scaled timeout calculation
- 5 unit tests

## Test Results
- TestAsync_StateNames ✅
- TestAsync_PriorityNames ✅
- TestAsync_Counts ✅
- TestAsync_ValidateConfig ✅
- TestAsync_Timeout ✅
