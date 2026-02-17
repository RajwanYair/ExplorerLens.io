# ===========================================================================
# Build-Critical-Libraries.ps1
# Build critical missing libraries for DarkThumbs using NMake
# ===========================================================================
# 
# ⚠️  DEPRECATED: Use Build-All-DarkThumbs-V7.ps1 instead
# This script is kept for reference only.
# See docs/development/PATH_UPDATE_SUMMARY_2026-02-16.md for current build workflow.
# 

$ErrorActionPreference = "Continue"

Write-Host ""
Write-Host "==========================================================================" -ForegroundColor Cyan
Write-Host "Building Critical Libraries" -ForegroundColor Cyan
Write-Host "==========================================================================" -ForegroundColor Cyan
Write-Host ""

$ProjectRoot = $PSScriptRoot
Set-Location $ProjectRoot

$vcvarsall = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"

if (-not (Test-Path $vcvarsall)) {
    Write-Host "ERROR: vcvarsall.bat not found!" -ForegroundColor Red
    exit 1
}

# =============================================================================
# 1. Build Zstd 1.5.7 with NMake
# =============================================================================
Write-Host "[1/4] Building Zstd 1.5.7..." -ForegroundColor Yellow

$zstdOutput = "external\compression-libs\zstd-1.5.7\build-vs\lib\Release\zstd_static.lib"
if (Test-Path $zstdOutput) {
    $size = (Get-Item $zstdOutput).Length
    if ($size -gt 1KB) {
        Write-Host "  [SKIP] Already built ($([Math]::Round($size/1KB, 1)) KB)" -ForegroundColor Green
    } else {
        Remove-Item $zstdOutput -Force
    }
}

if (-not (Test-Path $zstdOutput)) {
    $zstdSrc = "external\compression-libs\zstd-1.5.7\build\cmake"
    $zstdBuild = "external\compression-libs\zstd-1.5.7\build-vs"
    
    if (Test-Path $zstdBuild) {
        Remove-Item $zstdBuild -Recurse -Force -ErrorAction SilentlyContinue
    }
    New-Item -ItemType Directory -Path $zstdBuild -Force | Out-Null
    
    $cmd = @"
call "$vcvarsall" x64 && cd /d "$PWD\$zstdBuild" && cmake "$PWD\$zstdSrc" -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DZSTD_BUILD_PROGRAMS=OFF -DZSTD_BUILD_SHARED=OFF -DZSTD_BUILD_STATIC=ON && nmake
"@
    
    Write-Host "  Configuring and building..." -ForegroundColor White
    cmd /c $cmd 2>&1 | Select-String "error|warning|Building|Linking|\[.*%\]" | Select-Object -Last 20
    
    if (Test-Path $zstdOutput) {
        $size = (Get-Item $zstdOutput).Length
        Write-Host "  [SUCCESS] $([Math]::Round($size/1KB, 1)) KB" -ForegroundColor Green
    } else {
        Write-Host "  [FAILED]" -ForegroundColor Red
    }
}
Write-Host ""

# =============================================================================
# 2. Build Bzip2 1.0.8 with NMake
# =============================================================================
Write-Host "[2/4] Building Bzip2 1.0.8..." -ForegroundColor Yellow

$bzip2Output = "external\compression-libs\bzip2-1.0.8\x64\Release\bzip2.lib"
if (Test-Path $bzip2Output) {
    $size = (Get-Item $bzip2Output).Length
    if ($size -gt 1KB) {
        Write-Host "  [SKIP] Already built ($([Math]::Round($size/1KB, 1)) KB)" -ForegroundColor Green
    } else {
        Remove-Item $bzip2Output -Force
    }
}

if (-not (Test-Path $bzip2Output)) {
    $bzip2Src = "external\compression\bzip2-1.0.8"
    $bzip2Build = "external\compression\bzip2-1.0.8\build-vs"
    
    if (Test-Path $bzip2Build) {
        Remove-Item $bzip2Build -Recurse -Force -ErrorAction SilentlyContinue
    }
    New-Item -ItemType Directory -Path $bzip2Build -Force | Out-Null
    
    $cmd = @"
call "$vcvarsall" x64 && cd /d "$PWD\$bzip2Build" && cmake "$PWD\$bzip2Src" -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF && nmake
"@
    
    Write-Host "  Configuring and building..." -ForegroundColor White
    cmd /c $cmd 2>&1 | Select-String "error|warning|Building|Linking" | Select-Object -Last 15
    
    # Find the output
    $bzip2Built = Get-ChildItem "$bzip2Build" -Recurse -Filter "*bz2*.lib" | Where-Object { $_.Length -gt 1KB } | Select-Object -First 1
    
    if ($bzip2Built) {
        $targetDir = Split-Path $bzip2Output
        if (-not (Test-Path $targetDir)) {
            New-Item -ItemType Directory -Path $targetDir -Force | Out-Null
        }
        Copy-Item $bzip2Built.FullName $bzip2Output -Force
        $size = (Get-Item $bzip2Output).Length
        Write-Host "  [SUCCESS] $([Math]::Round($size/1KB, 1)) KB" -ForegroundColor Green
    } else {
        Write-Host "  [FAILED]" -ForegroundColor Red
    }
}
Write-Host ""

