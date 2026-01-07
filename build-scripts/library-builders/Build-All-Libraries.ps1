# ===========================================================================
# Build-All-Libraries.ps1
# Build all external libraries for DarkThumbs
# ===========================================================================

$ErrorActionPreference = "Stop"

Write-Host ""
Write-Host "==========================================================================" -ForegroundColor Cyan
Write-Host "Building All External Libraries for DarkThumbs" -ForegroundColor Cyan
Write-Host "==========================================================================" -ForegroundColor Cyan
Write-Host ""

$ProjectRoot = $PSScriptRoot
Set-Location $ProjectRoot

# Find vcvarsall.bat
$vcvarsall = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"
if (-not (Test-Path $vcvarsall)) {
    Write-Host "ERROR: vcvarsall.bat not found!" -ForegroundColor Red
    exit 1
}

# Find CMake
$CMake = Get-Command cmake -ErrorAction SilentlyContinue
if (-not $CMake) {
    Write-Host "ERROR: CMake not found!" -ForegroundColor Red
    exit 1
}

Write-Host "✓ vcvarsall: $vcvarsall" -ForegroundColor Green
Write-Host "✓ CMake: $($CMake.Path)" -ForegroundColor Green
Write-Host ""

# Function to build a library with CMake + Ninja
function Build-Library {
    param(
        [string]$Name,
        [string]$SourceDir,
        [string]$BuildDir,
        [hashtable]$CMakeOptions = @{}
    )
    
    Write-Host "Building $Name..." -ForegroundColor Yellow
    Write-Host "  Source: $SourceDir" -ForegroundColor White
    Write-Host "  Build:  $BuildDir" -ForegroundColor White
    
    if (-not (Test-Path $SourceDir)) {
        Write-Host "  [SKIP] Source directory not found" -ForegroundColor Red
        return $false
    }
    
    # Create build directory
    if (-not (Test-Path $BuildDir)) {
        New-Item -ItemType Directory -Path $BuildDir -Force | Out-Null
    }
    
    # Build CMake arguments
    $cmakeArgs = @(
        "-S", $SourceDir,
        "-B", $BuildDir,
        "-G", "Ninja",
        "-DCMAKE_BUILD_TYPE=Release",
        "-DBUILD_SHARED_LIBS=OFF"
    )
    
    foreach ($key in $CMakeOptions.Keys) {
        $cmakeArgs += "-D$key=$($CMakeOptions[$key])"
    }
    
    # Configure
    Write-Host "  Configuring..." -ForegroundColor White
    $env:VSINSTALLDIR = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\"
    & cmd /c "`"$vcvarsall`" x64 >nul 2>&1 && cmake $($cmakeArgs -join ' ') 2>&1"
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "  [FAILED] Configuration failed!" -ForegroundColor Red
        return $false
    }
    
    # Build
    Write-Host "  Building Release..." -ForegroundColor White
    & cmd /c "`"$vcvarsall`" x64 >nul 2>&1 && cmake --build `"$BuildDir`" --config Release --parallel 2>&1" | Where-Object { 
        $_ -match "error|warning|fatal|\[.*%\]|Building|Linking|Generating" 
    }
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "  [FAILED] Build failed!" -ForegroundColor Red
        return $false
    }
    
    Write-Host "  [SUCCESS]" -ForegroundColor Green
    Write-Host ""
    return $true
}

