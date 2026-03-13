---
applyTo: "**"
---

# GitHub Copilot Workspace Instructions

## Project Overview

Cross-platform Python development workspace containing multiple utility and automation projects.
All projects follow the **Universal Project Enhancement Framework v12.0.0**.

## Projects in this Workspace

| Project                             | Description                                | Language          |
| ----------------------------------- | ------------------------------------------ | ----------------- |
| `RegiLattice`                       | Windows registry tweak toolkit             | Python            |
| `Scripts.DupDetector`               | Duplicate file detection and removal       | Python            |
| `Scripts.FileNameManipulator`       | Batch filename manipulation                | Python            |
| `Scripts.FileProcessor`             | File processing pipeline                   | Python            |
| `Scripts.OptimizeBrowsers`          | Cross-platform browser optimization        | Python            |
| `Scripts.SortComics`                | Comic book sorting and organization        | Python            |
| `Scripts.PPA`                       | PPA/package management utilities           | Python/Shell      |
| `Scripts.UbuntuEnhancer`            | Ubuntu system enhancements                 | Python/Shell      |
| `Scripts.VHDXCompress`              | VHDX disk image compression                | Python/PowerShell |
| `Scripts.OptimizeWIN_n_WSL`         | Windows + WSL optimization                 | Python/PowerShell |
| `Scripts.VSCode.RemoteSSH.Verifier` | VS Code Remote SSH verification            | Python            |
| `ExplorerLens.io`                   | Windows Shell Extension thumbnail provider | C++/Python        |

## Technical Stack

- **Language**: Python 3.9+ (prefer system-wide installation over virtual environments)
- **CLI Framework**: Click 8.1+ or argparse with Rich 13.0+
- **Configuration**: Pydantic 2.0+ models, YAML with `${ENV_VAR:default}` substitution
- **Packaging**: PEP 517/518 with hatchling build backend
- **Testing**: pytest with coverage, GitHub Actions CI/CD
- **Linting**: ruff (primary), flake8 (compat)
- **Formatting**: ruff format / black
- **Type Checking**: mypy + pyright/pylance

## Core Architecture Patterns

### Single Entry Point

- One main executable per project with command routing
- All functionality via CLI, Desktop GUI (Tkinter), and Web GUI (FastAPI)
- Shared backend services

### Portability First

```python
# ✅ Always use relative paths
PROJECT_ROOT = Path(__file__).parent.resolve()
config = PROJECT_ROOT / "config" / "default.yaml"

# ❌ Never hardcode absolute paths
config = "C:\\Users\\name\\project\\config.yaml"
```

### Configuration Hierarchy (highest → lowest priority)

1. Command-line arguments
2. Environment variables (`${VAR:default}` in YAML)
3. User config file
4. Default config

### Standard Project Structure

```
project-name/
├── project-name          # Single entry point (no .py extension)
├── README.md
├── CHANGELOG.md
├── LICENSE
├── VERSION
├── requirements.txt
├── pyproject.toml        # Tool configs (ruff, mypy, pytest, coverage)
├── pyrightconfig.json    # Pyright/Pylance settings
├── src/
│   ├── cli/              # Click commands
│   ├── core/             # Business logic
│   ├── gui/              # Tkinter desktop GUI
│   └── utils/            # Shared helpers
├── config/
│   └── default.yaml      # Default configuration
├── tests/
│   ├── conftest.py
│   ├── unit/
│   └── integration/
└── docs/
```

## Coding Standards

### Type Hints — Required Everywhere

```python
from typing import Callable
from pathlib import Path

def process_item(
    item: str,
    verbose: bool = False,
    callback: Callable[[int, int], None] | None = None,
) -> ProcessResult:
    ...
```

### Data Classes Over Dicts

```python
from dataclasses import dataclass, field

@dataclass
class ProcessResult:
    success: bool
    message: str
    duration: float = 0.0
    errors: list[str] = field(default_factory=list)
```

### Enums for Constants

```python
from enum import Enum

class Status(str, Enum):
    PENDING = "pending"
    RUNNING = "running"
    COMPLETE = "complete"
    FAILED = "failed"
```

### Error Handling

- Use specific exception types (never bare `except:`)
- Provide meaningful messages with context
- Implement graceful degradation
- Log errors with full context

### Signal Handling — Always Implement

```python
import signal
import sys

def handle_shutdown(signum: int, frame: object) -> None:
    print("\nShutting down gracefully...")
    cleanup()
    sys.exit(0)

signal.signal(signal.SIGTERM, handle_shutdown)
signal.signal(signal.SIGINT, handle_shutdown)
```

### Progress Tracking — Use Rich

```python
from rich.progress import Progress, SpinnerColumn, TimeElapsedColumn

with Progress(SpinnerColumn(), *Progress.get_default_columns(), TimeElapsedColumn()) as progress:
    task = progress.add_task("Processing...", total=len(items))
    for item in items:
        process(item)
        progress.advance(task)
```

### CLI Entry Points — Use Click

```python
import click
from rich.console import Console

console = Console()

@click.command()
@click.option("--verbose", "-v", is_flag=True, help="Enable verbose output")
@click.option("--config", "-c", type=click.Path(exists=True), help="Config file path")
@click.version_option(version="1.0.0")
def main(verbose: bool, config: str | None) -> None:
    """Project description here."""
    console.print("[green]Starting...[/green]")
```

## Package Management

### Preference Order

1. **System packages** (`apt`, `yum`, `brew`) — most stable
2. **pip** (`--break-system-packages`) — for packages not in system repos
3. **User install** (`pip --user`) — fallback only

### Never Use venv by Default

Unless explicitly requested, install system-wide.

## Testing Requirements

- **Goal**: 90%+ code coverage
- **Framework**: pytest with pytest-cov
- **Platforms**: Windows, Linux, WSL, macOS
- **Types**: unit + integration + cross-platform
- **Hypothesis**: property-based tests for complex logic

## Documentation Standards

- `README.md` — comprehensive with examples and badges
- `CHANGELOG.md` — Keep-a-Changelog format
- `docs/` — MkDocs with Material theme
- Google-style docstrings
- Type hints = documentation

## Security Guidelines (OWASP)

- **No hardcoded credentials** — use environment variables or keyring
- **No hardcoded proxy URLs** — use configuration files
- **Validate all user input** — at system boundaries
- **Parameterized commands** — never build shell strings from user input
- **Least privilege** — request admin only when necessary
- **No secrets in git** — `.env` files in `.gitignore`

## What NOT to Do

- Don't hardcode absolute paths anywhere
- Don't skip signal handlers (SIGTERM/SIGINT)
- Don't use `print()` — use `logging` or `rich.console`
- Don't leave debug code in production paths
- Don't commit secrets, credentials, or API keys
- Don't create multiple entry points per project
- Don't skip tests for "simple" code
- Don't use bare `except:` clauses
- Don't use mutable default arguments
