#Requires -Version 7.0
# ExplorerLens v7.0 - vcpkg Setup Script
# Detects, installs, and configures vcpkg for ExplorerLens project
#
# USAGE:
#   .\Setup-Vcpkg.ps1                  # Detect and install if needed
#   .\Setup-Vcpkg.ps1 -Force           # Force reinstall
#   .\Setup-Vcpkg.ps1 -InstallPackages # Also install required packages
#
# Date: February 16, 2026

param(
    [switch]$Force,
    [switch]$InstallPackages,
    [string]$InstallPath = "$env:USERPROFILE\vcpkg"
)

# Import helper module
$helperModule = Join-Path $PSScriptRoot "core\Build-Helpers.ps1"
if (-not (Test-Path $helperModule)) {
    Write-Host "✗ Build-Helpers.ps1 not found at: $helperModule" -ForegroundColor Red
    exit 1
}

. $helperModule

# ============================================================================
# Main Script
# ============================================================================

Write-Host ""
Write-Host ("=" * 80) -ForegroundColor Cyan
Write-Host "  ExplorerLens v7.0 - vcpkg Setup" -ForegroundColor Cyan
Write-Host ("=" * 80) -ForegroundColor Cyan
Write-Host ""

# Check if vcpkg is already installed
$vcpkgInstalled = Test-VcpkgInstalled

if ($vcpkgInstalled -and -not $Force) {
    Write-Host "✓ vcpkg is already installed and configured" -ForegroundColor Green
    $vcpkgPath = Get-VcpkgPath
    Write-Host "  Location: $vcpkgPath" -ForegroundColor Gray
    
    if (-not $InstallPackages) {
        Write-Host ""
        Write-Host "Use -InstallPackages to install ExplorerLens dependencies" -ForegroundColor Yellow
        exit 0
    }
} else {
    # Install vcpkg
    try {
        Write-Host "Installing vcpkg..." -ForegroundColor Cyan
        $vcpkgPath = Install-VcpkgIfNeeded -InstallPath $InstallPath -Force:$Force
        
        Write-Host ""
        Write-Host "✓ vcpkg setup complete" -ForegroundColor Green
    } catch {
        Write-Host ""
        Write-Host "✗ vcpkg setup failed: $($_.Exception.Message)" -ForegroundColor Red
        exit 1
    }
}

# Optional: Install required packages
if ($InstallPackages) {
    Write-Host ""
    Write-Host ("=" * 80) -ForegroundColor Cyan
    Write-Host "  Installing ExplorerLens Dependencies" -ForegroundColor Cyan
    Write-Host ("=" * 80) -ForegroundColor Cyan
    Write-Host ""
    
    # Define packages needed for ExplorerLens
    # Note: ExplorerLens currently builds most dependencies from source,
    # but vcpkg can be used for future dependency management
    $requiredPackages = @(
        # Core compression libraries (optional - we build from source)
        # "zlib:x64-windows-static",
        # "zstd:x64-windows-static",
        # "lz4:x64-windows-static",
        # "liblzma:x64-windows-static",
        
        # Image libraries (optional - we build from source)
        # "libwebp:x64-windows-static",
        # "libavif:x64-windows-static",
        # "libjxl:x64-windows-static",
        
        # Build tools
        "cmake:x64-windows"
    )
    
    Write-Host "The following packages can be installed via vcpkg:" -ForegroundColor Cyan
    foreach ($pkg in $requiredPackages) {
        Write-Host "  - $pkg" -ForegroundColor Gray
    }
    
    Write-Host ""
    Write-Host "Note: ExplorerLens currently builds most libraries from source." -ForegroundColor Yellow
    Write-Host "vcpkg is available for future dependency management." -ForegroundColor Yellow
    Write-Host ""
    
    $response = Read-Host "Install packages? (y/N)"
    
    if ($response -eq 'y' -or $response -eq 'Y') {
        foreach ($pkg in $requiredPackages) {
            try {
                Write-Host ""
                Install-VcpkgPackage -PackageName $pkg
            } catch {
                Write-Host "✗ Failed to install $pkg : $($_.Exception.Message)" -ForegroundColor Red
            }
        }
        
        Write-Host ""
        Write-Host "✓ Package installation complete" -ForegroundColor Green
    } else {
        Write-Host "Skipping package installation" -ForegroundColor Yellow
    }
}

# Display vcpkg information
Write-Host ""
Write-Host ("=" * 80) -ForegroundColor Cyan
Write-Host "  vcpkg Configuration" -ForegroundColor Cyan
Write-Host ("=" * 80) -ForegroundColor Cyan
Write-Host ""

$vcpkgPath = Get-VcpkgPath
Write-Host "vcpkg Location: $vcpkgPath" -ForegroundColor Cyan
Write-Host "vcpkg Triplet:  x64-windows-static (recommended)" -ForegroundColor Cyan
Write-Host ""

# Check for CMake integration
$vcpkgCMake = Join-Path $vcpkgPath "scripts\buildsystems\vcpkg.cmake"
if (Test-Path $vcpkgCMake) {
    Write-Host "CMake Integration: Use -DCMAKE_TOOLCHAIN_FILE=$vcpkgCMake" -ForegroundColor Green
}

Write-Host ""
Write-Host "For more information, visit: https://vcpkg.io" -ForegroundColor Gray
Write-Host ""

exit 0

