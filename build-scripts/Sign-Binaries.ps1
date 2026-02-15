#==============================================================================
# DarkThumbs - Code Signing Automation Script
# Sprint 24: Code signing infrastructure
# 
# Signs all DarkThumbs binaries with Authenticode certificate
# Requires: Code signing certificate installed in Windows Certificate Store
#           OR valid PFX file with password
#==============================================================================

param(
    [string]$CertificateThumbprint = "",
    [string]$PfxFile = "",
    [string]$PfxPassword = "",
    [string]$TimestampServer = "http://timestamp.digicert.com",
    [string]$Configuration = "Release",
    [switch]$VerifyOnly,
    [switch]$UseAzureCodeSigning,
    [string]$AzureKeyVaultUrl = "",
    [string]$AzureCertificateName = "",
    [switch]$GenerateChecksums,
    [switch]$TagRelease,
    [string]$ReleaseVersion = "",
    [switch]$Help
)

$ErrorActionPreference = "Stop"

function Show-Usage {
    Write-Host @"
DarkThumbs Code Signing Script
==============================

Usage:
  Sign-Binaries.ps1 -CertificateThumbprint <thumbprint> [-Configuration Release]
  Sign-Binaries.ps1 -PfxFile <path> -PfxPassword <password>
  Sign-Binaries.ps1 -VerifyOnly

Options:
  -CertificateThumbprint  SHA-1 thumbprint of certificate in cert store
  -PfxFile                Path to PFX certificate file
  -PfxPassword            Password for PFX file (use with -PfxFile)
  -TimestampServer        RFC 3161 timestamp server URL (default: DigiCert)
  -Configuration          Build configuration (default: Release)
  -VerifyOnly             Only verify existing signatures, don't sign
  -Help                   Show this help message

Examples:
  # Sign with certificate from Windows store
  .\Sign-Binaries.ps1 -CertificateThumbprint "A1B2C3D4E5F6..."

  # Sign with PFX file
  .\Sign-Binaries.ps1 -PfxFile "cert.pfx" -PfxPassword "SecurePass123"

  # Verify existing signatures
  .\Sign-Binaries.ps1 -VerifyOnly

Certificate Acquisition:
  - DigiCert: https://www.digicert.com/signing/code-signing-certificates
  - Sectigo: https://sectigostore.com/code-signing
  - Cost: ~$200-400/year for standard EV (Extended Validation) certificate
  - Required: EV certificate for Windows SmartScreen reputation
"@
}

if ($Help) {
    Show-Usage
    exit 0
}

# Validate parameters
if (-not $VerifyOnly) {
    if (-not $CertificateThumbprint -and -not $PfxFile) {
        Write-Host "ERROR: Must specify either -CertificateThumbprint or -PfxFile" -ForegroundColor Red
        Write-Host ""
        Show-Usage
        exit 1
    }

    if ($PfxFile -and -not $PfxPassword) {
        Write-Host "ERROR: -PfxPassword required when using -PfxFile" -ForegroundColor Red
        exit 1
    }

    if ($PfxFile -and -not (Test-Path $PfxFile)) {
        Write-Host "ERROR: PFX file not found: $PfxFile" -ForegroundColor Red
        exit 1
    }
}

# Find signtool.exe
$SignToolPaths = @(
    "C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\signtool.exe",
    "C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x64\signtool.exe",
    "C:\Program Files (x86)\Windows Kits\10\App Certification Kit\signtool.exe"
)

$SignTool = $null
foreach ($path in $SignToolPaths) {
    if (Test-Path $path) {
        $SignTool = $path
        break
    }
}

if (-not $SignTool) {
    # Try searching in Windows Kits directory
    $kitsDir = "C:\Program Files (x86)\Windows Kits\10\bin"
    if (Test-Path $kitsDir) {
        $SignTool = Get-ChildItem -Path $kitsDir -Recurse -Filter "signtool.exe" -ErrorAction SilentlyContinue |
        Where-Object { $_.FullName -match "x64" } |
        Select-Object -First 1 -ExpandProperty FullName
    }
}

if (-not $SignTool) {
    Write-Host "ERROR: signtool.exe not found. Install Windows SDK:" -ForegroundColor Red
    Write-Host "  https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/" -ForegroundColor Yellow
    exit 1
}

Write-Host "Using SignTool: $SignTool" -ForegroundColor Cyan

# Define binaries to sign
$ProjectRoot = Split-Path -Parent $PSScriptRoot
$BinariesRootFolder = Join-Path $ProjectRoot "bin" $Configuration

