#Requires -Version 7.0
<#
.SYNOPSIS
    Install barebone PowerShell profile for DarkThumbs development
.DESCRIPTION
    Installs a minimal, high-performance PowerShell profile optimized for slow machines
    Backs up existing profile if present
.PARAMETER Force
    Overwrite existing profile without prompting
.PARAMETER Backup
    Create backup of existing profile (default: true)
.PARAMETER NoBackup
    Skip backup of existing profile
.EXAMPLE
    .\Install-BareboneProfile.ps1
.EXAMPLE
    .\Install-BareboneProfile.ps1 -Force -NoBackup
#>

param(
    [switch]$Force,
    [switch]$Backup = $true,
    [switch]$NoBackup
)

$ErrorActionPreference = "Stop"

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Barebone Profile Installer" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Determine profile path
$profilePath = $PROFILE.CurrentUserAllHosts
$profileDir = Split-Path $profilePath
$sourcePath = Join-Path $PSScriptRoot "barebone-profile.ps1"

if (-not (Test-Path $sourcePath)) {
    Write-Host "❌ ERROR: barebone-profile.ps1 not found!" -ForegroundColor Red
    Write-Host "   Expected location: $sourcePath" -ForegroundColor Yellow
    exit 1
}

Write-Host "Profile Location: $profilePath" -ForegroundColor Gray
Write-Host "Source Template: $sourcePath" -ForegroundColor Gray
Write-Host ""

# Check if profile directory exists
if (-not (Test-Path $profileDir)) {
    Write-Host "Creating profile directory..." -ForegroundColor Yellow
    New-Item -ItemType Directory -Path $profileDir -Force | Out-Null
    Write-Host "✅ Created: $profileDir" -ForegroundColor Green
}

# Backup existing profile
if ((Test-Path $profilePath) -and -not $NoBackup) {
    $timestamp = Get-Date -Format 'yyyyMMdd-HHmmss'
    $backupPath = "$profilePath.backup-$timestamp"
    
    try {
        Copy-Item $profilePath $backupPath -Force
        Write-Host "✅ Backed up existing profile to:" -ForegroundColor Green
        Write-Host "   $backupPath" -ForegroundColor Gray
        Write-Host ""
    } catch {
        Write-Host "⚠️  Warning: Could not backup profile: $_" -ForegroundColor Yellow
    }
}

# Check if profile exists and prompt if needed
if ((Test-Path $profilePath) -and -not $Force) {
    Write-Host "⚠️  Profile already exists" -ForegroundColor Yellow
    Write-Host ""
    $response = Read-Host "Overwrite existing profile? (y/N)"
    
    if ($response -notmatch '^[Yy]') {
        Write-Host ""
        Write-Host "Installation cancelled" -ForegroundColor Yellow
        exit 0
    }
}

# Install profile
try {
    Copy-Item $sourcePath $profilePath -Force
    Write-Host "✅ Barebone profile installed successfully!" -ForegroundColor Green
    Write-Host ""
} catch {
    Write-Host "❌ ERROR: Failed to install profile: $_" -ForegroundColor Red
    exit 1
}

# Display profile content preview
Write-Host "Profile Preview:" -ForegroundColor Cyan
Write-Host "----------------------------------------" -ForegroundColor Gray
Get-Content $profilePath | Select-Object -First 20 | ForEach-Object {
    Write-Host $_ -ForegroundColor DarkGray
}
Write-Host "..." -ForegroundColor DarkGray
Write-Host "----------------------------------------" -ForegroundColor Gray
Write-Host ""

# Show available commands
Write-Host "Available Commands:" -ForegroundColor Cyan
Write-Host "  Navigation:" -ForegroundColor Yellow
Write-Host "    dt               - Go to DarkThumbs root" -ForegroundColor Gray
Write-Host "    bs               - Go to build-scripts" -ForegroundColor Gray
Write-Host "    src              - Go to CBXShell source" -ForegroundColor Gray
Write-Host ""
Write-Host "  Building:" -ForegroundColor Yellow
Write-Host "    Build-Quick      - Quick build without libraries" -ForegroundColor Gray
Write-Host "    Build-Quick -Clean - Clean build" -ForegroundColor Gray
Write-Host ""
Write-Host "  Monitoring:" -ForegroundColor Yellow
Write-Host "    Build-Monitor    - Tail latest build log" -ForegroundColor Gray
Write-Host "    Build-Status     - Show recent build results" -ForegroundColor Gray
Write-Host "    Start-BuildMonitor - Live file-based monitoring" -ForegroundColor Gray
Write-Host ""

# Test profile
Write-Host "Testing profile..." -ForegroundColor Cyan
try {
    $testResult = & pwsh -NoLogo -Command {
        . $args[0]
        if (Get-Command dt -ErrorAction SilentlyContinue) {
            return "OK"
        } else {
            return "FAIL"
        }
    } -args $profilePath
    
    if ($testResult -eq "OK") {
        Write-Host "✅ Profile test passed" -ForegroundColor Green
    } else {
        Write-Host "⚠️  Profile test failed - commands may not work" -ForegroundColor Yellow
    }
} catch {
    Write-Host "⚠️  Could not test profile: $_" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Installation Complete!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Yellow
Write-Host "  1. Restart PowerShell to load the new profile" -ForegroundColor White
Write-Host "  2. Or run: . `$PROFILE" -ForegroundColor White
Write-Host "  3. Try commands: dt, Build-Quick, Build-Status" -ForegroundColor White
Write-Host ""
Write-Host "For VSCode integration, add to settings.json:" -ForegroundColor Yellow
Write-Host '  "terminal.integrated.profiles.windows": {' -ForegroundColor Gray
Write-Host '    "PowerShell": {' -ForegroundColor Gray
Write-Host '      "args": ["-NoProfile", "-NoLogo"]' -ForegroundColor Gray
Write-Host '    }' -ForegroundColor Gray
Write-Host '  }' -ForegroundColor Gray
Write-Host ""
Write-Host "Documentation: docs\BUILD_MONITORING.md" -ForegroundColor Cyan
Write-Host ""
