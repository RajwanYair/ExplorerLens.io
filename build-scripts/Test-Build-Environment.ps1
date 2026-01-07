#Requires -Version 7.0

<#
.SYNOPSIS
    Test and validate build environment for DarkThumbs
.DESCRIPTION
    Checks all prerequisites and external library directories
#>

$ErrorActionPreference = "Continue"

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "  DarkThumbs Build Environment Test" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

$rootDir = Split-Path -Parent $PSScriptRoot
Set-Location $rootDir

$results = @()

# Test Visual Studio Build Tools
Write-Host "[1] Visual Studio Build Tools" -ForegroundColor Yellow
$vcvarsPath = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
if (Test-Path $vcvarsPath) {
    Write-Host "  ✅ vcvars64.bat found" -ForegroundColor Green
    $results += [PSCustomObject]@{ Component = "VS Build Tools"; Status = "✅ OK"; Path = $vcvarsPath }
} else {
    Write-Host "  ❌ vcvars64.bat NOT found" -ForegroundColor Red
    $results += [PSCustomObject]@{ Component = "VS Build Tools"; Status = "❌ MISSING"; Path = $vcvarsPath }
}

# Test CMake
Write-Host "`n[2] CMake" -ForegroundColor Yellow
$cmake = Get-Command cmake -ErrorAction SilentlyContinue
if ($cmake) {
    $version = & cmake --version | Select-Object -First 1
    Write-Host "  ✅ $version" -ForegroundColor Green
    $results += [PSCustomObject]@{ Component = "CMake"; Status = "✅ OK"; Path = $cmake.Source }
} else {
    Write-Host "  ❌ CMake NOT found in PATH" -ForegroundColor Red
    $results += [PSCustomObject]@{ Component = "CMake"; Status = "❌ MISSING"; Path = "N/A" }
}

# Test external libraries
Write-Host "`n[3] External Libraries" -ForegroundColor Yellow

$libraries = @{
    "zlib-1.3.1"         = "external\compression\zlib-1.3.1"
    "lz4-1.10.0"         = "external\compression\lz4-1.10.0"
    "zstd-1.5.7"         = "external\compression\zstd-1.5.7"
    "xz-5.6.3 (liblzma)" = "external\compression\xz-5.6.3"
    "minizip-ng-4.0.10"  = "external\compression\minizip-ng-4.0.10"
    "libwebp-1.5.0"      = "external\image-libs\libwebp-1.5.0"
    "dav1d-1.5.1"        = "external\image-libs\dav1d-1.5.1"
    "libavif-1.3.0"      = "external\image-libs\libavif-1.3.0"
    "libjxl-0.11.1"      = "external\image-libs\libjxl-0.11.1"
}

foreach ($lib in $libraries.GetEnumerator()) {
    $path = Join-Path $rootDir $lib.Value
    if (Test-Path $path) {
        Write-Host "  ✅ $($lib.Key)" -ForegroundColor Green
        $results += [PSCustomObject]@{ Component = $lib.Key; Status = "✅ FOUND"; Path = $lib.Value }
    } else {
        Write-Host "  ❌ $($lib.Key) - NOT FOUND" -ForegroundColor Red
        $results += [PSCustomObject]@{ Component = $lib.Key; Status = "❌ MISSING"; Path = $lib.Value }
    }
}

# Test critical build files
Write-Host "`n[4] Critical Build Files" -ForegroundColor Yellow

$criticalFiles = @{
    "libwebp Makefile.vc"       = "external\image-libs\libwebp-1.5.0\Makefile.vc"
    "xz CMakeLists.txt"         = "external\compression\xz-5.6.3\CMakeLists.txt"
    "minizip-ng CMakeLists.txt" = "external\compression\minizip-ng-4.0.10\CMakeLists.txt"
}

foreach ($file in $criticalFiles.GetEnumerator()) {
    $path = Join-Path $rootDir $file.Value
    if (Test-Path $path) {
        Write-Host "  ✅ $($file.Key)" -ForegroundColor Green
    } else {
        Write-Host "  ❌ $($file.Key) - NOT FOUND" -ForegroundColor Red
    }
}

# Summary
Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "  Summary" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

$passed = ($results | Where-Object { $_.Status -like "*OK*" -or $_.Status -like "*FOUND*" }).Count
$failed = ($results | Where-Object { $_.Status -like "*MISSING*" }).Count

Write-Host "Passed: $passed" -ForegroundColor Green
Write-Host "Failed: $failed" -ForegroundColor $(if ($failed -gt 0) { "Red" } else { "Green" })

if ($failed -eq 0) {
    Write-Host "`n✅ Build environment is ready!" -ForegroundColor Green
    exit 0
} else {
    Write-Host "`n❌ Build environment has missing components!" -ForegroundColor Red
    exit 1
}
