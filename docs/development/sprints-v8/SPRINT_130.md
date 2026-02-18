# Sprint 130 — Decoder Hotset Manager

## Objective
Implement a decoder hotset manager that loads/unloads decoders on demand based on directory format profiling, idle eviction timers, and memory budgets.

## Scope
- **File**: `Engine/Memory/DecoderHotsetManager.h`
- **Tests**: `tests/Sprint130_DecoderHotsetManager.cpp` (12 tests)

## Key Components
| Component | Purpose |
|-----------|---------|
| `DecoderLoadState` | 5-state lifecycle: Cold → Warming → Hot → Cooling → Evicted |
| `DecoderHotsetEntry` | Per-decoder metadata: state, memory, load time, use count |
| `HotsetMode` | 4 activation strategies: AllDecoders, DominantOnly, TopN, OnDemand |
| `HotsetConfig` | 3 presets: Aggressive (64 MB), Balanced (128 MB), Conservative (256 MB) |
| `DecoderHotsetManager` | Core manager — load/unload, eviction, statistics |

## Design Decisions
- 8 pre-registered decoders covering all major format families
- Extension → decoder mapping for automatic activation
- Idle eviction scoring: priority = timeSinceLastUse / (useCount + 1)
- Memory budget enforcement with utilization percentage tracking

## Test Coverage
- Load/unload lifecycle
- Extension-based activation
- Memory statistics and budget tracking
- Config preset validation
- Idle candidate priority ordering

## Dependencies
- Sprint 129 (DirectoryFormatProfiler) — provides format distribution input

## Status: COMPLETE
