---
mode: agent
description: "Generate comprehensive tests for the selected code following workspace testing standards"
---

# Write Tests

Generate comprehensive tests for the selected/specified code.

## Context

File to test: `${file}`
Selected code: `${selection}`

## Language Detection

- **C++ headers/sources (`Engine/`)** → Use ExplorerLens custom test framework (see below)
- **Python modules (`src/`, `tests/`)** → Use pytest (see below)

---

## C++ Tests (ExplorerLens Engine)

Follow the custom test framework defined in `Engine/Tests/EngineTestsMacros.h`.

### Test Framework

ExplorerLens uses `TEST(name)`, `ASSERT(cond)`, and `RUN_TEST(name)` macros — **NOT GTest**.

### File Placement Rules

| What | Where |
|------|-------|
| `#include` directive | `Engine/Tests/EngineTestsIncludes.h` |
| `extern void TestXxx_Runner();` | `Engine/Tests/EngineTestsExterns.h` |
| `RUN_TEST(TestXxx);` call | `Engine/Tests/EngineTests.cpp` (before `// Isolation & Stability Tests`) |
| `TEST(TestXxx) { ... }` body | `Engine/Tests/EngineTests_Platform.cpp` |

### C++ Test Pattern (Custom Framework)

```cpp
// In EngineTests_Platform.cpp
TEST(FeatureName) {
    using namespace ExplorerLens::Engine;

    // 1. Construction / default state
    FeatureName feature;
    ASSERT(feature.GetStats().processed == 0);

    // 2. Initialize
    ASSERT(feature.Initialize());

    // 3. Core functionality
    feature.Process(L"test-input");
    ASSERT(feature.GetStats().processed > 0);

    // 4. Edge cases
    feature.Process(L"");  // empty input
    ASSERT(feature.GetStats().processed > 0);

    // 5. Boundary conditions
    ASSERT(feature.GetStats().avgLatencyMs >= 0.0);

    // 6-9. Additional assertions for enums, config, error paths
}
```

### C++ Test Pattern (Catch2 — for decoder corpus tests)

```cpp
// In Engine/Tests/Catch2Tests_Decoders.cpp
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "../Decoders/JpegDecoder.h"

using namespace ExplorerLens::Engine;

TEST_CASE("JpegDecoder decodes basic JPEG", "[decoder][jpeg][corpus]") {
    auto path = GetCorpusPath("images/jpeg/test-basic.jpg");
    REQUIRE(std::filesystem::exists(path));

    JpegDecoder decoder;
    auto data = LoadFileToSpan(path);

    SECTION("ProbeHeader identifies format") {
        auto result = decoder.ProbeHeader(std::span(data).first(64));
        REQUIRE(result == DecodeResult::Supported);
    }

    SECTION("DecodeAtSize produces valid thumbnail") {
        auto stream = MakeIStreamFromSpan(data);
        auto result = decoder.DecodeAtSize(stream.Get(), 256, std::stop_token{});
        REQUIRE(result == DecodeResult::Success);
        REQUIRE(result.width <= 256);
        REQUIRE(result.height <= 256);
    }

    SECTION("Malformed input returns error, does not crash") {
        auto truncated = std::span(data).first(64);
        auto stream = MakeIStreamFromSpan(truncated);
        auto result = decoder.DecodeAtSize(stream.Get(), 256, std::stop_token{});
        REQUIRE(result != DecodeResult::Success);
    }
}
```

### C++ Test Checklist

- [ ] 7-12 `ASSERT()` calls per `TEST()` block covering construction, methods, and edge cases
- [ ] `using namespace ExplorerLens::Engine;` at top of TEST block
- [ ] No GTest macros (`EXPECT_*`, `ASSERT_*`, `TEST_F`) — use custom `ASSERT()`
- [ ] `(std::min)` / `(std::max)` parenthesized to avoid Windows macro conflicts
- [ ] Extern declaration added to `EngineTestsExterns.h`
- [ ] `RUN_TEST()` call added to `EngineTests.cpp`

---

## Python Tests (pytest)

Follow `.github/instructions/testing.instructions.md`. Generate tests that:

1. **Cover all public functions/methods** — happy path, edge cases, error paths
2. **Use proper pytest fixtures** — defined in `conftest.py`
3. **Apply markers** — `@pytest.mark.unit`, `@pytest.mark.integration`, `@pytest.mark.slow`
4. **Mock external dependencies** — filesystem, network, subprocess calls
5. **Include property-based tests** using Hypothesis for data transformation functions
6. **Target 90%+ coverage** for the code under test

### Python Test Structure

```python
"""Tests for <module>."""
from __future__ import annotations

import pytest
from pathlib import Path
from unittest.mock import MagicMock, patch

from src.<module> import <Class>


class Test<Class>:
    """Tests for <Class>."""

    @pytest.mark.unit
    def test_<method>_happy_path(self, ...):
        ...

    @pytest.mark.unit
    def test_<method>_edge_case_empty_input(self, ...):
        ...

    @pytest.mark.unit
    def test_<method>_raises_on_invalid_input(self, ...):
        with pytest.raises(ValueError, match="expected message"):
            ...

    @pytest.mark.integration
    def test_<method>_with_real_filesystem(self, tmp_path: Path):
        ...
```

### Python Naming Conventions

- Test files: `test_<module>.py`
- Test classes: `Test<ClassName>`
- Test functions: `test_<method>_<scenario>`

## Generate

Create the complete test code with all imports, fixtures, and test cases.
