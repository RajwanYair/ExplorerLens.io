# ExplorerLens.py — Cross-Platform Python Thumbnail Provider

**Version:** 23.6.0
**Based on:** ExplorerLens.io v23.6.0 "Vega-W"
**Language:** Python 3.11+
**Platforms:** Windows (COM IThumbnailProvider), Linux (freedesktop.org)

A pure-Python cross-platform thumbnail provider supporting 200+ file formats.
Mirrors the full functionality of ExplorerLens.io with Python-native decoders.

## Features

- **200+ file formats** across archives, comics, images, video, audio, documents, fonts, 3D models
- **Windows Shell integration** via COM IThumbnailProvider (pythoncom)
- **Linux integration** via freedesktop.org thumbnailer spec (XDG)
- **Modern GUI manager** with dark mode, per-format toggles, performance dashboard
- **Admin elevation** for COM registration/unregistration
- **Multi-tier caching** — memory + disk with LRU eviction
- **GPU acceleration** — optional Pillow-SIMD / CUDA via CuPy
- **Plugin system** — drop-in Python decoder plugins
- **Archive collage** — multi-image preview for CBZ/CBR/ZIP/RAR archives
- **Config import/export** — JSON-based settings with registry backup

## Supported File Types

### Archives & Comics
CBZ, CBR, CB7, CBT, ZIP, RAR, 7Z, TAR, TAR.GZ, TAR.BZ2, TAR.XZ, TAR.ZST, EPUB, MOBI, FB2

### Images
WEBP, AVIF, HEIC/HEIF, JPEG XL, TIFF, SVG, RAW (CR2/NEF/ARW/DNG/...), PSD, DDS, HDR, EXR,
PPM, ICO, QOI, TGA, BMP, GIF, PNG, JPEG, JP2, EPS, PCX, SGI, XPM, XCF, ORA, JXR, KTX, VTF

### Video
AVI, WMV, ASF, MPG, MPEG, M1V, M2V, TS, M2TS, MTS, M2T, MP4, M4V, MP4V, MOV, 3G2, 3GP,
3GP2, 3GPP, MKV, MK3D, WEBM, FLV, F4V, OGM, OGV, RM, RMVB, DV, MXF, IVF, EVO, 264, VIDEO

### Audio
MP3, WAV, M4A, APE, FLAC, OGG, MKA, MPC, OPUS, TAK, WV, WMA, AAC

### Documents
PDF, DJVU, CHM, DOCX, PPTX, XLSX, DOC, PPT, XLS, ODT, ODP

### Fonts
TTF, OTF, WOFF, WOFF2

### 3D Models
OBJ, STL, PLY, GLTF, GLB, FBX, 3DS

## Requirements

```
Python >= 3.11
pip install -r requirements.txt
```

## Quick Start

```powershell
# Install dependencies
pip install -r requirements.txt

# Launch GUI (auto-elevates if needed)
python -m explorerlens

# Register shell extension (requires admin)
python -m explorerlens --register

# Unregister shell extension
python -m explorerlens --unregister

# Generate thumbnail from CLI
python -m explorerlens --thumbnail "C:\photo.jpg" --size 256 --output thumb.png

# Run benchmark on a folder
python -m explorerlens --benchmark "C:\Pictures"

# Export diagnostics report
python -m explorerlens --diagnostics report.json

# Run tests
python -m pytest tests/ -v
```

## Architecture

```
ExplorerLens.py/
├── explorerlens/          # Main package
│   ├── __init__.py
│   ├── __main__.py        # CLI entry point (--register/--thumbnail/--benchmark/--diagnostics)
│   ├── engine.py          # Core thumbnail engine with TieredCache auto-init
│   ├── config.py          # JSON-based configuration management
│   ├── registry.py        # Windows registry manager (backup/restore/conflict detection)
│   ├── decoders/          # Format-specific decoders (7 total)
│   │   ├── image_decoder.py   # Raster/SVG/RAW/HDR/WebP/AVIF/HEIC/JXL
│   │   ├── video_decoder.py   # 34+ video formats via ffmpeg
│   │   ├── audio_decoder.py   # Album art + waveform + WMA support
│   │   ├── archive_decoder.py # ZIP/RAR/7Z/TAR/EPUB/Comic collage
│   │   ├── document_decoder.py# PDF/DJVU/DOCX/PPTX/XLSX
│   │   ├── font_decoder.py    # TTF/OTF/WOFF preview rendering
│   │   └── model_decoder.py   # OBJ/STL/PLY/GLTF/GLB/FBX via trimesh
│   ├── cache/             # Multi-tier caching
│   │   ├── memory_cache.py    # L1 LRU (OrderedDict, thread-safe)
│   │   ├── disk_cache.py      # L2 SQLite WAL persistent cache
│   │   └── tiered_cache.py    # L1+L2 wrapper with auto-promotion
│   ├── shell/             # Platform shell integration
│   │   ├── com_server.py          # Windows COM IThumbnailProvider
│   │   ├── linux_thumbnailer.py   # Linux freedesktop.org thumbnailer
│   │   ├── platform_provider.py   # Cross-platform abstraction layer
│   │   └── diagnostics.py         # System/dependency/config diagnostics
│   ├── gui/               # Tkinter GUI manager
│   │   └── app.py             # 4-tab GUI + system tray icon
│   ├── plugins/           # Plugin system (drop-in .py decoders)
│   └── utils/             # Utilities (benchmark, elevation)
├── tests/                 # pytest unit tests (~40 tests)
├── requirements.txt       # Full dependency list
├── setup.py               # Package setup with extras_require
└── README.md
```

## License

MIT — see LICENSE file.
