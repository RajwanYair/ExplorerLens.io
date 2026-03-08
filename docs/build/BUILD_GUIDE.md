# ExplorerLens.io Build Guide

**Complete Build Instructions - x64 Only** 
**Last Updated:** February 12, 2026

---

## Important Notes

### Platform Support

**This project supports x64 (64-bit) builds ONLY.**

- ✅ **Supported:** x64 (64-bit Windows)
- ❌ **Not Supported:** Win32/x86 (32-bit) - removed as of January 2026
- **Reason:** Modern Windows shell extensions should target 64-bit architecture

All project files have been configured to build for x64 platform exclusively. Win32 configurations have been removed to simplify the build process and prevent accidental 32-bit builds.

---

## Quick Start

**Prerequisites:**

- Visual Studio 2026 BuildTools with MSVC v19.50+
- CMake 3.20+
- Git

**Fast Build:**

```powershell
.\scripts\build.ps1
```

---

## Current Build Status (February 12, 2026)

### ✅ Successfully Built - Production Ready (All Core Components)

| Component | Size | Status |
|-----------|------|--------|
| **ExplorerLens.ioEngine.lib** | 7.48 MB | ✅ 0 errors, 0 warnings |
| **LENSShell.dll** | 1.43 MB | ✅ Shell extension fully functional |
| **LENSManager.exe** | 305 KB | ✅ Management UI |
| **zlib 1.3.1** | 128.9 KB | ✅ Static library |
| **LZ4 1.10.0** | 645.6 KB | ✅ Static library |
| **zstd 1.5.7** | Built | ✅ Static library |
| **minizip-ng 4.0.10** | Built | ✅ Static library |
| **LibRaw** | Built | ✅ RAW photo support (WIC-based) |
| **LibWebP 1.5.0** | Built | ✅ WebP support |

### 🔄 Optional Libraries (In Progress)

| Library | Status | Notes |
|---------|--------|-------|
| **libjxl 0.11.1** | 🔄 Building | JPEG XL support (background build) |
| **libheif 1.19** | ⏳ Pending | HEIF/HEIC support (requires dav1d + libde265) |

**Build works perfectly without JXL/HEIF - they're optional enhancements.**

---

## Environment Setup

### Required Tools

#### 1. Visual Studio 2026 Build Tools

**How to Find:**

```powershell
# Check if VS Build Tools are installed
$vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $vsWhere) {
 $vsPath = & $vsWhere -latest -property installationPath
 Write-Host "Visual Studio found at: $vsPath"
} else {
 Write-Host "Visual Studio not found - download from https://visualstudio.microsoft.com/downloads/"
}
```

**Manual Check:**
- **Location:** `C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools`
- **Version Required:** MSVC v19.50+ (Visual Studio 2026)
- **Download:** https://visualstudio.microsoft.com/downloads/ → Build Tools for Visual Studio

**Verify Installation:**

```powershell
# Set up x64 build environment
&"${env:ProgramFiles(x86)}\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
cl /? # Should display MSVC 19.50+
```

#### 2. CMake (3.20+)

**How to Find:**

```powershell
# Check if CMake is in PATH
$cmake = Get-Command cmake -ErrorAction SilentlyContinue
if ($cmake) {
 cmake --version
} else {
 Write-Host "CMake not found - download from https://cmake.org/download/"
}
```

**Download:** https://cmake.org/download/ 
**Install Location:** Typically `C:\Program Files\CMake\bin\cmake.exe`

#### 3. Git

**How to Find:**

```powershell
# Check if Git is installed
$git = Get-Command git -ErrorAction SilentlyContinue
if ($git) {
 git --version
} else {
 Write-Host "Git not found - download from https://git-scm.com/downloads"
}
```

**Download:** https://git-scm.com/downloads 
**Install Location:** Typically `C:\Program Files\Git\bin\git.exe`

### Opening Native Command Prompt for Build Tools

**Why Native?** PowerShell wrappers may cause CMake environment issues.

**Option 1: Developer Command Prompt**
1. Start Menu → Search "Developer Command Prompt for VS 2026"
2. This automatically sets up the build environment

**Option 2: Manual Setup**
1. Press `Win + X` → Select "Terminal" or "Command Prompt"
2. Run: `"C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat"`
3. Verify: `where nmake` should show VS BuildTools path

**Option 3: PowerShell with vcvars64 (Recommended)**

```powershell
# Import VS environment variables (use in PowerShell scripts)
cmd /c '"C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat" && set' | ForEach-Object {
 if ($_ -match "^(.*?)=(.*)$") {
 Set-Item -Force -Path "ENV:\$($matches[1])" -Value $matches[2]
 }
}
```

---

## Manual Library Builds (Recommended for First Time)

### 1. liblzma (XZ 5.6.3)

```cmd
cd "C:\Users\ryair\OneDrive - Intel Corporation\Documents\MyScripts\ExplorerLens.io"
cd external\compression\xz-5.6.3
mkdir build-native
cd build-native
cmake .. -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS_RELEASE="/MT /O2" -DBUILD_SHARED_LIBS=OFF
nmake
dir /s *.lib
```

**Expected Output:** `liblzma.lib` (~558 KB)

---

### 2. zstd 1.5.7

**Problem:** Source missing `build/cmake/` directory.

**Solution - Download Complete Source:**

