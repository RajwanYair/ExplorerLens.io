# DarkThumbs - Build Requirements (64-bit Only)

## Platform

- **Architecture:** x64 (64-bit) ONLY - 32-bit (Win32) support removed
- **Operating System:** Windows 10 Build 19041+ or Windows 11
- **Visual Studio:** Build Tools 2026 (v180 platform toolset)
- **Windows SDK:** 10.0.26100.0 (Windows 11 SDK)

## Build Tools Status (Verified: 2026-01-06)

| Tool | Version | Source | Status |
|------|---------|--------|--------|
| CMake | 4.2.1 | scoop | ✅ Up-to-date |
| Ninja | 1.13.2 | scoop | ✅ Up-to-date |
| MSBuild | 18.3.0 | VS BuildTools 2026 | ✅ Up-to-date |
| Git | 2.52.0 | scoop | ✅ Up-to-date |
| LLVM/Clang | 21.1.8 | scoop | ✅ Up-to-date |
| 7zip | 25.01 | scoop | ✅ Up-to-date |

## Optimizations Applied

### Performance-Focused Build Settings

All Release builds use maximum speed optimizations:

- `/O2` - Maximize speed
- `/Ot` - Favor fast code
- `/Oi` - Enable intrinsic functions
- `/GL` - Whole program optimization
- `/LTCG` - Link-time code generation
- `/MT` - Multi-threaded static runtime
- SSE2 instruction set enabled
- Fast floating-point model

### 64-bit Native Benefits

- Larger address space for processing large archives
- Native 64-bit integer operations
- Better register allocation
- Improved SIMD performance
- No WoW64 overhead

## ✅ Enhancements Applied

### 1. Modern Windows 11 Interfaces

- **IThumbnailProvider** - Modern Windows 10/11 thumbnail interface
- **IInitializeWithStream** - Stream-based file loading for better performance
- Maintains backward compatibility with IExtractImage2

### 2. Dark Mode Support

- Automatic system theme detection via registry
- Adaptive thumbnail backgrounds (white in light mode, dark gray in dark mode)
- High contrast mode detection
- Theme-aware border colors

### 3. Modern C++17 Features

- `std::filesystem` for path operations
- Smart file time handling with modern APIs
- Better error handling with `std::optional`
- Type-safe conversions

### 4. Security Enhancements (Already in vcxproj)

- Control Flow Guard (CFG)
- Address Space Layout Randomization (ASLR)
- Data Execution Prevention (DEP)
- SDL Security Checks
- Stack protection

### 5. DPI Awareness

- Per-Monitor V2 DPI awareness manifests
- Dynamic thumbnail scaling
- Windows 11 compatibility declarations

## ⚠️ Build Tool Requirements

### Why MSVC is Required

This project **cannot** be built with free/open-source compilers like MinGW or Clang alone because:

1. **ATL (Active Template Library)** - Microsoft-proprietary library for COM development
   - `atlbase.h`, `atlcom.h`, `atlwin.h` - Not available in MinGW
   - Core to the shell extension architecture

2. **COM/Shell Integration** - Windows-specific APIs with Microsoft-specific implementations
   - Shell extension interfaces require MSVC-specific COM support
   - Resource compilation (.rc files) needs Microsoft RC compiler

3. **WTL (Windows Template Library)** - Built on top of ATL
   - Requires ATL headers and libraries
   - Not portable to non-Microsoft compilers

### Required Microsoft Tools

To build this project, you need **one** of the following:

#### Option 1: Visual Studio 2022 (Recommended)

- **Free Edition**: Visual Studio 2022 Community (free for individuals and small teams)
- **Components Needed**:
  - Desktop development with C++
  - Windows 11 SDK (10.0.22621.0 or later)
  - ATL for latest v143 build tools
  - C++ CMake tools (optional, for CMake build)

Download: <https://visualstudio.microsoft.com/vs/community/>

#### Option 2: Visual Studio Build Tools 2022 (Minimal, Free)

- **Free Download**: Build Tools for Visual Studio 2022
- **Components Needed**:
  - MSVC v143 - VS 2022 C++ x64/x86 build tools
  - Windows 11 SDK
  - C++ ATL for latest v143 build tools

Download: <https://visualstudio.microsoft.com/downloads/> (scroll to "Tools for Visual Studio")

### Installation via WinGet (Automated)

```cmd
REM Install Visual Studio Build Tools with required components
winget install Microsoft.VisualStudio.2022.BuildTools --override "--add Microsoft.VisualStudio.Component.VC.Tools.x86.x64 --add Microsoft.VisualStudio.Component.Windows11SDK.22621 --add Microsoft.VisualStudio.Component.VC.ATL --passive --wait"
```

## 🔧 Build Instructions

### Using Visual Studio 2022 (GUI)

