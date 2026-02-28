# External Library Inventory

**Last Updated:** February 26, 2026  
**Project Version:** 15.0.0 "Zenith"  
**CRT Linkage:** All libraries must use `/MD` (`MultiThreadedDLL`).  
**Build Toolset:** MSVC v145 (cl.exe 19.50.35720) — Visual Studio 18 (2026) BuildTools

---

## Compression Libraries (`compression-libs/`)

| Library | Version | License | Source Directory | Build Script | Build System | Status |
|---------|---------|---------|------------------|--------------|--------------|--------|
| zlib | 1.3.1 | zlib | `zlib-1.3.1/` | `Build-Zlib.ps1` | CMake | ✅ Built |
| zstd | 1.5.7 | BSD-3-Clause | `zstd-1.5.7/` | `Build-Zstd.ps1` | CMake | ✅ Built |
| LZ4 | 1.10.0 | BSD-2-Clause | `lz4-1.10.0/` | `Build-LZ4.ps1` | CMake | ✅ Built |
| minizip-ng | 4.0.10 | zlib | `minizip-ng-4.0.10/` | `Build-MinizipNG.ps1` | CMake | ✅ Built |
| bzip2 | 1.0.8 | BSD-like | `bzip2-1.0.8/` | — | Custom | ✅ Built |
| LZMA SDK | 26.00 | Public Domain | `lzma-26.00/` | `Build-LZMA-SDK-26.00.ps1` | Custom | ✅ Built |
| xz/liblzma | 5.6.3 | 0BSD | `xz-5.6.3/` | — | CMake | ✅ Built |
| UnRAR | 7.2.2 | UnRAR (restrictive) | `unrar-7.2.2/` | `Build-UnRAR.ps1` | MSBuild | ✅ Built |
| libarchive | 3.7.6 | BSD-2-Clause | `libarchive-3.7.6/` | `Build-Libarchive.ps1` | CMake | ✅ Built |

## Image Libraries (`image-libs/`)

| Library | Version | License | Source Directory | Build Script | Build System | Status |
|---------|---------|---------|------------------|--------------|--------------|--------|
| libwebp | 1.5.0 | BSD-3-Clause | `libwebp-1.5.0-original/` | `Build-LibWebP-NMake.ps1` | NMake | ✅ Built |
| libavif | 1.3.0 | BSD-2-Clause | `libavif-1.3.0/` | `Build-LibAVIF.ps1` | CMake | ✅ Built |
| dav1d | 1.5.1 | BSD-2-Clause | `dav1d-1.5.1/` | `Build-Dav1d.ps1` | Meson | ✅ Built |
| libjxl | 0.11.1 | BSD-3-Clause | `libjxl-0.11.1/` | `Build-LibJXL.ps1` | CMake | ✅ Built |
| libheif | 1.19.5 | LGPL-3.0 | `libheif-1.19.5/` | `Build-LibHEIF.ps1` | CMake | ✅ Built |
| libde265 | 1.0.15 | LGPL-3.0 | `libde265-1.0.15/` | `Build-LibHEIF.ps1` | CMake | ✅ Built |

### Bundled Sub-dependencies (inside `libjxl-0.11.1/third_party/`)

| Library | Purpose | Linked As |
|---------|---------|-----------|
| brotli | Compression for JXL | `brotlicommon`, `brotlidec` |
| highway (hwy) | SIMD abstraction | `hwy` |
| lcms2 | Color management | `jxl_cms` |

## PDF Libraries (`pdf-libs/`)

| Library | Version | License | Source Directory | Build Script | Build System | Status |
|---------|---------|---------|------------------|--------------|--------------|--------|
| MuPDF | 1.24.11 | AGPL-3.0 | `mupdf-1.24.11-source/` | `Build-MuPDF.ps1` | MSBuild | ✅ Built |

### MuPDF Sub-libraries

| Library | Size | Status |
|---------|------|--------|
| libthirdparty.lib | ~23 MB | ✅ Built |
| libresources.lib | ~42 MB | ✅ Built |
| libextract.lib | ~2 MB | ✅ Built |
| libpkcs7.lib | ~0.1 MB | ✅ Built |
| libharfbuzz.lib | ~194 MB | ✅ Built |
| libmupdf.lib | ~300 MB | ✅ Built |

## Camera RAW Libraries (`camera-libs/`)

| Library | Version | License | Source Directory | Build Script | Build System | Status |
|---------|---------|---------|------------------|--------------|--------------|--------|
| LibRaw | 0.21.2 | LGPL-2.1 / CDDL-1.0 | `libraw/` | `Build-LibRaw.ps1` | MSBuild | ✅ Built |

