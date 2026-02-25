<#
.SYNOPSIS
    Build FFmpeg minimal libraries for ExplorerLens video thumbnail extraction.

.DESCRIPTION
    Downloads and builds a minimal FFmpeg configuration (libavformat + libavcodec + swscale)
    as shared DLLs for dynamic loading. LGPL-compatible build only.
    ExplorerLens v15.0.0 "Zenith" — Sprints 367-368

.PARAMETER Clean
    Remove existing build artifacts before building.

.PARAMETER FFmpegVersion
    FFmpeg version to build (default: 7.1).

.PARAMETER SharedLibs
    Build as shared DLLs for dynamic loading (default: true).
    Required for LGPL compliance when statically linking is not desired.
#>

[CmdletBinding()]
param(
    [switch]$Clean,
    [string]$FFmpegVersion = "7.1",
    [bool]$SharedLibs = $true
)

$ErrorActionPreference = "Stop"

# Import core build library
. "$PSScriptRoot\..\core\Build-Library-Core.ps1"

$rootDir = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$externalDir = Join-Path $rootDir "external" "media-libs"
$sourceDir = Join-Path $externalDir "ffmpeg-$FFmpegVersion"
$buildDir = Join-Path $sourceDir "build-msvc"
$installDir = Join-Path $externalDir "ffmpeg-install"
$outputDir = Join-Path $rootDir "x64" "Release"

Write-BuildHeader "FFmpeg $FFmpegVersion Build Script"
Write-BuildLog "ExplorerLens v15.0.0 Zenith — Sprints 367-368" -Level Info
Write-BuildLog "LGPL-compatible minimal build for video thumbnails" -Level Info

# ============================================================================
# Verify or download MSYS2/MinGW for FFmpeg configure script
# ============================================================================

function Get-MSys2Path {
    # Check common locations
    $paths = @(
        "C:\msys64",
        "$env:USERPROFILE\scoop\apps\msys2\current",
        "C:\tools\msys64"
    )
    foreach ($p in $paths) {
        if (Test-Path (Join-Path $p "usr\bin\bash.exe")) {
            return $p
        }
    }
    return $null
}

$msys2Path = Get-MSys2Path
if (-not $msys2Path) {
    Write-BuildLog "MSYS2 not found. Installing via scoop..." -Level Warning
    try {
        $scoop = Get-Command scoop -ErrorAction SilentlyContinue
        if ($scoop) {
            & scoop install msys2
            $msys2Path = Get-MSys2Path
        }
    } catch {
        Write-BuildLog "Failed to install MSYS2 via scoop" -Level Warning
    }
    
    if (-not $msys2Path) {
        Write-BuildLog "MSYS2 is required for FFmpeg build (configure script)" -Level Error
        Write-BuildLog "Install via: scoop install msys2" -Level Info
        Write-BuildLog "Or download from: https://www.msys2.org/" -Level Info
        Write-BuildLog "" -Level Info
        Write-BuildLog "Alternative: Download pre-built FFmpeg shared DLLs:" -Level Info
        Write-BuildLog "  https://github.com/BtbN/FFmpeg-Builds/releases" -Level Info
        Write-BuildLog "  Place avformat-61.dll, avcodec-61.dll, swscale-8.dll in x64\Release\" -Level Info
        exit 1
    }
}

# ============================================================================
# Download FFmpeg source if not present
# ============================================================================

if (-not (Test-Path $sourceDir)) {
    $archiveUrl = "https://ffmpeg.org/releases/ffmpeg-$FFmpegVersion.tar.xz"
    $archivePath = Join-Path $externalDir "ffmpeg-$FFmpegVersion.tar.xz"

    Write-BuildLog "Downloading FFmpeg v$FFmpegVersion..." -Level Info

    if (-not (Test-Path $externalDir)) {
        New-Item -ItemType Directory -Path $externalDir -Force | Out-Null
    }

    try {
        Invoke-WebRequest -Uri $archiveUrl -OutFile $archivePath -UseBasicParsing
        Write-BuildLog "Download complete" -Level Success
    } catch {
        Write-BuildLog "Download failed: $($_.Exception.Message)" -Level Error
        Write-BuildLog "" -Level Info
        Write-BuildLog "Alternative: Download pre-built FFmpeg shared DLLs from:" -Level Info
        Write-BuildLog "  https://github.com/BtbN/FFmpeg-Builds/releases" -Level Info
        Write-BuildLog "  Download: ffmpeg-n$FFmpegVersion-latest-win64-lgpl-shared" -Level Info
        exit 1
    }

    # Extract (requires 7z for .tar.xz)
    Write-BuildLog "Extracting FFmpeg..." -Level Info
    $7z = Get-Command 7z -ErrorAction SilentlyContinue
    if ($7z) {
        & 7z x $archivePath -o"$externalDir" -y | Out-Null
        $tarFile = $archivePath -replace '\.xz$', ''
        if (Test-Path $tarFile) {
            & 7z x $tarFile -o"$externalDir" -y | Out-Null
            Remove-Item $tarFile -Force
        }
    } else {
        Write-BuildLog "7z not found. Install via: scoop install 7zip" -Level Error
        exit 1
    }
    Remove-Item $archivePath -Force -ErrorAction SilentlyContinue
}

# ============================================================================
# Download pre-built binaries (fast path) or build from source
# ============================================================================

