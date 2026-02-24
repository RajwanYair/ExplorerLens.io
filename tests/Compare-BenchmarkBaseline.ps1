<#
.SYNOPSIS
    Compare current benchmark results against saved baseline to detect performance regressions.

.DESCRIPTION
    Runs current benchmarks and compares against saved baseline.
    Fails (exit 1) if any metric regresses by >10% (configurable).
    Used in CI to block PRs that introduce performance regressions.

.PARAMETER BaselineVersion
    Version of baseline to compare against (e.g., "7.0.0").

.PARAMETER RegressionThreshold
    Maximum allowed regression percentage (default: 10).
    Example: 10 = fail if 10% slower than baseline.

.PARAMETER BaselinePath
    Path to baseline JSON directory (default: baselines/).

.PARAMETER FailOnRegression
    Exit with code 1 if regression detected (CI mode).

.EXAMPLE
    .\tests\Compare-BenchmarkBaseline.ps1 -BaselineVersion "7.0.0"
    
.EXAMPLE
    .\tests\Compare-BenchmarkBaseline.ps1 -BaselineVersion "7.0.0" -RegressionThreshold 5 -FailOnRegression
#>

param(
    [Parameter(Mandatory = $true)]
    [string]$BaselineVersion,
    
    [Parameter(Mandatory = $false)]
    [double]$RegressionThreshold = 10.0,
    
    [Parameter(Mandatory = $false)]
    [string]$BaselinePath = "baselines",
    
    [Parameter(Mandatory = $false)]
    [switch]$FailOnRegression
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

# Paths
$workspaceRoot = Split-Path -Parent $PSScriptRoot
$engineExe = Join-Path $workspaceRoot "build\bin\Release\EngineBenchmarks.exe"
$baselineDir = Join-Path $workspaceRoot $BaselinePath
$baselineFile = Join-Path $baselineDir "baseline-$BaselineVersion.json"

# Validate paths
if (-not (Test-Path $engineExe)) {
    Write-Error "EngineBenchmarks.exe not found at: $engineExe"
    exit 1
}

if (-not (Test-Path $baselineFile)) {
    Write-Error "Baseline file not found: $baselineFile"
    Write-Error "Create baseline first: .\tests\Save-BenchmarkBaseline.ps1 -Version `"$BaselineVersion`""
    exit 1
}

Write-Host ""
Write-Host "═══════════════════════════════════════════════════════════════" -ForegroundColor Cyan
Write-Host "  ExplorerLens Performance Regression Detection" -ForegroundColor Cyan
Write-Host "═══════════════════════════════════════════════════════════════" -ForegroundColor Cyan
Write-Host "Baseline:     $BaselineVersion" -ForegroundColor White
Write-Host "Threshold:    $RegressionThreshold%" -ForegroundColor White
Write-Host "Fail on Reg:  $($FailOnRegression ? 'Yes' : 'No')" -ForegroundColor White
Write-Host ""

# Load baseline
$baseline = Get-Content $baselineFile -Raw | ConvertFrom-Json
Write-Host "✓ Loaded baseline from $(Get-Date $baseline.timestamp -Format 'yyyy-MM-dd HH:mm')" -ForegroundColor Green

# Run current benchmarks
Write-Host ""
Write-Host "Running current benchmarks..." -ForegroundColor Yellow

$currentDecoders = @{
    "jpeg_throughput" = & $engineExe --benchmark_filter="BM_DecodeJPEG_.*" --benchmark_format=json | ConvertFrom-Json
    "png_throughput" = & $engineExe --benchmark_filter="BM_DecodePNG_.*" --benchmark_format=json | ConvertFrom-Json
    "webp_throughput" = & $engineExe --benchmark_filter="BM_DecodeWebP_.*" --benchmark_format=json | ConvertFrom-Json
    "avif_throughput" = & $engineExe --benchmark_filter="BM_DecodeAVIF_.*" --benchmark_format=json | ConvertFrom-Json
}

$currentLatency = & $engineExe --benchmark_filter="BM_EndToEnd.*" --benchmark_format=json | ConvertFrom-Json

Write-Host "✓ Current benchmarks complete" -ForegroundColor Green

# Compare results
Write-Host ""
Write-Host "═══════════════════════════════════════════════════════════════" -ForegroundColor Cyan
Write-Host "  Performance Comparison" -ForegroundColor Cyan
Write-Host "═══════════════════════════════════════════════════════════════" -ForegroundColor Cyan
Write-Host ""

$regressions = @()
$improvements = @()

# Compare decoder throughput
Write-Host "Decoder Throughput:" -ForegroundColor White
Write-Host ("─" * 63) -ForegroundColor Gray

function Compare-Benchmark {
    param(
        [string]$Name,
        [object]$BaselineData,
        [object]$CurrentData
    )
    
    # Extract p95 latency from benchmark results
    $baselineP95 = ($BaselineData.benchmarks | Where-Object { $_.name -match "p95" }).real_time
    $currentP95 = ($CurrentData.benchmarks | Where-Object { $_.name -match "p95" }).real_time
    
    if ($baselineP95 -and $currentP95) {
        $changePercent = (($currentP95 - $baselineP95) / $baselineP95) * 100
        
        $status = "✓"
        $color = "Green"
        
        if ($changePercent -gt $RegressionThreshold) {
            $status = "✗"
            $color = "Red"
            $script:regressions += @{
                "name" = $Name
                "baseline_ms" = $baselineP95
                "current_ms" = $currentP95
                "regression_percent" = $changePercent
            }
        } elseif ($changePercent -lt -5) {
            $status = "↑"
            $color = "Cyan"
            $script:improvements += @{
                "name" = $Name
                "baseline_ms" = $baselineP95
                "current_ms" = $currentP95
                "improvement_percent" = [math]::Abs($changePercent)
            }
        }
        
        $changeStr = if ($changePercent -gt 0) { "+$($changePercent.ToString('F1'))%" } else { "$($changePercent.ToString('F1'))%" }
        
        Write-Host "$status $($Name.PadRight(25)) " -NoNewline -ForegroundColor $color
        Write-Host "$([math]::Round($baselineP95, 1))ms → $([math]::Round($currentP95, 1))ms " -NoNewline -ForegroundColor White
        Write-Host "($changeStr)" -ForegroundColor $color
    }
}

# Compare each decoder
Compare-Benchmark -Name "JPEG Decode (4K)" -BaselineData $baseline.decoder_benchmarks.jpeg_throughput -CurrentData $currentDecoders.jpeg_throughput
Compare-Benchmark -Name "PNG Decode (4K)" -BaselineData $baseline.decoder_benchmarks.png_throughput -CurrentData $currentDecoders.png_throughput
Compare-Benchmark -Name "WebP Decode (4K)" -BaselineData $baseline.decoder_benchmarks.webp_throughput -CurrentData $currentDecoders.webp_throughput
Compare-Benchmark -Name "AVIF Decode (4K)" -BaselineData $baseline.decoder_benchmarks.avif_throughput -CurrentData $currentDecoders.avif_throughput

Write-Host ""

# Compare memory usage (if available)
if ($baseline.memory_profile) {
    Write-Host "Memory Usage:" -ForegroundColor White
    Write-Host ("─" * 63) -ForegroundColor Gray
    
    $currentMemory = & $engineExe --benchmark_filter="BM_Memory.*" --benchmark_format=json | ConvertFrom-Json
    $currentPeakHeap = ($currentMemory.benchmarks | Where-Object { $_.name -match "PeakHeap" }).bytes_per_second / 1MB
    
    $baselinePeakHeap = $baseline.memory_profile.peak_heap_mb
    $memoryThreshold = $baselinePeakHeap * 2  # Fail if >2× baseline
    
    $memoryChangePercent = (($currentPeakHeap - $baselinePeakHeap) / $baselinePeakHeap) * 100
    
    if ($currentPeakHeap -gt $memoryThreshold) {
        Write-Host "✗ Peak Heap: $([math]::Round($baselinePeakHeap, 2)) MB → $([math]::Round($currentPeakHeap, 2)) MB (+$($memoryChangePercent.ToString('F1'))%)" -ForegroundColor Red
        $regressions += @{
            "name" = "Peak Heap Memory"
            "baseline_mb" = $baselinePeakHeap
            "current_mb" = $currentPeakHeap
            "regression_percent" = $memoryChangePercent
        }
    } else {
        $color = if ($memoryChangePercent -gt 0) { "Yellow" } else { "Green" }
        $changeStr = if ($memoryChangePercent -gt 0) { "+$($memoryChangePercent.ToString('F1'))%" } else { "$($memoryChangePercent.ToString('F1'))%" }
        Write-Host "✓ Peak Heap: $([math]::Round($baselinePeakHeap, 2)) MB → $([math]::Round($currentPeakHeap, 2)) MB ($changeStr)" -ForegroundColor $color
    }
    
    Write-Host ""
}

# Summary
Write-Host "═══════════════════════════════════════════════════════════════" -ForegroundColor Cyan
Write-Host "  Summary" -ForegroundColor Cyan
Write-Host "═══════════════════════════════════════════════════════════════" -ForegroundColor Cyan
Write-Host ""

if ($regressions.Count -gt 0) {
    Write-Host "✗ Performance Regressions Detected: $($regressions.Count)" -ForegroundColor Red
    Write-Host ""
    
    foreach ($reg in $regressions) {
        Write-Host "  • $($reg.name)" -ForegroundColor Red
        Write-Host "    Baseline: $([math]::Round($reg.baseline_ms, 2)) ms" -ForegroundColor Gray
        Write-Host "    Current:  $([math]::Round($reg.current_ms, 2)) ms" -ForegroundColor Gray
        Write-Host "    Change:   +$($reg.regression_percent.ToString('F1'))%" -ForegroundColor Red
        Write-Host ""
    }
    
    if ($FailOnRegression) {
        Write-Host "❌ CI FAILURE: Performance regression threshold exceeded ($RegressionThreshold%)" -ForegroundColor Red
        Write-Host ""
        exit 1
    } else {
        Write-Host "⚠️  Warning: Regressions detected but not in fail mode" -ForegroundColor Yellow
        Write-Host "   Use -FailOnRegression to enforce in CI" -ForegroundColor Gray
        Write-Host ""
    }
} else {
    Write-Host "✓ No Performance Regressions Detected" -ForegroundColor Green
    Write-Host ""
}

if ($improvements.Count -gt 0) {
    Write-Host "↑ Performance Improvements: $($improvements.Count)" -ForegroundColor Cyan
    Write-Host ""
    
    foreach ($imp in $improvements) {
        Write-Host "  • $($imp.name)" -ForegroundColor Cyan
        Write-Host "    Improvement: $($imp.improvement_percent.ToString('F1'))% faster" -ForegroundColor Green
        Write-Host ""
    }
}

Write-Host "═══════════════════════════════════════════════════════════════" -ForegroundColor Green
Write-Host ""

# Exit based on regression detection
if ($FailOnRegression -and $regressions.Count -gt 0) {
    exit 1
} else {
    exit 0
}

