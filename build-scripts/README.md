# Build Scripts Directory

This directory contains all build automation scripts for the DarkThumbs project.

## Directory Structure

```
build-scripts/
├── external-libs/       # External library build scripts (libwebp, libjxl, libraw, etc.)
├── library-builders/    # Consolidated library builders
├── production/          # Production build orchestrators
├── utilities/           # Build utilities and helpers
├── validation/          # Build validation and verification scripts
├── archive/             # Archived/deprecated build scripts
├── build.ps1            # Main build script (Standard builds)
├── Find-MSBuild.ps1     # MSBuild detection utility
├── Test-Build-Environment.ps1  # Environment verification
└── README.md            # This file
```

## Quick Start

### Standard Build
```powershell
.\build.ps1 -Configuration Release
```

### Production Build (All Libraries)
```powershell
.\production\Build-Production-SlowMachine.ps1 -Clean
```

### Build Individual External Library
```powershell
.\external-libs\Build-LibWebP-NMake.ps1
.\external-libs\build-libjxl.ps1
```

### Verify Build Environment
```powershell
.\Test-Build-Environment.ps1
```

## Build Scripts Organization

### Core Build Scripts

- **build.ps1** - Main build orchestrator for DarkThumbs solution
- **build-cbxshell-quick.ps1** - Quick incremental build
- **Find-MSBuild.ps1** - Locate MSBuild.exe and Visual Studio
- **Find-All-Tools.ps1** - Discover all required build tools
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
- **darkthumbs.ps1** - DarkThumbs command-line interface
- **Enable-DarkThumbsDiagnostics.ps1** - Enable diagnostic logging

### Validation (`validation/`)

Build verification and testing:

- **Validate-Build.ps1** - Comprehensive build validation
- **Simple-Validate.ps1** - Quick validation checks
- **check-tools.ps1** - Tool availability verification

## Build Requirements

### Required Tools

- **Visual Studio 2022** (17.8+) with VC++ 143 toolset
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

- `DARKTHUMBS_ROOT` - Project root directory (auto-detected)
- `DARKTHUMBS_BUILD_TYPE` - Override configuration (Debug/Release)
- `DARKTHUMBS_EXTERNAL_LIBS` - External libraries directory
- `CMAKE_GENERATOR` - CMake generator (default: Visual Studio 17 2022)

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
# Install Visual Studio 2022 if missing
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

**Last Updated:** February 11, 2026  
**Maintainer:** DarkThumbs Development Team
