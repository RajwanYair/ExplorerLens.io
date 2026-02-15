# DarkThumbs Build System - Tool Versions & Paths
**Last Updated:** February 9, 2026 (Updated: Tools refreshed)  
**Machine:** Intel Development Workstation  
**Environment:** Windows 11 + VS 2026 Build Tools

---

## ✅ Installed Build Tools (Verified & Updated)

### Compiler Toolchain
- **Visual Studio:** 2026 Build Tools (Version 18)
- **MSVC Compiler:** 14.44.35207
- **MSBuild:** 18.3.0-preview-25570-20
- **Windows SDK:** 10.0.26100.0
- **Platform Toolset:** v144 (VS 2026)

### Build Systems
- **CMake:** 4.2.3 (Updated from 4.2.1) ✅
- **Ninja:** 1.13.2 (via Scoop)
- **NMake:** (included with VS Build Tools)

### Version Control & Utilities
- **Git:** 2.53.0 (Updated from 2.52.0) ✅
- **Python:** (Microsoft Store version)

---

## 📁 Installation Paths (Hard-Coded)

All paths verified and working as of February 9, 2026:

```powershell
# Visual Studio Build Tools
VS_PATH = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools"
MSVC_PATH = "$VS_PATH\VC\Tools\MSVC\14.44.35207"
VCVARS_ALL = "$VS_PATH\VC\Auxiliary\Build\vcvarsall.bat"
VCVARS_64 = "$VS_PATH\VC\Auxiliary\Build\vcvars64.bat"

# MSBuild
MSBUILD = "$VS_PATH\MSBuild\Current\Bin\amd64\MSBuild.exe"

# Scoop Tools (via shims)
SCOOP_ROOT = "C:\Users\ryair\scoop"
CMAKE = "$SCOOP_ROOT\shims\cmake.exe"
GIT = "$SCOOP_ROOT\shims\git.exe"
NINJA = "$SCOOP_ROOT\shims\ninja.exe"

# Windows SDK
SDK_PATH = "C:\Program Files (x86)\Windows Kits\10"
SDK_VERSION = "10.0.26100.0"
```

---

## 🚀 Instant Environment Setup (NEW!)

### Automatic Setup in Every PowerShell Session

Add to your PowerShell profile (`$PROFILE`):

```powershell
# DarkThumbs Development Environment - Auto-load
$darkThumbsEnv = "C:\Users\ryair\OneDrive - Intel Corporation\Documents\MyScripts\DarkThumbs\scripts\Setup-DevEnvironment.ps1"
if (Test-Path $darkThumbsEnv) {
    . $darkThumbsEnv
}
```

**What this does:**
- ✅ Loads MSVC environment (CL, Link, NMake, RC)
- ✅ Verifies all build tools are accessible
- ✅ Sets up convenient aliases (dtbuild, dttest, dtclean)
- ✅ Configures PATH for immediate use
- ✅ No manual tool searching every build!

### Manual Load (if not in profile)

```powershell
cd "C:\Users\ryair\OneDrive - Intel Corporation\Documents\MyScripts\DarkThumbs"
.\scripts\Setup-DevEnvironment.ps1
```

### Verify Environment

```powershell
# Check what's loaded
Show-DarkThumbsInfo

# Test tools
Test-BuildTools

# See build commands
dtbuild
```

---

## 🛠️ Quick Build Commands (After Environment Setup)

Once environment is loaded, use these shortcuts:

```powershell
# Build full solution (Release)
dtbuild Release

# Build Engine only (CMake)
dtbuild Engine

# Clean all artifacts
dtbuild Clean

# Rebuild from scratch
dtbuild Rebuild

# Run tests
dttest
```

---

## 🔧 Build Tool Commands (Direct)

If you need to run tools directly without shortcuts:

### MSBuild (VS 2026)

```powershell
# Build full solution
msbuild CBXShell.sln /p:Configuration=Release /p:Platform=x64 /m /v:minimal

# Build specific project
msbuild CBXShell\CBXShell.vcxproj /p:Configuration=Release /p:Platform=x64 /m

# Rebuild (clean build)
msbuild CBXShell.sln /t:Rebuild /p:Configuration=Release /p:Platform=x64 /m

# With logging
msbuild CBXShell.sln /p:Configuration=Release /p:Platform=x64 /fl /flp:LogFile=build.log /v:minimal
```

### CMake (Engine)

```powershell
# Configure (Visual Studio 2026 generator)
cd Engine
cmake -B build -S . -G "Visual Studio 18 2026" -A x64

# Build
cmake --build build --config Release -j 8

# With tests
cmake --build build --config Release -j 8
cd build
ctest --output-on-failure
```

### Git

```powershell
# Common commands
git status
git add .
git commit -m "message"
git log --oneline --graph -10
```

---

## 📋 CMake Generators Available

