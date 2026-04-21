---
mode: agent
description: "Diagnose and fix a failing GitHub Actions workflow run for ExplorerLens"
---

# CI Troubleshooting — ExplorerLens

A GitHub Actions workflow has failed. Diagnose the root cause and propose a fix.

## Inputs

- **Workflow:** `${input:workflowName}` (e.g., `ci-matrix.yml`, `release.yml`)
- **Failing step:** `${input:failingStep}`
- **Error message:** `${input:errorMessage}`

## Diagnostic Procedure

### Step 1 — Read the Workflow

Read `.github/workflows/${input:workflowName}` to understand:
- Trigger conditions (`on:`)
- Runner OS and matrix strategy
- Job dependencies (`needs:`)
- Permissions block

### Step 2 — Match Against Known Failure Patterns

| Pattern | Root Cause | Fix |
|---------|-----------|-----|
| `toolset "14.50" not found` | MSVC v145 not on GitHub runner | Remove `toolset:` from `ilammy/msvc-dev-cmd` |
| `CMake Error: could not find preset` | Wrong preset name or missing `CMakePresets.json` | Check preset names match exactly |
| `vcvarsall.bat not found` | MSVC not installed on runner | Use `ilammy/msvc-dev-cmd@v1` before cmake |
| `permission denied` | Missing `permissions:` block | Add required permissions to workflow |
| `Node.js 16 actions are deprecated` | Old action version | Upgrade action to latest major |
| `Error: Resource not accessible by integration` | GITHUB_TOKEN lacks scope | Add permission (e.g., `issues: write`) |
| `Process completed with exit code 1` | Generic failure | Read full log for actual error |
| `The process '/usr/bin/git' failed with exit code 128` | Shallow clone issue | Add `fetch-depth: 0` to checkout |
| `LINK : fatal error LNK1104` | Missing library | Check external lib build step ran first |
| `error C2039: is not a member of` | Missing include or namespace | Check `#include` and `using namespace` |

### Step 3 — Check Recent Changes

Search for recent commits that may have introduced the failure:
- `git log --oneline -10 -- .github/workflows/`
- `git log --oneline -5 -- Engine/CMakeLists.txt`

### Step 4 — Propose Fix

Provide:
1. **Root cause** — one sentence explaining why it failed
2. **Fix** — exact file path, line number, and replacement text
3. **Verification** — how to confirm the fix works (push to branch, re-run workflow)

## Constraints

- Do NOT change workflow triggers without understanding downstream impact
- Do NOT remove `permissions:` blocks — only add or tighten
- Do NOT pin MSVC toolset versions in CI (see `cicd.instructions.md`)
- Reference `cicd.instructions.md` for approved action versions
- After fixing, verify the workflow still has `permissions:` and `timeout-minutes:`

## Output Format

```markdown
## Diagnosis

**Workflow:** <name>
**Failing Step:** <step name>
**Root Cause:** <explanation>

## Fix

**File:** `.github/workflows/<name>.yml`
**Line:** <N>
**Change:** <before> → <after>

## Verification

Push to a feature branch and verify the workflow passes:
\`\`\`powershell
git checkout -b fix/ci-<issue>
git add -A && git commit -m "ci: fix <description>"
git push origin fix/ci-<issue>
\`\`\`
```
