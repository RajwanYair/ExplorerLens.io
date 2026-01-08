# Cleanup Duplicate Files
# Consolidates duplicate scripts and documentation
# Run this after reviewing duplicates

#Requires -Version 7.0

param(
    [switch]$DryRun,
    [switch]$Force
)

$ErrorActionPreference = "Stop"
$root = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent

function Write-Action {
    param([string]$Message, [string]$Type = "INFO")
    switch ($Type) {
        "REMOVE" { Write-Host "[REMOVE] $Message" -ForegroundColor Red }
        "ARCHIVE" { Write-Host "[ARCHIVE] $Message" -ForegroundColor Yellow }
        "KEEP" { Write-Host "[KEEP] $Message" -ForegroundColor Green }
        "INFO" { Write-Host "[INFO] $Message" -ForegroundColor Cyan }
        default { Write-Host $Message }
    }
}

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "DarkThumbs - Duplicate File Cleanup" -ForegroundColor Cyan
if ($DryRun) {
    Write-Host "[DRY RUN MODE - No changes will be made]" -ForegroundColor Yellow
}
Write-Host "========================================`n" -ForegroundColor Cyan

# Define duplicates to remove/archive
$duplicatesToArchive = @(
    @{
        Path        = "scripts/install/Install-DarkThumbs.ps1"
        Reason      = "Superseded by scripts/install.ps1"
        ArchivePath = "scripts/install/_archive/Install-DarkThumbs.ps1.old"
    },
    @{
        Path        = "scripts/install/install-x64.ps1"
        Reason      = "Superseded by scripts/install.ps1"
        ArchivePath = "scripts/install/_archive/install-x64.ps1.old"
    },
    @{
        Path        = "scripts/install/uninstall-x64.ps1"
        Reason      = "Use scripts/install.ps1 -Unregister instead"
        ArchivePath = "scripts/install/_archive/uninstall-x64.ps1.old"
    }
)

$documentationToConsolidate = @(
    @{
        Path   = "docs/QUICK_SETUP.md"
        Reason = "Content merged into INSTALLATION_READY.md"
        Action = "Review and delete if no unique content"
    }
)

# Create archive directory
$archiveDir = Join-Path $root "scripts/install/_archive"
if (-not (Test-Path $archiveDir)) {
    Write-Action "Creating archive directory: $archiveDir" "INFO"
    if (-not $DryRun) {
        New-Item -ItemType Directory -Path $archiveDir -Force | Out-Null
    }
}

# Process duplicate scripts
Write-Host "`nDuplicate Scripts:" -ForegroundColor Yellow
Write-Host "==================`n" -ForegroundColor Yellow

foreach ($item in $duplicatesToArchive) {
    $fullPath = Join-Path $root $item.Path
    $archivePath = Join-Path $root $item.ArchivePath
    
    if (Test-Path $fullPath) {
        Write-Action "$($item.Path)" "ARCHIVE"
        Write-Host "  Reason: $($item.Reason)" -ForegroundColor Gray
        Write-Host "  Archive: $($item.ArchivePath)" -ForegroundColor Gray
        
        if (-not $DryRun) {
            $archiveParent = Split-Path $archivePath -Parent
            if (-not (Test-Path $archiveParent)) {
                New-Item -ItemType Directory -Path $archiveParent -Force | Out-Null
            }
            Move-Item -Path $fullPath -Destination $archivePath -Force
            Write-Host "  ✓ Archived" -ForegroundColor Green
        }
    } else {
        Write-Host "  [NOT FOUND] $($item.Path)" -ForegroundColor DarkGray
    }
    Write-Host ""
}

# Report on documentation
Write-Host "`nDocumentation to Review:" -ForegroundColor Yellow
Write-Host "========================`n" -ForegroundColor Yellow

foreach ($item in $documentationToConsolidate) {
    $fullPath = Join-Path $root $item.Path
    
    if (Test-Path $fullPath) {
        Write-Host "$($item.Path)" -ForegroundColor Yellow
        Write-Host "  Action: $($item.Action)" -ForegroundColor Cyan
        Write-Host "  Reason: $($item.Reason)" -ForegroundColor Gray
        Write-Host ""
    }
}

# Show canonical locations
Write-Host "`nCanonical Script Locations:" -ForegroundColor Green
Write-Host "===========================`n" -ForegroundColor Green
Write-Host "  Build:        scripts/build.ps1" -ForegroundColor White
Write-Host "  Install:      scripts/install.ps1" -ForegroundColor White
Write-Host "  Verify:       scripts/verify-tools.ps1" -ForegroundColor White
Write-Host "  Library Build: build-scripts/*.ps1" -ForegroundColor White
Write-Host "  Updates:      build-scripts/update-all-libraries.ps1" -ForegroundColor White

Write-Host "`nCanonical Documentation:" -ForegroundColor Green
Write-Host "========================`n" -ForegroundColor Green
Write-Host "  Installation (user):    INSTALLATION_READY.md" -ForegroundColor White
Write-Host "  Installation (detail):  docs/getting-started/installation.md" -ForegroundColor White
Write-Host "  Build guide:            docs/BUILD_GUIDE.md" -ForegroundColor White
Write-Host "  Tool setup:             docs/TOOLS_SETUP.md" -ForegroundColor White
Write-Host "  COM diagnostics:        docs/COM_REGISTRATION_DIAGNOSTICS.md" -ForegroundColor White

# Summary
Write-Host "`n========================================" -ForegroundColor Cyan
if ($DryRun) {
    Write-Host "DRY RUN COMPLETE - No changes made" -ForegroundColor Yellow
    Write-Host "Run without -DryRun to apply changes" -ForegroundColor Yellow
} else {
    Write-Host "CLEANUP COMPLETE" -ForegroundColor Green
    Write-Host "`nNext steps:" -ForegroundColor Yellow
    Write-Host "  1. Review archived files in scripts/install/_archive/" -ForegroundColor White
    Write-Host "  2. Manually review documentation for unique content" -ForegroundColor White
    Write-Host "  3. Commit changes: git add . && git commit -m 'refactor: consolidate duplicate files'" -ForegroundColor White
}
Write-Host "========================================`n" -ForegroundColor Cyan

exit 0
