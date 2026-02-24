#Requires -Version 7.0
#Requires -RunAsAdministrator

<#
.SYNOPSIS
    Installation and registration testing for ExplorerLens LENSShell extension
.DESCRIPTION
    Tests the complete installation workflow including:
    - File deployment to install directory
    - DLL registration via regsvr32
    - Registry entry validation
    - Shell extension activation
    - Uninstallation and cleanup
.EXAMPLE
    .\Test-Installation.ps1
    .\Test-Installation.ps1 -SkipUninstall
#>

param(
    [switch]$SkipUninstall,
    [switch]$Verbose
)

$ErrorActionPreference = "Continue"
$testResults = @{
    Passed   = @()
    Failed   = @()
    Warnings = @()
}

function Write-TestResult {
    param(
        [string]$TestName,
        [bool]$Success,
        [string]$Message = ""
    )
    
    if ($Success) {
        Write-Host "  ✅ $TestName" -ForegroundColor Green
        if ($Message) { Write-Host "     $Message" -ForegroundColor Gray }
        $script:testResults.Passed += $TestName
    }
    else {
        Write-Host "  ❌ $TestName" -ForegroundColor Red
        if ($Message) { Write-Host "     $Message" -ForegroundColor Yellow }
        $script:testResults.Failed += $TestName
    }
}

function Write-TestWarning {
    param([string]$Message)
    Write-Host "  ⚠️  $Message" -ForegroundColor Yellow
    $script:testResults.Warnings += $Message
}

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "  ExplorerLens Installation Test Suite" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

# Test 1: Pre-installation checks
Write-Host "📋 Phase 1: Pre-Installation Checks" -ForegroundColor Cyan

$dllPath = "install\x64\LENSShell.dll"
Write-TestResult "LENSShell.dll exists" (Test-Path $dllPath) -Message (if (Test-Path $dllPath) { "Found at: $dllPath" } else { "Not found!" })

if (Test-Path $dllPath) {
    $dll = Get-Item $dllPath
    $expectedSize = 1.0  # MB minimum
    $actualSize = [Math]::Round($dll.Length / 1MB, 2)
    Write-TestResult "LENSShell.dll size check" ($actualSize -gt $expectedSize) -Message "$actualSize MB"
}

$managerPath = "install\x64\LENSManager.exe"
Write-TestResult "LENSManager.exe exists" (Test-Path $managerPath)

# Test 2: Installation
Write-Host "`n📦 Phase 2: Installation" -ForegroundColor Cyan

try {
    Write-Host "  Running: .\explorerlens.ps1 -Install" -ForegroundColor Yellow
    
    # Backup current state
    $backupNeeded = Test-Path "$env:SystemRoot\System32\LENSShell.dll"
    if ($backupNeeded) {
        Write-TestWarning "LENSShell.dll already exists in System32 - will be replaced"
    }
    
    # Run installation
    $installOutput = & ".\explorerlens.ps1" -Install 2>&1
    
    if ($LASTEXITCODE -eq 0 -or $?) {
        Write-TestResult "Installation completed" $true -Message "Exit code: $LASTEXITCODE"
    }
    else {
        Write-TestResult "Installation completed" $false -Message "Exit code: $LASTEXITCODE"
        Write-Host "Output: $installOutput" -ForegroundColor Gray
    }
}
catch {
    Write-TestResult "Installation completed" $false -Message "Exception: $_"
}

# Test 3: File deployment verification
Write-Host "`n📁 Phase 3: File Deployment Verification" -ForegroundColor Cyan

$system32Dll = "$env:SystemRoot\System32\LENSShell.dll"
Write-TestResult "LENSShell.dll in System32" (Test-Path $system32Dll)

if (Test-Path $system32Dll) {
    $deployedDll = Get-Item $system32Dll
    $deployedSize = [Math]::Round($deployedDll.Length / 1MB, 2)
    Write-Host "     Size: $deployedSize MB, Modified: $($deployedDll.LastWriteTime)" -ForegroundColor Gray
}

