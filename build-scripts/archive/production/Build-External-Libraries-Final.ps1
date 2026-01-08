# Build-External-Libraries-Final.ps1
# Final comprehensive build script for all DarkThumbs external libraries

param(
    [int]$TimeoutSeconds = 600
)

$ErrorActionPreference = "Continue"
$ProgressPreference = "SilentlyContinue"

$rootDir = "C:\Users\ryair\OneDrive - Intel Corporation\Documents\MyScripts\DarkThumbs"
Set-Location $rootDir

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Dark Thumbs External Libraries Builder" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

$msbuild = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\MSBuild\Current\Bin\amd64\MSBuild.exe"
if (-not (Test-Path $msbuild)) {
    Write-Host "ERROR: MSBuild not found" -ForegroundColor Red
    exit 1
}

function Build-MinizipNG {
    Write-Host ">>> Building MinizipNG..." -ForegroundColor White
    
    $dir = Join-Path $rootDir "external\compression\minizip-ng-4.0.10"
    $buildDir = Join-Path $dir "build-vs"
    $outputLib = Join-Path $buildDir "Release\minizip.lib"
    
    # Clean old build
    if (Test-Path $buildDir) {
        Remove-Item -Path $buildDir -Recurse -Force -ErrorAction SilentlyContinue
    }
    
    New-Item -ItemType Directory -Path $buildDir -Force | Out-Null
    Push-Location $buildDir
    
    try {
        Write-Host "  Configuring..." -ForegroundColor Cyan
        cmake .. -G "Visual Studio 18 2026" -A x64 `
            -DCMAKE_BUILD_TYPE=Release `
            -DMZ_COMPAT=OFF `
            -DMZ_BZIP2=OFF `
            -DMZ_LZMA=OFF `
            -DMZ_ZSTD=OFF `
            -DMZ_OPENSSL=OFF `
            -DMZ_LIBBSD=OFF `
            2>&1 | Out-Null
        
        if ($LASTEXITCODE -eq 0) {
            Write-Host "  Building..." -ForegroundColor Cyan
            cmake --build . --config Release --target minizip 2>&1 | Out-Null
            
            if (Test-Path $outputLib) {
                $size = (Get-Item $outputLib).Length / 1KB
                Write-Host "  ✓ Success: $([math]::Round($size, 2)) KB" -ForegroundColor Green
                Pop-Location
                return $true
            }
        }
        
        Write-Host "  ✗ Build failed" -ForegroundColor Red
        Pop-Location
        return $false
    } catch {
        Write-Host "  ✗ Error: $($_.Exception.Message)" -ForegroundColor Red
        Pop-Location
        return $false
    }
}

