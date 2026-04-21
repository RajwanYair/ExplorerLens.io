# CI-Ops — GitHub Actions Management Skill

## Purpose

Use this skill when debugging CI failures, auditing action versions, managing workflow
permissions, creating reusable workflows, or upgrading the Node.js runtime across
GitHub Actions. Read it fully before modifying any `.github/workflows/*.yml` file
for operational (non-feature) changes.

---

## When to Use This Skill

- A GitHub Actions workflow is failing and needs diagnosis
- Auditing action versions across all 21 workflows
- Adding or tightening `permissions:` blocks
- Migrating Node.js runtime (e.g., 20 → 24)
- Creating or modifying reusable/callable workflows
- Adding `concurrency:` groups to prevent duplicate runs
- Pinning actions to SHA for supply-chain security

---

## Step-by-Step: Debug a Failing Workflow

1. **Identify the failing workflow and job**
   ```
   gh run list --workflow=<name>.yml --limit 5
   gh run view <run-id> --log-failed
   ```

2. **Match against known failure patterns**

   | Pattern | Root Cause | Fix |
   |---------|-----------|-----|
   | `Unable to resolve action` | Action version yanked or renamed | Pin to SHA or update to latest tag |
   | `Node.js 16 actions are deprecated` | Old action using Node 16 | Set `FORCE_JAVASCRIPT_ACTIONS_TO_NODE24: true` or upgrade action |
   | `Resource not accessible by integration` | Missing `permissions:` | Add explicit permission block |
   | `The process cannot access the file` | Windows file locking | Add retry or use `shell: pwsh` with `-ErrorAction` |
   | `fatal: unsafe repository` | Git safe.directory not set | Add `GIT_CONFIG` env vars |
   | `CMake Error: could not find compiler` | vcvars not sourced | Use `ilammy/msvc-dev-cmd@v1` step before cmake |
   | `Error: Timeout` | Job exceeds 6h limit | Add `timeout-minutes:` or split into smaller jobs |
   | `disk space` or `no space left on device` | Runner disk full | Add cleanup step or use larger runner |
   | `Rate limit exceeded` | GitHub API rate limit | Add exponential backoff or cache API responses |
   | `Annotations limit` | >50 annotations in a run | Batch annotations or use `::group::` for log sections |

3. **Check if the failure is flaky**
   ```
   gh run list --workflow=<name>.yml --limit 10 --json conclusion,startedAt
   ```

4. **Propose fix** — edit the workflow file and test via `workflow_dispatch`

---

## Step-by-Step: Audit Action Versions

1. **List all actions and their versions**
   ```powershell
   Get-ChildItem .github/workflows/*.yml | ForEach-Object {
       Select-String -Path $_.FullName -Pattern 'uses:\s+(\S+)' |
       ForEach-Object { $_.Matches[0].Groups[1].Value }
   } | Sort-Object -Unique
   ```

2. **Check for outdated versions**
   - `actions/checkout` → should be `@v4`
   - `actions/upload-artifact` / `download-artifact` → should be `@v4`
   - `actions/cache` → should be `@v4`
   - `actions/setup-node` → should be `@v4`
   - `actions/setup-python` → should be `@v5`
   - `ilammy/msvc-dev-cmd` → should be `@v1` (auto-latest)
   - `release-drafter/release-drafter` → should be `@v6`

3. **Upgrade pattern** — replace tag with latest stable:
   ```yaml
   # Before
   - uses: actions/checkout@v3
   # After
   - uses: actions/checkout@v4
   ```

4. **SHA pinning** (supply-chain security) — for critical workflows:
   ```yaml
   - uses: actions/checkout@11bd71901bbe5b1630ceea73d27597364c9af683 # v4.2.2
   ```

---

## Step-by-Step: Add Permissions Block

Every workflow MUST have an explicit top-level `permissions:` block.
GitHub defaults to restricted permissions since 2024.

### Minimal Permission Templates

