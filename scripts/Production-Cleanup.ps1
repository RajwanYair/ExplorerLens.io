# Production-Cleanup.ps1
# Clean and organize DarkThumbs project for production readiness

param([switch]$WhatIf = $false)

$ErrorActionPreference = "Stop"
$root = $PSScriptRoot | Split-Path

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "DarkThumbs Production Cleanup" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

if ($WhatIf) {
    Write-Host "⚠️  WHATIF MODE" -ForegroundColor Yellow
    Write-Host ""
}

# Clean Build Artifacts
Write-Host "Cleaning Build Artifacts..." -ForegroundColor Green

$artifactDirs = @("build", "x64", "packages", "CBXShell\x64", "CBXManager\x64")

foreach ($dir in $artifactDirs) {
    $path = Join-Path $root $dir
    if (Test-Path $path) {
        $size = (Get-ChildItem $path -Recurse -File -ErrorAction SilentlyContinue | Measure-Object -Property Length -Sum).Sum / 1MB
        Write-Host "  Removing: $dir ($([math]::Round($size, 1)) MB)" -ForegroundColor Yellow
        
        if (-not $WhatIf) {
            Remove-Item $path -Recurse -Force -ErrorAction SilentlyContinue
        }
    }
}

# Clean log files
Write-Host "`nCleaning Log Files..." -ForegroundColor Green
$logPath = Join-Path $root "build-logs"
if (Test-Path $logPath) {
    $logs = Get-ChildItem $logPath -Filter "*.log"
    foreach ($log in $logs) {
        Write-Host "  Removing: $($log.Name)" -ForegroundColor Yellow
        if (-not $WhatIf) {
            Remove-Item $log.FullName -Force
        }
    }
}

# Clean test output
if (Test-Path (Join-Path $root "jxl-test-build.log")) {
    Write-Host "  Removing: jxl-test-build.log" -ForegroundColor Yellow
    if (-not $WhatIf) {
        Remove-Item (Join-Path $root "jxl-test-build.log") -Force
    }
}

Write-Host "`n✅ Cleanup Complete!" -ForegroundColor Green
Write-Host ""
Write-Host "Next: Run full rebuild to verify project" -ForegroundColor Cyan
