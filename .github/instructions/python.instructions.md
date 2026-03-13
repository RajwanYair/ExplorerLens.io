---
applyTo: "**/*.py"
---

# Python Coding Instructions

## Style & Formatting

- **Line length**: 100 characters (enforced by ruff/black)
- **Quotes**: double quotes for strings
- **Imports**: grouped (stdlib → third-party → local), sorted by ruff/isort
- **Trailing commas**: always on multi-line collections

## Type Annotations — Non-Negotiable

Every function signature must have type annotations:

```python
# ✅ Good
def transform(items: list[str], max_count: int = 10) -> list[str]:
    ...

# ❌ Bad
def transform(items, max_count=10):
    ...
```

Use `from __future__ import annotations` for forward references on Python <3.10.

## Pathlib Over os.path

```python
# ✅ Good
from pathlib import Path
config = Path(__file__).parent / "config" / "default.yaml"
output = Path(args.output).resolve()

# ❌ Bad
import os
config = os.path.join(os.path.dirname(__file__), "config", "default.yaml")
```

## Logging Over print()

```python
# ✅ Good
import logging
logger = logging.getLogger(__name__)
logger.info("Processing %d files", len(files))
logger.error("Failed to process %s: %s", path, err)

# Also OK in CLI entry points
from rich.console import Console
console = Console(stderr=True)
console.print("[red]Error:[/red] file not found")

# ❌ Bad (in library/core code)
print(f"Processing {len(files)} files")
```

## Exception Handling

```python
# ✅ Good — specific, with context
try:
    result = process(path)
except FileNotFoundError as err:
    logger.error("Input file not found: %s", err)
    raise
except PermissionError as err:
    logger.error("Permission denied reading %s: %s", path, err)
    return None

# ❌ Bad — swallows errors silently
try:
    result = process(path)
except Exception:
    pass
```

## Context Managers for Resources

```python
# ✅ Good
with open(path, encoding="utf-8") as fh:
    content = fh.read()

# ❌ Bad
fh = open(path)
content = fh.read()
fh.close()
```

## Avoid Mutable Default Arguments

```python
# ✅ Good
def process(items: list[str] | None = None) -> list[str]:
    if items is None:
        items = []
    ...

# ❌ Bad — shared across calls!
def process(items: list[str] = []) -> list[str]:
    ...
```

## Subprocess — Never Use shell=True with User Input

```python
# ✅ Good — parameterized
result = subprocess.run(
    ["git", "commit", "-m", user_message],
    capture_output=True,
    text=True,
    check=True,
)

# ❌ Bad — shell injection risk
result = subprocess.run(
    f"git commit -m {user_message}",
    shell=True,
)
```

## Dataclasses and Named Tuples Over Dicts

```python
# ✅ Good
from dataclasses import dataclass, field

@dataclass
class ScanResult:
    path: Path
    duplicates: list[Path] = field(default_factory=list)
    size_bytes: int = 0

# ❌ Less good — no type safety
result = {"path": path, "duplicates": [], "size_bytes": 0}
```