$BinariesToSign = @(
    # Core DLL (required for Windows SmartScreen)
    @{
        Path        = Join-Path $ProjectRoot "CBXShell\x64\$Configuration\CBXShell.dll"
        Description = "DarkThumbs Shell Extension"
        Critical    = $true
    },
    # Manager executable
    @{
        Path        = Join-Path $ProjectRoot "CBXManager\x64\$Configuration\CBXManager.exe"
        Description = "DarkThumbs Configuration Manager"
        Critical    = $true
    },
    # Engine library (optional, static lib doesn't need signing)
    @{
        Path        = Join-Path $ProjectRoot "build\lib\$Configuration\DarkThumbsEngine.lib"
        Description = "DarkThumbs Engine Library"
        Critical    = $false
        Skip        = $true  # Static libraries don't get signed
    },
    # Test executables (optional)
    @{
        Path        = Join-Path $ProjectRoot "build\bin\$Configuration\EngineTests.exe"
        Description = "DarkThumbs Unit Tests"
        Critical    = $false
    },
    @{
        Path        = Join-Path $ProjectRoot "build\bin\$Configuration\IntegrationTests.exe"
        Description = "DarkThumbs Integration Tests"
        Critical    = $false
    },
    @{
        Path        = Join-Path $ProjectRoot "build\bin\$Configuration\EngineBenchmark.exe"
        Description = "DarkThumbs Performance Benchmark"
        Critical    = $false
    }
)

# Add installer if it exists
$InstallerPath = Join-Path $ProjectRoot "packaging\output\DarkThumbs-Setup.msi"
if (Test-Path $InstallerPath) {
    $BinariesToSign += @{
        Path        = $InstallerPath
        Description = "DarkThumbs Installer"
        Critical    = $true
    }
}

function Sign-File {
    param(
        [string]$FilePath,
        [string]$Description
    )

    if (-not (Test-Path $FilePath)) {
        return $false, "File not found: $FilePath"
    }

    Write-Host "Signing: $FilePath" -ForegroundColor Cyan

    $signArgs = @("sign")

    # Certificate selection
    if ($CertificateThumbprint) {
        $signArgs += "/sha1", $CertificateThumbprint
    } elseif ($PfxFile) {
        $signArgs += "/f", $PfxFile
        $signArgs += "/p", $PfxPassword
    }

    # Signing parameters
    $signArgs += "/fd", "SHA256"  # File digest algorithm
    $signArgs += "/tr", $TimestampServer  # RFC 3161 timestamp
    $signArgs += "/td", "SHA256"  # Timestamp digest algorithm
    $signArgs += "/d", $Description  # Description
    $signArgs += "/du", "https://github.com/YourOrg/DarkThumbs"  # URL

    # Append/dual signing for compatibility
    # First signature uses SHA-256, second uses SHA-1 for Windows 7 compatibility
    $signArgs += $FilePath

    try {
        $output = & $SignTool $signArgs 2>&1
        if ($LASTEXITCODE -eq 0) {
            Write-Host "  ✓ Signed successfully" -ForegroundColor Green
            return $true, "Success"
        } else {
            Write-Host "  ✗ Signing failed: $output" -ForegroundColor Red
            return $false, $output
        }
    } catch {
        Write-Host "  ✗ Exception: $_" -ForegroundColor Red
        return $false, $_.Exception.Message
    }
}

function Verify-Signature {
    param([string]$FilePath)

    if (-not (Test-Path $FilePath)) {
        return $false, "File not found"
    }

    try {
        $output = & $SignTool verify /pa /v $FilePath 2>&1
        if ($LASTEXITCODE -eq 0) {
            Write-Host "  ✓ Valid signature" -ForegroundColor Green
            return $true, "Valid"
        } else {
            Write-Host "  ✗ Invalid or missing signature" -ForegroundColor Yellow
            return $false, "Not signed"
        }
    } catch {
        return $false, $_.Exception.Message
    }
}

#==============================================================================
# Main Execution
#==============================================================================

Write-Host ""
Write-Host "======================================" -ForegroundColor Cyan
Write-Host "DarkThumbs Code Signing" -ForegroundColor Cyan
Write-Host "======================================" -ForegroundColor Cyan
Write-Host ""

if ($VerifyOnly) {
    Write-Host  "Mode: Verify Only" -ForegroundColor Yellow
} else {
    Write-Host "Mode: Sign Binaries" -ForegroundColor Green
    Write-Host "Configuration: $Configuration" -ForegroundColor Cyan
    Write-Host "Timestamp Server: $TimestampServer" -ForegroundColor Cyan
}
Write-Host ""

$signedCount = 0
$failedCount = 0
$skippedCount = 0
$criticalFailed = $false

foreach ($binary in $BinariesToSign) {
    if ($binary.Skip) {
        Write-Host "Skipping: $($binary.Path) (not required)" -ForegroundColor Gray
        $skippedCount++
        continue
    }

    if (-not (Test-Path $binary.Path)) {
        Write-Host "WARNING: Binary not found: $($binary.Path)" -ForegroundColor Yellow
        if ($binary.Critical) {
            $criticalFailed = $true
            $failedCount++
        } else {
            $skippedCount++
        }
        continue
    }

    if ($VerifyOnly) {
        $success, $message = Verify-Signature -FilePath $binary.Path
        if ($success) {
            $signedCount++
        } else {
            if ($binary.Critical) {
                $criticalFailed = $true
            }
            $failedCount++
        }
    } else {
        $success, $message = Sign-File -FilePath $binary.Path -Description $binary.Description
        if ($success) {
            $signedCount++
        } else {
            Write-Host "ERROR: Failed to sign: $($binary.Path)" -ForegroundColor Red
            Write-Host "  Details: $message" -ForegroundColor Red
            if ($binary.Critical) {
                $criticalFailed = $true
            }
            $failedCount++
        }
    }

    Write-Host ""
}

