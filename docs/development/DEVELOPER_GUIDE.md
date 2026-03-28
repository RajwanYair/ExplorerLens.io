# ExplorerLens Developer Guide
**Version:** 23.5.0 "Vega-V" 
**Last Updated:** March 27, 2026 
**Target Audience:** Contributors, Plugin Developers, Maintainers 
**Build Status:** 0 errors, 0 warnings (Release x64)

## Table of Contents
- [Architecture Overview](#architecture-overview)
- [Development Setup](#development-setup)
- [Building from Source](#building-from-source)
- [Code Structure](#code-structure)
- [Contributing Guidelines](#contributing-guidelines)
- [Testing](#testing)
- [Debugging](#debugging)

---

## Architecture Overview

### High-Level Architecture

```
┌─────────────────────────────────────────────────────┐
│ Windows Explorer (Shell) │
└─────────────────┬───────────────────────────────────┘
 │ IThumbnailProvider Interface
 ▼
┌─────────────────────────────────────────────────────┐
│ LENSShell.dll (Shell Extension) │
│ ┌────────────────────────────────────────────────┐ │
│ │ LENSShellClass (IThumbnailProvider impl) │ │
│ │ - COM Registration │ │
│ │ - IThumbnailProvider::GetThumbnail() │ │
│ │ - SEH Exception Handling │ │
│ └─────────────────┬──────────────────────────────┘ │
└────────────────────┼────────────────────────────────┘
 │ Engine API
 ▼
┌─────────────────────────────────────────────────────┐
│ ExplorerLensEngine.lib (Core Engine) │
│ ┌────────────────────────────────────────────────┐ │
│ │ ImageEngine (Main API) │ │
│ │ - InitializeForThumbnails() │ │
│ │ - CreateThumbnailFromFile() │ │
│ └─────────────────┬──────────────────────────────┘ │
│ │ │
│ ┌────────────────┼──────────────────────────────┐ │
│ │ Decoder Pipeline │ │
│ │ ├─ Archive Decoders (ZIP, RAR, 7Z, CBZ/CBR) │ │
│ │ ├─ Image Decoders (JPEG, PNG, WebP, JXL │ │
│ │ ├─ RAW Decoders (CR2, NEF, ARW via LibRaw) │ │
│ │ ├─ Video Decoders (MP4, MKV via DirectShow) │ │
│ │ └─ Audio Decoders (MP3, FLAC album art) │ │
│ └─────────────────┬──────────────────────────────┘ │
│ │ │
│ ┌────────────────┼──────────────────────────────┐ │
│ │ GPU Acceleration (D3D11) │ │
│ │ - SIMDScaler (AVX2 optimized resizing) │ │
│ │ - D3D11TextureRenderer (GPU compositing) │ │
│ │ - Shader compilation & caching │ │
│ └───────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────┘
 │
 ▼
┌─────────────────────────────────────────────────────┐
│ External Libraries (Static linking) │
│ - zlib 1.3.1, zstd 1.5.7, LZ4 1.10.0, LZMA 26.00 │
│ - minizip-ng 4.0.10, UnRAR 7.2.2 │
│ - libwebp 1.5.0, libjxl 0.11.1 │
│ - LibRaw 0.21.3 (RAW photos) │
│ - DirectX 11, Windows WIC │
└─────────────────────────────────────────────────────┘
```

### Key Components

1. **LENSShell.dll** - Windows Shell Extension (COM DLL)
 - Implements `IThumbnailProvider` COM interface
 - Handles Explorer integration
 - SEH exception protection wrapper
 - Delegates to `ExplorerLensEngine.lib`

2. **ExplorerLensEngine.lib** - Core thumbnail engine (static library)
 - Format detection and dispatching
 - Decoder implementations
 - GPU acceleration via DirectX 11
 - SIMD-optimized image processing
 - Circuit breaker pattern for failing decoders

3. **LENSManager.exe** - Configuration utility (WinUI 3 / Win32)
 - Handler registration/unregistration
 - Cache management
 - GPU selection
 - Statistics and diagnostics

---

## Development Setup

### Prerequisites

Install using the provided script:

```powershell
.\scripts\Setup-DevEnvironment.ps1
```

Or manually install:

- **Visual Studio 2026** (18.x+) with:
 - C++ desktop development workload
 - Windows 11 SDK (10.0.22621.0+)
 - CMake tools

- **Build Tools:**
 - CMake 3.28+
 - Ninja 1.11+
 - PowerShell 7+

- **Compilers:**
 - MSVC v145+ (Visual Studio 2026)
 - Clang 17+ (optional, for LLVM builds)

- **Verification:**
```powershell
.\scripts\verify-tools.ps1
```

### Repository Structure

```
ExplorerLens/
├── Engine/ # Core thumbnail engine (C++20)
│ ├── Core/ # API surface, initialization
│ ├── Decoders/ # Format-specific decoders
│ ├── GPU/ # DirectX 11 acceleration
│ ├── Utils/ # Utilities, SIMD, profiling
│ └── CMakeLists.txt
│
├── LENSShell/ # Shell extension (COM DLL)
│ ├── LENSShellClass.cpp/.h # IThumbnailProvider impl
│ ├── LENSShell.idl # COM interface definition
│ └── LENSShell.vcxproj # MSBuild project
│
├── LENSManager/ # Configuration utility
│ ├── MainDlg.cpp/.h # Main dialog
│ ├── DarkModeHelper.h # Dark mode support
│ └── LENSManager.vcxproj
│
├── external/ # Third-party libraries
│ ├── compression-libs/ # zlib, zstd, lz4, lzma, minizip-ng
│ ├── archive-libs/ # unrar
│ ├── image-libs/ # libwebp, libjxl
│ └── camera-libs/ # LibRaw
│
├── build-scripts/ # Build automation
│ ├── external-libs/ # Library build scripts
│ ├── production/ # Production build pipelines
│ └── Build-With-Monitoring.ps1
│
├── tests/ # Unit tests (Google Test)
├── docs/ # Documentation
├── packaging/ # WiX installer
└── CMakeLists.txt # CMake entry point
```

---

## Building from Source

### ⭐ Recommended: Complete Build (NEW in v7.0)

```powershell
# Complete build: dependencies, engine, solution, and MSI installer
.\build-scripts\Build-All-And-Package.ps1 -Configuration Release -Version 7.0.0

# Skip dependencies if already built (faster)
.\build-scripts\Build-All-And-Package.ps1 -SkipDependencies

# Clean build (rebuild from scratch)
.\build-scripts\Build-All-And-Package.ps1 -Clean

# Or use Visual Studio task
# Ctrl+Shift+B → "Build Release (Standard)"
```

### Quick Build (Standard - Legacy)

```powershell
# Full build with dependencies (uses old build system)
.\scripts\build.ps1 -Configuration Release

# Or use Visual Studio task
# Ctrl+Shift+B → "Build Release (Standard)"
```

### Step-by-Step Build (Manual Control)

#### 0. Setup vcpkg (Optional - NEW in v7.0)

```powershell
# vcpkg can be used for future dependency management
.\build-scripts\Setup-Vcpkg.ps1

# With package installation
.\build-scripts\Setup-Vcpkg.ps1 -InstallPackages

# Note: ExplorerLens currently builds most libraries from source
```

#### 1. Build External Libraries

**NEW v7.0 Refactored Scripts (Recommended):**

```powershell
# Using new unified build modules (50% less code)
.\build-scripts\external-libs\Build-LibWebP-NMake.ps1 -Configuration Release -Clean
.\build-scripts\external-libs\Build-MinizipNG.ps1 -Configuration Release -Clean
.\build-scripts\external-libs\build-libjxl.ps1 -Configuration Release -Clean
.\build-scripts\external-libs\build-libavif.ps1 -Configuration Release -Clean

# Remaining libraries (legacy scripts - being refactored)
.\build-scripts\external-libs\build-lzma-sdk-26.00.ps1
.\build-scripts\external-libs\Build-Zlib.ps1
.\build-scripts\external-libs\Build-Zstd.ps1
```

**Legacy Method:**

```powershell
# Build all external libraries with /MD runtime (4-6 hours)
.\build-scripts\Rebuild-All-With-MD.ps1 -Clean

# Or build individually (old scripts)
.\build-scripts\external-libs\build-lzma-sdk-26.00.ps1
.\build-scripts\external-libs\Build-LibWebP-NMake.ps1
.\build-scripts\external-libs\Build-MinizipNG.ps1
```

> **💡 TIP:** Use the new `Build-All-And-Package.ps1` script to build everything automatically.

#### 2. Build Engine (CMake + Ninja)

```powershell
# Configure
cmake -S . -B build -G Ninja `
 -DCMAKE_BUILD_TYPE=Release `
 -DCMAKE_CXX_COMPILER=clang++ `
 -DCMAKE_C_COMPILER=clang

# Build
cmake --build build --config Release --target ExplorerLensEngine -j 8

# Output: build/lib/Release/ExplorerLensEngine.lib
```

#### 3. Build LENSShell (MSBuild)

```powershell
msbuild LENSShell.sln /p:Configuration=Release /p:Platform=x64 /t:LENSShell /m

# Output: x64/Release/LENSShell.dll
```

#### 4. Build LENSManager

```powershell
msbuild LENSShell.sln /p:Configuration=Release /p:Platform=x64 /t:LENSManager /m

# Output: x64/Release/LENSManager.exe
```

### Build Configurations

| Configuration | Compiler | Optimization | Debug Info | Runtime |
|---------------|----------|--------------|------------|---------|
| Debug | MSVC | O0 | Full | /MDd |
| Release | MSVC/Clang | O2/O3 | None | /MD |
| RelWithDebInfo| MSVC | O2 | Full | /MD |

---

## Code Structure

### Engine API (Public Surface)

**Header:** `Engine/Core/EngineAPI.h`

```cpp
class ExplorerLensEngine {
public:
 static bool InitializeForThumbnails(HWND hwnd = nullptr);
 static bool CreateThumbnailFromFile(
 LPCWSTR pszPath,
 UINT cx, // Requested width
 HBITMAP* phBitmap,
 WTS_ALPHATYPE* pdwAlpha
 );
 static void Shutdown();
};
```

### Decoder Interface

All decoders implement:

```cpp
class IDecoder {
public:
 virtual ~IDecoder() = default;
 virtual bool CanDecode(const std::wstring& extension) = 0;
 virtual DecoderResult Decode(
 const std::wstring& filePath,
 uint32_t maxWidth,
 uint32_t maxHeight
 ) = 0;
};
```

### Adding a New Decoder

1. Create decoder class in `Engine/Decoders/`:

```cpp
// Engine/Decoders/PNGDecoder.h
class PNGDecoder : public IDecoder {
public:
 bool CanDecode(const std::wstring& extension) override;
 DecoderResult Decode(...) override;
private:
 // Implementation
};
```

2. Register in `Engine/Core/DecoderRegistry.cpp`:

```cpp
void DecoderRegistry::Initialize() {
 RegisterDecoder(std::make_unique<PNGDecoder>());
 // ... other decoders
}
```

3. Add tests in `tests/DecoderTests/`:

```cpp
TEST(PNGDecoderTest, DecodeValidPNG) {
 PNGDecoder decoder;
 auto result = decoder.Decode(L"test.png", 256, 256);
 ASSERT_TRUE(result.success);
 ASSERT_NE(result.bitmap, nullptr);
}
```

---

## Contributing Guidelines

### Code Style

- **C++ Standard:** C++20
- **Formatting:** clang-format (Google style)
- **Naming:**
 - Classes: `PascalCase`
 - Functions: `PascalCase`
 - Variables: `camelCase`
 - Constants: `UPPER_SNAKE_CASE`

### Pull Request Process

1. **Fork** the repository
2. **Create branch** from `main`:
 ```bash
 git checkout -b feature/amazing-decoder
 ```
3. **Implement** your changes
4. **Add tests** (required for new features)
5. **Run test suite**:
 ```powershell
 ctest --test-dir build --output-on-failure
 ```
6. **Build with zero warnings**:
 ```powershell
 msbuild /p:TreatWarningsAsErrors=true
 ```
7. **Submit PR** with:
 - Clear description
 - Test coverage report
 - Before/after performance metrics (if applicable)

### Commit Messages

Follow conventional commits:

```
feat: Add TIFF decoder with BigTIFF support
fix: Resolve memory leak in WebP decoder
docs: Update build instructions for Clang 17
perf: Optimize SIMD scaler with AVX-512
test: Add fuzzing tests for RAR archives
```

---

## Testing

### Unit Tests (Google Test)

```powershell
# Build tests
cmake --build build --target ExplorerLensTests

# Run all tests
ctest --test-dir build --output-on-failure

# Run specific test
.\build\tests\ExplorerLensTests.exe --gtest_filter=*PNGDecoder*
```

### Integration Tests

```powershell
# Test with real file corpus
.\tests\Integration-Tests.ps1

# Performance benchmarks
.\tests\Performance-Tests.ps1
```

### Memory Leak Detection

```powershell
# Enable CRT debug heap
$env:_NO_DEBUG_HEAP = 0
.\build\tests\ExplorerLensTests.exe

# Or use Application Verifier
appverif /verify handles locks heaps memory /for LENSShell.dll
```

---

## Debugging

### Debugging Shell Extension

Shell extensions run in Explorer process. Use:

```powershell
# Method 1: Attach to Explorer
# 1. Build Debug configuration
# 2. Open Visual Studio → Debug → Attach to Process → explorer.exe
# 3. Set breakpoint in LENSShellClass::GetThumbnail
# 4. Navigate to folder with target file in Explorer

# Method 2: Standalone test harness
.\tests\ShellExtensionTestHarness.exe "path\to\file.cbz"
```

### Debug Logging

Enable debug output:

```cpp
// Engine/Utils/DebugLog.h
#define ExplorerLens_DEBUG_LOG 1

// Logs to OutputDebugString, viewable in DebugView
LOG_DEBUG(L"Decoding file: %s", filePath.c_str());
LOG_ERROR(L"Failed to decode: %s (HRESULT: 0x%08X)", filePath.c_str(), hr);
```

View logs with **DebugView** (Sysinternals):

```powershell
# Download and run
.\DebugView.exe

# Filter: ExplorerLens*
```

### Performance Profiling

```powershell
# Visual Studio Profiler
# 1. Build RelWithDebInfo
# 2. Alt+F2 → Performance Profiler → CPU Usage
# 3. Attach to explorer.exe
# 4. Navigate to test folder

# Or use Tracy Profiler (integrated)
# Macro: PROFILE_FUNCTION() at function entry
```

---

## Additional Resources

- **Build Instructions:** [build-method.md](.github/standards/build-method.md)
- **Library Inventory:** [external/LIBRARY_INVENTORY.md](external/LIBRARY_INVENTORY.md)
- **User Guide:** [USER_GUIDE.md](USER_GUIDE.md)
- **Plugin SDK:** [SDK/docs/PLUGIN_SDK.md](SDK/docs/PLUGIN_SDK.md)

**Questions?** Open an issue on GitHub or contact maintainers.

---

**Happy coding! 🚀**
