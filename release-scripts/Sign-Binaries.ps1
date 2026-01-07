# =============================================================================
# Sign-Binaries.ps1 - Code Signing Script for DarkThumbs Release
# =============================================================================
# Sprint 18 - Task 18.2: Code Signing Infrastructure
# Part of v6.0.0 Release Engineering
# =============================================================================

param(
    [Parameter(Mandatory = $false)]
    [string]$CertificateThumbprint,
    
    [Parameter(Mandatory = $false)]
    [string]$CertificatePath,
    
    [Parameter(Mandatory = $false)]
    [string]$CertificatePassword,
    
    [Parameter(Mandatory = $false)]
    [string]$TimestampServer = "http://timestamp.digicert.com",
    
    [switch]$TestMode,
    
    [switch]$VerifyOnly
)

$ErrorActionPreference = "Stop"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  DarkThumbs Code Signing v1.0" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Configuration
$projectRoot = Split-Path -Parent $PSScriptRoot
$binariesPath = Join-Path $projectRoot "x64\Release"
$signLog = Join-Path $projectRoot "release-logs\signing-$(Get-Date -Format 'yyyy-MM-dd_HHmmss').log"

# Files to sign
$filesToSign = @(
    "CBXShell.dll",
    "CBXManager.exe"
)

# Helper function for logging
function Write-Log {
    param([string]$Message, [string]$Level = "INFO")
    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    $logMessage = "[$timestamp] [$Level] $Message"
    Write-Host $logMessage
    if (-not $VerifyOnly) {
        $logMessage | Out-File -FilePath $signLog -Append -Encoding UTF8
    }
}

# Create log directory
$logDir = Split-Path -Parent $signLog
if (-not (Test-Path $logDir)) {
    New-Item -ItemType Directory -Path $logDir -Force | Out-Null
}

Write-Log "=== DarkThumbs Code Signing Started ===" "INFO"
Write-Log "Binary Path: $binariesPath" "INFO"
Write-Log "Test Mode: $TestMode" "INFO"

# Check if signtool.exe is available
$signTool = $null
$possiblePaths = @(
    "C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\signtool.exe",
    "C:\Program Files (x86)\Windows Kits\10\bin\x64\signtool.exe"
)

foreach ($path in $possiblePaths) {
    if (Test-Path $path) {
        $signTool = $path
        break
    }
}

if (-not $signTool) {
    # Try to find in PATH
    $signTool = (Get-Command signtool.exe -ErrorAction SilentlyContinue).Source
}

if (-not $signTool) {
    Write-Log "signtool.exe not found. Please install Windows SDK." "ERROR"
    Write-Host "❌ signtool.exe not found" -ForegroundColor Red
    Write-Host ""
    Write-Host "Install Windows SDK from:" -ForegroundColor Yellow
    Write-Host "https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/" -ForegroundColor Yellow
    exit 1
}

Write-Log "Found signtool: $signTool" "INFO"

# Verify mode - check existing signatures
if ($VerifyOnly) {
    Write-Host "[VERIFY MODE] Checking signatures..." -ForegroundColor Yellow
    Write-Host ""
    
    $allValid = $true
    foreach ($file in $filesToSign) {
        $filePath = Join-Path $binariesPath $file
        
        if (-not (Test-Path $filePath)) {
            Write-Host "⚠️  File not found: $file" -ForegroundColor Yellow
            continue
        }
        
        Write-Host "Verifying: $file" -ForegroundColor White
        
        $verifyResult = & $signTool verify /pa /v $filePath 2>&1
        
        if ($LASTEXITCODE -eq 0) {
            Write-Host "  ✅ Valid signature" -ForegroundColor Green
        } else {
            Write-Host "  ❌ No valid signature" -ForegroundColor Red
            $allValid = $false
        }
    }
    
    Write-Host ""
    if ($allValid) {
        Write-Host "========================================" -ForegroundColor Green
        Write-Host "  All binaries have valid signatures" -ForegroundColor Green
        Write-Host "========================================" -ForegroundColor Green
    } else {
        Write-Host "========================================" -ForegroundColor Red
        Write-Host "  Some binaries lack valid signatures" -ForegroundColor Red
        Write-Host "========================================" -ForegroundColor Red
    }
    
    exit 0
}

