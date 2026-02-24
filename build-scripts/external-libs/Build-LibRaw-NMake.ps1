#Requires -Version 7.0
#================================================================
# Build-LibRaw-NMake.ps1
# ExplorerLens v7.0 - Build LibRaw 0.21.2 using MSVC nmake
# Refactored to use Build-Library-Core.ps1 module
# Date: February 18, 2026
#
# Directory structure (post-cleanup):
#   Project root:       <repo>\
#   This script:        <repo>\build-scripts\external-libs\Build-LibRaw-NMake.ps1
#   Core module:        <repo>\build-scripts\core\Build-Library-Core.ps1
#   LibRaw source:      <repo>\external\camera-libs\libraw\
#   Install dir:        <repo>\external\camera-libs\libraw-install\
#================================================================

param(
    [switch]$Clean
)

# Import core build module
. "$PSScriptRoot\..\core\Build-Library-Core.ps1"

# Paths — use double Split-Path to get project root from external-libs/
$rootDir = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$LibRawSource = Join-Path $rootDir "external\camera-libs\libraw"
$OutputDir = Join-Path $rootDir "external\camera-libs\libraw-install"

Write-BuildHeader "Building LibRaw with MSVC nmake"

if (!(Test-Path $LibRawSource)) {
    Write-BuildLog "LibRaw source not found at: $LibRawSource" -Level Error
    exit 1
}

Write-BuildLog "Source:  $LibRawSource" -Level Info
Write-BuildLog "Install: $OutputDir" -Level Info

# Clean if requested
if ($Clean) {
    Write-Host "Cleaning previous build..." -ForegroundColor Yellow
    Push-Location $LibRawSource
    if (Test-Path "lib") { Remove-Item "lib" -Recurse -Force }
    if (Test-Path "bin") { Remove-Item "bin" -Recurse -Force }
    if (Test-Path "object") { Remove-Item "object" -Recurse -Force }
    Pop-Location
    
    if (Test-Path $OutputDir) {
        Remove-Item $OutputDir -Recurse -Force
    }
}

# Setup VS environment
Write-Host "Setting up MSVC environment..." -ForegroundColor Cyan

# Try to find vcvars64.bat
$vcvarsPath = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

if (!(Test-Path $vcvarsPath)) {
    $vcvarsPath = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
}

if (!(Test-Path $vcvarsPath)) {
    Write-Host "ERROR: vcvars64.bat not found" -ForegroundColor Red
    Write-Host "Please install Visual Studio Build Tools" -ForegroundColor Red
    exit 1
}

Write-Host "  Using: $vcvarsPath" -ForegroundColor Gray

# Build LibRaw
Write-Host "Building LibRaw static library..." -ForegroundColor Cyan
Push-Location $LibRawSource

# Create a batch file to run nmake in VS environment
$batchScript = @"
@echo off
call "$vcvarsPath"
nmake /f Makefile.msvc lib\libraw_static.lib
if %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%
"@

$batchFile = [System.IO.Path]::GetTempFileName() + ".bat"
$batchScript | Out-File -FilePath $batchFile -Encoding ASCII

& cmd.exe /c $batchFile
$buildResult = $LASTEXITCODE
Remove-Item $batchFile -ErrorAction SilentlyContinue

Pop-Location

if ($buildResult -ne 0) {
    Write-Host ""
    Write-Host "================================================" -ForegroundColor Red
    Write-Host "  Build Failed!" -ForegroundColor Red
    Write-Host "================================================" -ForegroundColor Red
    exit 1
}

# Copy outputs to install directory
Write-Host "Installing LibRaw..." -ForegroundColor Cyan
New-Item -ItemType Directory -Path "$OutputDir\lib" -Force | Out-Null
New-Item -ItemType Directory -Path "$OutputDir\include\libraw" -Force | Out-Null

# Copy library
if (Test-Path "$LibRawSource\lib\libraw_static.lib") {
    Copy-Item "$LibRawSource\lib\libraw_static.lib" "$OutputDir\lib\" -Force
    Write-Host "  Copied libraw_static.lib" -ForegroundColor Green
} else {
    Write-Host "ERROR: libraw_static.lib not found" -ForegroundColor Red
    exit 1
}

# Copy headers
Copy-Item "$LibRawSource\libraw\*.h" "$OutputDir\include\libraw\" -Force
Write-Host "  Copied header files" -ForegroundColor Green

Write-Host ""
Write-Host "================================================" -ForegroundColor Green
Write-Host "  LibRaw Build Complete!" -ForegroundColor Green
Write-Host "================================================" -ForegroundColor Green
Write-Host "Library: $OutputDir\lib\libraw_static.lib" -ForegroundColor Green
Write-Host "Headers: $OutputDir\include\libraw\" -ForegroundColor Green
Write-Host ""

