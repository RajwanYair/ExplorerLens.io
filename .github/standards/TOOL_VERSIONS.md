# Tool Versions & Upgrade Matrix

**Last Updated:** February 2026
**Version:** 15.0.0 "Zenith"

---

## Build Toolchain

| Tool | Current | Latest Stable | Status | Upgrade Path |
|------|---------|---------------|--------|--------------|
| **MSVC (cl.exe)** | 19.50.35720 (v145) | 19.50.35720 | ✅ Current | VS 18 2026 BuildTools |
| **MSBuild** | 18.3 | 18.3 | ✅ Current | Bundled with VS 18 |
| **CMake** | 4.2.3 | 4.2.3 | ✅ Current | `scoop update cmake` |
| **Ninja** | 1.13.2 | 1.13.2 | ✅ Current | `scoop update ninja` |
| **Windows SDK** | 10.0.26100.0 | 10.0.26100.0 | ✅ Current | VS Installer |
| **vcpkg** | 2025-11-19 | 2025-11-19 | ✅ Current | Bundled with VS 18 |

## Developer Tools

| Tool | Current | Latest Stable | Status | Install |
|------|---------|---------------|--------|---------|
| **Git** | 2.53.0 | 2.53.0 | ✅ Current | `scoop update git` |
| **NASM** | 3.01 | 3.01 | ✅ Current | `scoop update nasm` |
| **Meson** | 1.10.0 | 1.10.0 | ✅ Current | `scoop update meson` |
| **NuGet** | 7.3.0 | 7.3.0 | ✅ Current | `scoop update nuget` |
| **7-Zip** | 26.00 | 26.00 | ✅ Current | `scoop update 7zip` |
| **WiX** | 6.0.2 | 6.0.2 | ✅ Current | `dotnet tool update wix -g` |
| **PowerShell** | 7.x | 7.5+ | ✅ Current | `winget upgrade PowerShell` |

## External Libraries (Statically Linked)

### Compression Libraries

| Library | Current | Latest Stable | Status | Notes |
|---------|---------|---------------|--------|-------|
| **zlib** | 1.3.1 | 1.3.1 | ✅ Current | Stable, no recent releases |
| **LZ4** | 1.10.0 | 1.10.0 | ✅ Current | |
| **zstd** | 1.5.7 | 1.5.7 | ✅ Current | Facebook/Meta maintained |
| **LZMA SDK** | 26.00 | 26.00 | ✅ Current | Igor Pavlov, stable |
| **minizip-ng** | 4.0.10 | 4.0.10 | ✅ Current | |
| **UnRAR** | 7.2.2 | 7.2.2 | ✅ Current | |
| **bzip2** | 1.0.8 | 1.0.8 | ✅ Current | Very stable |
| **libarchive** | 3.7.6 | 3.7.6 | ✅ Current | |

### Image Libraries

| Library | Current | Latest Stable | Status | Notes |
|---------|---------|---------------|--------|-------|
| **libwebp** | 1.5.0 | 1.5.0 | ✅ Current | Google maintained |
| **libavif** | 1.3.0 | 1.3.0 | ✅ Current | AOM/Google maintained |
| **dav1d** | 1.5.1 | 1.5.1 | ✅ Current | VideoLAN AV1 decoder |
| **libjxl** | 0.11.1 | 0.11.1 | ✅ Current | JPEG XL reference impl |
| **libheif** | 1.19.5 | 1.19.5 | ✅ Current | HEIF/HEIC container |
| **libde265** | 1.0.15 | 1.0.15 | ✅ Current | H.265/HEVC decoder |
| **LibRaw** | 0.21.3 | 0.21.3 | ✅ Current | RAW camera formats |

### Document/PDF Libraries

| Library | Current | Latest Stable | Status | Notes |
|---------|---------|---------------|--------|-------|
| **MuPDF** | 1.24.11 | 1.25.x | ⚠️ Update Available | Check ABI compatibility |
| **FreeType** | bundled | 2.13.3 | ✅ Current | Via MuPDF |
| **OpenJPEG** | bundled | 2.5.3 | ✅ Current | Via MuPDF |

## CRT Linkage Policy

| Target | CRT Mode | Notes |
|--------|----------|-------|
| **ExplorerLensEngine** | `/MD` (MultiThreadedDLL) | Standard |
| **LENSShell.dll** | `/MD` (MultiThreadedDLL) | COM DLL |
| **LENSManager.exe** | `/MD` (MultiThreadedDLL) | GUI app |
| **EngineTests.exe** | `/MD` (MultiThreadedDLL) | Test runner |
| **libwebp** | `/MT` → `/MD` | **Workaround**: `/NODEFAULTLIB:LIBCMT` |

> **Action Item:** Rebuild libwebp with `-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL` to eliminate the `/NODEFAULTLIB:LIBCMT` workaround.

## Upgrade Procedure

### For Scoop-managed tools

```powershell
# Update all tools at once
scoop update *

# Or individually
scoop update cmake ninja git nasm meson nuget 7zip
```

### For external libraries

1. Download new version source
2. Update version in build script filename (e.g., `Build-Zlib.ps1`)
3. Update version in `Build-Library-Core.ps1` if needed
4. Clean build: `.\build-scripts\external-libs\Build-<Library>.ps1 -Clean`
5. Update this file with new version
6. Update `copilot-instructions.md` version table
7. Run full build + test: `.\build-scripts\Build-MSVC.ps1 -Clean -Test`

### For MSVC toolchain

1. Run VS Installer → Update VS 18 BuildTools
2. Update `copilot-instructions.md` with new cl.exe version
3. Update `BUILD_METHOD.md` tool paths
4. Update `c_cpp_properties.json` compiler path
5. Clean build and verify zero warnings

## Version Checking Script

```powershell
# Quick version check for all tools
.\build-scripts\Find-All-Tools.ps1

# Or manually:
cmake --version
ninja --version
cl.exe 2>&1 | Select-String "Version"
git --version
```

---

## Compatibility Notes

- **Windows SDK 10.0.26100.0** — Targets Windows 10 1809+ and Windows 11
- **C++20** — Requires MSVC 19.29+ (v142+), we use v145
- **AVX2** — Required for SIMD acceleration; runtime fallback exists for SSE4.2
- **DirectX 11** — Primary GPU renderer, available on all supported Windows versions
- **DirectX 12** — Used for compute pipeline, requires Windows 10 1607+
- **Vulkan** — Optional compute pipeline, requires Vulkan-capable GPU + drivers
