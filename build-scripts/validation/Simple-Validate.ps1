# Simple Build Validation
# Quick validation of build status without complex functions

$ErrorActionPreference = "Continue"

Write-Host "`nDarkThumbs Build Validation`n" -ForegroundColor Cyan

# Check new headers
Write-Host "Checking new header files..." -ForegroundColor Yellow
$headers = @(
    "CBXShell\error_logger.h",
    "CBXShell\performance_profiler.h",
    "CBXShell\memory_utils.h",
    "CBXShell\enhanced_cache.h"
)

$headerCount = 0
foreach ($h in $headers) {
    if (Test-Path $h) {
        Write-Host "  [OK] $h" -ForegroundColor Green
        $headerCount++
    } else {
        Write-Host "  [MISS] $h" -ForegroundColor Red
    }
}
Write-Host "Headers: $headerCount / $($headers.Count)`n"

# Check build outputs
Write-Host "Checking build outputs..." -ForegroundColor Yellow
$dll = "CBXShell\x64\Release\CBXShell.dll"
$exe = "CBXManager\x64\Release\CBXManager.exe"

if (Test-Path $dll) {
    $size = (Get-Item $dll).Length / 1MB
    $date = (Get-Item $dll).LastWriteTime
    Write-Host "  [OK] CBXShell.dll - $([math]::Round($size, 2)) MB - $date" -ForegroundColor Green
} else {
    Write-Host "  [MISS] CBXShell.dll" -ForegroundColor Red
}

if (Test-Path $exe) {
    $size = (Get-Item $exe).Length / 1MB
    $date = (Get-Item $exe).LastWriteTime
    Write-Host "  [OK] CBXManager.exe - $([math]::Round($size, 2)) MB - $date" -ForegroundColor Green
} else {
    Write-Host "  [MISS] CBXManager.exe" -ForegroundColor Red
}

# Check static libraries
Write-Host "`nChecking static libraries..." -ForegroundColor Yellow
$libs = @(
    "external\compression\zlib\lib\zlibstatic.lib",
    "external\compression\bzip2\lib\libbz2.lib",
    "external\compression\zstd\lib\libzstd_static.lib",
    "external\compression\lz4\lib\liblz4_static.lib",
    "external\compression\lzma\lib\lzma.lib",
    "external\compression\minizip-ng\lib\minizip.lib",
    "external\compression\unrar\lib\unrar.lib",
    "external\image\libwebp\lib\libwebp.lib"
)

$libCount = 0
foreach ($lib in $libs) {
    if (Test-Path $lib) {
        $libCount++
    }
}
Write-Host "Static libraries: $libCount / $($libs.Count)"

if ($libCount -eq $libs.Count) {
    Write-Host "  [OK] All libraries present" -ForegroundColor Green
} else {
    Write-Host "  [WARN] Some libraries missing" -ForegroundColor Yellow
}

Write-Host "`nValidation complete!`n" -ForegroundColor Cyan
