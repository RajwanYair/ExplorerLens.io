# DarkThumbs v7.0.0 - Installation Package Documentation
# Complete Guide to Building and Deploying Installers

## Overview

DarkThumbs v7.0.0 provides multiple installer formats to support different deployment scenarios:

- **MSI Installer** (WiX Toolset) - Enterprise deployment, GPO support, standard Windows installer
- **Inno Setup Installer** - User-friendly setup wizard, compact size, portable mode
- **Portable ZIP** - No installation required, USB-portable, isolated deployment
- **MSIX Package** - Microsoft Store deployment, modern Windows app platform

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [vcpkg Setup](#vcpkg-setup)
3. [Building the Project](#building-the-project)
4. [Creating MSI Installer (WiX)](#creating-msi-installer-wix)
5. [Creating Inno Setup Installer](#creating-inno-setup-installer)
6. [Creating Portable ZIP](#creating-portable-zip)
7. [Code Signing](#code-signing)
8. [Testing Installers](#testing-installers)
9. [Distribution Checklist](#distribution-checklist)

---

## Prerequisites

### Required Tools

| Tool | Version | Purpose | Download |
|------|---------|---------|----------|
| **Visual Studio Build Tools** | 2022 (v143) | MSVC compiler | [Download](https://visualstudio.microsoft.com/downloads/) |
| **CMake** | 3.20+ | Build configuration | [Download](https://cmake.org/download/) |
| **WiX Toolset** | 4.0.5+ or 5.0+ | MSI installer creation | [Download](https://wixtoolset.org/releases/) |
| **Inno Setup** | 6.3+ | Setup wizard creation | [Download](https://jrsoftware.org/isinfo.php) |
| **vcpkg** | Latest | Package management (optional) | [Install Guide](#vcpkg-setup) |
| **SignTool** | Windows SDK | Code signing | Included with VS |
| **.NET SDK** | 8.0+ | WiX 5.x requirement | [Download](https://dotnet.microsoft.com/) |

### Verify Tool Installation

```powershell
# Run verification script
.\build-scripts\Find-All-Tools.ps1

# Expected output:
# ✓ CMake
# ✓ MSBuild.exe
# ✓ Ninja
# ✓ vcvarsall.bat
# ✓ Visual Studio Path
```

---

## vcpkg Setup

### Automatic Installation

vcpkg will be automatically downloaded when needed, but you can pre-install it:

```powershell
# Run vcpkg setup script
.\build-scripts\Setup-Vcpkg.ps1

# Or manually install
cd $env:USERPROFILE
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# Add to PATH (optional)
[System.Environment]::SetEnvironmentVariable('VCPKG_ROOT', "$env:USERPROFILE\vcpkg", 'User')
```

### vcpkg Integration with DarkThumbs

The project includes `vcpkg.json` manifest with all dependencies:

```json
{
  "name": "darkthumbs",
  "version": "7.0.0",
  "dependencies": [
    "zlib", "zstd", "lz4", "bzip2",
    "libwebp", "libavif", "libjpeg-turbo",
    "libpng", "giflib", "tiff",
    "libheif", "libraw"
  ]
}
```

**Note:** DarkThumbs currently builds most dependencies from source for maximum control. vcpkg is optional but recommended for future updates.

---

## Building the Project

### Quick Build (Standard)

```powershell
# Build Release configuration (default)
.\scripts\build.ps1 -Configuration Release

# Or use MSBuild directly
msbuild CBXShell.sln /p:Configuration=Release /p:Platform=x64 /m
```

### Build with vcpkg (Optional)

```powershell
# Configure with vcpkg toolchain
cmake -B build `
  -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" `
  -DCMAKE_BUILD_TYPE=Release `
  -G Ninja

# Build
cmake --build build --config Release -j 8
```

### Verify Build Output

```powershell
# Check build artifacts
Get-ChildItem -Recurse -Include *.dll,*.exe,*.lib | 
  Where-Object { $_.DirectoryName -like "*Release*" } |
  Select-Object Name, Length, LastWriteTime
```

**Expected artifacts:**
- `x64\Release\CBXShell.dll` (~3 MB) - COM shell extension
- `x64\Release\CBXManager.exe` (~2 MB) - Configuration utility
- `build\lib\Release\DarkThumbsEngine.lib` (~130 MB) - Core engine

---

## Creating MSI Installer (WiX)

### Prerequisites

1. **Install WiX Toolset:**
   ```powershell
   # Using winget
   winget install WiXToolset.WiX
   
   # Or download from https://wixtoolset.org/releases/
   ```

2. **Install WiX Extensions:**
   ```powershell
   # WiX 4.x/5.x extensions
   dotnet tool install --global wix
   ```

### Build MSI Package

```powershell
# Navigate to packaging directory
cd packaging

# Run MSI build script
.\Build-Installer.ps1 -Configuration Release -Version "7.0.0"

# Or build manually
wix build DarkThumbs.wxs `
  -out output\DarkThumbs-Setup-7.0.0.msi `
  -define "BuildDir=.." `
  -define "Version=7.0.0" `
  -arch x64 `
  -ext WixToolset.UI.wixext `
  -ext WixToolset.Util.wixext
```

### MSI Features

The MSI installer includes:

- ✅ **Core Components:** CBXShell.dll, DarkThumbsEngine.lib
- ✅ **Manager Application:** CBXManager.exe configuration utility
- ✅ **External Libraries:** libwebp, libavif, libjxl, dav1d
- ✅ **Registry Entries:** 200+ file extension registrations
- ✅ **Start Menu Shortcuts:** Configuration Manager, GPU Validator
- ✅ **COM Registration:** Automatic shell extension registration
- ⚙️ **Optional:** Debug symbols (PDB files) - Level 2 feature

### MSI Command-Line Options

```powershell
# Silent install
msiexec /i DarkThumbs-Setup-7.0.0.msi /quiet /norestart /l*v install.log

# Install with logging
msiexec /i DarkThumbs-Setup-7.0.0.msi /l*v install.log

# Uninstall
msiexec /x DarkThumbs-Setup-7.0.0.msi /quiet /norestart

# Repair
msiexec /fa DarkThumbs-Setup-7.0.0.msi
```

---

## Creating Inno Setup Installer

### Prerequisites

1. **Install Inno Setup:**
   ```powershell
   winget install JRSoftware.InnoSetup
   ```

2. **Verify Installation:**
   ```powershell
   & "C:\Program Files (x86)\Inno Setup 6\ISCC.exe" /?
   ```

### Build Inno Setup Installer

```powershell
# Navigate to Inno Setup directory
cd packaging\inno

# Compile installer
& "C:\Program Files (x86)\Inno Setup 6\ISCC.exe" DarkThumbs-Installer.iss

# Output: ..\release-packages\DarkThumbs-v7.0.0-x64-Setup.exe
```

### Inno Setup Features

- ✅ **User-Friendly:** Modern wizard interface
- ✅ **Compact:** ~50 MB installer size (vs ~60 MB for MSI)
- ✅ **Explorer Restart:** Automatic Explorer.exe restart during install/uninstall
- ✅ **Version Check:** Ensures Windows 10 build 19041+
- ✅ **COM Registration:** Automatic regsvr32 handling
- ✅ **Uninstaller:** Clean removal with registry cleanup

### Inno Setup Command-Line

```powershell
# Silent install
.\DarkThumbs-v7.0.0-x64-Setup.exe /VERYSILENT /NORESTART /LOG=install.log

# Silent uninstall
& "$env:ProgramFiles\DarkThumbs\unins000.exe" /VERYSILENT /NORESTART
```

---

## Creating Portable ZIP

### Build Portable Package

```powershell
# Run portable package script
.\packaging\Build-Portable-Package.ps1 -Version "7.0.0"

# Manual creation
$PortableFiles = @(
    "x64\Release\CBXShell.dll",
    "x64\Release\CBXManager.exe",
    "build\lib\Release\DarkThumbsEngine.lib",
    "external\image-libs\libwebp-1.5.0-build\libwebp.dll",
    "external\image-libs\libavif-1.3.0\build\Release\avif.dll",
    "LICENSE",
    "README.md",
    "docs\*"
)

Compress-Archive -Path $PortableFiles `
  -DestinationPath "DarkThumbs-v7.0.0-Portable-x64.zip" `
  -CompressionLevel Optimal
```

### Portable Package Contents

```
DarkThumbs-v7.0.0-Portable-x64.zip (45 MB)
├── CBXShell.dll               # COM shell extension
├── CBXManager.exe             # Configuration utility
├── DarkThumbsEngine.lib       # Core engine
├── libwebp.dll                # WebP decoder
├── avif.dll                   # AVIF decoder
├── jxl.dll                    # JPEG XL decoder
├── LICENSE                    # MIT license
├── README.md                  # Quick start guide
├── docs\                      # Documentation
└── install-portable.ps1       # Registration script
```

### Portable Installation

```powershell
# Extract and register manually
Expand-Archive DarkThumbs-v7.0.0-Portable-x64.zip -DestinationPath C:\Apps\DarkThumbs

# Register COM DLL
cd C:\Apps\DarkThumbs
regsvr32 /s CBXShell.dll

# Unregister before moving/deleting
regsvr32 /s /u CBXShell.dll
```

---

## Code Signing

### Prerequisites

- **Code Signing Certificate:** EV or OV certificate from DigiCert, Sectigo, etc.
- **SignTool.exe:** Installed with Windows SDK
- **Timestamp Server:** For long-term signature validity

### Sign Binaries

```powershell
# Sign DLL and EXE files
$SignToolPath = "C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\signtool.exe"
$CertThumbprint = "1234567890ABCDEF1234567890ABCDEF12345678"
$TimestampUrl = "http://timestamp.digicert.com"

# Sign shell extension
& $SignToolPath sign /sha1 $CertThumbprint /fd SHA256 /t $TimestampUrl `
  "x64\Release\CBXShell.dll"

# Sign manager application
& $SignToolPath sign /sha1 $CertThumbprint /fd SHA256 /t $TimestampUrl `
  "x64\Release\CBXManager.exe"

# Sign MSI installer
& $SignToolPath sign /sha1 $CertThumbprint /fd SHA256 /t $TimestampUrl `
  "packaging\output\DarkThumbs-Setup-7.0.0.msi"

# Sign Inno Setup installer
& $SignToolPath sign /sha1 $CertThumbprint /fd SHA256 /t $TimestampUrl `
  "packaging\release-packages\DarkThumbs-v7.0.0-x64-Setup.exe"
```

### Verify Signatures

```powershell
# Verify signature
& $SignToolPath verify /pa /v "x64\Release\CBXShell.dll"

# Check certificate details
Get-AuthenticodeSignature "x64\Release\CBXShell.dll" | 
  Select-Object Status, SignerCertificate, TimeStamperCertificate
```

---

## Testing Installers

### Pre-Release Testing Checklist

#### Test Environment Setup

```powershell
# Create test VMs
# - Windows 10 22H2 (Clean install)
# - Windows 11 24H2 (Clean install)
# - Windows 11 23H2 (Upgrade scenario)
```

#### MSI Installer Tests

```powershell
# Test 1: Clean Install
msiexec /i DarkThumbs-Setup-7.0.0.msi /l*v install.log

# Verify:
# ✓ Files copied to C:\Program Files\DarkThumbs
# ✓ CBXShell.dll registered (check HKCR\CLSID)
# ✓ Start menu shortcuts created
# ✓ CBXManager launches successfully
# ✓ Thumbnails appear in Explorer for test files

# Test 2: Upgrade Install
# (Install v6.2.0 first, then v7.0.0)

# Test 3: Uninstall
msiexec /x DarkThumbs-Setup-7.0.0.msi /l*v uninstall.log

# Verify:
# ✓ All files removed
# ✓ Registry entries cleaned
# ✓ No leftover processes
```

#### Inno Setup Installer Tests

```powershell
# Test 1: GUI Install
.\DarkThumbs-v7.0.0-x64-Setup.exe

# Test 2: Silent Install
.\DarkThumbs-v7.0.0-x64-Setup.exe /VERYSILENT /NORESTART /LOG=install.log

# Test 3: Silent Uninstall
& "$env:ProgramFiles\DarkThumbs\unins000.exe" /VERYSILENT /NORESTART
```

#### Portable Package Tests

```powershell
# Test 1: USB Drive Deployment
# Copy ZIP to USB drive, extract, register

# Test 2: Non-Admin User
# Test registration with limited privileges

# Test 3: Multiple Instances
# Install portable versions in different directories
```

### Automated Test Script

```powershell
# Run comprehensive installer tests
.\packaging\Test-Installers.ps1 -Version "7.0.0" -Verbose

# Expected output:
# ✓ MSI installer size: 55-60 MB
# ✓ Inno Setup installer size: 48-52 MB
# ✓ Portable ZIP size: 43-47 MB
# ✓ All signatures valid
# ✓ Installation succeeds on all OS versions
# ✓ Uninstallation leaves no artifacts
```

---

## Distribution Checklist

### Pre-Release Validation

- [ ] **All builds succeed:** Release x64 configuration, 0 errors, 0 warnings
- [ ] **Unit tests pass:** 152/152 tests pass, 0 failures
- [ ] **Integration tests pass:** Format support, thumbnail generation, Explorer integration
- [ ] **Performance benchmarks:** 28ms avg decode time, 92% cache hit rate
- [ ] **Memory leak check:** No leaks detected with 10K file stress test
- [ ] **GPU acceleration:** Verified on NVIDIA and AMD hardware
- [ ] **Code signed:** All binaries and installers signed with valid certificate
- [ ] **Malware scan:** Clean scan from Windows Defender, VirusTotal

### Installer Validation

- [ ] **MSI installer:**
  - [ ] Builds without errors
  - [ ] Size: 55-60 MB
  - [ ] Code signed
  - [ ] Clean install succeeds
  - [ ] Upgrade from v6.2.0 succeeds
  - [ ] Uninstall leaves no artifacts
  - [ ] SHA256 checksum generated

- [ ] **Inno Setup installer:**
  - [ ] Compiles without errors
  - [ ] Size: 48-52 MB
  - [ ] Code signed
  - [ ] GUI install completes
  - [ ] Silent install completes
  - [ ] Uninstaller removes all files
  - [ ] SHA256 checksum generated

- [ ] **Portable ZIP:**
  - [ ] Contains all required files
  - [ ] Size: 43-47 MB
  - [ ] README.md with instructions
  - [ ] install-portable.ps1 script included
  - [ ] SHA256 checksum generated

### Documentation Validation

- [ ] **README.md:** Updated to v7.0.0, features documented
- [ ] **CHANGELOG.md:** v7.0.0 entry with new features, bug fixes
- [ ] **USER_GUIDE.md:** Installation instructions current
- [ ] **DEVELOPER_GUIDE.md:** Build instructions accurate
- [ ] **FORMAT_SUPPORT_MATRIX_V7.md:** All 200+ formats documented
- [ ] **RELEASE_CHECKLIST_V7.0.md:** All items completed

### Distribution Channels

- [ ] **GitHub Release:**
  - [ ] Create v7.0.0 release tag
  - [ ] Upload MSI installer
  - [ ] Upload Inno Setup installer
  - [ ] Upload Portable ZIP
  - [ ] Upload SDK package (optional)
  - [ ] Upload source archive
  - [ ] Generate release notes from CHANGELOG.md

- [ ] **Website/Download Page:**
  - [ ] Update download links
  - [ ] Update version badges
  - [ ] Update screenshots
  - [ ] Update system requirements

- [ ] **Social Media/Announcements:**
  - [ ] Prepare release announcement
  - [ ] List key features (200+ formats, GPU acceleration, 28ms decode)
  - [ ] Include download links and documentation

---

## Release Package Structure

### Final Release Artifacts

```
release-v7.0.0/
├── DarkThumbs-Setup-7.0.0.msi                    # WiX MSI installer (58 MB)
├── DarkThumbs-Setup-7.0.0.msi.sha256            # Checksum
├── DarkThumbs-v7.0.0-x64-Setup.exe              # Inno Setup installer (50 MB)
├── DarkThumbs-v7.0.0-x64-Setup.exe.sha256       # Checksum
├── DarkThumbs-v7.0.0-Portable-x64.zip           # Portable package (45 MB)
├── DarkThumbs-v7.0.0-Portable-x64.zip.sha256    # Checksum
├── DarkThumbs-SDK-v7.0.0.zip                    # Development SDK (200 MB)
├── DarkThumbs-SDK-v7.0.0.zip.sha256             # Checksum
├── DarkThumbs-v7.0.0-Source.zip                 # Source code archive
├── DarkThumbs-v7.0.0-Source.zip.sha256          # Checksum
├── RELEASE_NOTES.md                              # Release notes
└── CHECKSUMS.txt                                 # All checksums
```

### Generate Checksums

```powershell
# Generate SHA256 checksums for all release files
Get-ChildItem release-v7.0.0\*.msi,*.exe,*.zip | ForEach-Object {
    $hash = (Get-FileHash $_.FullName -Algorithm SHA256).Hash
    "$hash  $($_.Name)" | Out-File "$($_.FullName).sha256" -Encoding ASCII
}

# Consolidate all checksums
Get-Content release-v7.0.0\*.sha256 | Out-File release-v7.0.0\CHECKSUMS.txt
```

---

## Troubleshooting

### Common Issues

#### WiX Build Fails

```
Error: WiX Toolset not found in PATH
```

**Solution:**
```powershell
# Install WiX
winget install WiXToolset.WiX

# Add to PATH
$env:PATH += ";C:\Program Files\WiX Toolset v5.0\bin"
```

#### Inno Setup Compilation Error

```
Error: Cannot find iscc.exe
```

**Solution:**
```powershell
# Locate ISCC.exe
$isccPath = Get-ChildItem "C:\Program Files (x86)\Inno Setup*" -Recurse -Filter iscc.exe | 
  Select-Object -First 1 -ExpandProperty FullName

# Set alias
Set-Alias iscc $isccPath
```

#### Code Signing Fails

```
Error: SignTool error: No certificates were found that met all the given criteria
```

**Solution:**
```powershell
# List available certificates
$certs = Get-ChildItem Cert:\CurrentUser\My -CodeSigningCert
$certs | Select-Object Thumbprint, Subject, NotAfter

# Use correct thumbprint
$thumbprint = ($certs | Select-Object -First 1).Thumbprint
```

#### vcpkg Clone Timeout

```
Error: Failed to connect to github.com port 443
```

**Solution:**
```powershell
# Use proxy or alternative mirror
$env:HTTP_PROXY = "http://proxy.company.com:8080"
git clone https://github.com/Microsoft/vcpkg.git

# Or use GitHub CLI
gh repo clone microsoft/vcpkg
```

---

## Support

For installation issues, consult:
- **Documentation:** `docs/` directory
- **Known Issues:** `KNOWN_ISSUES.md`
- **GitHub Issues:** https://github.com/yourusername/DarkThumbs/issues
- **User Guide:** `USER_GUIDE.md`

---

## Version History

| Version | Date | Installer Changes |
|---------|------|-------------------|
| 7.0.0 | 2026-02-16 | MSI + Inno Setup + Portable, 200+ formats, vcpkg integration |
| 6.2.0 | 2025-12-15 | MSI only, 155+ formats |
| 6.0.0 | 2025-11-01 | Initial installer release |

---

**DarkThumbs v7.0.0 - Installation Package Documentation**  
*Last Updated: February 16, 2026*
