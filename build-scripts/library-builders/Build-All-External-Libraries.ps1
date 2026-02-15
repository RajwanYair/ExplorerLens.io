# ===========================================================================
# Build-All-External-Libraries.ps1
# Build all external libraries required for DarkThumbs
# ===========================================================================

param(
    [switch]$Force
)

$ErrorActionPreference = "Continue"
$ProgressPreference = "SilentlyContinue"

Write-Host ""
Write-Host "==========================================================================" -ForegroundColor Cyan
Write-Host "Building All External Libraries for DarkThumbs v6.2" -ForegroundColor Cyan
Write-Host "==========================================================================" -ForegroundColor Cyan
Write-Host ""

$ProjectRoot = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
Set-Location $ProjectRoot

# Find tools
$vcvarsall = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"
$cmake = Get-Command cmake -ErrorAction SilentlyContinue
$ninja = Get-Command ninja -ErrorAction SilentlyContinue
$msbuildScript = Join-Path $ProjectRoot "build-scripts\Find-MSBuild.ps1"
$msbuild = if (Test-Path $msbuildScript) { & $msbuildScript } else { "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\MSBuild\Current\Bin\amd64\MSBuild.exe" }

if (-not (Test-Path $vcvarsall)) {
    Write-Host "ERROR: vcvarsall.bat not found!" -ForegroundColor Red
    exit 1
}

if (-not $cmake) {
    Write-Host "ERROR: CMake not found! Install with: scoop install cmake" -ForegroundColor Red
    exit 1
}

if (-not $msbuild) {
    Write-Host "ERROR: MSBuild not found!" -ForegroundColor Red
    exit 1
}

Write-Host "✓ vcvarsall: $vcvarsall" -ForegroundColor Green
Write-Host "✓ CMake: $($cmake.Path)" -ForegroundColor Green
Write-Host "✓ MSBuild: $msbuild" -ForegroundColor Green
if ($ninja) { Write-Host "✓ Ninja: $($ninja.Path)" -ForegroundColor Green }
Write-Host ""

