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
Write-Host "  DarkThumbs Build Status" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Define expected library paths
$libraries = @(
    @{
        Name         = "zlib 1.3.1"
        Path         = "external\compression\zlib-1.3.1\x64\Release\zlibstatic.lib"
        ExpectedSize = 195
    },
    @{
        Name         = "LZ4 1.10.0"
        Path         = "external\compression\lz4-1.10.0\build\VS2022\bin\x64_Release\liblz4_static.lib"
        ExpectedSize = 150
    },
    @{
        Name         = "Zstandard 1.5.7"
        Path         = "external\compression\zstd-1.5.7\build\VS2022\bin\x64\Release\zstd_static.lib"
        ExpectedSize = 800
    },
    @{
        Name         = "liblzma (xz-5.6.3)"
        Path         = "external\compression\xz-5.6.3\build-vs\Release\liblzma.lib"
        ExpectedSize = 200
    },
    @{
        Name         = "LibWebP 1.5.0"
        Path         = "external\image-libs\libwebp-1.5.0\build-vs\output\x64\Release\release-static\x64\lib\webp.lib"
        ExpectedSize = 1700
    },
    @{
        Name         = "LibWebP demux"
        Path         = "external\image-libs\libwebp-1.5.0\build-vs\output\x64\Release\release-static\x64\lib\libwebpdemux.lib"
        ExpectedSize = 41
    },
    @{
        Name         = "LibWebP sharpyuv"
        Path         = "external\image-libs\libwebp-1.5.0\build-vs\output\x64\Release\release-static\x64\lib\libsharpyuv.lib"
        ExpectedSize = 79
    },
    @{
        Name         = "Minizip-NG 4.0.10"
        Path         = "external\compression\minizip-ng-4.0.10\build-vs\Release\minizip.lib"
        ExpectedSize = 200
    }
)

$found = 0
$missing = 0
$sizeIssues = 0

foreach ($lib in $libraries) {
    Write-Host "$($lib.Name):" -ForegroundColor White -NoNewline
    Write-Host " " -NoNewline
    
    if (Test-Path $lib.Path) {
        $file = Get-Item $lib.Path
        $sizeKB = [math]::Round($file.Length / 1KB, 1)
        
        # Check if size is reasonable (within 50% tolerance)
        $minSize = $lib.ExpectedSize * 0.5
        $maxSize = $lib.ExpectedSize * 2.0
        
        if ($sizeKB -lt $minSize -or $sizeKB -gt $maxSize) {
            Write-Host "⚠️  Found but unexpected size: $sizeKB KB (expected ~$($lib.ExpectedSize) KB)" -ForegroundColor Yellow
            $sizeIssues++
        } else {
            Write-Host "✅ $sizeKB KB" -ForegroundColor Green
            $found++
        }
    } else {
        Write-Host "❌ Not found" -ForegroundColor Red
        Write-Host "   Expected: $($lib.Path)" -ForegroundColor DarkGray
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

# Check CBXShell DLL
Write-Host "CBXShell Output:" -ForegroundColor Cyan
$cbxDll = "x64\Release\CBXShell.dll"
if (Test-Path $cbxDll) {
    $file = Get-Item $cbxDll
    $sizeKB = [math]::Round($file.Length / 1KB, 1)
    Write-Host "  ✅ CBXShell.dll: $sizeKB KB" -ForegroundColor Green
    Write-Host "     Modified: $($file.LastWriteTime)" -ForegroundColor Gray
} else {
    Write-Host "  ❌ CBXShell.dll not found" -ForegroundColor Red
}

Write-Host ""

# Exit code: 0 if all found, 1 if any missing
if ($missing -gt 0) {
    exit 1
} else {
    exit 0
}
