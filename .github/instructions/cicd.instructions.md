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

---

## ExplorerLens Release-on-Tag Procedure (C++ / MSVC)

### Trigger

Every version bump **must** create a `git tag vX.Y.Z` which auto-triggers `release.yml`.

```powershell
# 1. Update version references in ALL of:
#    - VERSION  (repo root — single source of truth)
#    - CHANGELOG.md  (new [X.Y.Z] section at top)
#    - Engine/Engine.h  (EXPLORERLENS_ENGINE_VERSION_MINOR / _PATCH)
#    - .github/copilot-instructions.md  (version line near top)
#    - docs/assets/social-preview.svg  (version chip + any stat updates)

# 2. Build + verify locally (MSVC v145 — vcvars sourced automatically)
.\build-scripts\Build-MSVC.ps1 -Test

# 3. Stage, commit, tag, push
$git = "$env:USERPROFILE\scoop\apps\git\current\bin\git.exe"
& $git add -A
& $git commit -m "chore: bump version to X.Y.Z (Codename)"
& $git tag vX.Y.Z
& $git push origin main --tags   # --tags fires release.yml
```

### What `release.yml` Publishes

| Artifact | Notes |
|----------|-------|
| `LENSShell.dll` (x64) | COM shell extension — always |
| `LENSManager.exe` | WTL config GUI — always |
| `lens.exe` | CLI tool — Sprint 17+ |
| `Manager.WinUI.exe` | WinUI 3 GUI — Sprint 52+ |
| `ExplorerLens-X.Y.Z-x64.msi` | WiX installer — always |
| `ExplorerLens-X.Y.Z-x64.zip` | Portable archive — always |
| `SHA256SUMS.txt` | Checksums for all artifacts |
| `ExplorerLens-X.Y.Z-SBOM.json` | CycloneDX SBOM |

### Release Workflow Architecture

```
git tag vX.Y.Z → release.yml
  └─ build job (windows-latest)
       configure MSVC via vswhere
       cmake --preset default-release + msbuild LENSShell.sln
       discover binaries (multi-path search, warn on missing)
       build MSI (WiX), create ZIP, generate SHA256SUMS, update SBOM
       upload-artifact → passes to publish job
  └─ publish job (ubuntu-latest)
       download all artifacts
       extract CHANGELOG section for this version → release body
       create GitHub Release (draft) via softprops/action-gh-release@v2
```

### Rules

- **Never skip the tag** — the tag is what fires the release pipeline
- **Never change the COM CLSID** `9E6ECB90-5A61-42BD-B851-D3297D9C7F39`
- **Always build with MSVC v145** — never Clang for production artifacts
- **VERSION file is the fallback** — release.yml reads it when no tag info available
- **Sprint plan is in** `docs/SPRINT_PLAN_100.md` — 100 sprints through v17.0.0 "Nova"

---

## Post-Release Verification (Required)

Every release workflow **must** include a `verify` job that runs after `publish` and
confirms the release finished cleanly. If verification fails, a GitHub Issue is opened
automatically so the team has an actionable ticket.

### Architecture

```
git tag vX.Y.Z → release.yml
  └─ build job   (windows-latest)  — compile, package, upload artifacts
  └─ publish job (ubuntu-latest)   — create GitHub Release
  └─ verify job  (ubuntu-latest)   — check health, open issue on failure
```

### What the `verify` Job Checks

| Check | Pass Condition |
|-------|---------------|
| Required artifacts present | `ExplorerLens-X.Y.Z-x64.zip`, `SHA256SUMS.txt`, `SBOM.json`, `verification-report.json` all in `dist/` |
| ZIP integrity | ZIP contains `LENSShell.dll` |
| SHA256 checksums | Every hash in `SHA256SUMS.txt` matches the actual artifact |
| GitHub Release accessible | `gh release view vX.Y.Z` returns HTTP 200 with ≥3 assets |
| Version consistency | Tag version matches contents of `VERSION` file |
| MSI present (warn only) | `ExplorerLens-X.Y.Z-x64.msi` in `dist/` |

### Failure Handling

- Errors → job fails, GitHub Issue created with label `release-failure` + `bug`
- Duplicate prevention: if an open issue for the same version already exists, a comment is appended instead of opening a new issue
- Warnings → job succeeds with annotations; no issue created

### Re-triggering a Failed Release

```powershell
# After fixing the root cause:
git tag -d vX.Y.Z
git tag vX.Y.Z
git push origin --tags --force   # fires release.yml again
```

### Required Job Permissions

```yaml
verify:
  permissions:
    contents: read   # download artifacts, read VERSION
    issues: write    # create/comment on GitHub Issues
```

### Rule: Never Skip Verification

- The `verify` job must always be the last job in `release.yml`
- It must have `needs: [build, publish]` so it only runs after both succeed
- It must use `actions/github-script@v7` (not `gh` CLI) for issue creation to
  avoid shell-injection risks when constructing the issue body
