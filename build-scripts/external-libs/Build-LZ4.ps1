#Requires -Version 7.0
# DarkThumbs v7.0 - Build LZ4 1.10.0 (Fast Compression)
# Refactored to use Build-Library-Core.ps1 module
# Date: February 16, 2026

param(
    [string]$Configuration = "Release",
    [switch]$Clean
)

# Import core build module
. "$PSScriptRoot\..\core\Build-Library-Core.ps1"

$rootDir = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$lz4Dir = Join-Path $rootDir "external\compression-libs\lz4-1.10.0"
$buildDir = Join-Path $lz4Dir "build\VS2022"

Write-BuildHeader "Building LZ4 1.10.0 (Fast Compression)"

# Verify source directory
if (-not (Test-Path $lz4Dir)) {
    Write-BuildLog "lz4-1.10.0 not found at $lz4Dir" -Level Error
    Write-BuildLog "Please download and extract LZ4 source" -Level Warning
    exit 1
}

Write-BuildLog "Source: $lz4Dir" -Level Info
Write-BuildLog "Build: $buildDir" -Level Info

try {
    # LZ4 has Visual Studio project files
    $vcxproj = Join-Path $buildDir "liblz4\liblz4.vcxproj"
    
    if (-not (Test-Path $vcxproj)) {
        # Try CMake fallback
        $cmakeLists = Join-Path $lz4Dir "build\cmake\CMakeLists.txt"
        
        if (Test-Path $cmakeLists) {
            Write-BuildLog "Using CMake build" -Level Info
            
            $cmakeBuildDir = Join-Path $lz4Dir "build-vs"
            $cmakeOptions = @{
                'CMAKE_BUILD_TYPE'      = 'Release'
                'BUILD_SHARED_LIBS'     = 'OFF'
                'LZ4_BUILD_CLI'         = 'OFF'
                'LZ4_BUILD_LEGACY_LZ4C' = 'OFF'
            }
            
            Invoke-CMakeBuild `
                -LibraryName "lz4" `
                -SourceDir (Join-Path $lz4Dir "build\cmake") `
                -BuildDir $cmakeBuildDir `
                -Configuration $Configuration `
                -CMakeOptions $cmakeOptions `
                -Clean:$Clean
        } else {
            throw "No build system found (liblz4.vcxproj or CMakeLists.txt missing)"
        }
    } else {
        Write-BuildLog "Using MSBuild" -Level Info
        
        Invoke-MSBuildLibrary `
            -LibraryName "liblz4" `
            -ProjectFile $vcxproj `
            -Configuration $Configuration `
            -Platform "x64" `
            -Clean:$Clean
        
        # Verify output (LZ4 outputs to non-standard location)
        $expectedLib = Join-Path $buildDir "liblz4\bin\x64_$Configuration\liblz4_static.lib"
        Test-BuildOutput -Files @($expectedLib) -ThrowOnMissing
    }
    
    Write-BuildLog "LZ4 1.10.0 build completed successfully" -Level Success
    Write-BuildLog "Features: Fast compression, streaming API" -Level Info
    
} catch {
    Write-BuildLog "Build failed: $($_.Exception.Message)" -Level Error
    exit 1
}
