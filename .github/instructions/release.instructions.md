---
applyTo: "**/Bump-Version.ps1,**/CHANGELOG.md,**/VERSION"
---

# Release Authoring Rules — ExplorerLens

## The Only Correct Release Command

```powershell
.\build-scripts\Bump-Version.ps1 -Version "X.Y.Z" -Codename "Codename" -TestCount NNNN `
    -ChangelogEntry "Short release summary" -TagAndPush
```

**Never edit version-bearing files manually.** The script updates all 21 files atomically.

## 21 Version-Bearing Files

See `.github/instructions/version-bump.instructions.md` for the complete registry.
Every file listed there MUST be updated before tagging. The script handles this automatically.

## Pre-Release Checklist

```powershell
# 1. Local build must pass: 0 errors, 0 warnings
.\build-scripts\Build-MSVC.ps1 -Clean -Test

# 2. Scrub corporate artifacts from ALL tracked files
git grep -rn "intel.com" -- "*.ps1" "*.yml" "*.yaml" "*.md" "*.json" "*.h" "*.cpp"
git grep -rn "proxy"     -- "*.ps1" "*.yml" "*.yaml" "*.md" "*.json"
git grep -rn "928\b"     -- "*.ps1" "*.yml" "*.yaml"

# 3. Find stale old-version references
$old = "39.9.0"  # previous version
git ls-files | Where-Object { $_ -notmatch '^external/' } | ForEach-Object {
    $c = Get-Content $_ -Raw -ErrorAction SilentlyContinue
    if ($c -match [regex]::Escape($old)) { Write-Host "STALE: $_" }
}
```

## CHANGELOG.md Entry Format

```markdown
## [X.Y.Z] — YYYY-MM-DD "Codename"

### Added
- Feature description with reference to `Engine/Path/File.h`

### Changed
- What changed and why

### Fixed
- Bug description (#issue or commit SHA)

### Removed
- What was removed and why
```

## Post-Release Verification

| Step | Command / Check |
|------|----------------|
| Tag pushed | `git tag -l vX.Y.Z` |
| Release exists | `gh release view vX.Y.Z` |
| All artifacts | LENSShell.dll, LENSManager.exe, .msi, .zip, SHA256SUMS.txt, SBOM.json |
| NuGet published | `publish-packages.yml` summary ✅ |
| winget PR submitted | Check Windows Package Manager community repo |

## Idempotency Guard

`Bump-Version.ps1` skips files if the new version is already present.
Running it twice on the same version is safe — no duplicate CHANGELOG entries.

## SBOMGenerator.h File Lock

If `Bump-Version.ps1` fails with "used by another process" on `SBOMGenerator.h`,
close any editor holding the file open and re-run with identical parameters.
