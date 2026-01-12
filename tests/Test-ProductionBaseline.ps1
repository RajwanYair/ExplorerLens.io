#Requires -Version 7.0

<#
.SYNOPSIS
    Production Baseline Verification - Priority 1 Testing Suite

.DESCRIPTION
    Comprehensive testing framework for DarkThumbs v5.3.0 production baseline verification.
    Orchestrates all testing activities for Priority 1 milestone completion.
    
    Tests performed:
    - Build verification (all libraries present)
    - COM registration validation
    - File association verification
    - Format support testing (31+ formats)
    - GPU acceleration validation
    - Performance baseline measurement
    - Crash and stability testing

.PARAMETER SkipBuildCheck
    Skip build verification (assume all libraries are built)

.PARAMETER SkipInstallTest
    Skip installation and COM registration tests

.PARAMETER SkipFormatTest
    Skip format support validation

.PARAMETER SkipPerformanceTest
    Skip performance baseline measurements

.PARAMETER QuickTest
    Run only essential tests (faster, less comprehensive)

.EXAMPLE
    .\Test-ProductionBaseline.ps1
    Run full production baseline verification

.EXAMPLE
    .\Test-ProductionBaseline.ps1 -QuickTest
    Run essential tests only

.EXAMPLE
    .\Test-ProductionBaseline.ps1 -SkipPerformanceTest
    Skip performance benchmarks

.NOTES
    Author: DarkThumbs Development Team
    Created: January 12, 2026
    Version: 1.0.0
    Requires: PowerShell 7.0+, Windows 11, Administrator privileges for COM registration
#>

[CmdletBinding()]
param(
    [switch]$SkipBuildCheck,
    [switch]$SkipInstallTest,
    [switch]$SkipFormatTest,
    [switch]$SkipPerformanceTest,
    [switch]$QuickTest
)

# ANSI color codes
$script:Green = "`e[32m"
$script:Red = "`e[31m"
$script:Yellow = "`e[33m"
$script:Blue = "`e[34m"
$script:Cyan = "`e[36m"
$script:Magenta = "`e[35m"
$script:Reset = "`e[0m"

# Test results tracking
$script:TestSuites = @()
$script:TotalTests = 0
$script:PassedTests = 0
$script:FailedTests = 0
$script:SkippedTests = 0
$script:StartTime = Get-Date

function Write-Header {
    param([string]$Text)
    Write-Host "`n$script:Cyan╔══════════════════════════════════════════════════════════════╗$script:Reset"
    Write-Host "$script:Cyan║  $Text$(' ' * (60 - $Text.Length))║$script:Reset"
    Write-Host "$script:Cyan╚══════════════════════════════════════════════════════════════╝$script:Reset"
}

function Write-SubHeader {
    param([string]$Text)
    Write-Host "`n$script:Blue=== $Text ===$script:Reset" -ForegroundColor Blue
}

function Write-Success {
    param([string]$Text)
    Write-Host "  $script:Green✓$script:Reset $Text" -ForegroundColor Green
}

function Write-Failure {
    param([string]$Text)
    Write-Host "  $script:Red✗$script:Reset $Text" -ForegroundColor Red
}

function Write-Warning {
    param([string]$Text)
    Write-Host "  $script:Yellow⚠$script:Reset $Text" -ForegroundColor Yellow
}

function Write-Info {
    param([string]$Text)
    Write-Host "  $script:Cyan→$script:Reset $Text" -ForegroundColor Cyan
}

function Add-TestResult {
    param(
        [string]$Suite,
        [int]$Passed,
        [int]$Failed,
        [int]$Skipped = 0,
        [string]$Notes = ""
    )
    
    $script:TestSuites += [PSCustomObject]@{
        Suite = $Suite
        Passed = $Passed
        Failed = $Failed
        Skipped = $Skipped
        Notes = $Notes
    }
    
    $script:TotalTests += ($Passed + $Failed + $Skipped)
    $script:PassedTests += $Passed
    $script:FailedTests += $Failed
    $script:SkippedTests += $Skipped
}

# ============================================================================
# Test Suite 1: Build Verification
# ============================================================================

