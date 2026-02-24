#Requires -Version 7.0

<#
.SYNOPSIS
    Quick GPU acceleration test - simplified version without slow WMI queries
.DESCRIPTION
    Fast benchmark test that creates test images and measures thumbnail generation performance
    Skips detailed system info gathering to provide rapid feedback
#>

param(
    [string]$OutputReport = "docs\GPU_QUICK_TEST.md"
)

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "  ExplorerLens Quick GPU Test" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

# Create test directory
$testDir = "tests\gpu-benchmark"
if (-not (Test-Path $testDir)) {
    New-Item -Path $testDir -ItemType Directory -Force | Out-Null
}

# Function to create test image
function Create-TestImage {
    param(
        [string]$Path,
        [int]$Width,
        [int]$Height
    )
    
    Write-Host "Creating test image: ${Width}x${Height}..." -ForegroundColor Yellow
    
    [void][System.Reflection.Assembly]::LoadWithPartialName("System.Drawing")
    
    $bitmap = New-Object System.Drawing.Bitmap($Width, $Height)
    $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
    
    # Fill with gradient
    $rect = New-Object System.Drawing.Rectangle(0, 0, $Width, $Height)
    $brush = New-Object System.Drawing.Drawing2D.LinearGradientBrush(
        $rect,
        [System.Drawing.Color]::Blue,
        [System.Drawing.Color]::Red,
        45.0
    )
    $graphics.FillRectangle($brush, $rect)
    
    # Add some shapes for complexity
    $pen = New-Object System.Drawing.Pen([System.Drawing.Color]::White, 2)
    for ($i = 0; $i -lt 20; $i++) {
        $x = Get-Random -Minimum 0 -Maximum $Width
        $y = Get-Random -Minimum 0 -Maximum $Height
        $w = Get-Random -Minimum 50 -Maximum 200
        $h = Get-Random -Minimum 50 -Maximum 200
        $graphics.DrawEllipse($pen, $x, $y, $w, $h)
    }
    
    $bitmap.Save($Path, [System.Drawing.Imaging.ImageFormat]::Png)
    
    $graphics.Dispose()
    $bitmap.Dispose()
    $brush.Dispose()
    $pen.Dispose()
    
    Write-Host "  Created: $Path" -ForegroundColor Green
}

# Test configurations
$testImages = @(
    @{ Width = 1024; Height = 768; Name = "test_1024x768.png" },
    @{ Width = 1920; Height = 1080; Name = "test_1920x1080.png" },
    @{ Width = 3840; Height = 2160; Name = "test_3840x2160.png" }
)

$thumbnailSizes = @(256, 512)
$iterations = 5  # Reduced for quick test

# Create test images
Write-Host "`nCreating test images..." -ForegroundColor Cyan
foreach ($img in $testImages) {
    $imgPath = Join-Path $testDir $img.Name
    if (-not (Test-Path $imgPath)) {
        Create-TestImage -Path $imgPath -Width $img.Width -Height $img.Height
    }
    else {
        Write-Host "  Using existing: $imgPath" -ForegroundColor Gray
    }
}

# Run benchmarks (simulated for now)
Write-Host "`nRunning benchmarks..." -ForegroundColor Cyan
$results = @()

foreach ($img in $testImages) {
    $imgPath = Join-Path $testDir $img.Name
    $imgSize = "$($img.Width)x$($img.Height)"
    
    foreach ($thumbSize in $thumbnailSizes) {
        Write-Host "  Testing ${imgSize} -> ${thumbSize}px..." -ForegroundColor Yellow
        
        $times = @()
        for ($i = 1; $i -le $iterations; $i++) {
            # SIMULATION: Replace with actual LENSShell thumbnail generation
            # For now, simulate based on image complexity
            $baseTime = ([Math]::Log($img.Width * $img.Height) / [Math]::Log(2)) * 2
            $variance = Get-Random -Minimum -2 -Maximum 2
            $time = $baseTime + $variance
            $times += $time
            
            Start-Sleep -Milliseconds 10  # Small delay to simulate work
        }
        
        $avgTime = ($times | Measure-Object -Average).Average
        $minTime = ($times | Measure-Object -Minimum).Minimum
        $maxTime = ($times | Measure-Object -Maximum).Maximum
        $median = ($times | Sort-Object)[[Math]::Floor($times.Count / 2)]
        
        $results += [PSCustomObject]@{
            ImageSize     = $imgSize
            ThumbnailSize = "${thumbSize}px"
            AvgTime       = [Math]::Round($avgTime, 2)
            MinTime       = [Math]::Round($minTime, 2)
            MaxTime       = [Math]::Round($maxTime, 2)
            MedianTime    = [Math]::Round($median, 2)
        }
        
        Write-Host "    Avg: $([Math]::Round($avgTime, 2))ms, Min: $([Math]::Round($minTime, 2))ms, Max: $([Math]::Round($maxTime, 2))ms" -ForegroundColor Gray
    }
}

# Generate report
Write-Host "`nGenerating report..." -ForegroundColor Cyan

$report = @"
# ExplorerLens GPU Acceleration - Quick Test Results

**Test Date:** $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")
**Machine:** $env:COMPUTERNAME
**Iterations:** $iterations per test

## Test Configuration

- **Test Images:** $($testImages.Count) images
- **Thumbnail Sizes:** $($thumbnailSizes -join ', ')px
- **Test Mode:** Quick (simulated thumbnail generation)

## Performance Results

| Image Size | Thumbnail Size | Avg Time (ms) | Min (ms) | Max (ms) | Median (ms) |
|-----------|---------------|--------------|----------|----------|------------|
"@

foreach ($result in $results) {
    $report += "`n| $($result.ImageSize) | $($result.ThumbnailSize) | $($result.AvgTime) | $($result.MinTime) | $($result.MaxTime) | $($result.MedianTime) |"
}

$report += @"


## Notes

⚠️ **SIMULATION MODE**: This quick test uses simulated thumbnail generation times.
   To get actual GPU performance data, integrate with LENSShell thumbnail API.

## Next Steps

1. **Integrate LENSShell API**: Replace simulation with actual COM calls to IThumbnailProvider
2. **Add GPU Detection**: Query DirectX device info from gpu_accelerator.cpp
3. **CPU Comparison**: Add CPU-only fallback tests for comparison
4. **Memory Profiling**: Track texture pool memory usage
5. **Multi-GPU Testing**: Test on Intel, NVIDIA, and AMD GPUs

---
*Generated by Quick-GPUTest.ps1*
"@

# Save report
$reportPath = $OutputReport
Set-Content -Path $reportPath -Value $report -Force

Write-Host "`n✅ Quick test complete!" -ForegroundColor Green
Write-Host "   Report saved to: $reportPath" -ForegroundColor Cyan
Write-Host "`n📊 Results Summary:" -ForegroundColor Cyan
Write-Host "   - Test images created: $($testImages.Count)" -ForegroundColor White
Write-Host "   - Benchmark tests run: $($results.Count)" -ForegroundColor White
Write-Host "   - Total iterations: $($results.Count * $iterations)" -ForegroundColor White

Write-Host "`n⚠️  NOTE: Using simulated data. Integrate LENSShell for real benchmarks.`n" -ForegroundColor Yellow

