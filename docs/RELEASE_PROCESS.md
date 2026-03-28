# ExplorerLens — Release Process

**Version:** 23.6.0 "Vega-W" · **Updated:** March 28, 2026

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

> **`release.yml` runs automatically** on `git tag vX.Y.Z` and:
> 1. Compiles `ExplorerLensEngine.lib` (CMake + Ninja + MSVC via `ilammy/msvc-dev-cmd`)
> 2. Compiles `LENSShell.dll` + `LENSManager.exe` (MSBuild, VS 2022+)
> 3. Builds `ExplorerLens-X.Y.Z-x64.msi` (WiX v6.0.2)
> 4. Creates `ExplorerLens-X.Y.Z-x64.zip` portable archive
> 5. Generates SHA256 checksums and CycloneDX SBOM
> 6. **Auto-publishes** the GitHub Release with all compiled artifacts attached

### Re-triggering a Release Without Re-tagging

If the release workflow fails (e.g. transient runner issue) or needs to be rerun:

```bash
# Option A: Re-run the failed workflow in the GitHub UI
# Actions → Release workflow → ⋯ Re-run all jobs

# Option B: Trigger manually with workflow_dispatch (gh CLI)
gh workflow run release.yml \
  --ref vX.Y.Z \
  -f version_override=X.Y.Z

# Option C: Delete and re-push the tag (also triggers release.yml)
git push origin :refs/tags/vX.Y.Z   # delete remote tag
git tag -d vX.Y.Z                    # delete local tag
git tag vX.Y.Z HEAD
git push origin --tags
```

> **Note:** If the GitHub Release draft already exists, delete it first before re-triggering or the workflow will fail on duplicate tag.

---

## 5. Release Artifacts (Produced by `release.yml`)

Every GitHub Release includes these **compiled binaries** built from source on GitHub-hosted runners:

| Artifact | Description | Condition |
|----------|-------------|----------|
| `LENSShell.dll` (x64) | COM shell extension (IThumbnailProvider) | Always |
| `LENSManager.exe` | WTL configuration GUI | Always |
| `lens.exe` | CLI tool | Sprint 17+ |
| `ExplorerLens-X.Y.Z-x64.msi` | WiX installer (registers COM, sets up shortcuts) | Always |
| `ExplorerLens-X.Y.Z-x64.zip` | Portable archive with all binaries | Always |
| `SHA256SUMS.txt` | SHA256 checksums for all artifacts | Always |
| `ExplorerLens-X.Y.Z-SBOM.json` | CycloneDX bill of materials | Always |
| `verification-report-X.Y.Z.json` | Build quality / artifact manifest | Always |

### How Binaries Are Compiled

```
GitHub Actions runner: windows-latest (Windows Server 2025, VS 2022)
├── ilammy/msvc-dev-cmd@v1              ← sets up cl.exe, ninja, MSBuild in PATH
├── cmake --preset ci-release           ← configures Engine with Ninja + MSVC
├── cmake --build --preset ci-release   ← → ExplorerLensEngine.lib + EngineTests.exe
├── msbuild LENSShell.sln               ← → LENSShell.dll + LENSManager.exe
├── packaging/Build-Installer.ps1       ← → ExplorerLens-X.Y.Z-x64.msi (WiX v6)
└── Compress-Archive                    ← → ExplorerLens-X.Y.Z-x64.zip
```

> **External decoder libraries** (libwebp, libavif, libheif, etc.) are cached between
> CI runs via `actions/cache@v4` keyed on `build-scripts/external-libs/**`. On a cache
> miss, advanced decoders are gracefully disabled but the core binary still compiles.
> Run the library build scripts locally and push to populate the cache for full builds.

### MSI Installer Details

The MSI is built by `packaging/Build-Installer.ps1` which calls
`wix build packaging/ExplorerLens.wxs`. It:
- Installs `LENSShell.dll` to `%ProgramFiles%\ExplorerLens\bin\`
- Registers the COM CLSID `{9E6ECB90-5A61-42BD-B851-D3297D9C7F39}`
- Installs `LENSManager.exe` configuration utility
- Creates Start Menu shortcuts
- Supports per-machine install, upgrade, and silent install (`/quiet`)

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

1. Verify the GitHub Release page shows **all artifacts** (DLL, EXE, MSI, ZIP, SHA256SUMS, SBOM)
2. Test download + install of the MSI on a clean Windows 11 VM
3. Confirm `LENSShell.dll` self-registers via the MSI installer
4. Run `.\.scripts\Set-RepoTopics.ps1` if new topics are relevant
5. Update `docs/SPRINT_PLAN_100.md` — mark completed tasks with `✅ Done vX.Y.Z`
6. Announce in GitHub Discussions (Announcements category)

---

## 8. CI Compilation Details

### Tools Available on `windows-latest` Runner

| Tool | Version | Source |
|------|---------|--------|
| MSVC (cl.exe) | 19.43.x (v143) | VS 2022 Enterprise, pre-installed |
| MSBuild | 17.x | VS 2022, via `microsoft/setup-msbuild@v2` |
| CMake | latest | pre-installed |
| Ninja | latest | provided by `ilammy/msvc-dev-cmd` |
| WiX | 6.0.2 | installed via `dotnet tool install wix` |
| .NET SDK | 10.0.x | via `actions/setup-dotnet@v4` |

> The GitHub runner uses MSVC v143 (VS 2022). The local build uses MSVC v145 (VS 2026
> BuildTools). Both compilers are fully C++20-compliant. The CI build uses the `ci-release`
> CMake preset which resolves all tool paths from the environment (no hardcoded paths).

### Re-running the Release Build Locally

To validate the release build locally before pushing a tag:

```powershell
# Uses the temp-release preset (avoids OneDrive sync conflicts)
.\build-scripts\Build-MSVC.ps1 -Preset temp-release -Test

# Build installer from result
$env:BuildDir = $env:TEMP + "\ExplorerLens-build"
.\packaging\Build-Installer.ps1 -Version (Get-Content VERSION -Raw).Trim() -Configuration Release
```
