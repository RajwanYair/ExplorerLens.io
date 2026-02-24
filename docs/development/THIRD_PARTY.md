# Third-Party Libraries - Build & Integration Guide

**Last Updated:** July 2025  
**Version:** 14.0.0  
**Policy:** All libraries must be x64 and use `/MD` (dynamic CRT — `MultiThreadedDLL`) for Release builds

---

## Library Status

| Library | Version | Status | Built | Linked |
|---------|---------|--------|-------|--------|
| **zlib** | 1.3.1 | ✅ Complete | Yes | Yes |
| **lz4** | 1.10.0 | ✅ Complete | Yes | Yes |
| **zstd** | 1.5.7 | ✅ Complete | Yes | Yes |
| **liblzma** | 5.6.3 | ✅ Complete | Yes | Yes |
| **minizip-ng** | 4.0.10 | ✅ Complete | Yes | Yes |
| **libwebp** | 1.5.0 | ✅ Complete | Yes | Yes |
| **libavif** | 1.3.0 | ✅ Complete | Yes | Yes |
| **libjxl** | 0.11.1 | ✅ Complete | Yes | Yes |
| **dav1d** | 1.5.1 | 🔄 Dependency | For AVIF | N/A |
| **brotli** | 1.1.0 | 🔄 Dependency | For JXL | N/A |
| **highway** | 1.0.7 | 🔄 Dependency | For JXL | N/A |
| **libheif** | 1.19.5 | ✅ Complete | Yes | Yes |
| **libde265** | 1.0.15 | ✅ Complete | Yes | Yes |
| **LibRaw** | 0.21.3 | ✅ Complete | Yes | Yes |
| **UnRAR** | 7.2.2 | ✅ Complete | Yes | Yes |

---

## Download & Extract

### Centralized Downloads
All third-party archives go to `/downloads/` only:

```
/downloads/
  compression/
    zlib-1.3.1.tar.gz
    lz4-1.10.0.tar.gz
    zstd-1.5.7.tar.gz
    minizip-ng-4.0.10.tar.gz
  image-libs/
    libwebp-1.5.0.tar.gz
    libavif-1.3.0.tar.gz
    libjxl-0.11.1.tar.gz
  dependencies/
    highway-1.0.7.tar.gz
    brotli-1.1.0.tar.gz
```

### Extraction Script
```powershell
# Extract all archives to /external/
$archives = Get-ChildItem downloads -Recurse -Filter "*.tar.gz"
foreach ($archive in $archives) {
    $destDir = Join-Path "external" $archive.BaseName
    7z x $archive.FullName -o$destDir
}
```

---

## Build Instructions

### zlib 1.3.1
```powershell
cd external/zlib-1.3.1
cmake -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded
cmake --build build --config Release
# Output: build/Release/zlibstatic.lib
```

### lz4 1.10.0
```powershell
cd external/lz4-1.10.0
cmake -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded
cmake --build build --config Release
# Output: build/Release/liblz4_static.lib
```

### zstd 1.5.7
```powershell
cd external/zstd-1.5.7/build/cmake
cmake -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded
cmake --build build --config Release
# Output: build/lib/Release/zstd_static.lib
```

### minizip-ng 4.0.10
```powershell
cd external/minizip-ng-4.0.10
cmake -B build -G "Ninja" `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL `
  -DMZ_FETCH_DEPS=ON `
  -DZLIB_ROOT=../zlib-1.3.1/build
cmake --build build --config Release
# Output: build/Release/minizip.lib
```

### libwebp 1.5.0
```powershell
cd external/libwebp-1.5.0
cmake -B build -G "Ninja" `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded `
  -DWEBP_BUILD_VWEBP=OFF `
  -DWEBP_BUILD_CWEBP=OFF `
  -DWEBP_BUILD_DWEBP=OFF
cmake --build build --config Release
# Output: build/Release/webp.lib, sharpyuv.lib
```

### libavif 1.3.0 (requires dav1d)
```powershell
# Build dav1d first
cd external/dav1d-1.5.1
meson setup build --buildtype=release --default-library=static
ninja -C build

# Build libavif
cd external/libavif-1.3.0
cmake -B build -G "Ninja" `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded `
  -DAVIF_CODEC_DAV1D=ON `
  -DDAV1D_LIBRARY=../dav1d-1.5.1/build/src/libdav1d.a `
  -DDAV1D_INCLUDE_DIR=../dav1d-1.5.1/include
cmake --build build --config Release
# Output: build/Release/avif.lib
```

### libjxl 0.11.1 (requires highway, brotli)
```powershell
# Build dependencies first
cd external/highway-1.0.7
cmake -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

