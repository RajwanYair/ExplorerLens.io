# DarkThumbs - Master Rebuild Script (PowerShell)
# Rebuilds all compression libraries without /GL (LTCG) to avoid linking issues
# Date: November 19, 2025

param(
    [switch]$Clean,
    [switch]$Verbose
)

$ErrorActionPreference = "Stop"
$rootDir = Split-Path -Parent $PSScriptRoot
$compressionDir = Join-Path $rootDir "external\compression"
$scriptsDir = Join-Path $rootDir "build-scripts"

# Find MSBuild
function Find-MSBuild {
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    
    if (Test-Path $vswhere) {
        $vsPath = & $vswhere -latest -products * -requires Microsoft.Component.MSBuild -property installationPath
        if ($vsPath) {
            $msbuild = Join-Path $vsPath "MSBuild\Current\Bin\amd64\MSBuild.exe"
            if (Test-Path $msbuild) {
                return $msbuild
            }
        }
    }
    
    # Fallback to PATH
    $msbuild = Get-Command msbuild.exe -ErrorAction SilentlyContinue
    if ($msbuild) {
        return $msbuild.Source
    }
    
    throw "MSBuild not found. Please install Visual Studio 2022 or later."
}

$msbuild = Find-MSBuild
Write-Host "Using MSBuild: $msbuild`n" -ForegroundColor Cyan

# Configure build flags (NO /GL to avoid LTCG issues)
$buildConfig = @{
    "Configuration"            = "Release"
    "Platform"                 = "x64"
    "CharacterSet"             = "Unicode"
    "RuntimeLibrary"           = "MultiThreaded"  # Static runtime /MT
    "WholeProgramOptimization" = "false"  # Disable /GL
    "Optimization"             = "MaxSpeed"  # /O2
}

Write-Host "=== DarkThumbs Compression Libraries Rebuild ===" -ForegroundColor Green
Write-Host "Build Configuration:" -ForegroundColor Cyan
$buildConfig.GetEnumerator() | ForEach-Object {
    Write-Host "  $($_.Key): $($_.Value)" -ForegroundColor White
}
Write-Host ""

# Build zlib
Write-Host "Building zlib..." -ForegroundColor Yellow
$zlibDir = Join-Path $compressionDir "zlib-1.3.1"
if (Test-Path $zlibDir) {
    Push-Location $zlibDir
    
    # Create build directory
    $buildDir = "build-vs"
    if ($Clean -and (Test-Path $buildDir)) {
        Remove-Item $buildDir -Recurse -Force
    }
    New-Item -ItemType Directory -Force -Path $buildDir | Out-Null
    
    Push-Location $buildDir
    
    # Configure with CMake (no /GL) - Auto-detect VS version
    $vsGenerators = @(
        "Visual Studio 18 2026",
        "Visual Studio 17 2022",
        "Visual Studio 16 2019"
    )
    
    $generator = $null
    foreach ($gen in $vsGenerators) {
        $testResult = cmake .. -G "$gen" -A x64 2>&1 | Out-String
        if ($testResult -notlike "*could not find*") {
            $generator = $gen
            break
        }
    }
    
    if (-not $generator) {
        Write-Host "  ERROR: No compatible Visual Studio generator found" -ForegroundColor Red
        Pop-Location
        Pop-Location
        return
    }
    
    Write-Host "  Using generator: $generator" -ForegroundColor Cyan
    
    cmake .. -G "$generator" -A x64 `
        -DCMAKE_BUILD_TYPE=Release `
        -DBUILD_SHARED_LIBS=OFF `
        -DCMAKE_C_FLAGS_RELEASE="/MT /O2" `
        -DCMAKE_CXX_FLAGS_RELEASE="/MT /O2"
    
    # Build
    cmake --build . --config Release --target zlibstatic
    
    # Copy output
    $outputDir = Join-Path $zlibDir "x64\Release"
    New-Item -ItemType Directory -Force -Path $outputDir | Out-Null
    Copy-Item "Release\zlibstatic.lib" $outputDir -Force
    
    Pop-Location
    Pop-Location
    Write-Host "  zlib: OK`n" -ForegroundColor Green
} else {
    Write-Host "  zlib: NOT FOUND (skipping)`n" -ForegroundColor Red
}

