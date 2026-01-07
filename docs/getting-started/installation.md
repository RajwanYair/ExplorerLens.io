# Installation Guide

## Overview

DarkThumbs v6.0.0 is available in multiple package formats to suit different deployment scenarios.

## Package Options

### 1. MSIX Package (Recommended for Windows 11)

**Best for:** Windows 11 users, Microsoft Store distribution, automatic updates

**Download:** `DarkThumbs_v6.0.0_x64.msix`

**Installation:**

1. Double-click the MSIX file
2. Click "Install" when prompted
3. Grant necessary permissions
4. Launch "DarkThumbs Manager" from Start Menu

**Advantages:**

- Clean installation and removal
- Automatic updates (when published to Store)
- Sandboxed and secure
- No registry pollution

### 2. MSI Installer (Recommended for Enterprise)

**Best for:** Enterprise deployments, Group Policy installation, IT management

**Download:** `DarkThumbs_v6.0.0_x64.msi`

**Installation:**

```powershell
# Silent installation
msiexec /i DarkThumbs_v6.0.0_x64.msi /quiet

# With logging
msiexec /i DarkThumbs_v6.0.0_x64.msi /quiet /l*v install.log

# Install for all users
msiexec /i DarkThumbs_v6.0.0_x64.msi ALLUSERS=1 /quiet
```

**Advantages:**

- Enterprise deployment support
- Group Policy installation
- Configurable install locations
- Logging and error reporting

### 3. Portable ZIP (No Installation Required)

**Best for:** Testing, portable drives, no-admin scenarios

**Download:** `DarkThumbs_v6.0.0_Portable.zip`

**Installation:**

1. Extract ZIP to any folder
2. Right-click `Install.cmd` → "Run as administrator"
3. Launch `CBXManager.exe` to configure

**Uninstallation:**

1. Right-click `Uninstall.cmd` → "Run as administrator"
2. Delete the folder

**Advantages:**

- No installer required
- Portable (can run from USB)
- Easy to test
- Complete control over files

## System Requirements

### Minimum Requirements

- **OS:** Windows 10 version 2004 (May 2020 Update) or later
- **Architecture:** x64 (64-bit)
- **RAM:** 4 GB
- **Disk Space:** 100 MB for installation, 1 GB recommended for cache
- **Permissions:** Administrator rights for installation

### Recommended

- **OS:** Windows 11 (latest version)
- **GPU:** DirectX 11 compatible graphics card
- **RAM:** 8 GB or more
- **Disk:** SSD for cache storage
- **Display:** 1920x1080 or higher resolution

### GPU Acceleration Requirements

For optimal performance with GPU acceleration:

- DirectX 11 compatible GPU
- Updated graphics drivers
- Minimum 2 GB VRAM (4 GB recommended)

**Supported GPU Vendors:**

- NVIDIA (GTX 600 series or newer)
- AMD (GCN architecture or newer)
- Intel (HD Graphics 4000 or newer)

## Installation Steps

### MSIX Installation (Detailed)

1. **Download Package**

   ```
   Download DarkThumbs_v6.0.0_x64.msix from releases
   ```

2. **Verify Signature** (Optional but recommended)
   - Right-click the MSIX file
   - Select "Properties"
   - Go to "Digital Signatures" tab
   - Verify signature is valid

3. **Install**
   - Double-click the MSIX file
   - Windows will show app details
   - Click "Install"
   - Wait for installation to complete

4. **Configure**
   - Open Start Menu
   - Search for "DarkThumbs Manager"
   - Launch the application
   - Select desired formats
   - Click "Apply"

5. **Verify Installation**
   - Navigate to a folder with supported files (e.g., CBZ, WebP)
   - Switch to thumbnail view
   - Thumbnails should appear

### MSI Installation (Detailed)

1. **Download Package**

   ```
   Download DarkThumbs_v6.0.0_x64.msi from releases
   ```

2. **Interactive Installation**
   - Double-click the MSI file
   - Follow the setup wizard
   - Choose installation location
   - Select components (optional)
   - Click "Install"

3. **Silent Installation** (Enterprise)

   ```powershell
   # Basic silent install
   msiexec /i "DarkThumbs_v6.0.0_x64.msi" /quiet /norestart
   
   # With full logging
   msiexec /i "DarkThumbs_v6.0.0_x64.msi" /quiet /norestart /l*v "%TEMP%\darkthumbs_install.log"
   
   # Custom install location
   msiexec /i "DarkThumbs_v6.0.0_x64.msi" /quiet INSTALLDIR="C:\Program Files\DarkThumbs"
   ```

4. **Verify Installation**
   - Check installation directory
   - Run CBXManager.exe
   - Test thumbnail generation

### Portable Installation (Detailed)

1. **Extract Package**

   ```
   Extract DarkThumbs_v6.0.0_Portable.zip to desired location
   Example: C:\Tools\DarkThumbs
   ```

2. **Register Shell Extension**
   - Right-click `Install.cmd`
   - Select "Run as administrator"
   - Wait for "SUCCESS" message

3. **Configure**
   - Launch `CBXManager.exe`
   - Configure format support
   - Click "Apply"

4. **Verify**
   - Test with supported file types
   - Check Windows Explorer thumbnails

## Post-Installation

### First-Time Configuration

