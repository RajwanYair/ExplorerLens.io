#Requires -Version 7.0
# ExplorerLens v7.0 - Build libjxl 0.11.1 (JPEG XL)
# Refactored to use Build-Library-Core.ps1 module
# Date: February 18, 2026
#
# Directory structure (post-cleanup):
#   Project root:       <repo>\
#   This script:        <repo>\build-scripts\external-libs\Build-LibJXL.ps1
#   Core module:        <repo>\build-scripts\core\Build-Library-Core.ps1
#   libjxl source:      <repo>\external\image-libs\libjxl-0.11.1\
#   Build dir:          <repo>\external\image-libs\libjxl-0.11.1\build-msvc\
#   Install dir:        <repo>\external\image-libs\libjxl-0.11.1\install\

param(
    [string]$Configuration = "Release",
    [switch]$Clean
)

# Import core build module
. "$PSScriptRoot\..\core\Build-Library-Core.ps1"

$rootDir = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$sourceDir = Join-Path $rootDir "external\image-libs\libjxl-0.11.1"
$buildDir = Join-Path $sourceDir "build-msvc"
$installDir = Join-Path $sourceDir "install"

Write-BuildHeader "Building libjxl 0.11.1 (JPEG XL)"

# Verify source directory
if (-not (Test-Path $sourceDir)) {
    Write-BuildLog "libjxl-0.11.1 not found at $sourceDir" -Level Error
    Write-BuildLog "Please run download-all-libs.ps1 first" -Level Warning
    exit 1
}

Write-BuildLog "Source: $sourceDir" -Level Info
Write-BuildLog "Build: $buildDir" -Level Info
Write-BuildLog "Install: $installDir" -Level Info

# Initialize git submodules if this is a git repo
if (Test-Path (Join-Path $sourceDir ".git")) {
    Write-BuildLog "Initializing git submodules..." -Level Info
    Push-Location $sourceDir
    try {
        git submodule update --init --recursive 2>&1 | Out-Null
    } catch {
        Write-BuildLog "Git submodule init failed, continuing with bundled dependencies" -Level Warning
    } finally {
        Pop-Location
    }
}

try {
    # CMake options for libjxl
    $cmakeOptions = @{
        'CMAKE_BUILD_TYPE'           = 'Release'
        'BUILD_SHARED_LIBS'          = 'ON'
        'BUILD_TESTING'              = 'ON'
        'JPEGXL_ENABLE_TOOLS'        = 'ON'
        'JPEGXL_ENABLE_DOXYGEN'      = 'ON'
        'JPEGXL_ENABLE_MANPAGES'     = 'ON'
        'JPEGXL_ENABLE_BENCHMARK'    = 'ON'
        'JPEGXL_ENABLE_EXAMPLES'     = 'ON'
        'JPEGXL_ENABLE_JNI'          = 'ON'
        'JPEGXL_ENABLE_SJPEG'        = 'ON'
        'JPEGXL_ENABLE_OPENEXR'      = 'ON'
        'JPEGXL_ENABLE_SKCMS'        = 'ON'
        'JPEGXL_FORCE_SYSTEM_BROTLI' = 'ON'
        'JPEGXL_FORCE_SYSTEM_HWY'    = 'ON'
    }
    
    # Build with CMake
    Invoke-CMakeBuild `
        -LibraryName "libjxl" `
        -SourceDir $sourceDir `
        -BuildDir $buildDir `
        -InstallDir $installDir `
        -Configuration $Configuration `
        -CMakeOptions $cmakeOptions `
        -Clean:$Clean
    
    # Verify outputs (jxl_dec.lib was merged into jxl.lib in 0.11.x)
    $expectedLibs = @(
        (Join-Path $installDir "lib\jxl.lib"),
        (Join-Path $installDir "lib\jxl_cms.lib"),
        (Join-Path $installDir "lib\jxl_threads.lib")
    )
    
    Test-BuildOutput -Files $expectedLibs -ThrowOnMissing:$false
    
    Write-BuildLog "libjxl 0.11.1 build completed successfully" -Level Success
    Write-BuildLog "Features: JPEG XL decoding, SKCMS color management, bundled Brotli/Highway" -Level Info
    
} catch {
    Write-BuildLog "Build failed: $($_.Exception.Message)" -Level Error
    exit 1
}


