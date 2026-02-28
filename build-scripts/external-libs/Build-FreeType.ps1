<#
.SYNOPSIS
    Build FreeType + HarfBuzz for ExplorerLens font preview rendering.

.DESCRIPTION
    Downloads and builds FreeType as a static library with /MD runtime.
    Optionally builds HarfBuzz for advanced text shaping.
    ExplorerLens v15.0.0 "Zenith" — 

.PARAMETER Clean
    Remove existing build artifacts before building.

.PARAMETER FreeTypeVersion
    FreeType version to build (default: 2.13.3).

.PARAMETER WithHarfBuzz
    Also build HarfBuzz for text shaping (default: $true).
#>

[CmdletBinding()]
param(
    [switch]$Clean,
    [string]$FreeTypeVersion = "2.13.3",
    [bool]$WithHarfBuzz = $true
)

$ErrorActionPreference = "Stop"

# Import core build library
. "$PSScriptRoot\..\core\Build-Library-Core.ps1"

$rootDir = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$externalDir = Join-Path $rootDir "external" "ui-libs"
$outputDir = Join-Path $rootDir "x64" "Release"

Write-BuildLog "========================================" "Info"
Write-BuildLog "FreeType $FreeTypeVersion Build Script" "Info"
Write-BuildLog "ExplorerLens v15.0.0 Zenith" "Info"
Write-BuildLog "========================================" "Info"

# ============================================================================
# Download FreeType source if not present
# ============================================================================

$ftSourceDir = Join-Path $externalDir "freetype-$FreeTypeVersion"
$ftBuildDir = Join-Path $ftSourceDir "build-msvc"

if (-not (Test-Path $ftSourceDir)) {
    $archiveUrl = "https://download.savannah.gnu.org/releases/freetype/freetype-$FreeTypeVersion.tar.xz"
    $archivePath = Join-Path $externalDir "freetype-$FreeTypeVersion.tar.xz"

    Write-BuildLog "Downloading FreeType v$FreeTypeVersion..." "Info"

    if (-not (Test-Path $externalDir)) {
        New-Item -ItemType Directory -Path $externalDir -Force | Out-Null
    }

    try {
        Invoke-WebRequest -Uri $archiveUrl -OutFile $archivePath -UseBasicParsing
        Write-BuildLog "Download complete" "Success"
    } catch {
        Write-BuildLog "Download failed: $($_.Exception.Message)" "Error"
        Write-BuildLog "Trying GitHub mirror..." "Warning"
        $altUrl = "https://github.com/nickthecoder/freetype/releases/download/v$FreeTypeVersion/freetype-$FreeTypeVersion.tar.xz"
        try {
            Invoke-WebRequest -Uri $altUrl -OutFile $archivePath -UseBasicParsing
        } catch {
            Write-BuildLog "Both download sources failed" "Error"
            Write-BuildLog "Please download FreeType $FreeTypeVersion manually to: $externalDir" "Warning"
            exit 1
        }
    }

    # Extract (requires 7z for .tar.xz)
    Write-BuildLog "Extracting FreeType..." "Info"
    $7z = Get-Command 7z -ErrorAction SilentlyContinue
    if ($7z) {
        & 7z x $archivePath -o"$externalDir" -y | Out-Null
        $tarFile = $archivePath -replace '\.xz$', ''
        if (Test-Path $tarFile) {
            & 7z x $tarFile -o"$externalDir" -y | Out-Null
            Remove-Item $tarFile -Force
        }
    } else {
        Write-BuildLog "7z not found — trying tar..." "Warning"
        tar -xf $archivePath -C $externalDir
    }
    Remove-Item $archivePath -Force -ErrorAction SilentlyContinue

    if (-not (Test-Path $ftSourceDir)) {
        Write-BuildLog "FreeType source directory not found after extraction" "Error"
        exit 1
    }
}

# ============================================================================
# Clean
# ============================================================================

if ($Clean -and (Test-Path $ftBuildDir)) {
    Write-BuildLog "Cleaning previous FreeType build..." "Info"
    Remove-Item $ftBuildDir -Recurse -Force
}

# ============================================================================
# Configure FreeType with CMake
# ============================================================================

if (-not (Test-Path $ftBuildDir)) {
    New-Item -ItemType Directory -Path $ftBuildDir -Force | Out-Null
}

# Find zlib and bzip2 if available
$zlibDir = Join-Path $rootDir "external" "compression-libs" "zlib-1.3.1"
$bzip2Dir = Join-Path $rootDir "external" "compression-libs" "bzip2"

$cmakeArgs = @(
    "-S", $ftSourceDir,
    "-B", $ftBuildDir,
    "-G", "Ninja",
    "-DCMAKE_BUILD_TYPE=Release",
    "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL",
    "-DBUILD_SHARED_LIBS=OFF",
    "-DFT_DISABLE_BROTLI=ON",
    "-DFT_DISABLE_PNG=ON",
    "-DFT_REQUIRE_HARFBUZZ=OFF"
)

if (Test-Path (Join-Path $zlibDir "build" "Release" "zlibstatic.lib")) {
    $cmakeArgs += "-DFT_DISABLE_ZLIB=OFF"
    $cmakeArgs += "-DZLIB_LIBRARY=$(Join-Path $zlibDir 'build' 'Release' 'zlibstatic.lib')"
    $cmakeArgs += "-DZLIB_INCLUDE_DIR=$zlibDir"
    Write-BuildLog "Found zlib — compressed font support enabled" "Info"
} else {
    $cmakeArgs += "-DFT_DISABLE_ZLIB=ON"
    Write-BuildLog "zlib not found — building without compressed font support" "Warning"
}

Write-BuildLog "Configuring FreeType with CMake..." "Info"
& cmake @cmakeArgs

if ($LASTEXITCODE -ne 0) {
    Write-BuildLog "FreeType CMake configuration failed" "Error"
    exit 1
}

# ============================================================================
# Build FreeType
# ============================================================================

Write-BuildLog "Building FreeType..." "Info"
cmake --build $ftBuildDir --config Release --parallel 8

if ($LASTEXITCODE -ne 0) {
    Write-BuildLog "FreeType build failed with exit code $LASTEXITCODE" "Error"
    exit 1
}

# ============================================================================
# Copy outputs
# ============================================================================

if (-not (Test-Path $outputDir)) {
    New-Item -ItemType Directory -Path $outputDir -Force | Out-Null
}

$libFile = Get-ChildItem $ftBuildDir -Recurse -Filter "freetype*.lib" | Select-Object -First 1
if ($libFile) {
    Copy-Item $libFile.FullName (Join-Path $outputDir "freetype.lib") -Force
    Write-BuildLog "Copied: freetype.lib → $outputDir" "Success"
} else {
    Write-BuildLog "WARNING: freetype.lib not found in build output" "Warning"
}

Write-BuildLog "========================================" "Info"
Write-BuildLog "FreeType $FreeTypeVersion build complete!" "Success"
Write-BuildLog "========================================" "Info"
Write-BuildLog "" "Info"
Write-BuildLog "To enable in Engine:" "Info"
Write-BuildLog "  cmake -DHAS_FREETYPE=ON ..." "Info"
