# Sprint 240 — ResourcePoolEngine

**Date:** 2026-06-15
**Component:** `Engine/Core/ResourcePoolEngine.h`, `Engine/Core/ResourcePoolEngine.cpp`
**Theme:** Object Pool for Decoder/GPU Resource Reuse

## Summary
Implemented a generic object pool engine that manages reusable resources for decoder contexts, GPU textures, render targets, compute/staging buffers, and command lists. Supports checkout/return semantics, TTL-based eviction, prewarming, and pool hit-rate statistics.

## Key Types
- `ResourceType` — 6 resource categories (DecoderContext, GPUTexture, RenderTarget, ComputeBuffer, StagingBuffer, CommandList)
- `ResourceState` — Available, InUse, Expired, Corrupted
- `PoolConfig` — maxPoolSize, minPoolSize, TTL, growthFactor, prewarming toggle
- `PooledResource` — id, type, state, timestamps, checkout count, size
- `PoolStats` — created/destroyed/checkout/return counters, cache hit rate

## Tests Added (5)
- `TestResourcePool_Checkout` — checkout allocates and marks InUse
- `TestResourcePool_Return` — return makes resource Available
- `TestResourcePool_Stats` — statistics track checkouts
- `TestResourcePool_TypeNames` — all 6 type names resolve
- `TestResourcePool_Prewarm` — prewarm creates requested count

## Files Modified
- `Engine/CMakeLists.txt` — registered header + source
- `Engine/Tests/EngineTests.cpp` — 5 tests + RUN_TEST calls
