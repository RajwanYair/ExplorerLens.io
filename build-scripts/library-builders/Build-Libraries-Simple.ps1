# ===========================================================================
# Build-Libraries-Simple.ps1
# Build required external libraries using existing build systems
# ===========================================================================

$ErrorActionPreference = "Stop"

Write-Host ""
Write-Host "==========================================================================" -ForegroundColor Cyan
Write-Host "Building External Libraries - Simple Approach" -ForegroundColor Cyan
Write-Host "==========================================================================" -ForegroundColor Cyan
Write-Host ""

$ProjectRoot = $PSScriptRoot
Set-Location $ProjectRoot

# Find MSBuild
$msbuild = & "$PSScriptRoot\build-scripts\Find-MSBuild.ps1"
if (-not $msbuild) {
    Write-Host "ERROR: MSBuild not found!" -ForegroundColor Red
    exit 1
}

Write-Host "✓ MSBuild: $msbuild" -ForegroundColor Green
Write-Host ""

# ============================================================================
# 1. Zlib (already built)
# ============================================================================
Write-Host "[1/5] Zlib 1.3.1" -ForegroundColor Yellow
$zlibLib = "external\compression\zlib-1.3.1\build-vs\Release\zlibstatic.lib"
if (Test-Path $zlibLib) {
    $size = (Get-Item $zlibLib).Length
    Write-Host "  ✓ Already built ($([Math]::Round($size/1KB, 1)) KB)" -ForegroundColor Green
} else {
    Write-Host "  Building..." -ForegroundColor White
    & "$PSScriptRoot\build-scripts\Build-Zlib.ps1"
    if ($LASTEXITCODE -ne 0) {
        Write-Host "  ✗ Build failed!" -ForegroundColor Red
    }
}
Write-Host ""

# ============================================================================
# 2. LZ4 (use existing VS project)
# ============================================================================
Write-Host "[2/5] LZ4 1.10.0" -ForegroundColor Yellow
$lz4Sln = "external\compression\lz4-1.10.0\build\VS2022\lz4.sln"
$lz4Lib = "external\compression\lz4-1.10.0\build\VS2022\liblz4\bin\x64_Release\liblz4_static.lib"

if (Test-Path $lz4Lib) {
    $size = (Get-Item $lz4Lib).Length
    Write-Host "  ✓ Already built ($([Math]::Round($size/1KB, 1)) KB)" -ForegroundColor Green
    
    # Copy to expected location
    $targetDir = "external\compression\lz4-1.10.0\build-vs\Release"
    if (-not (Test-Path $targetDir)) {
        New-Item -ItemType Directory -Path $targetDir -Force | Out-Null
    }
    Copy-Item $lz4Lib (Join-Path $targetDir "liblz4_static.lib") -Force
    Write-Host "  ✓ Copied to build-vs\Release\" -ForegroundColor Green
} else {
    Write-Host "  Building with MSBuild..." -ForegroundColor White
    if (Test-Path $lz4Sln) {
        Push-Location (Split-Path $lz4Sln)
        & $msbuild "lz4.sln" /p:Configuration=Release /p:Platform=x64 /t:liblz4 /m /v:minimal /nologo 2>&1 | Select-String "error|warning" | Select-Object -Last 5
        Pop-Location
        
        if (Test-Path $lz4Lib) {
            Write-Host "  ✓ Build succeeded!" -ForegroundColor Green
            # Copy to expected location
            $targetDir = "external\compression\lz4-1.10.0\build-vs\Release"
            if (-not (Test-Path $targetDir)) {
                New-Item -ItemType Directory -Path $targetDir -Force | Out-Null
            }
            Copy-Item $lz4Lib (Join-Path $targetDir "liblz4_static.lib") -Force
            Write-Host "  ✓ Copied to build-vs\Release\" -ForegroundColor Green
        } else {
            Write-Host "  ✗ Build failed!" -ForegroundColor Red
        }
    } else {
        Write-Host "  ✗ Solution file not found!" -ForegroundColor Red
    }
}
Write-Host ""

# ============================================================================
# 3. Zstd (use existing VS project)
# ============================================================================
Write-Host "[3/5] Zstd 1.5.7" -ForegroundColor Yellow
$zstdSln = "external\compression\zstd-1.5.7\build\VS2010\zstd.sln"
$zstdLib = "external\compression\zstd-1.5.7\build\VS2010\bin\x64_Release\libzstd_static.lib"

if (Test-Path "external\compression\zstd-1.5.7\build-vs\lib\Release\zstd_static.lib") {
    $size = (Get-Item "external\compression\zstd-1.5.7\build-vs\lib\Release\zstd_static.lib").Length
    Write-Host "  ✓ Already built ($([Math]::Round($size/1KB, 1)) KB)" -ForegroundColor Green
} elseif (Test-Path $zstdLib) {
    $size = (Get-Item $zstdLib).Length
    Write-Host "  ✓ Found existing build ($([Math]::Round($size/1KB, 1)) KB)" -ForegroundColor Green
    
    # Copy to expected location
    $targetDir = "external\compression\zstd-1.5.7\build-vs\lib\Release"
    if (-not (Test-Path $targetDir)) {
        New-Item -ItemType Directory -Path $targetDir -Force | Out-Null
    }
    Copy-Item $zstdLib (Join-Path $targetDir "zstd_static.lib") -Force
    Write-Host "  ✓ Copied to build-vs\lib\Release\" -ForegroundColor Green
} else {
    Write-Host "  Building with MSBuild..." -ForegroundColor White
    if (Test-Path $zstdSln) {
        Push-Location (Split-Path $zstdSln)
        & $msbuild "zstd.sln" /p:Configuration=Release /p:Platform=x64 /t:libzstd /m /v:minimal /nologo 2>&1 | Select-String "error|warning" | Select-Object -Last 5
        Pop-Location
        
        if (Test-Path $zstdLib) {
            Write-Host "  ✓ Build succeeded!" -ForegroundColor Green
            # Copy to expected location
            $targetDir = "external\compression\zstd-1.5.7\build-vs\lib\Release"
            if (-not (Test-Path $targetDir)) {
                New-Item -ItemType Directory -Path $targetDir -Force | Out-Null
            }
            Copy-Item $zstdLib (Join-Path $targetDir "zstd_static.lib") -Force
            Write-Host "  ✓ Copied to build-vs\lib\Release\" -ForegroundColor Green
        } else {
            Write-Host "  ✗ Build failed!" -ForegroundColor Red
        }
    } else {
        Write-Host "  ✗ Solution file not found!" -ForegroundColor Red
        Write-Host "  Trying build script..." -ForegroundColor Yellow
        & "$PSScriptRoot\build-scripts\build-zstd-1.5.7.ps1"
    }
}
Write-Host ""