# Test 4: DLL registration check
Write-Host "`n🔧 Phase 4: DLL Registration" -ForegroundColor Cyan

try {
    # Check if DLL is registered by looking for known CLSIDs
    # Note: Actual CLSID would need to be extracted from LENSShell.idl
    $regPath = "HKCR:\CLSID"
    
    if (Test-Path "Registry::HKEY_CLASSES_ROOT\CLSID") {
        Write-TestResult "Registry CLSID hive accessible" $true
        
        # Search for LENSShell registration
        # This is a heuristic - actual CLSID should be known
        $foundRegistration = $false
        try {
            $clsids = Get-ChildItem "Registry::HKEY_CLASSES_ROOT\CLSID" -ErrorAction SilentlyContinue
            foreach ($clsid in $clsids) {
                $inprocServer = Get-ItemProperty -Path "$($clsid.PSPath)\InprocServer32" -ErrorAction SilentlyContinue
                if ($inprocServer -and $inprocServer.'(default)' -like "*LENSShell.dll") {
                    $foundRegistration = $true
                    Write-Host "     Found CLSID: $($clsid.PSChildName)" -ForegroundColor Gray
                    break
                }
            }
        }
        catch {
            Write-TestWarning "Could not enumerate CLSIDs: $_"
        }
        
        Write-TestResult "LENSShell CLSID registered" $foundRegistration
    }
    else {
        Write-TestWarning "Cannot access HKCR registry hive"
    }
}
catch {
    Write-TestWarning "Registry check failed: $_"
}

# Test 5: Shell extension verification
Write-Host "`n🖼️  Phase 5: Shell Extension Check" -ForegroundColor Cyan

# Check for thumbnail provider registration
try {
    $thumbProviders = Get-ChildItem "Registry::HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\ShellIconOverlayIdentifiers" -ErrorAction SilentlyContinue
    
    Write-TestResult "Shell extension registry accessible" ($null -ne $thumbProviders)
}
catch {
    Write-TestWarning "Cannot check shell extensions: $_"
}

# Test 6: Functional test (if possible)
Write-Host "`n🎯 Phase 6: Functional Test" -ForegroundColor Cyan

# Create a test image
$testImagePath = "tests\test-images\install-test.png"
if (-not (Test-Path "tests\test-images")) {
    New-Item -Path "tests\test-images" -ItemType Directory -Force | Out-Null
}

try {
    # Create simple test PNG
    Add-Type -AssemblyName System.Drawing
    $bitmap = New-Object System.Drawing.Bitmap(256, 256)
    $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
    $graphics.Clear([System.Drawing.Color]::Blue)
    $bitmap.Save($testImagePath, [System.Drawing.Imaging.ImageFormat]::Png)
    $graphics.Dispose()
    $bitmap.Dispose()
    
    Write-TestResult "Test image created" (Test-Path $testImagePath) -Message $testImagePath
    
    # Note: Actual thumbnail extraction would require COM interface testing
    Write-Host "     Manual verification required:" -ForegroundColor Yellow
    Write-Host "       1. Open File Explorer" -ForegroundColor Gray
    Write-Host "       2. Navigate to: $(Resolve-Path 'tests\test-images')" -ForegroundColor Gray
    Write-Host "       3. Verify thumbnails are displayed" -ForegroundColor Gray
    
}
catch {
    Write-TestWarning "Could not create test image: $_"
}

# Test 7: Event log check
Write-Host "`n📋 Phase 7: Event Log Check" -ForegroundColor Cyan

try {
    $recentErrors = Get-WinEvent -FilterHashtable @{
        LogName   = 'Application'
        Level     = 2  # Error
        StartTime = (Get-Date).AddMinutes(-5)
    } -ErrorAction SilentlyContinue | Where-Object { $_.Message -like "*LENS*" -or $_.Message -like "*ExplorerLens*" }
    
    if ($recentErrors) {
        Write-TestResult "No installation errors in event log" $false -Message "Found $($recentErrors.Count) errors"
        $recentErrors | ForEach-Object {
            Write-Host "       $($_.TimeCreated): $($_.Message)" -ForegroundColor Gray
        }
    }
    else {
        Write-TestResult "No installation errors in event log" $true
    }
}
catch {
    Write-TestWarning "Could not check event log: $_"
}

