# ADR-014 — Safe Integer Arithmetic for Decode Dimensions

**Status:** Accepted  
**Date:** 2026-04-23  
**Version:** v38.7.0 "Betelgeuse"  
**Author:** ExplorerLens Engineering  
**ROADMAP:** §15.1 (P0 Security Hardening Phase 1), OWASP A4/A8

---

## Context

Image decoders routinely multiply `width × height × bytesPerPixel` to compute
buffer sizes.  In C++ this arithmetic is performed on `uint32_t` values; if
`width = 65536` and `height = 65536`, the product silently wraps around to 0
(CVE-class CWE-190).  The resulting zero-length allocation is immediately
written past, causing heap corruption or memory disclosure.

**Affected operations in ExplorerLens:**

| Operation | Risk |
|-----------|------|
| `WICDecodeMetadataQueryReader` dimension read → buffer alloc | TIFF width=65535 produces 0-byte alloc |
| `IWICBitmapSource::CopyPixels` stride computation | `stride = width * 4` wraps at 2^30 on 32-bit path |
| `ScaleToFit` intermediate multiply | unchecked 64-bit interim can overflow with extreme aspect ratios |
| `LibRaw::dcraw_make_mem_image` output size | external library returns `uint32_t` dimensions |

ExplorerLens v38.6.0 and earlier performed all dimension arithmetic without
overflow checks.  Security audit (§15.1) classified this as P0 — must harden
in Phase 1 before any public corpus-validation release.

---

## Decision

Introduce `Engine/Core/SafeDimensions.h` — a header-only, pure C++20 library
that provides:

1. **Project constants**: `kMaxThumbDimension` (32768), `kMaxThumbPixels`, `kMaxThumbBytes` — single source of truth for all decode budget limits.

2. **`SafeMul2` / `SafeMul3` / `SafeAdd`**: inline constexpr helpers that return `std::optional<uint64_t>`, never wrapping.  Callers treat `nullopt` as an invalid/malicious input.

3. **`SafeDim`**: a value-checked single dimension type.  Only constructible via `SafeDim::Make(v)` — rejects 0 and values > 32768.

4. **`SafeDimensions`**: a validated `(width, height)` pair.  `SafeDimensions::Make(w, h)` checks all four invariants (per-axis range, pixel-count budget, byte-count budget) and returns `std::optional<SafeDimensions>`.  Provides `PixelCount()`, `ByteCount()`, `RowStride()`, `FitsIn()`, `ScaleToFit()` — all safe.

---

## Rationale

### Why `std::optional` instead of exceptions?

- ExplorerLens runs inside `explorer.exe`; uncaught exceptions from a COM DLL
  will crash the shell.  All failure paths must be `noexcept` or return error
  codes, not throw.

- `std::optional<T>` is the idiomatic C++17/20 way to express "may not exist".
  It composes naturally with `if (!dims)` early-exit patterns the codebase
  already uses for `HRESULT` checks.

### Why a separate header rather than inline in each decoder?

- **Single source of truth for constants**: max-dimension budget was previously
  repeated in `InputValidationTests.cpp`, `Engine/Core/BuildValidation.h`, and
  at least 3 decoder files — all with slightly different values.

- **SDK boundary**: `SafeDimensions.h` ships in the plugin SDK.  Plugin authors
  get the same overflow-safe budget checks the core engine uses, for free.

- **Testability**: header-only means full Catch2 test coverage without linking
  the full Engine (which requires MSVC v145 and the COM/WinRT stack).

### Why cap at 32768×32768?

| Limit | Rationale |
|-------|-----------|
| `32768` per axis | 2^15; comfortably fits in `uint32_t` half-range; no legitimate thumbnail needs > 8K edge |
| `268 M pixels` total | ≈ 8K × 8K × 4 bytes = 1 GiB; exceeds any practical thumbnail; hard ceiling against allocation bombs |
| `1 GiB` byte budget | Matches Windows LPTR heap limit inside `explorer.exe` per-process decode budget |

### Why not the Windows `SafeInt.h` (CRT)?

- `SafeInt.h` is Windows-only and throws `SafeIntException` by default.
- `Engine/Core/SafeDimensions.h` is cross-platform (macOS Quick Look, Linux
  Nautilus — Phase 5/6 targets) and `noexcept` throughout.
- `SafeDimensions` encodes domain knowledge (thumbnail budget, pixel/byte
  limits) that generic SafeInt templates do not express.

---

## Consequences

### Positive

- Eliminates the integer-overflow class of CVEs from ExplorerLens thumbnail
  decode for all callers that adopt `SafeDimensions::Make`.
- Compile-time constants replace 5+ independent copies of the same magic numbers.
- 50+ Catch2 tests (`SafeDimensionsTests.cpp`) provide continuous regression
  coverage — impossible with raw arithmetic.
- Ships in the plugin SDK → third-party decoders benefit automatically.
- `noexcept` + `constexpr` throughout — zero runtime cost vs unchecked arithmetic
  for the happy path; the `std::optional` carries only a 1-byte discriminator.

### Negative / Constraints

- **Adoption is not automatic**: existing decoder code must be updated to call
  `SafeDimensions::Make` before allocating buffers.  This is incremental work
  tracked in §15.1 — all P0 decoders (JPEG, PNG, WebP, AVIF, HEIC, JXL, PDF,
  RAW) must adopt by Phase 1 exit.

- **`ScaleToFit` uses integer division**: aspect-ratio rounding is floor-based.
  Callers that need exact scaling (e.g. WIC `IWICBitmapScaler`) must apply their
  own rounding after the safe result.

---

## Alternatives Considered

| Alternative | Verdict |
|-------------|---------|
| Use Windows CRT `SafeInt<T>` | Rejected: Windows-only, throws exceptions |
| Use checked-integer library (checked_int, Microsoft GSL narrow) | GSL `narrow` covers int narrowing but not multiply; adds a GSL dependency |
| Inline asserts / `__assume` | Do not fire in Release builds; no observable error return |
| Let each decoder clamp independently | Creates inconsistent caps and duplicate constant definitions |

---

## Related Decisions

| ADR | Topic |
|-----|-------|
| ADR-010 | Catch2 as primary test framework — test infrastructure that validates this ADR |
| ADR-011 | IStreamingDecoder `ProbeHeader`/`DecodeAtSize` — callers pass `SafeDimensions` for target size |
| D31 | `std::optional` for new Engine APIs (ROADMAP §20) |

## References

- CWE-190: Integer Overflow or Wraparound
- OWASP A4: Insecure Design
- OWASP A8: Software and Data Integrity Failures
- ROADMAP.md §15.1 Security Hardening roadmap
- `Engine/Core/SafeDimensions.h` — implementation
- `Engine/Tests/Catch2Tests/SafeDimensionsTests.cpp` — test suite
