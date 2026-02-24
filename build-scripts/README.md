# Build Scripts - ExplorerLens v15.0.0

**Unified Build System for Windows 11**  
Comprehensive build infrastructure for external dependencies, engine, and installer packaging.

> **⚡ NEW in v15.0.0:** Unified build modules, 50% code reduction, vcpkg integration

---

## 📁 Directory Structure

```
build-scripts/
├── core/                           # 🆕 Core build modules
│   ├── Build-Library-Core.ps1      #     Unified build functions (CMake, MSBuild, NMake)
│   └── Build-Helpers.ps1           #     vcpkg integration, Git helpers, environment
│
├── external-libs/                  # External library build scripts
│   ├── Build-LibWebP-NMake.ps1     #     ✅ Refactored (175 → 102 lines)
│   ├── Build-MinizipNG.ps1         #     ✅ Refactored (104 → 60 lines)
│   ├── build-libjxl.ps1            #     ✅ Refactored (150 → 90 lines)
│   ├── build-libavif.ps1           #     ✅ Refactored (150 → 80 lines)
│   └── [other libraries...]        #     🔄 Migration in progress
│
├── library-builders/               # Library orchestration scripts
├── production/                     # Production build orchestrators
├── utilities/                      # Build utilities and helpers
├── validation/                     # Build validation scripts
│   └── check-tools.ps1             #     Verify build environment
│
├── Build-All-And-Package.ps1       # 🆕 Complete build orchestrator (recommended)
├── Setup-Vcpkg.ps1                 # 🆕 vcpkg package manager setup
├── DEPRECATED.md                   # 🆕 List of deprecated scripts
├── Find-MSBuild.ps1                # ⚠️  Deprecated - use Build-Library-Core
├── Test-Build-Environment.ps1      #     Environment verification
└── README.md                       #     This file
```

---

## 🚀 Quick Start

### ⭐ Recommended: Complete Build
```powershell
# Build everything: dependencies, engine, solution, and MSI installer
.\build-scripts\Build-All-And-Package.ps1

# With options
.\build-scripts\Build-All-And-Package.ps1 -Configuration Release -Version 7.0.0 -Clean

# Skip dependencies (if already built)
.\build-scripts\Build-All-And-Package.ps1 -SkipDependencies

# Skip MSI packaging
.\build-scripts\Build-All-And-Package.ps1 -SkipPackaging
```

### Legacy: Individual Components
```powershell
# Build external dependencies (refactored)
.\build-scripts\external-libs\Build-LibWebP-NMake.ps1
.\build-scripts\external-libs\Build-MinizipNG.ps1
.\build-scripts\external-libs\build-libjxl.ps1
.\build-scripts\external-libs\build-libavif.ps1

# Build ExplorerLens Engine (CMake)
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# Build LENSShell solution (MSBuild)
msbuild LENSShell.sln /p:Configuration=Release /p:Platform=x64 /m

# Create MSI installer
.\packaging\Build-Installer.ps1 -Version 7.0.0
```

### Verify Build Environment
```powershell
.\build-scripts\validation\check-tools.ps1
```

---

## 📚 Core Module Reference (NEW in v15.0.0)

### Build-Library-Core.ps1

The unified build module provides consistent functions for all build systems.

#### Key Functions

##### `Invoke-CMakeBuild`
Build CMake-based libraries with automatic configuration and verification.

```powershell
. "$PSScriptRoot\..\core\Build-Library-Core.ps1"

$cmakeOptions = @{
    'CMAKE_BUILD_TYPE'  = 'Release'
    'BUILD_SHARED_LIBS' = 'OFF'
}

Invoke-CMakeBuild `
    -LibraryName "MyLibrary" `
    -SourceDir $sourceDir `
    -BuildDir $buildDir `
    -InstallDir $installDir `
    -Configuration $Configuration `
    -CMakeOptions $cmakeOptions `
    -Clean:$Clean
```

##### `Invoke-MSBuildLibrary` | `Invoke-NMakeBuild`
Build MSBuild projects or NMake projects with automatic tool discovery.

##### `Find-MSBuildPath`, `Find-CMakePath`, `Test-VisualStudioTools`
Automatically locate and verify build tools.

##### `Write-BuildLog`
Unified colored logging across all scripts.

```powershell
Write-BuildLog "Starting build..." -Level Info      # Blue
Write-BuildLog "Build successful!" -Level Success   # Green
Write-BuildLog "Warning!" -Level Warning             # Yellow
Write-BuildLog "Build failed!" -Level Error          # Red
```

### Build-Helpers.ps1

Additional helper functions for vcpkg, Git, and environment management.

```powershell
. "$PSScriptRoot\..\core\Build-Helpers.ps1"

# vcpkg integration
if (-not (Test-VcpkgInstalled)) {
    Install-VcpkgIfNeeded
}
Install-VcpkgPackage -PackageName "zlib:x64-windows-static"

# Git helpers
Initialize-GitSubmodules -Path $sourceDir

