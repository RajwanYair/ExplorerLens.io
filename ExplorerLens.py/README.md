# ExplorerLens.py — Python Thumbnail Provider for Windows

**Version:** 1.0.0
**Based on:** ExplorerLens.io v15.0.0 "Zenith"
**Language:** Python 3.11+

A pure-Python Windows Shell Extension thumbnail provider supporting 200+ file formats.
Mirrors the full functionality of ExplorerLens.io with Python-native decoders.

## Features

- **200+ file formats** across archives, comics, images, video, audio, documents, fonts, 3D models
- **Windows Shell integration** via COM IThumbnailProvider (pythoncom)
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
```

## Architecture

```
ExplorerLens.py/
├── explorerlens/          # Main package
│   ├── __init__.py
│   ├── __main__.py        # CLI entry point
│   ├── engine.py          # Core thumbnail engine
│   ├── decoders/          # Format-specific decoders
│   ├── cache/             # Multi-tier caching
│   ├── gpu/               # GPU acceleration (optional)
│   ├── shell/             # COM shell extension
│   ├── gui/               # Tkinter GUI manager
│   ├── plugins/           # Plugin system
│   ├── registry.py        # Windows registry manager
│   ├── config.py          # Configuration management
│   └── utils/             # Utilities
├── plugins/               # User plugins directory
├── tests/                 # Unit tests
├── requirements.txt
├── setup.py
└── README.md
```

## License

MIT — see LICENSE file.
