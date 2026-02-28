# ExplorerLens User Guide
**Version:** 15.0.0 
**Last Updated:** July 2025

## Table of Contents
- [Installation](#installation)
- [Getting Started](#getting-started)
- [Supported Formats](#supported-formats)
- [Configuration](#configuration)
- [Troubleshooting](#troubleshooting)

---

## Installation

### System Requirements
- **OS:** Windows 11 (x64) or Windows 10 (x64) version 1809+
- **Memory:** 4 GB RAM minimum, 8 GB recommended
- **GPU:** DirectX 11 compatible (for hardware acceleration)
- **Disk Space:** 50 MB for installation
- **.NET Runtime:** Not required (native C++ application)

### Installation Steps

1. **Download** the latest installer from the [Releases](https://github.com/yourusername/ExplorerLens/releases) page

2. **Run the installer** as Administrator:
 ```powershell
 # Right-click installer → "Run as Administrator"
 .\ExplorerLens-Setup-15.0.0.msi
 ```

3. **Follow the wizard**:
 - Accept the license agreement
 - Choose installation directory (default: `C:\Program Files\ExplorerLens`)
 - Select file type associations
 - Click "Install"

4. **Verify installation**:
 - Open **File Explorer**
 - Navigate to a folder with `.cbz`, `.webp`, or `.jxl` files
 - Thumbnails should appear automatically
 - If not, see [Troubleshooting](#troubleshooting)

### Alternative: Manual Installation

Use the PowerShell script for custom installations:

```powershell
# Clone or extract source
cd ExplorerLens

# Run installation script
.\Install-ExplorerLens.ps1 -Verbose

# Restart Explorer
Stop-Process -Name explorer -Force
```

---

## Getting Started

### Viewing Thumbnails

1. **Open File Explorer** (Win+E)
2. **Navigate** to a folder with supported files
3. **Change view** to Large Icons, Extra Large Icons, or Tiles
4. **Thumbnails appear automatically** for:
 - Comic books (`.cbz`, `.cbr`, `.cb7`, `.cbt`)
 - Archives (`.zip`, `.rar`, `.7z`, `.tar`, `.gz`)
 - Modern images (`.webp`, `.jxl`, `.avif`, `.heic`)
 - RAW photos (`.cr2`, `.nef`, `.arw`, `.dng`, etc.)
 - Videos (`.mp4`, `.mkv`, `.avi`, `.mov`, etc.)
 - Audio (`.mp3`, `.flac`, `.wav` with album art)

### Using LENSManager

**LENSManager** is the configuration utility for ExplorerLens.

1. **Launch LENSManager**:
 - Search for "LENSManager" in Start Menu
 - Or run: `C:\Program Files\ExplorerLens\LENSManager.exe`

2. **Configuration Options**:
 - **Enable/Disable** thumbnail handler
 - **Set thumbnail quality** (Fast, Balanced, High Quality)
 - **Configure cache settings**
 - **Choose GPU** (if multiple GPUs available)
 - **View statistics** (cache hits, decode times)
 - **Force cache clear**

3. **Quick Actions**:
 - **Register Handler:** Enables ExplorerLens for all file types
 - **Unregister Handler:** Disables ExplorerLens temporarily
 - **Clear Thumbnail Cache:** Removes all cached thumbnails
 - **Rebuild Cache:** Forces regeneration of thumbnails

---

## Supported Formats

ExplorerLens supports **200+ file formats** across multiple categories:

### Archives & Comic Books
- **Comic Books:** `.cbz`, `.cbr`, `.cb7`, `.cbt`
- **Archives:** `.zip`, `.rar`, `.7z`, `.tar`, `.gz`, `.bz2`, `.xz`, `.arj`, `.lzh`

### Images
- **Standard:** `.jpg`, `.jpeg`, `.png`, `.bmp`, `.gif`, `.tiff`, `.tif`
- **Modern:** `.webp`, `.jxl` (JPEG XL), `.avif`, `.heic`, `.heif` ✅
- **Professional:** `.psd`, `.psb` (Photoshop), `.svg` (vector), `.exr` (OpenEXR), `.hdr` (Radiance)
- **Texture/Legacy:** `.dds`, `.tga`, `.ico`, `.qoi`, `.ppm`/`.pgm`/`.pbm`
- **RAW Photos:** `.cr2`, `.cr3`, `.nef`, `.arw`, `.orf`, `.dng`, `.rw2`, `.raf`, `.pef`, `.dcr`, `.mrw`, `.x3f`, `.gpr` + 100 more camera formats

### Videos
- **Common:** `.mp4`, `.avi`, `.mkv`, `.mov`, `.wmv`, `.flv`, `.webm`
- **Professional:** `.mts`, `.m2ts`, `.vob`, `.mpg`, `.mpeg`, `.m4v`

### Audio (Album Art)
- **Lossless:** `.flac`, `.ape`, `.wav`, `.wv`, `.tta`
- **Compressed:** `.mp3`, `.m4a`, `.aac`, `.ogg`, `.opus`

### Documents (✅ Supported)
- **PDF:** `.pdf` (first page thumbnail)
- **Office:** `.docx`, `.xlsx`, `.pptx` (via Windows thumbnail provider)

### Fonts & 3D Models (✅ Supported)
- **Fonts:** `.ttf`, `.otf`, `.ttc` (font preview rendering)
- **3D Models:** `.obj`, `.stl`, `.gltf`, `.glb` (orthographic preview)

---

## Configuration

### Performance Tuning

**For fast machines** (8+ cores, SSD, dedicated GPU):
```powershell
# High quality, no compromises
Set-ItemProperty -Path "HKLM:\Software\ExplorerLens" -Name "Quality" -Value "High"
Set-ItemProperty -Path "HKLM:\Software\ExplorerLens" -Name "CacheSize" -Value 2048
Set-ItemProperty -Path "HKLM:\Software\ExplorerLens" -Name "GPUAcceleration" -Value 1
```

**For slow machines** (4 cores, HDD, integrated GPU):
```powershell
# Balanced quality, faster decoding
Set-ItemProperty -Path "HKLM:\Software\ExplorerLens" -Name "Quality" -Value "Balanced"
Set-ItemProperty -Path "HKLM:\Software\ExplorerLens" -Name "CacheSize" -Value 512
Set-ItemProperty -Path "HKLM:\Software\ExplorerLens" -Name "GPUAcceleration" -Value 1
```

### GPU Selection

If you have multiple GPUs (e.g., Intel iGPU + NVIDIA dGPU):

```powershell
# List available GPUs
.\LENSManager.exe /ListGPUs

# Set preferred GPU (0 = default, 1 = second GPU, etc.)
Set-ItemProperty -Path "HKLM:\Software\ExplorerLens" -Name "PreferredGPU" -Value 1
```

### Cache Configuration

```powershell
# Cache location (default: %LOCALAPPDATA%\ExplorerLens\Cache)
Set-ItemProperty -Path "HKLM:\Software\ExplorerLens" -Name "CachePath" -Value "D:\Cache\ExplorerLens"

# Cache size in MB (default: 1024 MB)
Set-ItemProperty -Path "HKLM:\Software\ExplorerLens" -Name "CacheSize" -Value 2048

# Clear cache
Remove-Item "$env:LOCALAPPDATA\ExplorerLens\Cache\*" -Recurse -Force
```

---

## Troubleshooting

### Thumbnails Not Appearing

1. **Restart File Explorer**:
 ```powershell
 Stop-Process -Name explorer -Force
 ```

2. **Clear thumbnail cache**:
 ```powershell
 .\LENSManager.exe /ClearCache
 ```

3. **Re-register handler**:
 ```powershell
 regsvr32 /i "C:\Program Files\ExplorerLens\LENSShell.dll"
 ```

4. **Check Windows thumbnail settings**:
 - Open **File Explorer Options**
 - Go to **View** tab
 - Ensure "Always show icons, never thumbnails" is **unchecked**

### Slow Thumbnail Generation

- **Increase thumbnail size**: Thumbnails > 256x256 take longer
- **Check disk speed**: HDDs are slower than SSDs
- **Verify GPU acceleration**: Open LENSManager → GPU Status
- **Reduce image quality**: Set Quality to "Fast" in LENSManager

### Crashes or Errors

1. **Check Event Viewer**:
 ```powershell
 Get-EventLog -LogName Application -Source ExplorerLens -Newest 10
 ```

2. **Enable debug logging**:
 ```powershell
 Set-ItemProperty -Path "HKLM:\Software\ExplorerLens" -Name "DebugLog" -Value 1
 # Logs written to: %TEMP%\ExplorerLens-Debug.log
 ```

3. **Report issues** on GitHub with:
 - Windows version
 - File type causing crash
 - Debug log excerpt

---

## Uninstallation

### Via Control Panel
1. Open **Settings** → **Apps** → **Installed apps**
2. Find **ExplorerLens**
3. Click **Uninstall**

### Via PowerShell
```powershell
# Unregister handler
regsvr32 /u "C:\Program Files\ExplorerLens\LENSShell.dll"

# Remove registry keys
Remove-Item -Path "HKLM:\Software\ExplorerLens" -Recurse -Force

# Clear cache
Remove-Item "$env:LOCALAPPDATA\ExplorerLens" -Recurse -Force

# Uninstall MSI
msiexec /x {ExplorerLens-GUID} /quiet
```

---

## FAQ

**Q: Does ExplorerLens replace Windows thumbnail providers?** 
A: No, ExplorerLens *extends* Windows thumbnail support. Native formats (JPG, PNG) still use Windows providers.

**Q: Can I use ExplorerLens with network drives?** 
A: Yes, but performance depends on network speed. Enable caching for best results.

**Q: Is GPU acceleration required?** 
A: No, ExplorerLens works with CPU-only decoding. GPU acceleration improves performance for large images.

**Q: How much memory does ExplorerLens use?** 
A: ~50-100 MB idle, ~200-500 MB when actively decoding large files.

**Q: Can I customize thumbnail appearance (borders, shadows)?** 
A: Thumbnail appearance is controlled by Windows Explorer. ExplorerLens only provides the image data.

---

## Additional Resources

- **GitHub Repository:** https://github.com/yourusername/ExplorerLens
- **Issue Tracker:** https://github.com/yourusername/ExplorerLens/issues
- **Developer Guide:** [DEVELOPER_GUIDE.md](DEVELOPER_GUIDE.md)
- **Build Instructions:** [.github/standards/BUILD_METHOD.md](.github/standards/BUILD_METHOD.md)

**Support:** For issues, please open a GitHub issue with detailed information.

---

**Enjoy seamless thumbnail previews for all your files! 🎨**

