# ===========================================================================
# Download-And-Build-Libraries.ps1
# Download prebuilt libraries or build from source for ExplorerLens
# ===========================================================================
# 
# ⚠️  DEPRECATED: Use Build-All-ExplorerLens-V7.ps1 instead
# This script is kept for reference only.
# See docs/development/PATH_UPDATE_SUMMARY_2026-02-16.md for current build workflow.
# 

$ErrorActionPreference = "Continue"
$ProgressPreference = "SilentlyContinue"

Write-Host ""
Write-Host "==========================================================================" -ForegroundColor Cyan
Write-Host "Download and Build External Libraries" -ForegroundColor Cyan
Write-Host "==========================================================================" -ForegroundColor Cyan
Write-Host ""

$ProjectRoot = $PSScriptRoot
Set-Location $ProjectRoot

# Check if vcpkg is available
$vcpkg = Get-Command vcpkg -ErrorAction SilentlyContinue

if ($vcpkg) {
    Write-Host "✓ vcpkg found: $($vcpkg.Source)" -ForegroundColor Green
    
    Write-Host ""
    Write-Host "Installing libraries via vcpkg..." -ForegroundColor Cyan
    Write-Host "This may take a while..." -ForegroundColor Yellow
    Write-Host ""
    
    $packages = @(
        "zlib:x64-windows-static",
        "lz4:x64-windows-static",
        "zstd:x64-windows-static",
        "bzip2:x64-windows-static",
        "liblzma:x64-windows-static",
        "minizip-ng[zlib,bzip2,lzma,zstd]:x64-windows-static",
        "libwebp[all]:x64-windows-static"
    )
    
    foreach ($pkg in $packages) {
        Write-Host "Installing $pkg..." -ForegroundColor White
        & vcpkg install $pkg
    }
    
    # Copy libraries to expected locations
    $vcpkgRoot = & vcpkg integrate install 2>&1 | Select-String "installed" | ForEach-Object { $_ -replace ".*: ", "" }
    
    if ($vcpkgRoot) {
        Write-Host ""
        Write-Host "Copying libraries from vcpkg..." -ForegroundColor Cyan
        
        # Map vcpkg libraries to expected locations
        $libMappings = @{
            "zlib.lib"        = "external\compression\zlib-1.3.1\build-vs\Release\zlibstatic.lib"
            "lz4.lib"         = "external\compression\lz4-1.10.0\build-vs\Release\liblz4_static.lib"
            "zstd_static.lib" = "external\compression\zstd-1.5.7\build-vs\lib\Release\zstd_static.lib"
            "bz2.lib"         = "external\compression\bzip2-1.0.8\x64\Release\bzip2.lib"
            "lzma.lib"        = "external\compression-libs\lzma-26.00\build-vs\Release\lzma.lib"
            "minizip.lib"     = "external\compression\minizip-ng-4.0.10\build-vs\Release\minizip.lib"
            "webp.lib"        = "external\image-libs\libwebp-1.5.0-build\build-vs\Release\webp.lib"
            "sharpyuv.lib"    = "external\image-libs\libwebp-1.5.0-build\build-vs\Release\sharpyuv.lib"
        }
        
        $vcpkgLibDir = Join-Path $vcpkgRoot "installed\x64-windows-static\lib"
        
        foreach ($srcLib in $libMappings.Keys) {
            $src = Join-Path $vcpkgLibDir $srcLib
            $dst = Join-Path $ProjectRoot $libMappings[$srcLib]
            
            if (Test-Path $src) {
                $dstDir = Split-Path $dst
                if (-not (Test-Path $dstDir)) {
                    New-Item -ItemType Directory -Path $dstDir -Force | Out-Null
                }
                
                Copy-Item $src $dst -Force
                $size = (Get-Item $dst).Length
                Write-Host "  ✓ Copied $srcLib -> $(Split-Path $dst -Leaf) ($([Math]::Round($size/1KB, 1)) KB)" -ForegroundColor Green
            } else {
                Write-Host "  ✗ Not found: $srcLib" -ForegroundColor Yellow
            }
        }
    }
    
} else {
    Write-Host "⚠ vcpkg not found" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "To install vcpkg:" -ForegroundColor Cyan
    Write-Host "  git clone https://github.com/microsoft/vcpkg" -ForegroundColor White
    Write-Host "  cd vcpkg" -ForegroundColor White
    Write-Host "  .\\bootstrap-vcpkg.bat" -ForegroundColor White
    Write-Host "  .\\vcpkg integrate install" -ForegroundColor White
    Write-Host ""
    Write-Host "Or install with scoop:" -ForegroundColor Cyan
    Write-Host "  scoop install vcpkg" -ForegroundColor White
    Write-Host ""
    
    # Fall back to individual build scripts
    Write-Host "Falling back to individual build scripts..." -ForegroundColor Yellow
    Write-Host ""
    
    # Try to build each library
    $buildScripts = @(
        "build-scripts\Build-Zlib.ps1",
        "build-scripts\Build-LZ4.ps1",
        "build-scripts\build-zstd-1.5.7.ps1",
        "build-scripts\build-libwebp-1.5.ps1"
    )
    
    foreach ($script in $buildScripts) {
        if (Test-Path $script) {
            Write-Host "Running $script..." -ForegroundColor White
            try {
                & ".\ $script" 2>&1 | Select-String "SUCCESS|FAILED|ERROR|error" | Select-Object -Last 5
            } catch {
                Write-Host "  Failed: $_" -ForegroundColor Red
            }
            Write-Host ""
        }
    }
}

