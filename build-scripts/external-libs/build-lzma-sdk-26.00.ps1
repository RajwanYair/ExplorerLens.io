#Requires -Version 7.0
# DarkThumbs v7.0 - Build LZMA SDK 26.00 (7-Zip compression)
# Refactored to use Build-Library-Core.ps1 module
# Date: February 16, 2026

param(
    [string]$Configuration = "Release",
    [switch]$Clean
)

# Import core build module
. "$PSScriptRoot\..\core\Build-Library-Core.ps1"

$rootDir = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$lzmaDir = Join-Path $rootDir "external\compression-libs\lzma-26.00\C"
$buildDir = Join-Path $lzmaDir "build-vs"
$installDir = Join-Path $rootDir "SDK\lzma"

Write-BuildHeader "Building LZMA SDK 26.00 (7-Zip Compression)"

# Verify source directory
if (-not (Test-Path $lzmaDir)) {
    Write-BuildLog "LZMA SDK 26.00 not found at $lzmaDir" -Level Error
    Write-BuildLog "Please download LZMA SDK from 7-zip.org" -Level Warning
    exit 1
}

Write-BuildLog "Source: $lzmaDir" -Level Info
Write-BuildLog "Build: $buildDir" -Level Info
Write-BuildLog "Install: $installDir" -Level Info

try {
    # LZMA SDK uses CMake with NMake
    $cmakeLists = Join-Path $lzmaDir "CMakeLists.txt"
    
    if (-not (Test-Path $cmakeLists)) {
        Write-BuildLog "Creating basic CMakeLists.txt for LZMA SDK" -Level Warning
        # LZMA SDK may not have CMakeLists.txt, we may need to use NMake directly
        # For now, try NMake build pattern
        
        $makefileDir = $lzmaDir
        $environment = @{
            'CFLAGS' = '/MD /O2 /DNDEBUG'
        }
        
        Invoke-NMakeBuild `
            -LibraryName "lzma" `
            -SourceDir $lzmaDir `
            -MakefileDir $makefileDir `
            -Target "all" `
            -Environment $environment
    }
    else {
        # Use CMake if available
        $cmakeOptions = @{
            'CMAKE_BUILD_TYPE'           = 'Release'
            'CMAKE_C_FLAGS_RELEASE'      = '/MD /O2 /DNDEBUG'
            'CMAKE_MSVC_RUNTIME_LIBRARY' = 'MultiThreadedDLL'
            'CMAKE_INSTALL_PREFIX'       = $installDir
        }
        
        Invoke-CMakeBuild `
            -LibraryName "lzma" `
            -SourceDir $lzmaDir `
            -BuildDir $buildDir `
            -InstallDir $installDir `
            -Configuration $Configuration `
            -CMakeOptions $cmakeOptions `
            -Generator "NMake Makefiles" `
            -Clean:$Clean
    }
    
    # Verify output
    $expectedLib = Join-Path $installDir "lib\lzma.lib"
    Test-BuildOutput -Files @($expectedLib) -ThrowOnMissing
    
    Write-BuildLog "LZMA SDK 26.00 build completed successfully" -Level Success
    Write-BuildLog "Features: 7-Zip compression, XZ format support" -Level Info
    
}
catch {
    Write-BuildLog "Build failed: $($_.Exception.Message)" -Level Error
    exit 1
}
