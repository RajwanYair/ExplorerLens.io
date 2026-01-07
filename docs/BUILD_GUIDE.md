# DarkThumbs Build Guide

**Complete Build Instructions - Consolidated Guide**  
**Last Updated:** January 7, 2026

---

## Quick Start

**Prerequisites:**

- Visual Studio 2026 BuildTools with MSVC v19.50+
- CMake 3.20+
- Git

**Fast Build (Normal Machines):**

```powershell
.\Build-Production.ps1
```

---

## Current Build Status (January 7, 2026)

### ✅ Successfully Built (2/6)

- **zlib 1.3.1**: `external\compression\zlib-1.3.1\x64\Release\zlibstatic.lib` (128.9 KB)
- **LZ4 1.10.0**: `external\compression\lz4-1.10.0\build-vs\Release\liblz4_static.lib` (645.6 KB)

### ❌ Build Issues (4/6)

| Library | Issue | Solution |
|---------|-------|----------|
| **liblzma** | CMake doesn't generate Makefile via PowerShell | Build in native VS command prompt |
| **zstd** | Source missing CMake files | Re-download complete source from GitHub |
| **LibWebP** | nmake runs but no .lib output | Under investigation, try CMake build |
| **Minizip-NG** | Blocked by dependencies | Build after zstd + liblzma complete |

---

## Environment Setup

### Required Tools

1. **Visual Studio 2026 BuildTools**

   ```powershell
   # Verify installation
   &"C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
   cl /? # Should show MSVC 19.50+
   ```

2. **CMake** (3.20+)

   ```powershell
   cmake --version  # Should be 3.20 or higher
   ```

3. **Git**

   ```powershell
   git --version
   ```

### Opening Native Command Prompt (CRITICAL)

**Why Native?** PowerShell wrappers cause CMake environment issues.

1. Press `Win + X`
2. Select "Terminal" or "Command Prompt"
3. Run: `"C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat"`
4. Verify: `where nmake` should show VS BuildTools path

---

## Manual Library Builds (Recommended for First Time)

### 1. liblzma (XZ 5.6.3)

```cmd
cd "C:\Users\ryair\OneDrive - Intel Corporation\Documents\MyScripts\DarkThumbs"
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
where nmake  # Should show VS path
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
dir Makefile  # Verify it exists
findstr CMAKE_C_COMPILER CMakeCache.txt  # Check compiler path
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

1. **Build CBXShell**

   ```cmd
   msbuild CBXShell.sln /t:Rebuild /p:Configuration=Release /p:Platform=x64
   ```

2. **Register DLL**

   ```cmd
   regsvr32 /s x64\Release\CBXShell.dll
   ```

3. **Test**
   - Open Explorer
   - Navigate to folder with test images
   - Verify thumbnails appear

4. **Unregister** (when done testing)

   ```cmd
   regsvr32 /u /s x64\Release\CBXShell.dll
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
**Maintainer:** DarkThumbs Build Team
