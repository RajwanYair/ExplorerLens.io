# Build lzma 24.08 static library
# Using NMake Makefiles generator for VS2026 BuildTools compatibility

$ErrorActionPreference = "Stop"
$ProgressPreference = "SilentlyContinue"

Write-Host "=== Building lzma 24.08 ===" -ForegroundColor Cyan

$ScriptRoot = Split-Path -Parent $PSScriptRoot
# Use XZ 5.6.3 which contains liblzma and has CMake support
$LzmaDir = Join-Path $ScriptRoot "external\compression\xz-5.6.3"
$BuildDir = Join-Path $LzmaDir "build-vs"

if (-not (Test-Path $LzmaDir)) {
    Write-Host "ERROR: XZ source directory not found: $LzmaDir" -ForegroundColor Red
    Write-Host "       Note: This script builds liblzma from the XZ Utils project" -ForegroundColor Yellow
    exit 1
}

# Clean previous build
Write-Host "Cleaning previous build..." -ForegroundColor Yellow
if (Test-Path $BuildDir) {
    Remove-Item -Recurse -Force $BuildDir -ErrorAction SilentlyContinue
}
New-Item -ItemType Directory -Path $BuildDir -Force | Out-Null

# Setup Visual Studio environment
Write-Host "Configuring lzma with CMake..." -ForegroundColor Yellow
$vcvarsPath = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"

$cmakeCommand = @"
call "$vcvarsPath" x64
cd /d "$BuildDir"
cmake .. -G "NMake Makefiles" ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_C_FLAGS_RELEASE="/MT /O2 /DNDEBUG" ^
    -DCMAKE_CXX_FLAGS_RELEASE="/MT /O2 /DNDEBUG" ^
    -DBUILD_SHARED_LIBS=OFF ^
    -DENABLE_NLS=OFF
"@

$cmakeCommand | cmd

if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: CMake configuration failed" -ForegroundColor Red
    exit 1
}

# Build with NMake
Write-Host "Building lzma with NMake..." -ForegroundColor Yellow

$nmakeCommand = @"
call "$vcvarsPath" x64
cd /d "$BuildDir"
nmake
"@

$nmakeCommand | cmd

if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: NMake build failed" -ForegroundColor Red
    exit 1
}

# Verify output
Write-Host "Verifying build output..." -ForegroundColor Yellow

# Check for liblzma.lib in various locations
$possibleLocations = @(
    (Join-Path $BuildDir "liblzma.lib"),
    (Join-Path $BuildDir "liblzma.a"),
    (Join-Path $BuildDir "Release\liblzma.lib"),
    (Join-Path $BuildDir "src\liblzma\liblzma.lib")
)

$outputLib = $null
foreach ($location in $possibleLocations) {
    if (Test-Path $location) {
        $outputLib = $location
        Write-Host "Found library at: $outputLib" -ForegroundColor Green
        break
    }
}

if ($outputLib) {
    $size = (Get-Item $outputLib).Length
    Write-Host "SUCCESS: liblzma library created - $size bytes" -ForegroundColor Green
    
    # Copy to standard Release directory
    $releaseDir = Join-Path $BuildDir "Release"
    if (-not (Test-Path $releaseDir)) {
        New-Item -ItemType Directory -Path $releaseDir -Force | Out-Null
    }
    
    $targetPath = Join-Path $releaseDir "liblzma.lib"
    if ($outputLib -ne $targetPath) {
        Copy-Item $outputLib -Destination $targetPath -Force
        Write-Host "Copied to: $targetPath" -ForegroundColor Green
    }
} else {
    Write-Host "ERROR: Output library not found" -ForegroundColor Red
    
    # List all .lib and .a files for debugging
    Write-Host "Available library files in build directory:" -ForegroundColor Yellow
    Get-ChildItem -Path $BuildDir -Recurse -Include "*.lib", "*.a" | ForEach-Object {
        Write-Host "  $($_.FullName) - $($_.Length) bytes" -ForegroundColor Gray
    }
    
    exit 1
}

Write-Host ""
Write-Host "=== lzma 24.08 build completed successfully ===" -ForegroundColor Green
