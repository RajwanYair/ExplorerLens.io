---
mode: agent
description: "Sweep the ExplorerLens workspace for dead code, stale references, orphan files, and inconsistencies"
---

# Workspace Hygiene Sweep — ExplorerLens

Run a comprehensive hygiene audit across the workspace. Identify stale references,
dead code, orphan files, and inconsistencies that accumulate over sprint deliveries.

## Scope

Focus area: `${input:scope:all}` (all | engine | docs | ci | config)

## Audit Checklist

### 1. Stale Version References

```powershell
# Find files still containing the PREVIOUS version string
$oldVer = "<previous version>"
git ls-files | Where-Object { $_ -notmatch '^external/' } | ForEach-Object {
    $c = Get-Content $_ -Raw -ErrorAction SilentlyContinue
    if ($c -match [regex]::Escape($oldVer)) { Write-Host "STALE: $_" }
}
```

Check that `VERSION`, `CHANGELOG.md`, `copilot-instructions.md`, and all 20 version-bearing
files match the current version. Reference `version-bump.instructions.md` for the full registry.

### 2. Orphan Headers

Search `Engine/CMakeLists.txt` ENGINE_HEADERS list and verify every listed `.h` file exists on disk.
Then search `Engine/**/*.h` for headers NOT listed in ENGINE_HEADERS.

### 3. Orphan Sources

Same as headers but for ENGINE_SOURCES and `.cpp` files.

### 4. Dead Test References

Check `Engine/Tests/EngineTestsExterns.h` for `extern void` declarations where the
corresponding `TEST()` body doesn't exist in any `EngineTests_*.cpp` file.

Check `Engine/Tests/EngineTests.cpp` for `RUN_TEST()` calls where the extern declaration
is missing from `EngineTestsExterns.h`.

### 5. Orphan Workflow Files

List all `.github/workflows/*.yml` files and verify each one:
- Has been triggered in the last 90 days (check git log for workflow file changes)
- Is referenced in `ai-tooling-capabilities.md` workflow inventory
- Has a valid `on:` trigger

### 6. Documentation Link Rot

Scan all `.md` files for internal links `[text](path)` and verify the target exists.
Flag broken links with file name and line number.

### 7. Corporate Artifact Scrub

```powershell
git grep -rn "intel\.com|proxy\.|:928\b|:911\b|password|secret|api[._]key" \
    -- "*.ps1" "*.yml" "*.h" "*.cpp" "*.json" "*.md"
```

### 8. Config File Consistency

- `.editorconfig` vs `.vscode/settings.json` — no conflicting rules
- `.markdownlint.json` vs `documentation.instructions.md` — rules aligned
- `.gitattributes` — all listed patterns match actual file types in repo

### 9. Unused Dependencies

Check `external/LIBRARY_INVENTORY.md` against actual `#include` usage in `Engine/**/*.h`.
Flag libraries listed in inventory but never included.

### 10. TODO/FIXME/HACK Audit

```powershell
git grep -rn "TODO\|FIXME\|HACK\|XXX" -- "*.h" "*.cpp" "*.ps1"
```

Report each occurrence with file, line, and context. Classify as:
- **Actionable** — has a clear next step
- **Stale** — refers to completed work
- **Permanent** — intentional limitation (should be converted to a code comment)

## Output Format

```markdown
## Hygiene Report — ExplorerLens v<version>

### Summary
| Category | Issues Found | Critical |
|----------|-------------|----------|
| Stale versions | N | Y/N |
| Orphan headers | N | Y/N |
| ...

### Issues (by severity)

#### Critical
1. [file](path#LN): description

#### Warning
1. [file](path#LN): description

#### Info
1. [file](path#LN): description

### Recommended Actions
1. ...
```

## Constraints

- This is a **read-only audit** — do not modify files unless explicitly asked
- Flag issues; don't auto-fix (the user decides what to address)
- Skip `external/` directory (third-party code)
- Skip `build/` and `x64/` directories (build artifacts)