```powershell
cmake --help

Available generators:
* Visual Studio 18 2026        - VS 2026 (default)
  Visual Studio 17 2022        - VS 2022 (not installed)
  Visual Studio 16 2019        - VS 2019 (not installed)
  NMake Makefiles              - NMake (requires MSVC env)
  Ninja                        - Ninja (fast, recommended for CI)
  Unix Makefiles               - Make (WSL/MSYS2)
```

**Recommended for DarkThumbs:**
- **Local Development:** `Visual Studio 18 2026` (best IDE integration)
- **CI/CD:** `Ninja` (fastest compile times)
- **Command Line:** `NMake Makefiles` (simple, no IDE needed)

---

## 🧪 Testing Build Environment

### Verify All Tools

```powershell
# Load environment first
.\scripts\Setup-DevEnvironment.ps1

# Or if auto-loaded in profile:
Test-BuildTools

# Expected output:
#   ✅ MSBuild (18.3.0)
#   ✅ CMake (4.2.1)
#   ✅ Git (2.x.x)
#   ✅ MSVC (CL) (19.44.x)
#   ✅ NMake
#   ✅ Link
#   ✅ RC
#   ✅ Ninja (1.x.x)
```

### Verify MSVC Environment

```powershell
# Check compiler
cl.exe
# Should output: Microsoft (R) C/C++ Optimizing Compiler Version 19.44.35207

# Check linker
link.exe
# Should output: Microsoft (R) Incremental Linker Version 14.44.35207

# Check nmake
nmake /?
# Should output: Microsoft (R) Program Maintenance Utility Version 14.44.35207
```

### Verify CMake Generator

```powershell
cd Engine
cmake -B build -S . -G "Visual Studio 18 2026" -A x64

# Should output:
# -- The CXX compiler identification is MSVC 19.44.35207.0
# -- Detecting CXX compiler ABI info - done
# -- Configuring done
# -- Generating done
```

---

## 🐛 Troubleshooting

### Problem: "CL.exe not found" or "NMAKE not found"

**Solution:**
```powershell
# Manually load MSVC environment
Load-MSVCEnvironment

# Or reload full environment
Setup-DarkThumbsEnv -Force
```

### Problem: "Generator 'Visual Studio 18 2026' not found"

**Solution:**
Use explicit path to MSBuild:
```powershell
cmake -B build -S . -G "Visual Studio 18 2026" -A x64 `
    -DCMAKE_MAKE_PROGRAM="C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\MSBuild\Current\Bin\amd64\MSBuild.exe"
```

### Problem: Tools not persisting across sessions

**Solution:**
Add environment setup to PowerShell profile:
```powershell
# Edit profile
notepad $PROFILE

# Add this line:
. "C:\Users\ryair\OneDrive - Intel Corporation\Documents\MyScripts\DarkThumbs\scripts\Setup-DevEnvironment.ps1"

# Save and restart PowerShell
```

### Problem: Git/CMake not found

**Solution:**
Ensure Scoop shims are in PATH:
```powershell
$env:PATH = "C:\Users\ryair\scoop\shims;$env:PATH"
```

---

## 📦 Dependencies Status

All external libraries are pre-built and included in `external/` directory:

- ✅ **libwebp** - Google WebP codec
- ✅ **minizip-ng** - ZIP archive support
- ✅ **zlib** - Compression library
- ✅ **zstd** - Fast compression
- ✅ **libjxl** - JPEG XL support
- ✅ **libheif** - HEIF/HEIC support
- ✅ **libavif** - AVIF support
- ✅ **dav1d** - AV1 decoder
- ✅ **aom** - AOMedia codec
- ✅ **libraw** - RAW camera formats (in progress)

**No additional downloads required for standard builds.**

---

## 🔄 Tool Update Instructions

### Update CMake

```powershell
scoop update cmake
```

### Update Git

```powershell
scoop update git
```

### Update Visual Studio Build Tools

Use Visual Studio Installer:
```powershell
# Launch installer
& "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vs_installer.exe"

# Or via command line
vs_installer.exe modify --installPath "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools" --update --quiet
```

---

## 📝 Version History

| Date | MSVC | MSBuild | CMake | Git | Notes |
|------|------|---------|-------|-----|-------|
| 2026-02-09 PM | 14.44.35207 | 18.3.0 | 4.2.3 | 2.53.0 | Tools updated via Scoop |
| 2026-02-09 AM | 14.44.35207 | 18.3.0 | 4.2.1 | 2.52.0 | All tools verified |
| 2026-01-08 | 14.44.x | 18.3.0 | 4.1.x | 2.5x.x | Previous baseline |

---

## ⚡ Performance Tips

1. **Use Ninja for fast builds:**
   ```powershell
   cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
   cmake --build build -j 8
   ```

2. **Enable parallel compilation:**
   ```powershell
   msbuild /m /p:CL_MPCount=8
   ```

3. **Use ccache (if installed):**
   ```powershell
   cmake -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
   ```

4. **Incremental builds:**
   ```powershell
   # Don't clean unless necessary
   msbuild /t:Build  # (not /t:Rebuild)
   ```

---

*This document is auto-generated from tool discovery. Last verified: February 9, 2026.*
