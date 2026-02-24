#Requires -Version 7.0
# ExplorerLens v15.0 "Zenith" — Build libwebp 1.5.0 using native Makefile.vc
# Sprint 351: Ensures /MD (dynamic CRT) to eliminate NODEFAULTLIB:LIBCMT workaround
# Refactored to use Build-Library-Core.ps1 module
#
# Directory structure (post-cleanup):
#   Project root:       <repo>\
#   This script:        <repo>\build-scripts\external-libs\Build-LibWebP-NMake.ps1
#   Core module:        <repo>\build-scripts\core\Build-Library-Core.ps1
#   libwebp source:     <repo>\external\image-libs\libwebp-1.5.0-build\
#   Build dir:          <repo>\external\image-libs\libwebp-1.5.0-build\build-cmake\

param(
    [switch]$Clean
)

# Import core build module
. "$PSScriptRoot\..\core\Build-Library-Core.ps1"

$rootDir = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$webpDir = Join-Path $rootDir "external\image-libs\libwebp-1.5.0-build"
$outputDir = Join-Path $webpDir "build-vs\Release"

Write-BuildHeader "Building libwebp 1.5.0 (NMake)"

# Verify source directory
if (-not (Test-Path $webpDir)) {
    Write-BuildLog "libwebp-1.5.0-build not found at $webpDir" -Level Error
    Write-BuildLog "Note: Path was corrected from libwebp-1.5.0 to libwebp-1.5.0-build" -Level Info
    exit 1
}

Write-BuildLog "Source: $webpDir" -Level Info

# Clean if requested
if ($Clean) {
    $outputPath = Join-Path $webpDir "output"
    if (Test-Path $outputPath) {
        Write-BuildLog "Cleaning previous build..." -Level Info
        Remove-Item $outputPath -Recurse -Force -ErrorAction SilentlyContinue
    }
}

try {
    # Build with NMake if available, otherwise fallback to CMake
    $nmake = Get-Command nmake.exe -ErrorAction SilentlyContinue
    $makefilePath = Join-Path $webpDir "Makefile.vc"
    if ($nmake -and (Test-Path $makefilePath)) {
        Invoke-NMakeBuild -LibraryName "libwebp" -SourceDir $webpDir -Makefile "Makefile.vc" -MakefileVars @{
            'CFG'      = 'release-static'
            'RTLIBCFG' = 'dynamic'
            'OBJDIR'   = 'output\x64\Release'
        }
    } else {
        if (-not $nmake) {
            Write-BuildLog "nmake not found in environment; falling back to CMake build" -Level Warning
        } elseif (-not (Test-Path $makefilePath)) {
            Write-BuildLog "Makefile.vc not found in source tree; falling back to CMake build" -Level Warning
        }
        $cmakeBuildDir = Join-Path $webpDir "build-cmake"
        $cmakeOptions = @{
            'BUILD_SHARED_LIBS'          = 'OFF'
            'CMAKE_MSVC_RUNTIME_LIBRARY' = 'MultiThreadedDLL'
            'WEBP_BUILD_CWEBP'           = 'OFF'
            'WEBP_BUILD_DWEBP'           = 'OFF'
            'WEBP_BUILD_GIF2WEBP'        = 'OFF'
            'WEBP_BUILD_IMG2WEBP'        = 'OFF'
            'WEBP_BUILD_VWEBP'           = 'OFF'
            'WEBP_BUILD_WEBPINFO'        = 'OFF'
            'WEBP_BUILD_WEBPMUX'         = 'OFF'
            'WEBP_BUILD_EXTRAS'          = 'ON'
            'WEBP_BUILD_ANIM_UTILS'      = 'ON'
        }

        Invoke-CMakeBuild `
            -LibraryName "libwebp" `
            -SourceDir $webpDir `
            -BuildDir $cmakeBuildDir `
            -Configuration "Release" `
            -CMakeOptions $cmakeOptions `
            -Clean:$Clean
    }
    
    # Find and verify outputs
    Write-BuildLog "Verifying build outputs..." -Level Info
    
    $possibleOutputDirs = @(
        "output\release-static\x64\lib",
        "output\x64\Release\release-static\x64\lib",
        "output\x64\Release",
        "build-cmake\Release",
        "build-cmake\lib\Release"
    )
    
    $outputLibDir = $null
    foreach ($dir in $possibleOutputDirs) {
        $fullPath = Join-Path $webpDir $dir
        if (Test-Path $fullPath) {
            $outputLibDir = $fullPath
            break
        }
    }
    
    if (-not $outputLibDir) {
        # Fallback: search for libwebp.lib
        $foundLib = Get-ChildItem (Join-Path $webpDir "output") -Recurse -Filter "libwebp.lib" -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($foundLib) {
            $outputLibDir = $foundLib.DirectoryName
        } else {
            throw "Build outputs not found"
        }
    }
    
    # Verify expected libraries
    $expectedLibs = @(
        (Join-Path $outputLibDir "libwebp.lib"),
        (Join-Path $outputLibDir "libsharpyuv.lib")
    )
    
    Test-BuildOutput -Files $expectedLibs -ThrowOnMissing:$false
    
    # Copy to standard location
    New-CleanDirectory -Path $outputDir
    
    # Copy main libraries with rename
    $mainLib = Join-Path $outputLibDir "libwebp.lib"
    if (Test-Path $mainLib) {
        Copy-Item $mainLib (Join-Path $outputDir "webp.lib") -Force
        Write-BuildLog "Copied webp.lib" -Level Success
    }
    
    $sharpyuvLib = Join-Path $outputLibDir "libsharpyuv.lib"
    if (Test-Path $sharpyuvLib) {
        Copy-Item $sharpyuvLib (Join-Path $outputDir "sharpyuv.lib") -Force
        Write-BuildLog "Copied sharpyuv.lib" -Level Success
    }

    # Optional WebP companion libraries (present in some build variants)
    $webpDemuxCandidates = @(
        (Join-Path $outputLibDir "webpdemux.lib"),
        (Join-Path $outputLibDir "libwebpdemux.lib")
    )
    $webpDecoderCandidates = @(
        (Join-Path $outputLibDir "webpdecoder.lib"),
        (Join-Path $outputLibDir "libwebpdecoder.lib")
    )

    $webpDemuxLib = $webpDemuxCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1
    if ($webpDemuxLib) {
        Copy-Item $webpDemuxLib (Join-Path $outputDir "webpdemux.lib") -Force
        Write-BuildLog "Copied webpdemux.lib" -Level Success
    }

    $webpDecoderLib = $webpDecoderCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1
    if ($webpDecoderLib) {
        Copy-Item $webpDecoderLib (Join-Path $outputDir "webpdecoder.lib") -Force
        Write-BuildLog "Copied webpdecoder.lib" -Level Success
    }
    
    Write-BuildLog "libwebp 1.5.0 build completed successfully" -Level Success
    Write-BuildLog "Output: $outputDir" -Level Info
    
} catch {
    Write-BuildLog "Build failed: $($_.Exception.Message)" -Level Error
    exit 1
}


