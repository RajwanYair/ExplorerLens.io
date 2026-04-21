---
mode: agent
description: "Create a new Python project following the Universal Project Enhancement Framework v12.0.0 structure"
---

# Create New Project

Create a new Python project in this workspace following the **Universal Project Enhancement Framework v12.0.0**.

## Project Details

Project name: `${input:projectName}`
Description: `${input:description}`
Python minimum version: `${input:pythonVersion:3.9}`

## Required Output Structure

```
${input:projectName}/
├── ${input:projectName}        # Entry point script (no .py extension)
├── README.md                   # Full documentation with badges
├── CHANGELOG.md                # Keep-a-Changelog format
├── LICENSE                     # MIT license
├── VERSION                     # Semver string e.g. 0.1.0
├── requirements.txt            # Runtime dependencies
├── pyproject.toml              # [project] + all tool configs from workspace root
├── pyrightconfig.json          # Copy from workspace root
├── .flake8                     # Copy from workspace root
├── .gitattributes              # Copy from workspace root
├── .markdownlint.json          # Copy from workspace root
├── src/
│   ├── __init__.py
│   ├── cli/
│   │   ├── __init__.py
│   │   └── main.py             # Click CLI entry point
│   ├── core/
│   │   ├── __init__.py
│   │   └── processor.py        # Main business logic
│   └── utils/
│       ├── __init__.py
│       └── helpers.py
├── config/
│   └── default.yaml            # App config with ${ENV_VAR:default} substitution
├── tests/
│   ├── conftest.py
│   ├── unit/
│   │   └── test_core.py
│   └── integration/
│       └── test_integration.py
├── docs/
│   └── README.md
└── .github/
    ├── copilot-instructions.md
    ├── contributing.md
    ├── security.md
    ├── codeowners
    ├── dependabot.yml
    ├── pull_request_template.md
    ├── issue_template/
    │   ├── bug_report.md
    │   ├── feature_request.md
    │   └── performance_issue.md
    ├── instructions/           # Copy relevant .instructions.md files from workspace .github/instructions/
    └── workflows/
        ├── ci.yml
        └── release.yml
```

## Code Standards

- Use Click 8.1+ for CLI
- Use Rich 13+ for terminal output
- Type hints everywhere
- Signal handlers (SIGTERM/SIGINT)
- Dataclasses for data structures
- Zero hardcoded paths (always `Path(__file__).parent.resolve()`)
- Google-style docstrings
- No bare `except:` clauses

## Entry Point Template

```python
#!/usr/bin/env python3
"""${input:projectName} — ${input:description}"""

import signal
import sys
from pathlib import Path

import click
from rich.console import Console

PROJECT_ROOT = Path(__file__).parent.resolve()
console = Console()


def handle_shutdown(signum: int, frame: object) -> None:
    console.print("\n[yellow]Shutting down gracefully...[/yellow]")
    sys.exit(0)


signal.signal(signal.SIGTERM, handle_shutdown)
signal.signal(signal.SIGINT, handle_shutdown)


@click.group()
@click.version_option(version="0.1.0")
def cli() -> None:
    """${input:description}"""


if __name__ == "__main__":
    cli()
```
