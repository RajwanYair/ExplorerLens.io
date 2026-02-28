# Sign-Release.ps1
# Signs binaries and MSI installer with Authenticode certificate
# Date: February 17, 2026

[CmdletBinding()]
param(
    [Parameter(Mandatory=$false)]
    [string]$CertificatePath = "",
    
    [Parameter(Mandatory=$false)]
    [SecureString]$CertificatePassword = $null,
    
    [Parameter(Mandatory=$false)]
    [string]$TimestampServer = "http://timestamp.digicert.com",
    
    [Parameter()]
    [string]$BinaryDirectory = "$PSScriptRoot\..\ x64\Release",
    
    [Parameter()]
    [string]$MSIPath = "$PSScriptRoot\..\packaging\ExplorerLens-Setup.msi",
    
    [Parameter()]
    [switch]$SkipBinaries,
    
    [Parameter()]
    [switch]$SkipMSI,
    
    [Parameter()]
    [switch]$DryRun
)

$ErrorActionPreference = "Stop"

Write-Host "=== ExplorerLens Release Signing ===" -ForegroundColor Cyan
Write-Host ": Code Signing & Distribution"
Write-Host ""

# Certificate discovery
if (-not $CertificatePath) {
    Write-Host "Searching for code signing certificate..." -ForegroundColor Yellow
    
    # Check for certificate in Windows Certificate Store
    $cert = Get-ChildItem -Path Cert:\CurrentUser\My -CodeSigningCert | Select-Object -First 1
    
    if ($cert) {
        Write-Host "✓ Found certificate in Windows store: $($cert.Subject)" -ForegroundColor Green
        $useCertStore = $true
    } else {
        # Check common certificate locations
        $certPaths = @(
            "$env:USERPROFILE\Documents\Certificates\ExplorerLens.pfx",
            "$PSScriptRoot\..\certs\ExplorerLens.pfx",
            "C:\Certificates\ExplorerLens.pfx"
        )
        
        foreach ($path in $certPaths) {
            if (Test-Path $path) {
                $CertificatePath = $path
                Write-Host "✓ Found certificate file: $path" -ForegroundColor Green
                break
            }
        }
        
        if (-not $CertificatePath) {
            Write-Host "⚠ No certificate found. Signing will be skipped." -ForegroundColor Yellow
            Write-Host "  To enable signing:" -ForegroundColor Gray
            Write-Host "  1. Obtain EV code signing certificate from DigiCert/Sectigo" -ForegroundColor Gray
            Write-Host "  2. Install to Windows Certificate Store (preferred)" -ForegroundColor Gray
            Write-Host "  3. Or place PFX at: $env:USERPROFILE\Documents\Certificates\ExplorerLens.pfx" -ForegroundColor Gray
            Write-Host ""
            Write-Host "Continuing without signing (development/testing only)..." -ForegroundColor Yellow
            exit 0
        }
    }
}

# Find SignTool.exe
$signTool = $null
$sdkPaths = @(
    "${env:ProgramFiles(x86)}\Windows Kits\10\bin\*\x64\signtool.exe",
    "${env:ProgramFiles}\Windows Kits\10\bin\*\x64\signtool.exe"
)

foreach ($pattern in $sdkPaths) {
    $found = Get-ChildItem -Path $pattern -ErrorAction SilentlyContinue | Sort-Object -Descending | Select-Object -First 1
    if ($found) {
        $signTool = $found.FullName
        break
    }
}

if (-not $signTool) {
    Write-Host "ERROR: SignTool.exe not found" -ForegroundColor Red
    Write-Host "Install Windows SDK: https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/" -ForegroundColor Yellow
    exit 1
}

Write-Host "SignTool: $signTool" -ForegroundColor Cyan
Write-Host "Timestamp Server: $TimestampServer" -ForegroundColor Cyan
Write-Host ""

