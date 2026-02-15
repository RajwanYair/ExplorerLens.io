# External Dependencies

This directory contains third-party libraries and dependencies used by DarkThumbs.

## Purpose

The `external/` directory holds:
- **Source code** of external libraries (extracted from downloads/)
- **Built libraries** (.lib, .dll files)
- **Header files** for compilation
- **Installation directories** for CMake-built libraries

## Directory Structure

```
external/
├── archives/              # Archive handling libraries (libarchive)
├── compression/           # Compression libraries (zlib, zstd, lz4, lzma, minizip-ng)
├── image-libs/            # Image format libraries (libwebp, libjxl, libavif, libraw, libheif)
├── libraw/                # RAW photo format library
├── libraw-install/        # LibRaw installation
├── libwebp-1.5.0/         # WebP library source
├── pdf-libs/              # PDF handling libraries (future)
├── ui-frameworks/         # UI frameworks (WTL - Windows Template Library)
├── CMakeLists.txt         # CMake configuration for external libs
├── LIBRARY_INVENTORY.md   # Complete inventory of all dependencies
└── README.md              # This file
```

## Categories

### Archives (`archives/`)
Libraries for handling archive formats (.zip, .7z, .rar, etc.):
- **libarchive** - Multi-format archive library

### Compression (`compression/`)
Compression and decompression libraries:
- **zlib** - DEFLATE compression (used by PNG, ZIP)
- **zstd** - Zstandard compression (fast, high ratio)
- **lz4** - Extremely fast compression
- **lzma** - LZMA/LZMA2/XZ compression (7-Zip)
- **minizip-ng** - Modern ZIP library (fork of minizip)
- **unrar** - RAR decompression

### Image Libraries (`image-libs/`)
Modern and specialized image format support:
- **libwebp** - WebP encoder/decoder
- **libjxl** - JPEG XL codec (next-gen format)
- **libavif** - AVIF/AV1 image format
- **libheif** - HEIF/HEIC format (Apple photos)
- **dav1d** - AV1 video decoder (used by libavif)

### RAW Photo Libraries
Professional camera RAW format support:
- **libraw** - Decode 600+ camera RAW formats
- **libraw-install/** - Built LibRaw installation

### UI Frameworks (`ui-frameworks/`)
User interface frameworks:
- **WTL** (Windows Template Library) - Lightweight Win32 API wrapper

### PDF Libraries (`pdf-libs/`)
PDF thumbnail generation (planned):
- Reserved for future PDF support

## Library Inventory

See [LIBRARY_INVENTORY.md](LIBRARY_INVENTORY.md) for:
- Complete list of all dependencies
- Version information
- License details
- Build status
- Dependencies between libraries

## Build Process

### Automated Building

External libraries are built automatically by scripts in `build-scripts/external-libs/`:

```powershell
# Build all external libraries
.\build-scripts\library-builders\Build-All-External-Libraries.ps1

# Build specific category
.\build-scripts\external-libs\Build-LibWebP-NMake.ps1
.\build-scripts\external-libs\build-libjxl.ps1
.\build-scripts\external-libs\Build-Zlib.ps1
```

### Build Flow

1. **Download** - Archives downloaded to `downloads/` (if not present)
2. **Extract** - Source extracted to `external/[category]/[library]/`
3. **Configure** - CMake/NMake configuration
4. **Build** - Compile for x64 Release (and Debug if requested)
5. **Install** - Headers and libs copied to install/ directories

### Build Systems Used

- **CMake** - Most modern libraries (libjxl, libavif, zstd, etc.)
- **NMake** - Some Windows-specific builds (LibRaw, LibWebP)
- **Visual Studio** - Direct .sln/.vcxproj builds (zlib, WTL)

## Usage in DarkThumbs

### Linking

External libraries are linked into DarkThumbs via:

```cpp
// In CBXShell.vcxproj:
<AdditionalIncludeDirectories>
  $(SolutionDir)external\libwebp-1.5.0\src\;
  $(SolutionDir)external\image-libs\libjxl\install\include\;
  $(SolutionDir)external\compression\zlib\;
  ...
</AdditionalIncludeDirectories>

<AdditionalLibraryDirectories>
  $(SolutionDir)external\libwebp-1.5.0\x64\Release\;
  $(SolutionDir)external\image-libs\libjxl\install\lib\;
  ...
</AdditionalLibraryDirectories>

<AdditionalDependencies>
  libwebp.lib;
  jxl.lib;
  zlibstatic.lib;
  ...
</AdditionalDependencies>
```

## Runtime Considerations

### Static vs Dynamic Linking

DarkThumbs uses **static linking** for external libraries to:
- ✅ Simplify deployment (no external DLLs)
- ✅ Avoid DLL conflicts with other software
- ✅ Enable whole program optimization
- ⚠️ Increases binary size (~5-10 MB)

### Runtime Library

All external libraries must be built with:
- **Release:** `/MD` (Multi-threaded DLL)
- **Debug:** `/MDd` (Multi-threaded Debug DLL)

Mismatched runtime libraries cause link errors. To fix:

```powershell
.\build-scripts\production\Rebuild-External-Libs-Correct-Runtime.ps1
```

### Dependencies

Some libraries depend on others:

```
libavif
  └─ dav1d (AV1 decoder)
  └─ libwebp (fallback encoder)

libjxl
  └─ brotli (compression)
  └─ highway (SIMD)
  └─ lcms2 (color management)

minizip-ng
  └─ zlib (compression)
```

Build scripts handle dependencies automatically.

## Maintenance

### Updating Libraries

To update an external library to a new version:

1. **Download** new version to `downloads/`
2. **Update** build script with new version number
3. **Test** build: `.\build-scripts\external-libs\Build-[Library].ps1`
4. **Test** DarkThumbs with new version
5. **Update** `LIBRARY_INVENTORY.md`
6. **Remove** old version from `external/`

### Adding New Libraries

To add a new external library:

1. **Research** - Verify license compatibility (MIT, BSD, Apache 2.0, etc.)
2. **Download** - Add source archive to `downloads/`
3. **Create** build script in `build-scripts/external-libs/`
4. **Extract** to appropriate `external/[category]/`
5. **Build** and test integration
6. **Document** in `LIBRARY_INVENTORY.md` and `docs/development/THIRD_PARTY.md`
7. **Update** `.gitignore` if needed

### Cleaning

To clean built external libraries:

```powershell
# Remove all build outputs (keeps source)
Get-ChildItem external -Recurse -Include build,install,x64,Release,Debug | Remove-Item -Recurse -Force

# Clean and rebuild everything
.\build-scripts\library-builders\Build-All-External-Libraries.ps1 -Clean
```

## .gitignore Configuration

External library source and builds are tracked selectively:

```gitignore
# External library dependencies (downloaded submodules)
external/**/third_party/    # Don't track nested third-party dirs
external/**/build*/         # Don't track build directories
external/**/install/        # Don't track install directories
external/**/.git/           # Don't track nested git repos
external/**/.github/        # Don't track GitHub dirs
```

This means:
- ✅ **Source code** is tracked (extracted libraries)
- ❌ **Build outputs** are NOT tracked (generated files)
- ❌ **Install dirs** are NOT tracked (CMake install destinations)

## Licenses

All external libraries must be:
- **Open source** with permissive license
- **Compatible** with MIT license (our project license)
- **Documented** in `docs/development/THIRD_PARTY.md`

Common compatible licenses:
- ✅ MIT, BSD, Apache 2.0, Zlib, Boost
- ⚠️ LGPL (with dynamic linking only)
- ❌ GPL (not compatible with proprietary licensing)

See [THIRD_PARTY.md](../docs/development/THIRD_PARTY.md) for complete license information.

## Troubleshooting

### Build Failures

**"Cannot find header file"**
```powershell
# Rebuild the specific library
.\build-scripts\external-libs\Build-[Library].ps1

