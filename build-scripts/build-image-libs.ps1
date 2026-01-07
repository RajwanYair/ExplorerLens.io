# Build All Missing Image Libraries for DarkThumbs v5.1.0
# Builds: dav1d, libavif, libjxl and integrates them into the project

param(
    [switch]$SkipDav1d,
    [switch]$SkipLibavif,
    [switch]$SkipLibjxl,
    [switch]$Clean
)

$ErrorActionPreference = "Stop"

Write-Host "=== DarkThumbs Image Libraries Build Script ===" -ForegroundColor Cyan
Write-Host "Building: dav1d 1.5.1, libavif 1.3.0, libjxl 0.11.1" -ForegroundColor Yellow
Write-Host ""

$RootDir = "c:\Users\ryair\OneDrive - Intel Corporation\Documents\MyScripts\DarkThumbs"
$ExternalDir = Join-Path $RootDir "external"
$ImageLibsDir = Join-Path $ExternalDir "image-libs"

# Check prerequisites
Write-Host "[1/5] Checking prerequisites..." -ForegroundColor Cyan

$prereqMissing = $false

# Check CMake
if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    Write-Host "  [ERROR] CMake not found. Please install CMake 3.20+" -ForegroundColor Red
    $prereqMissing = $true
} else {
    Write-Host "  [OK] CMake found" -ForegroundColor Green
}

# Check Python (for meson/ninja)
if (-not (Get-Command python -ErrorAction SilentlyContinue)) {
    Write-Host "  [ERROR] Python not found. Required for dav1d build" -ForegroundColor Red
    $prereqMissing = $true
} else {
    Write-Host "  [OK] Python found" -ForegroundColor Green
}

# Check Visual Studio
$VsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $VsWhere) {
    $VsPath = & $VsWhere -latest -property installationPath
    Write-Host "  [OK] Visual Studio found: $VsPath" -ForegroundColor Green
} else {
    Write-Host "  [ERROR] Visual Studio not found" -ForegroundColor Red
    $prereqMissing = $true
}

if ($prereqMissing) {
    Write-Host ""
    Write-Host "Please install missing prerequisites and try again." -ForegroundColor Red
    exit 1
}

Write-Host ""

# Build dav1d (AV1 decoder - required for libavif)
if (-not $SkipDav1d) {
    Write-Host "[2/5] Building dav1d 1.5.1..." -ForegroundColor Cyan
    
    $Dav1dSrc = Join-Path $ImageLibsDir "dav1d-1.5.1"
    $Dav1dBuild = Join-Path $Dav1dSrc "build"
    $Dav1dInstall = Join-Path $ImageLibsDir "dav1d-install"
    
    if (-not (Test-Path $Dav1dSrc)) {
        Write-Host "  [ERROR] dav1d source not found at: $Dav1dSrc" -ForegroundColor Red
        exit 1
    }
    
    # Install meson/ninja if needed
    Write-Host "  Installing meson and ninja..." -ForegroundColor Gray
    python -m pip install --quiet meson ninja
    
    # Clean build directory
    if ($Clean -and (Test-Path $Dav1dBuild)) {
        Remove-Item -Recurse -Force $Dav1dBuild
    }
    
    # Build with meson
    Push-Location $Dav1dSrc
    try {
        Write-Host "  Configuring dav1d..." -ForegroundColor Gray
        meson setup $Dav1dBuild --buildtype=release --default-library=static --prefix=$Dav1dInstall -Denable_tools=false -Denable_tests=false 2>&1 | Out-Null
        
        Write-Host "  Building dav1d..." -ForegroundColor Gray
        meson compile -C $Dav1dBuild 2>&1 | Out-Null
        
        Write-Host "  Installing dav1d..." -ForegroundColor Gray
        meson install -C $Dav1dBuild 2>&1 | Out-Null
        
        Write-Host "  [OK] dav1d build complete" -ForegroundColor Green
    }
    catch {
        Write-Host "  [ERROR] dav1d build failed: $($_.Exception.Message)" -ForegroundColor Red
        Pop-Location
        exit 1
    }
    finally {
        Pop-Location
    }
} else {
    Write-Host "[2/5] Skipping dav1d..." -ForegroundColor Yellow
}

Write-Host ""