# Library Definitions
$libraries = @(
    @{
        Name = "Zlib 1.3.1"
        SourceDir = Join-Path $ProjectRoot "external\compression\zlib-1.3.1"
        BuildDir = Join-Path $ProjectRoot "external\compression\zlib-1.3.1\build-vs"
        Options = @{}
        OutputLib = "external\compression\zlib-1.3.1\build-vs\Release\zlibstatic.lib"
    },
    @{
        Name = "LZ4 1.10.0"
        SourceDir = Join-Path $ProjectRoot "external\compression\lz4-1.10.0\build\cmake"
        BuildDir = Join-Path $ProjectRoot "external\compression\lz4-1.10.0\build-vs"
        Options = @{
            "LZ4_BUILD_CLI" = "OFF"
            "LZ4_BUILD_LEGACY_LZ4C" = "OFF"
        }
        OutputLib = "external\compression\lz4-1.10.0\build-vs\Release\liblz4_static.lib"
    },
    @{
        Name = "Zstd 1.5.7"
        SourceDir = Join-Path $ProjectRoot "external\compression\zstd-1.5.7\build\cmake"
        BuildDir = Join-Path $ProjectRoot "external\compression\zstd-1.5.7\build-vs"
        Options = @{
            "ZSTD_BUILD_PROGRAMS" = "OFF"
            "ZSTD_BUILD_CONTRIB" = "OFF"
            "ZSTD_BUILD_TESTS" = "OFF"
        }
        OutputLib = "external\compression\zstd-1.5.7\build-vs\lib\Release\zstd_static.lib"
    },
    @{
        Name = "MinizipNG 4.0.10"
        SourceDir = Join-Path $ProjectRoot "external\compression\minizip-ng-4.0.10"
        BuildDir = Join-Path $ProjectRoot "external\compression\minizip-ng-4.0.10\build-vs"
        Options = @{
            "MZ_COMPAT" = "OFF"
            "MZ_ZLIB" = "ON"
            "MZ_BZIP2" = "ON"
            "MZ_LZMA" = "ON"
            "MZ_ZSTD" = "ON"
        }
        OutputLib = "external\compression\minizip-ng-4.0.10\build-vs\Release\minizip.lib"
    },
    @{
        Name = "LibWebP 1.5.0"
        SourceDir = Join-Path $ProjectRoot "external\image-libs\libwebp-1.5.0"
        BuildDir = Join-Path $ProjectRoot "external\image-libs\libwebp-1.5.0\build-vs"
        Options = @{
            "WEBP_BUILD_ANIM_UTILS" = "OFF"
            "WEBP_BUILD_CWEBP" = "OFF"
            "WEBP_BUILD_DWEBP" = "OFF"
            "WEBP_BUILD_GIF2WEBP" = "OFF"
            "WEBP_BUILD_IMG2WEBP" = "OFF"
            "WEBP_BUILD_VWEBP" = "OFF"
            "WEBP_BUILD_WEBPINFO" = "OFF"
            "WEBP_BUILD_WEBPMUX" = "OFF"
            "WEBP_BUILD_EXTRAS" = "OFF"
        }
        OutputLib = "external\image-libs\libwebp-1.5.0\build-vs\Release\webp.lib"
    }
)

# Build all libraries
$successCount = 0
$failCount = 0
$builtLibs = @()

foreach ($lib in $libraries) {
    $result = Build-Library -Name $lib.Name `
                           -SourceDir $lib.SourceDir `
                           -BuildDir $lib.BuildDir `
                           -CMakeOptions $lib.Options
    
    if ($result) {
        $successCount++
        
        # Check if output exists
        $outPath = Join-Path $ProjectRoot $lib.OutputLib
        if (Test-Path $outPath) {
            $size = (Get-Item $outPath).Length
            $builtLibs += "  ✓ $($lib.Name) - $([Math]::Round($size/1KB, 1)) KB"
        }
    } else {
        $failCount++
    }
}

Write-Host ""
Write-Host "==========================================================================" -ForegroundColor Cyan
Write-Host "Build Summary" -ForegroundColor Cyan
Write-Host "==========================================================================" -ForegroundColor Cyan
Write-Host "Success: $successCount" -ForegroundColor Green
Write-Host "Failed:  $failCount" -ForegroundColor Red
Write-Host ""

if ($builtLibs.Count -gt 0) {
    Write-Host "Built Libraries:" -ForegroundColor White
    foreach ($lib in $builtLibs) {
        Write-Host $lib -ForegroundColor Green
    }
}

if ($failCount -eq 0) {
    Write-Host ""
    Write-Host "All libraries built successfully!" -ForegroundColor Green
    exit 0
} else {
    Write-Host ""
    Write-Host "Some libraries failed to build. Check errors above." -ForegroundColor Red
    exit 1
}
