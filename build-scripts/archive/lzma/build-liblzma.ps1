# ===========================================================================
# build-liblzma.ps1
# Build liblzma from XZ Utils for LZMA-compressed ZIP support
# ===========================================================================

$ErrorActionPreference = "Stop"
$ProjectRoot = Split-Path -Parent $PSScriptRoot

Write-Host "==========================================================================" -ForegroundColor Cyan
Write-Host "Building liblzma (XZ Utils 5.6.3)" -ForegroundColor Cyan
Write-Host "==========================================================================" -ForegroundColor Cyan
Write-Host ""

$XZ_ROOT = "$ProjectRoot\external\compression\xz-5.6.3"
$BUILD_DIR = "$XZ_ROOT\build-vs"

if (-not (Test-Path $XZ_ROOT)) {
    Write-Host "[FAIL] XZ Utils not found at: $XZ_ROOT" -ForegroundColor Red
    Write-Host "Please download XZ Utils 5.6.3 from: https://github.com/tukaani-project/xz/releases" -ForegroundColor Yellow
    exit 1
}

Write-Host "Source directory: $XZ_ROOT" -ForegroundColor Cyan
Write-Host "Build directory:  $BUILD_DIR" -ForegroundColor Cyan
Write-Host ""

# Create build directory
if (Test-Path $BUILD_DIR) {
    Write-Host "Cleaning existing build directory..." -ForegroundColor Yellow
    Remove-Item $BUILD_DIR -Recurse -Force
}
New-Item -ItemType Directory -Path $BUILD_DIR -Force | Out-Null

# Find CMake
$cmake = Get-Command cmake.exe -ErrorAction SilentlyContinue
if (-not $cmake) {
    Write-Host "[FAIL] CMake not found in PATH" -ForegroundColor Red
    Write-Host "Please install CMake from: https://cmake.org/download/" -ForegroundColor Yellow
    exit 1
}

Write-Host "[OK] Found CMake: $($cmake.Source)" -ForegroundColor Green
Write-Host ""

# Configure with CMake
Write-Host "Configuring liblzma with CMake..." -ForegroundColor Yellow
Push-Location $BUILD_DIR
try {
    # Configure for static library, Release build, x64
    & cmake .. `
        -G "Visual Studio 18 2026" `
        -A x64 `
        -DCMAKE_BUILD_TYPE=Release `
        -DBUILD_SHARED_LIBS=OFF `
        -DENABLE_THREADS=ON `
        -DENABLE_ENCODERS=ON `
        -DENABLE_DECODERS=ON `
        -DENABLE_LZMA1=ON `
        -DENABLE_LZMA2=ON `
        -DENABLE_DELTA=ON `
        -DENABLE_BCJ=ON `
        -DCMAKE_INSTALL_PREFIX="$BUILD_DIR\install"
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "[FAIL] CMake configuration failed" -ForegroundColor Red
        exit 1
    }
    
    Write-Host "[OK] CMake configuration complete" -ForegroundColor Green
    Write-Host ""
    
    # Build
    Write-Host "Building liblzma..." -ForegroundColor Yellow
    & cmake --build . --config Release --target liblzma
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "[FAIL] Build failed" -ForegroundColor Red
        exit 1
    }
    
    Write-Host "[OK] Build complete" -ForegroundColor Green
    Write-Host ""
    
    # Check for output
    $libFile = Get-ChildItem -Path $BUILD_DIR -Recurse -Filter "liblzma.lib" -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($libFile) {
        $sizeKB = [math]::Round($libFile.Length / 1KB, 2)
        Write-Host "==========================================================================" -ForegroundColor Green
        Write-Host "[OK] liblzma.lib built successfully!" -ForegroundColor Green
        Write-Host "==========================================================================" -ForegroundColor Green
        Write-Host "Location: $($libFile.FullName)" -ForegroundColor Cyan
        Write-Host "Size:     $sizeKB KB" -ForegroundColor Cyan
        Write-Host ""
        Write-Host "Next Steps:" -ForegroundColor Yellow
        Write-Host "  1. Add to CBXShell.vcxproj:" -ForegroundColor White
        Write-Host "     <AdditionalLibraryDirectories>..\external\compression\xz-5.6.3\build-vs\Release</AdditionalLibraryDirectories>" -ForegroundColor Gray
        Write-Host "     <AdditionalDependencies>liblzma.lib</AdditionalDependencies>" -ForegroundColor Gray
        Write-Host "  2. Enable HAVE_LZMA in minizip-ng configuration" -ForegroundColor White
        Write-Host "  3. Rebuild CBXShell to enable LZMA-compressed ZIP support" -ForegroundColor White
    }
    else {
        Write-Host "[WARN] liblzma.lib not found - check build output" -ForegroundColor Yellow
        Write-Host "Searching for library files..." -ForegroundColor Cyan
        Get-ChildItem -Path $BUILD_DIR -Recurse -Filter "*.lib" | ForEach-Object {
            Write-Host "  Found: $($_.FullName) ($([math]::Round($_.Length/1KB, 2)) KB)" -ForegroundColor Gray
        }
    }
}
finally {
    Pop-Location
}

Write-Host ""
Write-Host "[OK] Done!" -ForegroundColor Green