1. Go to: <https://github.com/facebook/zstd/releases/tag/v1.5.7>
2. Download `zstd-1.5.7.tar.gz` (NOT the GitHub source zip)
3. Extract to replace `external\compression\zstd-1.5.7\`
4. Verify `build\cmake\CMakeLists.txt` exists

**Build with CMake:**

```cmd
cd external\compression\zstd-1.5.7\build-native
cmake ..\build\cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release
nmake
```

**Expected Output:** `libzstd_static.lib` (500-800 KB)

---

### 3. LibWebP 1.5.0

**Method A - NMake (Native):**

```cmd
cd external\image-libs\libwebp-1.5.0
nmake /f Makefile.vc CFG=release-static RTLIBCFG=static OBJDIR=output clean
nmake /f Makefile.vc CFG=release-static RTLIBCFG=static OBJDIR=output all
dir output /s /b *.lib
```

**Method B - CMake (Alternative):**

```cmd
cd external\image-libs\libwebp-1.5.0
mkdir build-cmake
cd build-cmake
cmake .. -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DWEBP_BUILD_ANIM_UTILS=OFF
nmake
```

**Expected Output:**

- `libwebp.lib` (~500 KB)
- `libwebpdecoder.lib` (~200 KB)
- `libsharpyuv.lib` (~50 KB)

---

### 4. Minizip-NG 4.0.10

**Prerequisites:** zlib ✅, liblzma, zstd must be built first.

```cmd
cd external\compression\minizip-ng-4.0.10
mkdir build-native
cd build-native

set ZLIB_ROOT=%CD%\..\..\zlib-1.3.1
set LZMA_ROOT=%CD%\..\..\xz-5.6.3
set ZSTD_ROOT=%CD%\..\..\zstd-1.5.7

cmake .. -G "NMake Makefiles" ^
 -DCMAKE_BUILD_TYPE=Release ^
 -DMZ_ZLIB=ON ^
 -DMZ_LZMA=ON ^
 -DMZ_ZSTD=ON ^
 -DZLIB_ROOT="%ZLIB_ROOT%" ^
 -DLIBLZMA_ROOT="%LZMA_ROOT%" ^
 -DZSTD_ROOT="%ZSTD_ROOT%"

nmake
```

**Expected Output:** `libminizip.lib` or `minizip.lib` (~100-200 KB)

---

## Troubleshooting

### "nmake not found"

**Cause:** Visual Studio environment not loaded.

**Solution:**

```cmd
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
where nmake # Should show VS path
```

---

### "CMake Error: CMAKE_C_COMPILER not set"

**Cause:** CMake can't find compiler because VS environment not loaded.

**Solution:** Run `vcvars64.bat` first (see above), then retry CMake.

---

### "MAKEFILE not found" after CMake

**Cause:** CMake configuration failed silently OR using wrong generator.

**Debug:**

```cmd
cmake .. -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release
dir Makefile # Verify it exists
findstr CMAKE_C_COMPILER CMakeCache.txt # Check compiler path
```

**Solution:**

1. Ensure vcvars64.bat run first
2. Clean build directory: `rmdir /s build-native && mkdir build-native`
3. Retry CMake from VS command prompt (not PowerShell)

---

### LibWebP builds but no .lib files

**Investigation Steps:**

```cmd
cd external\image-libs\libwebp-1.5.0
nmake /f Makefile.vc CFG=release-static RTLIBCFG=static VERBOSE=1 all > build.log 2>&1
type build.log
```

**Workaround:** Try CMake build method (see above).

---

## Build Verification

```powershell
# Check all library outputs
$libs = @{
 "zlib" = "external\compression\zlib-1.3.1\x64\Release\zlibstatic.lib"
 "LZ4" = "external\compression\lz4-1.10.0\build-vs\Release\liblz4_static.lib"
 "liblzma" = "external\compression\xz-5.6.3\build-native\liblzma.lib"
 "zstd" = "external\compression\zstd-1.5.7\lib\libzstd_static.lib"
 "libwebp" = "external\image-libs\libwebp-1.5.0\output\lib\libwebp.lib"
 "minizip" = "external\compression\minizip-ng-4.0.10\build-native\libminizip.lib"
}

foreach ($name in $libs.Keys) {
 $path = $libs[$name]
 if (Test-Path $path) {
 $size = (Get-Item $path).Length / 1KB
 Write-Host "✅ $name : $([math]::Round($size, 1)) KB" -ForegroundColor Green
 } else {
 Write-Host "❌ $name : NOT FOUND" -ForegroundColor Red
 }
}
```

---

## Next Steps After Successful Build

1. **Build LENSShell**

 ```cmd
 msbuild LENSShell.sln /t:Rebuild /p:Configuration=Release /p:Platform=x64
 ```

2. **Register DLL**

 ```cmd
 regsvr32 /s x64\Release\LENSShell.dll
 ```

3. **Test**
 - Open Explorer
 - Navigate to folder with test images
 - Verify thumbnails appear

4. **Unregister** (when done testing)

 ```cmd
 regsvr32 /u /s x64\Release\LENSShell.dll
 ```

---

## Build Scripts Reference

| Script | Purpose |
|--------|---------|
| `Build-Production.ps1` | Full automated build |
| `build-scripts\Build-Zlib.ps1` | Build zlib only |
| `build-scripts\Build-LZ4.ps1` | Build LZ4 only |
| `build-scripts\build-lzma-24.08.ps1` | Build liblzma only |
| `build-scripts\Build-LibWebP-NMake.ps1` | Build LibWebP only |
| `build-scripts\Build-MinizipNG.ps1` | Build Minizip-NG only |

---

## Getting Help

**Build fails?**

1. Check troubleshooting section above
2. Verify all prerequisites installed
3. Try manual build in native VS command prompt
4. Check build log files in `build-logs\`

**Still stuck?**

- Review [ACTION_PLAN_2026.md](ACTION_PLAN_2026.md) for current priorities
- Check GitHub issues

---

**Status:** ACTIVE - Under development 
**Maintainer:** ExplorerLens.io Build Team
