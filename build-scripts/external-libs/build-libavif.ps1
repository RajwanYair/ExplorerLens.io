# Build libavif 1.3.0 for DarkThumbs
# PowerShell build script with CMake and MSVC

param(
    [string]$Configuration = "Release",
    [string]$Platform = "x64"
)

$ErrorActionPreference = "Stop"

Write-Host "Building libavif 1.3.0 for DarkThumbs" -ForegroundColor Cyan
Write-Host "Configuration: $Configuration" -ForegroundColor Yellow
Write-Host "Platform: $Platform" -ForegroundColor Yellow
Write-Host ""

# Setup paths
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$RootDir = Split-Path -Parent (Split-Path -Parent $ScriptDir)
$ExternalDir = Join-Path $RootDir "external"
$LibavifDir = Join-Path $ExternalDir "image-libs\libavif-1.3.0"
$Dav1dDir = Join-Path $ExternalDir "image-libs\dav1d-1.5.1\install"
$BuildDir = Join-Path $LibavifDir "build-msvc"
$InstallDir = Join-Path $LibavifDir "install"

# Verify source exists
if (-not (Test-Path $LibavifDir)) {
    Write-Host "[ERROR] libavif source not found at: $LibavifDir" -ForegroundColor Red
    Write-Host "Please run download-all-libs.ps1 first." -ForegroundColor Yellow
    exit 1
}

# Verify dav1d is built
if (-not (Test-Path (Join-Path $Dav1dDir "lib\dav1d.lib"))) {
    Write-Host "[ERROR] dav1d not built. Please run build-dav1d.ps1 first." -ForegroundColor Red
    exit 1
}

# Find Visual Studio and CMake
$VsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $VsWhere) {
    $VsPath = & $VsWhere -latest -property installationPath
    Write-Host "[OK] Found Visual Studio at: $VsPath" -ForegroundColor Green
} else {
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

# Configure with CMake
Write-Host "Configuring libavif with CMake..." -ForegroundColor Yellow
Push-Location $BuildDir

try {
    & cmake .. `
        -G "Visual Studio 17 2022" `
        -A x64 `
        -DCMAKE_BUILD_TYPE=Release `
        -DCMAKE_INSTALL_PREFIX="$InstallDir" `
        -DBUILD_SHARED_LIBS=OFF `
        -DAVIF_CODEC_DAV1D=ON `
        -DAVIF_LOCAL_DAV1D=OFF `
        -Ddav1d_DIR="$Dav1dDir" `
        -DAVIF_CODEC_AOM=OFF `
        -DAVIF_BUILD_APPS=OFF `
        -DAVIF_BUILD_TESTS=OFF `
        -DAVIF_BUILD_EXAMPLES=OFF `
        -DAVIF_ENABLE_EXPERIMENTAL_GAIN_MAP=ON `
        -DAVIF_ENABLE_EXPERIMENTAL_SAMPLE_TRANSFORM=ON
    
    if ($LASTEXITCODE -ne 0) {
        throw "CMake configuration failed"
    }
    
    Write-Host "[OK] CMake configuration complete" -ForegroundColor Green
    
    # Build
    Write-Host "Building libavif..." -ForegroundColor Yellow
    & cmake --build . --config Release --target install
    
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed"
    }
    
    Write-Host "[OK] Build complete" -ForegroundColor Green
} catch {
    Write-Host "[ERROR] $($_.Exception.Message)" -ForegroundColor Red
    Pop-Location
    exit 1
} finally {
    Pop-Location
}

# Verify build
$LibPath = Join-Path $InstallDir "lib\avif.lib"
if (Test-Path $LibPath) {
    Write-Host ""
    Write-Host "[SUCCESS] libavif built successfully!" -ForegroundColor Green
    Write-Host "Library: $LibPath" -ForegroundColor Green
    Write-Host "Headers: $(Join-Path $InstallDir 'include')" -ForegroundColor Green
    Write-Host ""
    Write-Host "Features enabled:" -ForegroundColor Cyan
    Write-Host "  - DAV1D decoder (AV1)" -ForegroundColor White
    Write-Host "  - Gain Map API (HDR)" -ForegroundColor White
    Write-Host "  - Sample Transform" -ForegroundColor White
    exit 0
} else {
    Write-Host ""
    Write-Host "[ERROR] Build failed - library not found" -ForegroundColor Red
    exit 1
}
