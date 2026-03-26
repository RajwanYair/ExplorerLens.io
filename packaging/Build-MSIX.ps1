#!/usr/bin/env pwsh
<#
.SYNOPSIS
    Build-MSIX.ps1 — Build MSIX Package for ExplorerLens
    Sprint 38 (v15.6.0 Release Engineering)

.DESCRIPTION
    Packages LENSShell.dll + LENSManager.exe + assets into an MSIX archive
    using MakeAppx.exe from the Windows SDK.
    Optionally signs the MSIX with the configured certificate.

.PARAMETER BuildDir
    CMake build output directory (default: build).

.PARAMETER OutputDir
    Directory to write the MSIX (default: release-staging).

.PARAMETER Sign
    If specified, sign the MSIX after creation.

.EXAMPLE
    .\packaging\Build-MSIX.ps1 -BuildDir build -Sign
#>
[CmdletBinding()]
param(
    [string] $BuildDir   = "build",
    [string] $OutputDir  = "release-staging",
    [switch] $Sign
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$rootDir   = Split-Path -Parent $scriptDir

# ── Resolve version ───────────────────────────────────────────────────────────
$versionFile = Join-Path $rootDir "VERSION"
$version = if (Test-Path $versionFile) {
    (Get-Content $versionFile -Raw).Trim()
} else { "0.0.0" }

Write-Host "[msix] Building MSIX for ExplorerLens v$version"

# ── Find MakeAppx.exe ─────────────────────────────────────────────────────────
$wkKitsRoot = "${env:ProgramFiles(x86)}\Windows Kits\10\bin"
$makeAppx   = Get-ChildItem -Path $wkKitsRoot -Filter "makeappx.exe" -Recurse -ErrorAction SilentlyContinue |
              Sort-Object -Property FullName -Descending |
              Select-Object -First 1 -ExpandProperty FullName

if (-not $makeAppx) {
    Write-Error "MakeAppx.exe not found. Ensure Windows SDK 10.0.17763+ is installed."
    exit 1
}
Write-Host "[msix] MakeAppx: $makeAppx"

# ── Prepare package content directory ────────────────────────────────────────
$pkgContent = Join-Path $rootDir "packaging\PackageContent"
Remove-Item -Path $pkgContent -Recurse -Force -ErrorAction SilentlyContinue
New-Item  -ItemType Directory -Path $pkgContent -Force | Out-Null

# Copy binaries
$buildBinDir = Join-Path $rootDir $BuildDir "bin"
$binaries = @("LENSShell.dll", "LENSManager.exe")
foreach ($bin in $binaries) {
    $src = Join-Path $buildBinDir $bin
    if (Test-Path $src) {
        Copy-Item $src $pkgContent
        Write-Host "[msix] + $bin"
    } else {
        Write-Warning "[msix] Binary not found: $src (skipping)"
    }
}

# Copy manifest
$manifestSrc = Join-Path $rootDir "packaging\ExplorerLens.msixmanifest"
if (-not (Test-Path $manifestSrc)) {
    Write-Error "Manifest not found: $manifestSrc"
    exit 1
}
Copy-Item $manifestSrc -Destination (Join-Path $pkgContent "AppxManifest.xml")

# Create placeholder assets if not present
$assetsDir = Join-Path $pkgContent "Assets"
New-Item -ItemType Directory -Path $assetsDir -Force | Out-Null
$assetFiles = @(
    "StoreLogo.png", "Square150x150Logo.png", "Square44x44Logo.png",
    "Wide310x150Logo.png", "SplashScreen.png"
)
foreach ($asset in $assetFiles) {
    $srcAsset = Join-Path $rootDir "packaging\assets\$asset"
    $dstAsset = Join-Path $assetsDir $asset
    if (Test-Path $srcAsset) {
        Copy-Item $srcAsset $dstAsset
    } else {
        # Create a minimal placeholder PNG (1×1 pixel, transparent)
        # This is the smallest valid PNG header
        $pngBytes = [byte[]] @(
            0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A,
            0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52,
            0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01,
            0x08, 0x06, 0x00, 0x00, 0x00, 0x1F, 0x15, 0xC4,
            0x89, 0x00, 0x00, 0x00, 0x0B, 0x49, 0x44, 0x41,
            0x54, 0x78, 0x9C, 0x62, 0x00, 0x00, 0x00, 0x02,
            0x00, 0x01, 0xE5, 0x27, 0xDE, 0xFC, 0x00, 0x00,
            0x00, 0x00, 0x49, 0x45, 0x4E, 0x44, 0xAE, 0x42,
            0x60, 0x82
        )
        [IO.File]::WriteAllBytes($dstAsset, $pngBytes)
        Write-Host "[msix] Created placeholder: $asset"
    }
}

# ── Run MakeAppx ─────────────────────────────────────────────────────────────
$outputFullDir = if ([IO.Path]::IsPathRooted($OutputDir)) {
    $OutputDir
} else {
    Join-Path $rootDir $OutputDir
}
New-Item -ItemType Directory -Path $outputFullDir -Force | Out-Null

$msixName = "ExplorerLens-$version-x64.msix"
$msixPath = Join-Path $outputFullDir $msixName

Write-Host "[msix] Creating $msixName..."
& $makeAppx pack /h SHA256 /d $pkgContent /p $msixPath /overwrite 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Error "MakeAppx failed (exit $LASTEXITCODE)"
    exit 1
}
Write-Host "[msix] Created: $msixPath"

# ── Optional signing ─────────────────────────────────────────────────────────
if ($Sign) {
    $signTool = Get-ChildItem "${env:ProgramFiles(x86)}\Windows Kits\10\bin" `
                -Filter "signtool.exe" -Recurse -ErrorAction SilentlyContinue |
                Sort-Object FullName -Descending | Select-Object -First 1 -ExpandProperty FullName

    if ($signTool) {
        Write-Host "[msix] Signing with signtool..."
        & $signTool sign /a /fd sha256 /tr "http://timestamp.digicert.com" /td sha256 $msixPath
        if ($LASTEXITCODE -eq 0) {
            Write-Host "[msix] Signed successfully"
        } else {
            Write-Warning "[msix] Signing failed (no certificate?). MSIX is unsigned."
        }
    } else {
        Write-Warning "[msix] signtool.exe not found. MSIX is unsigned."
    }
}

Write-Host "[msix] DONE: $msixPath ($('{0:N0}' -f (Get-Item $msixPath).Length) bytes)"