# =============================================================================
# 3. Build LibWebP 1.5.0 with NMake
# =============================================================================
Write-Host "[3/4] Building LibWebP 1.5.0..." -ForegroundColor Yellow

$webpOutput = "external\image-libs\libwebp-1.5.0-build\build-vs\Release\webp.lib"
$sharpyuvOutput = "external\image-libs\libwebp-1.5.0-build\build-vs\Release\sharpyuv.lib"

if ((Test-Path $webpOutput) -and (Test-Path $sharpyuvOutput)) {
    $size1 = (Get-Item $webpOutput).Length
    $size2 = (Get-Item $sharpyuvOutput).Length
    if ($size1 -gt 1KB -and $size2 -gt 1KB) {
        Write-Host "  [SKIP] Already built (webp: $([Math]::Round($size1/1KB, 1)) KB, sharpyuv: $([Math]::Round($size2/1KB, 1)) KB)" -ForegroundColor Green
    } else {
        Remove-Item $webpOutput, $sharpyuvOutput -Force -ErrorAction SilentlyContinue
    }
}

if (-not ((Test-Path $webpOutput) -and (Test-Path $sharpyuvOutput))) {
    $webpSrc = "external\image-libs\libwebp-1.5.0-build"
    $webpBuild = "external\image-libs\libwebp-1.5.0-build\build-vs"
    
    if (Test-Path $webpBuild) {
        Remove-Item $webpBuild -Recurse -Force -ErrorAction SilentlyContinue
    }
    New-Item -ItemType Directory -Path $webpBuild -Force | Out-Null
    
    $cmd = @"
call "$vcvarsall" x64 && cd /d "$PWD\$webpBuild" && cmake "$PWD\$webpSrc" -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DWEBP_BUILD_ANIM_UTILS=OFF -DWEBP_BUILD_CWEBP=OFF -DWEBP_BUILD_DWEBP=OFF -DWEBP_BUILD_GIF2WEBP=OFF -DWEBP_BUILD_IMG2WEBP=OFF -DWEBP_BUILD_VWEBP=OFF -DWEBP_BUILD_WEBPINFO=OFF -DWEBP_BUILD_WEBPMUX=OFF -DWEBP_BUILD_EXTRAS=OFF -DBUILD_SHARED_LIBS=OFF && nmake
"@
    
    Write-Host "  Configuring and building..." -ForegroundColor White
    cmd /c $cmd 2>&1 | Select-String "error|warning|Building|Linking|\[.*%\]" | Select-Object -Last 25
    
    # Find the outputs
    $builtLibs = Get-ChildItem "$webpBuild" -Recurse -Filter "*.lib" | Where-Object { $_.Length -gt 1KB }
    
    $releaseDir = "$webpBuild\Release"
    if (-not (Test-Path $releaseDir)) {
        New-Item -ItemType Directory -Path $releaseDir -Force | Out-Null
    }
    
    $found = @()
    foreach ($lib in $builtLibs) {
        if ($lib.Name -eq "webp.lib") {
            Copy-Item $lib.FullName $webpOutput -Force
            $found += "webp"
        } elseif ($lib.Name -eq "sharpyuv.lib") {
            Copy-Item $lib.FullName $sharpyuvOutput -Force
            $found += "sharpyuv"
        }
    }
    
    if ($found.Count -eq 2) {
        $size1 = (Get-Item $webpOutput).Length
        $size2 = (Get-Item $sharpyuvOutput).Length
        Write-Host "  [SUCCESS] webp: $([Math]::Round($size1/1KB, 1)) KB, sharpyuv: $([Math]::Round($size2/1KB, 1)) KB" -ForegroundColor Green
    } else {
        Write-Host "  [FAILED] Only found: $($found -join ', ')" -ForegroundColor Red
    }
}
Write-Host ""

# =============================================================================
# 4. Build MinizipNG 4.0.10 with NMake
# =============================================================================
Write-Host "[4/4] Building MinizipNG 4.0.10..." -ForegroundColor Yellow

$minizipOutput = "external\compression\minizip-ng-4.0.10\build-vs\Release\minizip.lib"
if (Test-Path $minizipOutput) {
    $size = (Get-Item $minizipOutput).Length
    if ($size -gt 1KB) {
        Write-Host "  [SKIP] Already built ($([Math]::Round($size/1KB, 1)) KB)" -ForegroundColor Green
    } else {
        Remove-Item $minizipOutput -Force
    }
}

