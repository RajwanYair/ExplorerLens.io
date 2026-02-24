#Requires -Version 7.0
# ExplorerLens v7.0 - Build zstd 1.5.7 (Zstandard Compression)
# Refactored to use Build-Library-Core.ps1 module
# Date: February 18, 2026
#
# Directory structure (post-cleanup):
#   Project root:       <repo>\
#   This script:        <repo>\build-scripts\external-libs\Build-Zstd.ps1
#   Core module:        <repo>\build-scripts\core\Build-Library-Core.ps1
#   zstd source:        <repo>\external\compression-libs\zstd-1.5.7\
#   Build dir:          <repo>\external\compression-libs\zstd-1.5.7\build-vs\
#   Install dir:        <repo>\external\compression-libs\zstd-1.5.7\install\

param(
    [string]$Configuration = "Release",
    [switch]$Clean
)

# Import core build module
. "$PSScriptRoot\..\core\Build-Library-Core.ps1"

$rootDir = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$zstdDir = Join-Path $rootDir "external\compression-libs\zstd-1.5.7"
$buildDir = Join-Path $zstdDir "build-vs"
$installDir = Join-Path $zstdDir "install"

Write-BuildHeader "Building zstd 1.5.7 (Zstandard Compression)"

# Verify source directory
if (-not (Test-Path $zstdDir)) {
    Write-BuildLog "zstd-1.5.7 not found at $zstdDir" -Level Error
    Write-BuildLog "Please download and extract zstd source" -Level Warning
    exit 1
}

Write-BuildLog "Source: $zstdDir" -Level Info
Write-BuildLog "Build: $buildDir" -Level Info
Write-BuildLog "Install: $installDir" -Level Info

try {
    # zstd uses CMake under build/cmake directory
    $cmakeSource = Join-Path $zstdDir "build\cmake"
    
    if (-not (Test-Path (Join-Path $cmakeSource "CMakeLists.txt"))) {
        throw "CMakeLists.txt not found in $cmakeSource"
    }
    
    $cmakeOptions = @{
        'CMAKE_BUILD_TYPE'        = 'Release'
        'ZSTD_BUILD_SHARED'       = 'ON'
        'ZSTD_BUILD_STATIC'       = 'ON'
        'ZSTD_BUILD_PROGRAMS'     = 'ON'
        'ZSTD_BUILD_TESTS'        = 'ON'
        'CMAKE_C_FLAGS_RELEASE'   = '/MT /O2'
        'CMAKE_CXX_FLAGS_RELEASE' = '/MT /O2'
    }
    
    Invoke-CMakeBuild `
        -LibraryName "zstd" `
        -SourceDir $cmakeSource `
        -BuildDir $buildDir `
        -InstallDir $installDir `
        -Configuration $Configuration `
        -CMakeOptions $cmakeOptions `
        -Clean:$Clean
    
    # Verify output (zstd uses different naming)
    $expectedLib = Join-Path $installDir "lib\zstd_static.lib"
    if (-not (Test-Path $expectedLib)) {
        # Try alternate location
        $expectedLib = Join-Path $buildDir "lib\$Configuration\zstd_static.lib"
    }
    
    Test-BuildOutput -Files @($expectedLib) -ThrowOnMissing
    
    Write-BuildLog "zstd 1.5.7 build completed successfully" -Level Success
    Write-BuildLog "Features: Zstandard compression, dictionary support" -Level Info
    
} catch {
    Write-BuildLog "Build failed: $($_.Exception.Message)" -Level Error
    exit 1
}

