---
applyTo: "**/*.yml,**/*.yaml,.github/**"
---

# CI/CD and GitHub Actions Instructions

## Workflow Design Principles

- Pin action versions to a full SHA or semver tag (`@v4`, not `@main`)
- Use `permissions: contents: read` (least privilege) as default
- Cache pip dependencies to speed up runs
- Run on both push to main and pull_request
- Use matrix strategy for multi-platform and multi-Python-version testing

## Standard Python CI Workflow Pattern

```yaml
name: CI

on:
    push:
        branches: [main]
    pull_request:
        branches: [main]

permissions:
    contents: read

jobs:
    test:
        runs-on: ${{ matrix.os }}
        strategy:
            fail-fast: false
            matrix:
                os: [ubuntu-latest, windows-latest]
                python-version: ["3.9", "3.10", "3.11", "3.12", "3.13"]

        steps:
            - uses: actions/checkout@v4

            - uses: actions/setup-python@v5
              with:
                  python-version: ${{ matrix.python-version }}
                  allow-prereleases: true

            - name: Cache pip
              uses: actions/cache@v4
              with:
                  path: ~/.cache/pip
                  key: ${{ runner.os }}-pip-${{ hashFiles('**/requirements*.txt') }}
                  restore-keys: ${{ runner.os }}-pip-

            - name: Install dependencies
              run: |
                  python -m pip install --upgrade pip
                  pip install -r requirements.txt

            - name: Lint (ruff)
              run: python -m ruff check src/ tests/

            - name: Format check (ruff)
              run: python -m ruff format --check src/ tests/

            - name: Type check (mypy)
              run: python -m mypy src/ --ignore-missing-imports

            - name: Security scan (bandit)
              run: python -m bandit -r src/ -ll

            - name: Test (pytest + coverage)
              run: python -m pytest tests/ -v --tb=short --cov=src --cov-report=xml

            - name: Upload coverage
              if: matrix.python-version == '3.12' && matrix.os == 'ubuntu-latest'
              uses: actions/upload-artifact@v4
              with:
                  name: coverage-report
                  path: coverage.xml
```

## Commit Message Convention (Conventional Commits)

```
<type>(<scope>): <subject>

[optional body]

[optional footer(s)]
```

Types: `feat`, `fix`, `docs`, `style`, `refactor`, `test`, `chore`, `ci`, `perf`, `security`

Examples:

- `feat(cli): add --dry-run flag`
- `fix(scanner): handle empty directories gracefully`
- `ci: pin actions to SHA for security`
- `security: sanitize user input in file path arguments`

## Security Best Practices in Workflows

- Never log secrets or tokens
- Use `${{ secrets.TOKEN }}` not hardcoded values
- Set `permissions` explicitly on every job
- Use `if: github.event_name == 'push'` to limit sensitive steps to main
