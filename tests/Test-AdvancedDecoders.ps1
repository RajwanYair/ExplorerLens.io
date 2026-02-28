# Test-AdvancedDecoders.ps1
# Validates PSD, SVG, and EPUB decoder functionality
# Date: February 17, 2026

[CmdletBinding()]
param(
    [Parameter()]
    [string]$TestCorpusPath = "$PSScriptRoot\data\corpus",
    
    [Parameter()]
    [string]$ValidatorPath = "$PSScriptRoot\..\build\bin\Release\ExplorerLensValidator.exe",
    
    [Parameter()]
    [switch]$Verbose
)

$ErrorActionPreference = "Stop"

Write-Host "=== ExplorerLens Advanced Decoder Testing ===" -ForegroundColor Cyan
Write-Host "PSD, SVG, EPUB Format Support" -ForegroundColor Cyan
Write-Host ""

# Check if validator exists
if (-not (Test-Path $ValidatorPath)) {
    Write-Host "ERROR: Validator not found: $ValidatorPath" -ForegroundColor Red
    Write-Host "Build it first: cmake --build build --config Release --target ExplorerLensValidator" -ForegroundColor Yellow
    exit 1
}

# Test formats for this module
$testFormats = @{
    "PSD" = @{
        Path = Join-Path $TestCorpusPath "images\psd"
        Description = "Photoshop Documents"
        Extensions = @(".psd", ".psb")
    }
    "SVG" = @{
        Path = Join-Path $TestCorpusPath "svg"
        Description = "Scalable Vector Graphics"
        Extensions = @(".svg", ".svgz")
    }
    "EPUB" = @{
        Path = Join-Path $TestCorpusPath "documents\epub"
        Description = "Electronic Publications (eBooks)"
        Extensions = @(".epub")
    }
}

$totalTests = 0
$passedTests = 0
$failedTests = 0
$results = @()

foreach ($format in $testFormats.Keys) {
    $formatInfo = $testFormats[$format]
    $formatPath = $formatInfo.Path
    
    Write-Host "=== Testing $format Decoder ===" -ForegroundColor Yellow
    Write-Host "Description: $($formatInfo.Description)" -ForegroundColor Gray
    Write-Host "Path: $formatPath" -ForegroundColor Gray
    Write-Host ""
    
    if (-not (Test-Path $formatPath)) {
        Write-Host "⚠ Directory not found: $formatPath" -ForegroundColor Yellow
        Write-Host "  Skipping $format tests" -ForegroundColor Gray
        Write-Host ""
        continue
    }
    
    # Get test files
    $testFiles = Get-ChildItem -Path $formatPath -File -Recurse | 
                 Where-Object { $formatInfo.Extensions -contains $_.Extension.ToLower() }
    
    if ($testFiles.Count -eq 0) {
        Write-Host "⚠ No $format test files found" -ForegroundColor Yellow
        Write-Host ""
        continue
    }
    
    Write-Host "Found $($testFiles.Count) $format file(s)" -ForegroundColor Cyan
    
    foreach ($file in $testFiles) {
        $totalTests++
        
        # Run validator on single file
        $tempOutput = Join-Path $env:TEMP "validator-output-$(Get-Random).csv"
        
        try {
            $output = & $ValidatorPath -o $tempOutput $file.FullName 2>&1
            $exitCode = $LASTEXITCODE
            
            # Parse results
            if (Test-Path $tempOutput) {
                $result = Import-Csv $tempOutput | Select-Object -First 1
                
                if ($result.Success -eq "TRUE") {
                    $passed Tests++
                    $status = "✓ PASS"
                    $color = "Green"
                } else {
                    $failedTests++
                    $status = "✗ FAIL"
                    $color = "Red"
                }
                
                $results += [PSCustomObject]@{
                    Format = $format
                    FileName = $file.Name
                    Success = ($result.Success -eq "TRUE")
                    TimeMs = $result.TimeMs
                    ErrorMessage = $result.ErrorMessage
                }
                
                if ($Verbose) {
                    Write-Host "  $status $($file.Name) ($($result.TimeMs) ms)" -ForegroundColor $color
                    if ($result.ErrorMessage) {
                        Write-Host "    Error: $($result.ErrorMessage)" -ForegroundColor Red
                    }
                }
                
                Remove-Item -Path $tempOutput -Force -ErrorAction SilentlyContinue
            } else {
                $failedTests++
                Write-Host "  ✗ FAIL $($file.Name) (validator produced no output)" -ForegroundColor Red
            }
            
        } catch {
            $failedTests++
            Write-Host "  ✗ FAIL $($file.Name) (exception: $_)" -ForegroundColor Red
        }
    }
    
    Write-Host ""
}

# Summary
Write-Host "==================================" -ForegroundColor Cyan
Write-Host "     DECODER TEST SUMMARY        " -ForegroundColor Cyan
Write-Host "==================================" -ForegroundColor Cyan
Write-Host "Total Tests:     $totalTests" -ForegroundColor White
Write-Host "Passed:          $passedTests " -NoNewline
Write-Host "($([math]::Round(100.0 * $passedTests / [Math]::Max($totalTests, 1), 1))%)" -ForegroundColor Green
Write-Host "Failed:          $failedTests" -ForegroundColor $(if ($failedTests -gt 0) { "Red" } else { "White" })
Write-Host "==================================" -ForegroundColor Cyan
Write-Host ""

# Format-specific summary
$groupedResults = $results | Group-Object -Property Format
foreach ($group in $groupedResults) {
    $formatPassed = ($group.Group | Where-Object { $_.Success }).Count
    $formatTotal = $group.Count
    $formatPct = [math]::Round(100.0 * $formatPassed / $formatTotal, 1)
    
    $statusColor = if ($formatPct -eq 100) { "Green" } 
                   elseif ($formatPct -ge 80) { "Yellow" } 
                   else { "Red" }
    
    Write-Host "$($group.Name): $formatPassed/$formatTotal ($formatPct%)" -ForegroundColor $statusColor
}

Write-Host ""

# ExplorerLens
$exitCriteriaMet = ($passedTests -eq $totalTests) -and ($totalTests -gt 0)

if ($exitCriteriaMet) {
    Write-Host "✓  Exit Criteria MET: All advanced decoders operational" -ForegroundColor Green
} else {
    if ($totalTests -eq 0) {
        Write-Host "⚠  Exit Criteria INCONCLUSIVE: No test files found" -ForegroundColor Yellow
        Write-Host "  Add test files to ${TestCorpusPath}\images\psd, ${TestCorpusPath}\svg, ${TestCorpusPath}\documents\epub" -ForegroundColor Gray
    } else {
        Write-Host "✗  Exit Criteria NOT MET: $failedTests decoder failures" -ForegroundColor Red
    }
}

Write-Host ""

# Show failures in detail
if ($fail edTests -gt 0) {
    Write-Host "=== Failed Tests ===" -ForegroundColor Red
    $results | Where-Object { -not $_.Success } | ForEach-Object {
        Write-Host "  $($_.Format): $($_.FileName)" -ForegroundColor Yellow
        if ($_.ErrorMessage) {
            Write-Host "    $($_.ErrorMessage)" -ForegroundColor Gray
        }
    }
    Write-Host ""
}

Write-Host "Advanced decoder testing complete." -ForegroundColor Cyan

exit $(if ($exitCriteriaMet) { 0 } else { 1 })

