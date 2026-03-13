---
mode: agent
description: "Fix linting, type errors, and security issues in the current file"
---

# Fix Quality Issues

Fix all quality issues in: `${file}`

## What to Fix

Run through these checks and fix all issues found:

### 1. Ruff Lint Fixes

Apply all ruff auto-fixable rules:
- Remove unused imports (`F401`)
- Fix import order (`I` rules)
- Apply pyupgrade modernizations (`UP` rules)
- Fix bugbear issues (`B` rules)
- Simplify expressions (`SIM` rules)

### 2. Type Annotation Gaps

Add missing type hints to all function signatures:
```python
# Before
def process(items, verbose=False):

# After
def process(items: list[str], verbose: bool = False) -> list[ProcessResult]:
```

### 3. Exception Handling

Replace bare excepts with specific types:
```python
# Before
try: ...
except: pass

# After
try: ...
except (ValueError, RuntimeError) as err:
    logger.error("Context: %s", err)
```

### 4. Path Modernization

Replace `os.path` with `pathlib`:
```python
# Before
import os
path = os.path.join(os.path.dirname(__file__), "config")

# After
from pathlib import Path
path = Path(__file__).parent / "config"
```

### 5. Security Issues

- Replace `shell=True` subprocess calls with list form
- Replace hardcoded secrets with `os.environ.get()` or `keyring`
- Validate user input before use

## Constraints

- Do NOT change logic or behavior
- Do NOT add features
- Do NOT refactor beyond fixing the issues above
- Keep all existing tests passing
