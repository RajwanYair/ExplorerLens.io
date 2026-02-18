#Requires -Version 7.0
# DarkThumbs v7.0 - Build minizip-ng 4.0.10 (Modern ZIP Library)
# Refactored to use Build-Library-Core.ps1 module
# Date: February 16, 2026

param(
    [string]$Configuration = "Release",
    [switch]$Clean
)

# Import core build module
. "$PSScriptRoot\..\core\Build-Library-Core.ps1"

$rootDir = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$sourceDir = Join-Path $rootDir "external\compression-libs\minizip-ng-4.0.10"
$buildDir = Join-Path $sourceDir "build-vs"
$outputDir = Join-Path $buildDir $Configuration

Write-BuildHeader "Building minizip-ng 4.0.10"

# Verify source directory
if (-not (Test-Path $sourceDir)) {
    Write-BuildLog "minizip-ng-4.0.10 not found at $sourceDir" -Level Error
    exit 1
}

Write-BuildLog "Source: $sourceDir" -Level Info
Write-BuildLog "Build: $buildDir" -Level Info

try {
    # CMake options for minizip-ng
    $cmakeOptions = @{
        'BUILD_SHARED_LIBS' = 'OFF'
        'MZ_COMPAT'         = 'OFF'
        'MZ_ZLIB'           = 'ON'
        'MZ_BZIP2'          = 'OFF'
        'MZ_LZMA'           = 'ON'
        'MZ_ZSTD'           = 'ON'
        'MZ_OPENSSL'        = 'OFF'
        'MZ_LIBCOMP'        = 'OFF'
        'MZ_FETCH_LIBS'     = 'OFF'
    }
    
    # Build with CMake
    Invoke-CMakeBuild `
        -LibraryName "minizip-ng" `
        -SourceDir $sourceDir `
        -BuildDir $buildDir `
        -Configuration $Configuration `
        -CMakeOptions $cmakeOptions `
        -Clean:$Clean
    
    # Verify output
    $expectedLib = Join-Path $outputDir "minizip.lib"
    Test-BuildOutput -Files @($expectedLib) -ThrowOnMissing
    
    Write-BuildLog "minizip-ng 4.0.10 build completed successfully" -Level Success
    Write-BuildLog "Output: $expectedLib" -Level Info
    Write-BuildLog "Next step: Copy to external\compression-libs\minizip-ng-4.0.10\build-manual\$Configuration\" -Level Info
    
} catch {
    Write-BuildLog "Build failed: $($_.Exception.Message)" -Level Error
    exit 1
}

Get-ChildItem $buildDir -Recurse -Filter "*.lib" | Format-Table FullName, Length