# ============================================================================
# 4. LibWebP (use build script)
# ============================================================================
Write-Host "[4/5] LibWebP 1.5.0" -ForegroundColor Yellow
$webpLib = "external\image-libs\libwebp-1.5.0\build-vs\Release\webp.lib"
if (Test-Path $webpLib) {
    $size = (Get-Item $webpLib).Length
    Write-Host "  ✓ Already built ($([Math]::Round($size/1KB, 1)) KB)" -ForegroundColor Green
} else {
    Write-Host "  Building..." -ForegroundColor White
    $buildScript = "build-scripts\build-libwebp-1.5.ps1"
    if (Test-Path $buildScript) {
        & "$PSScriptRoot\$buildScript"
    } else {
        Write-Host "  ✗ Build script not found!" -ForegroundColor Red
    }
}
Write-Host ""

# ============================================================================
# 5. MinizipNG - Copy from existing build or skip
# ============================================================================
Write-Host "[5/5] MinizipNG 4.0.10" -ForegroundColor Yellow
$minizipLib = "external\compression\minizip-ng-4.0.10\build-vs\Release\minizip.lib"

# Check if there's a minizip anywhere
$existingMinizip = Get-ChildItem -Path "external\compression\minizip-ng-4.0.10" -Recurse -Filter "minizip*.lib" -ErrorAction SilentlyContinue | Select-Object -First 1

if (Test-Path $minizipLib) {
    $size = (Get-Item $minizipLib).Length
    Write-Host "  ✓ Already built ($([Math]::Round($size/1KB, 1)) KB)" -ForegroundColor Green
} elseif ($existingMinizip) {
    Write-Host "  ✓ Found existing build at: $($existingMinizip.FullName)" -ForegroundColor Green
    $targetDir = "external\compression\minizip-ng-4.0.10\build-vs\Release"
    if (-not (Test-Path $targetDir)) {
        New-Item -ItemType Directory -Path $targetDir -Force | Out-Null
    }
    Copy-Item $existingMinizip.FullName (Join-Path $targetDir "minizip.lib") -Force
    Write-Host "  ✓ Copied to build-vs\Release\" -ForegroundColor Green
} else {
    Write-Host "  ⚠ MinizipNG not built - will try to build" -ForegroundColor Yellow
    Write-Host "  Note: MinizipNG may not be critical for initial build" -ForegroundColor Gray
}
Write-Host ""

# ============================================================================
# Summary
# ============================================================================
Write-Host "==========================================================================" -ForegroundColor Cyan
Write-Host "Library Build Summary" -ForegroundColor Cyan
Write-Host "==========================================================================" -ForegroundColor Cyan

$requiredLibs = @(
    @{Name="Zlib"; Path="external\compression\zlib-1.3.1\build-vs\Release\zlibstatic.lib"},
    @{Name="LZ4"; Path="external\compression\lz4-1.10.0\build-vs\Release\liblz4_static.lib"},
    @{Name="Zstd"; Path="external\compression\zstd-1.5.7\build-vs\lib\Release\zstd_static.lib"},
    @{Name="WebP"; Path="external\image-libs\libwebp-1.5.0\build-vs\Release\webp.lib"},
    @{Name="SharpYUV"; Path="external\image-libs\libwebp-1.5.0\build-vs\Release\sharpyuv.lib"},
    @{Name="MinizipNG"; Path="external\compression\minizip-ng-4.0.10\build-vs\Release\minizip.lib"}
)

$builtCount = 0
$missingCount = 0

foreach ($lib in $requiredLibs) {
    if (Test-Path $lib.Path) {
        $size = (Get-Item $lib.Path).Length
        Write-Host "  ✓ $($lib.Name) - $([Math]::Round($size/1KB, 1)) KB" -ForegroundColor Green
        $builtCount++
    } else {
        Write-Host "  ✗ $($lib.Name) - MISSING" -ForegroundColor Red
        $missingCount++
    }
}

Write-Host ""
Write-Host "Built: $builtCount / $($requiredLibs.Count)" -ForegroundColor $(if ($builtCount -eq $requiredLibs.Count) { "Green" } else { "Yellow" })

if ($missingCount -eq 0) {
    Write-Host ""
    Write-Host "All libraries ready! You can now build CBXShell.sln" -ForegroundColor Green
    exit 0
} else {
    Write-Host ""
    Write-Host "Some libraries are missing. CBXShell build may fail." -ForegroundColor Yellow
    exit 1
}
