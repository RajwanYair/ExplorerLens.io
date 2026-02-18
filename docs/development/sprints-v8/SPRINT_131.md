# Sprint 131 — Buffer Pool & Slab Allocator

## Objective
Implement a slab-based buffer pool for decode buffer reuse, reducing heap churn during Explorer thumbnail batch operations with dimension-classified buffers.

## Scope
- **File**: `Engine/Memory/BufferPoolAllocator.h`
- **Tests**: `tests/Sprint131_BufferPoolAllocator.cpp` (13 tests)

## Key Components
| Component | Purpose |
|-----------|---------|
| `SlabClass` | 6 size classes: Tiny (16KB) → Huge (16MB) |
| `ClassifyDimension()` | Maps width×height to appropriate slab class |
| `SlabPool` | Per-class thread-safe free-list with configurable max |
| `BufferPool` | Aggregate manager across all 6 slab classes |
| `BufferPoolConfig` | 3 presets: Default, LowMemory (16MB), HighThroughput (128MB) |
| `PooledBuffer` | RAII handle with pool-originating metadata |

## Design Decisions
- 4-byte-per-pixel (BGRA) buffer sizing matches DirectX texture formats
- Thread-safe with per-class mutex granularity
- Free list capped per class to prevent unbounded retention
- Optional zero-on-release for security-sensitive modes

## Test Coverage
- Dimension classification across all slab classes
- Acquire/release lifecycle validation
- Buffer reuse (pointer identity check)
- Drain operations
- Config preset values
- Statistics and summary formatting

## Dependencies
- Integrates with Sprint 130 (DecoderHotsetManager) for per-decoder buffer allocation

## Status: COMPLETE
