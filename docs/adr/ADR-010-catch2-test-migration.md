# ADR-010: Catch2 as Primary Test Framework

**Status:** Proposed  
**Version:** Target: v40.0.0+ (not yet implemented — custom `TEST()`/`RUN_TEST()` framework active)  
**Date:** 2026-07-19 (planned)  

## Context

ExplorerLens has ~5108 unit tests using a custom macro-based framework (`TEST()`, `RUN_TEST()`,
`ASSERT()` with counters) defined in `EngineTestsMacros.h`. While functional, this framework has
limitations:

- **No test discovery** — tests must be manually registered via `extern void` + `RUN_TEST()`
- **No parameterized tests** — each variant requires a separate `TEST()` body
- **No BDD-style sections** — nested test contexts require manual setup/teardown
- **No built-in benchmarking integration** — Google Benchmark linked separately
- **Limited CI integration** — no JUnit XML output for GitHub Actions
- **Manual skip mechanism** — no `SKIP()` equivalent for conditional tests
- **No expression decomposition** — `ASSERT(a == b)` doesn't show `a` and `b` values on failure

Catch2 v3.7.1 was added in an earlier sprint but disabled by default (`BUILD_CATCH2_TESTS=OFF`).
Eight Catch2 test files already exist in `Engine/Tests/Catch2Tests/`.

## Decision

Enable Catch2 as the **primary test framework** for all new tests, with the custom framework
retained for existing tests (no mass migration).

### Specifics

1. `BUILD_CATCH2_TESTS` defaults to `ON` in `Engine/Tests/CMakeLists.txt`
2. All new test files go into `Engine/Tests/Catch2Tests/`
3. Existing `EngineTests*.cpp` files remain unchanged (custom macros)
4. Corpus validation tests use Catch2 exclusively (`CorpusValidationTests.cpp`)
5. Future decoder tests, integration tests, and regression tests use Catch2

## Rationale

- **Incremental adoption** — no risky mass rewrite of 4744 tests
- **Better CI output** — Catch2 reporters produce JUnit XML for GitHub Actions
- **Expression decomposition** — failed assertions show actual vs expected values
- **SKIP() support** — corpus tests gracefully skip when test files are absent
- **BDD sections** — `SECTION()` blocks allow structured test organization
- **Active maintenance** — Catch2 is industry-standard with regular releases

## Consequences

### Positive
- New tests are faster to write and more expressive
- CI reports show which assertion failed and why
- Corpus validation tests exercise real I/O with graceful degradation
- Both test executables (`EngineTests` and `EngineCatch2Tests`) run in CTest

### Negative
- Two test executables in the build (minor build time impact)
- Developers must know both frameworks during the transition period
- Catch2 header-only compile adds ~3-5s to first build

## Alternatives Considered

| Alternative | Why Rejected |
|-------------|-------------|
| Google Test (GTest) | Heavier dependency, less expressive matchers |
| Mass migration to Catch2 | Too risky for 4744 tests; would block development |
| Keep custom framework only | Lacks CI integration, skip, expression decomposition |
| doctest | Slightly faster compile but smaller ecosystem than Catch2 |