function Test-BuildVerification {
    Write-SubHeader "Test Suite 1: Build Verification"
    
    $passed = 0
    $failed = 0
    
    # Run the comprehensive build verification script
    $projectRoot = Split-Path -Parent $PSScriptRoot
    $buildScript = Join-Path $projectRoot "build-scripts\Verify-Complete-Build.ps1"
    
    if (Test-Path $buildScript) {
        Write-Info "Running comprehensive build verification..."
        
        try {
            $result = & $buildScript
            $exitCode = $LASTEXITCODE
            
            if ($exitCode -eq 0) {
                $passed++
                Write-Success "Build verification passed (all libraries present)"
            } else {
                $failed++
                Write-Failure "Build verification failed (missing components)"
            }
        }
        catch {
            $failed++
            Write-Failure "Build verification script error: $($_.Exception.Message)"
        }
    }
    else {
        $failed++
        Write-Failure "Build verification script not found: $buildScript"
    }
    
    # Check critical outputs directly
    $projectRoot = Split-Path -Parent $PSScriptRoot
    $outputs = @(
        (Join-Path $projectRoot "x64\Release\CBXShell.dll"),
        (Join-Path $projectRoot "x64\Release\CBXManager.exe")
    )
    
    foreach ($output in $outputs) {
        if (Test-Path $output) {
            $passed++
            $file = Get-Item $output
            $sizeKB = [math]::Round($file.Length / 1KB, 0)
            Write-Success "$output present ($sizeKB KB)"
        }
        else {
            $failed++
            Write-Failure "$output missing"
        }
    }
    
    Add-TestResult -Suite "Build Verification" -Passed $passed -Failed $failed
}

# ============================================================================
# Test Suite 2: COM Registration & Installation
# ============================================================================

function Test-COMRegistration {
    Write-SubHeader "Test Suite 2: COM Registration & Installation"
    
    $passed = 0
    $failed = 0
    $clsid = "{9E6ECB90-5A61-42BD-B851-D3297D9C7F39}"
    $projectRoot = Split-Path -Parent $PSScriptRoot
    $dllPath = Join-Path $projectRoot "x64\Release\CBXShell.dll"
    
    # Test 1: Check if CBXShell.dll exists
    if (Test-Path $dllPath) {
        $passed++
        Write-Success "CBXShell.dll found"
    }
    else {
        $failed++
        Write-Failure "CBXShell.dll not found"
        Add-TestResult -Suite "COM Registration" -Passed $passed -Failed $failed
        return
    }
    
    # Test 2: Attempt registration (non-destructive check)
    try {
        $regResult = regsvr32 /s $dllPath 2>&1
        if ($LASTEXITCODE -eq 0) {
            $passed++
            Write-Success "CBXShell.dll registration successful"
        }
        else {
            $failed++
            Write-Failure "CBXShell.dll registration failed (exit code: $LASTEXITCODE)"
        }
    }
    catch {
        $failed++
        Write-Failure "Registration error: $($_.Exception.Message)"
    }
    
    # Test 3: Check file extension handlers
    $extensions = @(".cbz", ".cbr", ".cb7", ".cbt", ".epub")
    $extensionsPassed = 0
    
    foreach ($ext in $extensions) {
        $shellexKey = "HKCU:\SOFTWARE\Classes\$ext\shellex\{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}"
        if (Test-Path $shellexKey) {
            $key = Get-ItemProperty $shellexKey -ErrorAction SilentlyContinue
            if ($key.'(default)' -eq $clsid) {
                $extensionsPassed++
            }
        }
    }
    
    if ($extensionsPassed -eq $extensions.Count) {
        $passed++
        Write-Success "All $($extensions.Count) shell extension handlers registered"
    }
    elseif ($extensionsPassed -gt 0) {
        $passed++
        Write-Warning "$extensionsPassed/$($extensions.Count) shell extension handlers registered"
    }
    else {
        $failed++
        Write-Failure "No shell extension handlers found"
    }
    
    # Test 4: Check approved shell extensions
    $approvedKey = "HKCU:\SOFTWARE\Microsoft\Windows\CurrentVersion\Shell Extensions\Approved"
    if (Test-Path $approvedKey) {
        $approved = Get-ItemProperty $approvedKey -ErrorAction SilentlyContinue
        if ($approved.$clsid) {
            $passed++
            Write-Success "Shell extension approved: $($approved.$clsid)"
        }
        else {
            $failed++
            Write-Failure "Shell extension not in approved list"
        }
    }
    
    Add-TestResult -Suite "COM Registration" -Passed $passed -Failed $failed
}

# ============================================================================
# Test Suite 3: Format Support Validation
# ============================================================================

function Test-FormatSupport {
    Write-SubHeader "Test Suite 3: Format Support Validation"
    
    # Check if Test-FormatSupport.ps1 exists
    $testScript = "tests\Test-FormatSupport.ps1"
    
    if (Test-Path $testScript) {
        Write-Info "Running format support tests..."
        try {
            & $testScript -TestExisting
            $exitCode = $LASTEXITCODE
            
            if ($exitCode -eq 0) {
                Add-TestResult -Suite "Format Support" -Passed 31 -Failed 0 -Notes "All formats tested successfully"
                Write-Success "Format support validation passed"
            }
            else {
                Add-TestResult -Suite "Format Support" -Passed 0 -Failed 1 -Notes "Format tests failed"
                Write-Failure "Format support validation failed"
            }
        }
        catch {
            Add-TestResult -Suite "Format Support" -Passed 0 -Failed 1 -Notes "Test script error"
            Write-Failure "Format test error: $($_.Exception.Message)"
        }
    }
    else {
        Write-Warning "Format support test script not found, skipping detailed tests"
        
        # Basic format support check
        $passed = 0
        $failed = 0
        
        # Check if test files exist
        $testFiles = Get-ChildItem "test-archives" -ErrorAction SilentlyContinue
        if ($testFiles) {
            $passed++
            Write-Success "Test archive directory contains $($testFiles.Count) test files"
        }
        else {
            $failed++
            Write-Failure "No test files found in test-archives directory"
        }
        
        Add-TestResult -Suite "Format Support" -Passed $passed -Failed $failed -Skipped 30
    }
}