# Check that install/ directory was created
Get-ChildItem external\image-libs\libjxl\install\include\
```

**"Unresolved external symbol"**
```powershell
# Check library was built
Get-ChildItem external -Recurse -Filter *.lib

# Verify runtime library compatibility
.\build-scripts\validation\Verify-Build.ps1
```

**"CMake error"**
```powershell
# Verify CMake is installed
cmake --version  # Should be 3.20+

# Check for required tools (NASM, etc.)
.\build-scripts\Test-Build-Environment.ps1
```

### Version Mismatches

If you see warnings about version mismatches:

```powershell
# Check current versions
Get-Content external\LIBRARY_INVENTORY.md

# Update to latest versions
.\build-scripts\download-updates.ps1
```

## CI/CD

In continuous integration:

```yaml
# Cache external libraries to speed up builds
- uses: actions/cache@v3
  with:
    path: |
      external/**/install
      external/**/x64/Release
    key: external-libs-${{ hashFiles('build-scripts/external-libs/*.ps1') }}

# Build external libraries if not cached
- name: Build External Libraries
  run: .\build-scripts\library-builders\Build-Critical-Libraries.ps1
```

## Performance Considerations

### Build Time

Building all external libraries takes:
- **Full build:** 10-30 minutes (depends on CPU)
- **Incremental:** 2-5 minutes (if only one library changed)
- **Cached (CI):** < 1 minute (restore from cache)

### Disk Space

External libraries require:
- **Source:** ~500 MB
- **Build outputs:** ~1-2 GB
- **Total:** ~2-3 GB

## Support

For issues with external libraries:

1. **Check** build logs in `build-logs/`
2. **Verify** prerequisites: `.\build-scripts\Test-Build-Environment.ps1`
3. **Review** library documentation (see LIBRARY_INVENTORY.md for URLs)
4. **Search** GitHub issues for similar problems
5. **Open** new issue with full log output

## See Also

- [LIBRARY_INVENTORY.md](LIBRARY_INVENTORY.md) - Complete dependency list
- [THIRD_PARTY.md](../docs/development/THIRD_PARTY.md) - License information
- [Build Scripts](../build-scripts/external-libs/) - Individual library builders
- [Build Guide](../docs/build/BUILD_GUIDE.md) - Building DarkThumbs

---

**Last Updated:** February 11, 2026  
**Total Libraries:** 15+  
**Total Size:** ~2.5 GB (source + builds)
