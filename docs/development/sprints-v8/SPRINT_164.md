# Sprint 164: Format Fallback Intelligence Engine

**Block:** v8.3.0 — Phase P3: Format Expansion  
**Status:** ✅ Done  
**Sprint Count:** 164 / 174

---

## Overview

Introduces the `FormatFallbackEngine` — an intelligent routing layer that selects the optimal
decoder chain for a given file format, with automatic fallback when a primary decoder fails.
Closes Phase P3 (Format Expansion).

---

## Deliverables

| Artifact | Path | Notes |
|---|---|---|
| Header | `Engine/Pipeline/FormatFallbackEngine.h` | `FallbackTrigger` flags, `FormatFallbackChain`, `FormatFallbackEngine` |
| GTest | `Engine/Tests/Sprint164_FallbackEngine.cpp` | 13 test cases |
| Sprint doc | `docs/development/sprints-v8/SPRINT_164.md` | This document |

---

## Fallback Triggers

| Flag | Value | Meaning |
|---|---|---|
| `None` | 0x00 | No fallback needed |
| `DecodeError` | 0x01 | Primary decoder returned error |
| `Timeout` | 0x02 | Decode exceeded time budget |
| `MemoryPressure` | 0x04 | Insufficient memory for primary decoder |
| `FormatMismatch` | 0x08 | File extension ≠ actual format |
| `PluginUnavailable` | 0x10 | Required plugin not loaded |

---

## Tests (13)

- `FallbackEngine_TriggerFlags` — all 6 flags defined and unique
- `FallbackEngine_FallbackStageFields`
- `FallbackEngine_ChainCreation` — FormatFallbackChain add/query
- `FallbackEngine_ChainLength`
- `FallbackEngine_CreateDefaultForJXL` — JXL chain has ≥2 stages
- `FallbackEngine_CreateDefaultForHEIC` — HEIC chain has ≥2 stages
- `FallbackEngine_CreateDefaultForRAW` — RAW chain has ≥1 stage
- `FallbackEngine_CreateDefaultForCAD` — CAD chain has ≥1 stage
- `FallbackEngine_SelectDecoder_Primary` — primary decoder selected first
- `FallbackEngine_SelectDecoder_Fallback` — fallback selected on trigger
- `FallbackEngine_EventRecord` — FallbackEvent fields
- `FallbackEngine_EmptyChain` — empty chain handled gracefully
- `FallbackEngine_EngineInstantiation`

---

## Acceptance Criteria

- [x] Header compiles with `/W4` zero warnings
- [x] 6 trigger flags as bitmask
- [x] Default chains for JXL/HEIC/RAW/CAD
- [x] `FormatFallbackEngine::CreateDefault()` factory
- [x] All 13 GTest cases pass
- [x] Sprint doc created

---

## Phase P3 Closure

Sprint 164 closes Phase P3 (Format Expansion). Sprints 160–164 are all complete.
