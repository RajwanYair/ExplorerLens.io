# ===========================================================================
# Build-MinizipNG.ps1
# Build minizip-ng 4.0.10 - Modern ZIP Archive Library
# ===========================================================================

param(
    [string]$Configuration = "Release",
    [string]$Platform = "x64"
)

$ErrorActionPreference = "Stop"

Write-Host ""
Write-Host "==========================================================================" -ForegroundColor Cyan
Write-Host "Building minizip-ng 4.0.10 (Modern ZIP Library)" -ForegroundColor Cyan
Write-Host "==========================================================================" -ForegroundColor Cyan
Write-Host ""

# Get project root
$ProjectRoot = Split-Path -Parent $PSScriptRoot
Set-Location $ProjectRoot

# Find CMake
$CMake = Get-Command cmake -ErrorAction SilentlyContinue
if (-not $CMake) {
    Write-Host "ERROR: CMake not found. Please install CMake." -ForegroundColor Red
    exit 1
}

$SourceDir = Join-Path $ProjectRoot "external\compression\minizip-ng-4.0.10"
$BuildDir = Join-Path $SourceDir "build-vs"

if (-not (Test-Path $SourceDir)) {
    Write-Host "ERROR: Source directory not found!" -ForegroundColor Red
    Write-Host "Expected: $SourceDir" -ForegroundColor Yellow
    exit 1
}

# Create build directory
if (-not (Test-Path $BuildDir)) {
    Write-Host "Creating build directory..." -ForegroundColor White
    New-Item -ItemType Directory -Path $BuildDir -Force | Out-Null
}

Set-Location $BuildDir

Write-Host "Source: $SourceDir" -ForegroundColor White
Write-Host "Build:  $BuildDir" -ForegroundColor White
Write-Host ""

# Configure with CMake
Write-Host "Configuring with CMake..." -ForegroundColor White
& cmake $SourceDir `
    -G "Visual Studio 17 2022" `
    -A x64 `
    -DBUILD_SHARED_LIBS=OFF `
    -DMZ_COMPAT=OFF `
    -DMZ_ZLIB=ON `
    -DMZ_BZIP2=ON `
    -DMZ_LZMA=ON `
    -DMZ_ZSTD=ON `
    -DMZ_OPENSSL=OFF `
    -DMZ_LIBCOMP=OFF `
    -DMZ_FETCH_LIBS=OFF

if ($LASTEXITCODE -ne 0) {
    Write-Host ""
    Write-Host "[FAILED] CMake configuration failed!" -ForegroundColor Red
    exit 1
}

# Build
Write-Host ""
Write-Host "Building $Configuration configuration..." -ForegroundColor White
& cmake --build . --config $Configuration --target minizip

if ($LASTEXITCODE -ne 0) {
    Write-Host ""
    Write-Host "[FAILED] Build failed!" -ForegroundColor Red
    exit 1
}

# Check for output
$OutputLib = Join-Path $BuildDir "$Configuration\minizip.lib"

if (Test-Path $OutputLib) {
    $Size = (Get-Item $OutputLib).Length
    Write-Host ""
    Write-Host "[SUCCESS] minizip.lib built: $Size bytes" -ForegroundColor Green
    Write-Host ""
    Write-Host "Output: $OutputLib" -ForegroundColor White
    Write-Host ""
    Write-Host "Next Steps:" -ForegroundColor Yellow
    Write-Host "1. Copy minizip.lib to external\compression\minizip-ng\x64\Release\minizip-ng.lib" -ForegroundColor Gray
    Write-Host "2. Update CBXShell.vcxproj AdditionalLibraryDirectories if needed" -ForegroundColor Gray
    exit 0
}
else {
    Write-Host ""
    Write-Host "[ERROR] Output file not found: $OutputLib" -ForegroundColor Red
    Write-Host "Build directory contents:" -ForegroundColor Yellow
    Get-ChildItem $BuildDir -Recurse -Filter "*.lib" | Format-Table FullName, Length
    exit 1
}
