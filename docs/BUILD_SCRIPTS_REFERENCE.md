# DarkThumbs Build Scripts Reference
## Complete Build System Documentation

**Last Updated:** November 18, 2025  
**Build System Version:** 2.0 (Modular)

---

## 📁 Build Script Organization

```
DarkThumbs/
├── build-scripts/           # Modular build scripts (NEW)
│   ├── find-msbuild.cmd          # Dynamic MSBuild finder
│   ├── build-zlib.cmd            # Build zlib 1.3.1
│   ├── build-bzip2.cmd           # Build bzip2 1.0.8
│   ├── build-zstd.cmd            # Build zstd 1.5.6
│   ├── build-lz4.cmd             # Build lz4 1.10.0
│   ├── build-lzma.cmd            # Build LZMA SDK
│   ├── build-minizip.cmd         # Build minizip-ng 4.0.7
│   ├── build-unrar.cmd           # Build UnRAR 7.2.1
│   ├── build-cbxshell.cmd        # Build CBXShell.dll
│   ├── build-cbxmanager.cmd      # Build CBXManager.exe
│   ├── build-all-libs.cmd        # Build all compression libs
│   ├── build-release-x64.cmd     # Legacy full build
│   └── verify-enhancements.cmd   # Verification script
│
├── build-all-sequential.cmd # NEW: Sequential build (recommended)
├── rebuild-all.cmd          # Build libraries + binaries
├── rebuild-compression-libs.cmd  # Build all libraries
├── rebuild-gui.cmd          # Build GUI only
├── install-x64.cmd          # Install shell extension
└── uninstall-x64.cmd        # Uninstall shell extension
```

---

## 🚀 Quick Start

### Option 1: Complete Sequential Build (Recommended)
```cmd
build-all-sequential.cmd
```
**What it does:**
1. Builds all 7 compression libraries in order
2. Builds CBXShell.dll with all compression support
3. Builds CBXManager.exe
4. Shows detailed progress and errors
5. No background processes - easy to debug

### Option 2: Build Individual Component
```cmd
cd build-scripts
build-cbxshell.cmd
```

### Option 3: Build Only Libraries
```cmd
cd build-scripts
build-all-libs.cmd
```

---

## 🔧 Build Scripts Details

### Core Build Scripts

#### `find-msbuild.cmd`
**Purpose:** Dynamically locate MSBuild.exe on any system

**Search Strategy:**
1. Use vswhere.exe (VS 2017+)
2. Check common VS 2022-2026 locations
3. Check PATH environment variable

**Output:** Sets `%MSBUILD%` environment variable

**Example Usage:**
```cmd
call build-scripts\find-msbuild.cmd
if %ERRORLEVEL% NEQ 0 exit /b 1
"%MSBUILD%" MyProject.vcxproj /t:Build
```

---

#### `build-zlib.cmd`
**Builds:** zlib 1.3.1 (DEFLATE compression)

**Output:** `external\compression\zlib\x64\Release\zlibstatic.lib`

**Size:** ~850 KB

**Optimizations:**
- Link-Time Code Generation (LTCG)
- AVX2 instructions
- Maximum speed (/O2)

---

#### `build-bzip2.cmd`
**Builds:** bzip2 1.0.8 (BZIP2 compression)

**Output:** `external\compression\bzip2\x64\Release\bzip2.lib`

**Size:** ~910 KB

**Optimizations:** Same as zlib

---

#### `build-zstd.cmd`
**Builds:** Zstandard 1.5.6

**Output:** `external\compression\zstd\x64\Release\zstd.lib`

**Size:** ~3.1 MB

**Note:** Built WITHOUT LTCG to avoid linker issues

---

#### `build-lz4.cmd`
**Builds:** LZ4 1.10.0 (ultra-fast compression)

**Output:** `external\compression\lz4\x64\Release\lz4.lib`

**Size:** ~380 KB

**Optimizations:** LTCG + AVX2

---

#### `build-lzma.cmd`
**Builds:** LZMA SDK (7-Zip)

**Output:** `external\compression\lzma\x64\Release\lzma.lib`

**Size:** ~500 KB (estimated)

**Note:** LZMA SDK API, not liblzma (XZ Utils)

---

#### `build-minizip.cmd`
**Builds:** minizip-ng 4.0.7 with compression support

**Output:** `external\compression\minizip-ng\x64\Release\minizip-ng.lib`

**Size:** ~1.2 MB

**Dependencies:** zlib, bzip2, zstd (checks before building)

**Preprocessor Defines:**
- `HAVE_ZLIB`
- `HAVE_BZIP2`
- `HAVE_ZSTD`

**Note:** LZMA support disabled (requires liblzma, not LZMA SDK)

---

#### `build-unrar.cmd`
**Builds:** UnRAR 7.2.1

**Output:** `external\compression\unrar\x64\Release\unrar.lib`

**Size:** ~500 KB

**License:** Freeware (source usage allowed)

---

#### `build-cbxshell.cmd`
**Builds:** Main shell extension DLL

**Output:** `CBXShell\x64\Release\CBXShell.dll`

**Size:** ~3 MB (with all compression libs)

**Dependencies:**
- All 7 compression libraries
- WTL 10.0.10320
- Windows SDK

**Features:**
- IThumbnailProvider interface
- Dark mode support
- DPI awareness
- All compression formats

---

#### `build-cbxmanager.cmd`
**Builds:** Configuration manager GUI

**Output:** `CBXManager\x64\Release\CBXManager.exe`

**Size:** ~200 KB

**Dependencies:**
- WTL 10.0.10320
- Windows SDK

---

### Master Build Scripts

#### `build-all-libs.cmd`
**Purpose:** Build all 7 compression libraries sequentially

