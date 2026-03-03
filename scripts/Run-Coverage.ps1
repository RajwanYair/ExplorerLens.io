#==============================================================================
# Run-Coverage.ps1 — Code Coverage Report Generator
# ExplorerLens v15.0.0
#
# Uses OpenCppCoverage to collect C++ code coverage from EngineTests.exe.
# Automatically downloads OpenCppCoverage if not installed.
#
# Usage:
#   pwsh -File scripts/Run-Coverage.ps1 [-HtmlReport] [-CoberturaXml]
#==============================================================================

param(
    [switch]$HtmlReport,
    [switch]$CoberturaXml,
    [string]$OutputDir = "coverage-report",
    [string]$TestExe = "",
    [switch]$Install
)

$ErrorActionPreference = "Stop"
$rootDir = Split-Path -Parent $PSScriptRoot

# Default test executable path
if (-not $TestExe) {
    $TestExe = Join-Path $rootDir "build\bin\EngineTests.exe"
}

#==============================================================================
# Ensure OpenCppCoverage is available
#==============================================================================

function Find-OpenCppCoverage {
    # Check PATH
    $cmd = Get-Command "OpenCppCoverage.exe" -ErrorAction SilentlyContinue
    if ($cmd) { return $cmd.Source }

    # Check common install locations
    $locations = @(
        "$env:ProgramFiles\OpenCppCoverage\OpenCppCoverage.exe",
        "${env:ProgramFiles(x86)}\OpenCppCoverage\OpenCppCoverage.exe",
        "$env:LOCALAPPDATA\Programs\OpenCppCoverage\OpenCppCoverage.exe"
    )

    foreach ($loc in $locations) {
        if (Test-Path $loc) { return $loc }
    }

    # Check scoop
    $scoopPath = "$env:USERPROFILE\scoop\apps\opencppcoverage\current\OpenCppCoverage.exe"
    if (Test-Path $scoopPath) { return $scoopPath }

    return $null
}

function Install-OpenCppCoverage {
    Write-Host "Installing OpenCppCoverage..." -ForegroundColor Yellow

    # Try scoop first
    $scoop = Get-Command "scoop" -ErrorAction SilentlyContinue
    if ($scoop) {
        Write-Host "  Using scoop..." -ForegroundColor Cyan
        scoop install opencppcoverage
        $path = Find-OpenCppCoverage
        if ($path) { return $path }
    }

    # Try chocolatey
    $choco = Get-Command "choco" -ErrorAction SilentlyContinue
    if ($choco) {
        Write-Host "  Using chocolatey..." -ForegroundColor Cyan
        choco install opencppcoverage -y
        $path = Find-OpenCppCoverage
        if ($path) { return $path }
    }

    # Download directly
    $version = "0.9.9.0"
    $url = "https://github.com/OpenCppCoverage/OpenCppCoverage/releases/download/release-$version/OpenCppCoverageSetup-x64-$version.exe"
    $installer = Join-Path $env:TEMP "OpenCppCoverageSetup.exe"

    Write-Host "  Downloading $url ..." -ForegroundColor Cyan
    try {
        [System.Net.WebClient]::new().DownloadFile($url, $installer)
        Write-Host "  Running installer (silent)..." -ForegroundColor Cyan
        Start-Process -FilePath $installer -ArgumentList "/VERYSILENT", "/SP-" -Wait
        Remove-Item $installer -ErrorAction SilentlyContinue
    } catch {
        Write-Host "  Direct download failed. Please install manually:" -ForegroundColor Red
        Write-Host "    https://github.com/OpenCppCoverage/OpenCppCoverage/releases" -ForegroundColor Yellow
        return $null
    }

    return Find-OpenCppCoverage
}

#==============================================================================
# Main
#==============================================================================

$occExe = Find-OpenCppCoverage
if (-not $occExe) {
    if ($Install) {
        $occExe = Install-OpenCppCoverage
    }
    if (-not $occExe) {
        Write-Host "OpenCppCoverage not found. Run with -Install to auto-install." -ForegroundColor Red
        Write-Host "  Or install manually: scoop install opencppcoverage" -ForegroundColor Yellow
        exit 1
    }
}

Write-Host "OpenCppCoverage: $occExe" -ForegroundColor Green

# Verify test executable
if (-not (Test-Path $TestExe)) {
    Write-Host "Test executable not found: $TestExe" -ForegroundColor Red
    Write-Host "Build first: .\build-scripts\Build-MSVC.ps1 -Test" -ForegroundColor Yellow
    exit 1
}

# Create output directory
$outPath = Join-Path $rootDir $OutputDir
if (-not (Test-Path $outPath)) {
    New-Item -ItemType Directory -Path $outPath -Force | Out-Null
}

# Build OpenCppCoverage arguments
$occArgs = @(
    "--sources", (Join-Path $rootDir "Engine"),
    "--excluded_sources", (Join-Path $rootDir "Engine\Tests"),
    "--excluded_sources", (Join-Path $rootDir "external"),
    "--modules", "EngineTests.exe"
)

# Default to HTML if no format specified
if (-not $HtmlReport -and -not $CoberturaXml) {
    $HtmlReport = $true
}

if ($HtmlReport) {
    $htmlPath = Join-Path $outPath "html"
    $occArgs += "--export_type", "html:$htmlPath"
    Write-Host "HTML report → $htmlPath" -ForegroundColor Cyan
}

if ($CoberturaXml) {
    $xmlPath = Join-Path $outPath "coverage.xml"
    $occArgs += "--export_type", "cobertura:$xmlPath"
    Write-Host "Cobertura XML → $xmlPath" -ForegroundColor Cyan
}

$occArgs += "--", $TestExe

Write-Host "`nRunning coverage collection..." -ForegroundColor Yellow
Write-Host "  $occExe $($occArgs -join ' ')" -ForegroundColor DarkGray

$sw = [System.Diagnostics.Stopwatch]::StartNew()
& $occExe @occArgs
$exitCode = $LASTEXITCODE
$sw.Stop()

Write-Host "`nCoverage collection completed in $($sw.Elapsed.TotalSeconds.ToString('F1'))s" -ForegroundColor Green

if ($HtmlReport) {
    $indexFile = Join-Path $htmlPath "index.html"
    if (Test-Path $indexFile) {
        Write-Host "Open report: start $indexFile" -ForegroundColor Cyan
    }
}

exit $exitCode
