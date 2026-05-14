---
applyTo: "**/tests/**,**/conftest.py,**/*_test.py,**/test_*.py"
---

# Testing Instructions

## Framework & Tools

- **Primary**: pytest 7.0+
- **Coverage**: pytest-cov, targeting 90%+
- **Property-based**: hypothesis for complex logic
- **Mocking**: pytest-mock (prefer over unittest.mock directly)
- **Async**: pytest-asyncio for async code

## Test Structure

```text
tests/
├── conftest.py           # Shared fixtures, markers
├── unit/                 # Pure Python, no I/O
│   ├── test_core.py
│   └── test_utils.py
├── integration/          # File/network/OS interaction
│   └── test_integration.py
└── fixtures/             # Test data files
```

## Naming Conventions

```python
# Functions: test_<what>_<when>_<expected>
def test_process_file_with_valid_input_returns_result():
    ...

def test_process_file_with_missing_file_raises_file_not_found_error():
    ...
```

## Fixtures Pattern

```python
import pytest
from pathlib import Path

@pytest.fixture
def temp_dir(tmp_path: Path) -> Path:
    """Provide a temporary directory cleaned up after the test."""
    return tmp_path

@pytest.fixture
def sample_config(temp_dir: Path) -> Path:
    """Create a minimal config file for testing."""
    config = temp_dir / "config.yaml"
    config.write_text("app:\n  debug: true\n")
    return config
```

## Markers — Always Mark Your Tests

```python
@pytest.mark.unit
def test_pure_logic():
    ...

@pytest.mark.integration
def test_with_filesystem(tmp_path):
    ...

@pytest.mark.slow
def test_large_dataset():
    ...

@pytest.mark.windows
def test_registry_operation():
    ...

@pytest.mark.network
def test_api_call():
    ...
```

## Coverage Rules

- Minimum: 80% overall, 90% for core modules
- Exclude: `__main__.py`, `**/gui*.py` display code, `TYPE_CHECKING` blocks
- Run: `pytest --cov=src --cov-report=term-missing --cov-report=html`

## Hypothesis — Property-Based Testing

```python
from hypothesis import given, settings, strategies as st

@given(st.text(min_size=1), st.integers(min_value=0))
@settings(max_examples=100)
def test_process_handles_any_valid_input(text: str, count: int) -> None:
    result = process(text, count)
    assert result is not None
```

## What NOT to Do in Tests

- Don't test implementation details — test behaviour
- Don't use `time.sleep()` — use mocks or events
- Don't leave temporary files — use `tmp_path` fixture
- Don't catch exceptions to silence test failures
- Don't use `assert` on mutable defaults

---

## C++20 Test Conventions (ExplorerLens Engine)

ExplorerLens uses a **custom test framework** — NOT GTest, NOT Catch2.

### Framework Macros (defined in `EngineTestsMacros.h`)

| Macro | Purpose |
| ------- | --------- |
| `TEST(name)` | Define a test function (expands to `void TestName_Runner()`) |
| `ASSERT(cond)` | Assert condition; throws on failure, increments counters |
| `RUN_TEST(name)` | Execute a test, catch failures, print pass/fail |

### Test File Layout

```text
Engine/Tests/
├── EngineTestsIncludes.h      — Shared #include block (all Engine headers)
├── EngineTestsMacros.h        — TEST/ASSERT/RUN_TEST macros + MockDecoder
├── EngineTestsExterns.h       — extern void TestXxx_Runner() declarations
├── EngineTests.cpp            — main() + RUN_TEST() calls
├── EngineTests_Core.cpp       — TEST() bodies: decoder, registry, cache, GPU
├── EngineTests_Features.cpp   — TEST() bodies: feature modules, SIMD, enterprise
├── EngineTests_Mid.cpp        — TEST() bodies: settings, memory, plugin, format
└── EngineTests_Late.cpp       — TEST() bodies: CLI, workflow, AI, platform
```

### Adding a New C++ Test

