# DarkThumbs - GPU-Accelerated Thumbnail Generator

**High-Performance Windows Shell Extension for 155+ File Formats**

DarkThumbs generates thumbnails for comic books, archives, modern images, RAW photos, and videos using **DirectX 11 GPU acceleration** and **AVX2 SIMD-optimized processing**.

![Windows 11](https://img.shields.io/badge/Windows-11-blue)
![Platform](https://img.shields.io/badge/Platform-x64-green)
![C++20](https://img.shields.io/badge/C++-20-orange)
![Version](https://img.shields.io/badge/Version-6.2.0-brightgreen)
![License](https://img.shields.io/badge/License-MIT-yellow)

---

## 📚 Documentation

**Getting Started:**

- [Build Guide](docs/build/BUILD_GUIDE.md) - Complete build instructions
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

### Supported Formats (155+ formats)

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
- **RAW Photos:** `.cr2`, `.cr3`, `.nef`, `.arw`, `.orf`, `.dng`, `.rw2`, `.raf`, `.pef`, `.dcr`, `.mrw`, `.x3f` and 100+ more camera formats (✅ LibRaw 0.21.2)
  - **Features:** Embedded JPEG thumbnail extraction (< 10ms), full RAW decode with demosaicing, EXIF orientation support, auto white balance
  - **Cameras:** Canon, Nikon, Sony, Olympus, Panasonic, Fujifilm, Pentax, Adobe DNG, Leica, Samsung, Hasselblad, Phase One, Sigma
- **Modern Images:** `.jxl` (JPEG XL) via libjxl 0.11.1, `.heif`/`.heic` (HEIF/HEIC) via WIC with hardware acceleration
  - **JXL Features:** Next-gen format support, better compression than WebP, wide color gamut
  - **HEIF Features:** Apple iPhone photos (iOS 11+), HDR support, 16-bit depth, wide color
- **Design:** `.psd`, `.psb` (Photoshop) ⏳, `.svg` (vector graphics) ⏳
- **HDR:** `.exr` (OpenEXR), `.hdr` (Radiance RGBE)
- **Texture:** `.dds` (DirectX textures)
- **Legacy:** `.tga` (Targa), `.ico` (icons), `.jp2` (JPEG2000)

#### Video Formats (✅ Via DirectShow)
- **Common:** `.mp4`, `.avi`, `.mkv`, `.mov`, `.wmv`, `.flv`, `.webm`

**Legend:**  
✅ = Fully implemented | 🔄 = In progress | 📋 = Planned

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

See [Build Guide](docs/build/BUILD_GUIDE.md) for detailed instructions.

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

**Current Version:** 5.2.0  
**Build System:** Active Recovery  
**Next Milestone:** v6.0.0 (July 2026)

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

See [Build Guide](docs/build/BUILD_GUIDE.md) for more troubleshooting.

---

## 📦 External Libraries

**Compression:** zlib 1.3.1, LZ4 1.10.0, zstd 1.5.7, LZMA 26.00, minizip-ng 4.0.10  
**Images:** libwebp 1.5.0, Windows WIC (HEIF/AVIF/RAW)  
**Archives:** UnRAR 7.2.1  
**Rendering:** DirectX 11

---

## 📄 License

MIT License - See [LICENSE](LICENSE) for details.

---

## 🔗 Links

- **Repository:** [GitHub](https://github.com/username/DarkThumbs) _(update with actual URL)_
- **Issues:** [GitHub Issues](https://github.com/username/DarkThumbs/issues)
- **Discussions:** [GitHub Discussions](https://github.com/username/DarkThumbs/discussions)

---

**Built with ❤️ using C++20 and DirectX 11**

_Last Updated: January 7, 2026_
