# Build libheif for HEIF/HEIC Support
# DarkThumbs External Library Build Script

param(
    [switch]$Clean,
    [string]$Configuration = "Release"
)

$ErrorActionPreference = "Stop"

# Paths
$repoRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$externalDir = Join-Path $repoRoot "external"
$libheifDir = Join-Path $externalDir "libheif"
$buildDir = Join-Path $libheifDir "build"
$installDir = Join-Path $externalDir "libheif-install"

Write-Host "================================================" -ForegroundColor Cyan
Write-Host "  Building libheif for HEIF/HEIC Support" -ForegroundColor Cyan
Write-Host "================================================" -ForegroundColor Cyan
Write-Host ""

# Clean if requested
if ($Clean -and (Test-Path $buildDir)) {
    Write-Host "Cleaning build directory..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force $buildDir
}

if ($Clean -and (Test-Path $installDir)) {
    Write-Host "Cleaning install directory..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force $installDir
}

# Clone libheif if not present
if (-not (Test-Path $libheifDir)) {
    Write-Host "Cloning libheif repository..." -ForegroundColor Green
    Push-Location $externalDir
    git clone --depth 1 --branch v1.19.5 https://github.com/strukturag/libheif.git
    Pop-Location
}

# Create build directory
if (-not (Test-Path $buildDir)) {
    New-Item -ItemType Directory -Path $buildDir | Out-Null
}

# Configure with CMake
Write-Host "Configuring libheif with CMake..." -ForegroundColor Green
Push-Location $buildDir

$cmakeArgs = @(
    "-G", "Visual Studio 17 2022",
    "-A", "x64",
    "-DCMAKE_INSTALL_PREFIX=$installDir",
    "-DCMAKE_BUILD_TYPE=$Configuration",
    "-DBUILD_SHARED_LIBS=OFF",  # Static library
    "-DWITH_EXAMPLES=OFF",      # No examples
    "-DWITH_GDK_PIXBUF=OFF",    # No pixbuf
    "-DWITH_LIBDE265=ON",       # HEVC decoder
    "-DWITH_X265=OFF",          # No encoder needed
    "-DWITH_DAV1D=ON",          # AVIF decoder (we have dav1d)
    "-DWITH_AOM_DECODER=OFF",   # Use dav1d instead
    "-DWITH_AOM_ENCODER=OFF",
    ".."
)

& cmake @cmakeArgs

if ($LASTEXITCODE -ne 0) {
    Write-Host "CMake configuration failed!" -ForegroundColor Red
    Pop-Location
    exit 1
}

# Build
Write-Host "Building libheif ($Configuration)..." -ForegroundColor Green
& cmake --build . --config $Configuration --parallel

if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed!" -ForegroundColor Red
    Pop-Location
    exit 1
}

# Install
Write-Host "Installing libheif to $installDir..." -ForegroundColor Green
& cmake --install . --config $Configuration

if ($LASTEXITCODE -ne 0) {
    Write-Host "Install failed!" -ForegroundColor Red
    Pop-Location
    exit 1
}

Pop-Location

# Copy binaries to external lib directories
$libDir = Join-Path $installDir "lib"
$includeDir = Join-Path $installDir "include"

Write-Host ""
Write-Host "================================================" -ForegroundColor Green
Write-Host "  libheif Build Complete!" -ForegroundColor Green
Write-Host "================================================" -ForegroundColor Green
Write-Host ""
Write-Host "Libraries:" -ForegroundColor Cyan
Get-ChildItem $libDir -Filter *.lib | ForEach-Object {
    Write-Host "  - $($_.Name)" -ForegroundColor White
}
Write-Host ""
Write-Host "Headers:" -ForegroundColor Cyan
Get-ChildItem $includeDir -Recurse -Filter *.h | Select-Object -First 5 | ForEach-Object {
    Write-Host "  - $($_.Name)" -ForegroundColor White
}
Write-Host "  ..." -ForegroundColor Gray
Write-Host ""
Write-Host "Install directory: $installDir" -ForegroundColor Yellow
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Cyan
Write-Host "1. Update Engine/CMakeLists.txt to include libheif" -ForegroundColor White
Write-Host "2. Uncomment HEIFDecoder.h/cpp in CMakeLists.txt" -ForegroundColor White
Write-Host "3. Implement HEIFDecoder::DecodeHEIFImage()" -ForegroundColor White
Write-Host "4. Link heif.lib in target_link_libraries" -ForegroundColor White
Write-Host ""
