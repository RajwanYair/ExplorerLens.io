# DarkThumbs Installation Guide - Windows 11 (64-bit)

## Table of Contents
1. [Prerequisites](#prerequisites)
2. [Quick Installation](#quick-installation)
3. [Build from Source](#build-from-source)
4. [Configuration](#configuration)
5. [Verification](#verification)
6. [Troubleshooting](#troubleshooting)
7. [Uninstallation](#uninstallation)

---

## Prerequisites

### System Requirements
- **Operating System:** Windows 10 64-bit (Build 19041+) or Windows 11 64-bit
- **RAM:** 4 GB minimum, 8 GB recommended
- **Disk Space:** 50 MB for installation, 500 MB for build tools
- **Administrator Access:** Required for installation

### Build Requirements (For Source Build)
- **Visual Studio Build Tools 2022** (Free)
- **Windows 11 SDK** (10.0.22621.0)
- **CMake** 3.20+ (Optional)
- **Python** 3.6+ (For tests only)

---

## Quick Installation

### Option 1: Pre-Built Binary (Easiest)

1. **Download latest release**
   ```
   Visit: https://github.com/yourusername/DarkThumbs/releases
   Download: DarkThumbs-x64-v4.6.zip
   ```

2. **Extract files**
   ```cmd
   Extract ZIP to: C:\Program Files\DarkThumbs\
   ```

3. **Run installer as Administrator**
   ```cmd
   Right-click: install-x64.cmd
   Select: Run as administrator
   ```

4. **Done!** Thumbnails will now appear in Explorer for supported files.

### Option 2: Build from Source (Recommended for Developers)

Continue to next section for detailed build instructions.

---

## Build from Source

### Step 1: Install Visual Studio Build Tools 2022

**Download URL:** https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022

**Installation Steps:**

1. Run `vs_BuildTools.exe`
2. Select **"Desktop development with C++"**
3. In **"Installation details"** pane, ensure these are checked:
   - ☑ MSVC v143 - VS 2022 C++ x64/x86 build tools (Latest)
   - ☑ Windows 11 SDK (10.0.22621.0)
   - ☑ C++ ATL for latest v143 build tools (x86 & x64)
   - ☑ C++ CMake tools for Windows (Optional)
4. Click **Install** (approx 6 GB download, 20 GB installed)
5. Restart computer after installation

**Verification:**
```cmd
# Open "x64 Native Tools Command Prompt for VS 2022"
cl
msbuild /version
```

### Step 2: Download DarkThumbs Source

**Method 1: Git Clone**
```cmd
git clone https://github.com/yourusername/DarkThumbs.git
cd DarkThumbs
```

**Method 2: Download ZIP**
```cmd
# Download from GitHub
# Extract to: C:\Dev\DarkThumbs\
```

### Step 3: Build 64-bit Release

**Method A: Automated Build Script (Recommended)**

```cmd
# Open x64 Native Tools Command Prompt for VS 2022
cd C:\Dev\DarkThumbs

# Build release version
build-release-x64.cmd
```

**Expected Output:**
```
================================================
DarkThumbs 64-bit Release Build
================================================

[1/5] Verifying build environment...
[2/5] Cleaning previous build...
[3/5] Building CBXShell.dll (64-bit, Optimized)...
[4/5] Verifying build outputs...
  [OK] CBXShell.dll
  [OK] CBXManager.exe
  [OK] UnRAR64.dll
[5/5] Build completed successfully!

Output directory: x64\Release\
CBXShell.dll
CBXManager.exe
UnRAR64.dll

DLL Size: 156 KB

Build Summary
Platform:      x64 (64-bit native)
Configuration: Release (Optimized)
Optimization:  /O2 /Ot /Oi /GL /LTCG
```

**Method B: MSBuild Command Line**

```cmd
# Clean and build
msbuild CBXShell.sln /t:Clean;Rebuild /p:Configuration=Release /p:Platform=x64 /m
```

**Method C: Visual Studio IDE**

1. Open `CBXShell.sln` in Visual Studio 2022
2. Select **Release** configuration
3. Select **x64** platform
4. **Build → Build Solution** (Ctrl+Shift+B)
5. Check **Output** window for build status

### Step 4: Run Unit Tests (Optional but Recommended)

```cmd
cd tests
run-all-tests.cmd
```

**Expected:**
```
*** ALL TEST SUITES PASSED ***
Tests Passed: 220+
Tests Failed: 0
```

### Step 5: Install Shell Extension

```cmd
# Run as Administrator!
install-x64.cmd
```

**Installation Process:**
```
[1/7] Stopping Windows Explorer...
[2/7] Unregistering previous version (if any)...
[3/7] Copying UnRAR64.dll to System32...
[4/7] Registering CBXShell.dll (64-bit COM Server)...
[5/7] Configuring file associations...
[6/7] Clearing thumbnail cache...
[7/7] Restarting Windows Explorer...

Installation Complete!
```

---

## Configuration

### Using CBXManager GUI

1. **Launch Configuration Tool**
   ```cmd
   cd x64\Release
   CBXManager.exe
   ```

2. **Available Settings:**
   - ☑ **Sort Files Alphabetically** - Order images by filename
   - ☑ **Show File Type Icon** - Display file icon overlay
   - ☑ **Enable Thumbnail Cache** - Cache thumbnails for faster display
   - ☑ **High Quality Thumbnails** - Better quality, slower generation

3. **Click Apply** to save changes

### Registry Settings (Advanced)

```cmd
# Open Registry Editor
regedit
```

**Navigate to:**
```
HKEY_CURRENT_USER\Software\T800 Productions\{9E6ECB90-5A61-42BD-B851-D3297D9C7F39}
```

**Available Keys:**
| Key | Type | Values | Default |
|-----|------|--------|---------|
| SortFiles | DWORD | 0=No, 1=Yes | 1 |
| ShowIcon | DWORD | 0=No, 1=Yes | 0 |
| ThumbnailCache | DWORD | 0=No, 1=Yes | 1 |
| ThumbnailQuality | DWORD | 1-100 | 85 |

---

## Verification

### Test 1: Create Test CBZ File

```cmd
# Create test folder
mkdir C:\Test
cd C:\Test

# Create sample images (use Paint or download)
# Save as: page01.jpg, page02.jpg, page03.jpg

# Create CBZ (just rename ZIP)
# Add images to ZIP
# Rename .zip to .cbz
```

### Test 2: View in Explorer

1. Open `C:\Test` in Windows Explorer
2. **View → Large Icons** (or Ctrl+Mouse Wheel)
3. **Thumbnail should appear** showing first image from CBZ

### Test 3: Verify Supported Formats

**Supported Extensions:**
- ✅ .cbz (Comic Book ZIP)
- ✅ .cbr (Comic Book RAR)
- ✅ .cb7 (Comic Book 7-Zip)
- ✅ .cbt (Comic Book TAR)
- ✅ .zip (Generic ZIP)
- ✅ .rar (Generic RAR)
- ✅ .7z (7-Zip)
- ✅ .tar (TAR Archive)
- ✅ .epub (EPUB Ebook)
- ✅ .mobi (Kindle)
- ✅ .azw (Kindle)
- ✅ .fb2 (FictionBook)
- ✅ .phz (Photo ZIP)

### Test 4: Check Dark Mode

1. **Settings → Personalization → Colors**
2. Toggle between **Light** and **Dark** mode
3. Thumbnails should have adaptive backgrounds:
   - Light mode: White background
   - Dark mode: Black background

---

## Troubleshooting

### Issue: Thumbnails Not Appearing

**Solution 1: Clear Thumbnail Cache**
```cmd
# Open Command Prompt as Administrator
cleanmgr /d C:

# Or manually:
del /f /s /q /a %LocalAppData%\Microsoft\Windows\Explorer\thumbcache_*.db
```

**Solution 2: Restart Explorer**
```cmd
taskkill /f /im explorer.exe
start explorer.exe
```

**Solution 3: Re-register DLL**
```cmd
cd x64\Release
regsvr32 /u CBXShell.dll
regsvr32 CBXShell.dll
```

**Solution 4: Verify Service Running**
```cmd
# Open Services (services.msc)
# Find: "Shell Hardware Detection"
# Status should be: Running
# Startup type: Automatic
```

### Issue: Build Errors

**Error: "Cannot find Windows SDK"**
```
Solution:
1. Install Windows 11 SDK (10.0.22621.0)
2. Or modify CBXShell.vcxproj:
   <WindowsTargetPlatformVersion>10.0.19041.0</WindowsTargetPlatformVersion>
```

**Error: "ATL not found" or "Cannot open atlbase.h"**
```
Solution:
1. Run VS Installer
2. Modify VS Build Tools 2022
3. Check: C++ ATL for latest v143 build tools (x64)
4. Install
```

**Error: "UnRAR64.dll not found"**
```
Solution:
copy CBXShell\UnRAR64.dll x64\Release\
```

### Issue: Runtime Errors

**Error: "The code execution cannot proceed because MSVCP140.dll was not found"**
```
Solution:
Download and install:
Visual C++ Redistributable for Visual Studio 2022 (x64)
https://aka.ms/vs/17/release/vc_redist.x64.exe
```

**Error: "Class not registered"**
```
Cause: DLL not registered or wrong architecture
Solution:
1. Verify running 64-bit Windows
2. Run install-x64.cmd as Administrator
3. Check Event Viewer for detailed errors
```

**Error: "Module could not be loaded"**
```
Cause: Missing dependencies
Solution:
1. Use Dependency Walker (depends.exe) on CBXShell.dll
2. Identify missing DLLs
3. Install missing Visual C++ Redistributables
```

### Issue: Performance Problems

**Slow Thumbnail Generation**
```
Solutions:
1. Disable sort: CBXManager → Uncheck "Sort Files"
2. Reduce quality: Registry → ThumbnailQuality → 70
3. Close other programs using Explorer
4. Defragment hard drive
5. Use SSD instead of HDD
```

**High Memory Usage**
```
Solutions:
1. Limit archive size to < 500 MB
2. Reduce thumbnail size in Explorer
3. Clear thumbnail cache regularly
4. Restart Explorer periodically
```

---

## Uninstallation

### Method 1: Automated Uninstaller

```cmd
# Run as Administrator
uninstall-x64.cmd
```

### Method 2: Manual Uninstallation

**Step 1: Unregister DLL**
```cmd
cd x64\Release
regsvr32 /u CBXShell.dll
```

**Step 2: Remove System Files**
```cmd
del %SystemRoot%\System32\UnRAR64.dll
```

**Step 3: Remove Registry Entries**
```cmd
reg delete HKCR\.cbz /f
reg delete HKCR\.cbr /f
reg delete HKCR\.epub /f
reg delete HKCR\.cb7 /f
reg delete HKCR\.cbt /f
reg delete HKCR\CBXShell.CBZ /f
reg delete HKCR\CBXShell.CBR /f
reg delete HKCR\CBXShell.EPUB /f
reg delete HKCR\CBXShell.CB7 /f
reg delete HKCR\CBXShell.CBT /f
```

**Step 4: Clear Thumbnail Cache**
```cmd
del /f /s /q /a %LocalAppData%\Microsoft\Windows\Explorer\thumbcache_*.db
```

**Step 5: Restart Explorer**
```cmd
taskkill /f /im explorer.exe
start explorer.exe
```

**Step 6: Delete Project Files (Optional)**
```cmd
# Delete entire project directory
rmdir /s /q C:\Dev\DarkThumbs
```

---

## Advanced Topics

### Silent Installation (IT Deployment)

```cmd
# Install silently (no prompts)
install-x64.cmd /S

# Or use MSI (create with WiX Toolset)
msiexec /i DarkThumbs-x64.msi /quiet /norestart
```

### Network Deployment (Group Policy)

1. Copy `x64\Release\` to network share
2. Create deployment script:
```cmd
@echo off
\\server\share\DarkThumbs\install-x64.cmd
```
3. Deploy via GPO: Computer Configuration → Policies → Windows Settings → Scripts → Startup

### Performance Tuning

**Maximum Speed (Sacrifice Quality)**
```
Registry Settings:
ThumbnailQuality = 60
SortFiles = 0
ThumbnailCache = 1
```

**Maximum Quality (Sacrifice Speed)**
```
Registry Settings:
ThumbnailQuality = 100
SortFiles = 1
ThumbnailCache = 1
```

---

## Support

- **Issues:** https://github.com/yourusername/DarkThumbs/issues
- **Discussions:** https://github.com/yourusername/DarkThumbs/discussions
- **Email:** support@example.com

---

**Installation Guide Version:** 4.6  
**Last Updated:** November 18, 2025  
**Platform:** Windows 11 64-bit