# Test mode - generate self-signed certificate
if ($TestMode) {
    Write-Host "[TEST MODE] Generating self-signed certificate..." -ForegroundColor Yellow
    Write-Host ""
    
    $certName = "CN=DarkThumbs Test Certificate"
    $cert = New-SelfSignedCertificate -DnsName "DarkThumbs" -Type CodeSigning -CertStoreLocation Cert:\CurrentUser\My
    $CertificateThumbprint = $cert.Thumbprint
    
    Write-Log "Created test certificate: $CertificateThumbprint" "INFO"
    Write-Host "  ✅ Test certificate created: $CertificateThumbprint" -ForegroundColor Green
    Write-Host ""
    Write-Host "⚠️  WARNING: This is a TEST certificate!" -ForegroundColor Yellow
    Write-Host "   It will NOT be trusted by Windows without additional configuration." -ForegroundColor Yellow
    Write-Host ""
}

# Validate certificate configuration
if (-not $CertificateThumbprint -and -not $CertificatePath) {
    Write-Host "❌ ERROR: No certificate specified" -ForegroundColor Red
    Write-Host ""
    Write-Host "Usage examples:" -ForegroundColor Yellow
    Write-Host "  # Using certificate from store:" -ForegroundColor Gray
    Write-Host "  .\Sign-Binaries.ps1 -CertificateThumbprint 'ABC123...'" -ForegroundColor Gray
    Write-Host ""
    Write-Host "  # Using PFX file:" -ForegroundColor Gray
    Write-Host "  .\Sign-Binaries.ps1 -CertificatePath 'cert.pfx' -CertificatePassword 'pass'" -ForegroundColor Gray
    Write-Host ""
    Write-Host "  # Test mode (self-signed):" -ForegroundColor Gray
    Write-Host "  .\Sign-Binaries.ps1 -TestMode" -ForegroundColor Gray
    Write-Host ""
    Write-Host "  # Verify existing signatures:" -ForegroundColor Gray
    Write-Host "  .\Sign-Binaries.ps1 -VerifyOnly" -ForegroundColor Gray
    exit 1
}

# Sign each binary
Write-Host "[SIGNING] Processing binaries..." -ForegroundColor Yellow
Write-Host ""

$signedCount = 0
$failedCount = 0

foreach ($file in $filesToSign) {
    $filePath = Join-Path $binariesPath $file
    
    if (-not (Test-Path $filePath)) {
        Write-Log "File not found: $file" "WARNING"
        Write-Host "⚠️  Skipping: $file (not found)" -ForegroundColor Yellow
        continue
    }
    
    Write-Host "Signing: $file" -ForegroundColor White
    Write-Log "Signing: $filePath" "INFO"
    
    # Build signtool command
    $signArgs = @("sign", "/v")
    
    if ($CertificateThumbprint) {
        $signArgs += "/sha1"
        $signArgs += $CertificateThumbprint
    } elseif ($CertificatePath) {
        $signArgs += "/f"
        $signArgs += $CertificatePath
        if ($CertificatePassword) {
            $signArgs += "/p"
            $signArgs += $CertificatePassword
        }
    }
    
    # Add timestamp server (unless test mode)
    if (-not $TestMode) {
        $signArgs += "/tr"
        $signArgs += $TimestampServer
        $signArgs += "/td"
        $signArgs += "SHA256"
    }
    
    $signArgs += "/fd"
    $signArgs += "SHA256"
    $signArgs += $filePath
    
    # Execute signing
    $signOutput = & $signTool @signArgs 2>&1
    
    if ($LASTEXITCODE -eq 0) {
        Write-Log "Successfully signed: $file" "SUCCESS"
        Write-Host "  ✅ Signed successfully" -ForegroundColor Green
        $signedCount++
    } else {
        Write-Log "Failed to sign: $file - $signOutput" "ERROR"
        Write-Host "  ❌ Signing failed" -ForegroundColor Red
        Write-Host "     Error: $signOutput" -ForegroundColor Red
        $failedCount++
    }
    
    Write-Host ""
}

# Summary
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Signing Summary" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Total files: $($filesToSign.Count)" -ForegroundColor White
Write-Host "Signed: $signedCount" -ForegroundColor Green
Write-Host "Failed: $failedCount" -ForegroundColor $(if ($failedCount -gt 0) { "Red" } else { "Green" })
Write-Host ""
Write-Host "Log file: $signLog" -ForegroundColor Gray
Write-Host ""

Write-Log "=== Signing Complete === Signed: $signedCount, Failed: $failedCount" "INFO"

if ($failedCount -gt 0) {
    exit 1
}

Write-Host "========================================" -ForegroundColor Green
Write-Host "  ✅ All binaries signed successfully" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
