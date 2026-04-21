---
applyTo: "**"
---

# Version Bump Checklist — ExplorerLens

## One Command to Do It All

```powershell
.\build-scripts\Bump-Version.ps1 -Version "X.Y.Z" -Codename "Codename" -TestCount NNNN `
    -ChangelogEntry "Short release summary" -TagAndPush
```

This is the **only** correct way to bump a version. Never edit version-bearing files manually.

---

## Complete Version-Bearing File Registry (20 files)

All 20 files are updated by `Bump-Version.ps1`. Files marked *(conditional)* are only updated when
they exist on disk; the script skips them silently if absent.

| # | File | What Changes | Conditional? |
|---|------|-------------|--------------|
| 1 | `VERSION` | Plain version string | No |
| 2 | `CHANGELOG.md` | Prepends new `## [X.Y.Z]` section | No |
| 3 | `CMakeLists.txt` | `project(... VERSION X.Y.Z ...)` | No |
| 4 | `Engine/CMakeLists.txt` | `project(... VERSION X.Y.Z ...)` | No |
| 5 | `Engine/Core/BuildValidation.h` | `LENS_VERSION_STRING`, `LENS_CODENAME` | No |
| 6 | `Engine/Core/SBOMGenerator.h` | `ExplorerLens-X.Y.Z` string literals | No |
| 7 | `Engine/Tests/benchmarks/baseline.json` | `_comment`, `version`, `_updated` | No |
| 8 | `LENSManager/LENSManager.rc` | `FILEVERSION`, `PRODUCTVERSION`, `FileVersion`, `ProductVersion` | No |
| 9 | `LENSShell/LENSShell.rc` | `FILEVERSION`, `PRODUCTVERSION`, `FileVersion`, `ProductVersion` | No |
| 10 | `README.md` | Tests badge count, feature table version row | No |
| 11 | `vcpkg.json` | `"version"` field | No |
| 12 | `docs/assets/social-preview.svg` | Version chip, codename label | No |
| 13 | `docs/assets/architecture-build.svg` | MSI artifact filename chip, version label | No |
| 14 | `docs/USER_GUIDE.md` | `**Version:** X.Y.Z "Codename"` header line | No |
| 15 | `docs/SBOM.json` | `serialNumber`, `metadata.component.version`, `timestamp` | No |
| 16 | `.github/copilot-instructions.md` | Version + codename in header; test count | No |
| 17 | `.github/standards/tool-versions.md` | Version header line | No |
| 18 | `.github/standards/build-method.md` | Version reference line | No |
| 19 | `packaging/npm/package.json` | `"version"` field | **Yes** |
| 20 | `Dockerfile` | `ARG EXPLORERLENS_VERSION=X.Y.Z` | **Yes** |

---

## Graphics Files — Updated Automatically

`Bump-Version.ps1` patches these SVG files with regex replacements:

- `docs/assets/social-preview.svg` — version chip, codename, build/test stats
- `docs/assets/architecture-build.svg` — MSI filename chip (`ExplorerLens-X.Y.Z-x64.msi`)

If a graphic update is missed, patch manually and verify in a browser.

---

## How to Find Missing Version References

Run this before pushing to verify no file was missed:

```powershell
# Find all tracked files still containing the OLD version number
$oldVer = "33.0.0"  # replace with previous version
git ls-files | Where-Object { $_ -notmatch '^external/' -and $_ -notmatch '^packages/' } | ForEach-Object {
    $c = Get-Content $_ -Raw -ErrorAction SilentlyContinue
    if ($c -match [regex]::Escape($oldVer)) { Write-Host "STALE: $_" }
}
```

---

## Adding a New Version-Bearing File

When a new file needs to track the version:

1. Add the update logic to `build-scripts/Bump-Version.ps1` (follow existing `# N. FileName` pattern)
2. Add the file to the registry table in this file (`version-bump.instructions.md`)
3. Add it to the `$details` string at the end of `Bump-Version.ps1`
4. Update the count in `.github/copilot-instructions.md` ("Updates all N version-bearing files")

---

## Release Artifact Checklist (run after Bump-Version.ps1)

```powershell
# 1. Build locally (Engine + LENSShell + LENSManager)
.\build-scripts\Build-MSVC.ps1 -Clean -Test
msbuild LENSShell.sln /p:Configuration=Release /p:Platform=x64 /p:PlatformToolset=v145 /m /v:minimal

# 2. Stage binaries
Copy-Item "LENSShell\x64\Release\LENSShell.dll" "x64\Release\" -Force

# 3. Build installer + portable ZIP
pwsh -File packaging\Build-Installer.ps1   -Version "X.Y.Z" -Configuration Release
pwsh -File packaging\Build-PortableZip.ps1 -Version "X.Y.Z" -Configuration Release

# 4. Generate checksums
$out = "packaging\output"
@( "$out\ExplorerLens-Setup-X.Y.Z.msi",
   "$out\ExplorerLens-X.Y.Z-Portable.zip",
   "x64\Release\LENSShell.dll",
   "x64\Release\LENSManager.exe" ) |
  ForEach-Object { "$((Get-FileHash $_ -Algorithm SHA256).Hash)  $(Split-Path $_ -Leaf)" } |
  Out-File "$out\SHA256SUMS.txt" -Encoding utf8

# 5. Upload to GitHub Release
gh auth login   # if token expired
gh release upload vX.Y.Z `
    "$out\ExplorerLens-Setup-X.Y.Z.msi" `
    "$out\ExplorerLens-X.Y.Z-Portable.zip" `
    "x64\Release\LENSShell.dll" `
    "x64\Release\LENSManager.exe" `
    "$out\SHA256SUMS.txt" --clobber
```

---

## Post-Release Verification

| Step | Command / Check |
|------|----------------|
| Tag pushed | `git tag -l vX.Y.Z` |
| Release exists | `gh release view vX.Y.Z` |
| All artifacts attached | GitHub Release page shows .dll, .exe, .msi, .zip, SHA256SUMS.txt, SBOM.json |
| Package registries | `publish-packages.yml` summary shows ✅ for all 5 (NuGet/npm/Container/Maven/RubyGems) |
| No stale old version | Run the "Find Missing" script above with previous version |
