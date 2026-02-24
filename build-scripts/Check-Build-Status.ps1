#Requires -Version 7.0
<#
.SYNOPSIS
    Quick status check for all built libraries

.DESCRIPTION
    Checks the existence and size of all built library files.
    Provides a quick overview of what's built and what's missing.

.EXAMPLE
    .\Check-Build-Status.ps1
#>

$ErrorActionPreference = "SilentlyContinue"

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  ExplorerLens Build Status" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

function Resolve-LibraryPath {
    param(
        [string[]]$CandidatePaths
    )

    foreach ($candidate in $CandidatePaths) {
        if (Test-Path $candidate) {
            return $candidate
        }
    }

    return $null
}

# Define expected library paths (support legacy + current output layouts)
$libraries = @(
    @{
        Name           = "zlib 1.3.1"
        CandidatePaths = @(
            "external\compression-libs\zlib-1.3.1\x64\Release\zlibstatic.lib",
            "external\compression-libs\zlib-1.3.1\build-vs\Release\zlibstatic.lib",
            "external\compression-libs\zlib-1.3.1\build-vs\Release\zlib.lib",
            "SDK\lib\zlib.lib"
        )
        MinSizeKB      = 100
        MaxSizeKB      = 2000
    },
    @{
        Name           = "LZ4 1.10.0"
        CandidatePaths = @(
            "external\compression-libs\lz4-1.10.0\build\VS2022\bin\x64_Release\liblz4_static.lib",
            "external\compression-libs\lz4-1.10.0\build-vs\Release\liblz4_static.lib",
            "external\compression-libs\lz4-1.10.0\build-msvc\lib\Release\liblz4_static.lib",
            "SDK\lib\liblz4_static.lib"
        )
        MinSizeKB      = 100
        MaxSizeKB      = 2000
    },
    @{
        Name           = "Zstandard 1.5.7"
        CandidatePaths = @(
            "external\compression-libs\zstd-1.5.7\build\VS2022\bin\x64\Release\zstd_static.lib",
            "external\compression-libs\zstd-1.5.7\build-manual\Release\zstd_static.lib",
            "external\compression-libs\zstd-1.5.7\build\cmake\build-md\lib\zstd.lib",
            "SDK\lib\zstd.lib"
        )
        MinSizeKB      = 20
        MaxSizeKB      = 4000
    },
    @{
        Name           = "liblzma (xz-5.6.3)"
        CandidatePaths = @(
            "external\compression-libs\xz-5.6.3\build-vs\Release\liblzma.lib",
            "external\compression-libs\xz-5.6.3\build-vs\Release\lzma.lib",
            "SDK\lzma\lib\lzma.lib",
            "SDK\lzma\lib\liblzma.lib"
        )
        MinSizeKB      = 50
        MaxSizeKB      = 3000
    },
    @{
        Name           = "LibWebP 1.5.0"
        CandidatePaths = @(
            "external\image-libs\libwebp-1.5.0-build\build-vs\Release\webp.lib",
            "external\image-libs\libwebp-1.5.0-build\build-vs\output\x64\Release\release-static\x64\lib\webp.lib",
            "external\image-libs\libwebp-1.5.0-build\build-cmake\Release\libwebp.lib",
            "external\image-libs\libwebp-1.5.0-original\build-vs\Release\webp.lib",
            "SDK\lib\webp.lib"
        )
        MinSizeKB      = 500
        MaxSizeKB      = 5000
    },
    @{
        Name           = "LibWebP demux"
        CandidatePaths = @(
            "external\image-libs\libwebp-1.5.0-build\build-vs\Release\webpdemux.lib",
            "external\image-libs\libwebp-1.5.0-build\build-vs\Release\libwebpdemux.lib",
            "external\image-libs\libwebp-1.5.0-build\build-cmake\Release\libwebpdemux.lib",
            "external\image-libs\libwebp-1.5.0-build\build-vs\output\x64\Release\release-static\x64\lib\libwebpdemux.lib",
            "external\image-libs\libwebp-1.5.0-build\build-vs\output\x64\Release\release-static\x64\lib\webpdemux.lib",
            "SDK\lib\libwebpdemux.lib",
            "SDK\lib\webpdemux.lib"
        )
        MinSizeKB      = 5
        MaxSizeKB      = 500
    },
    @{
        Name           = "LibWebP sharpyuv"
        CandidatePaths = @(
            "external\image-libs\libwebp-1.5.0-build\build-vs\Release\sharpyuv.lib",
            "external\image-libs\libwebp-1.5.0-build\build-vs\Release\libsharpyuv.lib",
            "external\image-libs\libwebp-1.5.0-build\build-cmake\Release\libsharpyuv.lib",
            "external\image-libs\libwebp-1.5.0-build\build-vs\output\x64\Release\release-static\x64\lib\libsharpyuv.lib",
            "external\image-libs\libwebp-1.5.0-build\build-vs\output\x64\Release\release-static\x64\lib\sharpyuv.lib",
            "SDK\lib\libsharpyuv.lib",
            "SDK\lib\sharpyuv.lib"
        )
        MinSizeKB      = 10
        MaxSizeKB      = 500
    },
    @{
        Name           = "Minizip-NG 4.0.10"
        CandidatePaths = @(
            "external\compression-libs\minizip-ng-4.0.10\build-vs\Release\minizip.lib",
            "external\compression-libs\minizip-ng-4.0.10\build-manual\minizip.lib",
            "external\compression-libs\minizip-ng-4.0.10\build-ninja\minizip.lib",
            "SDK\lib\minizip.lib"
        )
        MinSizeKB      = 50
        MaxSizeKB      = 1000
    }
)

