# ADR-012: Engine Directory Consolidation — 16 Subdirectories to 7

**Status:** Accepted
**Version:** v38.4.0 (scheduled for Phase 1 exit)
**Date:** 2026-04-23
**ROADMAP Reference:** §7.2, §17.1, Decision D32

## Context

As of v38.4.0 the `Engine/` directory contains **16 subdirectories**, many of which are near-empty
stub folders created speculatively for features not yet implemented:

| Subdirectory | Files | Status | Disposition |
| ------------- | ------- | -------- | ------------- |
| `Core/` | ~50 | Active — decode pipeline, registry, observability | **Keep** |
| `Decoders/` | ~45 | Active — 25+ format decoders | **Keep** |
| `GPU/` | ~8 | Active — vendor routing, D3D11 stubs | **Keep** |
| `Cache/` | ~12 | Active — L1 LRU, L2 scaffold | **Keep** |
| `Platform/` | ~10 | Active — Win32 PAL + macOS/Linux stubs | **Keep** |
| `Tests/` | ~30 | Active — unit tests, benchmarks | **Keep** |
| `Utils/` | ~15 | Active — release gates, installer lifecycle | **Keep** |
| `AI/` | ~6 | **Stub** — speculative; no inference runtime | Fold → `Core/` |
| `Enterprise/` | ~4 | **Stub** — GPO/ADMX stubs | Fold → `Core/` |
| `Media/` | ~5 | **Stub** — video scrubber, timeline | Fold → `Decoders/` |
| `Memory/` | ~8 | Partial — footprint optimizer, pressure controller | Fold → `Cache/` |
| `Pipeline/` | ~6 | **Stub** — fallback engine, zero-copy upload | Fold → `Core/` |
| `Plugin/` | ~7 | Partial — trust chain, sandbox scaffold | Fold → `Core/` |
| `PluginHost/` | ~4 | **Stub** — duplicate of `Plugin/` scope | Fold → `Core/` |
| `CLI/` | ~3 | **Empty** — moved to `src/Tools.CLI/` | Delete |
| `Codec/` | ~5 | **Stub** — duplicate of decoder routing | Fold → `Decoders/` |

The current **header:source ratio is ~5.1:1** (1,386 headers / 269 sources). ROADMAP Phase 1 exit
criterion requires ≤ 1.6:1. The inflated ratio comes primarily from stub headers with no `.cpp`
counterpart.

## Decision

Consolidate the 16 subdirectories into **7 target directories** over the course of Phase 1:

```text
Engine/
├── Core/        ← pipeline + routing + observability + enterprise + plugin host + AI stubs
├── Decoders/    ← all format decoders + video frame extraction (absorbs Media/ + Codec/)
├── GPU/         ← D3D11 compute + vendor routing (NVDEC, QuickSync, AMF) + future Vulkan
├── Cache/       ← L1 LRU + L2 SQLite + mmap blobs (absorbs Memory/)
├── Platform/    ← Win32 PAL (macOS Phase 5, Linux Phase 6)
├── Tests/       ← Catch2 + custom harness + benchmarks + corpus runner
└── Utils/       ← release gates + installer lifecycle + shared helpers
```

### Migration process

1. **No renaming of headers** in the first pass — move files physically, update `#include` paths
1. **Update `ENGINE_HEADERS` and `ENGINE_SOURCES`** in `Engine/CMakeLists.txt` after each move
1. **Zero-warnings build required** after each subdirectory fold before proceeding to the next
1. **Test pass rate must remain 100%** throughout the migration

### Priority order

| Step | Action | Risk |
| ------ | -------- | ------ |
| 1 | Delete `Engine/CLI/` (empty) | Low |
| 2 | Fold `Engine/AI/` → `Engine/Core/AI/` | Low (stubs only) |
| 3 | Fold `Engine/Enterprise/` → `Engine/Core/Enterprise/` | Low (stubs only) |
| 4 | Fold `Engine/Pipeline/` → `Engine/Core/Pipeline/` | Medium |
| 5 | Fold `Engine/Plugin/` + `Engine/PluginHost/` → `Engine/Core/Plugin/` | Medium |
| 6 | Fold `Engine/Codec/` → `Engine/Decoders/` | Medium |
| 7 | Fold `Engine/Media/` → `Engine/Decoders/Media/` | Low |
| 8 | Fold `Engine/Memory/` → `Engine/Cache/Memory/` | Medium |

### Implement-before-declare rule (for ratio reduction)

Alongside the directory consolidation, every header moved from a stub directory must either:

- Gain a corresponding `.cpp` implementation (preferred), OR
- Be deleted if it declares only empty/TBD stubs, OR
- Be merged into an existing active header

Tracking metric: checked against `Engine/CMakeLists.txt` ENGINE_SOURCES count after each sprint.

## Rationale

- **Premature subdivision** created cognitive overhead without delivery value
- **Header:source ratio** at 5.1:1 exceeds any credible C++ project (ratio > 2:1 signals stubs)
- **ROADMAP Phase 1 exit criterion** explicitly requires 7 subdirectories and ≤ 1.6:1 ratio
- **Developer onboarding** is harder when 9 of 16 directories contain only stubs
- **CMakeLists.txt** ENGINE_HEADERS list is 83 KB and hard to navigate (allowlisted in file-size policy)

## Consequences

### Positive

- Clearer ownership per directory — each of 7 has an active deliverable
- `Engine/CMakeLists.txt` list shrinks as stubs are either implemented or deleted
- Phase 1 exit criterion becomes achievable
- New contributors see a legible directory structure

### Negative

- Large git diff during migration — must use `git mv` to preserve history
- All in-repo `#include` paths change → automated `sed`/`grep` pass required
- Downstream users of the SDK include paths change (no external SDK consumers yet in Phase 1)

## Alternatives Considered

| Alternative | Why rejected |
| ------------- | ------------- |
| Keep 16 directories | Perpetuates ratio problem; blocks Phase 1 exit |
| Consolidate to 4 directories | Loses GPU/Platform separation; makes future macOS harder |
| Delete all stubs immediately | Risk breaking builds that include stubs transitively |
| Single `Engine/` flat layout | 300+ files in one directory is unusable |
