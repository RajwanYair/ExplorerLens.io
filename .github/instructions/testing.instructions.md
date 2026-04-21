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

```
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
|-------|---------|
| `TEST(name)` | Define a test function (expands to `void TestName_Runner()`) |
| `ASSERT(cond)` | Assert condition; throws on failure, increments counters |
| `RUN_TEST(name)` | Execute a test, catch failures, print pass/fail |

### Test File Layout

```
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
