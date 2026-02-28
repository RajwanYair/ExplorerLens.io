# Test-RealFileDecoding.ps1
# Integration test script for real file decoding
# Date: February 17, 2026

[CmdletBinding()]
param(
    [Parameter()]
    [string]$CorpusPath = "$PSScriptRoot\data\corpus",
    
    [Parameter()]
    [string]$ValidatorPath = "$PSScriptRoot\..\build\bin\Release\ExplorerLensValidator.exe",
    
    [Parameter()]
    [string]$OutputPath = "$PSScriptRoot\validation-results.csv",
    
    [Parameter()]
    [switch]$ExitOnFailure = $false
)

$ErrorActionPreference = "Stop"

Write-Host "=== ExplorerLens Real-File Decoding Test ===" -ForegroundColor Cyan
Write-Host "Corpus: $CorpusPath"
Write-Host "Validator: $ValidatorPath"
Write-Host ""

# Check if validator exists
if (-not (Test-Path $ValidatorPath)) {
    Write-Host "ERROR: Validator not found: $ValidatorPath" -ForegroundColor Red
    Write-Host "Build it first: cmake --build build --config Release --target ExplorerLensValidator" -ForegroundColor Yellow
    exit 1
}

# Check if corpus exists
if (-not (Test-Path $CorpusPath)) {
    Write-Host "ERROR: Test corpus not found: $CorpusPath" -ForegroundColor Red
    Write-Host "Create it first: .\tests\Organize-TestCorpus.ps1 -CreateInvalidFiles" -ForegroundColor Yellow
    exit 1
}

# Run validator
Write-Host "Running validator..." -ForegroundColor Cyan
$startTime = Get-Date

try {
    & $ValidatorPath -v -o $OutputPath $CorpusPath
    $exitCode = $LASTEXITCODE
    
    $duration = (Get-Date) - $startTime
    
    Write-Host ""
    Write-Host "Validation completed in $($duration.TotalSeconds) seconds" -ForegroundColor Cyan
    Write-Host "Results exported to: $OutputPath" -ForegroundColor Green
    
    # Parse results
    if (Test-Path $OutputPath) {
        $results = Import-Csv $OutputPath
        $totalFiles = $results.Count
        $successCount = ($results | Where-Object { $_.Success -eq "TRUE" }).Count
        $failureCount = $totalFiles - $successCount
        
        Write-Host ""
        Write-Host "=== Summary ===" -ForegroundColor Cyan
        Write-Host "Total Files:  $totalFiles"
        Write-Host "Success:      $successCount ($([math]::Round(100.0 * $successCount / $totalFiles, 1))%)" -ForegroundColor Green
        Write-Host "Failures:     $failureCount" -ForegroundColor $(if ($failureCount -gt 0) { "Yellow" } else { "Gray" })
        
        # Show failures
        if ($failureCount -gt 0) {
            Write-Host ""
            Write-Host "Failed files:" -ForegroundColor Yellow
            $results | Where-Object { $_.Success -ne "TRUE" } | ForEach-Object {
                Write-Host "  - $($_.FilePath): $($_.ErrorMessage)" -ForegroundColor Red
            }
        }
    }
    
    if ($ExitOnFailure -and $exitCode -ne 0) {
        Write-Host ""
        Write-Host "TEST FAILED: Validator returned exit code $exitCode" -ForegroundColor Red
        exit $exitCode
    }
    
    Write-Host ""
    Write-Host "✓ Real-file decoding test complete" -ForegroundColor Green
    
} catch {
    Write-Host "ERROR: Validator execution failed: $_" -ForegroundColor Red
    if ($ExitOnFailure) {
        exit 1
    }
}

