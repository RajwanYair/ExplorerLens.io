# Code Signing Infrastructure for DarkThumbs v6.0.0

## Overview

This document describes the code signing infrastructure for DarkThumbs releases, ensuring binary authenticity and trustworthiness.

## Components

### 1. Sign-Binaries.ps1

Automated code signing script supporting multiple certificate sources and verification modes.

### Modes of Operation

#### Production Mode

```powershell
# Using certificate from Windows Certificate Store
.\Sign-Binaries.ps1 -CertificateThumbprint 'YOUR_CERT_THUMBPRINT'

# Using PFX file
.\Sign-Binaries.ps1 -CertificatePath 'path\to\cert.pfx' -CertificatePassword 'password'
```

#### Test Mode (Development)

```powershell
# Generates self-signed certificate for testing
.\Sign-Binaries.ps1 -TestMode
```

#### Verify Mode

```powershell
# Check existing signatures without signing
.\Sign-Binaries.ps1 -VerifyOnly
```

## Certificate Requirements

### Production Certificates

For production releases, use a certificate from a trusted Certificate Authority (CA):

**Recommended CAs:**

- DigiCert Code Signing Certificate
- Sectigo (formerly Comodo) Code Signing
- GlobalSign Code Signing Certificate

**Certificate Type:** Code Signing Certificate (EV recommended for immediate trust)

**Requirements:**

- SHA-256 signing algorithm
- RSA 2048-bit or higher key length
- Valid for code signing purposes
- Timestamping enabled

### Certificate Acquisition

1. **Purchase from CA:**
   - DigiCert: <https://www.digicert.com/signing/code-signing-certificates>
   - Sectigo: <https://sectigo.com/ssl-certificates-tls/code-signing>
   - GlobalSign: <https://www.globalsign.com/en/code-signing-certificate>

2. **Validation Process:**
   - Organization validation (OV) or Extended Validation (EV)
   - Identity verification (business documents)
   - Processing time: 1-7 days

3. **Certificate Delivery:**
   - PFX/P12 file with private key
   - Or installed directly to Windows Certificate Store

### Test Certificates (Development Only)

For development and testing:

```powershell
# Script automatically creates self-signed certificate
.\Sign-Binaries.ps1 -TestMode
```

**⚠️ Warning:** Test certificates are NOT trusted by Windows by default and will show security warnings.

## Timestamping

Timestamping ensures signatures remain valid even after certificate expiration.

**Default Timestamp Server:** DigiCert  
**URL:** <http://timestamp.digicert.com>

**Alternative Servers:**

- Sectigo: <http://timestamp.sectigo.com>
- GlobalSign: <http://timestamp.globalsign.com>

**Algorithm:** SHA-256 (RFC3161)

## Build Integration

### Manual Signing

```powershell
# After building release binaries
cd release-scripts
.\Sign-Binaries.ps1 -CertificateThumbprint 'YOUR_THUMBPRINT'
```

### Automated CI/CD Integration

```yaml
# GitHub Actions example
- name: Sign Binaries
  run: |
    .\release-scripts\Sign-Binaries.ps1 `
      -CertificateThumbprint '${{ secrets.CERT_THUMBPRINT }}' `
      -TimestampServer 'http://timestamp.digicert.com'
  env:
    CERT_PASSWORD: ${{ secrets.CERT_PASSWORD }}
```

## Signed Files

The following binaries are signed:

1. **CBXShell.dll** - Main shell extension DLL
2. **CBXManager.exe** - Configuration manager application

## Verification

### Manual Verification

Right-click on signed file → Properties → Digital Signatures tab

### Script Verification

```powershell
.\Sign-Binaries.ps1 -VerifyOnly
```

### Expected Output (Valid Signature)

```
Verifying: CBXShell.dll
  ✅ Valid signature
Verifying: CBXManager.exe
  ✅ Valid signature
```

## Security Best Practices

### Certificate Protection

1. **Store private keys securely:**
   - Hardware Security Module (HSM) for EV certificates
   - Encrypted storage for standard certificates
   - Never commit certificates to source control

2. **Access Control:**
   - Limit access to signing certificates
   - Use separate signing machine/environment
   - Audit signing activities

3. **Certificate Renewal:**
   - Monitor expiration dates
   - Renew 30 days before expiration
   - Update build scripts with new thumbprints

### Signing Process Security

1. **Build Integrity:**
   - Sign only from clean, verified builds
   - Verify build outputs before signing
   - Maintain build reproducibility

2. **Timestamp All Signatures:**
   - Ensures validity after certificate expiration
   - Use reliable timestamp servers
   - Implement retry logic for network failures

3. **Verification:**
   - Always verify signatures after signing
   - Automated verification in CI/CD
   - Manual verification before release

## Troubleshooting

### Common Issues

#### "Certificate not found"

**Solution:**

```powershell
# List available certificates
Get-ChildItem Cert:\CurrentUser\My -CodeSigningCert
Get-ChildItem Cert:\LocalMachine\My -CodeSigningCert

# Use correct thumbprint
.\Sign-Binaries.ps1 -CertificateThumbprint 'CORRECT_THUMBPRINT'
```

#### "Invalid password"

**Solution:**

```powershell
# Ensure correct password for PFX file
.\Sign-Binaries.ps1 -CertificatePath 'cert.pfx' -CertificatePassword 'CORRECT_PASSWORD'
```

#### "Timestamp server unreachable"

**Solution:**

```powershell
# Use alternative timestamp server
.\Sign-Binaries.ps1 -CertificateThumbprint 'THUMBPRINT' `
  -TimestampServer 'http://timestamp.sectigo.com'
```

#### "signtool.exe not found"

**Solution:**
Install Windows SDK:
<https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/>

## Compliance

### Windows Logo Requirements

- Code signing required for kernel-mode drivers
- Recommended for user-mode applications
- Required for Windows Store distribution

### SmartScreen Integration

- EV Code Signing certificates gain immediate reputation
- Standard certificates require reputation building
- Consistent signing improves SmartScreen reputation

## Release Checklist

- [ ] Certificate is valid and not expiring soon
- [ ] Build completed successfully
- [ ] Binaries are in x64\Release directory
- [ ] Run signing script: `.\Sign-Binaries.ps1`
- [ ] Verify signatures: `.\Sign-Binaries.ps1 -VerifyOnly`
- [ ] All files show valid signatures
- [ ] Timestamp applied successfully
- [ ] Log file reviewed (no errors)

## Support

For signing issues:

1. Check signing log in `release-logs\signing-*.log`
2. Review troubleshooting section
3. Consult CA documentation
4. Contact DarkThumbs development team

---

**Document Version:** 1.0  
**Last Updated:** January 6, 2026  
**Part of:** Sprint 18 - v6.0.0 Release Engineering
