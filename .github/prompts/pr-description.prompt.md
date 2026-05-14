---
mode: ask
description: "Generate a pull request description for an ExplorerLens change. Follows the PR authoring standards from .github/instructions/pr-authoring.instructions.md."
---

# PR Description — ExplorerLens

Generate a PR description for the following change:

**Branch:** ${input:branchName}
**Type:** ${input:type:feat} (feat/fix/perf/refactor/docs/test/chore)
**Scope:** ${input:scope:engine} (engine/shell/manager/cli/tests/ci/ai/gpu/cache/decoder)

## PR Title

```powershell
${input:type:feat}(${input:scope:engine}): ${input:imperativeSummary}
```

## PR Body Template

```markdown
## Summary
${input:oneParagraphSummary}

## Changes
${input:listOfChangedFiles}

## Testing
- [ ] `.\build-scripts\Build-MSVC.ps1 -Test` passes (0 errors, 0 warnings)
- [ ] ${input:specificTestsRun}
- [ ] No P95 benchmark regression > 10%

## Security Considerations
${input:securityNotes}

## Checklist
- [ ] Zero warnings build (MSVC v145)
- [ ] New headers registered in ENGINE_HEADERS (Engine/CMakeLists.txt)
- [ ] New sources registered in ENGINE_SOURCES
- [ ] New tests in EngineTests_Platform.cpp + RUN_TEST() in EngineTests.cpp
- [ ] No corporate artifacts (intel.com, proxy, port 928)
- [ ] CHANGELOG.md updated under ## [Unreleased]
```

## Validation Steps to Run

```powershell
# 1. Build check
.\build-scripts\Build-MSVC.ps1 -Test

# 2. Corporate scrub
git grep -rn "intel.com|928\b" -- "*.ps1" "*.yml" "*.h" "*.cpp" "*.md"

# 3. Type collision check (for new types)
# Select-String -Path Engine\**\*.h -Pattern "\bNewTypeName\b" -Recurse

# 4. LENSTYPE enum check (if adding format)
# Select-String -Path LENSShell\LENSArchive.h -Pattern "NEW_ENUM_VALUE"
```
