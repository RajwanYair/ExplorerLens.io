#Requires -Version 7.0
# ExplorerLens v7.0 - Build LZMA SDK 26.00 (7-Zip compression)
# Refactored to use Build-Library-Core.ps1 module
# Date: February 18, 2026
#
# Directory structure (post-cleanup):
#   Project root:       <repo>\
#   This script:        <repo>\build-scripts\external-libs\Build-LZMA-SDK-26.00.ps1
#   Core module:        <repo>\build-scripts\core\Build-Library-Core.ps1
#   LZMA source:        <repo>\external\compression-libs\lzma-26.00\C\
#   Build dir:          <repo>\external\compression-libs\lzma-26.00\C\build-vs\
#   Install dir:        <repo>\SDK\lzma\

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
    # LZMA SDK build path
    $cmakeLists = Join-Path $lzmaDir "CMakeLists.txt"
    
    if (-not (Test-Path $cmakeLists)) {
        Write-BuildLog "Creating basic CMakeLists.txt for LZMA SDK" -Level Warning
        $cmakeContent = @"
cmake_minimum_required(VERSION 3.20)
project(lzma LANGUAGES C)

add_library(lzma STATIC
    Alloc.c
    CpuArch.c
    LzFind.c
    LzFindMt.c
    LzmaDec.c
    LzmaEnc.c
    LzmaLib.c
    Threads.c
)

target_include_directories(lzma PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

set_target_properties(lzma PROPERTIES OUTPUT_NAME "lzma")

install(TARGETS lzma
    ARCHIVE DESTINATION lib
)

install(FILES
    LzmaLib.h
    LzmaDec.h
    LzmaEnc.h
    DESTINATION include
)
"@
        Set-Content -Path $cmakeLists -Value $cmakeContent -Encoding UTF8
    }

    $cmakeOptions = @{
        'CMAKE_BUILD_TYPE'           = 'Release'
        'CMAKE_MSVC_RUNTIME_LIBRARY' = 'MultiThreadedDLL'
    }
    
    Invoke-CMakeBuild `
        -LibraryName "lzma" `
        -SourceDir $lzmaDir `
        -BuildDir $buildDir `
        -InstallDir $installDir `
        -Configuration $Configuration `
        -CMakeOptions $cmakeOptions `
        -Clean:$Clean
    
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