1. **Include:** Add `#include "../SubDir/Header.h"` to `EngineTestsIncludes.h`
2. **Extern:** Add `extern void TestFeatureName_Runner();` to `EngineTestsExterns.h`
3. **Run call:** Add `RUN_TEST(TestFeatureName);` to `EngineTests.cpp` (before `// Isolation & Stability Tests`)
4. **Body:** Add `TEST(TestFeatureName) { ... }` to `EngineTests_Late.cpp`

### C++ Test Pattern

```cpp
TEST(FeatureName) {
    using namespace ExplorerLens::Engine;

    // Construction & default state
    FeatureName feature;
    ASSERT(feature.GetStats().processed == 0);

    // Initialization
    ASSERT(feature.Initialize());

    // Core behavior
    feature.Process(L"test");
    ASSERT(feature.GetStats().processed > 0);

    // Edge cases
    feature.Process(L"");
    ASSERT(feature.GetStats().avgLatencyMs >= 0.0);
}
```

### C++ Test Rules

- **7-12 ASSERT() calls** per TEST() block — cover construction, methods, edge cases
- **Always `using namespace ExplorerLens::Engine;`** at top of TEST block
- **Parenthesize `(std::min)` / `(std::max)`** to avoid Windows macro conflicts
- **Never use GTest macros** (`EXPECT_*`, `ASSERT_*`, `TEST_F`) in Engine tests
- **After adding RUN_TEST() calls, do a clean build** — stale `.obj` can hide missing bodies
- **Test count:** Update in `Bump-Version.ps1` (`-TestCount`) on each sprint delivery
- **Split threshold:** When any test split file exceeds 500 KB, split again at `//==` boundary

---

## Catch2 v3 Integration (ExplorerLens Secondary Test Surface)

ExplorerLens uses **Catch2 v3.7.1** as a secondary test framework for isolated
component tests that benefit from `SECTION`, `GENERATE`, and `REQUIRE` semantics.

### File Location

```text
Engine/Tests/catch2/
├── catch2_tests.cpp      — Main Catch2 runner (SESSION_START/END)
├── test_*.cpp             — Individual test files
└── CMakeLists.txt         — Links against Catch2::Catch2WithMain
```

### Catch2 Test Pattern

```cpp
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

TEST_CASE("CacheManager basic operations", "[cache]") {
    CacheManager cache;

    SECTION("empty cache returns miss") {
        auto result = cache.Get(L"nonexistent.png");
        REQUIRE_FALSE(result.hit);
    }

    SECTION("insert then retrieve") {
        cache.Put(L"test.png", thumbnailData);
        auto result = cache.Get(L"test.png");
        REQUIRE(result.hit);
        REQUIRE(result.data.size() == thumbnailData.size());
    }
}

TEST_CASE("Decoder routing with generated inputs", "[decoder]") {
    auto ext = GENERATE(".png", ".jpg", ".webp", ".avif", ".tiff");
    CAPTURE(ext);

    auto decoder = DecoderRegistry::FindDecoder(ext);
    REQUIRE(decoder != nullptr);
}
```

### Catch2 Rules

1. **Use `[tags]`** to categorize tests: `[cache]`, `[decoder]`, `[gpu]`, `[perf]`, `[regression]`.
2. **Use `SECTION` for related scenarios** that share setup — avoids test body duplication.
3. **Use `GENERATE`** for parameterized inputs — cleaner than manual loops.
4. **Never mix Catch2 and custom TEST() macros** in the same file — use separate targets.
5. **CI workflow:** `catch2-tests.yml` runs the Catch2 target independently from `EngineTests`.
6. **Link against `Catch2::Catch2WithMain`** — do not write your own `main()` in Catch2 tests.

### When to Use Catch2 vs Custom Framework

| Use Custom `TEST()` | Use Catch2 |
| --------------------- | ----------- |
| Sprint test batches (10 per sprint) | Isolated component deep-dives |
| Build validation (compile-time checks) | Property-based / generative tests |
| Quick regression checks (<1ms per test) | Slow integration tests with rich output |
| Main test count tracking (~4744) | Supplementary validation surface |

---

## Corpus-Based Test Patterns

### Purpose

Corpus tests validate decoder correctness against real files. The test corpus lives in
`data/corpus/` with CC0-licensed files covering every supported format.