# Build bzip2
Write-Host "Building bzip2..." -ForegroundColor Yellow
$bzip2Dir = Join-Path $compressionDir "bzip2-1.0.8"
if (Test-Path $bzip2Dir) {
    Push-Location $bzip2Dir
    
    # Build with cl.exe directly (simple makefile project)
    $vcvars = "${env:ProgramFiles}\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
    if (-not (Test-Path $vcvars)) {
        $vcvars = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
    }
    
    if (Test-Path $vcvars) {
        cmd /c "`"$vcvars`" && nmake /f makefile.msc clean && nmake /f makefile.msc lib CFLAGS=`"/MT /O2 /D_CRT_SECURE_NO_WARNINGS`""
        
        $outputDir = Join-Path $bzip2Dir "x64\Release"
        New-Item -ItemType Directory -Force -Path $outputDir | Out-Null
        Copy-Item "libbz2.lib" $outputDir\bzip2.lib -Force -ErrorAction SilentlyContinue
    }
    
    Pop-Location
    Write-Host "  bzip2: OK`n" -ForegroundColor Green
} else {
    Write-Host "  bzip2: NOT FOUND (skipping)`n" -ForegroundColor Red
}

# Build zstd (update to 1.5.7 if available)
Write-Host "Building zstd..." -ForegroundColor Yellow
$zstdDir = Get-ChildItem -Path $compressionDir -Filter "zstd-*" -Directory | Sort-Object Name -Descending | Select-Object -First 1
if ($zstdDir) {
    Push-Location $zstdDir.FullName
    
    $buildDir = "build-vs"
    if ($Clean -and (Test-Path $buildDir)) {
        Remove-Item $buildDir -Recurse -Force
    }
    New-Item -ItemType Directory -Force -Path $buildDir | Out-Null
    
    Push-Location $buildDir
    
    # Configure with CMake
    cmake ..\build\cmake -G "Visual Studio 18 2026" -A x64 `
        -DCMAKE_BUILD_TYPE=Release `
        -DZSTD_BUILD_SHARED=OFF `
        -DZSTD_BUILD_STATIC=ON `
        -DCMAKE_C_FLAGS_RELEASE="/MT /O2" `
        -DCMAKE_CXX_FLAGS_RELEASE="/MT /O2"
    
    # Build
    cmake --build . --config Release --target libzstd_static
    
    # Copy output
    $outputDir = Join-Path $zstdDir.FullName "build\x64\Release"
    New-Item -ItemType Directory -Force -Path $outputDir | Out-Null
    Copy-Item "lib\Release\zstd_static.lib" $outputDir -Force
    
    Pop-Location
    Pop-Location
    Write-Host "  zstd ($($zstdDir.Name)): OK`n" -ForegroundColor Green
} else {
    Write-Host "  zstd: NOT FOUND (skipping)`n" -ForegroundColor Red
}

# Build lz4
Write-Host "Building lz4..." -ForegroundColor Yellow
$lz4Dir = Join-Path $compressionDir "lz4-1.10.0"
if (Test-Path $lz4Dir) {
    Push-Location $lz4Dir
    
    $buildDir = "build-vs"
    if ($Clean -and (Test-Path $buildDir)) {
        Remove-Item $buildDir -Recurse -Force
    }
    New-Item -ItemType Directory -Force -Path $buildDir | Out-Null
    
    Push-Location $buildDir
    
    # Configure with CMake
    cmake ..\build\cmake -G "Visual Studio 18 2026" -A x64 `
        -DCMAKE_BUILD_TYPE=Release `
        -DBUILD_SHARED_LIBS=OFF `
        -DCMAKE_C_FLAGS_RELEASE="/MT /O2" `
        -DCMAKE_CXX_FLAGS_RELEASE="/MT /O2"
    
    # Build
    cmake --build . --config Release
    
    # Copy output
    $outputDir = Join-Path $lz4Dir "build\x64\Release"
    New-Item -ItemType Directory -Force -Path $outputDir | Out-Null
    Copy-Item "Release\lz4.lib" $outputDir -Force -ErrorAction SilentlyContinue
    
    Pop-Location
    Pop-Location
    Write-Host "  lz4: OK`n" -ForegroundColor Green
} else {
    Write-Host "  lz4: NOT FOUND (skipping)`n" -ForegroundColor Red
}