# Function to sign a file
function Sign-File {
    param(
        [string]$FilePath,
        [string]$Description = "ExplorerLens Thumbnail Provider"
    )
    
    if (-not (Test-Path $FilePath)) {
        Write-Host "  ⚠ File not found: $FilePath" -ForegroundColor Yellow
        return $false
    }
    
    $fileName = Split-Path $FilePath -Leaf
    Write-Host "Signing: $fileName" -ForegroundColor Cyan
    
    if ($DryRun) {
        Write-Host "  [DRY RUN] Would sign: $FilePath" -ForegroundColor Gray
        return $true
    }
    
    $signArgs = @(
        "sign"
        "/fd", "SHA256"                  # File digest algorithm
        "/tr", $TimestampServer          # RFC 3161 timestamp server
        "/td", "SHA256"                  # Timestamp digest algorithm
        "/d", $Description              # Description
    )
    
    if ($useCertStore) {
        # Sign using certificate from Windows store
        $signArgs += "/a"                # Automatically select best certificate
        $signArgs += "/n", $cert.Subject
    } else {
        # Sign using PFX file
        $signArgs += "/f", $CertificatePath
        
        if ($CertificatePassword) {
            $bstr = [System.Runtime.InteropServices.Marshal]::SecureStringToBSTR($CertificatePassword)
            $plainPassword = [System.Runtime.InteropServices.Marshal]::PtrToStringAuto($bstr)
            $signArgs += "/p", $plainPassword
            [System.Runtime.InteropServices.Marshal]::ZeroFreeBSTR($bstr)
        }
    }
    
    $signArgs += $FilePath
    
    try {
        $output = & $signTool @signArgs 2>&1
        
        if ($LASTEXITCODE -eq 0) {
            Write-Host "  ✓ Signed successfully" -ForegroundColor Green
            return $true
        } else {
            Write-Host "  ✗ Signing failed: $output" -ForegroundColor Red
            return $false
        }
    } catch {
        Write-Host "  ✗ Signing error: $_" -ForegroundColor Red
        return $false
    }
}

# Sign binaries
$signedCount = 0
$failedCount = 0

if (-not $SkipBinaries) {
    Write-Host "=== Signing Binaries ===" -ForegroundColor Yellow
    Write-Host "Directory: $BinaryDirectory" -ForegroundColor Gray
    Write-Host ""
    
    $binaries = @(
        "LENSShell.dll",
        "LENSManager.exe",
        "PluginHost.exe",
        "ExplorerLensEngine.dll"
    )
    
    foreach ($binary in $binaries) {
        $binaryPath = Join-Path $BinaryDirectory $binary
        if (Sign-File -FilePath $binaryPath -Description "ExplorerLens $($binary -replace '\.(dll|exe)$', '')") {
            $signedCount++
        } else {
            $failedCount++
        }
    }
    
    Write-Host ""
}

# Sign MSI installer
if (-not $SkipMSI) {
    Write-Host "=== Signing MSI Installer ===" -ForegroundColor Yellow
    
    if (Test-Path $MSIPath) {
        if (Sign-File -FilePath $MSIPath -Description "ExplorerLens Installer") {
            $signedCount++
        } else {
            $failedCount++
        }
    } else {
        Write-Host "⚠ MSI not found: $MSIPath" -ForegroundColor Yellow
        Write-Host "  Build MSI first: msbuild packaging\ExplorerLens.wixproj /p:Configuration=Release" -ForegroundColor Gray
    }
    
    Write-Host ""
}

# Summary
Write-Host "==================================" -ForegroundColor Cyan
Write-Host "      SIGNING SUMMARY             " -ForegroundColor Cyan
Write-Host "==================================" -ForegroundColor Cyan
Write-Host "Signed:          $signedCount" -ForegroundColor Green
Write-Host "Failed:          $failedCount" -ForegroundColor $(if ($failedCount -gt 0) { "Red" } else { "Green" })
Write-Host "==================================" -ForegroundColor Cyan
Write-Host ""

if ($failedCount -eq 0 -and $signedCount -gt 0) {
    Write-Host "✓ All files signed successfully!" -ForegroundColor Green
    Write-Host ""
    Write-Host "SmartScreen status:" -ForegroundColor Cyan
    Write-Host "  • First installs may still show warning (new certificate)" -ForegroundColor Gray
    Write-Host "  • Reputation builds over time with downloads" -ForegroundColor Gray
    Write-Host "  • EV certificates bypass SmartScreen immediately" -ForegroundColor Gray
    exit 0
} elseif ($signedCount -eq 0) {
    Write-Host "⚠ No files were signed" -ForegroundColor Yellow
    exit 1
} else {
    Write-Host "✗ Some files failed to sign" -ForegroundColor Red
    exit 1
}