## UI Frameworks (`ui-libs/`)

| Library | Version | License | Status |
|---------|---------|---------|--------|
| WTL | 10.0.10320 | MS-PL | ✅ Header-only |

## Optional / Planned Libraries

| Library | Version | License | Build Script | Status |
|---------|---------|---------|--------------|--------|
| OpenJPEG | 2.5.3 | BSD-2-Clause | `Build-OpenJPEG.ps1` | 📋 Planned |
| FreeType | 2.13.3 | FTL / GPL-2.0 | `Build-FreeType.ps1` | 📋 Planned |
| FFmpeg | 7.1 | LGPL-2.1 | `Build-FFmpeg.ps1` | 📋 Planned |

---

## Engine Feature Flags (Auto-detected in CMake)

| Flag | Library | Engine Module | Default |
|------|---------|---------------|---------|
| `HAS_LIBJXL` | libjxl 0.11.1 | JPEG XL decoder | ON |
| `HAS_LIBHEIF` | libheif 1.19.5 | HEIF/HEIC decoder | ON |
| `HAS_LIBRAW` | LibRaw 0.21.2 | Camera RAW decoder | ON |
| `HAS_LIBAVIF` | libavif 1.3.0 | AVIF decoder | ON |
| `HAS_MUPDF` | MuPDF 1.24.11 | PDF renderer | ON |
| `HAS_OPENJPEG` | OpenJPEG 2.5.3 | JPEG 2000 decoder | OFF (planned) |
| `HAS_FREETYPE` | FreeType 2.13.3 | Font renderer | OFF (planned) |
| `HAS_LIBARCHIVE` | libarchive 3.7.6 | Multi-format archives | ON |
| `ENABLE_VIDEO_DECODER` | Media Foundation | Video thumbnails | ON |
| `ENABLE_AUDIO_DECODER` | Media Foundation | Audio thumbnails | ON |

---

## Windows System Libraries (Always Linked)

| Library | Purpose |
|---------|---------|
| `gdiplus` | GDI+ image rendering |
| `d3d11` | DirectX 11 GPU rendering |
| `d3d12` | DirectX 12 compute |
| `dxgi` | DirectX Graphics Infrastructure |
| `windowscodecs` | Windows Imaging Component (WIC) |
| `shlwapi` | Shell Lightweight Utility |
| `bcrypt` | Crypto API (for minizip-ng AES) |
| `crypt32` | Certificate & signature handling |
| `wintrust` | Authenticode verification |
| `mfplat` | Media Foundation platform |
| `mfreadwrite` | MF Source Reader (video/audio) |
| `mfuuid` | MF GUIDs |
| `mf` | Media Foundation core |
| `propsys` | Property System (audio metadata) |

---

## Build Tools

| Tool | Version | Source | Path |
|------|---------|--------|------|
| cl.exe | 19.50.35720 | VS 18 BuildTools | `.../MSVC/14.50.35717/bin/Hostx64/x64/cl.exe` |
| MSBuild | 18.3.0-preview | VS 18 BuildTools | `.../MSBuild/Current/Bin/amd64/MSBuild.exe` |
| CMake | 4.2.3 | Scoop | `~/scoop/shims/cmake.exe` |
| Ninja | 1.13.2 | Scoop | `~/scoop/shims/ninja.exe` |
| Git | 2.53.0 | Scoop | `~/scoop/shims/git.exe` |
| NASM | 3.01 | Scoop | `~/scoop/shims/nasm.exe` |
| Meson | 1.10.0 | Scoop | `~/scoop/shims/meson.exe` |
| NuGet | 7.3.0 | Scoop | `~/scoop/shims/nuget.exe` |
| WiX | 6.0.2 | .NET tool | `~/.dotnet/tools/wix.exe` |

---

## Build Instructions

### Standard CMake Build (with /MD flag)

```powershell
cmake -S <source> -B <build> `
  -G Ninja `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreadedDLL" `
  -DBUILD_SHARED_LIBS=OFF

cmake --build <build> --config Release
```

### Build All Libraries + Engine + Shell + MSI

```powershell
.\build-scripts\Build-All-And-Package.ps1
```

### Build Engine Only

```powershell
.\build-scripts\Build-MSVC.ps1
```

---

## Cleanup History

- ✅ Removed: lzma-24.08 (superseded by 26.00)
- ✅ Removed: dav1d-0.1.0 (superseded by 1.5.1)
- ✅ Cleaned: Old build artifacts
- ✅ Consolidated: `unrar/` → `unrar-7.2.2/` (versioned directory)
