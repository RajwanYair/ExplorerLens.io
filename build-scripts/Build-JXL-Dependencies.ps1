# Build libjxl dependencies (brotli, highway) for JPEG XL Support
# ExplorerLens External Library Build Script

param(
    [switch]$Clean,
    [string]$Configuration = "Release"
)

$ErrorActionPreference = "Stop"

# Paths
$repoRoot = Split-Path -Parent $PSScriptRoot
$externalDir = Join-Path $repoRoot "external"

Write-Host "========================================================" -ForegroundColor Cyan
Write-Host "  Building libjxl Dependencies (brotli + highway)" -ForegroundColor Cyan
Write-Host "========================================================" -ForegroundColor Cyan
Write-Host ""

function Build-Library {
    param(
        [string]$Name,
        [string]$GitUrl,
        [string]$Version,
        [hashtable]$CMakeOptions = @{}
    )
    
    $libDir = Join-Path $externalDir $Name
    $buildDir = Join-Path $libDir "build"
    $installDir = Join-Path $externalDir "$Name-install"
    
    Write-Host "Building $Name..." -ForegroundColor Green
    Write-Host "----------------------------------------" -ForegroundColor Gray
    
    # Clean if requested
    if ($Clean) {
        if (Test-Path $buildDir) { Remove-Item -Recurse -Force $buildDir }
        if (Test-Path $installDir) { Remove-Item -Recurse -Force $installDir }
    }
    
    # Clone if not present
    if (-not (Test-Path $libDir)) {
        Write-Host "Cloning $Name repository..." -ForegroundColor Yellow
        Push-Location $externalDir
        if ($Version) {
            git clone --depth 1 --branch $Version $GitUrl $Name
        } else {
            git clone --depth 1 $GitUrl $Name
        }
        Pop-Location
    }
    
    # Create build directory
    if (-not (Test-Path $buildDir)) {
        New-Item -ItemType Directory -Path $buildDir | Out-Null
    }
    
    # Configure
    Write-Host "Configuring $Name..." -ForegroundColor Yellow
    Push-Location $buildDir
    
    $cmakeArgs = @(
        "-G", "Visual Studio 18 2026",
        "-A", "x64",
        "-DCMAKE_INSTALL_PREFIX=$installDir",
        "-DCMAKE_BUILD_TYPE=$Configuration",
        "-DBUILD_SHARED_LIBS=ON"
    )
    
    # Add custom options
    foreach ($key in $CMakeOptions.Keys) {
        $cmakeArgs += "-D$key=$($CMakeOptions[$key])"
    }
    
    $cmakeArgs += ".."
    
    & cmake @cmakeArgs
    if ($LASTEXITCODE -ne 0) {
        Write-Host "$Name configuration failed!" -ForegroundColor Red
        Pop-Location
        exit 1
    }
    
    # Build
    Write-Host "Building $Name..." -ForegroundColor Yellow
    & cmake --build . --config $Configuration --parallel
    if ($LASTEXITCODE -ne 0) {
        Write-Host "$Name build failed!" -ForegroundColor Red
        Pop-Location
        exit 1
    }
    
    # Install
    Write-Host "Installing $Name..." -ForegroundColor Yellow
    & cmake --install . --config $Configuration
    if ($LASTEXITCODE -ne 0) {
        Write-Host "$Name install failed!" -ForegroundColor Red
        Pop-Location
        exit 1
    }
    
    Pop-Location
    
    Write-Host "✅ $Name build complete!" -ForegroundColor Green
    Write-Host ""
}

# Build brotli (compression library)
Build-Library -Name "brotli" `
    -GitUrl "https://github.com/google/brotli.git" `
    -Version "v1.1.0" `
    -CMakeOptions @{
    "BROTLI_DISABLE_TESTS" = "ON"
    "BROTLI_BUNDLED_MODE"  = "ON"
}

# Build highway (SIMD library)
Build-Library -Name "highway" `
    -GitUrl "https://github.com/google/highway.git" `
    -Version "1.2.0" `
    -CMakeOptions @{
    "BUILD_TESTING"       = "ON"
    "HWY_ENABLE_EXAMPLES" = "ON"
    "HWY_ENABLE_TESTS"    = "ON"
}

Write-Host ""
Write-Host "========================================================" -ForegroundColor Green
Write-Host "  All libjxl Dependencies Built Successfully!" -ForegroundColor Green
Write-Host "========================================================" -ForegroundColor Green
Write-Host ""

Write-Host "Built libraries:" -ForegroundColor Cyan
Write-Host "  ✅ brotli (v1.1.0) - Compression library" -ForegroundColor White
Write-Host "  ✅ highway (1.2.0) - SIMD library" -ForegroundColor White
Write-Host ""

Write-Host "Installation directories:" -ForegroundColor Cyan
Write-Host "  - brotli: $externalDir\brotli-install" -ForegroundColor Yellow
Write-Host "  - highway: $externalDir\highway-install" -ForegroundColor Yellow
Write-Host ""

Write-Host "Next steps:" -ForegroundColor Cyan
Write-Host "1. libjxl 0.11.1 is already built (verify: external\libjxl-install)" -ForegroundColor White
Write-Host "2. Update Engine/CMakeLists.txt to link:" -ForegroundColor White
Write-Host "   - jxl.lib, jxl_threads.lib" -ForegroundColor Gray
Write-Host "   - brotlicommon.lib, brotlidec.lib" -ForegroundColor Gray
Write-Host "   - hwy.lib" -ForegroundColor Gray
Write-Host "3. Uncomment JXLDecoder.h/cpp in CMakeLists.txt" -ForegroundColor White
Write-Host "4. Implement JXLDecoder::DecodeJXLImage() method" -ForegroundColor White
Write-Host "5. Add unit tests and test with sample .jxl files" -ForegroundColor White
Write-Host ""

