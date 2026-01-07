# DarkThumbs - Build zstd 1.5.7
# Builds the updated zstd library
# Date: November 19, 2025

$ErrorActionPreference = "Stop"

Write-Host "`n=== Building zstd 1.5.7 ===" -ForegroundColor Green

$rootDir = Split-Path -Parent $PSScriptRoot
$zstdDir = Join-Path $rootDir "external\compression\zstd-1.5.7"

if (-not (Test-Path $zstdDir)) {
    Write-Host "ERROR: zstd-1.5.7 not found at $zstdDir" -ForegroundColor Red
    exit 1
}

Push-Location $zstdDir

# Setup Visual Studio environment
$vcvarsPath = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
if (-not (Test-Path $vcvarsPath)) {
    $vcvarsPath = "C:\Program Files\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
}

# Create build directory
$buildDir = "build-vs"
if (Test-Path $buildDir) {
    Write-Host "Cleaning previous build..." -ForegroundColor Yellow
    Remove-Item $buildDir -Recurse -Force
}

New-Item -ItemType Directory -Force -Path $buildDir | Out-Null
Push-Location $buildDir

try {
    Write-Host "Configuring zstd with CMake..." -ForegroundColor Cyan
    
    $cmakeCmd = @"
call "$vcvarsPath" && cmake ..\build\cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DZSTD_BUILD_SHARED=OFF -DZSTD_BUILD_STATIC=ON -DZSTD_BUILD_PROGRAMS=OFF -DZSTD_BUILD_TESTS=OFF -DCMAKE_C_FLAGS_RELEASE="/MT /O2" -DCMAKE_CXX_FLAGS_RELEASE="/MT /O2"
"@
    
    cmd /c $cmakeCmd
    
    if ($LASTEXITCODE -ne 0) {
        throw "CMake configuration failed"
    }
    
    Write-Host "Building zstd with NMake..." -ForegroundColor Cyan
    cmd /c "call `"$vcvarsPath`" && nmake libzstd_static"
    
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed"
    }
    
    # Copy output to standard location
    $outputDir = Join-Path $zstdDir "..\zstd-1.5.7\build\x64\Release"
    New-Item -ItemType Directory -Force -Path $outputDir | Out-Null
    
    # Find and copy the built library
    if (Test-Path "lib\zstd_static.lib") {
        Copy-Item "lib\zstd_static.lib" $outputDir -Force
        Write-Host "`n=== zstd 1.5.7 Build Complete ===" -ForegroundColor Green
        
        $size = (Get-Item "$outputDir\zstd_static.lib").Length
        Write-Host "zstd_static.lib: $($size.ToString('N0')) bytes ($([math]::Round($size/1024, 0)) KB)" -ForegroundColor White
    } else {
        Write-Host "Warning: Could not find zstd_static.lib" -ForegroundColor Yellow
    }
    
} catch {
    Write-Host "`nBuild Failed: $($_.Exception.Message)" -ForegroundColor Red
    Pop-Location
    Pop-Location
    exit 1
}

Pop-Location
Pop-Location

Write-Host "`nzstd 1.5.7 ready!" -ForegroundColor Cyan
