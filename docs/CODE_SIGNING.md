# DarkThumbs Code Signing Guide
**Sprint 24: Code Signing Infrastructure**  
**Date:** February 15, 2026

---

## Table of Contents
1. [Why Code Signing?](#why-code-signing)
2. [Certificate Acquisition](#certificate-acquisition)
3. [Certificate Installation](#certificate-installation)
4. [Signing Workflow](#signing-workflow)
5. [Verification](#verification)
6. [Troubleshooting](#troubleshooting)
7. [CI/CD Integration](#cicd-integration)

---

## Why Code Signing?

### **Requirements for Windows Shell Extensions**
Code signing is **mandatory** for production deployment of DarkThumbs:

1. **Windows SmartScreen Filter**
   - Unsigned binaries trigger "Windows protected your PC" warnings
   - Users must click "More info" → "Run anyway" (poor UX)
   - SmartScreen reputation builds over time with signed binaries

2. **Enterprise Deployment**
   - Many corporate environments block unsigned executables
   - Group Policy may require Authenticode signatures
   - Software Restriction Policies (SRP) enforcement

3. **User Trust & Professionalism**
   - Digital signatures prove publisher identity
   - Prevents tampering (file integrity verification)
   - Shows commitment to security best practices

4. **Windows 11 Requirements**
   - Future Windows versions may enforce mandatory code signing
   - Microsoft Store requires signed submissions

### **Files Requiring Signatures**
**Critical (Required for deployment):**
- `CBXShell.dll` - COM shell extension (loaded by explorer.exe)
- `CBXManager.exe` - Configuration UI
- `DarkThumbs-Setup.msi` - Installer package

**Optional (Recommended):**
- `EngineTests.exe`, `IntegrationTests.exe`, `EngineBenchmark.exe` - Test executables
- External DLLs (if distributing separately)

---

## Certificate Acquisition

### **Certificate Authorities (Recommended)**

#### **1. DigiCert (Industry Leader)**
- **Website:** https://www.digicert.com/signing/code-signing-certificates
- **Cost:** $469/year (Standard), $629/year (EV)
- **Validation Time:** 1-3 days (Standard), 3-7 days (EV)
- **Benefits:**
  - Instant SmartScreen reputation (EV only)
  - Trusted by all Windows versions
  - Excellent customer support
  - Timestamping service included

#### **2. Sectigo (Budget-Friendly)**
- **Website:** https://sectigostore.com/code-signing
- **Cost:** $199/year (Standard), $349/year (EV)
- **Validation Time:** 1-5 days
- **Benefits:**
  - Most affordable option
  - Good reputation, slightly longer SmartScreen buildup

#### **3. GlobalSign**
- **Website:** https://www.globalsign.com/en/code-signing-certificate
- **Cost:** $349/year (Standard), $599/year (EV)
- **Validation Time:** 1-3 days

### **Certificate Types**

| Feature | Standard (OV) | Extended Validation (EV) |
|---------|---------------|--------------------------|
| **Cost** | $199-469/year | $349-629/year |
| **Validation** | Organization verification | Enhanced identity verification |
| **SmartScreen** | Builds over ~6 months | Instant reputation |
| **Hardware Token** | No | Yes (USB token required) |
| **Recommended** | Small projects | Production/Commercial use |

**Recommendation:** Start with **EV certificate** if budget allows (instant SmartScreen reputation).

### **Ordering Process**

1. **Select Certificate Authority**
   - Choose DigiCert (best) or Sectigo (budget-friendly)

2. **Purchase Certificate**
   - Select "Extended Validation (EV)" code signing certificate
   - Windows Authenticode compatible (SHA-256)
   - 1-3 year duration (multi-year saves cost)

3. **Identity Verification**
   - **Company Documents Required:**
     - Business registration (Articles of Incorporation)
     - Tax ID / EIN number
     - Physical address proof (utility bill, bank statement)
     - Phone number listed in public directories
   - **Personal Verification:**
     - Government-issued ID (driver's license, passport)
     - Email verification
     - Phone call confirmation

4. **Receive Certificate**
   - Standard: Emailed as PFX file (.p12)
   - EV: Shipped as USB hardware token (SafeNet eToken)

**Timeline:** 3-7 business days for EV validation.

---

## Certificate Installation

### **Option A: Hardware Token (EV Certificate)**

1. **Insert USB Token**
   - Plug in SafeNet eToken 5110 (or similar)
   - Install manufacturer drivers if prompted

2. **Set PIN**
   - Default PIN usually: `1234` or `12345678`
   - Change to secure PIN (8+ characters)

3. **Verify Installation**
   ```powershell
   certutil -scinfo
   ```
   - Should show certificate details
   - Note the SHA-1 thumbprint

### **Option B: PFX File (Standard Certificate)**

1. **Import to Certificate Store**
   ```powershell
   # Import PFX (will prompt for password)
   certutil -user -importPFX "C:\Path\To\Certificate.pfx"
   ```

2. **Alternative: Manual Import**
   - Double-click `.pfx` file in Explorer
   - Enter password
   - Store location: **Current User → Personal**
   - Mark key as exportable: **No** (security best practice)

3. **Verify Installation**
   ```powershell
   # List certificates in Personal store
   Get-ChildItem Cert:\CurrentUser\My | Format-List Subject, Thumbprint, NotAfter
   ```

4. **Get Thumbprint**
   - Open `certmgr.msc` (Certificate Manager)
   - Navigate to **Personal → Certificates**
   - Double-click your code signing certificate
   - Go to **Details** tab → **Thumbprint**
   - Copy the SHA-1 hash (e.g., `A1B2C3D4E5F6...`)

---

## Signing Workflow

### **Using Sign-Binaries.ps1 Script**

#### **With Hardware Token (EV)**
```powershell
cd build-scripts
.\Sign-Binaries.ps1 -CertificateThumbprint "A1B2C3D4E5F6789ABCDEF..." -Configuration Release
```

#### **With PFX File**
```powershell
.\Sign-Binaries.ps1 -PfxFile "C:\Certs\DarkThumbs.pfx" -PfxPassword "SecurePassword123"
```

#### **Verify Existing Signatures**
```powershell
.\Sign-Binaries.ps1 -VerifyOnly
```

### **Manual Signing with SignTool**

#### **Common SignTool Locations**
```powershell
# Windows SDK 10
C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x64\signtool.exe

# App Certification Kit
C:\Program Files (x86)\Windows Kits\10\App Certification Kit\signtool.exe
```

#### **Sign with Certificate Store**
```powershell
signtool sign `
  /sha1 A1B2C3D4E5F6... `
  /fd SHA256 `
  /tr http://timestamp.digicert.com `
  /td SHA256 `
  /d "DarkThumbs Shell Extension" `
  /du "https://github.com/YourOrg/DarkThumbs" `
  CBXShell.dll
```

#### **Sign with PFX File**
```powershell
signtool sign `
  /f "C:\Certs\DarkThumbs.pfx" `
  /p "Password" `
  /fd SHA256 `
  /tr http://timestamp.digicert.com `
  /td SHA256 `
  /d "DarkThumbs Configuration Manager" `
  CBXManager.exe
```

#### **Dual Signing (Windows 7 Compatibility)**
```powershell
# First signature: SHA-256 (Windows 8+)
signtool sign /sha1 ... /fd SHA256 /tr ... CBXShell.dll

# Second signature: SHA-1 (Windows 7)
signtool sign /sha1 ... /fd SHA1 /t http://timestamp.digicert.com /as CBXShell.dll
```
- `/as` - Append signature (allows dual signing)
- Use if targeting Windows 7 users (now deprecated)

### **Timestamp Servers**

**Why Timestamp?**
- Signatures remain valid after certificate expires
- Proves when file was signed
- **Always use timestamping** (free service)

**Recommended Servers:**
```
http://timestamp.digicert.com              (DigiCert RFC 3161)
http://timestamp.sectigo.com               (Sectigo RFC 3161)
http://timestamp.globalsign.com/tsa/r6advanced1  (GlobalSign)
```

**Fallback (Legacy SHA-1 only):**
```
http://timestamp.comodoca.com              (Deprecated, SHA-1 only)
```

---

## Verification

### **PowerShell Verification**
```powershell
# Check signature details
Get-AuthenticodeSignature CBXShell.dll | Format-List *

# Expected output:
#   Status         : Valid
#   SignerCertificate : CN=Your Company Name, O=Your Company, ...
#   TimeStamperCertificate : CN=DigiCert Timestamp 2024, ...
```

### **Windows Explorer**
1. Right-click signed file → **Properties**
2. Go to **Digital Signatures** tab
3. Should show certificate details
4. Click **Details** → **View Certificate** to inspect

### **SignTool Verification**
```powershell
# Verify signature and certificate chain
signtool verify /pa /v CBXShell.dll
```
- `/pa` - Use default authentication policy
- `/v` - Verbose output

**Expected Output:**
```
Verifying: CBXShell.dll
Successfully verified: CBXShell.dll

Number of files successfully Verified: 1
Number of warnings: 0
Number of errors: 0
```

### **Automated Verification**
```powershell
# Use Sign-Binaries.ps1 in verify mode
.\Sign-Binaries.ps1 -VerifyOnly
```

---

## Troubleshooting

### **Error: Certificate not found**
**Cause:** Thumbprint incorrect or certificate not in store  
**Solution:**
```powershell
# List all personal certificates
Get-ChildItem Cert:\CurrentUser\My | Format-List Subject, Thumbprint

# Re-import PFX if needed
certutil -user -importPFX "Certificate.pfx"
```

### **Error: SignTool not found**
**Cause:** Windows SDK not installed  
**Solution:**
```powershell
# Download Windows SDK
winget install Microsoft.WindowsSDK

# Or manual download: https://developer.microsoft.com/windows/downloads/windows-sdk/
```

### **Error: The file is not signed**
**Cause:** Signing failed silently**Solution:**
```powershell
# Re-run with verbose logging
signtool sign [...args...] /v /debug CBXShell.dll

# Check specific error code
$LASTEXITCODE
```

### **Error: Timestamp server unavailable**
**Cause:** Network issue or server downtime  
**Solution:**
- Try alternative timestamp server (see list above)
- Check firewall/proxy settings (allow HTTP/HTTPS)
- Retry after a few minutes

### **Warning: Private key not found**
**Cause:** Certificate imported without private key  
**Solution:**
```powershell
# Re-import with -user flag
certutil -user -importPFX "Certificate.pfx"

# Mark key as exportable if re-importing
certutil -importPFX -user -p "Password" "Certificate.pfx"
```

### **SmartScreen Still Shows Warning**
**Cause:** New certificate needs reputation buildup  
**Solution:**
- **EV Certificates:** Instant reputation (should work immediately)
- **Standard Certificates:** Requires 6+ months + significant download volume
- **Workaround:** Submit binaries to Microsoft for manual review
  - https://www.microsoft.com/en-us/wdsi/filesubmission

---

## CI/CD Integration

### **GitHub Actions**

```yaml
name: Build and Sign
on: [push, pull_request]

jobs:
  build-and-sign:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v4
      
      - name: Build DarkThumbs
        run: |
          cmake -S . -B build -G "Visual Studio 18 2026" -A x64
          cmake --build build --config Release

      - name: Import Code Signing Certificate
        run: |
          # Decode base64-encoded PFX from secrets
          $pfxBytes = [System.Convert]::FromBase64String("${{ secrets.CODE_SIGN_PFX }}")
          [IO.File]::WriteAllBytes("cert.pfx", $pfxBytes)
        
      - name: Sign Binaries
        env:
          PFX_PASSWORD: ${{ secrets.CODE_SIGN_PASSWORD }}
        run: |
          .\build-scripts\Sign-Binaries.ps1 `
            -PfxFile "cert.pfx" `
            -PfxPassword $env:PFX_PASSWORD `
            -Configuration Release

      - name: Verify Signatures
        run: |
          .\build-scripts\Sign-Binaries.ps1 -VerifyOnly

      - name: Upload Signed Binaries
        uses: actions/upload-artifact@v4
        with:
          name: signed-binaries
          path: |
            CBXShell/x64/Release/CBXShell.dll
            CBXManager/x64/Release/CBXManager.exe
```

**Secrets Configuration:**
1. Go to GitHub repo → **Settings** → **Secrets and variables** → **Actions**
2. Add secrets:
   - `CODE_SIGN_PFX` - Base64-encoded PFX file
   - `CODE_SIGN_PASSWORD` - PFX password

**Encode PFX to Base64:**
```powershell
[Convert]::ToBase64String([IO.File]::ReadAllBytes("Certificate.pfx")) | Set-Clipboard
```

### **Azure DevOps Pipeline**

```yaml
trigger:
  - main

pool:
  vmImage: 'windows-latest'

steps:
- task: UseDotNet@2
  inputs:
    packageType: 'sdk'
    version: '8.x'

- task: CMake@1
  inputs:
    workingDirectory: 'build'
    cmakeArgs: '-G "Visual Studio 18 2026" -A x64 ..'

- task: VSBuild@1
  inputs:
    solution: 'build/DarkThumbs.sln'
    configuration: 'Release'

- task: DownloadSecureFile@1
  name: codeSignCert
  inputs:
    secureFile: 'DarkThumbs.pfx'

- powershell: |
    .\build-scripts\Sign-Binaries.ps1 `
      -PfxFile $(codeSignCert.secureFilePath) `
      -PfxPassword $(CODE_SIGN_PASSWORD) `
      -Configuration Release
  displayName: 'Sign Binaries'

- task: PublishBuildArtifacts@1
  inputs:
    pathToPublish: 'CBXShell/x64/Release'
    artifactName: 'signed-binaries'
```

---

## Security Best Practices

1. **Never Commit Certificates to Git**
   - Add `*.pfx` and `*.p12` to `.gitignore`
   - Use CI/CD secrets for automation

2. **Protect Private Keys**
   - Store PFX files encrypted
   - Use hardware tokens when possible (EV)
   - Limit access to signing infrastructure

3. **Rotate Certificates**
   - Renew before expiration (set reminder 30 days prior)
   - Re-sign all binaries with new certificate

4. **Audit Signed Files**
   - Track which files were signed when
   - Maintain signature verification logs

5. **Use Strong Passwords**
   - PFX passwords: 16+ characters, mixed case, symbols
   - Hardware token PINs: 8+ characters

6. **Timestamp Everything**
   - Signatures remain valid after cert expiry
   - Prevents re-signing old releases

---

## Cost-Benefit Analysis

### **Annual Costs**
| Item | Cost (USD) |
|------|-----------|
| EV Code Signing Certificate | $349-629 |
| Hardware Token (included with EV) | $0 |
| **Total** | **$349-629/year** |

### **Benefits**
- **Immediate:** No SmartScreen warnings (EV)
- **Trust:** Professional appearance, verified publisher
- **Security:** File integrity verification
- **Compliance:** Enterprise deployment requirements met
- **Future-Proof:** Windows 11+ compatibility

### **ROI**
- Prevents lost users due to security warnings
- Reduces support tickets ("How do I install?")
- Enables enterprise/corporate adoption
- Professional credibility

---

## Resources

- **SignTool Documentation:** https://learn.microsoft.com/en-us/windows/win32/seccrypto/signtool
- **Authenticode Overview:** https://learn.microsoft.com/en-us/windows-hardware/drivers/install/authenticode
- **SmartScreen FAQ:** https://learn.microsoft.com/en-us/windows/security/threat-protection/microsoft-defender-smartscreen/
- **Windows SDK Download:** https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/

---

**Last Updated:** February 15, 2026  
**Sprint:** 24 - Code Signing Infrastructure  
**Status:** Complete (scripts + documentation)
