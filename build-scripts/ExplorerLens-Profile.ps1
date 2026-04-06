# ExplorerLens v7.0.0 - PowerShell Profile Configuration
# Auto-configures VCPKG_ROOT for optimal build performance
#
# Installation:
#   1. Copy this file to: $PROFILE (e.g., C:\Users\YourName\Documents\PowerShell\Microsoft.PowerShell_profile.ps1)
#   2. Or append content to existing profile
#   3. Restart PowerShell
#
# Usage:
#   New PowerShell sessions will automatically have VCPKG_ROOT set

# ============================================================================
# ExplorerLens Build Environment Configuration
# ============================================================================

Write-Host "Initializing ExplorerLens build environment..." -ForegroundColor Cyan

# Set VCPKG_ROOT to Visual Studio installation
$vcpkgPath = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\vcpkg"
if (Test-Path "$vcpkgPath\vcpkg.exe") {
    $env:VCPKG_ROOT = $vcpkgPath
    Write-Host "✓ VCPKG_ROOT set to: $env:VCPKG_ROOT" -ForegroundColor Green

    # Add to PATH if not already there
    if ($env:PATH -notlike "*$vcpkgPath*") {
        $env:PATH = "$vcpkgPath;$env:PATH"
        Write-Host "✓ vcpkg added to PATH" -ForegroundColor Green
    }
} else {
    Write-Host "⚠ vcpkg not found at expected location" -ForegroundColor Yellow
    Write-Host "  Expected: $vcpkgPath\vcpkg.exe" -ForegroundColor Gray
}

# Verify Visual Studio Build Tools
$msbuildPath = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\MSBuild\Current\Bin\amd64\MSBuild.exe"
if (Test-Path $msbuildPath) {
    Write-Host "✓ Visual Studio Build Tools 2022 (v18) detected" -ForegroundColor Green
} else {
    Write-Host "⚠ Visual Studio Build Tools not found at expected location" -ForegroundColor Yellow
}

# Set Visual Studio environment variables
$vsInstallPath = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools"
if (Test-Path $vsInstallPath) {
    $env:VSINSTALLDIR = $vsInstallPath
    Write-Host "✓ VSINSTALLDIR set to: $env:VSINSTALLDIR" -ForegroundColor Green
}

Write-Host "ExplorerLens build environment ready!" -ForegroundColor Green
Write-Host ""

# ============================================================================
# Optional: ExplorerLens Helper Aliases
# ============================================================================

# Quick navigation to ExplorerLens project (customize this path)
$ExplorerLensPath = "$PSScriptRoot\.."  # Resolved from script location
if (Test-Path $ExplorerLensPath) {
    function dt { Set-Location $ExplorerLensPath }
    Write-Host "Alias 'dt' created for ExplorerLens directory" -ForegroundColor Gray
}

# Quick build commands
function Build-ExplorerLens {
    param([string]$Config = "Release")
    & "$ExplorerLensPath\scripts\build.ps1" -Configuration $Config
}

function Build-ExplorerLensEngine {
    param([string]$Config = "Release")
    cmake --build "$ExplorerLensPath\build" --config $Config --target ExplorerLensEngine -j 8
}

function Test-ExplorerLens {
    ctest --test-dir "$ExplorerLensPath\build" -C Release --output-on-failure
}

Set-Alias build Build-ExplorerLens
Set-Alias build-engine Build-ExplorerLensEngine
Set-Alias test Test-ExplorerLens

Write-Host "Build aliases: build, build-engine, test" -ForegroundColor Gray
Write-Host ""

# ============================================================================
# Performance Optimizations
# ============================================================================

# Increase PowerShell command history
$MaximumHistoryCount = 10000

# Enable tab completion for parameters
Set-PSReadLineKeyHandler -Key Tab -Function MenuComplete

# Enable predictive IntelliSense (PowerShell 7.2+)
if ($PSVersionTable.PSVersion.Major -ge 7) {
    Set-PSReadLineOption -PredictionSource History
    Set-PSReadLineOption -PredictionViewStyle ListView
}

Write-Host "Profile loaded successfully!" -ForegroundColor Cyan
Write-Host "Type 'dt' to navigate to ExplorerLens" -ForegroundColor Gray
Write-Host ""