# Environment setup
Set-VisualStudioEnvironment -VSVersion "2022"
```

See [build-scripts/core/](./core/) for full documentation.

---

## 📖 Build Scripts Organization

### Core Build Scripts

- 🆕 **Build-All-And-Package.ps1** - Complete build orchestrator (recommended)
- 🆕 **Setup-Vcpkg.ps1** - Automated vcpkg setup and package installation
- **scripts/build.ps1** - Main build orchestrator for ExplorerLens solution
- **build-LENSShell-quick.ps1** - Quick incremental build
- ⚠️  **Find-MSBuild.ps1** - DEPRECATED - Use `Find-MSBuildPath` from Build-Library-Core
- **Test-Build-Environment.ps1** - Verify build prerequisites

### External Library Builders (`external-libs/`)

Scripts for building third-party dependencies:

- **Image Libraries**: LibWebP, LibJXL, LibHEIF, LibRaw
- **Compression**: zlib, zstd, LZ4, LZMA, minizip-ng, unrar
- **Video**: dav1d, libavif

Each script downloads source, applies patches, and builds for x64 Release.

### Library Builder Aggregators (`library-builders/`)

High-level scripts that orchestrate multiple library builds:

- **Build-All-External-Libraries.ps1** - Build all dependencies
- **Build-Critical-Libraries.ps1** - Build essential libraries only
- **Download-And-Build-Libraries.ps1** - Download + build workflow

### Production Scripts (`production/`)

Scripts for production builds and releases:

- **Build-Production-SlowMachine.ps1** - Full clean production build
- **rebuild-compression-libs.ps1** - Rebuild compression libraries
- **Rebuild-External-Libs-Correct-Runtime.ps1** - Fix runtime linkage

### Utilities (`utilities/`)

Build monitoring and diagnostic utilities:

- **Monitor-Build.ps1** - Real-time build monitoring
- **ExplorerLens.ps1** - ExplorerLens command-line interface
- **Enable-ExplorerLensDiagnostics.ps1** - Enable diagnostic logging

### Validation (`validation/`)

Build verification and testing:

- **Validate-Build.ps1** - Comprehensive build validation
- **Simple-Validate.ps1** - Quick validation checks
- **check-tools.ps1** - Tool availability verification

## Build Requirements

### Required Tools

- **Visual Studio 18 2026** (17.8+) with VC++ 143 toolset
- **CMake** 3.20+ (for external libraries)
- **NASM** 2.15+ (for libavif, dav1d)
- **PowerShell** 7.0+ (recommended) or 5.1+
- **Git** (for submodule management)

### Optional Tools

- **Ninja** (faster CMake builds)
- **ccache** (build caching)
- **Python 3.8+** (for some external library scripts)

## Build Configurations

### Debug
- Full debug symbols, no optimizations
- Runtime Library: `/MDd` (Multi-threaded Debug DLL)
- Used for development and debugging

### Release
- Optimizations: `/O2` (Maximize Speed)
- Runtime Library: `/MD` (Multi-threaded DLL)
- Whole Program Optimization: `/GL`
- Used for production builds

## Environment Variables

The build system respects these environment variables:

- `ExplorerLens_ROOT` - Project root directory (auto-detected)
- `ExplorerLens_BUILD_TYPE` - Override configuration (Debug/Release)
- `ExplorerLens_EXTERNAL_LIBS` - External libraries directory
- `CMAKE_GENERATOR` - CMake generator (default: Visual Studio 18 2026)

## Build Output

- **x64/Release/** - Compiled binaries (DLLs, EXEs)
- **build/** - CMake build files
- **build-logs/** - Build logs with timestamps
- **external/*/install/** - Installed external libraries

## Troubleshooting

### Common Issues

**"MSBuild not found"**
```powershell
.\Find-MSBuild.ps1  # Locate installation
# Install Visual Studio 18 2026 if missing
```

**"NASM not found"**
```powershell
# Download from https://www.nasm.us/
# Add to PATH or install via chocolatey:
choco install nasm
```

**"Library build failed"**
```powershell
# Check prerequisites
.\Test-Build-Environment.ps1

# View detailed logs
Get-Content build-logs\*.log | Select-Object -Last 50
```

**"Link errors with external libraries"**
```powershell
# Rebuild external libraries with correct runtime
.\production\Rebuild-External-Libs-Correct-Runtime.ps1
```

## CI/CD Integration

GitHub Actions workflows use these scripts:

- `.github/workflows/build.yml` - Standard CI build
- `.github/workflows/build-and-test.yml` - Build + tests
- `.github/workflows/release.yml` - Release builds

## Contributing

When adding new build scripts:

1. Place in appropriate subdirectory
2. Follow PowerShell naming conventions: `Verb-Noun.ps1`
3. Include script documentation (`.SYNOPSIS`, `.DESCRIPTION`)
4. Add error handling (`$ErrorActionPreference = "Stop"`)
5. Update this README

## See Also

- [Build Guide](../docs/build/BUILD_GUIDE.md) - Comprehensive build documentation
- [Development Guide](../docs/development/) - Development workflow
- [Third-Party Libraries](../docs/development/THIRD_PARTY.md) - Dependency information
- [Tool Discovery](../docs/development/TOOL_DISCOVERY.md) - Build tool detection

## Support

For build issues:
1. Check [Build Guide](../docs/build/BUILD_GUIDE.md)
2. Run diagnostics: `.\Test-Build-Environment.ps1`
3. Review logs in `build-logs/`
4. Open issue on GitHub with build log

---

**Last Updated:** February 16, 2026  
**Maintainer:** ExplorerLens Development Team