# Test 8: Uninstallation (optional)
if (-not $SkipUninstall) {
    Write-Host "`n🗑️  Phase 8: Uninstallation Test" -ForegroundColor Cyan
    
    Write-Host "  Running: .\explorerlens.ps1 -Uninstall" -ForegroundColor Yellow
    
    try {
        $uninstallOutput = & ".\explorerlens.ps1" -Uninstall 2>&1
        
        Write-TestResult "Uninstallation completed" ($LASTEXITCODE -eq 0 -or $?)
        
        # Verify cleanup
        Start-Sleep -Seconds 2  # Give Windows time to release the DLL
        
        $dllGone = -not (Test-Path $system32Dll)
        Write-TestResult "LENSShell.dll removed from System32" $dllGone
        
        # Note: Registry cleanup verification would require knowing the exact CLSIDs
        
    }
    catch {
        Write-TestResult "Uninstallation completed" $false -Message "Exception: $_"
    }
}
else {
    Write-Host "`n⏭️  Phase 8: Uninstallation SKIPPED" -ForegroundColor Yellow
}

# Final summary
Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "  Test Results Summary" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

Write-Host "✅ Passed:   $($testResults.Passed.Count)" -ForegroundColor Green
$testResults.Passed | ForEach-Object { Write-Host "   - $_" -ForegroundColor White }

if ($testResults.Failed.Count -gt 0) {
    Write-Host "`n❌ Failed:   $($testResults.Failed.Count)" -ForegroundColor Red
    $testResults.Failed | ForEach-Object { Write-Host "   - $_" -ForegroundColor White }
}

if ($testResults.Warnings.Count -gt 0) {
    Write-Host "`n⚠️  Warnings: $($testResults.Warnings.Count)" -ForegroundColor Yellow
    $testResults.Warnings | ForEach-Object { Write-Host "   - $_" -ForegroundColor White }
}

$successRate = if (($testResults.Passed.Count + $testResults.Failed.Count) -gt 0) {
    [Math]::Round(($testResults.Passed.Count / ($testResults.Passed.Count + $testResults.Failed.Count)) * 100, 1)
}
else { 0 }

Write-Host "`n📊 Success Rate: $successRate%" -ForegroundColor Cyan

if ($successRate -ge 90) {
    Write-Host "`n🎉 Installation test PASSED!`n" -ForegroundColor Green
}
elseif ($successRate -ge 70) {
    Write-Host "`n⚠️  Installation test passed with warnings`n" -ForegroundColor Yellow
}
else {
    Write-Host "`n❌ Installation test FAILED`n" -ForegroundColor Red
}

# Save results
$timestamp = Get-Date -Format "yyyy-MM-dd_HHmmss"
$reportPath = "tests\test-results\installation-test-$timestamp.txt"

if (-not (Test-Path "tests\test-results")) {
    New-Item -Path "tests\test-results" -ItemType Directory -Force | Out-Null
}

@"
ExplorerLens Installation Test Results
=====================================
Date: $(Get-Date)
Test Duration: N/A

Passed:   $($testResults.Passed.Count)
Failed:   $($testResults.Failed.Count)
Warnings: $($testResults.Warnings.Count)
Success Rate: $successRate%

Passed Tests:
$($testResults.Passed | ForEach-Object { "  - $_" } | Out-String)

Failed Tests:
$($testResults.Failed | ForEach-Object { "  - $_" } | Out-String)

Warnings:
$($testResults.Warnings | ForEach-Object { "  - $_" } | Out-String)
"@ | Out-File -FilePath $reportPath -Encoding UTF8

Write-Host "Report saved to: $reportPath`n" -ForegroundColor Gray

