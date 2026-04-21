---
mode: agent
description: "Fix linting, type errors, and security issues in the current file"
---

# Fix Quality Issues

Fix all quality issues in: `${file}`

## Language Detection

- **C++ files (`*.h`, `*.cpp`)** → Apply C++/MSVC fixes below
- **Python files (`*.py`)** → Apply Python fixes below
- **Both apply** if the file is a build script or mixed-concern file

---

## C++20 / MSVC Quality Fixes (ExplorerLens)

### 1. Header Compliance

- Add missing `#pragma once`
- Fix header banner format: `// File.h — Title` + copyright block before `#pragma once`
- Remove stale version/sprint tags from banners
- Remove inline comments that restate the type name

### 2. Naming Compliance

- Classes/functions: `PascalCase`
- Variables: `camelBack`
- Private members: `m_` prefix
- Enum values: `UPPER_CASE` (enforced by `.clang-tidy`)
- Constants: `UPPER_CASE`

### 3. Windows Portability

- Replace `memmem()` with manual `memcmp` loop (POSIX-only)
- Replace `__builtin_*` intrinsics with standard equivalents (`memcpy`, `memset`)
- Parenthesize `(std::min)(a, b)` / `(std::max)(a, b)` to avoid Windows macro conflicts
- Remove `<versionhelpers.h>` — use `RtlGetVersion()` via ntdll.dll
- Verify all includes compile under `WIN32_LEAN_AND_MEAN`

### 4. MSVC Build Compliance

- Remove any `/wdXXXX` warning suppressions — fix the root cause
- Ensure all code compiles cleanly under `/W4 /WX`
- Guard linker-specific flags with `if(MSVC)` in CMake
- Use `/MD` (dynamic CRT) — never mix with `/MT`

### 5. Resource Safety

- Replace raw `new`/`delete` with RAII or smart pointers
- Ensure COM objects use `AddRef()`/`Release()` correctly
- Add `noexcept` where appropriate (destructors, move operations)

---

## Python Quality Fixes

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

---

## Constraints

- Do NOT change logic or behavior
- Do NOT add features
- Do NOT refactor beyond fixing the issues above
- Keep all existing tests passing
- Preserve zero-warning build status for C++ changes
