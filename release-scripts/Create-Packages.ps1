# =============================================================================
# Create-Packages.ps1 - Multi-Format Package Builder for DarkThumbs
# =============================================================================
# Sprint 18 - Task 18.3: Multi-Format Packaging
# Supports: MSIX, MSI, Portable ZIP
# =============================================================================

param(
    [Parameter(Mandatory = $false)]
    [ValidateSet("MSIX", "MSI", "Portable", "All")]
    [string]$PackageType = "All",
    
    [Parameter(Mandatory = $false)]
    [string]$Version = "6.0.0",
    
    [Parameter(Mandatory = $false)]
    [string]$OutputPath = "..\release-packages",
    
    [switch]$Sign,
    
    [string]$CertificateThumbprint
)

$ErrorActionPreference = "Stop"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  DarkThumbs Package Builder v1.0" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Version: $Version" -ForegroundColor White
Write-Host "Package Type: $PackageType" -ForegroundColor White
Write-Host "Output Path: $OutputPath" -ForegroundColor White
Write-Host ""

# Configuration
$projectRoot = Split-Path -Parent $PSScriptRoot
$binariesPath = Join-Path $projectRoot "x64\Release"
$packagingPath = Join-Path $projectRoot "packaging"
$outputFullPath = Join-Path $projectRoot $OutputPath

# Create output directory
if (-not (Test-Path $outputFullPath)) {
    New-Item -ItemType Directory -Path $outputFullPath -Force | Out-Null
}

$timestamp = Get-Date -Format "yyyy-MM-dd_HHmmss"

# Helper: Create Portable ZIP Package
function Create-PortablePackage {
    Write-Host "[PORTABLE] Creating ZIP package..." -ForegroundColor Yellow
    
    $portablePath = Join-Path $outputFullPath "DarkThumbs_v${Version}_Portable"
    $zipPath = "$portablePath.zip"
    
    # Create staging directory
    if (Test-Path $portablePath) {
        Remove-Item -Path $portablePath -Recurse -Force
    }
    New-Item -ItemType Directory -Path $portablePath -Force | Out-Null
    
    # Copy binaries
    Write-Host "  - Copying binaries..." -ForegroundColor Gray
    Copy-Item -Path (Join-Path $binariesPath "CBXShell.dll") -Destination $portablePath
    Copy-Item -Path (Join-Path $binariesPath "CBXManager.exe") -Destination $portablePath
    
    # Copy documentation
    Write-Host "  - Copying documentation..." -ForegroundColor Gray
    $docsToInclude = @(
        "README.md",
        "LICENSE",
        "CHANGELOG.md"
    )
    
    foreach ($doc in $docsToInclude) {
        $srcPath = Join-Path $projectRoot $doc
        if (Test-Path $srcPath) {
            Copy-Item -Path $srcPath -Destination $portablePath
        }
    }
    
    # Create install script
    Write-Host "  - Creating install script..." -ForegroundColor Gray
    $installScript = @"
@echo off
echo ========================================
echo   DarkThumbs v$Version - Installation
echo ========================================
echo.

echo Registering CBXShell.dll...
regsvr32 /s CBXShell.dll

if %ERRORLEVEL% EQU 0 (
    echo   SUCCESS: DarkThumbs installed successfully!
    echo.
    echo You can now launch CBXManager.exe to configure.
) else (
    echo   ERROR: Installation failed!
    echo   Make sure you run this as Administrator.
)

echo.
pause
"@
    $installScript | Out-File -FilePath (Join-Path $portablePath "Install.cmd") -Encoding ASCII
    
    # Create uninstall script
    $uninstallScript = @"
@echo off
echo ========================================
echo   DarkThumbs v$Version - Uninstallation
echo ========================================
echo.

echo Unregistering CBXShell.dll...
regsvr32 /s /u CBXShell.dll

if %ERRORLEVEL% EQU 0 (
    echo   SUCCESS: DarkThumbs uninstalled successfully!
) else (
    echo   ERROR: Uninstallation failed!
)

echo.
pause
"@
    $uninstallScript | Out-File -FilePath (Join-Path $portablePath "Uninstall.cmd") -Encoding ASCII
    
    # Create README
    $readmeContent = @"
# DarkThumbs v$Version - Portable Edition

## Installation

1. **Run as Administrator:** Right-click "Install.cmd" and select "Run as administrator"
2. **Configure:** Launch "CBXManager.exe" to configure supported formats
3. **Enjoy:** Thumbnails will now appear for supported file types in Windows Explorer

## Uninstallation

1. **Run as Administrator:** Right-click "Uninstall.cmd" and select "Run as administrator"
2. **Done:** DarkThumbs will be removed from your system

## Requirements

- Windows 10 version 2004 or later
- x64 architecture
- Administrator rights for installation

## Support

- Documentation: docs/
- Issues: https://github.com/darkthumbs/darkthumbs/issues

## License

MIT License - See LICENSE file for details
"@
    $readmeContent | Out-File -FilePath (Join-Path $portablePath "README.txt") -Encoding UTF8
    
    # Create ZIP
    Write-Host "  - Creating ZIP archive..." -ForegroundColor Gray
    if (Test-Path $zipPath) {
        Remove-Item -Path $zipPath -Force
    }
    
    Compress-Archive -Path "$portablePath\*" -DestinationPath $zipPath -CompressionLevel Optimal
    
    # Cleanup staging
    Remove-Item -Path $portablePath -Recurse -Force
    
    Write-Host "  ✅ Portable package created: $(Split-Path -Leaf $zipPath)" -ForegroundColor Green
    Write-Host "     Size: $([math]::Round((Get-Item $zipPath).Length / 1MB, 2)) MB" -ForegroundColor Gray
    Write-Host ""
    
    return $zipPath
}