$prebuiltUrl = "https://github.com/BtbN/FFmpeg-Builds/releases/download/latest/ffmpeg-n$FFmpegVersion-latest-win64-lgpl-shared.zip"
$prebuiltDir = Join-Path $externalDir "ffmpeg-prebuilt"

Write-BuildLog "Attempting pre-built LGPL shared library download..." -Level Info

try {
    $prebuiltZip = Join-Path $externalDir "ffmpeg-prebuilt.zip"
    
    if (-not (Test-Path $prebuiltDir)) {
        Invoke-WebRequest -Uri $prebuiltUrl -OutFile $prebuiltZip -UseBasicParsing -TimeoutSec 60
        Expand-Archive -Path $prebuiltZip -DestinationPath $prebuiltDir -Force
        Remove-Item $prebuiltZip -Force -ErrorAction SilentlyContinue
        Write-BuildLog "Pre-built FFmpeg download successful" -Level Success
    }

    # Copy DLLs and libs to output directory
    if (-not (Test-Path $outputDir)) {
        New-Item -ItemType Directory -Path $outputDir -Force | Out-Null
    }
    if (-not (Test-Path $installDir)) {
        New-Item -ItemType Directory -Path $installDir -Force | Out-Null
        New-Item -ItemType Directory -Path (Join-Path $installDir "bin") -Force | Out-Null
        New-Item -ItemType Directory -Path (Join-Path $installDir "lib") -Force | Out-Null
        New-Item -ItemType Directory -Path (Join-Path $installDir "include") -Force | Out-Null
    }

    # Find the actual directory inside the extracted archive
    $ffmpegExtracted = Get-ChildItem $prebuiltDir -Directory | Where-Object { $_.Name -like "ffmpeg*" } | Select-Object -First 1
    if (-not $ffmpegExtracted) {
        $ffmpegExtracted = Get-Item $prebuiltDir
    }
    $ffmpegBinDir = Join-Path $ffmpegExtracted.FullName "bin"
    $ffmpegLibDir = Join-Path $ffmpegExtracted.FullName "lib"
    $ffmpegIncDir = Join-Path $ffmpegExtracted.FullName "include"

    if (Test-Path $ffmpegBinDir) {
        # Copy shared DLLs needed for dynamic loading
        $requiredDlls = @("avformat*.dll", "avcodec*.dll", "avutil*.dll", "swscale*.dll", "swresample*.dll")
        foreach ($pattern in $requiredDlls) {
            $dlls = Get-ChildItem $ffmpegBinDir -Filter $pattern -ErrorAction SilentlyContinue
            foreach ($dll in $dlls) {
                Copy-Item $dll.FullName $outputDir -Force
                Copy-Item $dll.FullName (Join-Path $installDir "bin") -Force
                Write-BuildLog "Copied: $($dll.Name)" -Level Info
            }
        }
    }

    if (Test-Path $ffmpegLibDir) {
        # Copy import libraries
        $requiredLibs = @("avformat*.lib", "avcodec*.lib", "avutil*.lib", "swscale*.lib", "swresample*.lib")
        foreach ($pattern in $requiredLibs) {
            $libs = Get-ChildItem $ffmpegLibDir -Filter $pattern -ErrorAction SilentlyContinue
            foreach ($lib in $libs) {
                Copy-Item $lib.FullName $outputDir -Force
                Copy-Item $lib.FullName (Join-Path $installDir "lib") -Force
                Write-BuildLog "Copied: $($lib.Name)" -Level Info
            }
        }
    }

    if (Test-Path $ffmpegIncDir) {
        # Copy headers
        Copy-Item $ffmpegIncDir (Join-Path $installDir "include") -Recurse -Force -ErrorAction SilentlyContinue
        Write-BuildLog "Copied FFmpeg headers" -Level Info
    }

    Write-BuildLog "FFmpeg $FFmpegVersion pre-built libraries installed successfully" -Level Success
    Write-BuildLog "  DLLs: $outputDir" -Level Info
    Write-BuildLog "  Install: $installDir" -Level Info

} catch {
    Write-BuildLog "Pre-built download failed: $($_.Exception.Message)" -Level Warning
    Write-BuildLog "To build from source, MSYS2 + make are required." -Level Info
    Write-BuildLog "For now, FFmpeg integration will use dynamic loading with runtime detection." -Level Info
    Write-BuildLog "The ExplorerLens Engine will gracefully fall back to Media Foundation if FFmpeg DLLs are absent." -Level Info
}

# ============================================================================
# Summary
# ============================================================================

Write-BuildLog "" -Level Info
Write-BuildLog "========================================" -Level Info
Write-BuildLog "FFmpeg Build/Install Summary" -Level Info
Write-BuildLog "========================================" -Level Info
Write-BuildLog "  Version:      $FFmpegVersion" -Level Info
Write-BuildLog "  License:      LGPL 2.1" -Level Info
Write-BuildLog "  Link Mode:    Dynamic (LoadLibrary)" -Level Info
Write-BuildLog "  Fallback:     Windows Media Foundation" -Level Info
Write-BuildLog "  Components:   libavformat, libavcodec, libavutil, libswscale" -Level Info
Write-BuildLog "  Containers:   MKV, WebM, FLV, AVI, TS, OGV, RMVB" -Level Info
Write-BuildLog "  Codecs:       H.264, H.265, VP8/9, AV1, MPEG4, Theora" -Level Info
Write-BuildLog "========================================" -Level Info
