#Requires -Version 7.0
# DarkThumbs v7.0 - Build dav1d 1.5.1 (AV1 Video Decoder)
# Refactored to use Build-Library-Core.ps1 module
# Date: February 16, 2026

param(
    [string]$Configuration = "Release",
    [switch]$Clean
)

# Import core build module
. "$PSScriptRoot\..\core\Build-Library-Core.ps1"
# Import helper module for Meson environment setup
. "$PSScriptRoot\..\core\Build-Helpers.ps1"

$rootDir = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$dav1dDir = Join-Path $rootDir "external\image-libs\dav1d-1.5.1"
$buildDir = Join-Path $dav1dDir "build-msvc"
$installDir = Join-Path $dav1dDir "install"

Write-BuildHeader "Building dav1d 1.5.1 (AV1 Video Decoder)"

# Verify source directory
if (-not (Test-Path $dav1dDir)) {
    Write-BuildLog "dav1d-1.5.1 not found at $dav1dDir" -Level Error
    Write-BuildLog "Please run download-all-libs.ps1 first" -Level Warning
    exit 1
}

Write-BuildLog "Source: $dav1dDir" -Level Info
Write-BuildLog "Build: $buildDir" -Level Info
Write-BuildLog "Install: $installDir" -Level Info

try {
    # Check for Meson and Ninja
    if (-not (Test-CommandExists "meson")) {
        Write-BuildLog "Meson not found, installing via pip..." -Level Warning
        python -m pip install meson --quiet
    }
    
    if (-not (Test-CommandExists "ninja")) {
        Write-BuildLog "Ninja not found, installing via pip..." -Level Warning
        python -m pip install ninja --quiet
    }
    
    # Clean build directory if requested
    if ($Clean -and (Test-Path $buildDir)) {
        Write-BuildLog "Cleaning previous build" -Level Info
        Remove-Item $buildDir -Recurse -Force
    }
    
    # Configure with Meson
    if (-not (Test-Path $buildDir)) {
        Write-BuildLog "Configuring dav1d with Meson..." -Level Info
        
        Push-Location $dav1dDir
        try {
            & meson setup $buildDir `
                --buildtype=release `
                --default-library=static `
                --vsenv `
                --prefix=$installDir `
                -Denable_asm=true `
                -Denable_tools=false `
                -Denable_examples=false `
                -Denable_tests=false
            
            if ($LASTEXITCODE -ne 0) {
                throw "Meson configuration failed"
            }
        }
        finally {
            Pop-Location
        }
    }
    
    # Build with Ninja
    Write-BuildLog "Building dav1d..." -Level Info
    Push-Location $buildDir
    try {
        & ninja
        if ($LASTEXITCODE -ne 0) {
            throw "Ninja build failed"
        }
        
        # Install
        & ninja install
        if ($LASTEXITCODE -ne 0) {
            throw "Installation failed"
        }
    }
    finally {
        Pop-Location
    }
    
    # Verify output
    $expectedLib = Join-Path $installDir "lib\dav1d.lib"
    Test-BuildOutput -Files @($expectedLib) -ThrowOnMissing
    
    Write-BuildLog "dav1d 1.5.1 build completed successfully" -Level Success
    Write-BuildLog "Features: AV1 video decoder, optimized assembly" -Level Info
    
}
catch {
    Write-BuildLog "Build failed: $($_.Exception.Message)" -Level Error
    exit 1
}