**Features:**
- Dependency order enforcement
- Error tracking per library
- Build summary with success/failure count
- Stops on first error (optional)

**Duration:** ~2-5 minutes (clean build)

---

#### `build-all-sequential.cmd` (NEW - RECOMMENDED)
**Purpose:** Complete project build from scratch

**Steps:**
1. Build all compression libraries
2. Build CBXShell.dll
3. Build CBXManager.exe
4. Show build summary

**Features:**
- No background processes
- Clear error messages
- Build progress tracking
- File size reporting

**Duration:** ~3-7 minutes (clean build)

---

## 📊 Build Verification

### Check Build Outputs
```cmd
dir external\compression\*\x64\Release\*.lib
dir CBXShell\x64\Release\CBXShell.dll
dir CBXManager\x64\Release\CBXManager.exe
```

### Expected Outputs (Sizes)
| File | Size | Status |
|------|------|--------|
| zlibstatic.lib | ~850 KB | ✅ |
| bzip2.lib | ~910 KB | ✅ |
| zstd.lib | ~3.1 MB | ✅ |
| lz4.lib | ~380 KB | ✅ |
| lzma.lib | ~500 KB | ✅ |
| minizip-ng.lib | ~1.2 MB | ✅ |
| unrar.lib | ~500 KB | ✅ |
| **CBXShell.dll** | **~3 MB** | ✅ |
| **CBXManager.exe** | **~200 KB** | ✅ |

---

## 🐛 Troubleshooting

### MSBuild Not Found
**Error:** `MSBuild not found!`

**Solutions:**
1. Install Visual Studio 2022/2026 Build Tools
2. Run from "Developer Command Prompt for VS"
3. Add MSBuild to PATH

**Download:** https://visualstudio.microsoft.com/downloads/

---

### LTCG Mismatch Error
**Error:** `LNK1257: code generation failed`

**Cause:** Mixing LTCG and non-LTCG libraries

**Solution:** Rebuild all libraries, or disable LTCG in CBXShell.vcxproj

---

### Missing Compression Library
**Error:** `[MISSING] xxx.lib`

**Solution:**
```cmd
cd build-scripts
build-xxx.cmd
```

Or rebuild all:
```cmd
build-all-libs.cmd
```

---

### Build Hangs/Freezes
**Cause:** Background MSBuild processes

**Solution:** Use sequential build scripts (no `/m` flag)

---

## 🔍 Advanced Usage

### Build Specific Library Only
```cmd
cd build-scripts
build-zstd.cmd
```

### Rebuild Everything from Scratch
```cmd
REM Clean all outputs
rmdir /s /q external\compression\zlib\x64
rmdir /s /q external\compression\bzip2\x64
rmdir /s /q external\compression\zstd\x64
rmdir /s /q external\compression\lz4\x64
rmdir /s /q external\compression\lzma\x64
rmdir /s /q external\compression\minizip-ng\x64
rmdir /s /q external\compression\unrar\x64
rmdir /s /q CBXShell\x64
rmdir /s /q CBXManager\x64

REM Rebuild
build-all-sequential.cmd
```

### Custom MSBuild Path
```cmd
set "MSBUILD=C:\Path\To\MSBuild.exe"
build-scripts\build-zlib.cmd
```

---

## 📦 Installation After Build

### Install Shell Extension
```cmd
install-x64.cmd
```
**Requires:** Administrator privileges

**What it does:**
1. Stops Windows Explorer
2. Registers CBXShell.dll
3. Configures file associations
4. Restarts Explorer

---

### Uninstall
```cmd
uninstall-x64.cmd
```
**Requires:** Administrator privileges

---

## 🗂️ Legacy Scripts (Deprecated)

| Script | Status | Replacement |
|--------|--------|-------------|
| `rebuild-all.cmd` | Legacy | `build-all-sequential.cmd` |
| `rebuild-compression-libs.cmd` | Legacy | `build-scripts\build-all-libs.cmd` |
| `build-compression-libs.cmd` | Partial | `build-scripts\build-all-libs.cmd` |
| `rebuild-gui.cmd` | OK | `build-scripts\build-cbxmanager.cmd` |

**Note:** Legacy scripts still work but use hardcoded paths in some cases.

---

## 📝 Best Practices

### For Development
1. Use `build-all-sequential.cmd` for complete builds
2. Use individual `build-xxx.cmd` for quick iterations
3. Verify outputs with file size checks
4. Test with `verify-enhancements.cmd`

### For Release Builds
1. Clean all intermediate files
2. Run `build-all-sequential.cmd`
3. Verify no errors in output
4. Test DLL registration manually
5. Run comprehensive tests

### For CI/CD
```cmd
@echo off
call build-all-sequential.cmd
if %ERRORLEVEL% NEQ 0 exit /b 1

REM Verify outputs exist
if not exist "CBXShell\x64\Release\CBXShell.dll" exit /b 1
if not exist "CBXManager\x64\Release\CBXManager.exe" exit /b 1

echo Build successful!
exit /b 0
```

---

## 🎯 Next Steps

After successful build:
1. Run `install-x64.cmd` (as Administrator)
2. Test thumbnails in Windows Explorer
3. Run `CBXManager.exe` for configuration
4. Open GitHub Issues for any build problems

---

## 📚 Related Documentation

- [BUILD_REQUIREMENTS.md](BUILD_REQUIREMENTS.md) - Prerequisites
- [INSTALLATION_GUIDE.md](INSTALLATION_GUIDE.md) - Installation steps
- [ENHANCEMENT_ROADMAP_2025.md](ENHANCEMENT_ROADMAP_2025.md) - Future plans
- [PROJECT_STATUS.md](PROJECT_STATUS.md) - Current status
