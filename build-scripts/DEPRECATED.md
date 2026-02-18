# Deprecated Build Scripts

**Date:** February 18, 2026  
**Version:** DarkThumbs v7.1.0  
**Reason:** Consolidated into Build-Library-Core.ps1 module  
**Sprints:** 74 sprints completed

## Deprecated Scripts

The following scripts have been deprecated and replaced by the unified build system:

### 1. Find-MSBuild.ps1
- **Location:** `build-scripts/Find-MSBuild.ps1`
- **Replacement:** Use `Find-MSBuildPath` function from `core/Build-Library-Core.ps1`
- **Status:** ⚠️ Deprecated - Do not use in new code

### 2. external-libs/Find-MSBuild.ps1
- **Location:** `build-scripts/external-libs/Find-MSBuild.ps1`
- **Replacement:** Import `core/Build-Library-Core.ps1` module
- **Status:** ⚠️ Deprecated - Do not use in new code

### 3. Build-Zlib.ps1 (old version)
- **Location:** `build-scripts/external-libs/Build-Zlib.ps1`
- **Status:** 🔄 Needs refactoring to use Build-Library-Core.ps1
- **TODO:** Convert to use `Invoke-CMakeBuild` or `Invoke-MSBuildLibrary`

### 4. Build-Zstd.ps1 (old version)
- **Location:** `build-scripts/external-libs/Build-Zstd.ps1`
- **Status:** 🔄 Needs refactoring to use Build-Library-Core.ps1
- **TODO:** Convert to use `Invoke-CMakeBuild` with proper options

### 5. Build-LZ4.ps1 (old version)
- **Location:** `build-scripts/external-libs/Build-LZ4.ps1`
- **Status:** 🔄 Needs refactoring to use Build-Library-Core.ps1
- **TODO:** Convert to use `Invoke-CMakeBuild`

### 6. test-builds.ps1
- **Location:** `build-scripts/test-builds.ps1`
- **Replacement:** Use `Build-All-And-Package.ps1`
- **Status:** ⚠️ Consider deprecating if functionality duplicated

### 7. build-cbxshell-quick.ps1
- **Location:** `build-scripts/build-cbxshell-quick.ps1`
- **Replacement:** Use `Build-All-And-Package.ps1 -SkipDependencies`
- **Status:** ⚠️ Evaluate for consolidation

## Migration Guide

### Old Pattern (Deprecated)
```powershell
# Old way - calling duplicate utilities
$MSBuild = & "$PSScriptRoot\Find-MSBuild.ps1"
$vcvarsPath = "C:\Program Files (x86)\...\vcvars64.bat"
cmd /c "call `"$vcvarsPath`" && nmake"
```

### New Pattern (Recommended)
```powershell
# New way - using unified module
#Requires -Version 7.0
. "$PSScriptRoot\..\core\Build-Library-Core.ps1"

$cmakeOptions = @{
    'CMAKE_BUILD_TYPE' = 'Release'
    'BUILD_SHARED_LIBS' = 'OFF'
}

Invoke-CMakeBuild `
    -LibraryName "MyLibrary" `
    -SourceDir $sourceDir `
    -BuildDir $buildDir `
    -Configuration $Configuration `
    -CMakeOptions $cmakeOptions
```

## Refactored Scripts

The following scripts have been **successfully refactored** to use the new module:

- ✅ **Build-LibWebP-NMake.ps1** - Reduced from 175 to 102 lines
- ✅ **Build-MinizipNG.ps1** - Reduced from 104 to 60 lines
- ✅ **Build-LibJXL.ps1** - Reduced from 150 to ~90 lines
- ✅ **Build-LibAVIF.ps1** - Reduced from 150 to ~80 lines
- ✅ **Build-Zstd.ps1** - Refactored to use Invoke-CMakeBuild
- ✅ **Build-LZ4.ps1** - Refactored to use Invoke-CMakeBuild/MSBuild
- ✅ **Build-LibHEIF.ps1** - Refactored to use Invoke-CMakeBuild (includes libde265)

**Average code reduction:** ~50%  
**Benefits:** Consistent error handling, unified logging, automatic tool discovery

## Cleanup Plan

### Phase 1: Document (✅ Complete)
- Create DEPRECATED.md (this file)
- Update build-scripts/README.md with new patterns

### Phase 2: Refactor Remaining Scripts (✅ Complete)
All external library scripts now use Build-Library-Core.ps1:
- ✅ Build-Zlib.ps1
- ✅ Build-Zstd.ps1
- ✅ Build-LZ4.ps1
- ✅ Build-LZMA-SDK-26.00.ps1
- ✅ Build-LibRaw.ps1
- ✅ Build-Dav1d.ps1
- ✅ Build-LibHEIF.ps1 (NEW - includes libde265 dependency)

### Phase 3: Remove Deprecated Files (⏳ Scheduled v7.2.0)
- Archive old Find-MSBuild.ps1 scripts
- Update all references in documentation
- Add warning messages to deprecated scripts
- **Note:** Deprecated scripts should emit `Write-Warning` on invocation

### Phase 4: Consolidate Orchestrator Scripts (⏳ Scheduled v7.2.0)
- Evaluate test-builds.ps1 vs Build-All-And-Package.ps1
- Consolidate quick-build scripts
- Update VS Code tasks to use new scripts
- Retire Build-All-DarkThumbs-V7.ps1 in favor of Build-All-And-Package.ps1

## Questions or Issues?

If you encounter issues with deprecated scripts or need help migrating:
1. Check `build-scripts/README.md` for examples
2. Review `DEVELOPER_GUIDE.md` for detailed documentation
3. Examine refactored scripts (Build-LibWebP-NMake.ps1, Build-MinizipNG.ps1, etc.) as templates

## Core Module Reference

The new unified build system is located at:
- **Core Module:** `build-scripts/core/Build-Library-Core.ps1`
- **Helper Module:** `build-scripts/core/Build-Helpers.ps1`
- **Main Build Script:** `build-scripts/Build-All-And-Package.ps1`

### Key Functions
- `Invoke-CMakeBuild` - Build CMake-based libraries
- `Invoke-MSBuildLibrary` - Build MSBuild-based projects
- `Invoke-NMakeBuild` - Build NMake-based libraries
- `Find-MSBuildPath` - Locate MSBuild executable
- `Find-CMakePath` - Locate CMake executable
- `Test-VisualStudioTools` - Verify build environment
- `Write-BuildLog` - Unified logging with color coding