# Helper function to run commands in VS environment
function Invoke-VSCommand {
    param(
        [string]$Command,
        [string]$WorkingDir = $PWD
    )
    
    $batchFile = [System.IO.Path]::GetTempFileName() + ".cmd"
    $outputFile = [System.IO.Path]::GetTempFileName()
    
    @"
@echo off
call "$vcvarsall" x64 >nul 2>&1
cd /d "$WorkingDir"
$Command > "$outputFile" 2>&1
"@ | Out-File -FilePath $batchFile -Encoding ASCII
    
    $process = Start-Process -FilePath "cmd.exe" -ArgumentList "/c `"$batchFile`"" -Wait -PassThru -NoNewWindow
    
    if (Test-Path $outputFile) {
        $output = Get-Content $outputFile -Raw
        Remove-Item $outputFile -Force -ErrorAction SilentlyContinue
    }
    
    Remove-Item $batchFile -Force -ErrorAction SilentlyContinue
    
    return @{
        ExitCode = $process.ExitCode
        Output   = $output
    }
}

# Build function for CMake-based libraries
function Build-CMakeLibrary {
    param(
        [string]$Name,
        [string]$SourceDir,
        [string]$BuildDir,
        [hashtable]$CMakeOptions = @{},
        [string]$OutputLib,
        [string]$Generator = "Ninja"
    )
    
    Write-Host "Building $Name..." -ForegroundColor Yellow
    
    if (-not (Test-Path $SourceDir)) {
        Write-Host "  [SKIP] Source not found: $SourceDir" -ForegroundColor Gray
        return $false
    }
    
    # Check if already built
    if ((Test-Path $OutputLib) -and -not $Force) {
        $size = (Get-Item $OutputLib).Length
        if ($size -gt 1KB) {
            Write-Host "  [SKIP] Already built ($([Math]::Round($size/1KB, 1)) KB)" -ForegroundColor Gray
            return $true
        }
    }
    
    # Create build directory
    if (Test-Path $BuildDir) {
        Remove-Item $BuildDir -Recurse -Force -ErrorAction SilentlyContinue
    }
    New-Item -ItemType Directory -Path $BuildDir -Force | Out-Null
    
    # Build CMake arguments
    $cmakeArgs = "-G `"$Generator`" -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF"
    foreach ($key in $CMakeOptions.Keys) {
        $cmakeArgs += " -D$key=`"$($CMakeOptions[$key])`""
    }
    
    # Configure
    Write-Host "  Configuring..." -ForegroundColor White
    $result = Invoke-VSCommand -Command "cmake `"$SourceDir`" -B `"$BuildDir`" $cmakeArgs" -WorkingDir $BuildDir
    
    if ($result.ExitCode -ne 0) {
        Write-Host "  [FAILED] Configuration failed!" -ForegroundColor Red
        Write-Host $result.Output
        return $false
    }
    
    # Build
    Write-Host "  Building..." -ForegroundColor White
    $result = Invoke-VSCommand -Command "cmake --build `"$BuildDir`" --config Release --parallel" -WorkingDir $BuildDir
    
    if ($result.ExitCode -ne 0) {
        Write-Host "  [FAILED] Build failed!" -ForegroundColor Red
        Write-Host $result.Output
        return $false
    }
    
    # Verify output
    if (Test-Path $OutputLib) {
        $size = (Get-Item $OutputLib).Length
        Write-Host "  [SUCCESS] Built $([Math]::Round($size/1KB, 1)) KB" -ForegroundColor Green
        return $true
    } else {
        Write-Host "  [FAILED] Output file not found: $OutputLib" -ForegroundColor Red
        return $false
    }
}

# Build function for MSBuild-based libraries
function Build-MSBuildLibrary {
    param(
        [string]$Name,
        [string]$SolutionFile,
        [string]$Project = "",
        [string]$OutputLib
    )
    
    Write-Host "Building $Name..." -ForegroundColor Yellow
    
    if (-not (Test-Path $SolutionFile)) {
        Write-Host "  [SKIP] Solution not found: $SolutionFile" -ForegroundColor Gray
        return $false
    }
    
    # Check if already built
    if ((Test-Path $OutputLib) -and -not $Force) {
        $size = (Get-Item $OutputLib).Length
        if ($size -gt 1KB) {
            Write-Host "  [SKIP] Already built ($([Math]::Round($size/1KB, 1)) KB)" -ForegroundColor Gray
            return $true
        }
    }
    
    Write-Host "  Building with MSBuild..." -ForegroundColor White
    
    $buildArgs = "`"$SolutionFile`" /p:Configuration=Release /p:Platform=x64 /m /v:minimal /nologo"
    if ($Project) {
        $buildArgs += " /t:$Project"
    }
    
    $output = & $msbuild $buildArgs.Split(' ') 2>&1 | Out-String
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "  [FAILED] Build failed!" -ForegroundColor Red
        $output | Select-String "error" | Select-Object -First 5 | ForEach-Object { Write-Host "    $_" -ForegroundColor Red }
        return $false
    }
    
    # Verify output
    if (Test-Path $OutputLib) {
        $size = (Get-Item $OutputLib).Length
        Write-Host "  [SUCCESS] Built $([Math]::Round($size/1KB, 1)) KB" -ForegroundColor Green
        return $true
    } else {
        Write-Host "  [FAILED] Output file not found: $OutputLib" -ForegroundColor Red
        return $false
    }
}

# =============================================================================
# Library Definitions
# =============================================================================

