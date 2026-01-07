# DarkThumbs - Complete Build Script (PowerShell)
# Builds CBXShell.dll and CBXManager.exe
# Date: November 19, 2025

param(
    [ValidateSet("Release", "Debug")]
    [string]$Configuration = "Release",
    
    [switch]$Clean,
    [switch]$Rebuild,
    [switch]$Verbose,
    [switch]$SkipTests
)

$ErrorActionPreference = "Stop"
$rootDir = Split-Path -Parent $PSScriptRoot
$slnFile = Join-Path $rootDir "CBXShell.sln"

Write-Host "`n=== DarkThumbs v5.0 Build Script ===" -ForegroundColor Green
Write-Host "Configuration: $Configuration" -ForegroundColor Cyan
Write-Host "Solution: $slnFile`n" -ForegroundColor Cyan

# Find MSBuild
function Find-MSBuild {
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    
    if (Test-Path $vswhere) {
        $vsPath = & $vswhere -latest -products * -requires Microsoft.Component.MSBuild -property installationPath
        if ($vsPath) {
            $msbuild = Join-Path $vsPath "MSBuild\Current\Bin\amd64\MSBuild.exe"
            if (Test-Path $msbuild) {
                return $msbuild
            }
        }
    }
    
    throw "MSBuild not found. Please install Visual Studio 2022 or Build Tools."
}

$msbuild = Find-MSBuild
Write-Host "Using MSBuild: $msbuild`n" -ForegroundColor White

# Determine build action
$target = "Build"
if ($Rebuild) {
    $target = "Rebuild"
}
elseif ($Clean) {
    $target = "Clean"
}

# MSBuild arguments
$msbuildArgs = @(
    $slnFile
    "/t:$target"
    "/p:Configuration=$Configuration"
    "/p:Platform=x64"
    "/p:CharacterSet=Unicode"
    "/p:WholeProgramOptimization=false"  # Disable LTCG
    "/m"  # Multi-core build
    "/nologo"
)

if ($Verbose) {
    $msbuildArgs += "/v:detailed"
} else {
    $msbuildArgs += "/v:minimal"
}

# Build
Write-Host "Building DarkThumbs ($target)..." -ForegroundColor Yellow
$buildStart = Get-Date

try {
    & $msbuild $msbuildArgs
    
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed with exit code $LASTEXITCODE"
    }
    
    $buildEnd = Get-Date
    $duration = $buildEnd - $buildStart
    
    Write-Host "`n=== Build Successful ===" -ForegroundColor Green
    Write-Host "Time: $($duration.TotalSeconds.ToString('F1')) seconds`n" -ForegroundColor Cyan
    
    # Display output file sizes
    $cbxShellDll = Join-Path $rootDir "CBXShell\x64\$Configuration\CBXShell.dll"
    $cbxManagerExe = Join-Path $rootDir "CBXManager\x64\$Configuration\CBXManager.exe"
    
    if (Test-Path $cbxShellDll) {
        $size = (Get-Item $cbxShellDll).Length
        Write-Host "CBXShell.dll: $($size.ToString('N0')) bytes ($([math]::Round($size/1024, 1)) KB)" -ForegroundColor White
    }
    
    if (Test-Path $cbxManagerExe) {
        $size = (Get-Item $cbxManagerExe).Length
        Write-Host "CBXManager.exe: $($size.ToString('N0')) bytes ($([math]::Round($size/1024, 1)) KB)" -ForegroundColor White
    }
    
    # Run tests if requested
    if (-not $SkipTests -and $Configuration -eq "Release") {
        Write-Host "`nRunning tests..." -ForegroundColor Yellow
        $testScript = Join-Path $rootDir "tests\run-all-tests.ps1"
        
        if (Test-Path $testScript) {
            & powershell -ExecutionPolicy Bypass -File $testScript
        } else {
            Write-Host "Test script not found (skipping)" -ForegroundColor Yellow
        }
    }
    
    Write-Host "`n=== Build Complete ===" -ForegroundColor Green
    exit 0
}
catch {
    Write-Host "`n=== Build Failed ===" -ForegroundColor Red
    Write-Host $_.Exception.Message -ForegroundColor Red
    exit 1
}
