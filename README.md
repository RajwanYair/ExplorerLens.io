# DarkThumbs - GPU-Accelerated Thumbnail Generator

**High-Performance Windows Shell Extension for 200+ File Formats**

DarkThumbs generates thumbnails for images, videos, documents, 3D models, fonts, archives, and more using **DirectX 11 GPU acceleration** and **multi-threaded processing**. Version 7.0 adds video thumbnails, PDF support, SVG rendering, QOI format, and comprehensive multimedia format coverage.

![Windows 11](https://img.shields.io/badge/Windows-11-blue)
![Platform](https://img.shields.io/badge/Platform-x64-green)
![C++20](https://img.shields.io/badge/C++-20-orange)
![Version](https://img.shields.io/badge/Version-7.0.0-brightgreen)
![License](https://img.shields.io/badge/License-MIT-yellow)

---

## 📚 Documentation

**Getting Started:**

- [Build Quick Reference](docs/development/BUILD_QUICK_REFERENCE.md) - Complete build instructions
- [Installation Guide](docs/getting-started/installation.md) - Installation and setup
- [Testing Guide](docs/testing/TESTING_GUIDE.md) - Validation and test procedures

**Project Organization:**

- [Project Structure](docs/architecture/PROJECT_STRUCTURE.md) - Complete directory organization
- [Master Plan](MASTER_PLAN.md) - Unified roadmap and execution plan
- [Documentation Index](docs/INDEX.md) - Complete documentation index

**Development:**

- [Development Guide](docs/development/README.md) - Developer documentation
- [Contributing](.github/CONTRIBUTING.md) - How to contribute
- [Coding Standards](.github/standards/CODING_STANDARDS.md) - Code style and conventions
- [Build Scripts](build-scripts/README.md) - Build automation reference

---

## ✨ Features

### Supported Formats (200+ file extensions via 24 specialized decoders)

#### Core Image Formats (✅ Fully Supported)
- **Standard:** `.jpg`, `.jpeg`, `.png`, `.bmp`, `.gif`, `.tiff`, `.tif`
- **Modern:** `.webp` (WebP), `.avif` (AV1 Image), `.jxl` (JPEG XL) ✅
- **Mobile:** `.heif`, `.heic`, `.hif`, `.avci`, `.avcs` (Apple HEIC/HEIF) ✅
- **Implementation:** JXL via libjxl 0.11.1 | HEIF via Windows WIC (hardware-accelerated)

#### Archives & Comic Books (✅ Fully Supported)
- **Comic Books:** `.cbz`, `.cbr`, `.cb7`, `.cbt`
- **E-Books:** `.epub`, `.mobi`, `.azw`, `.azw3`, `.fb2`
- **Archives:** `.zip`, `.rar`, `.7z`, `.tar`, `.gz`, `.bz2`, `.xz`

#### Professional Formats (✅ RAW Photos & Modern Formats)
- **RAW Photos:** `.cr2`, `.cr3`, `.nef`, `.arw`, `.orf`, `.dng`, `.rw2`, `.raf`, `.pef`, `.dcr`, `.mrw`, `.x3f` and 100+ more camera formats (✅ LibRaw 0.21.3)
  - **Features:** Embedded JPEG thumbnail extraction (< 10ms), full RAW decode with demosaicing, EXIF orientation support, auto white balance
  - **Cameras:** Canon, Nikon, Sony, Olympus, Panasonic, Fujifilm, Pentax, Adobe DNG, Leica, Samsung, Hasselblad, Phase One, Sigma
- **Modern Images:** `.jxl` (JPEG XL) via libjxl 0.11.1, `.heif`/`.heic` (HEIF/HEIC) via WIC with hardware acceleration
  - **JXL Features:** Next-gen format support, better compression than WebP, wide color gamut
  - **HEIF Features:** Apple iPhone photos (iOS 11+), HDR support, 16-bit depth, wide color
- **Design:** `.psd`, `.psb` (Photoshop), `.svg` (vector graphics)
- **HDR:** `.exr` (OpenEXR), `.hdr` (Radiance RGBE)
- **Texture:** `.dds` (DirectX textures)
- **Legacy:** `.tga` (Targa), `.ico` (icons), `.jp2` (JPEG2000)

#### Video & Audio (✅ Media Foundation)
- **Video:** `.mp4`, `.mkv`, `.avi`, `.webm`, `.mov`, `.wmv`, `.flv`, `.mpg`, `.mpeg`, `.ts`, `.mts`, `.m2ts`, `.3gp`, `.vob`, `.ogv` (22 extensions)
- **Audio:** `.mp3`, `.flac`, `.m4a`, `.ogg`, `.wma`, `.wav`, `.opus` - extracts album art or generates waveform

#### Documents & Fonts (✅ Shell API + GDI+)
- **Documents:** `.pdf`, `.docx`, `.xlsx`, `.pptx`, `.epub` (Office/Edge required for Office formats)
- **Fonts:** `.ttf`, `.otf`, `.ttc` - renders font preview

#### 3D Models (✅ Built-in parser)
- **Models:** `.obj`, `.stl`, `.gltf`, `.glb` - orthographic preview rendering

#### Vector Graphics (✅ GDI+ renderer)
- **SVG:** `.svg`, `.svgz` - rasterizes vector graphics to thumbnail

#### Special Formats (✅ Native decoders)
- **QOI:** `.qoi` (Quite OK Image - fastest decode format)
- **ICO:** `.ico`, `.cur` (Windows icons/cursors)
- **TGA:** `.tga` (Targa)
- **PPM:** `.ppm`, `.pgm`, `.pbm`, `.pnm`, `.pam`, `.pfm` (Netpbm formats)
- **DDS:** `.dds` (DirectX textures)
- **HDR:** `.hdr`, `.pic` (Radiance RGBE)

**Legend:**  
✅ = Production ready | ⚠️ = Requires external dependency | 📋 = Planned

### Performance

- ⚡ **GPU Accelerated** - DirectX 11 hardware rendering
- 🚀 **Smart Caching** - Instant display of cached thumbnails
- 📦 **Multi-Format** - Single extension for all formats
- 🎯 **Low Memory** - Efficient resource usage

---

## 🚀 Quick Start

### Requirements

- Windows 10 1809+ or Windows 11 (64-bit)
- Visual Studio 2022/2026 BuildTools
- Administrator privileges for installation

### Build

```cmd
REM Open "x64 Native Tools Command Prompt for VS 2022/2026"
cd DarkThumbs
RUN-BUILD.bat
```

See [Build Quick Reference](docs/development/BUILD_QUICK_REFERENCE.md) for detailed instructions.

### Install

```cmd
REM Register DLL (as Administrator)
cd x64\Release
regsvr32 CBXShell.dll
```

### Configure

Run `CBXManager.exe` to enable/disable file format categories.

---

## 🏗️ Project Structure

```
DarkThumbs/
├── CBXShell/              # Main COM shell extension
├── CBXManager/            # Configuration GUI tool
├── build-scripts/         # Build automation
├── docs/                  # Documentation
├── external/              # Third-party libraries
├── .github/               # GitHub workflows and templates
├── CBXShell.sln          # Visual Studio solution
├── LICENSE               # MIT License
├── MASTER_PLAN.md        # Unified roadmap and execution plan
└── RUN-BUILD.bat         # Quick build script
```

---

## 🤝 Contributing

We welcome contributions! See [CONTRIBUTING.md](.github/CONTRIBUTING.md) for guidelines.

### How to Help

- 🐛 Report bugs and issues
- 💡 Suggest features
- 🔧 Submit pull requests
- 📝 Improve documentation
- 🧪 Write tests

---

## 📊 Status

**Current Version:** 7.0.0  
**Build Status:** 0 errors / 0 warnings  
**Test Status:** 100/100 unit tests, 5/5 benchmarks (100% pass rate)  
**Next Milestone:** v7.1 — Plugin system activation, ETW structured logging, context menu integration, animated format support

See [MASTER_PLAN.md](MASTER_PLAN.md) for the complete development plan.

---

## 🔧 Troubleshooting

### Thumbnails Not Appearing

```cmd
REM Restart Windows Explorer
taskkill /f /im explorer.exe && start explorer.exe

REM Clear Windows thumbnail cache
del /f /s /q "%LocalAppData%\Microsoft\Windows\Explorer\thumbcache_*.db"
```

### Check DLL Registration

```cmd
reg query "HKCR\CLSID\{9E6ECB90-5A61-42BD-B851-D3297D9C7F39}\InprocServer32"
```

See [Troubleshooting Guide](docs/TROUBLESHOOTING.md) for more troubleshooting.

---

## 📦 External Libraries

**Compression:** zlib 1.3.1, LZ4 1.10.0, zstd 1.5.7, LZMA 26.00, minizip-ng 4.0.10  
**Images:** libwebp 1.5.0, libavif 1.3.0 (dav1d 1.5.1), libjxl 0.11.1, LibRaw 0.21.3  
**Archives:** UnRAR 7.2.1, minizip-ng 4.0.10  
**Video:** Windows Media Foundation (system API)  
**Rendering:** DirectX 11, GDI+

---

## 📄 License

MIT License - See [LICENSE](LICENSE) for details.

---

## 🔗 Links

- **Repository:** Internal project repository
- **Issues:** Use project issue tracker configured for this workspace
- **Discussions:** Use team discussion channel

---

**Built with ❤️ using C++20 and DirectX 11**

_Last Updated: February 16, 2026_
