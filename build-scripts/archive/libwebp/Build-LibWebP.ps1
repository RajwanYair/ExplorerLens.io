# ===========================================================================
# Build-LibWebP.ps1
# Builds libwebp for DarkThumbs (x64 Static Release)
# ===========================================================================

param(
    [string]$Configuration = "Release",
    [string]$Platform = "x64"
)

$ErrorActionPreference = "Stop"

Write-Host ""
Write-Host "==========================================================================" -ForegroundColor Cyan
Write-Host " Building libwebp 1.5.0 (x64 Static Release)" -ForegroundColor Cyan
Write-Host "==========================================================================" -ForegroundColor Cyan
Write-Host ""

# Get project root
$ProjectRoot = Split-Path -Parent $PSScriptRoot
Set-Location $ProjectRoot

# Check for libwebp source
$WebpSrc = Join-Path $ProjectRoot "external\image-libs\libwebp-1.5.0"
if (-not (Test-Path $WebpSrc)) {
    Write-Host "ERROR: libwebp source not found" -ForegroundColor Red
    Write-Host "       Run: build-scripts\Download-ImageLibs.ps1" -ForegroundColor Yellow
    exit 1
}

Write-Host "[1/3] Creating Visual Studio project for libwebp..." -ForegroundColor Green

# Create build directory
$WebpBuild = Join-Path $WebpSrc "build-vs"
if (-not (Test-Path $WebpBuild)) {
    New-Item -ItemType Directory -Path $WebpBuild | Out-Null
}

Set-Location $WebpSrc

# Check for CMake
if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    Write-Host "WARNING: CMake not found in PATH" -ForegroundColor Yellow
    Write-Host "         Installing via winget..." -ForegroundColor Yellow
    winget install Kitware.CMake
    if ($LASTEXITCODE -ne 0) {
        Write-Host "ERROR: Failed to install CMake" -ForegroundColor Red
        Write-Host "       Please install manually: https://cmake.org/download/" -ForegroundColor Yellow
        exit 1
    }
}

Write-Host "[2/3] Generating build files with Ninja..." -ForegroundColor Green

# Setup Visual Studio environment
$VsPath = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
if (Test-Path $VsPath) {
    cmd /c "`"$VsPath`" && set" | ForEach-Object {
        if ($_ -match "^(.*?)=(.*)$") {
            [System.Environment]::SetEnvironmentVariable($matches[1], $matches[2])
        }
    }
}

# Configure with CMake
cmake -B "$WebpBuild" -G "Ninja" `
    -DCMAKE_BUILD_TYPE=Release `
    -DBUILD_SHARED_LIBS=OFF `
    -DWEBP_BUILD_ANIM_UTILS=OFF `
    -DWEBP_BUILD_CWEBP=OFF `
    -DWEBP_BUILD_DWEBP=OFF `
    -DWEBP_BUILD_GIF2WEBP=OFF `
    -DWEBP_BUILD_IMG2WEBP=OFF `
    -DWEBP_BUILD_VWEBP=OFF `
    -DWEBP_BUILD_WEBPINFO=OFF `
    -DWEBP_BUILD_WEBPMUX=OFF `
    -DWEBP_BUILD_EXTRAS=OFF

if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: CMake generation failed" -ForegroundColor Red
    exit 1
}

Write-Host "[3/3] Building libwebp..." -ForegroundColor Green
cmake --build "$WebpBuild" --config Release

if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Build failed" -ForegroundColor Red
    exit 1
}

# Check output (Static lib)
$WebpLib = Join-Path $WebpBuild "libwebp.lib"
if (-not (Test-Path $WebpLib)) {
    $WebpLib = Join-Path $WebpBuild "Release\libwebp.lib"
}

if (Test-Path $WebpLib) {
    $WebpSize = (Get-Item $WebpLib).Length
    $WebpSizeKb = [math]::Round($WebpSize / 1024, 2)
    
    Write-Host ""
    Write-Host "==========================================================================" -ForegroundColor Green
    Write-Host " Build Successful (Static Library)" -ForegroundColor Green
    Write-Host "==========================================================================" -ForegroundColor Green
    Write-Host "  Lib:    $WebpLib" -ForegroundColor White
    Write-Host "  Size:   $WebpSizeKb KB" -ForegroundColor White
    Write-Host ""
    
    exit 0
}
else {
    Write-Host "ERROR: Output Lib not found" -ForegroundColor Red
    exit 1
}
