#Requires -Version 7.0
# Build LZMA SDK 24.08 static library for Windows x64

$ErrorActionPreference = "Stop"

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "Building LZMA SDK 24.08" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

$rootDir = Split-Path -Parent $PSScriptRoot
$lzmaDir = Join-Path $rootDir "external\compression\lzma-24.08\C"
$buildDir = Join-Path $lzmaDir "build-vs"
$installDir = Join-Path $rootDir "external\compression\build-libs\lzma"

if (-not (Test-Path $lzmaDir)) {
    Write-Host "❌ ERROR: LZMA source directory not found: $lzmaDir" -ForegroundColor Red
    exit 1
}

Write-Host "Source:  $lzmaDir" -ForegroundColor Gray
Write-Host "Build:   $buildDir" -ForegroundColor Gray
Write-Host "Install: $installDir`n" -ForegroundColor Gray

# Clean previous build
if (Test-Path $buildDir) {
    Write-Host "[1/5] Cleaning previous build..." -ForegroundColor Yellow
    Remove-Item $buildDir -Recurse -Force
}
New-Item -ItemType Directory -Path $buildDir -Force | Out-Null
Write-Host "  ✓ Build directory created" -ForegroundColor Green

# Configure with CMake
Write-Host "`n[2/5] Configuring with CMake..." -ForegroundColor Yellow

# Setup MSVC environment first
$vcvarsPath = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"
$vcvarsCmd = @"
@echo off
call "$vcvarsPath" x64 > nul
set
"@

# Execute vcvarsall and capture environment variables
$tempBat = [System.IO.Path]::GetTempFileName() + ".bat"
$vcvarsCmd | Out-File -FilePath $tempBat -Encoding ASCII
$envVars = & cmd /c $tempBat
Remove-Item $tempBat

# Apply environment variables to current session
foreach ($line in $envVars) {
    if ($line -match '^([^=]+)=(.*)$') {
        $name = $matches[1]
        $value = $matches[2]
        [System.Environment]::SetEnvironmentVariable($name, $value, [System.EnvironmentVariableTarget]::Process)
    }
}

Push-Location $buildDir
try {
    $cmakeArgs = @(
        ".."
        "-G", "NMake Makefiles"
        "-DCMAKE_BUILD_TYPE=Release"
        "-DCMAKE_C_COMPILER=cl.exe"
        "-DCMAKE_C_FLAGS_RELEASE=/MT /O2 /DNDEBUG"
        "-DCMAKE_INSTALL_PREFIX=$installDir"
    )
    
    & cmake $cmakeArgs
    
    if ($LASTEXITCODE -ne 0) {
        throw "CMake configuration failed"
    }
    Write-Host "  ✓ Configuration complete" -ForegroundColor Green
} catch {
    Write-Host "  ❌ CMake error: $_" -ForegroundColor Red
    Pop-Location
    exit 1
}

# Build
Write-Host "`n[3/5] Building..." -ForegroundColor Yellow
try {
    & nmake
    
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed"
    }
    Write-Host "  ✓ Build complete" -ForegroundColor Green
} catch {
    Write-Host "  ❌ Build error: $_" -ForegroundColor Red
    Pop-Location
    exit 1
}

# Install
Write-Host "`n[4/5] Installing..." -ForegroundColor Yellow
try {
    & nmake install
    
    if ($LASTEXITCODE -ne 0) {
        throw "Installation failed"
    }
    Write-Host "  ✓ Installation complete" -ForegroundColor Green
} catch {
    Write-Host "  ❌ Install error: $_" -ForegroundColor Red
    Pop-Location
    exit 1
}

Pop-Location

# Verify output
Write-Host "`n[5/5] Verifying build output..." -ForegroundColor Yellow
$libFile = Join-Path $installDir "lib\lzma.lib"
if (Test-Path $libFile) {
    $size = (Get-Item $libFile).Length / 1KB
    Write-Host "  ✓ lzma.lib ($([math]::Round($size, 0)) KB)" -ForegroundColor Green
} else {
    Write-Host "  ❌ lzma.lib not found at: $libFile" -ForegroundColor Red
    exit 1
}

# Check headers
$headerDir = Join-Path $installDir "include\lzma"
if (Test-Path $headerDir) {
    $headers = Get-ChildItem $headerDir -Filter "*.h"
    Write-Host "  ✓ $($headers.Count) header files installed" -ForegroundColor Green
} else {
    Write-Host "  ⚠ Headers not found (may not be critical)" -ForegroundColor Yellow
}

Write-Host "`n========================================" -ForegroundColor Green
Write-Host "✓ LZMA SDK 24.08 Build Complete" -ForegroundColor Green
Write-Host "========================================`n" -ForegroundColor Green

Write-Host "Library: $libFile" -ForegroundColor Cyan
Write-Host "Headers: $headerDir`n" -ForegroundColor Cyan

exit 0
