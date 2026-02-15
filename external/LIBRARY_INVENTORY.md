# External Library Inventory

**Last Updated:** February 15, 2026  
**Note:** All libraries must be built with `-DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreadedDLL"` for proper CRT linkage.

---

## Compression Libraries (compression-libs/)

| Library | Version | Latest | Status | Build System |
|---------|---------|--------|--------|--------------|
| zlib | 1.3.1 | 1.3.1 | ✅ Up-to-date | CMake |
| zstd | 1.5.7 | 1.5.7 | ✅ Up-to-date | CMake |
| lz4 | 1.10.0 | 1.10.0 | ✅ Up-to-date | CMake |
| minizip-ng | 4.0.10 | 4.0.10 | ✅ Up-to-date | CMake |
| bzip2 | 1.0.8 | 1.0.8 | ✅ Up-to-date | CMake |
| lzma | 26.00 | 26.00 | ✅ Up-to-date | Custom build |
| xz | 5.6.3 | 5.6.3 | ✅ Up-to-date | CMake |
| unrar | 7.2.2 | 7.2.2 | ✅ Up-to-date | Custom build |
| libarchive | 3.7.6 | 3.7.6 | ✅ Up-to-date | CMake |

## Image Libraries (image-libs/)

| Library | Version | Latest | Status | Build System |
|---------|---------|--------|--------|--------------|
| libwebp | 1.5.0 | 1.5.0 | ✅ Up-to-date | NMake |
| libavif | 1.3.0 | 1.3.0 | ✅ Up-to-date | CMake |
| dav1d | 1.5.1 | 1.5.1 | ✅ Up-to-date | Meson |
| libjxl | 0.11.1 | 0.11.1 | ✅ Up-to-date | CMake |

## PDF Libraries (pdf-libs/)

| Library | Version | Status | Build System |
|---------|---------|--------|--------------|
| mupdf | 1.24.11 | ✅ Available | NMake |

## Camera RAW Libraries (camera-libs/)

| Library | Version | Status | Build System |
|---------|---------|--------|--------------|
| LibRaw | 0.21.3 | ⚠️ Optional | CMake |

## UI Frameworks (ui-libs/)

| Library | Version | Status |
|---------|---------|--------|
| wtl | 10.0.10320 | ✅ Header-only |

---

## Build Tools (Verified February 15, 2026)

| Tool | Version | Source | Path |
|------|---------|--------|------|
| CMake | 4.2.3 | scoop | `C:\Users\ryair\scoop\shims\cmake.exe` |
| Ninja | 1.13.2 | scoop | `C:\Users\ryair\scoop\shims\ninja.exe` |
| MSBuild | 18.3.0 | VS 2026 BuildTools | `C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\MSBuild\Current\Bin\amd64\MSBuild.exe` |
| vcvarsall | VS 2026 | VS 2026 BuildTools | `C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvarsall.bat` |
| Git | 2.52+ | scoop | Auto-detected |
| NASM | 3.01+ | scoop | Required for libwebp |
| Perl | 5.42+ | scoop | Required for some builds |

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

### Install to SDK
```powershell
cmake --install <build> --prefix "$PSScriptRoot\..\..\SDK"
```

---

## Cleanup Status
- ✅ Removed: lzma-24.08 (superseded by 25.00)
- ✅ Removed: dav1d-0.1.0 (superseded by 1.5.1)
- ✅ Cleaned: Old build artifacts
