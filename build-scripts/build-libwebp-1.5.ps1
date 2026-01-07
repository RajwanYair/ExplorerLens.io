# DarkThumbs - Build libwebp 1.5.0
# Builds the updated WebP library
# Date: November 19, 2025

param(
    [switch]$Clean
)

$ErrorActionPreference = "Stop"

Write-Host "`n=== Building libwebp 1.5.0 ===" -ForegroundColor Green

$rootDir = Split-Path -Parent $PSScriptRoot
$webpDir = Join-Path $rootDir "external\image-libs\libwebp-1.5.0"

if (-not (Test-Path $webpDir)) {
    Write-Host "ERROR: libwebp-1.5.0 not found at $webpDir" -ForegroundColor Red
    exit 1
}

Push-Location $webpDir

# Create build directory
$buildDir = "build-vs"
if ($Clean -and (Test-Path $buildDir)) {
    Write-Host "Cleaning previous build..." -ForegroundColor Yellow
    Remove-Item $buildDir -Recurse -Force
}

New-Item -ItemType Directory -Force -Path $buildDir | Out-Null

Push-Location $buildDir

try {
    Write-Host "Setting up Visual Studio environment..." -ForegroundColor Cyan
    
    # Find and run vcvarsall.bat to setup environment
    $vcvarsPath = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
    if (-not (Test-Path $vcvarsPath)) {
        $vcvarsPath = "C:\Program Files\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
    }
    
    if (-not (Test-Path $vcvarsPath)) {
        throw "vcvars64.bat not found - Visual Studio Build Tools may not be installed"
    }
    
    Write-Host "Configuring libwebp with CMake (NMake)..." -ForegroundColor Cyan
    
    # Use NMake generator instead
    $cmakeCmd = @"
call "$vcvarsPath" && cmake .. -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DWEBP_BUILD_ANIM_UTILS=OFF -DWEBP_BUILD_CWEBP=OFF -DWEBP_BUILD_DWEBP=OFF -DWEBP_BUILD_GIF2WEBP=OFF -DWEBP_BUILD_IMG2WEBP=OFF -DWEBP_BUILD_VWEBP=OFF -DWEBP_BUILD_WEBPINFO=OFF -DWEBP_BUILD_WEBPMUX=OFF -DWEBP_BUILD_EXTRAS=OFF -DBUILD_SHARED_LIBS=OFF -DCMAKE_C_FLAGS_RELEASE="/MT /O2" -DCMAKE_CXX_FLAGS_RELEASE="/MT /O2"
"@
    
    cmd /c $cmakeCmd
    
    if ($LASTEXITCODE -ne 0) {
        throw "CMake configuration failed"
    }
    
    Write-Host "Building libwebp with NMake..." -ForegroundColor Cyan
    cmd /c "call `"$vcvarsPath`" && nmake webp"
    
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed"
    }
    
    # Copy output to standard location
    $outputDir = Join-Path $webpDir "x64\Release"
    New-Item -ItemType Directory -Force -Path $outputDir | Out-Null
    
    # Find and copy the built libraries
    if (Test-Path "webp.lib") {
        Copy-Item "webp.lib" $outputDir\libwebp.lib -Force
    } elseif (Test-Path "src\webp.lib") {
        Copy-Item "src\webp.lib" $outputDir\libwebp.lib -Force
    }
    
    if (Test-Path "sharpyuv.lib") {
        Copy-Item "sharpyuv.lib" $outputDir\libsharpyuv.lib -Force
    } elseif (Test-Path "sharpyuv\sharpyuv.lib") {
        Copy-Item "sharpyuv\sharpyuv.lib" $outputDir\libsharpyuv.lib -Force
    }
    
    Write-Host "`n=== libwebp 1.5.0 Build Complete ===" -ForegroundColor Green
    
    $size = (Get-Item "$outputDir\libwebp.lib").Length
    Write-Host "libwebp.lib: $($size.ToString('N0')) bytes" -ForegroundColor White
    
    if (Test-Path "$outputDir\libsharpyuv.lib") {
        $size = (Get-Item "$outputDir\libsharpyuv.lib").Length
        Write-Host "libsharpyuv.lib: $($size.ToString('N0')) bytes" -ForegroundColor White
    }
    
} catch {
    Write-Host "`nBuild Failed: $($_.Exception.Message)" -ForegroundColor Red
    Pop-Location
    Pop-Location
    exit 1
}

Pop-Location
Pop-Location

Write-Host "`nReady to rebuild DarkThumbs with updated libwebp!" -ForegroundColor Cyan
