# DarkThumbs - Quick Start Guide
## Get Up and Running in 5 Minutes

**Last Updated:** November 18, 2025  
**Version:** 4.6 (Production Ready)

---

## 🎯 What is DarkThumbs?

DarkThumbs generates thumbnail previews for:
- **Comic Books:** CBZ, CBR, CB7, CBT
- **Ebooks:** EPUB, MOBI, AZW, FB2
- **Archives:** ZIP, RAR, 7Z, TAR
- **Photos:** PHZ

...directly in Windows 11 File Explorer!

---

## ⚡ Option 1: Use Pre-Built Binaries (5 minutes)

### Step 1: Download
Download the latest release from GitHub releases page.

### Step 2: Extract
Extract to a permanent location (e.g., `C:\Program Files\DarkThumbs\`)

### Step 3: Install
Right-click `install-x64.cmd` → **Run as Administrator**

### Step 4: Verify
1. Open File Explorer
2. Navigate to a folder with comic books or ebooks
3. Change view to **Large Icons**
4. Thumbnails appear automatically!

### Step 5: Configure (Optional)
Run `CBXManager.exe` from `x64\Release\` to:
- Enable/disable specific formats
- Configure display options
- Manage file associations

---

## 🛠️ Option 2: Build from Source (15 minutes)

### Prerequisites

**Install Visual Studio Build Tools:**
1. Download from: https://visualstudio.microsoft.com/downloads/
2. Install "Build Tools for Visual Studio 2022 or 2026"
3. Select:
   - ✅ Desktop development with C++
   - ✅ Windows 11 SDK
   - ✅ MSVC v143 or v145 toolset
   - ✅ ATL for latest build tools

### Quick Build

**1. Clone or download the repository**
```cmd
git clone https://github.com/yourusername/DarkThumbs.git
cd DarkThumbs
```

**2. Run the sequential build script**
```cmd
build-all-sequential.cmd
```

This automatically:
- Finds MSBuild on your system
- Builds all 7 compression libraries
- Builds CBXShell.dll
- Builds CBXManager.exe
- Shows detailed progress

**Duration:** 3-7 minutes (clean build)

**3. Install**
```cmd
install-x64.cmd
```
(Right-click → Run as Administrator)

**4. Test**
Open Windows Explorer, navigate to a comic book folder, switch to Large Icons view.

---

## 📋 Build Verification

### Check if build succeeded:

**Quick check:**
```cmd
dir CBXShell\x64\Release\CBXShell.dll
dir CBXManager\x64\Release\CBXManager.exe
```

**Full verification:**
```cmd
dir external\compression\zlib\x64\Release\*.lib
dir external\compression\bzip2\x64\Release\*.lib
dir external\compression\zstd\x64\Release\*.lib
dir external\compression\lz4\x64\Release\*.lib
dir external\compression\lzma\x64\Release\*.lib
dir external\compression\minizip-ng\x64\Release\*.lib
dir external\compression\unrar\x64\Release\*.lib
```

**Expected files:**
- ✅ zlibstatic.lib (~850 KB)
- ✅ bzip2.lib (~910 KB)
- ✅ zstd.lib (~3.1 MB)
- ✅ lz4.lib (~380 KB)
- ✅ lzma.lib (~500 KB)
- ✅ minizip-ng.lib (~1.2 MB)
- ✅ unrar.lib (~500 KB)
- ✅ **CBXShell.dll (~3 MB)**
- ✅ **CBXManager.exe (~200 KB)**

---

## 🔧 Troubleshooting

### Build Issues

**Error: MSBuild not found**
- Install Visual Studio Build Tools
- Or run from "Developer Command Prompt for VS 2022"

**Error: Library missing**
- Run `build-scripts\build-all-libs.cmd`
- Check output for specific library errors

**Error: LTCG mismatch (LNK1257)**
- Clean rebuild: Delete all `x64\Release` folders
- Run `build-all-sequential.cmd` again

**Build hangs or freezes**
- Use `build-all-sequential.cmd` instead of parallel builds
- Check Task Manager for stuck MSBuild processes

### Installation Issues

**Error: Must run as Administrator**
- Right-click `install-x64.cmd`
- Select "Run as Administrator"

**Thumbnails not showing**
- Restart Windows Explorer (or log off/log on)
- Check if DLL is registered: `regsvr32 /s CBXShell.dll`
- Clear thumbnail cache: `disk cleanup` → Thumbnails

**Specific format not working**
- Run `CBXManager.exe`
- Check if format is enabled
- Test with a known-good file

---

## 🎨 Configuration

### CBXManager Settings

Run `CBXManager.exe` to configure:

**Format Options:**
- Enable/disable CBZ, CBR, CB7, CBT
- Enable/disable EPUB, MOBI, AZW, FB2
- Enable/disable ZIP, RAR, 7Z, TAR
- Enable/disable PHZ

**Display Options:**
- Show/hide archive icon overlay
- Configure thumbnail size preference
- Set sorting behavior

**Advanced:**
- Clear thumbnail cache
- Reset to defaults
- View version information

---

## 📊 Supported Formats

### Comic Books (14 formats)
| Extension | Format | Status |
|-----------|--------|--------|
| .cbz | Comic Book ZIP | ✅ |
| .cbr | Comic Book RAR | ✅ |
| .cb7 | Comic Book 7-Zip | ✅ |
| .cbt | Comic Book TAR | ✅ |

### Ebooks (5 formats)
| Extension | Format | Status |
|-----------|--------|--------|
| .epub | Electronic Publication | ✅ |
| .mobi | Mobipocket | ✅ |
| .azw | Kindle AZW | ✅ |
| .azw3 | Kindle AZW3 | ✅ |
| .fb2 | FictionBook 2.0 | ✅ |

### Archives (4 formats)
| Extension | Format | Status |
|-----------|--------|--------|
| .zip | ZIP Archive | ✅ |
| .rar | RAR Archive | ✅ |
| .7z | 7-Zip Archive | ✅ |
| .tar | TAR Archive | ✅ |

### Photos (1 format)
| Extension | Format | Status |
|-----------|--------|--------|
| .phz | Photo ZIP | ✅ |

---

## 🚀 What's Coming in v5.0

### Modern Image Formats
- WebP (Google's format)
- AVIF (AV1 images)
- HEIC (iPhone photos)
- JPEG XL (next-gen JPEG)

### Enhanced Ebooks
- **PDF thumbnails** (most requested!)
- DjVu scanned documents
- CHM compiled help files

### Video Thumbnails
- MP4, MKV, AVI, MOV support
- GPU-accelerated decoding

### Performance
- DirectX GPU rendering (5-10x faster)
- Multi-file thumbnail preview
- Intelligent caching

See [ENHANCEMENT_ROADMAP_2025.md](docs/ENHANCEMENT_ROADMAP_2025.md) for details.

---

## 📚 Documentation

| Document | Description |
|----------|-------------|
| [README.md](README.md) | Main project overview |
| [BUILD_SCRIPTS_REFERENCE.md](docs/BUILD_SCRIPTS_REFERENCE.md) | Complete build system guide |
| [ENHANCEMENT_ROADMAP_2025.md](docs/ENHANCEMENT_ROADMAP_2025.md) | Future features roadmap |
| [INSTALLATION_GUIDE.md](docs/INSTALLATION_GUIDE.md) | Detailed installation |
| [PROJECT_STATUS.md](docs/PROJECT_STATUS.md) | Current status |
| [COMPLETE_OPTIMIZATION_SUMMARY.md](docs/COMPLETE_OPTIMIZATION_SUMMARY.md) | Recent improvements |

---

## 💡 Tips & Tricks

### Performance Tips
1. **Clear old thumbnails:** Disk Cleanup → Thumbnails
2. **Increase cache size:** Adjust Windows thumbnail cache
3. **Use SSD:** Faster archive reading

### Best Practices
1. **Extract large archives:** Better than reading directly
2. **Organize by format:** Separate folders for comics, ebooks
3. **Use CBXManager:** Configure only formats you need

### Known Issues
1. **Very large files (>1GB):** May be slow
2. **Encrypted archives:** Thumbnails not supported
3. **Corrupted files:** May crash Explorer (rare)

---

## 🆘 Getting Help

### Before Asking for Help
1. Check this guide's Troubleshooting section
2. Read [BUILD_SCRIPTS_REFERENCE.md](docs/BUILD_SCRIPTS_REFERENCE.md)
3. Search existing GitHub Issues

### Reporting Issues
Include:
- Windows version
- File format and size
- Error message (if any)
- Event Viewer logs (if Explorer crash)
- Steps to reproduce

### Feature Requests
- Check [ENHANCEMENT_ROADMAP_2025.md](docs/ENHANCEMENT_ROADMAP_2025.md) first
- Open GitHub Issue with "Feature Request" label
- Describe use case and benefits

---

## 🎓 Advanced Usage

### Build Individual Components
```cmd
cd build-scripts

REM Build only compression libraries
build-all-libs.cmd

REM Build only the DLL
build-cbxshell.cmd

REM Build only the manager
build-cbxmanager.cmd

REM Build specific library
build-zstd.cmd
```

### Custom Build Configurations
Edit `.vcxproj` files to customize:
- Optimization levels
- CPU instruction sets
- Compression method support
- Debug symbols

### Development Workflow
1. Make code changes
2. Build affected component only
3. Test manually
4. Run full build before commit

---

## 📄 License

**DarkThumbs:** MIT License

**Third-Party Libraries:**
- zlib (zlib License)
- bzip2 (BSD-like)
- zstd (BSD 2-Clause)
- lz4 (BSD 2-Clause)
- LZMA SDK (Public Domain)
- minizip-ng (zlib License)
- UnRAR (Freeware)
- WTL (MS-PL)

All components are open-source and commercially usable.

---

## ✅ Next Steps After Installation

1. ✅ Test with sample files
2. ✅ Configure CBXManager settings
3. ✅ Set Windows Explorer view to Large Icons
4. ✅ Star the GitHub repo if you like it!
5. ✅ Report any issues
6. ✅ Spread the word!

---

**Enjoy fast, beautiful thumbnails for all your archives!** 📚🎨

**Questions?** Check the documentation or open a GitHub issue.

**Want to contribute?** See [ENHANCEMENT_ROADMAP_2025.md](docs/ENHANCEMENT_ROADMAP_2025.md) for opportunities!