# =============================================================================
# Final Status Check
# =============================================================================
Write-Host ""
Write-Host "==========================================================================" -ForegroundColor Cyan
Write-Host "Library Status" -ForegroundColor Cyan
Write-Host "==========================================================================" -ForegroundColor Cyan

$allLibs = @(
    @{Name = "Zlib"; Path = "external\compression\zlib-1.3.1\build-vs\Release\zlibstatic.lib"; Required = $true },
    @{Name = "LZ4"; Path = "external\compression\lz4-1.10.0\build\VS2022\liblz4\bin\x64_Release\liblz4_static.lib"; Required = $true },
    @{Name = "Zstd"; Path = "external\compression\zstd-1.5.7\build-vs\lib\Release\zstd_static.lib"; Required = $false },
    @{Name = "Bzip2"; Path = "external\compression\bzip2-1.0.8\x64\Release\bzip2.lib"; Required = $false },
    @{Name = "LZMA"; Path = "external\compression-libs\lzma-26.00\build-vs\Release\lzma.lib"; Required = $false },
    @{Name = "MinizipNG"; Path = "external\compression\minizip-ng-4.0.10\build-vs\Release\minizip.lib"; Required = $true },
    @{Name = "WebP"; Path = "external\image-libs\libwebp-1.5.0-build\build-vs\Release\webp.lib"; Required = $true },
    @{Name = "SharpYUV"; Path = "external\image-libs\libwebp-1.5.0-build\build-vs\Release\sharpyuv.lib"; Required = $true }
)

$ready = 0
$missing = 0
$missingRequired = 0

foreach ($lib in $allLibs) {
    if (Test-Path $lib.Path) {
        $size = (Get-Item $lib.Path).Length
        if ($size -gt 1KB) {
            $req = if ($lib.Required) { "[REQ]" } else { "[OPT]" }
            Write-Host "  ✓ $req $($lib.Name.PadRight(12)) - $([Math]::Round($size/1KB, 1)) KB" -ForegroundColor Green
            $ready++
        } else {
            $req = if ($lib.Required) { "[REQ]" } else { "[OPT]" }
            Write-Host "  ⚠ $req $($lib.Name.PadRight(12)) - Empty placeholder" -ForegroundColor Yellow
            $missing++
            if ($lib.Required) { $missingRequired++ }
        }
    } else {
        $req = if ($lib.Required) { "[REQ]" } else { "[OPT]" }
        Write-Host "  ✗ $req $($lib.Name.PadRight(12)) - Not found" -ForegroundColor Red
        $missing++
        if ($lib.Required) { $missingRequired++ }
    }
}

Write-Host ""
Write-Host "Ready: $ready / $($allLibs.Count)" -ForegroundColor $(if ($ready -eq $allLibs.Count) { "Green" } elseif ($missingRequired -eq 0) { "Yellow" } else { "Red" })

if ($missingRequired -gt 0) {
    Write-Host ""
    Write-Host "Missing $missingRequired required libraries!" -ForegroundColor Red
    Write-Host "Build will likely fail." -ForegroundColor Yellow
} elseif ($missing -eq 0) {
    Write-Host ""
    Write-Host "All libraries ready! You can now build LENSShell." -ForegroundColor Green
} else {
    Write-Host ""
    Write-Host "Optional libraries missing. Build may succeed with reduced functionality." -ForegroundColor Yellow
}

