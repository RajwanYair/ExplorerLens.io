# Install ExplorerLens Development Environment to PowerShell Profile
# Run this once to enable persistent build tools in all PowerShell sessions
# Version: 1.0 - February 9, 2026

[CmdletBinding()]
param(
    [switch]$Force,
    [switch]$Uninstall
)

$profilePath = $PROFILE
$setupScript = "$PSScriptRoot\Setup-DevEnvironment.ps1"

# Lines to add to profile
$installBlock = @"

# ============================================================================
# ExplorerLens Development Environment - Auto-load
# Installed: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')
# ============================================================================
`$explorerLensEnv = "$setupScript"
if (Test-Path `$explorerLensEnv) {
    . `$explorerLensEnv
} else {
    Write-Warning "ExplorerLens environment script not found: `$explorerLensEnv"
}

"@

# Check if already installed
$currentProfile = if (Test-Path $profilePath) { Get-Content $profilePath -Raw } else { "" }
$isInstalled = $currentProfile -match "ExplorerLens Development Environment"

if ($Uninstall) {
    Write-Host "`n🗑️  Uninstalling ExplorerLens environment from profile..." -ForegroundColor Yellow
    
    if (-not $isInstalled) {
        Write-Host "✓ ExplorerLens environment not found in profile - nothing to uninstall" -ForegroundColor Green
        exit 0
    }
    
    # Remove the block
    $lines = Get-Content $profilePath
    $outLines = @()
    $inBlock = $false
    
    foreach ($line in $lines) {
        if ($line -match "ExplorerLens Development Environment - Auto-load") {
            $inBlock = $true
            continue
        }
        if ($inBlock -and $line -match "^}?\s*$") {
            $inBlock = $false
            continue
        }
        if (-not $inBlock) {
            $outLines += $line
        }
    }
    
    $outLines | Set-Content $profilePath -Force
    
    Write-Host "✓ ExplorerLens environment removed from profile" -ForegroundColor Green
    Write-Host "`nRestart PowerShell for changes to take effect.`n" -ForegroundColor Cyan
    exit 0
}

# Install
Write-Host "`n════════════════════════════════════════════════════════" -ForegroundColor Cyan
Write-Host " ExplorerLens Development Environment Installer" -ForegroundColor Cyan
Write-Host "════════════════════════════════════════════════════════`n" -ForegroundColor Cyan

# Check setup script exists
if (-not (Test-Path $setupScript)) {
    Write-Error "Setup script not found: $setupScript"
    Write-Host "`nMake sure you're running this from the ExplorerLens\scripts directory.`n" -ForegroundColor Yellow
    exit 1
}

Write-Host "Setup Script: $setupScript" -ForegroundColor Gray
Write-Host "Profile Path: $profilePath" -ForegroundColor Gray
Write-Host ""

# Check if already installed
if ($isInstalled -and -not $Force) {
    Write-Host "✓ ExplorerLens environment is already installed in your profile" -ForegroundColor Green
    Write-Host "`nTo reinstall, run: .\Install-DevEnvironment.ps1 -Force" -ForegroundColor Gray
    Write-Host "To uninstall, run: .\Install-DevEnvironment.ps1 -Uninstall`n" -ForegroundColor Gray
    exit 0
}

# Create profile if it doesn't exist
if (-not (Test-Path $profilePath)) {
    Write-Host "Creating PowerShell profile: $profilePath" -ForegroundColor Yellow
    $profileDir = Split-Path $profilePath -Parent
    if (-not (Test-Path $profileDir)) {
        New-Item -ItemType Directory -Path $profileDir -Force | Out-Null
    }
    New-Item -ItemType File -Path $profilePath -Force | Out-Null
}

# Backup existing profile
$backupPath = "$profilePath.backup-$(Get-Date -Format 'yyyyMMdd-HHmmss')"
Write-Host "Creating backup: $backupPath" -ForegroundColor Yellow
Copy-Item $profilePath $backupPath -Force

# Add to profile
if ($Force -and $isInstalled) {
    Write-Host "Removing existing installation..." -ForegroundColor Yellow
    
    # Remove old block first
    $lines = Get-Content $profilePath
    $outLines = @()
    $inBlock = $false
    
    foreach ($line in $lines) {
        if ($line -match "ExplorerLens Development Environment") {
            $inBlock = $true
            continue
        }
        if ($inBlock -and ($line -match "^}?\s*$" -or $line.Trim() -eq "")) {
            $inBlock = $false
            continue
        }
        if (-not $inBlock) {
            $outLines += $line
        }
    }
    
    $outLines | Set-Content $profilePath -Force
}

# Append new block
Add-Content -Path $profilePath -Value $installBlock

Write-Host "✓ ExplorerLens environment installed to profile" -ForegroundColor Green

# Test the installation
Write-Host "`nTesting installation..." -ForegroundColor Yellow

try {
    . $setupScript
    Write-Host "✓ Setup script loaded successfully" -ForegroundColor Green
    
    if ($Global:ExplorerLensEnvLoaded) {
        Write-Host "✓ Environment initialized" -ForegroundColor Green
    }
} catch {
    Write-Warning "Failed to load setup script: $_"
    Write-Host "`nThe installation completed, but there may be an issue with the script." -ForegroundColor Yellow
    Write-Host "Check the setup script for errors: $setupScript`n" -ForegroundColor Yellow
}

Write-Host "`n════════════════════════════════════════════════════════" -ForegroundColor Cyan
Write-Host "✅ Installation Complete!" -ForegroundColor Green
Write-Host "════════════════════════════════════════════════════════`n" -ForegroundColor Cyan

Write-Host "What happens now:" -ForegroundColor Cyan
Write-Host "  1. Every new PowerShell session will auto-load ExplorerLens tools" -ForegroundColor Gray
Write-Host "  2. MSVC environment (CL, NMake, Link) will be available" -ForegroundColor Gray
Write-Host "  3. Build aliases (dtbuild, dttest, dtclean) will be ready" -ForegroundColor Gray
Write-Host "  4. All tools verified before first use" -ForegroundColor Gray
Write-Host ""
Write-Host "Try it now:" -ForegroundColor Cyan
Write-Host "  1. Start a new PowerShell window" -ForegroundColor Gray
Write-Host "  2. Run: Show-ExplorerLensInfo" -ForegroundColor Gray
Write-Host "  3. Run: dtbuild" -ForegroundColor Gray
Write-Host ""
Write-Host "Backup created: $backupPath" -ForegroundColor Yellow
Write-Host ""

# Offer to restart
$restart = Read-Host "Open a new PowerShell window to test? (Y/N)"
if ($restart -eq 'Y' -or $restart -eq 'y') {
    Start-Process pwsh -NoNewWindow
    Write-Host "✓ New PowerShell window opened - check if environment loaded`n" -ForegroundColor Green
}


