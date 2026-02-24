#Requires -Version 7.0
# ExplorerLens v7.0 - Build zlib 1.3.1 (DEFLATE Compression)
# Refactored to use Build-Library-Core.ps1 module
# Date: February 18, 2026
#
# Directory structure (post-cleanup):
#   Project root:       <repo>\
#   This script:        <repo>\build-scripts\external-libs\Build-Zlib.ps1
#   Core module:        <repo>\build-scripts\core\Build-Library-Core.ps1
#   zlib source:        <repo>\external\compression-libs\zlib-1.3.1\
#   Build dir:          <repo>\external\compression-libs\zlib-1.3.1\build-vs\
#   Install dir:        <repo>\external\compression-libs\zlib-1.3.1\install\

param(
    [string]$Configuration = "Release",
    [switch]$Clean
)

# Import core build module
. "$PSScriptRoot\..\core\Build-Library-Core.ps1"

$rootDir = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$zlibDir = Join-Path $rootDir "external\compression-libs\zlib-1.3.1"
$buildDir = Join-Path $zlibDir "build-vs"
$installDir = Join-Path $zlibDir "install"

Write-BuildHeader "Building zlib 1.3.1 (DEFLATE Compression)"

# Verify source directory
if (-not (Test-Path $zlibDir)) {
    Write-BuildLog "zlib-1.3.1 not found at $zlibDir" -Level Error
    Write-BuildLog "Please download and extract zlib source" -Level Warning
    exit 1
}

Write-BuildLog "Source: $zlibDir" -Level Info
Write-BuildLog "Build: $buildDir" -Level Info
Write-BuildLog "Install: $installDir" -Level Info

try {
    # Try CMake build first (preferred method)
    $cmakeLists = Join-Path $zlibDir "CMakeLists.txt"
    
    if (Test-Path $cmakeLists) {
        Write-BuildLog "Using CMake build" -Level Info
        
        $cmakeOptions = @{
            'CMAKE_BUILD_TYPE'  = 'Release'
            'BUILD_SHARED_LIBS' = 'ON'
        }
        
        Invoke-CMakeBuild `
            -LibraryName "zlib" `
            -SourceDir $zlibDir `
            -BuildDir $buildDir `
            -InstallDir $installDir `
            -Configuration $Configuration `
            -CMakeOptions $cmakeOptions `
            -Clean:$Clean
        
        # Verify output
        $expectedLib = Join-Path $installDir "lib\zlibstatic.lib"
        if (-not (Test-Path $expectedLib)) {
            # Try alternate location
            $expectedLib = Join-Path $buildDir "$Configuration\zlibstatic.lib"
        }
        
        Test-BuildOutput -Files @($expectedLib) -ThrowOnMissing
    } else {
        # Fallback to MSBuild if vcxproj exists
        $vcxproj = Join-Path $zlibDir "build-vs\zlib.vcxproj"
        
        if (Test-Path $vcxproj) {
            Write-BuildLog "Using MSBuild" -Level Info
            
            Invoke-MSBuildLibrary `
                -LibraryName "zlib" `
                -ProjectFile $vcxproj `
                -Configuration $Configuration `
                -Platform "x64" `
                -Clean:$Clean
        } else {
            throw "No build system found (CMakeLists.txt or zlib.vcxproj missing)"
        }
    }
    
    Write-BuildLog "zlib 1.3.1 build completed successfully" -Level Success
    Write-BuildLog "Features: DEFLATE compression, standard library" -Level Info
    
} catch {
    Write-BuildLog "Build failed: $($_.Exception.Message)" -Level Error
    exit 1
}