# Build libavif (requires dav1d)
if (-not $SkipLibavif) {
    Write-Host "[3/5] Building libavif 1.3.0..." -ForegroundColor Cyan
    
    $LibavifSrc = Join-Path $ImageLibsDir "libavif-1.3.0"
    $LibavifBuild = Join-Path $LibavifSrc "build"
    $LibavifInstall = Join-Path $ImageLibsDir "libavif-install"
    $Dav1dInstall = Join-Path $ImageLibsDir "dav1d-install"
    
    if (-not (Test-Path $LibavifSrc)) {
        Write-Host "  [ERROR] libavif source not found at: $LibavifSrc" -ForegroundColor Red
        exit 1
    }
    
    if (-not (Test-Path $Dav1dInstall)) {
        Write-Host "  [ERROR] dav1d not built. Run without -SkipDav1d first" -ForegroundColor Red
        exit 1
    }
    
    # Clean build directory
    if ($Clean -and (Test-Path $LibavifBuild)) {
        Remove-Item -Recurse -Force $LibavifBuild
    }
    
    New-Item -ItemType Directory -Force -Path $LibavifBuild | Out-Null
    
    Push-Location $LibavifBuild
    try {
        Write-Host "  Configuring libavif..." -ForegroundColor Gray
        cmake .. `
            -G "Visual Studio 17 2022" -A x64 `
            -DCMAKE_BUILD_TYPE=Release `
            -DCMAKE_INSTALL_PREFIX="$LibavifInstall" `
            -DBUILD_SHARED_LIBS=OFF `
            -DAVIF_CODEC_DAV1D=LOCAL `
            -DAVIF_LOCAL_DAV1D="$Dav1dInstall" `
            -DAVIF_CODEC_AOM=OFF `
            -DAVIF_BUILD_APPS=OFF `
            -DAVIF_BUILD_TESTS=OFF | Out-Null
        
        Write-Host "  Building libavif..." -ForegroundColor Gray
        cmake --build . --config Release | Out-Null
        
        Write-Host "  Installing libavif..." -ForegroundColor Gray
        cmake --install . --config Release | Out-Null
        
        Write-Host "  [OK] libavif build complete" -ForegroundColor Green
    }
    catch {
        Write-Host "  [ERROR] libavif build failed: $($_.Exception.Message)" -ForegroundColor Red
        Pop-Location
        exit 1
    }
    finally {
        Pop-Location
    }
} else {
    Write-Host "[3/5] Skipping libavif..." -ForegroundColor Yellow
}

Write-Host ""

# Build libjxl (JPEG XL)
if (-not $SkipLibjxl) {
    Write-Host "[4/5] Building libjxl 0.11.1..." -ForegroundColor Cyan
    
    $LibjxlSrc = Join-Path $ImageLibsDir "libjxl-0.11.1"
    $LibjxlBuild = Join-Path $LibjxlSrc "build"
    $LibjxlInstall = Join-Path $ImageLibsDir "libjxl-install"
    
    if (-not (Test-Path $LibjxlSrc)) {
        Write-Host "  [ERROR] libjxl source not found at: $LibjxlSrc" -ForegroundColor Red
        exit 1
    }
    
    # Clean build directory
    if ($Clean -and (Test-Path $LibjxlBuild)) {
        Remove-Item -Recurse -Force $LibjxlBuild
    }
    
    New-Item -ItemType Directory -Force -Path $LibjxlBuild | Out-Null
    
    Push-Location $LibjxlBuild
    try {
        Write-Host "  Configuring libjxl..." -ForegroundColor Gray
        cmake .. `
            -G "Visual Studio 17 2022" -A x64 `
            -DCMAKE_BUILD_TYPE=Release `
            -DCMAKE_INSTALL_PREFIX="$LibjxlInstall" `
            -DBUILD_SHARED_LIBS=OFF `
            -DBUILD_TESTING=OFF `
            -DJPEGXL_ENABLE_TOOLS=OFF `
            -DJPEGXL_ENABLE_BENCHMARK=OFF `
            -DJPEGXL_ENABLE_EXAMPLES=OFF `
            -DJPEGXL_FORCE_SYSTEM_BROTLI=OFF `
            -DJPEGXL_FORCE_SYSTEM_HWY=OFF | Out-Null
        
        Write-Host "  Building libjxl (this may take several minutes)..." -ForegroundColor Gray
        cmake --build . --config Release | Out-Null
        
        Write-Host "  Installing libjxl..." -ForegroundColor Gray
        cmake --install . --config Release | Out-Null
        
        Write-Host "  [OK] libjxl build complete" -ForegroundColor Green
    }
    catch {
        Write-Host "  [ERROR] libjxl build failed: $($_.Exception.Message)" -ForegroundColor Red
        Pop-Location
        exit 1
    }
    finally {
        Pop-Location
    }
} else {
    Write-Host "[4/5] Skipping libjxl..." -ForegroundColor Yellow
}

Write-Host ""

# Summary
Write-Host "[5/5] Build Summary" -ForegroundColor Cyan
Write-Host ""
Write-Host "Installation directories:" -ForegroundColor Yellow
if (-not $SkipDav1d) {
    Write-Host "  dav1d:   $ImageLibsDir\dav1d-install\" -ForegroundColor Gray
}
if (-not $SkipLibavif) {
    Write-Host "  libavif: $ImageLibsDir\libavif-install\" -ForegroundColor Gray
}
if (-not $SkipLibjxl) {
    Write-Host "  libjxl:  $ImageLibsDir\libjxl-install\" -ForegroundColor Gray
}
Write-Host ""
Write-Host "=== Build Complete ===" -ForegroundColor Green
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Cyan
Write-Host "  1. Update CBXShell.vcxproj to include new library paths" -ForegroundColor Gray
Write-Host "  2. Uncomment jxl_decoder.cpp in project file" -ForegroundColor Gray
Write-Host "  3. Add libavif/libjxl dependencies to linker" -ForegroundColor Gray
Write-Host "  4. Rebuild CBXShell.sln" -ForegroundColor Gray
