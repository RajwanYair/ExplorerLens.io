---
mode: agent
description: "Prepare a release for ExplorerLens. Validates build, scrubs corporate artifacts, checks all 21 version-bearing files, and runs the Bump-Version.ps1 command."
---

# Release Prep — ExplorerLens

You are preparing release **v{{version}}** of ExplorerLens ("{{codename}}").

## Step 1: Confirm Current State

```powershell
# Read current version
Get-Content VERSION
# Read test count
(Select-String -Path Engine\Tests\EngineTests.cpp -Pattern "RUN_TEST\(").Count
# Check build is clean
.\build-scripts\Build-MSVC.ps1 -Test
```

## Step 2: Corporate Artifact Scrub

Run these and confirm ZERO results before proceeding:

```powershell
git grep -rn "intel.com" -- "*.ps1" "*.yml" "*.yaml" "*.md" "*.json" "*.h" "*.cpp"
git grep -rn "proxy"     -- "*.ps1" "*.yml" "*.yaml" "*.md" "*.json"
git grep -rn "928\b"     -- "*.ps1" "*.yml" "*.yaml"
```

## Step 3: Run Bump-Version.ps1

Fill in the placeholders and run:

```powershell
.\build-scripts\Bump-Version.ps1 `
    -Version "{{version}}" `
    -Codename "{{codename}}" `
    -TestCount {{testCount}} `
    -ChangelogEntry "{{changelogEntry}}" `
    -TagAndPush
```

## Step 4: Post-Release Verification

```powershell
# Tag pushed?
git tag -l v{{version}}

# GitHub Release created?
gh release view v{{version}}

# Artifacts attached?
gh release view v{{version}} --json assets --jq '.assets[].name'
```

## Checklist

- [ ] `Build-MSVC.ps1 -Test` passes: 0 errors, 0 warnings
- [ ] No corporate artifacts in tracked files
- [ ] CHANGELOG.md has `## [{{version}}]` section
- [ ] All 21 version-bearing files updated
- [ ] Tag `v{{version}}` pushed → `release.yml` triggered
- [ ] GitHub Release shows all artifacts (.dll, .exe, .msi, .zip, SHA256SUMS.txt, SBOM.json)
- [ ] `publish-packages.yml` shows ✅ for NuGet
