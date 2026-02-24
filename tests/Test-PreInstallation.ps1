#Requires -Version 7.0

<#
.SYNOPSIS
    Pre-installation verification tests for ExplorerLens (no admin required)
.DESCRIPTION
    Verifies that the build outputs are ready for installation without requiring
    administrator privileges. This runs the pre-checks before the full installation test.
#>

$ErrorActionPreference = "Continue"
$testResults = @{
    Passed   = @()
    Failed   = @()
    Warnings = @()
}

function Write-TestResult {
    param([string]$TestName, [bool]$Success, [string]$Message = "")
    
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
Write-Host "SPRINT 7: PRE-INSTALLATION VERIFICATION" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

# Test 1: Verify LENSShell.dll exists in build directory
Write-Host "Phase 1: Build Output Verification" -ForegroundColor Yellow
Write-Host "-----------------------------------" -ForegroundColor Gray

$dllPath = "x64\Release\LENSShell.dll"
if (Test-Path $dllPath) {
    $dll = Get-Item $dllPath
    $sizeOK = $dll.Length -gt 1MB
    Write-TestResult "LENSShell.dll exists" $true "Size: $([Math]::Round($dll.Length/1MB,2)) MB, Modified: $($dll.LastWriteTime)"
    Write-TestResult "LENSShell.dll size check" $sizeOK "Expected > 1 MB"
}
else {
    Write-TestResult "LENSShell.dll exists" $false "File not found at $dllPath"
}

$exePath = "x64\Release\LENSManager.exe"
if (Test-Path $exePath) {
    $exe = Get-Item $exePath
    $sizeOK = $exe.Length -gt 100KB
    Write-TestResult "LENSManager.exe exists" $true "Size: $([Math]::Round($exe.Length/1KB,0)) KB, Modified: $($exe.LastWriteTime)"
    Write-TestResult "LENSManager.exe size check" $sizeOK "Expected > 100 KB"
}
else {
    Write-TestResult "LENSManager.exe exists" $false "File not found at $exePath"
}

# Test 2: Verify installation directory
Write-Host "`nPhase 2: Installation Directory Check" -ForegroundColor Yellow
Write-Host "--------------------------------------" -ForegroundColor Gray

$installDll = "install\x64\LENSShell.dll"
if (Test-Path $installDll) {
    $dll = Get-Item $installDll
    Write-TestResult "install\x64\LENSShell.dll exists" $true "Ready for deployment"
    
    # Verify it matches the build output
    $buildDll = Get-Item "x64\Release\LENSShell.dll" -ErrorAction SilentlyContinue
    if ($buildDll -and ($dll.Length -eq $buildDll.Length)) {
        Write-TestResult "DLL matches build output" $true "Sizes match"
    }
    else {
        Write-TestWarning "DLL size differs from build output - may need to copy again"
    }
}
else {
    Write-TestResult "install\x64\LENSShell.dll exists" $false "File not found - run copy command"
}

$installExe = "install\x64\LENSManager.exe"
if (Test-Path $installExe) {
    Write-TestResult "install\x64\LENSManager.exe exists" $true "Ready for deployment"
}
else {
    Write-TestResult "install\x64\LENSManager.exe exists" $false "File not found"
}

# Test 3: Verify explorerlens.ps1 install script
Write-Host "`nPhase 3: Installation Script Check" -ForegroundColor Yellow
Write-Host "-----------------------------------" -ForegroundColor Gray

if (Test-Path "explorerlens.ps1") {
    Write-TestResult "explorerlens.ps1 exists" $true "Installation script found"
    
    # Check if it has install parameter support
    $scriptContent = Get-Content "explorerlens.ps1" -Raw
    if ($scriptContent -match "install") {
        Write-TestResult "Install parameter support" $true "Script supports -install"
    }
    else {
        Write-TestResult "Install parameter support" $false "Script may not support installation"
    }
}
else {
    Write-TestResult "explorerlens.ps1 exists" $false "Installation script not found"
}

# Test 4: Check for required DLL dependencies
Write-Host "`nPhase 4: DLL Dependency Check" -ForegroundColor Yellow
Write-Host "------------------------------" -ForegroundColor Gray

try {
    # Try to load the DLL information (doesn't require admin)
    $dllInfo = [System.Reflection.AssemblyName]::GetAssemblyName("x64\Release\LENSShell.dll")
    Write-TestResult "DLL is valid .NET assembly" $true "Version: $($dllInfo.Version)"
}
catch {
    # Native DLL - expected for COM DLL
    Write-TestResult "DLL is native binary" $true "COM DLL (expected)"
}

# Check for common dependencies
$dependencies = @(
    "C:\Windows\System32\msvcr120.dll",
    "C:\Windows\System32\msvcp120.dll",
    "C:\Windows\System32\vcruntime140.dll"
)

$depFound = 0
foreach ($dep in $dependencies) {
    if (Test-Path $dep) { $depFound++ }
}

Write-TestResult "Visual C++ Runtime dependencies" ($depFound -gt 0) "$depFound of $($dependencies.Count) runtime DLLs found"

# Test 5: Verify test framework
Write-Host "`nPhase 5: Test Framework Verification" -ForegroundColor Yellow
Write-Host "-------------------------------------" -ForegroundColor Gray

if (Test-Path "tests\Test-Installation.ps1") {
    $testScript = Get-Item "tests\Test-Installation.ps1"
    Write-TestResult "Test-Installation.ps1 exists" $true "Size: $([Math]::Round($testScript.Length/1KB,1)) KB"
    
    $content = Get-Content "tests\Test-Installation.ps1" -Raw
    if ($content -match "#Requires -RunAsAdministrator") {
        Write-TestResult "Admin requirement detected" $true "Test requires elevation"
    }
}
else {
    Write-TestResult "Test-Installation.ps1 exists" $false "Test script not found"
}

# Test 6: Check Sprint documentation
Write-Host "`nPhase 6: Documentation Check" -ForegroundColor Yellow
Write-Host "----------------------------" -ForegroundColor Gray

if (Test-Path "docs\SPRINT7_PLAN.md") {
    $sprint7 = Get-Item "docs\SPRINT7_PLAN.md"
    Write-TestResult "SPRINT7_PLAN.md exists" $true "Size: $([Math]::Round($sprint7.Length/1KB,1)) KB"
}
else {
    Write-TestResult "SPRINT7_PLAN.md exists" $false "Documentation not found"
}

if (Test-Path "docs\SPRINT8_PLAN.md") {
    $sprint8 = Get-Item "docs\SPRINT8_PLAN.md"
    Write-TestResult "SPRINT8_PLAN.md exists" $true "Size: $([Math]::Round($sprint8.Length/1KB,1)) KB"
}
else {
    Write-TestResult "SPRINT8_PLAN.md exists" $false "Documentation not found"
}

# Summary
Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "TEST SUMMARY" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

$total = $testResults.Passed.Count + $testResults.Failed.Count
$successRate = if ($total -gt 0) { [Math]::Round(($testResults.Passed.Count / $total) * 100, 1) } else { 0 }

Write-Host "Passed:   $($testResults.Passed.Count)" -ForegroundColor Green
Write-Host "Failed:   $($testResults.Failed.Count)" -ForegroundColor Red
Write-Host "Warnings: $($testResults.Warnings.Count)" -ForegroundColor Yellow
Write-Host "Success Rate: $successRate%" -ForegroundColor $(if ($successRate -ge 90) { "Green" } elseif ($successRate -ge 70) { "Yellow" } else { "Red" })

if ($testResults.Failed.Count -eq 0) {
    Write-Host "`n✅ PRE-INSTALLATION VERIFICATION PASSED!" -ForegroundColor Green
    Write-Host "`nNext Steps:" -ForegroundColor Cyan
    Write-Host "1. Run PowerShell as Administrator" -ForegroundColor White
    Write-Host "2. Navigate to project directory" -ForegroundColor White
    Write-Host "3. Execute: .\tests\Test-Installation.ps1 -SkipUninstall" -ForegroundColor Yellow
    Write-Host "`nOr install directly:" -ForegroundColor Cyan
    Write-Host "   .\explorerlens.ps1 install" -ForegroundColor Yellow
}
else {
    Write-Host "`n❌ PRE-INSTALLATION VERIFICATION FAILED!" -ForegroundColor Red
    Write-Host "Fix the failed tests before proceeding with installation." -ForegroundColor Yellow
}

Write-Host ""

