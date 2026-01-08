# DarkThumbs - Build libwebp 1.5.0 (Fixed)
# Builds the updated WebP library using Visual Studio generator
# Date: January 6, 2026

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
    Write-Host "Configuring libwebp with CMake (Visual Studio generator)..." -ForegroundColor Cyan
    
    # Use Visual Studio generator instead of NMake
    $cmakeArgs = @(
        "..",
        "-G", "Visual Studio 18 2026",
        "-A", "x64",
        "-DCMAKE_BUILD_TYPE=Release",
        "-DWEBP_BUILD_ANIM_UTILS=OFF",
        "-DWEBP_BUILD_CWEBP=OFF",
        "-DWEBP_BUILD_DWEBP=OFF",
        "-DWEBP_BUILD_GIF2WEBP=OFF",
        "-DWEBP_BUILD_IMG2WEBP=OFF",
        "-DWEBP_BUILD_VWEBP=OFF",
        "-DWEBP_BUILD_WEBPINFO=OFF",
        "-DWEBP_BUILD_WEBPMUX=OFF",
        "-DWEBP_BUILD_EXTRAS=OFF",
        "-DBUILD_SHARED_LIBS=OFF",
        "-DCMAKE_C_FLAGS_RELEASE=/MT /O2 /DNDEBUG",
        "-DCMAKE_CXX_FLAGS_RELEASE=/MT /O2 /DNDEBUG"
    )
    
    & cmake $cmakeArgs
    
    if ($LASTEXITCODE -ne 0) {
        throw "CMake configuration failed with exit code: $LASTEXITCODE"
    }
    
    Write-Host "Building libwebp with MSBuild..." -ForegroundColor Cyan
    
    # Find MSBuild
    $msbuildPath = & "$rootDir\build-scripts\Find-MSBuild.ps1"
    if (-not $msbuildPath) {
        throw "MSBuild not found"
    }
    
    Write-Host "Using MSBuild: $msbuildPath" -ForegroundColor Gray
    
    # Build ALL target to build all libraries at once
    & $msbuildPath "ALL_BUILD.vcxproj" /p:Configuration=Release /p:Platform=x64 /m /v:minimal /nologo
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "⚠️  ALL_BUILD failed, trying individual targets..." -ForegroundColor Yellow
        
        # Try building webp target specifically
        & $msbuildPath "webp.vcxproj" /p:Configuration=Release /p:Platform=x64 /v:minimal /nologo
        
        if ($LASTEXITCODE -ne 0) {
            throw "Build failed with exit code: $LASTEXITCODE"
        }
    }
    
    Write-Host "`n=== libwebp 1.5.0 Build Complete ===" -ForegroundColor Green
    
    # Check for built libraries
    $releaseDir = "Release"
    $builtLibs = @()
    
    if (Test-Path "$releaseDir\webp.lib") {
        $size = (Get-Item "$releaseDir\webp.lib").Length
        Write-Host "✅ webp.lib: $($size.ToString('N0')) bytes" -ForegroundColor Green
        $builtLibs += "webp.lib"
    } else {
        Write-Host "⚠️  webp.lib not found in $releaseDir" -ForegroundColor Yellow
    }
    
    if (Test-Path "$releaseDir\sharpyuv.lib") {
        $size = (Get-Item "$releaseDir\sharpyuv.lib").Length
        Write-Host "✅ sharpyuv.lib: $($size.ToString('N0')) bytes" -ForegroundColor Green
        $builtLibs += "sharpyuv.lib"
    }
    
    if (Test-Path "$releaseDir\libsharpyuv.lib") {
        $size = (Get-Item "$releaseDir\libsharpyuv.lib").Length
        Write-Host "✅ libsharpyuv.lib: $($size.ToString('N0')) bytes" -ForegroundColor Green
        $builtLibs += "libsharpyuv.lib"
    }
    
    if ($builtLibs.Count -eq 0) {
        throw "No libraries were built!"
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