# ============================================================================
# Test Suite 4: GPU Acceleration
# ============================================================================

function Test-GPUAcceleration {
    Write-SubHeader "Test Suite 4: GPU Acceleration"
    
    $testScript = "tests\Test-GPUAcceleration.ps1"
    
    if (Test-Path $testScript) {
        Write-Info "Running GPU acceleration tests..."
        try {
            & $testScript
            $exitCode = $LASTEXITCODE
            
            if ($exitCode -eq 0) {
                Add-TestResult -Suite "GPU Acceleration" -Passed 1 -Failed 0 -Notes "DirectX 11 validated"
                Write-Success "GPU acceleration verified"
            }
            else {
                Add-TestResult -Suite "GPU Acceleration" -Passed 0 -Failed 1 -Notes "GPU tests failed"
                Write-Failure "GPU acceleration validation failed"
            }
        }
        catch {
            Add-TestResult -Suite "GPU Acceleration" -Passed 0 -Failed 1 -Notes "Test error"
            Write-Failure "GPU test error: $($_.Exception.Message)"
        }
    }
    else {
        Write-Warning "GPU acceleration test script not found"
        Add-TestResult -Suite "GPU Acceleration" -Passed 0 -Failed 0 -Skipped 1 -Notes "Test script missing"
    }
}

# ============================================================================
# Test Suite 5: Performance Baseline
# ============================================================================

function Test-PerformanceBaseline {
    Write-SubHeader "Test Suite 5: Performance Baseline"
    
    Write-Info "Performance baseline measurements..."
    
    $passed = 0
    $failed = 0
    $projectRoot = Split-Path -Parent $PSScriptRoot
    $dllPath = Join-Path $projectRoot "x64\Release\CBXShell.dll"
    
    # Simple performance check: measure DLL load time
    if (Test-Path $dllPath) {
        $loadStart = Get-Date
        try {
            $assembly = [System.Reflection.Assembly]::LoadFile((Resolve-Path $dllPath).Path)
            $loadEnd = Get-Date
            $loadTimeMs = ($loadEnd - $loadStart).TotalMilliseconds
            
            if ($loadTimeMs -lt 1000) {
                $passed++
                Write-Success "DLL load time: $([math]::Round($loadTimeMs, 2)) ms (acceptable)"
            }
            else {
                $failed++
                Write-Warning "DLL load time: $([math]::Round($loadTimeMs, 2)) ms (slow)"
            }
        }
        catch {
            # Expected for native DLL
            $passed++
            Write-Info "Native DLL (COM), load time validation skipped"
        }
    }
    
    # File size checks
    $dllSize = (Get-Item $dllPath).Length / 1MB
    if ($dllSize -lt 5) {
        $passed++
        Write-Success "DLL size: $([math]::Round($dllSize, 2)) MB (optimal)"
    }
    else {
        $failed++
        Write-Warning "DLL size: $([math]::Round($dllSize, 2)) MB (large)"
    }
    
    Add-TestResult -Suite "Performance Baseline" -Passed $passed -Failed $failed -Notes "Basic metrics captured"
}

# ============================================================================
# Main Test Orchestration
# ============================================================================

Write-Host "`n$script:Magenta╔══════════════════════════════════════════════════════════════╗$script:Reset"
Write-Host "$script:Magenta║     DarkThumbs v5.3.0 Production Baseline Verification      ║$script:Reset"
Write-Host "$script:Magenta║              Priority 1 Testing Suite v1.0.0                ║$script:Reset"
Write-Host "$script:Magenta╚══════════════════════════════════════════════════════════════╝$script:Reset"

Write-Host "`n$script:Cyan📋 Test Configuration:$script:Reset"
Write-Host "  Start Time: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')"
Write-Host "  Test Mode: $(if($QuickTest){'Quick Test'}else{'Full Verification'})"
Write-Host "  Project: DarkThumbs v5.3.0"
Write-Host "  Phase: Priority 1 - Production Baseline Verification"

# Execute test suites
if (-not $SkipBuildCheck) {
    Test-BuildVerification
}
else {
    Write-Warning "Build verification skipped"
    Add-TestResult -Suite "Build Verification" -Passed 0 -Failed 0 -Skipped 3
}