$found = 0
$missing = 0
$sizeIssues = 0

foreach ($lib in $libraries) {
    Write-Host "$($lib.Name):" -ForegroundColor White -NoNewline
    Write-Host " " -NoNewline
    
    $resolvedPath = Resolve-LibraryPath -CandidatePaths $lib.CandidatePaths

    if ($resolvedPath) {
        $file = Get-Item $resolvedPath
        $sizeKB = [math]::Round($file.Length / 1KB, 1)
        
        # Check if size is within expected range for known valid variants
        $minSize = $lib.MinSizeKB
        $maxSize = $lib.MaxSizeKB
        
        if ($sizeKB -lt $minSize -or $sizeKB -gt $maxSize) {
            Write-Host "⚠️  Found but unexpected size: $sizeKB KB (expected range $minSize-$maxSize KB)" -ForegroundColor Yellow
            Write-Host "   Found at: $resolvedPath" -ForegroundColor DarkGray
            $sizeIssues++
        } else {
            Write-Host "✅ $sizeKB KB" -ForegroundColor Green
            Write-Host "   Found at: $resolvedPath" -ForegroundColor DarkGray
            $found++
        }
    } else {
        Write-Host "❌ Not found" -ForegroundColor Red
        Write-Host "   Tried:" -ForegroundColor DarkGray
        foreach ($candidate in $lib.CandidatePaths) {
            Write-Host "    - $candidate" -ForegroundColor DarkGray
        }
        $missing++
    }
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Summary:" -ForegroundColor Cyan
Write-Host "  ✅ Built: $found" -ForegroundColor Green
if ($sizeIssues -gt 0) {
    Write-Host "  ⚠️  Size issues: $sizeIssues" -ForegroundColor Yellow
}
if ($missing -gt 0) {
    Write-Host "  ❌ Missing: $missing" -ForegroundColor Red
}
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Check LENSShell DLL
Write-Host "LENSShell Output:" -ForegroundColor Cyan
$LENSDll = "x64\Release\LENSShell.dll"
if (Test-Path $LENSDll) {
    $file = Get-Item $LENSDll
    $sizeKB = [math]::Round($file.Length / 1KB, 1)
    Write-Host "  ✅ LENSShell.dll: $sizeKB KB" -ForegroundColor Green
    Write-Host "     Modified: $($file.LastWriteTime)" -ForegroundColor Gray
} else {
    Write-Host "  ❌ LENSShell.dll not found" -ForegroundColor Red
}

Write-Host ""

# Exit code: 0 if all found, 1 if any missing
if ($missing -gt 0) {
    exit 1
} else {
    exit 0
}

