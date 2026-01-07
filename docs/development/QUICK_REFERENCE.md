# DarkThumbs v5.0 - Quick Reference Guide
**Updated:** November 19, 2025

---

## 🚀 Quick Start Commands

### First-Time Setup
```powershell
# 1. Download all external libraries (with Intel proxy)
cd build-scripts
.\download-all-libs.ps1 -ProxyUrl "http://proxy-chain.intel.com:911"

# 2. Build compression libraries
.\rebuild-compression-libs.ps1 -Clean

# 3. Build DarkThumbs
.\build.ps1 -Configuration Release

# 4. Install (as Administrator)
cd ..
.\install-x64.ps1
```

### Update Libraries to Latest
```powershell
cd build-scripts
.\update-all-libraries.ps1 -UseProxy
.\rebuild-compression-libs.ps1 -Clean
.\rebuild-all.ps1
```

### Complete Rebuild
```powershell
.\rebuild-all.ps1
```

### Quick Build (Incremental)
```powershell
cd build-scripts
.\build.ps1
```

### Uninstall
```powershell
.\uninstall-x64.ps1
```

---

## 📋 Available Commands

### Build Scripts (PowerShell - PREFERRED)

| Script | Description | Usage |
|--------|-------------|-------|
| `build.ps1` | Build CBXShell.dll and CBXManager.exe | `.\build.ps1 [-Configuration Release/Debug] [-Clean] [-Rebuild] [-Verbose]` |
| `rebuild-all.ps1` | Complete rebuild of everything | `.\rebuild-all.ps1 [-SkipLibraries]` |
| `rebuild-compression-libs.ps1` | Rebuild all compression libraries | `.\rebuild-compression-libs.ps1 [-Clean] [-Verbose]` |
| `update-all-libraries.ps1` | Check and update all libraries | `.\update-all-libraries.ps1 [-UseProxy]` |
| `download-all-libs.ps1` | Download external libraries | `.\download-all-libs.ps1 [-ProxyUrl "http://..."]` |
| `install-x64.ps1` | Install shell extension | `.\install-x64.ps1 [-Force]` |
| `uninstall-x64.ps1` | Uninstall shell extension | `.\uninstall-x64.ps1` |

### Build Scripts (CMD - Legacy)

| Script | Description |
|--------|-------------|
| `build-all-sequential.cmd` | Sequential build of all components |
| `rebuild-all.cmd` | Complete rebuild (old version) |
| `rebuild-compression-libs.cmd` | Rebuild compression libraries (old) |
| `install-x64.cmd` | Install shell extension (old) |
| `uninstall-x64.cmd` | Uninstall shell extension (old) |

---

## 🔧 Common Tasks

### Test the Build
```powershell
# After building
cd tests
.\run-all-tests.ps1
```

### Check Current Sprint Status
```powershell
# View sprint documentation
code COMPLETE_PROJECT_STATUS.md
code SPRINT1_AND_2_COMPLETE.md
code PROJECT_ENHANCEMENT_SUMMARY.md
```

### Debug Build Issues
```powershell
# Verbose build
cd build-scripts
.\build.ps1 -Verbose

# Check for errors
Get-Content ..\build\*.log
```

### Clean Everything
```powershell
# Remove all build outputs
Remove-Item -Recurse -Force build, CBXShell\x64, CBXManager\x64
```

---

## 📦 Library Versions

### Current (Sprint 1 & 2)
- zlib: 1.3.1
- bzip2: 1.0.8
- zstd: 1.5.6
- lz4: 1.10.0
- lzma SDK: 24.07
- minizip-ng: 4.0.7
- unrar: 7.2.1
- libwebp: 1.4.0

### Latest Available
- zstd: **1.5.7** ⬆️ UPDATE AVAILABLE
- lzma SDK: **24.08** ⬆️ UPDATE AVAILABLE
- unrar: **7.2.2** ⬆️ UPDATE AVAILABLE
- libwebp: **1.5.0** ⬆️ UPDATE AVAILABLE

---

## 🎯 Sprint Status