1. Open `CBXShell.sln` in Visual Studio 2022
2. Select configuration: `Release | x64`
3. Build → Build Solution (Ctrl+Shift+B)
4. Output: `x64/Release/CBXShell.dll` and `CBXManager.exe`

### Using MSBuild (Command Line)

```cmd
REM Setup Visual Studio environment
"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

REM Build with MSBuild
msbuild CBXShell.sln /p:Configuration=Release /p:Platform=x64 /m
```

### Using CMake + Ninja (Modern Approach)

```cmd
REM Setup Visual Studio environment first
"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

REM Or for Build Tools:
"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

REM Configure and build
mkdir build && cd build
cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE=Release
ninja
```

## 📦 What We Improved

| Feature | Before | After | Benefit |
|---------|--------|-------|---------|
| **Thumbnail Interface** | IExtractImage2 (legacy) | IThumbnailProvider + IInitializeWithStream | Better Windows 11 integration, async support |
| **Dark Mode** | None | Full system theme detection | Looks native in Windows 11 dark mode |
| **C++ Standard** | Legacy/C++14 | C++17 with std::filesystem | Modern, safer code |
| **DPI Awareness** | None | Per-Monitor V2 | Sharp thumbnails on 4K displays |
| **Security** | Basic | CFG, ASLR, DEP, SDL | Hardened against exploits |
| **Build System** | VS 2019 only | VS 2022 + CMake option | Modern tools, CI/CD ready |

## 📝 Files Modified

### New Files Created

- `CBXShell/DarkModeHelper.h` - Dark mode detection utilities
- `CBXShell/ModernCppHelper.h` - C++17 filesystem wrappers
- `build-scoop.cmd` - Build script for Scoop tools
- `build-mingw.cmd` - Build script attempt (requires MSVC)
- `OPENSOURCE_BUILD.md` - Open-source build documentation
- `WINDOWS11_IMPROVEMENTS.md` - Improvement plan
- `CHANGELOG_MODERNIZATION.md` - Change log

### Modified Files

- `CBXShell/CBXShellClass.h` - Added IThumbnailProvider, IInitializeWithStream
- `CBXShell/CBXShellClass.cpp` - Implemented modern interfaces
- `CBXShell/cbxArchive.h` - Dark mode support, C++17 filesystem
- `CBXShell/targetver.h` - Updated to Windows 10/11
- `CBXShell/CBXShell.vcxproj` - VS 2022, C++17, security flags
- `CBXManager/CBXManager.vcxproj` - VS 2022, C++17, security flags
- `CBXShell/CBXShell.manifest` - DPI awareness
- `CBXManager/CBXManager.manifest` - DPI awareness + elevation

## ✨ Testing the Enhancements

### After Building

1. **Register Shell Extension** (as Administrator):

   ```cmd
   regsvr32 /i "x64\Release\CBXShell.dll"
   ```

2. **Test Dark Mode**:
   - Settings → Personalization → Colors → Choose your mode → Dark
   - Open File Explorer, navigate to folder with EPUB/CBZ files
   - Thumbnails should have dark backgrounds

3. **Test DPI Scaling**:
   - Settings → Display → Scale → 150% or 200%
   - Thumbnails should be sharp, not blurry

4. **Test Windows 11 Integration**:
   - Right-click on EPUB/CBZ file
   - Hover for thumbnail preview (modern IThumbnailProvider in action)

## 🎯 Next Steps

1. **Install Visual Studio 2022** (Community or Build Tools)
2. **Build the Project** using one of the methods above
3. **Test All Features** to verify improvements work
4. **Optional**: Set up CI/CD with GitHub Actions using hosted Windows runners

## 💡 Why We Can't Use Only Free Tools

While we use free Scoop tools (CMake, Ninja, Git), the **compiler** must be MSVC because:

- **ATL is proprietary** to Microsoft (no open-source equivalent)
- **Shell extensions** are deeply integrated with Windows COM architecture
- **Rewriting without ATL** would require months of work and break existing functionality

**Good News**: Visual Studio Community and Build Tools are **completely free** for:

- Individual developers
- Open source projects
- Academic use
- Small teams (<5 people)

So while not "Scoop-installable", they are still free and legal to use!

## 📚 Additional Resources

- [Windows Shell Extensions](https://docs.microsoft.com/en-us/windows/win32/shell/shell-exts)
- [IThumbnailProvider Interface](https://docs.microsoft.com/en-us/windows/win32/api/thumbcache/nn-thumbcache-ithumbnailprovider)
- [High DPI in Windows](https://docs.microsoft.com/en-us/windows/win32/hidpi/high-dpi-desktop-application-development-on-windows)
- [ATL Documentation](https://docs.microsoft.com/en-us/cpp/atl/atl-com-desktop-components)
