<#
.SYNOPSIS
    Save benchmark baseline for performance regression testing.

.DESCRIPTION
    Runs comprehensive performance benchmarks and saves results as JSON baseline.
    Used in CI to detect performance regressions (>10% slowdown = fail).

.PARAMETER Version
    Version tag for baseline (e.g., "7.0.0", "7.1.0-beta.1").

.PARAMETER OutputPath
    Directory to save baseline JSON (default: baselines/).

.PARAMETER IncludeMemoryProfile
    Include memory profiling data (heap, working set, GC).

.EXAMPLE
    .\tests\Save-BenchmarkBaseline.ps1 -Version "7.0.0"
#>

param(
    [Parameter(Mandatory = $true)]
    [string]$Version,
    
    [Parameter(Mandatory = $false)]
    [string]$OutputPath = "baselines",
    
    [Parameter(Mandatory = $false)]
    [switch]$IncludeMemoryProfile
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

# Paths
$workspaceRoot = Split-Path -Parent $PSScriptRoot
$engineExe = Join-Path $workspaceRoot "build\bin\Release\EngineBenchmarks.exe"
$baselineDir = Join-Path $workspaceRoot $OutputPath

# Validate engine binary exists
if (-not (Test-Path $engineExe)) {
    Write-Error "EngineBenchmarks.exe not found at: $engineExe"
    Write-Error "Build the project first: cmake --build build --config Release"
    exit 1
}

# Create baseline directory
if (-not (Test-Path $baselineDir)) {
    New-Item -ItemType Directory -Path $baselineDir -Force | Out-Null
    Write-Host "✓ Created baseline directory: $baselineDir" -ForegroundColor Green
}

# Output file
$baselineFile = Join-Path $baselineDir "baseline-$Version.json"

Write-Host ""
Write-Host "═══════════════════════════════════════════════════════════════" -ForegroundColor Cyan
Write-Host "  DarkThumbs Benchmark Baseline Capture" -ForegroundColor Cyan
Write-Host "═══════════════════════════════════════════════════════════════" -ForegroundColor Cyan
Write-Host "Version:    $Version" -ForegroundColor White
Write-Host "Output:     $baselineFile" -ForegroundColor White
Write-Host "Memory:     $($IncludeMemoryProfile ? 'Yes' : 'No')" -ForegroundColor White
Write-Host ""

# Run comprehensive benchmarks
Write-Host "[1/4] Running decoder throughput benchmarks..." -ForegroundColor Yellow

$decoderBenchmarks = @{
    "jpeg_throughput" = & $engineExe --benchmark_filter="BM_DecodeJPEG_.*" --benchmark_format=json | ConvertFrom-Json
    "png_throughput" = & $engineExe --benchmark_filter="BM_DecodePNG_.*" --benchmark_format=json | ConvertFrom-Json
    "webp_throughput" = & $engineExe --benchmark_filter="BM_DecodeWebP_.*" --benchmark_format=json | ConvertFrom-Json
    "avif_throughput" = & $engineExe --benchmark_filter="BM_DecodeAVIF_.*" --benchmark_format=json | ConvertFrom-Json
    "heif_throughput" = & $engineExe --benchmark_filter="BM_DecodeHEIF_.*" --benchmark_format=json | ConvertFrom-Json
    "jxl_throughput" = & $engineExe --benchmark_filter="BM_DecodeJXL_.*" --benchmark_format=json | ConvertFrom-Json
    "psd_throughput" = & $engineExe --benchmark_filter="BM_DecodePSD_.*" --benchmark_format=json | ConvertFrom-Json
    "svg_throughput" = & $engineExe --benchmark_filter="BM_DecodeSVG_.*" --benchmark_format=json | ConvertFrom-Json
}

Write-Host "✓ Decoder benchmarks complete" -ForegroundColor Green

# Run archive benchmarks
Write-Host "[2/4] Running archive performance benchmarks..." -ForegroundColor Yellow

$archiveBenchmarks = @{
    "zip_central_dir" = & $engineExe --benchmark_filter="BM_ZipCentralDirectory.*" --benchmark_format=json | ConvertFrom-Json
    "zip_mmap" = & $engineExe --benchmark_filter="BM_ZipMemoryMapped.*" --benchmark_format=json | ConvertFrom-Json
    "7z_decode" = & $engineExe --benchmark_filter="BM_7zDecode.*" --benchmark_format=json | ConvertFrom-Json
    "rar_decode" = & $engineExe --benchmark_filter="BM_RarDecode.*" --benchmark_format=json | ConvertFrom-Json
}

Write-Host "✓ Archive benchmarks complete" -ForegroundColor Green

# Run end-to-end latency benchmarks
Write-Host "[3/4] Running end-to-end latency benchmarks..." -ForegroundColor Yellow

$latencyBenchmarks = & $engineExe --benchmark_filter="BM_EndToEnd.*" --benchmark_format=json | ConvertFrom-Json

Write-Host "✓ Latency benchmarks complete" -ForegroundColor Green

# Memory profiling (optional)
$memoryProfile = $null
if ($IncludeMemoryProfile) {
    Write-Host "[4/4] Running memory profiling..." -ForegroundColor Yellow
    
    # Run memory benchmarks
    $memoryRaw = & $engineExe --benchmark_filter="BM_Memory.*" --benchmark_format=json | ConvertFrom-Json
    
    $memoryProfile = @{
        "peak_heap_mb" = ($memoryRaw.benchmarks | Where-Object { $_.name -match "PeakHeap" }).bytes_per_second / 1MB
        "working_set_mb" = ($memoryRaw.benchmarks | Where-Object { $_.name -match "WorkingSet" }).bytes_per_second / 1MB
        "allocations_per_decode" = ($memoryRaw.benchmarks | Where-Object { $_.name -match "Allocations" }).items_per_second
        "timestamp" = Get-Date -Format "o"
    }
    
    Write-Host "✓ Memory profiling complete" -ForegroundColor Green
    Write-Host "  Peak Heap:      $([math]::Round($memoryProfile.peak_heap_mb, 2)) MB" -ForegroundColor Gray
    Write-Host "  Working Set:    $([math]::Round($memoryProfile.working_set_mb, 2)) MB" -ForegroundColor Gray
} else {
    Write-Host "[4/4] Skipping memory profiling (use -IncludeMemoryProfile to enable)" -ForegroundColor Gray
}

# Build baseline JSON
$baseline = @{
    "version" = $Version
    "timestamp" = Get-Date -Format "o"
    "machine_info" = @{
        "os" = [System.Environment]::OSVersion.VersionString
        "cpu" = (Get-WmiObject -Class Win32_Processor).Name
        "cores" = (Get-WmiObject -Class Win32_Processor).NumberOfLogicalProcessors
        "ram_gb" = [math]::Round((Get-WmiObject -Class Win32_ComputerSystem).TotalPhysicalMemory / 1GB, 2)
    }
    "decoder_benchmarks" = $decoderBenchmarks
    "archive_benchmarks" = $archiveBenchmarks
    "latency_benchmarks" = $latencyBenchmarks
    "memory_profile" = $memoryProfile
}

# Save to JSON
$baseline | ConvertTo-Json -Depth 10 | Set-Content -Path $baselineFile -Encoding UTF8

Write-Host ""
Write-Host "═══════════════════════════════════════════════════════════════" -ForegroundColor Green
Write-Host "  ✓ Baseline Saved Successfully" -ForegroundColor Green
Write-Host "═══════════════════════════════════════════════════════════════" -ForegroundColor Green
Write-Host "File: $baselineFile" -ForegroundColor White
Write-Host "Size: $([math]::Round((Get-Item $baselineFile).Length / 1KB, 2)) KB" -ForegroundColor White
Write-Host ""
Write-Host "Usage in CI:" -ForegroundColor Cyan
Write-Host "  .\tests\Compare-BenchmarkBaseline.ps1 -BaselineVersion `"$Version`"" -ForegroundColor Gray
Write-Host ""
