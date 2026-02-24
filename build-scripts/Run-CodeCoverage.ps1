<#
.SYNOPSIS
    Run-CodeCoverage.ps1 — Code Coverage Analysis for ExplorerLens Engine
    Sprint 380 — ExplorerLens v15.0.0 "Zenith"

.DESCRIPTION
    Uses OpenCppCoverage to generate code coverage reports for the Engine test suite.
    Outputs HTML and Cobertura XML reports.

.PARAMETER OutputDir
    Directory for coverage reports (default: build-logs/coverage)

.PARAMETER TestExe
    Path to test executable (default: build/bin/EngineTests.exe)

.PARAMETER OpenReport
    Open HTML report in browser after generation

.EXAMPLE
    .\build-scripts\Run-CodeCoverage.ps1 -OpenReport
#>

param(
    [string]$OutputDir = "build-logs/coverage",
    [string]$TestExe = "build/bin/EngineTests.exe",
    [switch]$OpenReport
)

$ErrorActionPreference = "Stop"
$rootDir = Split-Path -Parent $PSScriptRoot

Write-Host "=============================================" -ForegroundColor Cyan
Write-Host "  ExplorerLens Code Coverage Analysis" -ForegroundColor Cyan
Write-Host "  v15.0.0 Zenith — Sprint 380" -ForegroundColor DarkCyan
Write-Host "=============================================" -ForegroundColor Cyan

# Check for OpenCppCoverage
$opencppcov = Get-Command "OpenCppCoverage" -ErrorAction SilentlyContinue
if (-not $opencppcov) {
    Write-Host "`n[!] OpenCppCoverage not found. Installing via scoop..." -ForegroundColor Yellow
    scoop install opencppcoverage 2>$null
    if ($LASTEXITCODE -ne 0) {
        Write-Host "[!] Trying choco install..." -ForegroundColor Yellow
        choco install opencppcoverage -y 2>$null
    }
    $opencppcov = Get-Command "OpenCppCoverage" -ErrorAction SilentlyContinue
    if (-not $opencppcov) {
        Write-Error "Cannot find OpenCppCoverage. Install manually: https://github.com/OpenCppCoverage/OpenCppCoverage"
        exit 1
    }
}

# Verify test executable exists
$testPath = Join-Path $rootDir $TestExe
if (-not (Test-Path $testPath)) {
    Write-Host "[!] Test executable not found: $testPath" -ForegroundColor Red
    Write-Host "[i] Build first: .\build-scripts\Build-MSVC.ps1 -Test" -ForegroundColor Yellow
    exit 1
}

# Create output directory
$outPath = Join-Path $rootDir $OutputDir
New-Item -ItemType Directory -Force -Path $outPath | Out-Null

Write-Host "`n[1/3] Running tests with coverage instrumentation..." -ForegroundColor Green

# Run OpenCppCoverage
$covArgs = @(
    "--sources", (Join-Path $rootDir "Engine\Core"),
    "--sources", (Join-Path $rootDir "Engine\Decoders"),
    "--sources", (Join-Path $rootDir "Engine\Pipeline"),
    "--sources", (Join-Path $rootDir "Engine\Memory"),
    "--sources", (Join-Path $rootDir "Engine\Cache"),
    "--sources", (Join-Path $rootDir "Engine\AI"),
    "--sources", (Join-Path $rootDir "Engine\GPU"),
    "--sources", (Join-Path $rootDir "Engine\Plugin"),
    "--sources", (Join-Path $rootDir "Engine\Utils"),
    "--excluded_sources", (Join-Path $rootDir "external"),
    "--excluded_sources", (Join-Path $rootDir "gtest"),
    "--excluded_sources", (Join-Path $rootDir "Engine\Tests"),
    "--export_type", "html:$outPath\html",
    "--export_type", "cobertura:$outPath\coverage.xml",
    "--", $testPath
)

& OpenCppCoverage @covArgs

Write-Host "`n[2/3] Coverage reports generated:" -ForegroundColor Green
Write-Host "  HTML:      $outPath\html\index.html" -ForegroundColor White
Write-Host "  Cobertura: $outPath\coverage.xml" -ForegroundColor White

# Parse coverage summary from XML
Write-Host "`n[3/3] Coverage Summary:" -ForegroundColor Green
$xmlPath = Join-Path $outPath "coverage.xml"
if (Test-Path $xmlPath) {
    [xml]$cov = Get-Content $xmlPath
    $lineRate = [double]$cov.coverage.'line-rate' * 100
    $branchRate = [double]$cov.coverage.'branch-rate' * 100
    
    $lineColor = if ($lineRate -ge 80) { "Green" } elseif ($lineRate -ge 60) { "Yellow" } else { "Red" }
    $branchColor = if ($branchRate -ge 70) { "Green" } elseif ($branchRate -ge 50) { "Yellow" } else { "Red" }
    
    Write-Host ("  Line Coverage:   {0:F1}%" -f $lineRate) -ForegroundColor $lineColor
    Write-Host ("  Branch Coverage: {0:F1}%" -f $branchRate) -ForegroundColor $branchColor
    
    # Targets
    Write-Host "`n  Targets:" -ForegroundColor DarkGray
    Write-Host "    Line:   80% (v15.0 target)" -ForegroundColor DarkGray
    Write-Host "    Branch: 70% (v15.0 target)" -ForegroundColor DarkGray
}

if ($OpenReport) {
    $htmlIndex = Join-Path $outPath "html\index.html"
    if (Test-Path $htmlIndex) {
        Start-Process $htmlIndex
    }
}

Write-Host "`n[OK] Coverage analysis complete." -ForegroundColor Green
