---
applyTo: "**/*.yml,**/*.yaml,.github/**"
---

# CI/CD and GitHub Actions Instructions

## ExplorerLens C++/MSVC — Lessons Learned (v31.x–v34.x)

### CRITICAL: Never pin the MSVC toolset in CI workflows

**Root cause of issues #145, #148–#156, #158–#160 (v31.2.0–v32.1.3):**
GitHub-hosted `windows-latest` runner ships VS2022 (MSVC v143). Local dev uses VS 18 2026
BuildTools (MSVC v145). Pinning `toolset: "14.50"` (or any specific toolset) in
`ilammy/msvc-dev-cmd@v1` causes ALL CI builds to fail because v145 does not exist on runners.

```yaml
# ❌ WRONG — fails on GitHub-hosted runners (toolset 14.50 not installed)
- uses: ilammy/msvc-dev-cmd@v1
  with:
    toolset: "14.50"

# ✅ CORRECT — lets the runner use whatever VS version is available
- uses: ilammy/msvc-dev-cmd@v1
  with:
    arch: x64
```

**Fix committed:** `c9e241d2` (v34.4.0 "Arcturus-U") — all toolset pins removed from `release.yml`.

### MSVC Portability Rules for C++ Code

- **`memmem()` is POSIX-only** — GNU extension, not available on MSVC. Use manual `memcmp` loop:
  ```cpp
  // ❌ MSVC error: memmem not declared
  auto p = (const char*)memmem(data, size, "</OME>", 6);
  // ✅ Works on all platforms
  for (const char* q = p; q + 6 <= end; ++q)
      if (memcmp(q, "</OME>", 6) == 0) { p = q; break; }
  ```
- **`#pragma pack(push,1)`** — valid and required for exact-size binary structs (e.g. LAS headers)
- **`WIN32_LEAN_AND_MEAN`** is globally defined — never include `<versionhelpers.h>` or headers that require it
- **`/MD` CRT** — all targets and all external libs use dynamic CRT; no `/NODEFAULTLIB` needed

### Sprint Test Infrastructure (per-sprint checklist)

Each sprint (10 tests, 5 headers, 5 sources) must update ALL of these:
1. `Engine/CMakeLists.txt` — add 5 headers to ENGINE_HEADERS, 5 sources to ENGINE_SOURCES
2. `Engine/Tests/EngineTestsIncludes.h` — 5 `#include` directives
3. `Engine/Tests/EngineTestsExterns.h` — 10 `extern void TestXxx_Runner();` declarations
4. `Engine/Tests/EngineTests.cpp` — 10 `RUN_TEST(TestXxx);` calls
5. `Engine/Tests/EngineTests_Late.cpp` — 10 `TEST(TestXxx) { ... }` bodies
Test count formula: `previous_count + 10`

### Bump-Version.ps1 Operational Notes

- **SBOMGenerator.h file lock:** VS sometimes holds the file open. Retry `Bump-Version.ps1` immediately — succeeds on second run.
- **Backtick-quote not backslash-quote:** `Bump-Version.ps1` uses PowerShell backtick for embedded quotes in strings (line 246). If `-ChangelogEntry` contains quotes, escape them properly.
- **Run with:** `.\build-scripts\Bump-Version.ps1 -Version "X.Y.Z" -Codename "Name" -TestCount N -ChangelogEntry "..." -TagAndPush`

### GitHub API from Corporate Network

