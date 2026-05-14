# vcpkg Setup Guide for ExplorerLens.io v39.9.0

## Overview

vcpkg is a C++ package manager that simplifies dependency management. While ExplorerLens.io currently builds most
libraries from source, vcpkg integration provides:

- ✅ **Simplified Updates:** Easily update to latest library versions
- ✅ **Consistent Builds:** Reproducible builds across machines
- ✅ **Alternative Source:** Use vcpkg packages instead of building from source
- ✅ **Future-Proofing:** Ready for additional dependencies

---

## Quick Start

### Option 1: Automatic Setup (Recommended)

```powershell
# Navigate to ExplorerLens.io root
cd C:\Users\YourName\Documents\MyScripts\ExplorerLens.io

# Run setup script
.\build-scripts\Setup-Vcpkg.ps1

# The script will:
# ✓ Check if vcpkg is already installed
# ✓ Clone vcpkg repository to $env:USERPROFILE\vcpkg
# ✓ Bootstrap vcpkg.exe
# ✓ Set environment variables
```

### Option 2: Manual Installation

```powershell
# 1. Clone vcpkg
cd $env:USERPROFILE
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg

# 2. Bootstrap vcpkg
.\bootstrap-vcpkg.bat

# 3. Set environment variable (optional but recommended)
[System.Environment]::SetEnvironmentVariable(
 'VCPKG_ROOT', 
 "$env:USERPROFILE\vcpkg", 
 'User'
)

# 4. Restart PowerShell to apply environment changes
```

---

## vcpkg Manifest Mode

ExplorerLens.io uses **vcpkg manifest mode** with `vcpkg.json` to define dependencies.

### vcpkg.json

Located in project root: `c:\...\ExplorerLens.io\vcpkg.json`

```json
{
 "name": "darkthumbs",
 "version": "7.0.0",
 "dependencies": [
 "zlib",
 "zstd",
 "lz4",
 "bzip2",
 "libwebp",
 "libavif",
 "libjpeg-turbo",
 "libpng",
 "giflib",
 "tiff",
 "libheif",
 "libraw"
 ],
 "builtin-baseline": "2024.02.14"
}
```

---

## Building with vcpkg

### CMake Integration

vcpkg provides a CMake toolchain file for automatic integration.

#### Method 1: CMake Command Line

```powershell
# Configure with vcpkg
cmake -B build `
 -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" `
 -DCMAKE_BUILD_TYPE=Release `
 -G Ninja

# Build
cmake --build build --config Release -j 8
```

#### Method 2: CMakePresets.json (Future)

Create `CMakePresets.json` in project root:

```json
{
 "version": 3,
 "configurePresets": [
 {
 "name": "vcpkg-release",
 "displayName": "Release with vcpkg",
 "generator": "Ninja",
 "binaryDir": "${sourceDir}/build-vcpkg",
 "cacheVariables": {
 "CMAKE_BUILD_TYPE": "Release",
 "CMAKE_TOOLCHAIN_FILE": "$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
 }
 }
 ]
}
```

Then build:

```powershell
cmake --preset vcpkg-release
cmake --build build-vcpkg --config Release
```

---

## Installing vcpkg Packages

### ExplorerLens.io Dependencies

The following packages can be installed via vcpkg:

| Package | Purpose | Size | Build Time |
| --------- | --------- | ------ | ------------ |
| **zlib** | Compression | ~1 MB | 30s |
| **zstd** | Zstandard compression | ~2 MB | 1m |
| **lz4** | LZ4 compression | ~1 MB | 20s |
| **bzip2** | BZip2 compression | ~1 MB | 30s |
| **libwebp** | WebP image format | ~5 MB | 2m |
| **libavif** | AVIF image format | ~10 MB | 5m |
| **libjpeg-turbo** | JPEG decoder | ~3 MB | 1m |
| **libpng** | PNG decoder | ~2 MB | 30s |
| **giflib** | GIF decoder | ~1 MB | 20s |
| **tiff** | TIFF decoder | ~3 MB | 1m |
| **libheif** | HEIF/HEIC decoder | ~8 MB | 3m |
| **libraw** | Camera RAW decoder | ~12 MB | 4m |

**Total Size:** ~50 MB
**Total Build Time:** ~20 minutes (first run only)