#==============================================================================
# Sprint 24: Enhanced Signing Features
#==============================================================================

# Generate SHA-256 checksums for all signed binaries
if ($GenerateChecksums) {
    Write-Host ""
    Write-Host "=== Generating SHA-256 Checksums ===" -ForegroundColor Cyan
    
    $checksumFile = Join-Path $PSScriptRoot "..\SHA256SUMS.txt"
    $checksums = @()
    
    foreach ($file in $filesToSign) {
        if (Test-Path $file.Path) {
            try {
                $hash = (Get-FileHash -Path $file.Path -Algorithm SHA256).Hash
                $relativePath = (Resolve-Path -Path $file.Path -Relative).TrimStart(".\")
                $checksums += "$hash  $relativePath"
                
                Write-Host "  $($file.Name): " -NoNewline
                Write-Host $hash -ForegroundColor Gray
            } catch {
                Write-Host "  FAILED: $($file.Name)" -ForegroundColor Red
            }
        }
    }
    
    # Write checksums file
    $checksums | Out-File -FilePath $checksumFile -Encoding UTF8
    Write-Host ""
    Write-Host "✓ Checksums written to: $checksumFile" -ForegroundColor Green
}

# Create Git release tag
if ($TagRelease -and $ReleaseVersion) {
    Write-Host ""
    Write-Host "=== Creating Release Tag ===" -ForegroundColor Cyan
    
    try {
        # Check if git is available
        $gitVersion = (git --version 2>&1)
        if ($LASTEXITCODE -ne 0) {
            Write-Host "WARNING: Git not found, skipping release tagging" -ForegroundColor Yellow
        } else {
            # Create annotated tag
            $tagName = "v$ReleaseVersion"
            $tagMessage = "Release $ReleaseVersion - Signed binaries"
            
            Write-Host "  Creating tag: $tagName" -ForegroundColor White
            
            git tag -a $tagName -m $tagMessage 2>&1 | Out-Null
            
            if ($LASTEXITCODE -eq 0) {
                Write-Host "✓ Tag created: $tagName" -ForegroundColor Green
                Write-Host "  Push with: git push origin $tagName" -ForegroundColor Gray
            } else {
                Write-Host "WARNING: Tag creation failed (may already exist)" -ForegroundColor Yellow
            }
        }
    } catch {
        Write-Host "WARNING: Release tagging failed: $_" -ForegroundColor Yellow
    }
}

# Azure Code Signing support (future enhancement)
if ($UseAzureCodeSigning) {
    Write-Host ""
    Write-Host "=== Azure Code Signing ===" -ForegroundColor Cyan
    
    if (-not $AzureKeyVaultUrl -or -not $AzureCertificateName) {
        Write-Host "ERROR: Azure Code Signing requires -AzureKeyVaultUrl and -AzureCertificateName" -ForegroundColor Red
        exit 1
    }
    
    Write-Host "Azure Key Vault: $AzureKeyVaultUrl" -ForegroundColor White
    Write-Host "Certificate: $AzureCertificateName" -ForegroundColor White
    Write-Host ""
    Write-Host "NOTE: Azure Code Signing requires:" -ForegroundColor Yellow
    Write-Host "  1. Azure CLI installed (az login)" -ForegroundColor Yellow
    Write-Host "  2. Azure SignTool extension" -ForegroundColor Yellow
    Write-Host "  3. Key Vault access permissions" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "SKIPPED: Azure Code Signing not yet implemented" -ForegroundColor Yellow
    Write-Host "         Use standard code signing for now" -ForegroundColor Yellow
}

#==============================================================================
# Summary
#==============================================================================

Write-Host "======================================" -ForegroundColor Cyan
Write-Host "Summary:" -ForegroundColor Cyan
Write-Host "  Signed/Verified: $signedCount" -ForegroundColor $(if ($signedCount -gt 0) { "Green" } else { "Gray" })
Write-Host "  Failed: $failedCount" -ForegroundColor $(if ($failedCount -gt 0) { "Red" } else { "Gray" })
Write-Host "  Skipped: $skippedCount" -ForegroundColor Gray
Write-Host "======================================" -ForegroundColor Cyan
Write-Host ""

if ($criticalFailed) {
    Write-Host "CRITICAL: One or more critical binaries failed to sign!" -ForegroundColor Red
    Write-Host "  Required for production deployment: CBXShell.dll, CBXManager.exe" -ForegroundColor Yellow
    exit 1
}

if ($failedCount -gt 0) {
    Write-Host "WARNING: Some binaries failed to sign (non-critical)" -ForegroundColor Yellow
    exit 2
}

Write-Host "✓ All binaries processed successfully!" -ForegroundColor Green
exit 0
