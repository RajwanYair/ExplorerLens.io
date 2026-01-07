# Build libjxl 0.11.1 (JPEG XL) for DarkThumbs
# PowerShell build script with CMake and MSVC

param(
    [string]$Configuration = "Release",
    [string]$Platform = "x64"
)

$ErrorActionPreference = "Stop"

Write-Host "Building libjxl 0.11.1 for DarkThumbs" -ForegroundColor Cyan
Write-Host "Configuration: $Configuration" -ForegroundColor Yellow
Write-Host "Platform: $Platform" -ForegroundColor Yellow
Write-Host ""

# Setup paths
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$RootDir = Split-Path -Parent $ScriptDir
$ExternalDir = Join-Path $RootDir "external"
$LibjxlDir = Join-Path $ExternalDir "image-libs\libjxl-0.11.1"
$BuildDir = Join-Path $LibjxlDir "build-msvc"
$OutputDir = Join-Path $LibjxlDir "x64\Release"

# Verify source exists
if (-not (Test-Path $LibjxlDir)) {
    Write-Host "[ERROR] libjxl source not found at: $LibjxlDir" -ForegroundColor Red
    Write-Host "Please run download-all-libs.ps1 first." -ForegroundColor Yellow
    exit 1
}

# Find Visual Studio and CMake
$VsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $VsWhere) {
    $VsPath = & $VsWhere -latest -property installationPath
    Write-Host "[OK] Found Visual Studio at: $VsPath" -ForegroundColor Green
}
else {
    Write-Host "[ERROR] Visual Studio not found" -ForegroundColor Red
    exit 1
}

# Check for CMake
$CMake = Get-Command cmake -ErrorAction SilentlyContinue
if (-not $CMake) {
    Write-Host "[ERROR] CMake not found in PATH" -ForegroundColor Red
    exit 1
}

# Create build directory
if (Test-Path $BuildDir) {
    Write-Host "Cleaning existing build directory..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force $BuildDir
}
New-Item -ItemType Directory -Path $BuildDir | Out-Null

# libjxl requires submodules - initialize them
Write-Host "Initializing git submodules..." -ForegroundColor Yellow
Push-Location $LibjxlDir
try {
    if (Test-Path ".git") {
        & git submodule update --init --recursive
    }
    else {
        Write-Host "[WARNING] Not a git repository, submodules may be missing" -ForegroundColor Yellow
        Write-Host "Continuing anyway - bundled dependencies will be used" -ForegroundColor Yellow
    }
}
catch {
    Write-Host "[WARNING] Git submodule init failed: $($_.Exception.Message)" -ForegroundColor Yellow
}
finally {
    Pop-Location
}

# Configure with CMake
Write-Host "Configuring libjxl with CMake..." -ForegroundColor Yellow
Push-Location $BuildDir

try {
    & cmake .. `
        -G "Visual Studio 17 2022" `
        -A x64 `
        -DCMAKE_BUILD_TYPE=Release `
        -DCMAKE_INSTALL_PREFIX="$InstallDir" `
        -DBUILD_SHARED_LIBS=OFF `
        -DBUILD_TESTING=OFF `
        -DJPEGXL_ENABLE_TOOLS=OFF `
        -DJPEGXL_ENABLE_DOXYGEN=OFF `
        -DJPEGXL_ENABLE_MANPAGES=OFF `
        -DJPEGXL_ENABLE_BENCHMARK=OFF `
        -DJPEGXL_ENABLE_EXAMPLES=OFF `
        -DJPEGXL_ENABLE_JNI=OFF `
        -DJPEGXL_ENABLE_SJPEG=OFF `
        -DJPEGXL_ENABLE_OPENEXR=OFF `
        -DJPEGXL_ENABLE_SKCMS=ON `
        -DJPEGXL_FORCE_SYSTEM_BROTLI=OFF `
        -DJPEGXL_FORCE_SYSTEM_HWY=OFF
    
    if ($LASTEXITCODE -ne 0) {
        throw "CMake configuration failed"
    }
    
    Write-Host "[OK] CMake configuration complete" -ForegroundColor Green
    
    # Build
    Write-Host "Building libjxl (this may take several minutes)..." -ForegroundColor Yellow
    & cmake --build . --config Release --target install --parallel
    
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed"
    }
    
    Write-Host "[OK] Build complete" -ForegroundColor Green
}
catch {
    Write-Host "[ERROR] $($_.Exception.Message)" -ForegroundColor Red
    Pop-Location
    exit 1
}
finally {
    Pop-Location
}

# Verify build
$LibPath = Join-Path $InstallDir "lib\jxl.lib"
$LibDecodePath = Join-Path $InstallDir "lib\jxl_dec.lib"

if ((Test-Path $LibPath) -or (Test-Path $LibDecodePath)) {
    Write-Host ""
    Write-Host "[SUCCESS] libjxl built successfully!" -ForegroundColor Green
    if (Test-Path $LibPath) {
        Write-Host "Library: $LibPath" -ForegroundColor Green
    }
    if (Test-Path $LibDecodePath) {
        Write-Host "Decoder: $LibDecodePath" -ForegroundColor Green
    }
    Write-Host "Headers: $(Join-Path $InstallDir 'include')" -ForegroundColor Green
    Write-Host ""
    Write-Host "Features enabled:" -ForegroundColor Cyan
    Write-Host "  - JPEG XL decoding" -ForegroundColor White
    Write-Host "  - SKCMS color management" -ForegroundColor White
    Write-Host "  - Bundled Brotli and Highway" -ForegroundColor White
    exit 0
}
else {
    Write-Host ""
    Write-Host "[ERROR] Build failed - library not found" -ForegroundColor Red
    exit 1
}