if (-not (Test-Path $minizipOutput)) {
    $minizipSrc = "external\compression\minizip-ng-4.0.10"
    $minizipBuild = "external\compression\minizip-ng-4.0.10\build-vs"
    
    if (Test-Path $minizipBuild) {
        Remove-Item $minizipBuild -Recurse -Force -ErrorAction SilentlyContinue
    }
    New-Item -ItemType Directory -Path $minizipBuild -Force | Out-Null
    
    # MinizipNG needs zlib, bzip2, zstd, lzma
    $zlibRoot = (Resolve-Path "external\compression\zlib-1.3.1").Path
    $bzip2Root = (Resolve-Path "external\compression\bzip2-1.0.8").Path
    $zstdRoot = (Resolve-Path "external\compression\zstd-1.5.7").Path
    $lzmaRoot = (Resolve-Path "external\compression-libs\lzma-26.00").Path
    
    $cmd = @"
call "$vcvarsall" x64 && cd /d "$PWD\$minizipBuild" && cmake "$PWD\$minizipSrc" -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DMZ_COMPAT=OFF -DMZ_ZLIB=ON -DMZ_BZIP2=ON -DMZ_LZMA=ON -DMZ_ZSTD=ON -DZLIB_ROOT="$zlibRoot" -DBZIP2_ROOT="$bzip2Root" -DZSTD_ROOT="$zstdRoot" -DLIBLZMA_ROOT="$lzmaRoot" && nmake
"@
    
    Write-Host "  Configuring and building..." -ForegroundColor White
    cmd /c $cmd 2>&1 | Select-String "error|warning|Building|Linking|\[.*%\]" | Select-Object -Last 25
    
    # Find the output
    $minizipBuilt = Get-ChildItem "$minizipBuild" -Recurse -Filter "minizip*.lib" | Where-Object { $_.Length -gt 1KB } | Select-Object -First 1
    
    if ($minizipBuilt) {
        $targetDir = "$minizipBuild\Release"
        if (-not (Test-Path $targetDir)) {
            New-Item -ItemType Directory -Path $targetDir -Force | Out-Null
        }
        Copy-Item $minizipBuilt.FullName $minizipOutput -Force
        $size = (Get-Item $minizipOutput).Length
        Write-Host "  [SUCCESS] $([Math]::Round($size/1KB, 1)) KB" -ForegroundColor Green
    } else {
        Write-Host "  [FAILED]" -ForegroundColor Red
    }
}
Write-Host ""

# =============================================================================
# Summary
# =============================================================================
Write-Host "==========================================================================" -ForegroundColor Cyan
Write-Host "Library Status" -ForegroundColor Cyan
Write-Host "==========================================================================" -ForegroundColor Cyan

$allLibs = @(
    @{Name = "Zlib"; Path = "external\compression\zlib-1.3.1\build-vs\Release\zlibstatic.lib" },
    @{Name = "LZ4"; Path = "external\compression\lz4-1.10.0\build\VS2022\liblz4\bin\x64_Release\liblz4_static.lib" },
    @{Name = "Zstd"; Path = "external\compression\zstd-1.5.7\build-vs\lib\Release\zstd_static.lib" },
    @{Name = "Bzip2"; Path = "external\compression\bzip2-1.0.8\x64\Release\bzip2.lib" },
    @{Name = "MinizipNG"; Path = "external\compression\minizip-ng-4.0.10\build-vs\Release\minizip.lib" },
    @{Name = "WebP"; Path = "external\image-libs\libwebp-1.5.0-build\build-vs\Release\webp.lib" },
    @{Name = "SharpYUV"; Path = "external\image-libs\libwebp-1.5.0-build\build-vs\Release\sharpyuv.lib" }
)

$ready = 0
$missing = 0

foreach ($lib in $allLibs) {
    if (Test-Path $lib.Path) {
        $size = (Get-Item $lib.Path).Length
        if ($size -gt 1KB) {
            Write-Host "  ✓ $($lib.Name.PadRight(12)) - $([Math]::Round($size/1KB, 1)) KB" -ForegroundColor Green
            $ready++
        } else {
            Write-Host "  ✗ $($lib.Name.PadRight(12)) - Empty placeholder" -ForegroundColor Red
            $missing++
        }
    } else {
        Write-Host "  ✗ $($lib.Name.PadRight(12)) - Not found" -ForegroundColor Red
        $missing++
    }
}

Write-Host ""
Write-Host "Ready: $ready / $($allLibs.Count)" -ForegroundColor $(if ($ready -eq $allLibs.Count) { "Green" } else { "Yellow" })

if ($missing -eq 0) {
    Write-Host ""
    Write-Host "All critical libraries built! Ready to build CBXShell." -ForegroundColor Green
    exit 0
} else {
    Write-Host ""
    Write-Host "Some libraries are missing. Will attempt to build anyway." -ForegroundColor Yellow
    exit 0
}
