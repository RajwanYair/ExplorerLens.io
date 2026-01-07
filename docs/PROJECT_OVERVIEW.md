# DarkThumbs - Project Overview

**Version:** 4.6  
**Status:** Production Ready  
**Last Updated:** November 18, 2025

---

## 📋 What is DarkThumbs?

DarkThumbs is a high-performance Windows shell extension that generates thumbnails for archive files and e-books directly in Windows Explorer. It's optimized for Windows 11 with modern APIs, dark mode support, and comprehensive format support.

### Supported Formats (14 Total)

**Comic Book Archives:**
- `.cbz` (Comic Book ZIP)
- `.cbr` (Comic Book RAR)
- `.cb7` (Comic Book 7-Zip)
- `.cbt` (Comic Book TAR)

**E-Books:**
- `.epub` (Electronic Publication)
- `.mobi` (Mobipocket)
- `.azw` / `.azw3` (Kindle)
- `.fb2` (FictionBook 2.0)

**Generic Archives:**
- `.zip`, `.rar`, `.7z`, `.tar`

**Photo Archives:**
- `.phz` (Photo ZIP)

---

## 🏗️ Project Architecture

### Core Components

```
DarkThumbs/
├── CBXShell/          # Shell extension DLL (main component)
├── CBXManager/        # Registration/management GUI tool
├── external/          # Third-party compression libraries
├── build-scripts/     # Build automation scripts
├── tests/             # Test suite and sample files
└── docs/              # Documentation
```

### Compression Libraries Used

| Library | Version | Purpose | License |
|---------|---------|---------|---------|
| **zlib** | 1.3.1 | ZIP/Deflate compression | zlib License |
| **BZIP2** | 1.0.8 | BZ2 compression | BSD-like |
| **ZSTD** | 1.5.6 | Zstandard compression | BSD/GPLv2 |
| **LZ4** | 1.10.0 | Fast compression | BSD 2-Clause |
| **LZMA** | SDK 24.07 | 7-Zip compression | Public Domain |
| **UnRAR** | 7.2.1 | RAR decompression | UnRAR License |
| **minizip-ng** | 4.0.7 | ZIP archive handling | zlib License |

All libraries compiled with:
- AVX2 CPU optimizations
- Link-Time Code Generation (LTCG)
- Maximum optimization (/O2)

---

## ✨ Key Features

### Windows 11 Integration
- ✅ **Modern IThumbnailProvider Interface** - Native Windows 10/11 API
- ✅ **Dark Mode Support** - Automatic theme detection
- ✅ **Per-Monitor V2 DPI Awareness** - Sharp thumbnails on 4K displays
- ✅ **Async Thumbnail Generation** - Non-blocking Explorer integration
- ✅ **System Theme Integration** - Matches Windows appearance settings

### Security & Performance
- ✅ **Control Flow Guard (CFG)**
- ✅ **Address Space Layout Randomization (ASLR)**
- ✅ **Data Execution Prevention (DEP)**
- ✅ **Optimized Binary Size** - Minimal memory footprint
- ✅ **Static Runtime** - No external dependencies

### Modern C++ (C++17)
- `std::filesystem` for path operations
- `std::optional` for safe error handling
- Smart pointers (RAII) for resource management
- Modern threading with `std::thread`

---

## 🚀 Quick Start

### Prerequisites
- Windows 11 (or Windows 10 version 1809+)
- Administrator privileges for installation

### Installation
```cmd
REM Install the shell extension
install-x64.cmd

REM Uninstall if needed
uninstall-x64.cmd
```

### Building from Source

**Option 1: Microsoft Visual Studio Build Tools (Recommended)**
```cmd
setup-build-env.cmd      # One-time setup
rebuild-all.cmd          # Full clean build
```

**Option 2: Open-Source Toolchain (LLVM/Clang + CMake)**
```cmd
build-scripts\setup-quick.cmd    # One-time setup
build-scripts\build-quick.cmd    # Build project
```

---

## 📚 Documentation

### User Documentation
- **[INSTALLATION_GUIDE.md](INSTALLATION_GUIDE.md)** - Complete installation and build instructions
- **[README.md](README.md)** - Project overview and features

### Developer Documentation
- **[BUILD_REQUIREMENTS.md](BUILD_REQUIREMENTS.md)** - Build environment setup
- **[OPENSOURCE_BUILD.md](OPENSOURCE_BUILD.md)** - Building with open-source tools
- **[PROJECT_STATUS.md](PROJECT_STATUS.md)** - Current development status

### Technical Documentation
- **[CHANGELOG_MODERNIZATION.md](CHANGELOG_MODERNIZATION.md)** - Windows 11 modernization history
- **[TEST_SUITE_SUMMARY.md](TEST_SUITE_SUMMARY.md)** - Testing strategy and results
- **[COMPRESSION_UPGRADE_STATUS.md](COMPRESSION_UPGRADE_STATUS.md)** - Library versions and optimizations

---

## 🛠️ Development Status

### Completed ✅
- Windows 11 API modernization (IThumbnailProvider, IInitializeWithStream)
- Dark mode and high DPI support
- C++17 modernization
- 14 file format support
- Comprehensive test suite
- Build automation scripts
- Open-source toolchain support

### Future Enhancements 🔮
- PDF thumbnail support
- Office document thumbnails (DOCX, XLSX, PPTX)
- Animated thumbnail previews
- Cloud storage integration
- Performance monitoring and analytics

---

## 📄 License

**DarkThumbs Core:** MIT License

**Third-Party Libraries:**
- zlib, minizip-ng: zlib License
- BZIP2: BSD-like License
- ZSTD: BSD/GPLv2 dual license
- LZ4: BSD 2-Clause License
- LZMA SDK: Public Domain
- UnRAR: UnRAR License (free for non-commercial use)
- WTL: MIT License

See [LICENSE](../LICENSE) for full license text.

---

## 🤝 Contributing

This project welcomes contributions! Areas of interest:
- Additional file format support
- Performance optimizations
- Bug fixes and testing
- Documentation improvements

---

## 📞 Support

For issues, questions, or feature requests:
- Check existing documentation
- Review the [PROJECT_STATUS.md](PROJECT_STATUS.md) for known issues
- Submit detailed bug reports with sample files

---

**Last Build:** November 2025  
**Build Tools:** Visual Studio 2026 (v145), Windows 11 SDK 10.0.26100.0  
**Target Platform:** Windows 11 x64
