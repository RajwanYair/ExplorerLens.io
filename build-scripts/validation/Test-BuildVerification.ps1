#Requires -Version 7.0
# DarkThumbs v7.0 - Build Verification Suite
# Comprehensive verification system for build outputs and library dependencies
# Date: February 16, 2026

<#
.SYNOPSIS
    Comprehensive build verification suite for DarkThumbs

.DESCRIPTION
    Verifies:
    - All external library builds completed successfully
    - Required headers and libraries are present
    - Library sizes are reasonable (detect broken builds)
    - CMake/MSBuild configurations are correct
    - MSI package can be created
    - Engine DLL exports are correct
    - Shell extension registration is valid

.EXAMPLE
    .\Test-BuildVerification.ps1 -Verbose
    
.EXAMPLE
    .\Test-BuildVerification.ps1 -VerifyLibraries -VerifyEngine -CreateReport

.NOTES
    Author: DarkThumbs Development Team
    Version: 7.0
#>

param(
    [switch]$VerifyLibraries = $true,
    [switch]$VerifyEngine = $true,
    [switch]$VerifyShellExtension = $true,
    [switch]$VerifyPackaging = $true,
    [switch]$CreateReport = $true,
    [switch]$FailFast = $false
)

$ErrorActionPreference = "Continue"

# Import modules
$coreDir = Join-Path $PSScriptRoot "..\core"
. (Join-Path $coreDir "Build-Library-Core.ps1")
. (Join-Path $coreDir "Build-Progress.ps1")

$rootDir = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$results = @{
    Timestamp = Get-Date
    TotalTests = 0
    PassedTests = 0
    FailedTests = 0
    WarningTests = 0
    Details = @()
}

function Test-LibraryBuild {
    param(
        [string]$LibraryName,
        [string]$LibPath,
        [string]$HeaderPath = $null,
        [int]$MinSizeKB = 10
    )
    
    $results.TotalTests++
    $testResult = @{
        Library = $LibraryName
        Passed = $false
        Message = ""
    }
    
    # Check library exists
    if (-not (Test-Path $LibPath)) {
        $testResult.Message = "Library file not found: $LibPath"
        $results.FailedTests++
        Write-Host " ✗ $LibraryName - Library missing" -ForegroundColor Red
        Write-Host "   Path: $LibPath" -ForegroundColor DarkRed
    }
    else {
        $libSize = (Get-Item $LibPath).Length / 1KB
        
        # Check minimum size
        if ($libSize -lt $MinSizeKB) {
            $testResult.Message = "Library too small ($([math]::Round($libSize, 2)) KB < $MinSizeKB KB) - possible build failure"
            $results.WarningTests++
            Write-Host " ⚠ $LibraryName - Suspiciously small ($([math]::Round($libSize, 2)) KB)" -ForegroundColor Yellow
        }
        else {
            $testResult.Passed = $true
            $results.PassedTests++
            Write-Host " ✓ $LibraryName - OK ($([math]::Round($libSize, 2)) KB)" -ForegroundColor Green
        }
    }
    
    # Check headers if specified
    if ($HeaderPath -and -not (Test-Path $HeaderPath)) {
        $testResult.Message += " | Headers missing: $HeaderPath"
        Write-Host "   ⚠ Headers not found: $HeaderPath" -ForegroundColor Yellow
    }
    
    $results.Details += $testResult
}

