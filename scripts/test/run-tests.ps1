# Build and Run GPU Tests
# PowerShell script for building and running GPU test suite

param(
    [string]$TestFolder = "",
    [string]$OutputFolder = "",
    [int]$ThumbnailSize = 256,
    [int]$Iterations = 10,
    [switch]$BuildOnly,
    [switch]$Verbose
)

Write-Host "=== ExplorerLens GPU Test Suite ===" -ForegroundColor Cyan
Write-Host ""

# Build test tools
Write-Host "Building test tools..." -ForegroundColor Yellow
& "$PSScriptRoot\build-tests.cmd"

if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Build failed!" -ForegroundColor Red
    exit 1
}

Write-Host "Build successful!" -ForegroundColor Green
Write-Host ""

if ($BuildOnly) {
    Write-Host "Build-only mode, exiting." -ForegroundColor Cyan
    exit 0
}

# Check if test folder specified
if ([string]::IsNullOrEmpty($TestFolder)) {
    Write-Host "No test folder specified. Use -TestFolder parameter." -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Usage Examples:" -ForegroundColor Cyan
    Write-Host "  .\run-tests.ps1 -TestFolder C:\TestImages" -ForegroundColor Gray
    Write-Host "  .\run-tests.ps1 -TestFolder C:\TestImages -OutputFolder C:\Thumbnails" -ForegroundColor Gray
    Write-Host "  .\run-tests.ps1 -TestFolder C:\TestImages -Iterations 20 -Verbose" -ForegroundColor Gray
    exit 0
}

# Validate test folder
if (-not (Test-Path $TestFolder)) {
    Write-Host "ERROR: Test folder not found: $TestFolder" -ForegroundColor Red
    exit 1
}

Write-Host "=== Running GPUThumbnailTest ===" -ForegroundColor Cyan
Write-Host ""

$args = @("-i", $TestFolder, "-s", $ThumbnailSize)
if (-not [string]::IsNullOrEmpty($OutputFolder)) {
    $args += @("-o", $OutputFolder)
}
if ($Verbose) {
    $args += "-v"
}

& "$PSScriptRoot\GPUThumbnailTest.exe" $args

Write-Host ""
Write-Host "=== Running LENSBench ===" -ForegroundColor Cyan
Write-Host ""

$csvPath = "$PSScriptRoot\benchmark_results_$(Get-Date -Format 'yyyyMMdd_HHmmss').csv"
$benchArgs = @("-i", $TestFolder, "-o", $csvPath, "-s", $ThumbnailSize, "-n", $Iterations)
if ($Verbose) {
    $benchArgs += "-v"
}

& "$PSScriptRoot\LENSBench.exe" $benchArgs

Write-Host ""
Write-Host "=== Test Suite Complete ===" -ForegroundColor Green
Write-Host "Benchmark results saved to: $csvPath" -ForegroundColor Cyan

