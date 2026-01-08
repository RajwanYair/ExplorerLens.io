# DarkThumbs - GPU-Accelerated Thumbnail Generator

**High-Performance Windows Shell Extension for 31+ File Formats**

DarkThumbs generates thumbnails for comic books, archives, modern images, RAW photos, and videos using **DirectX 11 GPU acceleration**.

![Windows 11](https://img.shields.io/badge/Windows-11-blue)
![Platform](https://img.shields.io/badge/Platform-x64-green)
![C++20](https://img.shields.io/badge/C++-20-orange)
![Version](https://img.shields.io/badge/Version-5.2.0-brightgreen)
![License](https://img.shields.io/badge/License-MIT-yellow)

---

## 📚 Documentation

**Getting Started:**

- [Build Guide](docs/BUILD_GUIDE.md) - Complete build instructions
- [Installation Testing Guide](docs/INSTALLATION_TESTING_GUIDE.md) - Installation and testing procedures
- [Quick Setup](docs/QUICK_SETUP.md) - Fast start guide
- **[Current Status](READY_FOR_TESTING.md)** - ✅ Ready for installation testing

**Development:**

- [ROADMAP](ROADMAP.md) - Development roadmap and milestones
- [Contributing](.github/CONTRIBUTING.md) - How to contribute
- [Project Structure](docs/PROJECT_STRUCTURE.md) - Architecture overview
- [Build Scripts Reference](docs/BUILD_SCRIPTS_REFERENCE.md) - Build automation reference

---

## ✨ Features

### Supported Formats (31+)

- **Comic Books:** `.cbz`, `.cbr`, `.cb7`, `.cbt`
- **E-Books:** `.epub`, `.mobi`, `.azw`, `.azw3`, `.fb2`
- **Archives:** `.zip`, `.rar`, `.7z`, `.tar`
- **Modern Images:** `.webp`, `.heif`, `.heic`, `.avif`, `.tif`, `.svg`, `.dng`
- **RAW Photos:** `.cr2`, `.cr3`, `.nef`, `.arw`, `.orf`
- **Videos:** `.mp4`, `.avi`, `.mkv`, `.mov`, `.wmv`, `.flv`, `.webm`

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

See [docs/BUILD_GUIDE.md](docs/BUILD_GUIDE.md) for detailed instructions.

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
├── ROADMAP.md            # Development roadmap
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

See [ROADMAP.md](ROADMAP.md) for the complete development plan.

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

See [docs/BUILD_GUIDE.md](docs/BUILD_GUIDE.md) for more troubleshooting.

---

## 📦 External Libraries

**Compression:** zlib 1.3.1, LZ4 1.10.0, zstd 1.5.7, LZMA 24.08, minizip-ng 4.0.10  
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