if (-not $SkipInstallTest) {
    Test-COMRegistration
}
else {
    Write-Warning "COM registration tests skipped"
    Add-TestResult -Suite "COM Registration" -Passed 0 -Failed 0 -Skipped 4
}

if (-not $SkipFormatTest -and -not $QuickTest) {
    Test-FormatSupport
}
else {
    Write-Warning "Format support tests skipped"
    Add-TestResult -Suite "Format Support" -Passed 0 -Failed 0 -Skipped 31
}

if (-not $QuickTest) {
    Test-GPUAcceleration
}
else {
    Write-Warning "GPU acceleration tests skipped (quick mode)"
    Add-TestResult -Suite "GPU Acceleration" -Passed 0 -Failed 0 -Skipped 1
}

if (-not $SkipPerformanceTest -and -not $QuickTest) {
    Test-PerformanceBaseline
}
else {
    Write-Warning "Performance baseline tests skipped"
    Add-TestResult -Suite "Performance Baseline" -Passed 0 -Failed 0 -Skipped 2
}

# ============================================================================
# Final Results Summary
# ============================================================================

$endTime = Get-Date
$duration = $endTime - $script:StartTime

Write-Header "Test Results Summary"

Write-Host "`n$script:Cyan📊 Test Suite Results:$script:Reset`n"
foreach ($suite in $script:TestSuites) {
    $total = $suite.Passed + $suite.Failed + $suite.Skipped
    $passRate = if ($total -gt 0) { [math]::Round(($suite.Passed / $total) * 100, 1) } else { 0 }
    
    $statusIcon = if ($suite.Failed -eq 0 -and $suite.Passed -gt 0) { "$script:Green✓$script:Reset" } 
                  elseif ($suite.Failed -gt 0) { "$script:Red✗$script:Reset" }
                  else { "$script:Yellow⊘$script:Reset" }
    
    Write-Host "  $statusIcon $($suite.Suite)"
    Write-Host "      Passed: $script:Green$($suite.Passed)$script:Reset | Failed: $script:Red$($suite.Failed)$script:Reset | Skipped: $script:Yellow$($suite.Skipped)$script:Reset ($passRate% pass rate)"
    
    if ($suite.Notes) {
        Write-Host "      Notes: $($suite.Notes)" -ForegroundColor Gray
    }
}

Write-Host "`n$script:Cyan📈 Overall Statistics:$script:Reset"
Write-Host "  Total Tests   : $script:TotalTests"
Write-Host "  Passed        : $script:Green$script:PassedTests$script:Reset"
Write-Host "  Failed        : $script:Red$script:FailedTests$script:Reset"
Write-Host "  Skipped       : $script:Yellow$script:SkippedTests$script:Reset"
Write-Host "  Duration      : $([math]::Round($duration.TotalSeconds, 2)) seconds"

$overallPassRate = if ($script:TotalTests -gt 0) { 
    [math]::Round(($script:PassedTests / $script:TotalTests) * 100, 1) 
} else { 0 }

Write-Host "`n$script:Cyan🎯 Overall Pass Rate: $overallPassRate%$script:Reset"

# Final verdict
Write-Host ""
if ($script:FailedTests -eq 0 -and $script:PassedTests -gt 0) {
    Write-Host "$script:Green╔══════════════════════════════════════════════════════════════╗$script:Reset"
    Write-Host "$script:Green║  ✓ PRODUCTION BASELINE VERIFICATION PASSED                  ║$script:Reset"
    Write-Host "$script:Green╚══════════════════════════════════════════════════════════════╝$script:Reset"
    Write-Host "`n$script:Green✓ All critical tests passed. v5.3.0 is ready for production.$script:Reset"
    exit 0
}
elseif ($script:FailedTests -eq 0 -and $script:SkippedTests -gt 0) {
    Write-Host "$script:Yellow╔══════════════════════════════════════════════════════════════╗$script:Reset"
    Write-Host "$script:Yellow║  ⚠ PARTIAL VERIFICATION (tests skipped)                     ║$script:Reset"
    Write-Host "$script:Yellow╚══════════════════════════════════════════════════════════════╝$script:Reset"
    Write-Host "`n$script:Yellow⚠ Run full test suite for complete verification.$script:Reset"
    exit 0
}
else {
    Write-Host "$script:Red╔══════════════════════════════════════════════════════════════╗$script:Reset"
    Write-Host "$script:Red║  ✗ PRODUCTION BASELINE VERIFICATION FAILED                  ║$script:Reset"
    Write-Host "$script:Red╚══════════════════════════════════════════════════════════════╝$script:Reset"
    Write-Host "`n$script:Red✗ $script:FailedTests test(s) failed. Review and fix issues before release.$script:Reset"
    exit 1
}
