# Build dav1d 1.5.1 AV1 decoder library for DarkThumbs
# PowerShell build script with MSVC toolchain

param(
    [string]$Configuration = "Release",
    [string]$Platform = "x64"
)

$ErrorActionPreference = "Stop"

Write-Host "Building dav1d 1.5.1 for DarkThumbs" -ForegroundColor Cyan
Write-Host "Configuration: $Configuration" -ForegroundColor Yellow
Write-Host "Platform: $Platform" -ForegroundColor Yellow
Write-Host ""

# Setup paths
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$RootDir = Split-Path -Parent $ScriptDir
$ExternalDir = Join-Path $RootDir "external"
$Dav1dDir = Join-Path $ExternalDir "dav1d-1.5.1"
$BuildDir = Join-Path $Dav1dDir "build-msvc"
$InstallDir = Join-Path $ExternalDir "image-libs\dav1d"

# Verify source exists
if (-not (Test-Path $Dav1dDir)) {
    Write-Host "[ERROR] dav1d source not found at: $Dav1dDir" -ForegroundColor Red
    Write-Host "Please run download-all-libs.ps1 first." -ForegroundColor Yellow
    exit 1
}

# Check for meson and ninja
$HasMeson = Get-Command meson -ErrorAction SilentlyContinue
$HasNinja = Get-Command ninja -ErrorAction SilentlyContinue

if (-not $HasMeson) {
    Write-Host "[INFO] Meson not found. Installing via pip..." -ForegroundColor Yellow
    python -m pip install meson --proxy http://proxy-dmz.intel.com:912
}

if (-not $HasNinja) {
    Write-Host "[INFO] Ninja not found. Installing via pip..." -ForegroundColor Yellow
    python -m pip install ninja --proxy http://proxy-dmz.intel.com:912
}

# Find Visual Studio
$VsPath = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools"
$VsDevCmd = Join-Path $VsPath "Common7\Tools\VsDevCmd.bat"

if (Test-Path $VsDevCmd) {
    Write-Host "[OK] Found Visual Studio BuildTools at: $VsPath" -ForegroundColor Green
}
else {
    # Try vswhere as fallback
    $VsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $VsWhere) {
        $VsPath = & $VsWhere -latest -property installationPath
        if ($VsPath) {
            $VsDevCmd = Join-Path $VsPath "Common7\Tools\VsDevCmd.bat"
        }
    }
    
    if (-not (Test-Path $VsDevCmd)) {
        Write-Host "[ERROR] Visual Studio not found" -ForegroundColor Red
        exit 1
    }
}

# Create build directory
if (Test-Path $BuildDir) {
    Write-Host "Cleaning existing build directory..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force $BuildDir
}
New-Item -ItemType Directory -Path $BuildDir | Out-Null

# Create build script that runs in VS environment
$MesonExe = "C:\Users\ryair\AppData\Local\Python\pythoncore-3.14-64\Scripts\meson.exe"
$NinjaExe = "C:\Users\ryair\AppData\Local\Python\pythoncore-3.14-64\Scripts\ninja.exe"

$BuildScriptContent = @"
@echo off
call "$VsDevCmd" -arch=amd64 -host_arch=amd64

cd /d "$Dav1dDir"

"$MesonExe" setup build-msvc --buildtype=release --default-library=static --prefix="$InstallDir" -Denable_asm=true -Denable_tools=false -Denable_examples=false -Denable_tests=false

cd build-msvc
"$NinjaExe"
"$NinjaExe" install

echo [OK] dav1d build complete
"@

$TempBuildScript = Join-Path $env:TEMP "build-dav1d-temp.cmd"
$BuildScriptContent | Out-File -FilePath $TempBuildScript -Encoding ASCII

# Execute build
Write-Host "Configuring and building dav1d..." -ForegroundColor Yellow
& cmd /c $TempBuildScript

# Verify build
$LibPath = Join-Path $InstallDir "lib\dav1d.lib"
if (Test-Path $LibPath) {
    Write-Host ""
    Write-Host "[SUCCESS] dav1d built successfully!" -ForegroundColor Green
    Write-Host "Library: $LibPath" -ForegroundColor Green
    Write-Host "Headers: $(Join-Path $InstallDir 'include')" -ForegroundColor Green
    exit 0
}
else {
    Write-Host ""
    Write-Host "[ERROR] Build failed - library not found" -ForegroundColor Red
    exit 1
}
