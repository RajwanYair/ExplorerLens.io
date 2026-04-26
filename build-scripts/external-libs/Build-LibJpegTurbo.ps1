#Requires -Version 7.2
# build-scripts/external-libs/Build-LibJpegTurbo.ps1
# Sprint S303 — libjpeg-turbo 3.0 SIMD-accelerated JPEG decoder build script
# ROADMAP v7.0 §19 "ADD" table / H28 (WIC codec integration) / Phase 2 stb_image replacement
#
# PURPOSE:
#   Builds libjpeg-turbo 3.0 from source with NASM for SIMD acceleration.
#   Used when vcpkg is unavailable or a custom build configuration is required.
#   In normal CI, libjpeg-turbo is pulled via vcpkg manifest mode (vcpkg.json).
#
# USAGE:
#   .\Build-LibJpegTurbo.ps1                         # Build to external/image-libs/libjpeg-turbo/
#   .\Build-LibJpegTurbo.ps1 -Version "3.0.4"        # Pin a specific version
#   .\Build-LibJpegTurbo.ps1 -Clean                  # Clean and rebuild
#   .\Build-LibJpegTurbo.ps1 -Configuration Debug    # Debug build
#
# PREREQUISITES:
#   - MSVC v145 (vcvars64 must be sourced or called via Build-MSVC.ps1)
#   - NASM >= 2.15 on PATH (for SIMD acceleration)
#   - CMake >= 3.25
#   - Ninja
#
# OUTPUTS:
#   external/image-libs/libjpeg-turbo/include/   — public headers (turbojpeg.h, jpeglib.h)
#   external/image-libs/libjpeg-turbo/lib/       — jpeg-static.lib, turbojpeg-static.lib
#
# REPLACES: stb_image JPEG decode path (see StbImageAuditContract.h, S302)
# TARGET SPRINT: S318 will wire these libs into the Engine JPEG decoder path

param(
    [string]$Version       = "3.0.4",
    [string]$Configuration = "Release",
    [switch]$Clean,
    [string]$InstallDir    = "$PSScriptRoot\..\..\external\image-libs\libjpeg-turbo"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

# Import shared build utilities
. "$PSScriptRoot\..\core\Build-Library-Core.ps1"

$libName    = "libjpeg-turbo"
$tarballUrl = "https://github.com/libjpeg-turbo/libjpeg-turbo/archive/refs/tags/${Version}.zip"
$srcDir     = "$PSScriptRoot\..\..\external\image-libs\libjpeg-turbo-${Version}"
$buildDir   = "$srcDir\build-msvc"

Write-ELHeader "$libName $Version" $Configuration

if ($Clean -and (Test-Path $srcDir)) {
    Write-ELStep "Cleaning previous build: $srcDir"
    Remove-Item $srcDir -Recurse -Force
}

if (-not (Test-Path $srcDir)) {
    Write-ELStep "Downloading $libName $Version..."
    $zip = "$env:TEMP\libjpeg-turbo-${Version}.zip"
    Invoke-ELDownload $tarballUrl $zip
    Expand-Archive $zip "$PSScriptRoot\..\..\external\image-libs\" -Force
    Remove-Item $zip
}

# Verify NASM is available for SIMD
$nasmPath = Get-Command nasm.exe -ErrorAction SilentlyContinue
if (-not $nasmPath) {
    Write-ELWarning "NASM not found on PATH — libjpeg-turbo will build WITHOUT SIMD acceleration."
    Write-ELWarning "Install NASM >= 2.15 for optimal decode performance."
    $nasmFlag = "-DWITH_SIMD=OFF"
} else {
    Write-ELStep "NASM found: $($nasmPath.Source) — SIMD enabled"
    $nasmFlag = "-DWITH_SIMD=ON"
}

Write-ELStep "Configuring with CMake..."
$cmakeArgs = @(
    "-S", $srcDir,
    "-B", $buildDir,
    "-G", "Ninja",
    "-DCMAKE_BUILD_TYPE=$Configuration",
    "-DCMAKE_INSTALL_PREFIX=$InstallDir",
    "-DENABLE_SHARED=OFF",
    "-DENABLE_STATIC=ON",
    "-DWITH_JPEG8=ON",        # libjpeg API compatibility
    "-DWITH_TURBOJPEG=ON",    # TurboJPEG high-level API
    $nasmFlag
)
& cmake @cmakeArgs
if ($LASTEXITCODE -ne 0) { throw "CMake configure failed (exit $LASTEXITCODE)" }

Write-ELStep "Building $libName..."
& cmake --build $buildDir --config $Configuration --parallel
if ($LASTEXITCODE -ne 0) { throw "Build failed (exit $LASTEXITCODE)" }

Write-ELStep "Installing to $InstallDir..."
& cmake --install $buildDir --config $Configuration
if ($LASTEXITCODE -ne 0) { throw "Install failed (exit $LASTEXITCODE)" }

# Verify outputs
$requiredHeaders = @("turbojpeg.h", "jpeglib.h")
foreach ($h in $requiredHeaders) {
    $p = Join-Path $InstallDir "include\$h"
    if (-not (Test-Path $p)) {
        throw "Expected header not found: $p"
    }
}

Write-ELSuccess "$libName $Version built successfully"
Write-ELInfo "Headers: $InstallDir\include\"
Write-ELInfo "Libs:    $InstallDir\lib\"
Write-ELInfo "Next:    S318 will wire turbojpeg.h into Engine/Decoders/JpegTurboDecoderV2.h"