function Build-WebP {
    Write-Host ">>> Building WebP..." -ForegroundColor White
    
    $dir = Join-Path $rootDir "external\image-libs\libwebp-1.5.0"
    $buildDir = Join-Path $dir "build-vs"
    $outputLib = Join-Path $buildDir "Release\webp.lib"
    $outputSharpYUV = Join-Path $buildDir "Release\sharpyuv.lib"
    
    # Clean old build
    if (Test-Path $buildDir) {
        Remove-Item -Path $buildDir -Recurse -Force -ErrorAction SilentlyContinue
    }
    
    New-Item -ItemType Directory -Path $buildDir -Force | Out-Null
    Push-Location $buildDir
    
    try {
        Write-Host "  Configuring..." -ForegroundColor Cyan
        cmake .. -G "Visual Studio 18 2026" -A x64 `
            -DCMAKE_BUILD_TYPE=Release `
            -DWEBP_BUILD_ANIM_UTILS=OFF `
            -DWEBP_BUILD_CWEBP=OFF `
            -DWEBP_BUILD_DWEBP=OFF `
            -DWEBP_BUILD_GIF2WEBP=OFF `
            -DWEBP_BUILD_IMG2WEBP=OFF `
            -DWEBP_BUILD_VWEBP=OFF `
            -DWEBP_BUILD_WEBPINFO=OFF `
            -DWEBP_BUILD_WEBPMUX=OFF `
            -DWEBP_BUILD_EXTRAS=OFF `
            2>&1 | Out-Null
        
        if ($LASTEXITCODE -eq 0) {
            Write-Host "  Building..." -ForegroundColor Cyan
            cmake --build . --config Release 2>&1 | Out-Null
            
            $success = $true
            if (Test-Path $outputLib) {
                $size = (Get-Item $outputLib).Length / 1KB
                Write-Host "  ✓ webp.lib: $([math]::Round($size, 2)) KB" -ForegroundColor Green
            } else {
                Write-Host "  ✗ webp.lib not found" -ForegroundColor Red
                $success = $false
            }
            
            if (Test-Path $outputSharpYUV) {
                $size = (Get-Item $outputSharpYUV).Length / 1KB
                Write-Host "  ✓ sharpyuv.lib: $([math]::Round($size, 2)) KB" -ForegroundColor Green
            } else {
                Write-Host "  ✗ sharpyuv.lib not found" -ForegroundColor Red
                $success = $false
            }
            
            Pop-Location
            return $success
        }
        
        Write-Host "  ✗ Build failed" -ForegroundColor Red
        Pop-Location
        return $false
    } catch {
        Write-Host "  ✗ Error: $($_.Exception.Message)" -ForegroundColor Red
        Pop-Location
        return $false
    }
}

function Build-LZMA {
    Write-Host ">>> Building LZMA..." -ForegroundColor White
    
    $dir = Join-Path $rootDir "external\compression\lzma-24.08"
    $buildDir = Join-Path $dir "build-vs"
    $outputLib = Join-Path $buildDir "Release\lzma.lib"
    
    # Clean old build
    if (Test-Path $buildDir) {
        Remove-Item -Path $buildDir -Recurse -Force -ErrorAction SilentlyContinue
    }
    
    New-Item -ItemType Directory -Path $buildDir -Force | Out-Null
    Push-Location $buildDir
    
    try {
        Write-Host "  Configuring..." -ForegroundColor Cyan
        cmake .. -G "Visual Studio 18 2026" -A x64 -DCMAKE_BUILD_TYPE=Release 2>&1 | Out-Null
        
        if ($LASTEXITCODE -eq 0) {
            Write-Host "  Building..." -ForegroundColor Cyan
            cmake --build . --config Release 2>&1 | Out-Null
            
            if (Test-Path $outputLib) {
                $size = (Get-Item $outputLib).Length / 1KB
                Write-Host "  ✓ Success: $([math]::Round($size, 2)) KB" -ForegroundColor Green
                Pop-Location
                return $true
            }
        }
        
        Write-Host "  ✗ Build failed" -ForegroundColor Red
        Pop-Location
        return $false
    } catch {
        Write-Host "  ✗ Error: $($_.Exception.Message)" -ForegroundColor Red
        Pop-Location
        return $false
    }
}

# Execute builds
$results = @{
    "Zlib"      = "Already built"
    "LZ4"       = "Already built"
    "Zstd"      = "Already built"
    "MinizipNG" = if (Build-MinizipNG) { "Success" } else { "Failed" }
    "WebP"      = if (Build-WebP) { "Success" } else { "Failed" }
    "LZMA"      = if (Build-LZMA) { "Success" } else { "Failed" }
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Build Summary" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

foreach ($lib in $results.Keys | Sort-Object) {
    $status = $results[$lib]
    if ($status -eq "Success" -or $status -eq "Already built") {
        Write-Host "  ✓ $($lib.PadRight(15)) $status" -ForegroundColor Green
    } else {
        Write-Host "  ✗ $($lib.PadRight(15)) $status" -ForegroundColor Red
    }
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Final library check
Write-Host "Verifying all libraries..." -ForegroundColor Cyan
Write-Host ""

$finalLibs = @{
    "Zlib"      = "external\compression\zlib-1.3.1\build-vs\Release\zlibstatic.lib"
    "LZ4"       = "external\compression\lz4-1.10.0\build\VS2022\liblz4\bin\x64_Release\liblz4_static.lib"
    "Zstd"      = "external\compression\zstd-1.5.7\build\VS2010\bin\x64_Release\libzstd_static.lib"
    "LZMA"      = "external\compression\lzma-24.08\build-vs\Release\lzma.lib"
    "MinizipNG" = "external\compression\minizip-ng-4.0.10\build-vs\Release\minizip.lib"
    "WebP"      = "external\image-libs\libwebp-1.5.0\build-vs\Release\webp.lib"
    "SharpYUV"  = "external\image-libs\libwebp-1.5.0\build-vs\Release\sharpyuv.lib"
}

$availableCount = 0
foreach ($lib in $finalLibs.GetEnumerator() | Sort-Object Name) {
    if (Test-Path $lib.Value) {
        $size = (Get-Item $lib.Value).Length
        if ($size -gt 1000) {
            $sizeKB = $size / 1KB
            Write-Host "  ✓ $($lib.Name.PadRight(15))" -ForegroundColor Green -NoNewline
            Write-Host " $([math]::Round($sizeKB, 2)) KB" -ForegroundColor Gray
            $availableCount++
        } else {
            Write-Host "  ⚠ $($lib.Name.PadRight(15)) Too small ($size bytes)" -ForegroundColor Yellow
        }
    } else {
        Write-Host "  ✗ $($lib.Name.PadRight(15)) NOT FOUND" -ForegroundColor Red
    }
}

Write-Host ""
Write-Host "Total: $availableCount / 7 libraries ready" -ForegroundColor Cyan
Write-Host ""

if ($availableCount -ge 5) {
    Write-Host "✓ Sufficient libraries available to build CBXShell!" -ForegroundColor Green
} else {
    Write-Host "⚠ Some libraries still missing" -ForegroundColor Yellow
}
Write-Host ""
