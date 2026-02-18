#Requires -Version 7.0
# DarkThumbs v7.0 - Build libavif 1.3.0 (AV1 Image Format)
# Refactored to use Build-Library-Core.ps1 module
# Date: February 18, 2026
#
# Directory structure (post-cleanup):
#   Project root:       <repo>\
#   This script:        <repo>\build-scripts\external-libs\Build-LibAVIF.ps1
#   Core module:        <repo>\build-scripts\core\Build-Library-Core.ps1
#   libavif source:     <repo>\external\image-libs\libavif-1.3.0\
#   dav1d install:      <repo>\external\image-libs\dav1d-1.5.1\install\
#   Build dir:          <repo>\external\image-libs\libavif-1.3.0\build-msvc\
#   Install dir:        <repo>\external\image-libs\libavif-1.3.0\install\

param(
    [string]$Configuration = "Release",
    [switch]$Clean
)

# Import core build module
. "$PSScriptRoot\..\core\Build-Library-Core.ps1"

$rootDir = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$sourceDir = Join-Path $rootDir "external\image-libs\libavif-1.3.0"
$dav1dDir = Join-Path $rootDir "external\image-libs\dav1d-1.5.1\install"
$buildDir = Join-Path $sourceDir "build-msvc"
$installDir = Join-Path $sourceDir "install"

Write-BuildHeader "Building libavif 1.3.0 (AV1 Image Format)"

# Verify source directory
if (-not (Test-Path $sourceDir)) {
    Write-BuildLog "libavif-1.3.0 not found at $sourceDir" -Level Error
    Write-BuildLog "Please run download-all-libs.ps1 first" -Level Warning
    exit 1
}

# Verify dav1d dependency
if (-not (Test-Path (Join-Path $dav1dDir "lib\dav1d.lib"))) {
    Write-BuildLog "dav1d not built. Please run Build-Dav1d.ps1 first" -Level Error
    exit 1
}

Write-BuildLog "Source: $sourceDir" -Level Info
Write-BuildLog "Build: $buildDir" -Level Info
Write-BuildLog "Install: $installDir" -Level Info
Write-BuildLog "DAV1D: $dav1dDir" -Level Info

try {
    # CMake options for libavif
    $cmakeOptions = @{
        'CMAKE_BUILD_TYPE'                          = 'Release'
        'BUILD_SHARED_LIBS'                         = 'OFF'
        'AVIF_CODEC_DAV1D'                          = 'ON'
        'AVIF_LOCAL_DAV1D'                          = 'OFF'
        'dav1d_DIR'                                 = $dav1dDir
        'AVIF_CODEC_AOM'                            = 'OFF'
        'AVIF_BUILD_APPS'                           = 'OFF'
        'AVIF_BUILD_TESTS'                          = 'OFF'
        'AVIF_BUILD_EXAMPLES'                       = 'OFF'
        'AVIF_ENABLE_EXPERIMENTAL_GAIN_MAP'         = 'ON'
        'AVIF_ENABLE_EXPERIMENTAL_SAMPLE_TRANSFORM' = 'ON'
    }
    
    # Build with CMake
    Invoke-CMakeBuild `
        -LibraryName "libavif" `
        -SourceDir $sourceDir `
        -BuildDir $buildDir `
        -InstallDir $installDir `
        -Configuration $Configuration `
        -CMakeOptions $cmakeOptions `
        -Clean:$Clean
    
    # Verify outputs
    $expectedLib = Join-Path $installDir "lib\avif.lib"
    Test-BuildOutput -Files @($expectedLib) -ThrowOnMissing
    
    Write-BuildLog "libavif 1.3.0 build completed successfully" -Level Success
    Write-BuildLog "Features: DAV1D decoder (AV1), Gain Map API (HDR), Sample Transform" -Level Info
    
} catch {
    Write-BuildLog "Build failed: $($_.Exception.Message)" -Level Error
    exit 1
}