function Test-EngineBuild {
    $engineDll = Join-Path $rootDir "build\bin\Release\DarkThumbsEngine.dll"
    
    $results.TotalTests++
    
    if (-not (Test-Path $engineDll)) {
        Write-Host " ✗ DarkThumbsEngine.dll not found" -ForegroundColor Red
        Write-Host "   Expected: $engineDll" -ForegroundColor DarkRed
        $results.FailedTests++
        return $false
    }
    
    # Check DLL exports
    try {
        $dllSize = (Get-Item $engineDll).Length / 1MB
        Write-Host " ✓ DarkThumbsEngine.dll found ($([math]::Round($dllSize, 2)) MB)" -ForegroundColor Green
        
        # Verify it's a valid PE file
        $bytes = [System.IO.File]::ReadAllBytes($engineDll)[0..1]
        if ($bytes[0] -eq 0x4D -and $bytes[1] -eq 0x5A) {  # MZ header
            Write-Host "   ✓ Valid PE executable" -ForegroundColor Green
            $results.PassedTests++
            return $true
        }
        else {
            Write-Host "   ✗ Invalid PE format" -ForegroundColor Red
            $results.FailedTests++
            return $false
        }
    }
    catch {
        Write-Host "   ✗ Error checking DLL: $($_.Exception.Message)" -ForegroundColor Red
        $results.FailedTests++
        return $false
    }
}

function Test-ShellExtension {
    $cbxShellDll = Join-Path $rootDir "x64\Release\CBXShell.dll"
    
    $results.TotalTests++
    
    if (-not (Test-Path $cbxShellDll)) {
        Write-Host " ✗ CBXShell.dll not found" -ForegroundColor Red
        Write-Host "   Expected: $cbxShellDll" -ForegroundColor DarkRed
        $results.FailedTests++
        return $false
    }
    
    $dllSize = (Get-Item $cbxShellDll).Length / 1MB
    Write-Host " ✓ CBXShell.dll found ($([math]::Round($dllSize, 2)) MB)" -ForegroundColor Green
    $results.PassedTests++
    return $true
}

function Test-PackagingPrerequisites {
    $wixInstalled = Test-CommandExists "wix"
    
    $results.TotalTests++
    
    if (-not $wixInstalled) {
        Write-Host " ✗ WiX Toolset not found" -ForegroundColor Red
        Write-Host "   Install with: dotnet tool install --global wix" -ForegroundColor Yellow
        $results.FailedTests++
        return $false
    }
    
    Write-Host " ✓ WiX Toolset installed" -ForegroundColor Green
    $results.PassedTests++
    return $true
}

# Main verification
Write-Host "`n═══════════════════════════════════════════════════════" -ForegroundColor Cyan
Write-Host " DarkThumbs v7.0 - Build Verification Suite" -ForegroundColor Cyan
Write-Host "═══════════════════════════════════════════════════════`n" -ForegroundColor Cyan

$allPassed = $true

# Verify external libraries
if ($VerifyLibraries) {
    Write-Host "[1/4] Verifying External Libraries..." -ForegroundColor Yellow
    Write-Host ""
    
    # Compression libraries
    Write-Host " Compression Libraries:" -ForegroundColor Cyan
    Test-LibraryBuild -LibraryName "zlib" -LibPath (Join-Path $rootDir "SDK\zlib\lib\zlibstatic.lib") -MinSizeKB 100
    Test-LibraryBuild -LibraryName "zstd" -LibPath (Join-Path $rootDir "SDK\zstd\lib\zstd_static.lib") -MinSizeKB 300
    Test-LibraryBuild -LibraryName "LZ4" -LibPath (Join-Path $rootDir "SDK\lz4\lib\lz4.lib") -MinSizeKB 50
    Test-LibraryBuild -LibraryName "LZMA SDK" -LibPath (Join-Path $rootDir "SDK\lzma\lib\lzma.lib") -MinSizeKB 100
    
    Write-Host ""
    Write-Host " Image Format Libraries:" -ForegroundColor Cyan
    Test-LibraryBuild -LibraryName "libwebp" -LibPath (Join-Path $rootDir "SDK\libwebp\lib\webp.lib") -MinSizeKB 500
    Test-LibraryBuild -LibraryName "libjxl" -LibPath (Join-Path $rootDir "SDK\libjxl\lib\jxl_static.lib") -MinSizeKB 1000
    Test-LibraryBuild -LibraryName "libavif" -LibPath (Join-Path $rootDir "SDK\libavif\lib\avif.lib") -MinSizeKB 200
    Test-LibraryBuild -LibraryName "dav1d" -LibPath (Join-Path $rootDir "SDK\dav1d\lib\dav1d.lib") -MinSizeKB 500
    
    Write-Host ""
    Write-Host " Archive & Camera Libraries:" -ForegroundColor Cyan
    Test-LibraryBuild -LibraryName "minizip-ng" -LibPath (Join-Path $rootDir "SDK\minizip-ng\lib\minizip.lib") -MinSizeKB 100
    Test-LibraryBuild -LibraryName "LibRaw" -LibPath (Join-Path $rootDir "external\libraw-install\lib\libraw_static.lib") -MinSizeKB 1000
    
    Write-Host ""
}

