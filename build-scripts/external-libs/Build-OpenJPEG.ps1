<#
.SYNOPSIS
    Build OpenJPEG 2.5.x for ExplorerLens JPEG 2000 decoding.

.DESCRIPTION
    Downloads and builds OpenJPEG as a static library with /MD runtime.
    ExplorerLens v15.0.0 "Zenith" — Sprint 365

.PARAMETER Clean
    Remove existing build artifacts before building.

.PARAMETER Version
    OpenJPEG version to build (default: 2.5.3).
#>

[CmdletBinding()]
param(
    [switch]$Clean,
    [string]$Version = "2.5.3"
)

$ErrorActionPreference = "Stop"

# Import core build library
. "$PSScriptRoot\..\core\Build-Library-Core.ps1"

$rootDir = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$externalDir = Join-Path $rootDir "external" "image-libs"
$sourceDir = Join-Path $externalDir "openjpeg-$Version"
$buildDir = Join-Path $sourceDir "build-msvc"
$installDir = Join-Path $externalDir "openjpeg-install"
$outputDir = Join-Path $rootDir "x64" "Release"

Write-BuildLog "========================================" "Info"
Write-BuildLog "OpenJPEG $Version Build Script" "Info"
Write-BuildLog "ExplorerLens v15.0.0 Zenith" "Info"
Write-BuildLog "========================================" "Info"

# ============================================================================
# Download source if not present
# ============================================================================

if (-not (Test-Path $sourceDir)) {
    $archiveUrl = "https://github.com/uclouvain/openjpeg/archive/refs/tags/v$Version.zip"
    $archivePath = Join-Path $externalDir "openjpeg-$Version.zip"

    Write-BuildLog "Downloading OpenJPEG v$Version from GitHub..." "Info"

    if (-not (Test-Path $externalDir)) {
        New-Item -ItemType Directory -Path $externalDir -Force | Out-Null
    }

    try {
        Invoke-WebRequest -Uri $archiveUrl -OutFile $archivePath -UseBasicParsing
        Write-BuildLog "Download complete: $archivePath" "Success"
    } catch {
        Write-BuildLog "Download failed: $($_.Exception.Message)" "Error"
        Write-BuildLog "Please download OpenJPEG v$Version manually to: $externalDir" "Warning"
        exit 1
    }

    # Extract
    Write-BuildLog "Extracting OpenJPEG..." "Info"
    Expand-Archive -Path $archivePath -DestinationPath $externalDir -Force
    Remove-Item $archivePath -Force

    # GitHub archives extract as openjpeg-<version>, rename if needed
    $extractedDir = Join-Path $externalDir "openjpeg-$Version"
    if (-not (Test-Path $extractedDir)) {
        # Try with 'v' prefix removal
        $altDir = Get-ChildItem $externalDir -Directory | Where-Object { $_.Name -like "openjpeg*$Version*" } | Select-Object -First 1
        if ($altDir) {
            Rename-Item $altDir.FullName $extractedDir
        }
    }

    if (-not (Test-Path $sourceDir)) {
        Write-BuildLog "Source directory not found after extraction" "Error"
        exit 1
    }
}

# ============================================================================
# Clean build artifacts
# ============================================================================

if ($Clean -and (Test-Path $buildDir)) {
    Write-BuildLog "Cleaning previous build..." "Info"
    Remove-Item $buildDir -Recurse -Force
}

# ============================================================================
# Configure with CMake
# ============================================================================

if (-not (Test-Path $buildDir)) {
    New-Item -ItemType Directory -Path $buildDir -Force | Out-Null
}

$cmakeArgs = @(
    "-S", $sourceDir,
    "-B", $buildDir,
    "-G", "Ninja",
    "-DCMAKE_BUILD_TYPE=Release",
    "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL",
    "-DCMAKE_INSTALL_PREFIX=$installDir",
    "-DBUILD_SHARED_LIBS=OFF",
    "-DBUILD_STATIC_LIBS=ON",
    "-DBUILD_CODEC=OFF",
    "-DBUILD_TESTING=OFF",
    "-DBUILD_THIRDPARTY=OFF",
    "-DBUILD_DOC=OFF",
    "-DBUILD_JPIP=OFF",
    "-DBUILD_JAVA=OFF",
    "-DBUILD_MJ2=OFF",
    "-DBUILD_JPWL=OFF"
)

# Find zlib if available (optional dependency for compressed JP2)
$zlibDir = Join-Path $rootDir "external" "compression-libs" "zlib-1.3.1"
if (Test-Path (Join-Path $zlibDir "build" "Release" "zlibstatic.lib")) {
    $cmakeArgs += "-DZLIB_LIBRARY=$(Join-Path $zlibDir 'build' 'Release' 'zlibstatic.lib')"
    $cmakeArgs += "-DZLIB_INCLUDE_DIR=$zlibDir"
    Write-BuildLog "Found zlib — enabling compressed JP2 streams" "Info"
}

Write-BuildLog "Configuring OpenJPEG with CMake..." "Info"
Invoke-CMakeBuild -SourceDir $sourceDir -BuildDir $buildDir -ExtraCMakeArgs $cmakeArgs -ConfigureOnly

# ============================================================================
# Build
# ============================================================================

Write-BuildLog "Building OpenJPEG..." "Info"
cmake --build $buildDir --config Release --parallel 8

if ($LASTEXITCODE -ne 0) {
    Write-BuildLog "OpenJPEG build failed with exit code $LASTEXITCODE" "Error"
    exit 1
}

# ============================================================================
# Install to staging directory
# ============================================================================

Write-BuildLog "Installing OpenJPEG..." "Info"
cmake --install $buildDir --config Release

# ============================================================================
# Copy outputs
# ============================================================================

if (-not (Test-Path $outputDir)) {
    New-Item -ItemType Directory -Path $outputDir -Force | Out-Null
}

# Find and copy the static library
$libFile = Get-ChildItem $buildDir -Recurse -Filter "openjp2.lib" | Select-Object -First 1
if (-not $libFile) {
    $libFile = Get-ChildItem $installDir -Recurse -Filter "openjp2.lib" | Select-Object -First 1
}
if ($libFile) {
    Copy-Item $libFile.FullName (Join-Path $outputDir "openjp2.lib") -Force
    Write-BuildLog "Copied: openjp2.lib → $outputDir" "Success"
} else {
    Write-BuildLog "WARNING: openjp2.lib not found in build output" "Warning"
}

Write-BuildLog "========================================" "Info"
Write-BuildLog "OpenJPEG $Version build complete!" "Success"
Write-BuildLog "========================================" "Info"
Write-BuildLog "" "Info"
Write-BuildLog "To enable in Engine:" "Info"
Write-BuildLog "  cmake -DHAS_OPENJPEG=ON ..." "Info"
