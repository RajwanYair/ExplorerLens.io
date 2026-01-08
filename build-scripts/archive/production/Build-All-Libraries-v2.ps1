# Build-All-Libraries-v2.ps1
# Comprehensive library builder for DarkThumbs

param(
    [switch]$SkipExisting = $true
)

$ErrorActionPreference = "Continue"
$ProgressPreference = "SilentlyContinue"

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "DarkThumbs Library Builder v2" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Find MSBuild
$msbuild = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\MSBuild\Current\Bin\amd64\MSBuild.exe"
if (-not (Test-Path $msbuild)) {
    Write-Host "ERROR: MSBuild not found at $msbuild" -ForegroundColor Red
    exit 1
}

Write-Host "Using MSBuild: $msbuild" -ForegroundColor Green
Write-Host ""

$builtLibs = @()
$failedLibs = @()

# Library definitions
$libraries = @(
    @{
        Name     = "Zlib"
        Path     = "external\compression\zlib-1.3.1\build-vs\Release\zlibstatic.lib"
        BuildDir = "external\compression\zlib-1.3.1"
        BuildCmd = { 
            if (-not (Test-Path "build-vs")) { mkdir "build-vs" }
            Set-Location "build-vs"
            cmake .. -G "Visual Studio 18 2026" -A x64 -DCMAKE_BUILD_TYPE=Release
            cmake --build . --config Release
            Set-Location ..
        }
    },
    @{
        Name     = "LZ4"
        Path     = "external\compression\lz4-1.10.0\build\VS2022\liblz4\bin\x64_Release\liblz4_static.lib"
        BuildDir = "external\compression\lz4-1.10.0"
        BuildCmd = {
            $sln = "build\VS2022\lz4.sln"
            if (Test-Path $sln) {
                & $msbuild $sln /p:Configuration=Release /p:Platform=x64 /m /v:minimal /t:liblz4
            }
        }
    },
    @{
        Name     = "Zstd"
        Path     = "external\compression\zstd-1.5.7\build\VS2010\bin\x64_Release\libzstd_static.lib"
        BuildDir = "external\compression\zstd-1.5.7"
        BuildCmd = {
            $sln = "build\VS2010\zstd.sln"
            if (Test-Path $sln) {
                # Upgrade solution to current toolset
                & $msbuild $sln /t:libzstd /p:Configuration=Release /p:Platform=x64 /p:PlatformToolset=v143 /m /v:minimal
            }
        }
    },
    @{
        Name     = "Bzip2"
        Path     = "external\compression\bzip2-1.0.8\x64\Release\bzip2.lib"
        BuildDir = "external\compression\bzip2-1.0.8"
        BuildCmd = {
            # Bzip2 needs manual setup - skip for now
            Write-Host "  Bzip2 requires custom build setup - skipping" -ForegroundColor Yellow
        }
    },
    @{
        Name     = "LZMA"
        Path     = "external\compression\lzma-24.08\build-vs\Release\lzma.lib"
        BuildDir = "external\compression\lzma-24.08"
        BuildCmd = {
            if (-not (Test-Path "build-vs")) { mkdir "build-vs" }
            Set-Location "build-vs"
            cmake .. -G "Visual Studio 18 2026" -A x64 -DCMAKE_BUILD_TYPE=Release
            cmake --build . --config Release
            Set-Location ..
        }
    }
)

# Build each library
foreach ($lib in $libraries) {
    Write-Host ">>> Processing $($lib.Name)..." -ForegroundColor White
    
    # Check if library already exists
    if ($SkipExisting -and (Test-Path $lib.Path)) {
        $size = (Get-Item $lib.Path).Length / 1KB
        Write-Host "  ✓ Already built: $([math]::Round($size, 2)) KB" -ForegroundColor Green
        $builtLibs += $lib.Name
        Write-Host ""
        continue
    }
    
    # Check if source directory exists
    if (-not (Test-Path $lib.BuildDir)) {
        Write-Host "  ✗ Source directory not found: $($lib.BuildDir)" -ForegroundColor Red
        $failedLibs += $lib.Name
        Write-Host ""
        continue
    }
    
    # Build the library
    try {
        Push-Location
        Set-Location $lib.BuildDir
        
        Write-Host "  Building in $(Get-Location)..." -ForegroundColor Cyan
        & $lib.BuildCmd
        
        Pop-Location
        
        # Verify output
        if (Test-Path $lib.Path) {
            $size = (Get-Item $lib.Path).Length / 1KB
            Write-Host "  ✓ Built successfully: $([math]::Round($size, 2)) KB" -ForegroundColor Green
            $builtLibs += $lib.Name
        } else {
            Write-Host "  ✗ Build failed - output not found" -ForegroundColor Red
            $failedLibs += $lib.Name
        }
    } catch {
        Write-Host "  ✗ Error: $($_.Exception.Message)" -ForegroundColor Red
        $failedLibs += $lib.Name
        Pop-Location
    }
    
    Write-Host ""
}

# Summary
Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Build Summary" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Built: $($builtLibs.Count) libraries" -ForegroundColor Green
if ($builtLibs.Count -gt 0) {
    foreach ($lib in $builtLibs) {
        Write-Host "  ✓ $lib" -ForegroundColor Green
    }
}
Write-Host ""

if ($failedLibs.Count -gt 0) {
    Write-Host "Failed: $($failedLibs.Count) libraries" -ForegroundColor Red
    foreach ($lib in $failedLibs) {
        Write-Host "  ✗ $lib" -ForegroundColor Red
    }
    Write-Host ""
}

Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