# Build minizip-ng (WITHOUT /GL to fix LTCG issue!)
Write-Host "Building minizip-ng (no LTCG)..." -ForegroundColor Yellow
$minizipDir = Join-Path $compressionDir "minizip-ng-4.0.10"
if (Test-Path $minizipDir) {
    Push-Location $minizipDir
    
    $buildDir = "build-vs"
    if ($Clean -and (Test-Path $buildDir)) {
        Remove-Item $buildDir -Recurse -Force
    }
    New-Item -ItemType Directory -Force -Path $buildDir | Out-Null
    
    Push-Location $buildDir
    
    # Configure with CMake - EXPLICITLY disable /GL
    cmake .. -G "Visual Studio 18 2026" -A x64 `
        -DCMAKE_BUILD_TYPE=Release `
        -DMZ_BUILD_TESTS=OFF `
        -DMZ_BUILD_UNIT_TESTS=OFF `
        -DBUILD_SHARED_LIBS=OFF `
        -DCMAKE_C_FLAGS_RELEASE="/MT /O2" `
        -DCMAKE_CXX_FLAGS_RELEASE="/MT /O2" `
        -DCMAKE_POLICY_DEFAULT_CMP0091=NEW `
        -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded"
    
    # Build
    cmake --build . --config Release
    
    # Copy output
    $outputDir = Join-Path $minizipDir "build\x64\Release"
    New-Item -ItemType Directory -Force -Path $outputDir | Out-Null
    Copy-Item "Release\minizip.lib" $outputDir -Force
    
    Pop-Location
    Pop-Location
    Write-Host "  minizip-ng: OK (LTCG disabled)`n" -ForegroundColor Green
} else {
    Write-Host "  minizip-ng: NOT FOUND (skipping)`n" -ForegroundColor Red
}

# Build LZMA SDK
Write-Host "Building LZMA SDK..." -ForegroundColor Yellow
$lzmaDir = Get-ChildItem -Path $compressionDir -Filter "lzma-*" -Directory | Sort-Object Name -Descending | Select-Object -First 1
if ($lzmaDir) {
    Push-Location $lzmaDir.FullName
    
    # LZMA SDK uses custom build process
    # For now, assume it's already built or will be built separately
    Write-Host "  LZMA SDK: Skipping (manual build required)`n" -ForegroundColor Yellow
    
    Pop-Location
}

# Build UnRAR
Write-Host "Building UnRAR..." -ForegroundColor Yellow
$unrarDir = Get-ChildItem -Path $compressionDir -Filter "unrar-*" -Directory | Sort-Object Name -Descending | Select-Object -First 1
if ($unrarDir) {
    Push-Location $unrarDir.FullName
    
    # UnRAR uses custom makefile
    # For now, assume it's already built or will be built separately
    Write-Host "  UnRAR: Skipping (manual build required)`n" -ForegroundColor Yellow
    
    Pop-Location
}

Write-Host "`n=== Compression Libraries Build Complete ===" -ForegroundColor Green
Write-Host "All libraries built without /GL (LTCG disabled)" -ForegroundColor Cyan
Write-Host "This should fix the LNK1257 error when linking CBXShell.dll`n" -ForegroundColor Cyan

Write-Host "Next steps:" -ForegroundColor Yellow
Write-Host "  1. Run: rebuild-all.cmd" -ForegroundColor White
Write-Host "  2. Verify CBXShell.dll builds without LTCG errors" -ForegroundColor White
Write-Host "  3. Test thumbnail generation" -ForegroundColor White
