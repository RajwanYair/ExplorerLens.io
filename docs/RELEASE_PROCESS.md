# ExplorerLens — Release Process

**Version:** 15.2.0 "Zenith-S" · **Updated:** March 2026

This document is the authoritative reference for producing a ExplorerLens release.
Every version bump — patch, minor, or major — **must** follow this process exactly.

---

## 1. Version Numbering

ExplorerLens uses **Semantic Versioning** (`MAJOR.MINOR.PATCH`):

| Component | Trigger | Example |
|-----------|---------|---------|
| MAJOR | Breaking API / COM CLSID change | 15 → 16 |
| MINOR | New format decoder or feature | 15.2 → 15.3 |
| PATCH | Bug fix, docs, CI, performance | 15.2.0 → 15.2.1 |

> **Codenames** are assigned at MINOR bumps. They are decorative; the number is canonical.

---

## 2. Files That Must Be Updated for Every Release

Use `scripts/New-Version.ps1` to update all of these atomically:

```powershell
.\scripts\New-Version.ps1 -NewVersion 15.3.0 -Codename "Apex"
```

The script updates all of the following:

| File | Field |
|------|-------|
| `VERSION` | Bare version string |
| `CMakeLists.txt` (root) | `project(...VERSION X.Y.Z...)` |
| `Engine/CMakeLists.txt` | `project(...VERSION X.Y.Z...)` |
| `Engine/Core/BuildValidation.h` | `MajorVersion`, `MinorVersion`, `PatchVersion`, `VersionString`, `Codename` |
| `LENSManager/LENSManager.rc` | `FILEVERSION`, `PRODUCTVERSION`, `FileVersion`, `ProductVersion`, About text |
| `LENSManager/LENSManager.manifest` | `assemblyIdentity version` |
| `.github/copilot-instructions.md` | Version line at top |
| `CHANGELOG.md` | New `[X.Y.Z]` section header |
| `docs/TROUBLESHOOTING.md` | Version header |
| `docs/PLUGIN_DEVELOPMENT.md` | Version header |
| `docs/ROADMAP_V16.md` | Current Version field |
| `docs/SPRINT_PLAN_100.md` | Version in header |

> **Never** manually edit these files — always use `New-Version.ps1` to prevent drift.
> The CI `version-consistency` job in `code-quality.yml` will fail the build if these get out of sync.

---

## 3. Pre-Release Checklist

Before creating a tag, verify all of the following:

### Code Quality
- [ ] `.\build-scripts\Build-MSVC.ps1 -Preset temp-release` builds with **0 errors, 0 warnings**
- [ ] `ctest --test-dir $env:TEMP\ExplorerLens-build -C Release --output-on-failure` — **all tests pass**
- [ ] `.\build-scripts\Build-MSVC.ps1 -Preset temp-release -Test` — combined build+test passes
- [ ] No new `clang-tidy` suppressions without documented rationale in `.clang-tidy`

### Version Consistency
- [ ] `VERSION` file matches `CMakeLists.txt`, `Engine/CMakeLists.txt`, `BuildValidation.h`, `LENSManager.rc`
- [ ] `CHANGELOG.md` has a new `[X.Y.Z]` section with **Added / Fixed / Changed / Performance** subsections
- [ ] `.github/copilot-instructions.md` version line is updated

### Documentation
- [ ] `docs/PERFORMANCE.md` benchmarks are current
- [ ] `README.md` badges correct (dynamic badges auto-update; check static ones)
- [ ] Any new decoders are listed in `README.md` format table

### Packaging
- [ ] `packaging/ExplorerLens.wxs` version attribute updated (for WiX MSI)
- [ ] `packaging/setup.iss` version updated (for Inno Setup)
- [ ] `packaging/ExplorerLens.appxmanifest` version updated (for MSIX)

---

## 4. Release Steps

```powershell
# Step 1: Bump version atomically
.\scripts\New-Version.ps1 -NewVersion X.Y.Z -Codename "Name"

# Step 2: Update CHANGELOG.md — fill in Added / Fixed / Changed / Performance

# Step 3: Build + test (must produce 0 errors, 0 warnings)
.\build-scripts\Build-MSVC.ps1 -Preset temp-release -Test

# Step 4: Commit
git add -A
git commit -m "chore: bump version to X.Y.Z (Codename)"

# Step 5: Tag — triggers release.yml automatically
git tag vX.Y.Z
git push origin main --tags
```

> `release.yml` runs automatically on `git tag vX.Y.Z` and produces all release artifacts.

---

## 5. Release Artifacts (Produced by `release.yml`)

| Artifact | Description |
|----------|-------------|
| `LENSShell.dll` (x64) | COM shell extension |
| `LENSManager.exe` | WTL configuration GUI |
| `lens.exe` | CLI tool (Sprint 17+) |
| `ExplorerLens-X.Y.Z-x64.msi` | WiX installer |
| `ExplorerLens-X.Y.Z-x64.zip` | Portable archive |
| `SHA256SUMS.txt` | Checksums for all artifacts |
| `ExplorerLens-X.Y.Z-SBOM.json` | CycloneDX bill of materials |

---

## 6. Hotfix Releases

For a critical fix on a released tag:

```powershell
# Cherry-pick onto a hotfix branch from the tag
git checkout -b hotfix/X.Y.Z+1 vX.Y.Z
# Apply fix, bump patch version, build, test, tag
.\scripts\New-Version.ps1 -NewVersion X.Y.Z+1 -Codename "Same-Codename"
git add -A && git commit -m "fix(hotfix): <description>"
git tag vX.Y.Z+1
git push origin hotfix/X.Y.Z+1 --tags
```

---

## 7. Post-Release

After a successful release:

1. Verify GitHub Release page shows all 7 artifacts
2. Test download + install of the MSI on a clean Windows 11 VM
3. Run `.\scripts\Set-RepoTopics.ps1` if new topics are relevant
4. Update `docs/SPRINT_PLAN_100.md` — mark completed tasks with `✅ Done vX.Y.Z`
5. Announce in GitHub Discussions (Announcements category)
