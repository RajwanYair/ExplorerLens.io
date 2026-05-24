# ExplorerLens User Guide

**Version:** 40.0.1 "Procyon"
**Last Updated:** July 2026

## Table of Contents

- [Installation](#installation)
- [Getting Started](#getting-started)
- [Manager (WinUI)](#manager-winui)
- [Command-Line Interface](#command-line-interface)
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

1. **Download** the latest installer from the [Releases](https://github.com/RajwanYair/ExplorerLens.io/releases) page

1. **Run the installer** as Administrator:

 ```powershell
 # Right-click installer → "Run as Administrator"
 .\ExplorerLens-40.0.1-x64.msi
 ```

1. **Follow the wizard**:

- Accept the license agreement
- Choose installation directory (default: `C:\Program Files\ExplorerLens`)
- Select file type associations
- Click "Install"

1. **Verify installation**:

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
.\packaging\Install-ExplorerLens.ps1 -Verbose

# Restart Explorer
Stop-Process -Name explorer -Force
```

---

## Getting Started

### Viewing Thumbnails

1. **Open File Explorer** (Win+E)
1. **Navigate** to a folder with supported files
1. **Change view** to Large Icons, Extra Large Icons, or Tiles
1. **Thumbnails appear automatically** for:

- Comic books (`.cbz`, `.cbr`, `.cb7`, `.cbt`)
- Archives (`.zip`, `.rar`, `.7z`, `.tar`, `.gz`)
- Modern images (`.webp`, `.jxl`, `.avif`, `.heic`)
- RAW photos (`.cr2`, `.nef`, `.arw`, `.dng`, etc.)
- Videos (`.mp4`, `.mkv`, `.avi`, `.mov`, etc.)
- Audio (`.mp3`, `.flac`, `.wav` with album art)

### Using LENSManager

**LENSManager** is the configuration utility for ExplorerLens. It ships in two flavors:

- **Manager.WinUI.exe** — Modern WinUI 3 interface (recommended)
- **LENSManager.exe** — Classic WTL/Win32 interface (lightweight fallback)

---

## Manager (WinUI)

The modern Manager provides a full-featured GUI organized into pages:

### Dashboard

- **Status cards** — Registration status, GPU backend, cache hit rate, thumbnails/session
- **Throughput sparkline** — Real-time graph of thumbnails generated per second (last 60s)
- **Format breakdown** — Visual bar chart showing which formats are decoded most
- **Quick Actions** — One-click buttons for Clear Cache, Restart Explorer, Export Logs, Manage Formats

### Formats

- **Search** by extension, decoder name, MIME type, or description
- **Category filter** — Images, Archives, Documents, RAW Camera, Video, Audio, 3D Models, CAD/BIM,
  Scientific, Fonts, Source Code
- **GPU filter** — Show only GPU-accelerated or CPU-only formats
- **Batch actions** — Register All, Register Category, Unregister All, Apply & Restart
- **Format detail panel** — Select any format to see decoder, MIME type, latency, GPU support, and a Test Decode button

### Cache

- **Live statistics** — Hit rate, entry count, budget used, average lookup time
- **Quick Profiles** — One-click presets:
  - **Lightweight** (128 MB, 10K entries) for low-RAM systems
  - **Balanced** (512 MB, 50K entries) for most desktops
  - **Performance** (2 GB, 100K entries, ARC) for power users
- **Fine-tune** budget, max entries, hash algorithm (XXH3/SHA-256), eviction policy (LRU/LFU/ARC/CLOCK)
- **Manage** cache location, clear/rebuild cache, memory-pressure-aware mode

### Settings

- **Thumbnail rendering** — Default size (128-1024 px), GPU backend (Auto/DX12/DX11/Vulkan/CPU), quality mode (Speed/Balanced/Quality)
- **Cache** — Budget, hot-mode toggle
- **Appearance** — Theme (System/Dark/Light)
- **Action bar** — Reset to Defaults, Import Config, Save & Restart Explorer, Save

### GPU

- **GPU identity** — Name, vendor, VRAM, DirectX feature level, active backend
- **Backend waterfall** — Visual chain showing which GPU backends were tried and which succeeded
- **Hardware video decode** — NVDEC, Intel QuickSync, AMD AMF availability

### Plugins

- **Browse Marketplace** — Discover third-party decoders and thumbnail providers
- **Install from file** — Load `.lenspkg` plugin files
- **Check for updates** — Update all installed plugins
- **Trust & enable** — Per-plugin toggle with trust level display

### Diagnostics

- **Log viewer** — Engine log with level filter (All/Info/Warning/Error), auto-scroll, monospaced font
- **ETW trace** — Start/stop Event Tracing for Windows sessions
- **Live counters** — TPS, decode latency, active threads, GPU memory, memory pressure
- **Export** — Diagnostic bundle (.zip), copy system info, open log folder

### About

- **Version** — v40.0.1 (Procyon) with build info
- **Quick stats** — 200+ formats, 25+ decoders, <17ms per thumbnail
- **System info** — GPU, Windows version, COM CLSID, test pass rate
- **Links** — GitHub, docs, bug reports, changelog
- **Copy System Info** — One-click copy for bug reports

### First-Run Onboarding

New installations show a 3-step guided setup:

1. **Welcome** — Feature highlights and version info
1. **Setup** — Shell registration, GPU backend selection, telemetry preference
1. **Completion** — Summary of configured settings, quick tip, and links to Manager

---

## Command-Line Interface

The `lens` CLI provides headless thumbnail generation for scripting and automation:

```powershell
# Generate a single thumbnail
lens generate photo.cr2 --size 512 --quality

# Batch-generate for an entire folder
lens batch ./photos --recursive --threads 8

# Show file metadata and decoder info
lens info document.pdf

# List supported formats (optionally by category)
lens formats --category raw

# Cache management
lens cache stats
lens cache clear

# Version info
lens version

# Detailed help for any command
lens help generate
```

**Exit codes:** 0 (success), 1 (error), 2 (invalid args), 3 (unknown format), 4 (license), 5 (I/O error),
6 (partial batch failure).

1. **Launch LENSManager**:

- Search for "LENSManager" in Start Menu
- Or run: `C:\Program Files\ExplorerLens\LENSManager.exe`

1. **Configuration Options**:

- **Enable/Disable** thumbnail handler
- **Set thumbnail quality** (Fast, Balanced, High Quality)
- **Configure cache settings**
- **Choose GPU** (if multiple GPUs available)
- **View statistics** (cache hits, decode times)
- **Force cache clear**

1. **Quick Actions**:

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
- **RAW Photos:** `.cr2`, `.cr3`, `.nef`, `.arw`, `.orf`, `.dng`, `.rw2`, `.raf`, `.pef`,
  `.dcr`, `.mrw`, `.x3f`, `.gpr` + 100 more camera formats

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

1. **Clear thumbnail cache**:

 ```powershell
 .\LENSManager.exe /ClearCache
 ```

1. **Re-register handler**:

 ```powershell
 regsvr32 /i "C:\Program Files\ExplorerLens\LENSShell.dll"
 ```

1. **Check Windows thumbnail settings**:

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

1. **Enable debug logging**:

 ```powershell
 Set-ItemProperty -Path "HKLM:\Software\ExplorerLens" -Name "DebugLog" -Value 1
 # Logs written to: %TEMP%\ExplorerLens-Debug.log
 ```

1. **Report issues** on GitHub with:

- Windows version
- File type causing crash
- Debug log excerpt

---

## Uninstallation

### Via Control Panel

1. Open **Settings** → **Apps** → **Installed apps**
1. Find **ExplorerLens**
1. Click **Uninstall**

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
A: No, ExplorerLens _extends_ Windows thumbnail support. Native formats (JPG, PNG) still use Windows providers.

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

- **GitHub Repository:** <https://github.com/RajwanYair/ExplorerLens.io>
- **Issue Tracker:** <https://github.com/RajwanYair/ExplorerLens.io/issues>
- **Developer Guide:** [DEVELOPER_GUIDE.md](development/DEVELOPER_GUIDE.md)
- **Build Instructions:** [.github/standards/build-method.md](../.github/standards/build-method.md)
- **Plugin SDK:** [SDK/README.md](../SDK/README.md)
- **Roadmap v34 (Arcturus):** _(archived — removed in v39.9.0 production cleanup)_

**Support:** For issues, please open a GitHub issue with detailed information.

---

**Enjoy seamless thumbnail previews for all your files! 🎨**