### Manual Package Installation

```powershell
# Install individual packages
vcpkg install zlib:x64-windows-static
vcpkg install libwebp:x64-windows-static
vcpkg install libavif:x64-windows-static

# Install all ExplorerLens.io dependencies
vcpkg install `
 zlib:x64-windows-static `
 zstd:x64-windows-static `
 lz4:x64-windows-static `
 bzip2:x64-windows-static `
 libwebp:x64-windows-static `
 libavif:x64-windows-static `
 libjpeg-turbo:x64-windows-static `
 libpng:x64-windows-static `
 giflib:x64-windows-static `
 tiff:x64-windows-static `
 libheif:x64-windows-static `
 libraw:x64-windows-static
```

### Manifest Mode (Automatic)

When using CMake with the vcpkg toolchain file, dependencies are installed automatically:

```powershell
# vcpkg reads vcpkg.json and installs dependencies automatically
cmake -B build -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"

# Output:
# -- Found vcpkg manifest at C:/.../ExplorerLens.io/vcpkg.json
# -- Installing vcpkg dependencies: zlib, libwebp, libavif, ...
# -- vcpkg install complete
```

---

## Triplets

Triplets define the target platform and linkage type.

### Recommended Triplets

| Triplet | Platform | Linkage | Use Case |
| --------- | ---------- | --------- | ---------- |
| **x64-windows** | Windows x64 | Dynamic | Development, quick iteration |
| **x64-windows-static** | Windows x64 | Static | Release builds, no DLL dependencies |
| **x64-windows-static-md** | Windows x64 | Static libs, `/MD` runtime | ExplorerLens.io preferred |

### Using Custom Triplet

ExplorerLens.io uses `/MD` runtime (Multi-threaded DLL), so we need a custom triplet:

#### Create Custom Triplet

Create `triplets\x64-windows-static-md.cmake`:

```cmake
set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
```

#### Install with Custom Triplet

```powershell
# Install using custom triplet
vcpkg install libwebp:x64-windows-static-md

# Or set default triplet
$env:VCPKG_DEFAULT_TRIPLET = "x64-windows-static-md"
vcpkg install libwebp
```

---

## Troubleshooting

### Issue: vcpkg Not Found

```text
Error: vcpkg command not found
```

**Solution:**

```powershell
# Add vcpkg to PATH
$vcpkgPath = "$env:USERPROFILE\vcpkg"
$env:PATH += ";$vcpkgPath"

# Or set VCPKG_ROOT
$env:VCPKG_ROOT = "$env:USERPROFILE\vcpkg"
```

### Issue: Git Clone Fails

```text
fatal: unable to access 'https://github.com/Microsoft/vcpkg.git/': 
Failed to connect to github.com port 443
```

**Solution:**

```powershell
# Check network connection
Test-NetConnection github.com -Port 443

# Use proxy if needed
$env:HTTP_PROXY = "http://proxy.company.com:8080"
$env:HTTPS_PROXY = "http://proxy.company.com:8080"

# Or use SSH instead of HTTPS
git clone git@github.com:Microsoft/vcpkg.git

# Or download ZIP and extract
Invoke-WebRequest -Uri "https://github.com/microsoft/vcpkg/archive/refs/heads/master.zip" `
 -OutFile vcpkg-master.zip
Expand-Archive vcpkg-master.zip -DestinationPath $env:USERPROFILE
Rename-Item "$env:USERPROFILE\vcpkg-master" "$env:USERPROFILE\vcpkg"
```

### Issue: Bootstrap Fails

```text
Error: Failed to bootstrap vcpkg
```

**Solution:**

```powershell
# Ensure Visual Studio Build Tools installed
Get-Command cl.exe -ErrorAction SilentlyContinue

# If not found, install VS Build Tools
winget install Microsoft.VisualStudio.2022.BuildTools

# Run vcvarsall.bat first
& "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64

# Then bootstrap
.\bootstrap-vcpkg.bat
```

### Issue: CMake Can't Find vcpkg

```powershell
CMake Error: Could not find CMAKE_TOOLCHAIN_FILE
```

**Solution:**

```powershell
# Verify VCPKG_ROOT is set
echo $env:VCPKG_ROOT