$libraries = @(
    # 1. Zlib
    @{
        Name         = "Zlib 1.3.1"
        Type         = "CMake"
        SourceDir    = "external\compression\zlib-1.3.1"
        BuildDir     = "external\compression\zlib-1.3.1\build-vs"
        OutputLib    = "external\compression\zlib-1.3.1\build-vs\Release\zlibstatic.lib"
        CMakeOptions = @{}
    },
    
    # 2. LZ4
    @{
        Name         = "LZ4 1.10.0"
        Type         = "MSBuild"
        SolutionFile = "external\compression\lz4-1.10.0\build\VS2022\lz4.sln"
        Project      = "liblz4"
        OutputLib    = "external\compression\lz4-1.10.0\build\VS2022\liblz4\bin\x64_Release\liblz4_static.lib"
    },
    
    # 3. Zstd
    @{
        Name         = "Zstd 1.5.7"
        Type         = "CMake"
        SourceDir    = "external\compression\zstd-1.5.7\build\cmake"
        BuildDir     = "external\compression\zstd-1.5.7\build-vs"
        OutputLib    = "external\compression\zstd-1.5.7\build-vs\lib\Release\zstd_static.lib"
        CMakeOptions = @{
            "ZSTD_BUILD_PROGRAMS" = "OFF"
            "ZSTD_BUILD_CONTRIB"  = "OFF"
            "ZSTD_BUILD_TESTS"    = "OFF"
            "ZSTD_BUILD_SHARED"   = "OFF"
            "ZSTD_BUILD_STATIC"   = "ON"
        }
    },
    
    # 4. Bzip2
    @{
        Name         = "Bzip2 1.0.8"
        Type         = "CMake"
        SourceDir    = "external\compression\bzip2-1.0.8"
        BuildDir     = "external\compression\bzip2-1.0.8\build-vs"
        OutputLib    = "external\compression\bzip2-1.0.8\build-vs\Release\bz2_static.lib"
        CMakeOptions = @{}
    },
    
    # 5. LZMA
    @{
        Name         = "LZMA 26.00"
        Type         = "CMake"
        SourceDir    = "external\compression-libs\lzma-26.00"
        BuildDir     = "external\compression-libs\lzma-26.00\build-vs"
        OutputLib    = "external\compression-libs\lzma-26.00\build-vs\Release\lzma.lib"
        CMakeOptions = @{}
    },
    
    # 6. MinizipNG
    @{
        Name         = "MinizipNG 4.0.10"
        Type         = "CMake"
        SourceDir    = "external\compression\minizip-ng-4.0.10"
        BuildDir     = "external\compression\minizip-ng-4.0.10\build-vs"
        OutputLib    = "external\compression\minizip-ng-4.0.10\build-vs\Release\minizip.lib"
        CMakeOptions = @{
            "MZ_COMPAT"    = "OFF"
            "MZ_ZLIB"      = "ON"
            "MZ_BZIP2"     = "ON"
            "MZ_LZMA"      = "ON"
            "MZ_ZSTD"      = "ON"
            "ZLIB_ROOT"    = (Resolve-Path "external\compression\zlib-1.3.1").Path
            "BZIP2_ROOT"   = (Resolve-Path "external\compression\bzip2-1.0.8").Path
            "ZSTD_ROOT"    = (Resolve-Path "external\compression\zstd-1.5.7").Path
            "LIBLZMA_ROOT" = (Resolve-Path "external\compression-libs\lzma-26.00").Path
        }
    },
    
    # 7. LibWebP
    @{
        Name         = "LibWebP 1.5.0"
        Type         = "CMake"
        SourceDir    = "external\image-libs\libwebp-1.5.0"
        BuildDir     = "external\image-libs\libwebp-1.5.0\build-vs"
        OutputLib    = "external\image-libs\libwebp-1.5.0\build-vs\Release\webp.lib"
        CMakeOptions = @{
            "WEBP_BUILD_ANIM_UTILS" = "OFF"
            "WEBP_BUILD_CWEBP"      = "OFF"
            "WEBP_BUILD_DWEBP"      = "OFF"
            "WEBP_BUILD_GIF2WEBP"   = "OFF"
            "WEBP_BUILD_IMG2WEBP"   = "OFF"
            "WEBP_BUILD_VWEBP"      = "OFF"
            "WEBP_BUILD_WEBPINFO"   = "OFF"
            "WEBP_BUILD_WEBPMUX"    = "OFF"
            "WEBP_BUILD_EXTRAS"     = "OFF"
        }
    }
)

