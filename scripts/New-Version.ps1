<#
.SYNOPSIS
    Bumps the ExplorerLens version across all version-bearing files atomically.

.DESCRIPTION
    Updates the canonical version in VERSION, CMakeLists.txt (root + Engine),
    Engine/Core/BuildValidation.h, LENSManager/LENSManager.rc, and key docs.
    Optionally creates a git tag and pushes to trigger the release workflow.

.PARAMETER NewVersion
    New semantic version string, e.g. "15.3.0"

.PARAMETER Codename
    Release codename, e.g. "Zenith-T"

.PARAMETER DryRun
    Show what would change without writing any files.

.PARAMETER Tag
    Create git tag vX.Y.Z after updating files.

.PARAMETER Push
    Push the tag to origin (requires -Tag). Triggers release.yml.

.EXAMPLE
    .\scripts\New-Version.ps1 -NewVersion "15.3.0" -Codename "Zenith-T"
    .\scripts\New-Version.ps1 -NewVersion "15.3.0" -Codename "Zenith-T" -Tag -Push
    .\scripts\New-Version.ps1 -NewVersion "15.3.0" -Codename "Zenith-T" -DryRun
#>

[CmdletBinding(SupportsShouldProcess)]
param(
    [Parameter(Mandatory)]
    [ValidatePattern('^\d+\.\d+\.\d+$')]
    [string]$NewVersion,

    [Parameter(Mandatory)]
    [string]$Codename,

    [switch]$DryRun,
    [switch]$Tag,
    [switch]$Push
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$Root = Split-Path -Parent $PSScriptRoot

function Write-Step { param([string]$Msg) Write-Host "  $Msg" -ForegroundColor Cyan }
function Write-Ok   { param([string]$Msg) Write-Host "  [OK] $Msg" -ForegroundColor Green }
function Write-Dry  { param([string]$Msg) Write-Host "  [DRY] $Msg" -ForegroundColor Yellow }
function Write-Fail { param([string]$Msg) Write-Host "  [FAIL] $Msg" -ForegroundColor Red; throw $Msg }

Write-Host ""
Write-Host "ExplorerLens Version Bump" -ForegroundColor White -BackgroundColor DarkBlue
Write-Host "  New version : $NewVersion (`"$Codename`")" -ForegroundColor Cyan
if ($DryRun) { Write-Host "  Mode        : DRY RUN (no files written)" -ForegroundColor Yellow }
Write-Host ""

# Read current version for display
$currentVersion = (Get-Content "$Root\VERSION" -Raw).Trim()
Write-Host "  Current version: $currentVersion" -ForegroundColor Gray

# Compute FILEVERSION tuple (e.g. "15, 3, 0")
$vTuple = $NewVersion -replace '\.', ', '

# ── Helper: safe replace with verification ──────────────────────────────────
function Update-File {
    param(
        [string]$RelPath,
        [string]$OldPattern,
        [string]$NewText,
        [switch]$IsRegex
    )

    $full = Join-Path $Root $RelPath
    if (-not (Test-Path $full)) { Write-Fail "File not found: $RelPath" }

    $content = Get-Content $full -Raw

    if ($IsRegex) {
        $updated = $content -replace $OldPattern, $NewText
    } else {
        $updated = $content.Replace($OldPattern, $NewText)
    }

    if ($updated -eq $content) {
        Write-Host "  [SKIP] $RelPath — pattern not found (already updated?)" -ForegroundColor DarkYellow
        return
    }

    if ($DryRun) {
        Write-Dry "$RelPath updated"
    } else {
        Set-Content -Path $full -Value $updated -NoNewline
        Write-Ok "$RelPath"
    }
}

# ── 1. VERSION file ──────────────────────────────────────────────────────────
Write-Step "VERSION"
Update-File -RelPath "VERSION" -OldPattern $currentVersion -NewText $NewVersion

# ── 2. Root CMakeLists.txt ───────────────────────────────────────────────────
Write-Step "CMakeLists.txt (root)"
Update-File -RelPath "CMakeLists.txt" `
    -OldPattern "Version: $currentVersion" -NewText "Version: $NewVersion `"$Codename`""
Update-File -RelPath "CMakeLists.txt" `
    -OldPattern "VERSION $currentVersion" -NewText "VERSION $NewVersion"

# ── 3. Engine/CMakeLists.txt ─────────────────────────────────────────────────
Write-Step "Engine/CMakeLists.txt"
Update-File -RelPath "Engine\CMakeLists.txt" `
    -OldPattern "VERSION $currentVersion" -NewText "VERSION $NewVersion"
Update-File -RelPath "Engine\CMakeLists.txt" -OldPattern "Version: $currentVersion" -NewText "Version: $NewVersion"

# ── 4. Engine/Core/BuildValidation.h ─────────────────────────────────────────
Write-Step "Engine/Core/BuildValidation.h"
$bvPath = "Engine\Core\BuildValidation.h"
$major, $minor, $patch = $NewVersion -split '\.'
Update-File -RelPath $bvPath `
    -OldPattern "MinorVersion = $((($currentVersion -split '\.')[1]))" `
    -NewText     "MinorVersion = $minor"
Update-File -RelPath $bvPath `
    -OldPattern "PatchVersion = $((($currentVersion -split '\.')[2]))" `
    -NewText     "PatchVersion = $patch"
Update-File -RelPath $bvPath `
    -OldPattern "VersionString = `"$currentVersion`"" `
    -NewText     "VersionString = `"$NewVersion`""
Update-File -RelPath $bvPath `
    -IsRegex -OldPattern 'Codename = "[^"]*"' `
    -NewText "Codename = `"$Codename`""

# ── 5. LENSManager/LENSManager.rc ────────────────────────────────────────────
Write-Step "LENSManager/LENSManager.rc"
$rcPath = "LENSManager\LENSManager.rc"
$curTuple = $currentVersion -replace '\.', ', '
Update-File -RelPath $rcPath `
    -OldPattern "FILEVERSION     $curTuple, 0" `
    -NewText     "FILEVERSION     $vTuple, 0"
Update-File -RelPath $rcPath `
    -OldPattern "PRODUCTVERSION  $curTuple, 0" `
    -NewText     "PRODUCTVERSION  $vTuple, 0"
Update-File -RelPath $rcPath `
    -OldPattern "VALUE `"FileVersion`", `"$currentVersion.0\0`"" `
    -NewText     "VALUE `"FileVersion`", `"$NewVersion.0\0`""
Update-File -RelPath $rcPath `
    -OldPattern "VALUE `"ProductVersion`", `"$currentVersion.0\0`"" `
    -NewText     "VALUE `"ProductVersion`", `"$NewVersion.0\0`""
Update-File -RelPath $rcPath -IsRegex `
    -OldPattern "version $([regex]::Escape($currentVersion)) """"[^""""]*""""" `
    -NewText     "version $NewVersion `"`"$Codename`"`""

# ── 6. .github/copilot-instructions.md ───────────────────────────────────────
Write-Step ".github/copilot-instructions.md"
Update-File -RelPath ".github\copilot-instructions.md" -IsRegex `
    -OldPattern "- \*\*Version:\*\* $([regex]::Escape($currentVersion)) \(Codename: [^)]+\)" `
    -NewText     "- **Version:** $NewVersion (Codename: $Codename)"

# ── 7. docs/TROUBLESHOOTING.md ───────────────────────────────────────────────
Write-Step "docs/TROUBLESHOOTING.md"
Update-File -RelPath "docs\TROUBLESHOOTING.md" -IsRegex `
    -OldPattern "\*\*Version:\*\* $([regex]::Escape($currentVersion)) [^—]+— all P0" `
    -NewText     "**Version:** $NewVersion `"$Codename`" — all P0"

# ── 8. docs/PLUGIN_DEVELOPMENT.md ────────────────────────────────────────────
Write-Step "docs/PLUGIN_DEVELOPMENT.md"
Update-File -RelPath "docs\PLUGIN_DEVELOPMENT.md" -IsRegex `
    -OldPattern "\*\*Version:\*\* [0-9]+\.[0-9]+\.[0-9]+ ""[^""]*""" `
    -NewText     "**Version:** $NewVersion `"$Codename`""

# ── 9. docs/ROADMAP_V16.md ────────────────────────────────────────────────────
Write-Step "docs/ROADMAP_V16.md"
Update-File -RelPath "docs\ROADMAP_V16.md" -IsRegex `
    -OldPattern '\*\*Current Version:\*\* [0-9]+\.[0-9]+\.[0-9]+ "[^"]*"' `
    -NewText     "**Current Version:** $NewVersion `"$Codename`""

# ── 10. docs/SPRINT_PLAN_100.md — Base Version line ──────────────────────────
Write-Step "docs/SPRINT_PLAN_100.md"
Update-File -RelPath "docs\SPRINT_PLAN_100.md" -IsRegex `
    -OldPattern '\*\*Base Version:\*\* [0-9]+\.[0-9]+\.[0-9]+ "[^"]*"' `
    -NewText     "**Base Version:** $NewVersion `"$Codename`""

# ── Summary ──────────────────────────────────────────────────────────────────
Write-Host ""
if ($DryRun) {
    Write-Host "DRY RUN complete — no files were modified." -ForegroundColor Yellow
    Write-Host "Rerun without -DryRun to apply changes." -ForegroundColor Yellow
} else {
    Write-Host "Version bump complete: $currentVersion → $NewVersion (`"$Codename`")" -ForegroundColor Green
    Write-Host ""
    Write-Host "Next steps:" -ForegroundColor White
    Write-Host "  1. Update CHANGELOG.md — add [$NewVersion] section" -ForegroundColor Gray
    Write-Host "  2. Verify: .\build-scripts\Build-MSVC.ps1 -Test" -ForegroundColor Gray

    if ($Tag) {
        Write-Host ""
        Write-Step "Creating git tag v$NewVersion"
        git -C $Root add -A
        git -C $Root commit -m "chore: bump version to $NewVersion ($Codename)"
        git -C $Root tag "v$NewVersion"
        Write-Ok "Tag v$NewVersion created"

        if ($Push) {
            Write-Step "Pushing to origin (triggers release.yml)"
            git -C $Root push origin main --tags
            Write-Ok "Pushed — GitHub Actions release workflow starting"
        } else {
            Write-Host "  3. Push: git push origin main --tags" -ForegroundColor Gray
        }
    } else {
        Write-Host "  3. Tag:  git tag v$NewVersion" -ForegroundColor Gray
        Write-Host "  4. Push: git push origin main --tags" -ForegroundColor Gray
    }
}
Write-Host ""
