# Publish-GitHubRelease.ps1
# Sprint 16: GitHub Releases automation
# Builds, signs, packages, and publishes release to GitHub
# Date: February 17, 2026

[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)]
    [string]$Version,
    
    [Parameter()]
    [string]$ReleaseNotes = "",
    
    [Parameter()]
    [switch]$Prerelease,
    
    [Parameter()]
    [switch]$SkipBuild,
    
    [Parameter()]
    [switch]$SkipSigning,
    
    [Parameter()]
    [switch]$DryRun
)

$ErrorActionPreference = "Stop"

Write-Host "=== DarkThumbs GitHub Release Publisher ===" -ForegroundColor Cyan
Write-Host "Version: $Version" -ForegroundColor Cyan
Write-Host "Prerelease: $Prerelease" -ForegroundColor Cyan
Write-Host ""

# Validate version format
if ($Version -notmatch '^\d+\.\d+\.\d+(-\w+)?$') {
    Write-Host "ERROR: Invalid version format: $Version" -ForegroundColor Red
    Write-Host "Expected format: X.Y.Z or X.Y.Z-suffix (e.g., 7.0.1 or 7.1.0-beta)" -ForegroundColor Yellow
    exit 1
}

# Check for GitHub CLI
$gh = Get-Command gh -ErrorAction SilentlyContinue
if (-not $gh) {
    Write-Host "ERROR: GitHub CLI (gh) not found" -ForegroundColor Red
    Write-Host "Install: winget install GitHub.cli" -ForegroundColor Yellow
    exit 1
}

# Check authentication
$authStatus = gh auth status 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Not authenticated with GitHub" -ForegroundColor Red
    Write-Host "Run: gh auth login" -ForegroundColor Yellow
    exit 1
}

Write-Host "✓ GitHub CLI authenticated" -ForegroundColor Green
Write-Host ""

# Build if needed
if (-not $SkipBuild) {
    Write-Host "=== Building Release ===" -ForegroundColor Yellow
    
    & "$PSScriptRoot\Build-Production-SlowMachine.ps1" -Clean
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "ERROR: Build failed" -ForegroundColor Red
        exit 1
    }
    
    Write-Host "✓ Build complete" -ForegroundColor Green
    Write-Host ""
}

# Sign binaries
if (-not $SkipSigning) {
    Write-Host "=== Signing Binaries ===" -ForegroundColor Yellow
    
    & "$PSScriptRoot\Sign-Release.ps1"
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "⚠ Signing failed or was skipped" -ForegroundColor Yellow
    } else {
        Write-Host "✓ Signing complete" -ForegroundColor Green
    }
    Write-Host ""
}

# Create release packages
Write-Host "=== Creating Release Packages ===" -ForegroundColor Yellow

$releaseDir = "$PSScriptRoot\..\..\release\v$Version"
New-Item -ItemType Directory -Path $releaseDir -Force | Out-Null

# 1. Portable ZIP
Write-Host "Creating portable ZIP..." -ForegroundColor Cyan
& "$PSScriptRoot\..\..\packaging\Build-PortableZip.ps1" -Version $Version -OutputPath "$releaseDir\DarkThumbs-v$Version-Portable.zip"

# 2. MSI Installer (if exists)
$msiPath = "$PSScriptRoot\..\..\packaging\DarkThumbs-Setup.msi"
if (Test-Path $msiPath) {
    Write-Host "Copying MSI installer..." -ForegroundColor Cyan
    Copy-Item -Path $msiPath -Destination "$releaseDir\DarkThumbs-v$Version-Setup.msi"
}

# 3. Generate SHA256 checksums
Write-Host "Generating checksums..." -ForegroundColor Cyan
$checksumFile = "$releaseDir\SHA256SUMS.txt"
Get-ChildItem -Path $releaseDir -File | Where-Object { $_.Name -ne "SHA256SUMS.txt" } | ForEach-Object {
    $hash = (Get-FileHash -Path $_.FullName -Algorithm SHA256).Hash
    "$hash  $($_.Name)" | Add-Content -Path $checksumFile
}

Write-Host "✓ Packages created" -ForegroundColor Green
Write-Host ""

# Prepare release notes
if (-not $ReleaseNotes) {
    $changelogPath = "$PSScriptRoot\..\..\CHANGELOG.md"
    
    if (Test-Path $changelogPath) {
        Write-Host "Extracting release notes from CHANGELOG.md..." -ForegroundColor Cyan
        
        # Extract section for this version
        $changelog = Get-Content -Path $changelogPath -Raw
        $versionSection = [regex]::Match($changelog, "## \[$Version\].*?(?=## \[|\z)", [System.Text.RegularExpressions.RegexOptions]::Singleline).Value
        
        if ($versionSection) {
            $ReleaseNotes = $versionSection
        } else {
            $ReleaseNotes = "Release $Version"
        }
    } else {
        $ReleaseNotes = "Release $Version"
    }
}

# Create release notes file
$releaseNotesPath = "$releaseDir\RELEASE_NOTES.md"
Set-Content -Path $releaseNotesPath -Value $ReleaseNotes -Encoding UTF8

# Display release info
Write-Host "=== Release Information ===" -ForegroundColor Cyan
Write-Host "Tag:         v$Version" -ForegroundColor White
Write-Host "Prerelease:  $Prerelease" -ForegroundColor White
Write-Host "Assets:" -ForegroundColor White
Get-ChildItem -Path $releaseDir -File | ForEach-Object {
    $sizeMB = [math]::Round($_.Length / 1MB, 2)
    Write-Host "  - $($_.Name) ($sizeMB MB)" -ForegroundColor Gray
}
Write-Host ""

if ($DryRun) {
    Write-Host "[DRY RUN] Would create GitHub release with these assets" -ForegroundColor Yellow
    Write-Host "Release directory: $releaseDir" -ForegroundColor Gray
    exit 0
}

# Create GitHub release
Write-Host "=== Publishing to GitHub ===" -ForegroundColor Yellow

$releaseArgs = @(
    "release", "create",
    "v$Version",
    "--title", "DarkThumbs v$Version",
    "--notes-file", $releaseNotesPath
)

if ($Prerelease) {
    $releaseArgs += "--prerelease"
}

# Add all asset files
Get-ChildItem -Path $releaseDir -File | Where-Object { $_.Name -ne "RELEASE_NOTES.md" } | ForEach-Object {
    $releaseArgs += $_.FullName
}

Write-Host "Creating release: gh $($releaseArgs -join ' ')" -ForegroundColor Gray
Write-Host ""

try {
    & gh @releaseArgs
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host ""
        Write-Host "✓ Release published successfully!" -ForegroundColor Green
        Write-Host ""
        Write-Host "View release: https://github.com/YOUR_ORG/DarkThumbs/releases/tag/v$Version" -ForegroundColor Cyan
        Write-Host ""
        Write-Host "Next steps:" -ForegroundColor Yellow
        Write-Host "  1. Update package managers (Scoop, WinGet)" -ForegroundColor Gray
        Write-Host "  2. Announce on social media / forums" -ForegroundColor Gray
        Write-Host "  3. Update website download links" -ForegroundColor Gray
        exit 0
    } else {
        Write-Host "ERROR: GitHub release creation failed" -ForegroundColor Red
        exit 1
    }
} catch {
    Write-Host "ERROR: $(_)" -ForegroundColor Red
    exit 1
}