### Structure

```text
data/corpus/
├── MANIFEST.json          — Index: file, format, expected dimensions, SSIM baseline
├── images/                — Real image files (PNG, JPEG, WebP, AVIF, TIFF, RAW, ...)
├── archives/              — Archive formats (ZIP, 7Z, RAR, TAR, ...)
├── documents/             — Document formats (PDF, EPUB, DOCX, ...)
└── baselines/             — Reference thumbnail PNGs for SSIM comparison
```

### Corpus Test Pattern (CI)

```yaml
# corpus-validation.yml runs:
# 1. Decode every file in MANIFEST.json
# 2. Compare output to baseline via SSIM score
# 3. Fail if SSIM < threshold (default 0.95)
# 4. Generate visual diff report on failure
```

### Adding a New Corpus Entry

1. Add the CC0-licensed file to the appropriate `data/corpus/` subdirectory.
2. Generate a baseline thumbnail: `EngineTests.exe --corpus-baseline <file>`.
3. Add an entry to `MANIFEST.json` with: path, format, expected width/height, SSIM threshold.
4. Run corpus validation locally before committing: `ctest -R corpus`.

### Rules

1. **All corpus files must be CC0 or public domain** — no copyrighted material.
2. **SSIM threshold is per-format** — lossy formats (JPEG, WebP) use 0.92; lossless use 0.99.
3. **Never check in files > 5 MB** — corpus files should be small representative samples.
4. **Baseline images are committed to git** — they are the ground truth for regression.

---

## Performance Test Patterns

### Benchmark Framework

ExplorerLens uses **Google Benchmark** for micro-benchmarks alongside the main test suite.

### Running Benchmarks

```powershell
# Build and run benchmarks
.\build-scripts\Build-MSVC.ps1 -Test
.\build\bin\EngineBenchmarks.exe --benchmark_format=json --benchmark_out=results.json
```

### Performance Regression Gate

The `performance-regression-gate.yml` workflow compares benchmark results against
`Engine/Tests/benchmarks/baseline.json` and fails if any metric regresses by >10%.

### Rules

1. **Update `baseline.json`** via `Bump-Version.ps1` on each release.
2. **Gate thresholds:** 17ms single thumbnail, 235 img/sec batch, <5ms cache hit.
3. **Never disable the gate** — fix the regression instead.

---

## Test Isolation & Environment Rules

### C++ Test Isolation

1. **No global state leakage** — each `TEST()` block must be self-contained.
   If a test modifies a singleton (e.g., `CacheManager::Instance()`), reset it in cleanup.
2. **No file system side effects** — use `GetTempPath()` for any files created during tests.
   Delete temp files in the test body or via RAII.
3. **No network access** — all decoder tests must work offline. Mock HTTP for API tests.
4. **Thread safety** — tests that validate concurrent behavior must use `std::latch` or
   `std::barrier` for synchronization, not `Sleep()`.

### Python Test Isolation

1. **Use `tmp_path`** fixture for all file I/O — never write to the project directory.
2. **Use `monkeypatch`** to override environment variables — never modify `os.environ` directly.
3. **Use `freezegun`** or `time_machine` for time-dependent tests — never depend on wall clock.

### CI-Specific Test Rules

1. **Timeout all test jobs** — no job should run longer than `timeout-minutes: 30`.
2. **Use `continue-on-error: false`** by default — failing tests must block the pipeline.
3. **Flaky test protocol:** If a test fails intermittently, add `@pytest.mark.flaky` (Python)
   or move to a separate `[flaky]` Catch2 tag, and file an issue to fix the root cause.
4. **Test output** — use `--output-on-failure` (CTest) and `-v --tb=short` (pytest) for
   actionable diagnostics in CI logs.

### Cross-Platform Test Considerations

- Tests using `wchar_t` paths must also handle UTF-8 on Linux/macOS stubs.
- Windows-only tests must be guarded with `#ifdef _WIN32` or `@pytest.mark.windows`.
- COM-dependent tests (registry, thumbnail provider) only run on Windows CI runners.
4. **Benchmark names must be stable** — renaming breaks baseline comparison.