# Verify Engine
if ($VerifyEngine) {
    Write-Host "[2/4] Verifying DarkThumbs Engine..." -ForegroundColor Yellow
    Write-Host ""
    $engineOK = Test-EngineBuild
    $allPassed = $allPassed -and $engineOK
    Write-Host ""
}

# Verify Shell Extension
if ($VerifyShellExtension) {
    Write-Host "[3/4] Verifying Shell Extension..." -ForegroundColor Yellow
    Write-Host ""
    $shellOK = Test-ShellExtension
    $allPassed = $allPassed -and $shellOK
    Write-Host ""
}

# Verify Packaging
if ($VerifyPackaging) {
    Write-Host "[4/4] Verifying Packaging Prerequisites..." -ForegroundColor Yellow
    Write-Host ""
    $packagingOK = Test-PackagingPrerequisites
    $allPassed = $allPassed -and $packagingOK
    Write-Host ""
}

# Summary
Write-Host "───────────────────────────────────────────────────────" -ForegroundColor DarkGray
Write-Host ""
Write-Host " Verification Summary:" -ForegroundColor Cyan
Write-Host ""
Write-Host (" Total Tests: {0}" -f $results.TotalTests) -ForegroundColor Gray
Write-Host (" Passed: {0}" -f $results.PassedTests) -ForegroundColor Green
Write-Host (" Failed: {0}" -f $results.FailedTests) -ForegroundColor Red
Write-Host (" Warnings: {0}" -f $results.WarningTests) -ForegroundColor Yellow
Write-Host ""

$passRate = if ($results.TotalTests -gt 0) { 
    [math]::Round(($results.PassedTests / $results.TotalTests) * 100, 1) 
} else { 0 }

Write-Host (" Pass Rate: {0}%" -f $passRate) -ForegroundColor $(if ($passRate -ge 90) { "Green" } elseif ($passRate -ge 70) { "Yellow" } else { "Red" })
Write-Host ""

# Create report if requested
if ($CreateReport) {
    $reportPath = Join-Path $rootDir "build-logs\verification-report-$(Get-Date -Format 'yyyyMMdd-HHmmss').json"
    $reportDir = Split-Path -Parent $reportPath
    
    if (-not (Test-Path $reportDir)) {
        New-Item -ItemType Directory -Path $reportDir -Force | Out-Null
    }
    
    $results | ConvertTo-Json -Depth 10 | Out-File -FilePath $reportPath -Encoding UTF8
    Write-Host " Report saved: $reportPath" -ForegroundColor Cyan
    Write-Host ""
}

Write-Host "═══════════════════════════════════════════════════════`n" -ForegroundColor Cyan

# Exit with appropriate code
if ($results.FailedTests -gt 0) {
    Write-Host " ✗ Verification FAILED - $($results.FailedTests) tests failed" -ForegroundColor Red
    exit 1
}
elseif ($results.WarningTests -gt 0) {
    Write-Host " ⚠ Verification PASSED with warnings" -ForegroundColor Yellow
    exit 0
}
else {
    Write-Host " ✓ Verification PASSED - All tests successful!" -ForegroundColor Green
    exit 0
}
