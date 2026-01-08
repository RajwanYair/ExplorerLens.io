#Requires -Version 7.0

<#
.SYNOPSIS
    Build image libraries (libavif, libjxl) using CMake
    
.DESCRIPTION
    Builds modern image format libraries with proper MSVC flags for DarkThumbs
    - libavif 1.3.0 (AVIF/AV1 images)
    - libjxl 0.11.1 (JPEG XL)
    
    Uses bundled codecs and minimal dependencies for simplicity
#>

[CmdletBinding()]
param(
    [Parameter()]
    [ValidateSet("Release", "Debug")]
    [string]$Configuration = "Release",
    
    [Parameter()]
    [ValidateSet("avif", "jxl", "all")]
    [string]$Library = "all"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$RootDir = Split-Path -Parent $PSScriptRoot
$ExternalDir = Join-Path $RootDir "external\image-libs"

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "DarkThumbs Image Library Builder" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan
Write-Host "Configuration: $Configuration" -ForegroundColor Yellow
Write-Host "Libraries: $Library`n" -ForegroundColor Yellow

# Find Visual Studio
$vsGenerators = @(
    @{Name = "Visual Studio 18 2026"; Version = "18"},
    @{Name = "Visual Studio 17 2022"; Version = "17"},
    @{Name = "Visual Studio 16 2019"; Version = "16"}
)

$generator = $null
foreach ($gen in $vsGenerators) {
    $vsPath = "C:\Program Files (x86)\Microsoft Visual Studio\$($gen.Version)\BuildTools"
    if (Test-Path $vsPath) {
        $generator = $gen.Name
        Write-Host "✓ Found: $generator" -ForegroundColor Green
        break
    }
}

if (-not $generator) {
    Write-Host "✗ No Visual Studio generator found" -ForegroundColor Red
    exit 1
}

#==============================================================================
# Build libavif
#==============================================================================
function Build-LibAVIF {
    Write-Host "`n[libavif 1.3.0]" -ForegroundColor Cyan
    Write-Host "----------------------------------------" -ForegroundColor Cyan
    
    $srcDir = Join-Path $ExternalDir "libavif-1.3.0"
    $buildDir = Join-Path $srcDir "build"
    $installDir = Join-Path $srcDir "install"
    
    if (-not (Test-Path $srcDir)) {
        Write-Host "✗ Source not found: $srcDir" -ForegroundColor Red
        return $false
    }
    
    # Clean build
    if (Test-Path $buildDir) {
        Remove-Item -Recurse -Force $buildDir
    }
    New-Item -ItemType Directory -Path $buildDir | Out-Null
    
    Set-Location $buildDir
    
    Write-Host "Configuring with CMake..."
    & cmake -S $srcDir -B . `
        -G $generator `
        -A x64 `
        -DCMAKE_BUILD_TYPE=$Configuration `
        -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded" `
        -DAVIF_CODEC_AOM=ON `
        -DAVIF_LOCAL_AOM=ON `
        -DAVIF_CODEC_DAV1D=OFF `
        -DAVIF_LIBYUV=LOCAL `
        -DBUILD_SHARED_LIBS=OFF `
        -DAVIF_BUILD_APPS=OFF `
        -DAVIF_BUILD_TESTS=OFF `
        -DAVIF_BUILD_EXAMPLES=OFF `
        -DCMAKE_INSTALL_PREFIX=$installDir
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "✗ CMake configuration failed" -ForegroundColor Red
        return $false
    }
    
    Write-Host "Building..."
    & cmake --build . --config $Configuration --parallel
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "✗ Build failed" -ForegroundColor Red
        return $false
    }
    
    Write-Host "Installing..."
    & cmake --install . --config $Configuration
    
    $libPath = Join-Path $installDir "lib\avif.lib"
    if (Test-Path $libPath) {
        $size = (Get-Item $libPath).Length / 1MB
        Write-Host "✓ Built successfully: avif.lib ($([math]::Round($size, 2)) MB)" -ForegroundColor Green
        return $true
    }
    
    Write-Host "✗ Library not found after build" -ForegroundColor Red
    return $false
}

#==============================================================================
# Build libjxl
#==============================================================================
function Build-LibJXL {
    Write-Host "`n[libjxl 0.11.1]" -ForegroundColor Cyan
    Write-Host "----------------------------------------" -ForegroundColor Cyan
    
    $srcDir = Join-Path $ExternalDir "libjxl-0.11.1"
    $buildDir = Join-Path $srcDir "build"
    $installDir = Join-Path $srcDir "install"
    
    if (-not (Test-Path $srcDir)) {
        Write-Host "✗ Source not found: $srcDir" -ForegroundColor Red
        return $false
    }
    
    # Clean build
    if (Test-Path $buildDir) {
        Remove-Item -Recurse -Force $buildDir
    }
    New-Item -ItemType Directory -Path $buildDir | Out-Null
    
    Set-Location $buildDir
    
    Write-Host "Configuring with CMake..."
    & cmake -S $srcDir -B . `
        -G $generator `
        -A x64 `
        -DCMAKE_BUILD_TYPE=$Configuration `
        -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded" `
        -DBUILD_SHARED_LIBS=OFF `
        -DJPEGXL_ENABLE_BENCHMARK=OFF `
        -DJPEGXL_ENABLE_EXAMPLES=OFF `
        -DJPEGXL_ENABLE_MANPAGES=OFF `
        -DJPEGXL_ENABLE_TOOLS=OFF `
        -DJPEGXL_ENABLE_VIEWERS=OFF `
        -DJPEGXL_ENABLE_PLUGINS=OFF `
        -DJPEGXL_ENABLE_DEVTOOLS=OFF `
        -DJPEGXL_ENABLE_SJPEG=OFF `
        -DJPEGXL_ENABLE_OPENEXR=OFF `
        -DJPEGXL_FORCE_SYSTEM_BROTLI=OFF `
        -DCMAKE_INSTALL_PREFIX=$installDir
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "✗ CMake configuration failed" -ForegroundColor Red
        return $false
    }
    
    Write-Host "Building..."
    & cmake --build . --config $Configuration --parallel
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "✗ Build failed" -ForegroundColor Red
        return $false
    }
    
    Write-Host "Installing..."
    & cmake --install . --config $Configuration
    
    $libPath = Join-Path $installDir "lib\jxl.lib"
    if (Test-Path $libPath) {
        $size = (Get-Item $libPath).Length / 1MB
        Write-Host "✓ Built successfully: jxl.lib ($([math]::Round($size, 2)) MB)" -ForegroundColor Green
        return $true
    }
    
    Write-Host "✗ Library not found after build" -ForegroundColor Red
    return $false
}

#==============================================================================
# Main execution
#==============================================================================

$success = $true

switch ($Library) {
    "avif" {
        $success = Build-LibAVIF
    }
    "jxl" {
        $success = Build-LibJXL
    }
    "all" {
        $avifOk = Build-LibAVIF
        $jxlOk = Build-LibJXL
        $success = $avifOk -and $jxlOk
    }
}

Write-Host "`n========================================" -ForegroundColor Cyan
if ($success) {
    Write-Host "✓ BUILD SUCCESSFUL" -ForegroundColor Green
    Write-Host "========================================`n" -ForegroundColor Cyan
    exit 0
} else {
    Write-Host "✗ BUILD FAILED" -ForegroundColor Red
    Write-Host "========================================`n" -ForegroundColor Cyan
    exit 1
}
