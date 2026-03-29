# ADR-008: C++23 Migration Strategy

**Status:** Proposed  
**Date:** 2026-03-29  
**Planned start:** v30.0.0 "Deneb"  
**Affected components:** All Engine headers and sources

---

## Context

ExplorerLens Engine currently targets C++20. MSVC v145 (cl.exe 19.50) ships with near-complete
C++23 support. Several C++23 features would significantly improve code clarity, safety,
and performance across the Engine:

- `std::expected<T,E>` — eliminates HRESULT patterns for decoder return types  
- `std::stacktrace` — richer crash diagnostics without external dependencies  
- Standardised `std::print` / `std::println` — replaces custom logging macros  
- `std::flat_map` / `std::flat_set` — cache-friendly alternatives to `std::map`  
- Deducing `this` — eliminates CRTP boilerplate in plugin interfaces  
- `[[assume(expr)]]` — compiler hint for performance-critical decode loops  

## Decision

Adopt C++23 as the Engine's language standard starting with v30.0.0, following this
incremental migration plan:

**Phase 1 — v30.0.0 (non-breaking additions):**
- Set `CMAKE_CXX_STANDARD 23` in Engine/CMakeLists.txt
- Replace all `HRESULT`-returning decoder functions with `std::expected<Result, LensError>`
- Add `[[nodiscard]]` audit pass (enforce on all bool/HRESULT return paths)
- Apply `[[assume]]` hints on hot decode loops (LibRaw demosaic, PNG filter)

**Phase 2 — v30.2.0 (data structure improvements):**
- Replace `std::map<std::wstring, CacheEntry>` with `std::flat_map` in hot-path caches
- Use `std::flat_set` for LENSArchive format lookup (sorted by enum value)
- Apply deducing `this` to eliminate CRTP in `IThumbnailPlugin` hierarchy

**Phase 3 — v30.5.0 (diagnostics + logging):**
- Integrate `std::stacktrace` into the crash reporter
- Replace `OutputDebugStringW` logging with `std::print` + structured log sink

## Rationale

| Feature | Benefit | Risk |
|---------|---------|------|
| `std::expected` | Eliminates `E_INVALIDARG`/`E_FAIL` ambiguity | Low — additive change |
| `std::flat_map` | 2–3× cache miss improvement on hot paths | Low — drop-in replacement |
| `[[assume]]` | ~5% decode loop speedup | Medium — UB if wrong |
| `std::stacktrace` | Instant crash context | Low — std library call |

MSVC v145 supports all targeted features. CI uses `windows-latest` (VS 2022 or 2026)
which also supports C++23, so no CI regression is expected.

## Consequences

**Positive:**
- `std::expected` eliminates the most error-prone HRESULT propagation patterns
- `std::flat_map` improves cache hit rates on the 256-entry format lookup table
- Decoder return types become self-documenting (no more comment-only error semantics)

**Negative:**
- MSVC IntelliSense for C++23 features is occasionally laggy (not a build issue)
- External plugin SDK must document that plugin ABI still uses C++20 for compatibility
- Initial migration PR will be large — plan for a dedicated review slot

## Compatibility

The public Plugin SDK (`SDK/plugin_api.h`) remains `extern "C"` C ABI — no impact on
plugin developer toolchain requirements. The C++23 migration is internal Engine only.

The `Engine/CMakeLists.txt` change (`CMAKE_CXX_STANDARD 23`) applies only to the Engine
static library target. LENSShell.dll and LENSManager.exe remain on C++20 until validated.