cd external/brotli-1.1.0
cmake -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# Build libjxl
cd external/libjxl-0.11.1
cmake -B build-msvc -G "Ninja" `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded `
  -DBUILD_TESTING=OFF `
  -DJPEGXL_ENABLE_TOOLS=OFF `
  -DJPEGXL_ENABLE_BENCHMARK=OFF `
  -DHWY_INCLUDE_DIR=../highway-1.0.7/include `
  -DBROTLI_INCLUDE_DIRS=../brotli-1.1.0/c/include
cmake --build build-msvc --config Release
# Output: build-msvc/lib/Release/jxl.lib, jxl_threads.lib
```

---

## Integration with ExplorerLens

### Include Paths (LENSShell.vcxproj)
```xml
<AdditionalIncludeDirectories>
  ..\Engine;
  ..\external\compression-libs\zlib-1.3.1;
  ..\external\compression-libs\zstd-1.5.7\lib;
  ..\external\compression-libs\lz4-1.10.0\lib;
  ..\external\compression-libs\minizip-ng-4.0.10;
  ..\external\image-libs\libwebp-1.5.0\src;
  ..\external\image-libs\libjxl-0.11.1\lib\include;
  ..\external\image-libs\libjxl-0.11.1\build-msvc\lib\include\jxl;
  ..\external\image-libs\libavif-1.3.0\include;
  %(AdditionalIncludeDirectories)
</AdditionalIncludeDirectories>
```

### Library Paths (LENSShell.vcxproj)
```xml
<AdditionalLibraryDirectories>
  ..\Engine\Release\Release;
  ..\external\compression-libs\zlib-1.3.1\x64\Release;
  ..\external\compression-libs\lz4-1.10.0\build-msvc\x64\Release;
  ..\external\compression-libs\zstd-1.5.7\lib\x64\Release;
  ..\external\compression-libs\minizip-ng-4.0.10\build-msvc\Release;
  ..\external\image-libs\libwebp-1.5.0\output\Release-static\x64\lib;
  ..\external\image-libs\libjxl-0.11.1\build-msvc\lib\Release;
  ..\external\image-libs\libavif-1.3.0\build\Release;
  %(AdditionalLibraryDirectories)
</AdditionalLibraryDirectories>
```

### Linked Libraries (LENSShell.vcxproj)
```xml
<AdditionalDependencies>
  ExplorerLensEngine.lib;
  zlibstatic.lib;
  liblz4_static.lib;
  zstd_static.lib;
  minizip.lib;
  webp.lib;
  sharpyuv.lib;
  avif.lib;
  jxl.lib;
  jxl_threads.lib;
  %(AdditionalDependencies)
</AdditionalDependencies>
```

---

## Verification

After building all libraries:

```powershell
# Check all libraries exist
$libs = @(
    "external\compression-libs\zlib-1.3.1\x64\Release\zlibstatic.lib",
    "external\compression-libs\lz4-1.10.0\build-msvc\x64\Release\liblz4_static.lib",
    "external\compression-libs\zstd-1.5.7\lib\x64\Release\zstd_static.lib",
    "external\compression-libs\minizip-ng-4.0.10\build-msvc\Release\minizip.lib",
    "external\image-libs\libwebp-1.5.0\output\Release-static\x64\lib\webp.lib",
    "external\image-libs\libavif-1.3.0\build\Release\avif.lib",
    "external\image-libs\libjxl-0.11.1\build-msvc\lib\Release\jxl.lib"
)

foreach ($lib in $libs) {
    if (Test-Path $lib) {
        $size = (Get-Item $lib).Length / 1KB
        Write-Host "✓ $lib ($([math]::Round($size, 0)) KB)" -ForegroundColor Green
    } else {
        Write-Host "✗ $lib NOT FOUND" -ForegroundColor Red
    }
}
```

---

## Troubleshooting

### "Cannot find library X"
- Verify library built successfully
- Check AdditionalLibraryDirectories paths
- Ensure x64 architecture (not Win32)

### "Unresolved external symbol"
- Check runtime library mismatch: all libs must use `/MD` (dynamic CRT)
- Verify all dependencies included (e.g., jxl needs jxl_threads)

### "LNK1104: cannot open file"
- Ensure CMake build completed without errors
- Check paths are relative to .vcxproj location (use `..`)

---

## Build Scripts

All libraries have dedicated build scripts in `build-scripts/external-libs/`.
Use `Build-Library-Core.ps1` from `build-scripts/core/` for shared build utilities.

```powershell
# Build all external libraries
.\build-scripts\Build-All-And-Package.ps1

# Build individual library
.\build-scripts\external-libs\Build-LibHEIF.ps1 -Clean
```

---

## References

- zlib: https://github.com/madler/zlib
- lz4: https://github.com/lz4/lz4
- zstd: https://github.com/facebook/zstd
- minizip-ng: https://github.com/zlib-ng/minizip-ng
- libwebp: https://github.com/webmproject/libwebp
- libavif: https://github.com/AOMediaCodec/libavif
- libjxl: https://github.com/libjxl/libjxl