`gh api` calls (which use Go's `net/http` directly) may fail with connection timeout on corporate
networks when the proxy isn't inherited. Use PowerShell's `Invoke-RestMethod` with `-UseDefaultCredentials`
which respects the Windows system proxy (WinINet) settings:

```powershell
$h = @{ Authorization = "Bearer $token"; "Content-Type" = "application/json"; "User-Agent" = "MyApp" }
$result = Invoke-RestMethod -Uri "https://api.github.com/repos/owner/repo/issues/1" `
    -Method Patch -Headers $h -Body '{"state":"closed"}' -UseDefaultCredentials
```

### Auto-Created CI Failure Issues

GitHub Actions creates issues automatically when a workflow fails with certain conditions.
To bulk-close stale CI-failure issues after a fix, use `Invoke-RestMethod` (see above).
All 13 issues from v31.2.0–v32.1.3 were closed by commit `c9e241d2` on 2026-04-09.

---


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

## Node.js 24 Runtime Migration

GitHub Actions runners are migrating from Node.js 20 to Node.js 24.
All ExplorerLens workflows opt into Node.js 24 early to avoid deprecation warnings.

```yaml
env:
  FORCE_JAVASCRIPT_ACTIONS_TO_NODE24: true
```

**Rules:**
- Set `FORCE_JAVASCRIPT_ACTIONS_TO_NODE24: true` at the workflow `env:` level (not per-job)
- When a JavaScript-based action breaks on Node 24, pin it to the last working version and file an issue
- Never downgrade to Node 16 or 18 — those are already EOL on GitHub-hosted runners

## Permissions-First Policy

Since GitHub's restricted default permissions (2024), every workflow MUST have an explicit
`permissions:` block. Missing permissions cause silent failures.

```yaml
# ✅ CORRECT — explicit least-privilege
permissions:
  contents: read

# ❌ WRONG — relies on repo default (may be restricted)
# (no permissions block)
```

**Common permission sets by workflow type:**

| Workflow Type | Required Permissions |
|--------------|---------------------|
| Build/test | `contents: read` |
| PR checks | `contents: read`, `pull-requests: read` |
| Code scanning | `contents: read`, `security-events: write` |
| Release | `contents: write`, `packages: write` |
| Pages deploy | `contents: read`, `pages: write`, `id-token: write` |
| Issue/PR labeling | `contents: read`, `issues: write`, `pull-requests: write` |

**Rules:**
- Never use `permissions: write-all` — always enumerate specific scopes
- Job-level `permissions:` overrides workflow-level — use job-level for heterogeneous workflows
- The `id-token: write` permission is only needed for OIDC token exchange (Pages, cloud deploys)

## Reusable Workflow Patterns

ExplorerLens has `.github/workflows/reusable-build.yml` as a `workflow_call` entry point.

```yaml
# Caller workflow:
jobs:
  build:
    uses: ./.github/workflows/reusable-build.yml
    with:
      build-type: Release
      run-tests: true
```

**Rules for reusable workflows:**
- Define inputs with `type:` (string, boolean, number) and clear `description:`
- Provide sensible defaults for all optional inputs
- Reusable workflows inherit the caller's `GITHUB_TOKEN` scope
- Keep reusable workflows self-contained — they should not depend on caller-side setup steps
- Test reusable workflows with `workflow_dispatch` before using as `workflow_call`

## Concurrency Groups

All PR-triggered workflows MUST use concurrency groups to avoid duplicate runs:

```yaml
concurrency:
  group: ${{ github.workflow }}-${{ github.ref }}
  cancel-in-progress: true
```

**Rules:**
- Use `cancel-in-progress: true` for PR workflows (supersede older runs)
- Use `cancel-in-progress: false` for release and deployment workflows (never cancel mid-deploy)
- The group key should include `github.workflow` to avoid cross-workflow cancellation

## Workflow Dispatch (Manual Re-run)

All workflows SHOULD support manual re-run via `workflow_dispatch`:

```yaml
on:
  workflow_dispatch:
    inputs:
      reason:
        description: 'Reason for manual trigger'
        required: false
        default: 'Manual verification'
```

**Exceptions:** `notify-failure.yml` (event-driven only) does not need `workflow_dispatch`.

## Artifact Retention

All `upload-artifact` steps MUST specify `retention-days:`:

| Artifact Type | Retention |
|--------------|-----------|
| Test results | 30 days |
| Build binaries | 14 days |
| Release artifacts | 90 days |
| Coverage reports | 30 days |
| Diagnostics (failure) | 14 days |
| Test corpus | 7 days |

## Timing Annotations

Build and test steps should emit timing via `::notice::` annotations:

```yaml
- name: Build
  shell: pwsh
  run: |
    $sw = [System.Diagnostics.Stopwatch]::StartNew()
    cmake --build --preset ci-release -j $env:NUMBER_OF_PROCESSORS
    $sw.Stop()
    Write-Host "::notice::Build took $($sw.Elapsed.TotalSeconds.ToString('F1'))s"
```

## ExplorerLens Workflow Inventory

| Workflow | Trigger | Role |
|----------|---------|------|
| `ci-matrix.yml` | push/PR | Canonical CI build matrix |
| `build.yml` | dispatch/schedule | Manual verification build |
| `reusable-build.yml` | workflow_call | DRY build+test callable |
| `pr-checks.yml` | PR | Title, size, changelog validation |
| `release.yml` | tags `v*` | Release packaging + artifacts |
| `docs-validation.yml` | docs/SVG changes | MkDocs + SVG validation |
| `code-quality.yml` | push/PR | Static analysis |
| `codeql.yml` | push/PR/schedule | Security scanning |
| `coverage.yml` | push | Coverage collection |
| `catch2-tests.yml` | push/PR | Catch2 test surface |
| `performance-regression-gate.yml` | push/PR | Perf regression gate |
| `corpus-validation.yml` | push/PR | Test corpus validation |
| `screenshot-regression.yml` | push/PR | Visual regression |
| `binary-size.yml` | push/PR | Size regression gate |
| `toolchain-verify.yml` | dispatch | Tool availability check |
| `publish-packages.yml` | release | Package registry publish |
| `release-drafter.yml` | push | Release note drafting |
| `pages.yml` | push | Documentation site |
| `auto-label.yml` | issues/PR | Auto-labeling |
| `sync-labels.yml` | dispatch | Label catalog sync |
| `stale.yml` | schedule | Issue/PR staleness |
| `notify-failure.yml` | workflow_run | Failure notifications |

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

---

## Action Version Pinning Policy

### Rules

1. **Always pin to a semver tag** (`@v4`, `@v2`) — never use `@main`, `@latest`, or branch refs.
2. **SHA pinning** is preferred for third-party actions outside `actions/*` org:
   ```yaml
   # ✅ SHA-pinned third-party action
   - uses: softprops/action-gh-release@c062e08bd532815e2082a375ba9127f6a901eb57  # v2.2.1
   # ✅ Semver-pinned official action
   - uses: actions/checkout@v4
   # ❌ Branch reference — supply-chain risk
   - uses: some-org/action@main
   ```
3. **Comment the human-readable version** next to SHA pins for auditability.
4. **Audit quarterly** — run `grep -rn 'uses:' .github/workflows/ | grep -v '@v'` to find unpinned refs.

### Approved Action Inventory (v36.5.0)

| Action | Pinned Version | Notes |
|--------|---------------|-------|
| `actions/checkout` | `@v4` | Official |
| `actions/upload-artifact` | `@v4` | Official |
| `actions/download-artifact` | `@v4` | Official |
| `actions/cache` | `@v4` | Official |
| `actions/setup-python` | `@v5` | Official |
| `actions/setup-node` | `@v4` | Official |
| `actions/github-script` | `@v7` | Official |
| `github/codeql-action/*` | `@v4` | GitHub Security |
| `ilammy/msvc-dev-cmd` | `@v1` | MSVC environment setup |
| `lukka/get-cmake` | `@v4.3.1` | CMake/Ninja provisioning |
| `softprops/action-gh-release` | `@v2` | Release asset publishing |
| `dorny/test-reporter` | `@v1` | Test result rendering |
| `EndBug/label-sync` | `@v2` | Label catalog sync |

---

## Node.js Runtime Migration Policy

GitHub Actions is migrating from Node.js 16 → 20 → 24. Actions using deprecated
Node.js versions emit warnings and will eventually fail.

### Rules

1. **All new actions must target Node.js 20+** (the current GitHub-supported minimum).
2. **Set `FORCE_JAVASCRIPT_ACTIONS_TO_NODE24: true`** in the workflow `env:` block when
   testing Node 24 readiness (opt-in during transition period).
3. **Monitor CI logs for `Node.js 16 actions are deprecated` warnings** — these indicate
   actions that need upgrading.
4. **When upgrading an action version specifically for Node.js compatibility**, note it in
   the commit message: `ci: upgrade X to vN for Node 24 compat`.

```yaml
# Opt-in to Node 24 for early detection of incompatibilities
env:
  FORCE_JAVASCRIPT_ACTIONS_TO_NODE24: true
```

---

## Permissions-First Workflow Authoring

### Rules

1. **Every workflow file MUST have a top-level `permissions:` block** — never rely on
   the repository default (which is `write-all` for classic tokens).
2. **Start with `contents: read`** and add only what the workflow actually needs.
3. **Document why each permission is needed** with an inline comment:
   ```yaml
   permissions:
     contents: write    # create GitHub Release, push tags
     actions: read      # download workflow artifacts
     issues: write      # open issue on failure
     id-token: write    # OIDC for package publishing
   ```
4. **Job-level permissions override workflow-level** — use job-level when different
   jobs in the same workflow need different scopes.
5. **Never use `permissions: write-all`** — it's the permissions equivalent of `chmod 777`.

### Common Permission Patterns

| Workflow Type | Minimum Permissions |
|-------------- |-------------------- |
| Build + test (read-only) | `contents: read` |
| Release (create GH Release) | `contents: write`, `actions: read` |
| Package publish (OIDC) | `contents: read`, `packages: write`, `id-token: write` |
| Issue/PR automation | `contents: read`, `issues: write` or `pull-requests: write` |
| CodeQL scanning | `contents: read`, `security-events: write` |
| Pages deployment | `contents: read`, `pages: write`, `id-token: write` |

---

## Reusable Workflow Patterns

### When to Extract a Reusable Workflow

Extract a called workflow (`.github/workflows/reusable-*.yml`) when:
1. Two or more workflows share the same job definition with minor parameter differences.
2. The shared logic is ≥20 lines and changes together.
3. The extracted workflow has a clear input/output contract.

### Naming Convention

```
.github/workflows/reusable-<purpose>.yml   # Called workflow
.github/workflows/<trigger>-<purpose>.yml  # Caller workflow
```

### Input Contract

```yaml
on:
  workflow_call:
    inputs:
      configuration:
        type: string
        required: true
        default: Release
      run-tests:
        type: boolean
        required: false
        default: true
    secrets:
      DEPLOY_TOKEN:
        required: false
```

### Rules

1. **Reusable workflows must set their own `permissions:`** — caller permissions don't propagate.
2. **Use `workflow_call` trigger only** — reusable workflows must not also trigger on `push`/`pull_request`.
3. **Prefer `inputs` over `env` variables** for parameterization — inputs are typed and documented.

---

## Concurrency and Cancellation

### Rules

1. **PR workflows should cancel in-progress runs** when a new commit is pushed:
   ```yaml
   concurrency:
     group: ${{ github.workflow }}-${{ github.ref }}
     cancel-in-progress: true
   ```
2. **Release workflows must NOT cancel** — a partially cancelled release leaves broken artifacts:
   ```yaml
   concurrency:
     group: release-${{ github.ref_name }}
     cancel-in-progress: false
   ```
3. **Scheduled workflows** should use `cancel-in-progress: false` to avoid skipping runs.