# If not set, set it
$env:VCPKG_ROOT = "$env:USERPROFILE\vcpkg"

# Or use absolute path
cmake -B build -DCMAKE_TOOLCHAIN_FILE="C:\Users\YourName\vcpkg\scripts\buildsystems\vcpkg.cmake"
```

### Issue: Package Build Fails

```text
Error: Building package libwebp:x64-windows-static failed
```

**Solution:**

```powershell
# Clean build tree
vcpkg remove libwebp:x64-windows-static
vcpkg install libwebp:x64-windows-static --clean-after-build

# Check logs
Get-Content "$env:VCPKG_ROOT\buildtrees\libwebp\*.log" | Select-Object -Last 50

# Update vcpkg
cd $env:VCPKG_ROOT
git pull
.\bootstrap-vcpkg.bat -disableMetrics
```

---

## vcpkg Commands Reference

### Installation & Management

```powershell
# Install package
vcpkg install <package>:x64-windows-static

# Remove package
vcpkg remove <package>:x64-windows-static

# Upgrade all packages
vcpkg upgrade --no-dry-run

# List installed packages
vcpkg list

# Search for packages
vcpkg search <keyword>

# Show package info
vcpkg info <package>
```

### Maintenance

```powershell
# Update vcpkg itself
cd $env:VCPKG_ROOT
git pull
.\bootstrap-vcpkg.bat

# Clean build artifacts
vcpkg clean

# Export installed packages
vcpkg export --zip libwebp libavif --output=darkthumbs-deps

# Import packages
vcpkg import darkthumbs-deps.zip
```

---

## Integration with ExplorerLens.io Build

### Option A: Use vcpkg Packages (Recommended for Development)

```powershell
# 1. Install dependencies via vcpkg
.\build-scripts\Setup-Vcpkg.ps1 -InstallPackages

# 2. Configure CMake with vcpkg toolchain
cmake -B build-vcpkg `
 -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" `
 -DCMAKE_BUILD_TYPE=Release

# 3. Build
cmake --build build-vcpkg --config Release

# 4. Run tests
ctest --test-dir build-vcpkg -C Release
```

### Option B: Use Source-Built Libraries (Current Approach for Release)

```powershell
# Build external libraries from source
.\build-scripts\external-libs\Build-LibWebP-NMake.ps1
.\build-scripts\external-libs\Build-LibAvif.ps1
.\build-scripts\external-libs\build-lzma-sdk-24.08.ps1

# Build ExplorerLens.io (standard CMake)
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

---

## Future Enhancements

### Planned vcpkg Improvements

- [ ] **CMakePresets.json:** Pre-configured presets for vcpkg builds
- [ ] **Custom Registry:** Private vcpkg registry for internal libraries
- [ ] **Binary Caching:** Cache compiled libraries for faster builds
- [ ] **CI/CD Integration:** Automated vcpkg dependency updates
- [ ] **Version Constraints:** Lock specific library versions for stability

### Binary Caching

```powershell
# Enable binary caching (future)
$env:VCPKG_BINARY_SOURCES = "clear;files,$env:USERPROFILE\vcpkg-cache,readwrite"

# Or use Azure NuGet feed
$env:VCPKG_BINARY_SOURCES = "clear;nuget,https://pkgs.dev.azure.com/...;readwrite"
```

---

## Additional Resources

- **vcpkg Official Docs:** <https://vcpkg.io/en/docs/README.html>
- **vcpkg GitHub:** <https://github.com/microsoft/vcpkg>
- **Package Search:** <https://vcpkg.io/en/packages.html>
- **Triplet Reference:** <https://vcpkg.io/en/docs/users/triplets.html>

---

## Version Compatibility

| ExplorerLens.io | vcpkg Baseline | CMake | Notes |
| ------------ | ---------------- | ------- | ------- |
| 7.0.0 | 2024.02.14 | 3.20+ | vcpkg manifest mode |
| 6.2.0 | N/A | 3.20+ | No vcpkg support |

---

**ExplorerLens.io v39.9.0 - vcpkg Setup Guide**
_Last Updated: May 2026_
_For packaging and installer creation, see: [INSTALLATION_GUIDE.md](INSTALLATION_GUIDE.md)_
