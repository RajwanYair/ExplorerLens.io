# Sprint 137 — Continuous Fuzzing Engine

## Objective
Implement automated fuzz testing framework with crash budget gating, corpus management, mutation strategies, and per-decoder malformed payload hardening.

## Scope
- **File**: `Engine/Utils/ContinuousFuzzEngine.h`
- **Tests**: `tests/Sprint137_ContinuousFuzzEngine.cpp` (14 tests)

## Key Components
| Component | Purpose |
|-----------|---------|
| `MutationStrategy` | 9 strategies: BitFlip through Havoc |
| `ByteMutator` | Deterministic byte-level mutations with PRNG seed |
| `CrashBudget` | Zero-tolerance and lenient crash/timeout/leak limits |
| `FuzzConfig` | Quick (1K iter) and Full (100K iter) campaign presets |
| `ContinuousFuzzEngine` | Campaign runner with corpus and statistics |

## Design Decisions
- SEH wrapper pattern for crash isolation (production implementation)
- Deterministic PRNG for reproducible mutation sequences
- Crash budget gating for CI integration (zero-tolerance default)
- Corpus-based fuzzing with interesting input tracking
- Per-decoder mutation targeting for format-specific coverage

## Test Coverage
- Mutation strategy naming and execution
- Crash budget exhaustion semantics
- Byte mutator determinism and size changes
- Campaign configuration presets
- Engine single execution and statistics
- Corpus management

## Status: COMPLETE