# Helper: Create MSIX Package
function Create-MSIXPackage {
    Write-Host "[MSIX] Creating MSIX package..." -ForegroundColor Yellow
    
    # Check for makeappx.exe
    $makeappx = $null
    $possiblePaths = @(
        "C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\makeappx.exe",
        "C:\Program Files (x86)\Windows Kits\10\bin\x64\makeappx.exe"
    )
    
    foreach ($path in $possiblePaths) {
        if (Test-Path $path) {
            $makeappx = $path
            break
        }
    }
    
    if (-not $makeappx) {
        Write-Host "  ⚠️  makeappx.exe not found - MSIX packaging skipped" -ForegroundColor Yellow
        Write-Host "     Install Windows SDK to enable MSIX packaging" -ForegroundColor Gray
        return $null
    }
    
    Write-Host "  - Found makeappx: $makeappx" -ForegroundColor Gray
    
    $msixStagingPath = Join-Path $outputFullPath "msix-staging"
    $msixPath = Join-Path $outputFullPath "DarkThumbs_v${Version}_x64.msix"
    
    # Create staging directory
    if (Test-Path $msixStagingPath) {
        Remove-Item -Path $msixStagingPath -Recurse -Force
    }
    New-Item -ItemType Directory -Path $msixStagingPath -Force | Out-Null
    
    # Copy manifest
    Write-Host "  - Copying MSIX manifest..." -ForegroundColor Gray
    $manifestPath = Join-Path $packagingPath "msix\AppxManifest.xml"
    if (-not (Test-Path $manifestPath)) {
        Write-Host "  ❌ AppxManifest.xml not found at: $manifestPath" -ForegroundColor Red
        return $null
    }
    Copy-Item -Path $manifestPath -Destination $msixStagingPath
    
    # Copy binaries
    Write-Host "  - Copying binaries..." -ForegroundColor Gray
    Copy-Item -Path (Join-Path $binariesPath "CBXShell.dll") -Destination $msixStagingPath
    Copy-Item -Path (Join-Path $binariesPath "CBXManager.exe") -Destination $msixStagingPath
    
    # Create Assets folder (placeholder icons)
    $assetsPath = Join-Path $msixStagingPath "Assets"
    New-Item -ItemType Directory -Path $assetsPath -Force | Out-Null
    
    # Package MSIX
    Write-Host "  - Creating MSIX package..." -ForegroundColor Gray
    $packArgs = @("pack", "/d", $msixStagingPath, "/p", $msixPath, "/o")
    & $makeappx @packArgs 2>&1 | Out-Null
    
    if (Test-Path $msixPath) {
        Write-Host "  ✅ MSIX package created: $(Split-Path -Leaf $msixPath)" -ForegroundColor Green
        Write-Host "     Size: $([math]::Round((Get-Item $msixPath).Length / 1MB, 2)) MB" -ForegroundColor Gray
    } else {
        Write-Host "  ❌ MSIX packaging failed" -ForegroundColor Red
        return $null
    }
    
    # Cleanup staging
    Remove-Item -Path $msixStagingPath -Recurse -Force
    
    Write-Host ""
    return $msixPath
}

# Helper: Create MSI Package
function Create-MSIPackage {
    Write-Host "[MSI] Creating MSI package..." -ForegroundColor Yellow
    Write-Host "  ℹ️  MSI packaging requires WiX Toolset" -ForegroundColor Cyan
    Write-Host "     Download from: https://wixtoolset.org/" -ForegroundColor Gray
    Write-Host ""
    Write-Host "  ⚠️  MSI packaging not yet implemented" -ForegroundColor Yellow
    Write-Host "     Use Portable or MSIX packages for now" -ForegroundColor Gray
    Write-Host ""
    
    # TODO: Implement WiX-based MSI packaging
    return $null
}

# Main packaging logic
Write-Host "[START] Beginning package creation..." -ForegroundColor Cyan
Write-Host ""

$createdPackages = @()

if ($PackageType -eq "Portable" -or $PackageType -eq "All") {
    $pkg = Create-PortablePackage
    if ($pkg) { $createdPackages += $pkg }
}

if ($PackageType -eq "MSIX" -or $PackageType -eq "All") {
    $pkg = Create-MSIXPackage
    if ($pkg) { $createdPackages += $pkg }
}

if ($PackageType -eq "MSI" -or $PackageType -eq "All") {
    $pkg = Create-MSIPackage
    if ($pkg) { $createdPackages += $pkg }
}

# Sign packages if requested
if ($Sign -and $createdPackages.Count -gt 0) {
    Write-Host ""
    Write-Host "[SIGNING] Signing packages..." -ForegroundColor Yellow
    
    if (-not $CertificateThumbprint) {
        Write-Host "  ⚠️  No certificate thumbprint provided - skipping signing" -ForegroundColor Yellow
        Write-Host "     Use -CertificateThumbprint parameter to enable signing" -ForegroundColor Gray
    } else {
        # TODO: Implement package signing
        Write-Host "  ℹ️  Package signing not yet implemented" -ForegroundColor Cyan
    }
    Write-Host ""
}

# Summary
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Package Creation Summary" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Packages created: $($createdPackages.Count)" -ForegroundColor White
Write-Host "Output directory: $outputFullPath" -ForegroundColor Gray
Write-Host ""

foreach ($pkg in $createdPackages) {
    $pkgName = Split-Path -Leaf $pkg
    $pkgSize = [math]::Round((Get-Item $pkg).Length / 1MB, 2)
    Write-Host "  ✅ $pkgName ($pkgSize MB)" -ForegroundColor Green
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "  ✅ Packaging Complete" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