# =============================================================================
# Build All Libraries
# =============================================================================

$results = @()
$successCount = 0
$failCount = 0

foreach ($lib in $libraries) {
    $success = $false
    
    if ($lib.Type -eq "CMake") {
        $success = Build-CMakeLibrary `
            -Name $lib.Name `
            -SourceDir (Join-Path $ProjectRoot $lib.SourceDir) `
            -BuildDir (Join-Path $ProjectRoot $lib.BuildDir) `
            -OutputLib (Join-Path $ProjectRoot $lib.OutputLib) `
            -CMakeOptions $lib.CMakeOptions
    } elseif ($lib.Type -eq "MSBuild") {
        $success = Build-MSBuildLibrary `
            -Name $lib.Name `
            -SolutionFile (Join-Path $ProjectRoot $lib.SolutionFile) `
            -Project $lib.Project `
            -OutputLib (Join-Path $ProjectRoot $lib.OutputLib)
    }
    
    $results += @{
        Name      = $lib.Name
        Success   = $success
        OutputLib = $lib.OutputLib
    }
    
    if ($success) { $successCount++ } else { $failCount++ }
    Write-Host ""
}

# =============================================================================
# Copy/link libraries to expected locations
# =============================================================================

Write-Host "Setting up library paths..." -ForegroundColor Cyan

# Copy Bzip2 to expected location
$bzip2Source = "external\compression\bzip2-1.0.8\build-vs\Release\bz2_static.lib"
$bzip2Target = "external\compression\bzip2-1.0.8\x64\Release\bzip2.lib"
if (Test-Path $bzip2Source) {
    $targetDir = Split-Path $bzip2Target
    if (-not (Test-Path $targetDir)) {
        New-Item -ItemType Directory -Path $targetDir -Force | Out-Null
    }
    Copy-Item $bzip2Source $bzip2Target -Force
    Write-Host "  ✓ Copied Bzip2 to $bzip2Target" -ForegroundColor Green
}

# Copy WebP sharpyuv library
$sharpyuvSource = "external\image-libs\libwebp-1.5.0\build-vs\Release\sharpyuv.lib"
if (Test-Path $sharpyuvSource) {
    Write-Host "  ✓ SharpYUV library found" -ForegroundColor Green
}

Write-Host ""

# =============================================================================
# Summary
# =============================================================================

Write-Host "==========================================================================" -ForegroundColor Cyan
Write-Host "Build Summary" -ForegroundColor Cyan
Write-Host "==========================================================================" -ForegroundColor Cyan
Write-Host ""

foreach ($result in $results) {
    $fullPath = Join-Path $ProjectRoot $result.OutputLib
    if (Test-Path $fullPath) {
        $size = (Get-Item $fullPath).Length
        Write-Host "  ✓ $($result.Name.PadRight(20)) - $([Math]::Round($size/1KB, 1)) KB" -ForegroundColor Green
    } else {
        Write-Host "  ✗ $($result.Name.PadRight(20)) - FAILED" -ForegroundColor Red
    }
}

Write-Host ""
Write-Host "Success: $successCount" -ForegroundColor Green
Write-Host "Failed:  $failCount" -ForegroundColor $(if ($failCount -eq 0) { "Green" } else { "Red" })
Write-Host ""

if ($failCount -eq 0) {
    Write-Host "All libraries built successfully!" -ForegroundColor Green
    Write-Host "Ready to build CBXShell.sln" -ForegroundColor Cyan
    exit 0
} else {
    Write-Host "Some libraries failed to build." -ForegroundColor Yellow
    Write-Host "CBXShell may still build if failed libraries are optional." -ForegroundColor Yellow
    exit 1
}