| Sprint | Feature | Status | Command to Enable |
|--------|---------|--------|-------------------|
| 1 | WebP Support | ✅ COMPLETE | Already active |
| 2 | HEIF/HEIC Support | ✅ COMPLETE | Already active |
| 3 | Performance (Cache, MT) | 🔧 Partial | Code exists, needs integration |
| 4 | PDF & DjVu | 📋 Planned | Not started |
| 5 | CHM & ODT | 📋 Planned | Not started |
| 6 | Library Updates | ⏳ Ready | Run `update-all-libraries.ps1` |
| 7 | Testing & Release | ⏳ Ready | Run test suite |

---

## 🐛 Troubleshooting

### Build Error: LNK1257 (LTCG)
**Solution:** Already fixed! LTCG disabled in CBXShell.vcxproj

### Build Error: Missing Libraries
```powershell
# Re-download libraries
cd build-scripts
.\download-all-libs.ps1 -ProxyUrl "http://proxy-chain.intel.com:911"
.\rebuild-compression-libs.ps1 -Clean
```

### Installation Fails
```powershell
# Make sure to run as Administrator
Right-click PowerShell -> Run as Administrator
cd path\to\DarkThumbs
.\install-x64.ps1 -Force
```

### Thumbnails Not Appearing
1. Check if DLL is registered: `reg query "HKLM\SOFTWARE\Classes\CLSID\{7D8A33DE-5C89-4508-A228-C05F4DCE0C99}"`
2. Restart Explorer: `Stop-Process -Name explorer -Force; Start-Process explorer`
3. Clear thumbnail cache: `Remove-Item "$env:LOCALAPPDATA\Microsoft\Windows\Explorer\thumbcache_*.db"`

### Proxy Issues
```powershell
# Test proxy connectivity
Test-NetConnection proxy-chain.intel.com -Port 911

# Use alternative proxy
.\download-all-libs.ps1 -ProxyUrl "http://proxy-dmz.intel.com:912"
```

---

## 📊 File Sizes (Typical)

- `CBXShell.dll`: ~1.27 MB (with all compression + WebP + HEIF)
- `CBXManager.exe`: ~156 KB
- Total project size: ~500 MB (including all source libraries)
- Build output: ~1.5 MB

---

## 🔐 Security Notes

**Build Configuration:**
- Control Flow Guard (CFG): ✅ Enabled
- ASLR: ✅ Enabled
- DEP: ✅ Enabled
- Spectre Mitigation: ✅ Enabled
- Static Runtime: ✅ /MT (no external dependencies)

---

## 📞 Support

**Documentation:**
- `README.md` - Main project documentation
- `COMPLETE_PROJECT_STATUS.md` - Detailed status
- `PROJECT_ENHANCEMENT_SUMMARY.md` - Latest enhancements
- `SPRINT1_AND_2_COMPLETE.md` - Sprint completion details

**Logs:**
- Build logs: `build/*.log`
- Test logs: `tests/*.log`
- Installation: Check Event Viewer (Application logs)

---

## ⚡ Performance Tips

1. **Use PowerShell scripts** - Better error handling and logging
2. **Enable multi-core builds** - Already enabled in new scripts
3. **Use SSD for build** - Significantly faster compilation
4. **Skip library rebuild** - Use `.\rebuild-all.ps1 -SkipLibraries` for faster iterations
5. **Incremental builds** - Use `.\build.ps1` instead of `rebuild-all.ps1`

---

## 🎨 Supported Formats

**Archives:**
- Comic Books: `.cbz`, `.cbr`, `.cb7`, `.cbt`
- Generic: `.zip`, `.rar`, `.7z`, `.tar`
- E-books: `.epub`, `.mobi`, `.azw`, `.azw3`, `.fb2`
- Photos: `.phz`

**Images (inside archives):**
- Modern: `.webp` ✅, `.heic/.heif` ✅, `.jxl` 🔧
- Legacy: `.jpg`, `.png`, `.bmp`, `.gif`, `.tif`

---

**Last Updated:** November 19, 2025  
**Version:** 5.0.0  
**Build Status:** ✅ Ready for Production