1. **Launch Manager**

   ```
   Start Menu → DarkThumbs Manager
   Or: C:\Program Files\DarkThumbs\CBXManager.exe
   ```

2. **Enable Formats**
   - Comic Books: CBZ, CBR, CB7, CBT
   - eBooks: EPUB, MOBI
   - Modern Images: WebP, AVIF, JXL
   - Archives: ZIP, RAR, 7Z
   - Check desired formats

3. **Configure Performance**
   - Enable GPU acceleration (recommended)
   - Set cache size (default: 1 GB)
   - Adjust thread count if needed

4. **Apply Settings**
   - Click "Apply" or "OK"
   - Restart Explorer if prompted

### Verify Installation

1. **Check Version**

   ```powershell
   # Via Registry
   Get-ItemProperty "HKLM:\SOFTWARE\DarkThumbs" -Name Version
   
   # Via CLI (if installed)
   darkthumbs --version
   ```

2. **Test Thumbnail Generation**
   - Create test folder
   - Copy sample files (CBZ, WebP, etc.)
   - Open in Explorer (thumbnail view)
   - Thumbnails should appear within seconds

3. **Check Logs** (if issues occur)

   ```
   %LOCALAPPDATA%\DarkThumbs\logs\
   ```

## Troubleshooting Installation

### Common Issues

#### "This app can't run on your PC"

**Solution:** Ensure you're using the x64 package on a 64-bit Windows system.

#### "Windows protected your PC" (SmartScreen)

**Solution:**

- Click "More info"
- Verify publisher: "DarkThumbs Project"
- Click "Run anyway" (only if you trust the source)

#### Installation Fails with Error Code

**Solution:**

```powershell
# Check logs
Get-Content "$env:TEMP\darkthumbs_install.log" -Tail 50

# Try with full logging
msiexec /i "DarkThumbs_v6.0.0_x64.msi" /l*v "%TEMP%\install.log"
```

#### "Access Denied" During Installation

**Solution:** Run installer as Administrator

```powershell
# Right-click installer → "Run as administrator"
# Or via PowerShell:
Start-Process -FilePath "DarkThumbs_v6.0.0_x64.msi" -Verb RunAs
```

#### Thumbnails Don't Appear After Installation

**Solution:**

1. Restart Windows Explorer

   ```powershell
   Stop-Process -Name explorer -Force
   # Explorer will restart automatically
   ```

2. Clear thumbnail cache

   ```powershell
   Remove-Item "$env:LOCALAPPDATA\Microsoft\Windows\Explorer\thumbcache_*.db" -Force
   ```

3. Verify registration

   ```powershell
   # Check COM registration
   Get-Item "HKCR:\CLSID\{YOUR-CLSID-HERE}"
   ```

## Upgrading from Previous Versions

### From v5.x to v6.0

1. **Uninstall Old Version**

   ```powershell
   # If installed via MSI
   msiexec /x {OLD-PRODUCT-CODE} /quiet
   
   # If portable
   regsvr32 /u CBXShell.dll
   ```

2. **Backup Settings** (optional)

   ```powershell
   # Export registry settings
   reg export "HKCU\Software\DarkThumbs" darkthumbs_settings.reg
   ```

3. **Install v6.0**
   - Follow installation steps above

4. **Migrate Settings**
   - v6.0 will attempt automatic migration
   - Or import backed-up registry file

### Breaking Changes in v6.0

- New configuration file format
- Plugin ABI changed (plugins need recompilation)
- Cache format updated (cache will be rebuilt)

## Uninstallation

### MSIX Uninstallation

1. **Via Settings**
   - Open Settings → Apps → Installed apps
   - Find "DarkThumbs"
   - Click "..." → "Uninstall"

2. **Via PowerShell**

   ```powershell
   Get-AppxPackage -Name "DarkThumbs" | Remove-AppxPackage
   ```

### MSI Uninstallation

1. **Via Control Panel**
   - Control Panel → Programs → Uninstall a program
   - Select "DarkThumbs"
   - Click "Uninstall"

2. **Via Command Line**

   ```powershell
   msiexec /x {PRODUCT-CODE} /quiet
   # Or by path
   msiexec /x "DarkThumbs_v6.0.0_x64.msi" /quiet
   ```

### Portable Uninstallation

1. Run `Uninstall.cmd` as Administrator
2. Delete the folder

### Complete Removal

To remove all traces:

```powershell
# Uninstall (via appropriate method above)

# Remove cache
Remove-Item "$env:LOCALAPPDATA\DarkThumbs" -Recurse -Force

# Remove settings
Remove-Item "HKCU:\Software\DarkThumbs" -Recurse -Force

# Clear thumbnail cache
Remove-Item "$env:LOCALAPPDATA\Microsoft\Windows\Explorer\thumbcache_*.db" -Force

# Restart Explorer
Stop-Process -Name explorer -Force
```

## Enterprise Deployment

See [Enterprise Deployment Guide](../enterprise/deployment.md) for:

- Group Policy installation
- SCCM/Intune deployment
- Silent configuration
- Fleet management

## Next Steps

- [Quick Start Guide](quick-start.md)
- [Configuration Options](configuration.md)
- [Troubleshooting](troubleshooting.md)

---

**Last Updated:** January 6, 2026  
**Version:** 6.0.0
