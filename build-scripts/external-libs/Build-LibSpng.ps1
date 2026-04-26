#Requires -Version 7.2
# build-scripts/external-libs/Build-LibSpng.ps1
# Sprint S304 — libspng 0.7 PNG decoder build script
# ROADMAP v7.0 §19 "ADD" table / H28 (WIC codec integration) / Phase 2 stb_image replacement
#
# PURPOSE:
#   Builds libspng 0.7 from source — a modern, single-file, ASAN-clean PNG decoder.
#   libspng is faster than libpng for streaming decode and has no LGPL dependency concerns.
#   Used when vcpkg is unavailable or a custom build configuration is required.
#   In normal CI, libpng is pulled via vcpkg manifest mode; libspng supplements it.
#
# USAGE:
#   .\Build-LibSpng.ps1                          # Build to external/image-libs/libspng/
#   .\Build-LibSpng.ps1 -Version "0.7.4"         # Pin a specific version
#   .\Build-LibSpng.ps1 -Clean                   # Clean and rebuild
#
# PREREQUISITES:
#   - MSVC v145 (vcvars64 must be sourced or called via Build-MSVC.ps1)
#   - zlib already built (Build-Zlib.ps1 or vcpkg)
#   - CMake >= 3.25
#   - Ninja
#
# OUTPUTS:
#   external/image-libs/libspng/include/spng.h   — single public header
#   external/image-libs/libspng/lib/spng.lib     — static library
#
# REPLACES: stb_image PNG decode path (see StbImageAuditContract.h, S302)
# TARGET SPRINT: S319 will wire libspng into the Engine PNG decoder path

param(
    [string]$Version       = "0.7.4",
    [string]$Configuration = "Release",
    [switch]$Clean,
    [string]$InstallDir    = "$PSScriptRoot\..\..\external\image-libs\libspng"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

# Import shared build utilities
. "$PSScriptRoot\..\core\Build-Library-Core.ps1"

$libName    = "libspng"
$tarballUrl = "https://github.com/randy408/libspng/archive/refs/tags/v${Version}.zip"
$srcDir     = "$PSScriptRoot\..\..\external\image-libs\libspng-${Version}"
$buildDir   = "$srcDir\build-msvc"

Write-ELHeader "$libName $Version" $Configuration

if ($Clean -and (Test-Path $srcDir)) {
    Write-ELStep "Cleaning previous build: $srcDir"
    Remove-Item $srcDir -Recurse -Force
}

if (-not (Test-Path $srcDir)) {
    Write-ELStep "Downloading $libName $Version..."
    $zip = "$env:TEMP\libspng-${Version}.zip"
    Invoke-ELDownload $tarballUrl $zip
    Expand-Archive $zip "$PSScriptRoot\..\..\external\image-libs\" -Force
    Remove-Item $zip
}

# Locate zlib (prefer vcpkg, then our own build)
$zlibHint = ""
$vcpkgRoot = Join-Path $PSScriptRoot "..\..\packages\vcpkg"
if (Test-Path $vcpkgRoot) {
    $zlibHint = "-DZLIB_ROOT=$vcpkgRoot\installed\x64-windows-static"
}

Write-ELStep "Configuring with CMake..."
$cmakeArgs = @(
    "-S", $srcDir,
    "-B", $buildDir,
    "-G", "Ninja",
    "-DCMAKE_BUILD_TYPE=$Configuration",
    "-DCMAKE_INSTALL_PREFIX=$InstallDir",
    "-DSPNG_STATIC=ON",
    "-DSPNG_SHARED=OFF",
    "-DSPNG_USE_MINIZ=OFF"   # Use system zlib, not miniz
)
if ($zlibHint) { $cmakeArgs += $zlibHint }
& cmake @cmakeArgs
if ($LASTEXITCODE -ne 0) { throw "CMake configure failed (exit $LASTEXITCODE)" }

Write-ELStep "Building $libName..."
& cmake --build $buildDir --config $Configuration --parallel
if ($LASTEXITCODE -ne 0) { throw "Build failed (exit $LASTEXITCODE)" }

Write-ELStep "Installing to $InstallDir..."
& cmake --install $buildDir --config $Configuration
if ($LASTEXITCODE -ne 0) { throw "Install failed (exit $LASTEXITCODE)" }

# Verify output
$headerPath = Join-Path $InstallDir "include\spng.h"
if (-not (Test-Path $headerPath)) {
    throw "Expected header not found: $headerPath"
}

Write-ELSuccess "$libName $Version built successfully"
Write-ELInfo "Header: $InstallDir\include\spng.h"
Write-ELInfo "Lib:    $InstallDir\lib\"
Write-ELInfo "Next:   S319 will wire spng.h into Engine/Decoders/SpngPngDecoder.h"