```yaml
# Read-only workflow (build, lint, test)
permissions:
  contents: read

# PR workflow that posts comments
permissions:
  contents: read
  pull-requests: write

# Release workflow
permissions:
  contents: write
  packages: write

# Pages deployment
permissions:
  pages: write
  id-token: write

# Issue/PR labeling
permissions:
  issues: write
  pull-requests: write
```

### Rules
- Start with `contents: read` and add only what's needed
- Never use `permissions: write-all` in production workflows
- Document why each permission is needed in a comment

---

## Step-by-Step: Add Concurrency Groups

All PR-triggered workflows should use concurrency to cancel outdated runs.

```yaml
concurrency:
  group: <workflow-name>-${{ github.event.pull_request.number || github.ref }}
  cancel-in-progress: true
```

### Rules
- Use the workflow name in the group key for uniqueness
- Use `cancel-in-progress: true` for build/test workflows
- Use `cancel-in-progress: false` for deploy/release workflows (don't cancel mid-deploy)

---

## Step-by-Step: Node.js 24 Migration

1. Add to every workflow's `env:` block:
   ```yaml
   env:
     FORCE_JAVASCRIPT_ACTIONS_TO_NODE24: true
   ```

2. Verify no action fails under Node 24 — common issues:
   - `ERR_OSSL_EVP_UNSUPPORTED` → action uses deprecated OpenSSL API
   - `primordials is not defined` → action uses very old Node API

3. If an action fails under Node 24, pin it to a version that supports Node 24
   or find an alternative action.

---

## Step-by-Step: Create a Reusable Workflow

Reusable workflows live in `.github/workflows/` and use `workflow_call:` trigger.

```yaml
# .github/workflows/reusable-build.yml
name: Reusable Build

on:
  workflow_call:
    inputs:
      cmake-preset:
        required: false
        type: string
        default: 'default-release'
      run-tests:
        required: false
        type: boolean
        default: true

permissions:
  contents: read

jobs:
  build:
    runs-on: windows-latest
    timeout-minutes: 30
    steps:
      - uses: actions/checkout@v4
      - uses: ilammy/msvc-dev-cmd@v1
      - name: Build
        run: cmake --preset ${{ inputs.cmake-preset }} && cmake --build build --config Release
        shell: pwsh
      - name: Test
        if: ${{ inputs.run-tests }}
        run: ctest --test-dir build -C Release --output-on-failure
        shell: pwsh
```

### Calling a Reusable Workflow
```yaml
jobs:
  build:
    uses: ./.github/workflows/reusable-build.yml
    with:
      cmake-preset: 'default-release'
      run-tests: true
```

---

## Workflow Inventory (21 Workflows)

| Category | Workflows |
|----------|-----------|
| **Build** | `build.yml`, `ci-matrix.yml`, `toolchain-verify.yml` |
| **Quality** | `code-quality.yml`, `codeql.yml`, `coverage.yml` |
| **Testing** | `catch2-tests.yml`, `corpus-validation.yml`, `performance-regression-gate.yml`, `screenshot-regression.yml` |
| **Release** | `release.yml`, `release-drafter.yml`, `publish-packages.yml` |
| **PR** | `pr-checks.yml`, `binary-size.yml`, `auto-label.yml` |
| **Docs** | `docs-validation.yml`, `pages.yml` |
| **Ops** | `notify-failure.yml`, `stale.yml`, `sync-labels.yml` |

---

## Validation Checklist

After any workflow change:

- [ ] `permissions:` block present and minimal
- [ ] `concurrency:` on all PR-triggered workflows
- [ ] `workflow_dispatch:` present for manual re-runs
- [ ] `timeout-minutes:` set on long-running jobs
- [ ] `FORCE_JAVASCRIPT_ACTIONS_TO_NODE24: true` in `env:`
- [ ] All actions at latest stable version (v4+)
- [ ] No hardcoded secrets — use `${{ secrets.* }}` or `${{ github.token }}`
- [ ] Update `.github/standards/ai-tooling-capabilities.md` workflow table
