---
mode: agent
description: "Generate comprehensive pytest tests for the selected code following workspace testing standards"
---

# Write Tests

Generate comprehensive pytest tests for the selected/specified code.

## Context

File to test: `${file}`
Selected code: `${selection}`

## Requirements

Follow `.github/instructions/testing.instructions.md`. Generate tests that:

1. **Cover all public functions/methods** — happy path, edge cases, error paths
2. **Use proper pytest fixtures** — defined in `conftest.py`
3. **Apply markers** — `@pytest.mark.unit`, `@pytest.mark.integration`, `@pytest.mark.slow`
4. **Mock external dependencies** — filesystem, network, subprocess calls
5. **Include property-based tests** using Hypothesis for data transformation functions
6. **Target 90%+ coverage** for the code under test

## Test Structure

```python
"""Tests for <module>."""
from __future__ import annotations

import pytest
from pathlib import Path
from unittest.mock import MagicMock, patch

# Import the module under test
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

## Naming Conventions

- Test files: `test_<module>.py`
- Test classes: `Test<ClassName>`
- Test functions: `test_<method>_<scenario>`

## Generate

Create the complete test file with all imports, fixtures, and test cases.
