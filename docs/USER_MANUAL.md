# DarkThumbs User Manual
**Sprint 25: Documentation Completion**  
**Version:** 6.2.0  
**Last Updated:** February 15, 2026

---

## Table of Contents
1. [Introduction](#introduction)
2. [Installation](#installation)
3. [CBXManager Configuration Tool](#cbxmanager-configuration-tool)
4. [Supported Formats](#supported-formats)
5. [Advanced Configuration](#advanced-configuration)
6. [Performance Tuning](#performance-tuning)
7. [Backup & Restore](#backup--restore)
8. [Uninstallation](#uninstallation)
9. [FAQ](#faq)

---

## Introduction

### What is DarkThumbs?

**DarkThumbs** is a Windows Shell Extension that enhances File Explorer with high-performance thumbnail generation for **55+ image, video, archive, and document formats**.

**Key Features:**
- 🖼️ **Modern Format Support:** WebP, AVIF, JPEG XL, HEIF, JXL  
- 📦 **Archive Previews:** RAR, 7z, ZIP with smart file selection  
- 🎬 **Video Thumbnails:** MP4, MKV, WEBM without Windows codecs  
- 🏎️ **GPU Acceleration:** Direct3D 11 rendering (10x faster)  
- 💾 **Intelligent Caching:** LRU cache with configurable size  
- 🌙 **Dark Mode:** Native Windows 11 dark theme integration  
- 🔒 **Safe:** No Explorer crashes, graceful fallbacks, thread-safe  

---

### System Requirements

**Minimum:**
- Windows 10 (version 1809 or later) 64-bit
- 4 GB RAM
- 500 MB disk space
- DirectX 11 compatible GPU (optional, uses software fallback)

**Recommended:**
- Windows 11 64-bit
- 8 GB RAM
- 1 GB disk space (for cache)
- NVIDIA/AMD GPU with D3D11 support

---

## Installation

### Standard Installation (MSI Installer)

1. **Download** the latest release:
   - GitHub Releases: https://github.com/YourOrg/DarkThumbs/releases
   - File: `DarkThumbs-Setup-6.2.0.msi` (~15 MB)

2. **Run Installer** (Right-click → **Run as Administrator**):
   ```powershell
   .\DarkThumbs-Setup-6.2.0.msi
   ```

3. **Follow Wizard:**
   - Accept license agreement
   - Choose installation directory (default: `C:\Program Files\DarkThumbs\`)
   - Select formats to enable (all recommended)
   - Click **Install**

4. **Verify Installation:**
   - Open File Explorer
   - Navigate to folder with WebP/AVIF/JXL images
   - Thumbnails should appear automatically
   - **If not:** Right-click folder → **Refresh** (or press F5)

---

### Silent Installation (Enterprise Deployment)

```powershell
# Install for all users
msiexec /i DarkThumbs-Setup-6.2.0.msi /qn /norestart

# Install with logging
msiexec /i DarkThumbs-Setup-6.2.0.msi /qn /l*v install.log

# Group Policy deployment
# Copy MSI to network share, create GPO:
# Computer Configuration → Software Settings → Software Installation
```

---

### Manual Installation (Development Builds)

```powershell
# Register COM server
cd "C:\Path\To\Build\bin\Release"
regsvr32 CBXShell.dll

# Configure via CBXManager
.\CBXManager.exe
```

---

## CBXManager Configuration Tool

**CBXManager** is the graphical configuration utility for DarkThumbs. Launch it from:
- Start Menu → **DarkThumbs** → **CBXManager**
- Or: `C:\Program Files\DarkThumbs\CBXManager.exe`

### Main Window

![CBXManager Main Window](images/cbxmanager_main.png)

**Components:**
1. **Format Selection List** (left panel)
2. **Action Buttons** (center)
3. **Health Check Panel** (right)
4. **Status Bar** (bottom)

---

### Format Selection

**List View Columns:**
| Column | Description |
|--------|-------------|
| **Format** | File format name (e.g., WebP, AVIF) |
| **Extensions** | Associated file extensions (*.webp) |
| **Registered** | ✅ Enabled / ❌ Disabled |
| **Decoder** | Decoder class name (WebPDecoder) |

**Selecting Formats:**
- **Ctrl+Click:** Select multiple formats
- **Ctrl+A:** Select all formats
- **Search Box:** Filter by name/extension

---

### Action Buttons

#### **Install** Button
Registers selected formats with Windows Shell.

**What it does:**
1. Creates registry keys: `HKCR\.<extension>\shellex\{IThumbnailProvider}`
2. Sets CLSID: `{A8394D0D-EE2B-4A00-9FAC-AB8D3B03F078}`
3. Restarts Explorer to apply changes

**Example:** Enable WebP thumbnails:
1. Select "WebP Images" in list
2. Click **Install**
3. Wait for "Registration successful" message

---

#### **Uninstall** Button
Removes selected formats from Shell.

**What it does:**
1. Deletes CLSID from `HKCR\.<extension>\shellex\{IThumbnailProvider}`
2. Optionally restores previous handler (backup system)
3. Restarts Explorer

**Use case:** Disable DarkThumbs for specific formats

---

#### **Uninstall All** Button
Removes all DarkThumbs registrations.

**What it does:**
1. Scans all registered extensions
2. Removes DarkThumbs CLSID from every extension
3. Restores previous handlers (if backed up)
4. Unregisters COM server: `regsvr32 /u CBXShell.dll`

**Use case:** Complete cleanup before uninstalling

---

#### **Backup** Button
Saves current registry state.

**What it does:**
1. Exports to: `C:\ProgramData\DarkThumbs\Backup\registry_backup_<timestamp>.reg`
2. Includes:
   - All `HKCR\.<extension>` keys
   - DarkThumbs configuration (`HKLM\SOFTWARE\DarkThumbs`)
   - COM registration (`HKCR\CLSID\{A8394D0D-...}`)

**Recommendation:** Always backup before major changes

---

#### **Restore** Button
Restores registry from backup.

**What it does:**
1. Lists available backups (sorted by date)
2. Imports selected .reg file
3. Restarts Explorer

**Use case:** Undo changes, recover from conflicts

---

### Health Check Panel

Real-time diagnostic display on the right side of CBXManager.

**Indicators:**

| Status | Meaning | Action |
|--------|---------|--------|
| ✅ **COM Server Registered** | CBXShell.dll properly registered | None |
| ⚠️ **Cache Size: 1.2 GB** | Current cache disk usage | Click "Clear Cache" if too large |
| ✅ **GPU Acceleration: Enabled** | DirectX 11 renderer active | None |
| ⚠️ **Conflicts Detected: 2** | Other handlers registered | Click "Resolve Conflicts" |
| ✅ **All Tests Passed** | Decoder self-tests successful | None |

**Auto-refresh:** Panel updates every 30 seconds

---

### Dark Mode

DarkThumbs automatically detects Windows theme.

**Manual Toggle:**
1. Menu → **View** → **Dark Mode**
2. Or: `Alt+D` keyboard shortcut

**Registry override:**
```powershell
Set-ItemProperty "HKLM:\SOFTWARE\DarkThumbs" -Name ForceDarkMode -Value 1
# 0=Light, 1=Dark, 2=Auto (default)
```

---

## Supported Formats

### Full Format List (55+ formats)

#### **Modern Image Formats**
| Extension | Format Name | Decoder | Notes |
|-----------|-------------|---------|-------|
| `.webp` | Google WebP | libwebp 1.5.0 | Lossy & lossless |
| `.avif` | AV1 Image | libavif 1.1.1 | HDR support, 10-bit |
| `.jxl` | JPEG XL | libjxl 0.11.1 | HDR, lossy/lossless |
| `.heic`, `.heif` | HEIF/HEIC | libheif 1.18 | Apple Photos format |

#### **Standard Image Formats**
| Extension | Format Name | Decoder | Notes |
|-----------|-------------|---------|-------|
| `.jpg`, `.jpeg` | JPEG | WIC (Windows) | Fast hardware decode |
| `.png` | PNG | WIC (Windows) | Transparency support |
| `.bmp`, `.dib` | Bitmap | WIC (Windows) | Uncompressed |
| `.gif` | GIF | WIC (Windows) | Animation not supported |
| `.tif`, `.tiff` | TIFF | WIC (Windows) | Multi-page support |
| `.ico` | Icon | WIC (Windows) | Multi-resolution |

#### **RAW Camera Formats**
| Extension | Format Name | Camera Brand | Decoder |
|-----------|-------------|--------------|---------|
| `.cr2`, `.cr3` | Canon RAW | Canon | LibRaw 0.21 |
| `.nef`, `.nrw` | Nikon RAW | Nikon | LibRaw 0.21 |
| `.arw`, `.srf` | Sony RAW | Sony | LibRaw 0.21 |
| `.orf` | Olympus RAW | Olympus | LibRaw 0.21 |
| `.raf` | Fujifilm RAW | Fujifilm | LibRaw 0.21 |
| `.dng` | Adobe DNG | Universal | LibRaw 0.21 |
| `.rw2` | Panasonic RAW | Panasonic | LibRaw 0.21 |

#### **Video Formats**
| Extension | Format Name | Codec Support | Decoder |
|-----------|-------------|---------------|---------|
| `.mp4` | MPEG-4 | H.264, H.265, AV1 | FFmpeg 7.1 |
| `.mkv` | Matroska | Any | FFmpeg 7.1 |
| `.webm` | WebM | VP8, VP9, AV1 | FFmpeg 7.1 |
| `.avi` | AVI | Various | FFmpeg 7.1 |
| `.mov` | QuickTime | H.264, ProRes | FFmpeg 7.1 |
| `.wmv` | Windows Media | WMV9 | FFmpeg 7.1 |
| `.flv` | Flash Video | VP6, H.264 | FFmpeg 7.1 |

**Thumbnail Extraction:** Seeks to 10% into video, applies scene detection

#### **Document Formats**
| Extension | Format Name | Decoder | Notes |
|-----------|-------------|---------|-------|
| `.pdf` | PDF | MuPDF 1.24 | Renders first page |
| `.svg` | SVG Vector | lunasvg | Rasterizes to target size |
| `.eps` | PostScript | Ghostscript | Requires external install |
| `.psd` | Photoshop | Custom parser | Layer preview |

#### **Archive Formats**
| Extension | Format Name | Library | Preview Logic |
|-----------|-------------|---------|---------------|
| `.zip` | ZIP | minizip-ng | First image/video |
| `.rar` | RAR | UnRAR | First image/video |
| `.7z` | 7-Zip | LZMA SDK 24.08 | First image/video |
| `.tar`, `.tar.gz` | Tarball | libarchive | First image/video |
| `.cbz`, `.cbr` | Comic Book | minizip/UnRAR | First page |

**Archive Logic:**
1. List archive contents
2. Find first image/video file (sorted alphabetically)
3. Extract to memory
4. Decode via appropriate decoder
5. Cache result

#### **Audio Formats (Album Art)**
| Extension | Format Name | Decoder | Notes |
|-----------|-------------|---------|-------|
| `.mp3` | MP3 | TagLib 2.0 | ID3v2 APIC frame |
| `.flac` | FLAC | TagLib 2.0 | METADATA_BLOCK_PICTURE |
| `.m4a`, `.mp4` | AAC | TagLib 2.0 | iTunes covr atom |
| `.ogg`, `.opus` | Vorbis/Opus | TagLib 2.0 | METADATA_BLOCK_PICTURE |
| `.wma` | Windows Media | TagLib 2.0 | WM/Picture |

#### **3D Model Formats**
| Extension | Format Name | Decoder | Notes |
|-----------|-------------|---------|-------|
| `.obj` | Wavefront OBJ | Custom parser | Renders with flat shading |
| `.stl` | Stereolithography | Custom parser | Renders with smooth normals |
| `.gltf`, `.glb` | glTF 2.0 | tinygltf | PBR materials |
| `.ply` | Polygon File | Custom parser | Vertex colors |

**3D Rendering:** Uses Direct3D 11 with orthographic camera, ambient + directional lighting

---

## Advanced Configuration

### Registry Settings

All settings stored in: `HKEY_LOCAL_MACHINE\SOFTWARE\DarkThumbs`

**Viewing current settings:**
```powershell
Get-ItemProperty "HKLM:\SOFTWARE\DarkThumbs"
```

---

### Performance Settings

#### **Enable GPU Acceleration** (default: ON)
```powershell
Set-ItemProperty "HKLM:\SOFTWARE\DarkThumbs" -Name UseGPU -Value 1
# 0=CPU only, 1=GPU (with WARP fallback)
```

**Effect:** Uses D3D11 hardware renderer (~10x faster for large images)

---

#### **Cache Size** (default: 500 MB)
```powershell
Set-ItemProperty "HKLM:\SOFTWARE\DarkThumbs" -Name CacheSizeMB -Value 1024
```

**Recommendations:**
- 500 MB: Default (stores ~5,000 thumbnails)
- 1 GB: Heavy photo usage
- 2 GB: Professional photographers
- 0: Disable cache (not recommended)

---

#### **Thread Pool Size** (default: CPU core count)
```powershell
Set-ItemProperty "HKLM:\SOFTWARE\DarkThumbs" -Name MaxThreads -Value 8
```

**Effect:** Controls parallel decode operations  
**Recommendations:**
- Fast SSDs: Match core count
- Slow HDDs: Reduce to 4 to avoid thrashing

---

#### **Max Image Dimensions** (default: 50 MP)
```powershell
Set-ItemProperty "HKLM:\SOFTWARE\DarkThumbs" -Name MaxImagePixels -Value 100000000
# 100 MP = 100,000,000 pixels
```

**Effect:** Skip oversized images (prevents OOM)  
**Example:** 10000x10000 = 100 MP

---

### Decoder-Specific Settings

#### **Video Seek Position** (default: 10%)
```powershell
Set-ItemProperty "HKLM:\SOFTWARE\DarkThumbs" -Name VideoSeekPercent -Value 25
# Seek to 25% into video instead of 10%
```

---

#### **Archive Preview Limit** (default: first file)
```powershell
Set-ItemProperty "HKLM:\SOFTWARE\DarkThumbs" -Name ArchiveMaxFiles -Value 5
# Create montage from first 5 images in archive
```

---

#### **RAW Embedded Thumbnail** (default: OFF)
```powershell
Set-ItemProperty "HKLM:\SOFTWARE\DarkThumbs" -Name RAWUseEmbedded -Value 1
# Use embedded JPEG preview (faster but lower quality)
```

---

### Logging

#### **Enable Debug Logging**
```powershell
Set-ItemProperty "HKLM:\SOFTWARE\DarkThumbs" -Name LogLevel -Value 4
# 0=Off, 1=Error, 2=Warn, 3=Info, 4=Debug
```

**Log Location:** `C:\ProgramData\DarkThumbs\Logs\Engine.log`

**Viewing logs:**
```powershell
Get-Content "C:\ProgramData\DarkThumbs\Logs\Engine.log" -Tail 50 -Wait
```

---

## Performance Tuning

### Scenario: Fast Thumbnail Generation

**Goal:** Minimize latency when browsing folders

**Settings:**
```powershell
# Enable all optimizations
Set-ItemProperty "HKLM:\SOFTWARE\DarkThumbs" -Name UseGPU -Value 1
Set-ItemProperty "HKLM:\SOFTWARE\DarkThumbs" -Name CacheSizeMB -Value 2048
Set-ItemProperty "HKLM:\SOFTWARE\DarkThumbs" -Name MaxThreads -Value 16
Set-ItemProperty "HKLM:\SOFTWARE\DarkThumbs" -Name RAWUseEmbedded -Value 1
```

**Additional Tips:**
- Use SSD for cache directory
- Increase Windows icon cache size
- Enable "Always show icons, never thumbnails" for network drives

---

### Scenario: Low Memory Usage

**Goal:** Minimize RAM consumption (4 GB systems)

**Settings:**
```powershell
# Reduce resource usage
Set-ItemProperty "HKLM:\SOFTWARE\DarkThumbs" -Name CacheSizeMB -Value 100
Set-ItemProperty "HKLM:\SOFTWARE\DarkThumbs" -Name MaxThreads -Value 2
Set-ItemProperty "HKLM:\SOFTWARE\DarkThumbs" -Name MaxImagePixels -Value 25000000  # 25 MP
Set-ItemProperty "HKLM:\SOFTWARE\DarkThumbs" -Name MaxFileSizeMB -Value 50
```

---

### Scenario: Network Drive Performance

**Problem:** Thumbnails on NAS/SMB shares are slow

**Solution:**
```powershell
# Pre-cache thumbnails locally
$networkPath = "\\NAS\Photos"
cd "C:\Program Files\DarkThumbs"
.\EngineBenchmark.exe --cache-warmup $networkPath

# Result: All thumbnails generated once, cached locally
```

---

## Backup & Restore

### Why Backup?

**Scenarios requiring backup:**
- Testing new formats
- Resolving conflicts with other shell extensions
- Uninstalling/reinstalling DarkThumbs
- Migrating to new Windows installation

---

### Creating Backups

#### **Via CBXManager** (Recommended)
1. Launch CBXManager
2. Click **Backup** button
3. Backup saved to: `C:\ProgramData\DarkThumbs\Backup\`
4. Filename: `registry_backup_2026-02-15_143022.reg`

---

#### **Via Command Line**
```powershell
# Export all DarkThumbs registry keys
reg export "HKLM\SOFTWARE\DarkThumbs" "C:\Backup\darkthumbs_config.reg"
reg export "HKCR\CLSID\{A8394D0D-EE2B-4A00-9FAC-AB8D3B03F078}" "C:\Backup\darkthumbs_com.reg"

# Export all extension associations
$extensions = @(".webp", ".avif", ".jxl", ".heic", ".cr3", ".mkv", ".cbz")
foreach ($ext in $extensions) {
    reg export "HKCR\$ext" "C:\Backup\ext_$($ext.TrimStart('.')).reg"
}
```

---

### Restoring Backups

#### **Via CBXManager**
1. Launch CBXManager
2. Click **Restore** button
3. Select backup file from list
4. Click **Restore Selected**
5. Explorer restarts automatically

---

#### **Via Command Line**
```powershell
# Import registry backup
reg import "C:\Backup\darkthumbs_config.reg"

# Restart Explorer
Stop-Process -Name explorer -Force
Start-Process explorer.exe
```

---

### Smart Uninstall (Backup System)

**How it works:**
1. Before installing DarkThumbs, existing handlers are backed up to:
   `HKCR\.<extension>\shellex\{IThumbnailProvider}_DarkThumbsBackup`

2. When uninstalling, DarkThumbs restores previous handlers

**Example:**
```
Before DarkThumbs:
    HKCR\.webp\shellex\{IThumbnailProvider} = {Windows.Shell.WebPHandler}

After DarkThumbs install:
    HKCR\.webp\shellex\{IThumbnailProvider} = {A8394D0D-EE2B-4A00-9FAC-AB8D3B03F078}
    HKCR\.webp\shellex\{IThumbnailProvider}_DarkThumbsBackup = {Windows.Shell.WebPHandler}

After DarkThumbs uninstall:
    HKCR\.webp\shellex\{IThumbnailProvider} = {Windows.Shell.WebPHandler}
    (Backup key deleted)
```

---

## Uninstallation

### Standard Uninstall

**Method 1: Windows Settings**
1. Open **Settings** → **Apps** → **Installed apps**
2. Search "DarkThumbs"
3. Click **⋮** → **Uninstall**
4. Click **Uninstall** again to confirm

**Method 2: Control Panel**
1. Open **Control Panel** → **Programs** → **Uninstall a program**
2. Select "DarkThumbs"
3. Click **Uninstall**

**Method 3: MSI Uninstall**
```powershell
msiexec /x {A8394D0D-EE2B-4A00-9FAC-AB8D3B03F078}
# Or: msiexec /x DarkThumbs-Setup-6.2.0.msi
```

---

### Clean Uninstall (Complete Removal)

```powershell
# 1. Unregister all formats
cd "C:\Program Files\DarkThumbs"
.\CBXManager.exe
# Click "Uninstall All" button

# 2. Unregister COM server
regsvr32 /u CBXShell.dll

# 3. Delete cache
Remove-Item "C:\ProgramData\DarkThumbs\Cache" -Recurse -Force

# 4. Delete configuration
Remove-Item "HKLM:\SOFTWARE\DarkThumbs" -Recurse -Force

# 5. Delete COM registration
Remove-Item "HKCR:\CLSID\{A8394D0D-EE2B-4A00-9FAC-AB8D3B03F078}" -Recurse -Force

# 6. Clear thumbnail cache
Remove-Item "$env:LOCALAPPDATA\Microsoft\Windows\Explorer\thumbcache_*.db" -Force

# 7. Restart Explorer
Stop-Process -Name explorer -Force
Start-Process explorer.exe
```

---

## FAQ

### **Q: Do thumbnails work for files on network drives?**
**A:** Yes, but performance depends on network speed. Use `--cache-warmup` to pre-generate thumbnails for better UX.

---

### **Q: Can I use DarkThumbs alongside other thumbnail providers?**
**A:** Yes, but conflicts may occur. Use CBXManager → "Scan for Conflicts" to detect issues. DarkThumbs includes a smart backup system to restore previous handlers.

---

### **Q: Why do some RAW files show generic icons?**
**A:** Some RAW formats lack embedded thumbnails. DarkThumbs will decode the full image (slower). Enable embedded thumbnail mode for faster previews:
```powershell
Set-ItemProperty "HKLM:\SOFTWARE\DarkThumbs" -Name RAWUseEmbedded -Value 1
```

---

### **Q: Does DarkThumbs support HDR images?**
**A:** Yes, AVIF and JXL formats support HDR. Thumbnails are tone-mapped to SDR for display in Explorer.

---

### **Q: How do I clear the cache?**
**A:**
```powershell
Remove-Item "C:\ProgramData\DarkThumbs\Cache" -Recurse -Force
```
Or: CBXManager → Health Check Panel → "Clear Cache" button

---

### **Q: Can I customize the 3D model renderer (lighting, camera angle)?**
**A:** Not via GUI. Advanced users can edit registry:
```powershell
Set-ItemProperty "HKLM:\SOFTWARE\DarkThumbs" -Name ModelCameraAngle -Value 45
Set-ItemProperty "HKLM:\SOFTWARE\DarkThumbs" -Name ModelLightIntensity -Value 1.5
```

---

### **Q: Does DarkThumbs work on Windows 11 ARM?**
**A:** Not officially supported. x64 binaries may work via emulation, but performance will be poor. ARM64 build planned for future release.

---

### **Q: How do I report bugs or request features?**
**A:** GitHub Issues: https://github.com/YourOrg/DarkThumbs/issues

---

### **Q: Is DarkThumbs open-source?**
**A:** Yes, MIT license. See [LICENSE](../LICENSE) file.

---

### **Q: Can I use DarkThumbs in commercial software?**
**A:** Yes, MIT license allows commercial use. Some dependencies (FFmpeg, LibRaw) have LGPL licenses — check [LICENSE](../LICENSE) for details.

---

### **Q: Why does Windows Defender flag the installer?**
**A:** Unsigned binaries may trigger SmartScreen. For production releases, binaries are code-signed (see [CODE_SIGNING.md](CODE_SIGNING.md)).

---

**Next Steps:**
- [Troubleshooting Guide](TROUBLESHOOTING.md) - Fix common issues
- [Performance Tuning](PERFORMANCE.md) - Optimize for your workflow
- [Plugin Development](PLUGIN_DEVELOPMENT.md) - Create custom decoders

---

**Last Updated:** February 15, 2026  
**Sprint:** 25 - Documentation Completion  
**Version:** 6.2.0
