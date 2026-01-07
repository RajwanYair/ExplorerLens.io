# DarkThumbs Project Structure

Last Updated: 2025-12-10  
Version: 6.2.0  
Status: Production-Ready

---

## Directory Organization

```
DarkThumbs/
│
├── .github/                    # GitHub Actions workflows and documentation
│   ├── workflows/
│   │   └── build.yml          # CI/CD pipeline for automated builds
│   └── docs/
│       └── WINDOWS_BUILD_TOOLS.md   # Comprehensive build tools guide
│
├── src/                        # Source code (CBXShell DLL)
│   ├── compression/           # Compression codec implementations
│   ├── image_decoders/        # Image format decoders (WebP, HEIF, JXL, etc.)
│   ├── gpu/                   # GPU acceleration (D3D11)
│   ├── security/              # Plugin sandbox and trust architecture
│   └── utils/                 # Utility classes
│
├── CBXManager/                 # Configuration GUI application
│   ├── x64/Release/           # Build output
│   └── CBXManager.vcxproj
│
├── build-scripts/              # Build automation scripts
│   ├── library-builders/      # External library build scripts
│   │   ├── Build-All-External-Libraries.ps1
│   │   ├── Build-All-Libraries.ps1
│   │   ├── Build-Critical-Libraries.ps1
│   │   ├── Build-Libraries-Simple.ps1
│   │   └── Download-And-Build-Libraries.ps1
│   ├── validation/            # Build validation scripts
│   │   ├── check-tools.ps1
│   │   ├── Simple-Validate.ps1
│   │   └── Validate-Build.ps1
│   └── utilities/             # Development utilities
│       ├── Enable-DarkThumbsDiagnostics.ps1
│       ├── View-DarkThumbsDiagnostics.ps1
│       ├── darkthumbs.ps1
│       └── Find-All-Build-Tools.ps1 (from docs)
│
├── external/                   # Third-party dependencies
│   ├── compression/           # Compression libraries
│   │   ├── zlib-1.3.1/       # ✅ Built (191 KB)
│   │   ├── lz4-1.10.0/       # ✅ Built (645.6 KB)
│   │   ├── zstd-1.5.7/       # ✅ Built (5.9 MB)
│   │   ├── bzip2-1.0.8/      # ❌ Pending
│   │   ├── lzma-24.08/       # ❌ Pending
│   │   └── minizip-ng-4.0.10/ # ❌ Pending
│   └── image-libs/            # Image processing libraries
│       └── libwebp-1.5.0/    # ❌ Pending
│
├── docs/                       # Consolidated documentation
│   ├── development/           # Development documentation (18 files)
│   │   ├── BUILD_VERIFICATION_2025-12-10.md
│   │   ├── CHANGELOG.md
│   │   ├── COMPLETE_PROJECT_STATUS.md
│   │   ├── CONTINUATION_SESSION.md
│   │   ├── INSTALL_GUIDE.txt
│   │   ├── INSTALLATION_FIX.md
│   │   ├── LIBRARY_UPDATE_STATUS.md
│   │   ├── PROJECT_ENHANCEMENT_SUMMARY.md
│   │   ├── QUICK_REFERENCE.md
│   │   ├── README-old.md
│   │   ├── SBOM.json
│   │   ├── SESSION_COMPLETE.md
│   │   ├── SESSION_SUMMARY_CURRENT.md
│   │   ├── SESSION_SUMMARY.md
│   │   ├── SPRINT_VALIDATION_STATUS.md
│   │   ├── SPRINT1_AND_2_COMPLETE.md
│   │   ├── SPRINT1_COMPLETE.md
│   │   ├── SPRINT1_INTEGRATION_GUIDE.md
│   │   ├── SPRINT2_HEIF_COMPLETE.md
│   │   └── SPRINT3_PLANNING.md
│   ├── release-notes/         # Release documentation
│   │   └── RELEASE_NOTES_v6.0.0.md
│   ├── api/                   # API documentation (future)
│   └── mkdocs.yml             # MkDocs configuration
│
├── archive/                    # Archived/deprecated content
│   ├── deprecated-scripts/    # Old build scripts (7 files)
│   │   ├── Build-Phase10.ps1
│   │   ├── build-production-clean.ps1
│   │   ├── build-sprint9.ps1
│   │   ├── build-sprint9.cmd
│   │   ├── Build-Complete.ps1
│   │   ├── Direct-Build.ps1
│   │   └── Quick-Build.ps1
│   └── old-builds/            # Historical build logs
│       └── logs/              # Build output logs (4 files)
│
├── x64/Release/                # Primary build output
│   └── CBXShell.dll           # Main shell extension (target: ~2-5 MB)
│
├── tests/                      # Unit and integration tests (future)
│
├── tools/                      # Development tools and utilities
│
├── packages/                   # NuGet packages
│
├── Build-Production.ps1        # Main production build script
├── CBXShell.sln                # Visual Studio solution
├── README.md                   # Project overview
└── LICENSE                     # Project license

```

---

## Key Components

### Core Libraries

| Component | Purpose | Status | Size |
|-----------|---------|--------|------|
| **CBXShell.dll** | Windows Shell Extension | ✅ Ready to build | Target: 2-5 MB |
| **CBXManager.exe** | Configuration GUI | ✅ Ready to build | Target: ~500 KB |

### External Dependencies

#### Compression Libraries (6)

- **zlib 1.3.1** - General purpose compression ✅ Built (191 KB)
- **LZ4 1.10.0** - Fast compression ✅ Built (645.6 KB)
- **Zstd 1.5.7** - Modern compression ✅ Built (5.9 MB)
- **Bzip2 1.0.8** - Legacy compression ❌ Pending
- **LZMA 24.08** - 7-Zip compression ❌ Pending
- **MinizipNG 4.0.10** - ZIP archive support ❌ Pending

#### Image Libraries (2)

- **LibWebP 1.5.0** - WebP image format ❌ Pending
- **SharpYUV** - YUV color space (part of WebP) ❌ Pending

---

## Build System

### Build Tools Required

| Tool | Version | Location | Purpose |
|------|---------|----------|---------|
| **MSBuild** | 18.x (VS 2026) | `C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\` | Compile C++ code |
| **vcvarsall.bat** | VS 2026 | `...\VC\Auxiliary\Build\vcvarsall.bat` | Setup environment |
| **CMake** | 3.28+ | `C:\Users\ryair\scoop\shims\cmake.exe` | Configure builds |
| **Ninja** | 1.13.2 | `C:\Users\ryair\scoop\shims\ninja.exe` | Fast parallel builds |
| **vcpkg** | Latest | `C:\Users\ryair\scoop\apps\vcpkg\` | Package manager |
| **NuGet** | Latest | Auto-detected | .NET packages |

### Build Scripts

#### Library Builders (`build-scripts/library-builders/`)

1. **Build-All-External-Libraries.ps1** - Comprehensive CMake+Ninja builder (recommended)
2. **Build-All-Libraries.ps1** - Original builder with VS generator
3. **Build-Critical-Libraries.ps1** - NMake-based approach
4. **Build-Libraries-Simple.ps1** - Simplified using VS projects
5. **Download-And-Build-Libraries.ps1** - vcpkg integration (recommended for quick setup)

#### Validation Scripts (`build-scripts/validation/`)

1. **check-tools.ps1** - Verify build tool installation
2. **Simple-Validate.ps1** - Quick validation checks
3. **Validate-Build.ps1** - Comprehensive build verification

#### Utilities (`build-scripts/utilities/`)

1. **Enable-DarkThumbsDiagnostics.ps1** - Enable debug logging
2. **View-DarkThumbsDiagnostics.ps1** - View extension logs
3. **darkthumbs.ps1** - General utility script
4. **Find-All-Build-Tools.ps1** - Automatic tool discovery (see .github/docs)

---

## Build Workflow

### Standard Build Process

```powershell
# 1. Verify tools
.\build-scripts\validation\check-tools.ps1

# 2. Build external libraries (choose one method):

# Method A: vcpkg (fastest, prebuilt)
.\build-scripts\library-builders\Download-And-Build-Libraries.ps1

# Method B: CMake + Ninja (from source)
.\build-scripts\library-builders\Build-All-External-Libraries.ps1

# 3. Build DarkThumbs
.\Build-Production.ps1

# 4. Validate build
.\build-scripts\validation\Validate-Build.ps1
```

### CI/CD Pipeline

See [`.github/workflows/build.yml`](.github/workflows/build.yml) for automated builds:

- Automatic tool detection
- vcpkg library installation
- Multi-platform build matrix
- Artifact publishing
- Release automation

---

## Documentation Structure

### Development Docs (`docs/development/`)

- **BUILD_VERIFICATION_2025-12-10.md** - Latest build status
- **COMPLETE_PROJECT_STATUS.md** - Overall project state
- **LIBRARY_UPDATE_STATUS.md** - External library status
- **SESSION_SUMMARY.md** - Development session summaries
- **SPRINT_VALIDATION_STATUS.md** - Sprint completion tracking

### Release Notes (`docs/release-notes/`)

- Version history and changelogs
- Breaking changes
- Migration guides

### Build Tools Guide (`.github/docs/`)

- **WINDOWS_BUILD_TOOLS.md** - Comprehensive tool installation guide
- Detection methods for all build tools
- Troubleshooting procedures
- Proxy configuration

---

## Features by Sprint

### Sprint 19 (v6.1) - Plugin Sandbox Architecture

- Restricted AppContainer for plugin isolation
- Capability-based security model
- Process isolation and memory protection

### Sprint 20 (v6.2) - Trust & Signing System

- Authenticode signature verification
- Publisher trust database
- Certificate chain validation
- Revocation checking

### Future Sprints

- Sprint 21: Advanced security features
- Sprint 22: Performance optimizations
- Sprint 23: Additional image format support

---

## Archive Structure

### Deprecated Scripts (`archive/deprecated-scripts/`)

Old build scripts preserved for reference:

- `Build-Phase10.ps1` - Sprint 10 builder
- `build-production-clean.ps1` - Old clean build
- `build-sprint9.*` - Sprint 9 scripts
- `Build-Complete.ps1` - Deprecated comprehensive builder
- `Direct-Build.ps1` - Old direct build approach
- `Quick-Build.ps1` - Old quick build script

### Old Builds (`archive/old-builds/logs/`)

Historical build logs for debugging:

- `build_log.txt`, `build_log_2.txt`
- `build-output.txt`
- `build-sprint8.txt`, `build-sprint8-full.txt`

---

## Project Statistics

- **Total Source Files**: ~150+ C++/H files
- **Lines of Code**: ~50,000+ (estimated)
- **Build Scripts**: 11 active + 7 archived
- **Documentation Files**: 20+ markdown files
- **External Dependencies**: 8 libraries
- **Build Time**: 5-15 minutes (depending on libraries)
- **Final Package Size**: ~10-15 MB (all formats enabled)

---

## Quick Reference

### Essential Files

```powershell
# Main solution
.\CBXShell.sln

# Production build
.\Build-Production.ps1

# Tool detection
.\build-scripts\validation\check-tools.ps1

# Library builds
.\build-scripts\library-builders\Download-And-Build-Libraries.ps1

# Build validation
.\build-scripts\validation\Validate-Build.ps1
```

### Build Output Locations

```
x64\Release\CBXShell.dll           # Main shell extension
CBXManager\x64\Release\CBXManager.exe  # Configuration tool
external\compression\*\build-vs\Release\*.lib  # Compression libs
external\image-libs\*\build-vs\Release\*.lib   # Image libs
```

### Log Locations

```
archive\old-builds\logs\*.txt      # Historical build logs
docs\development\*.md              # Development documentation
```

---

## Maintenance Notes

### Adding New Libraries

1. Download/clone to `external/<category>/<name>-<version>/`
2. Create build script in `build-scripts/library-builders/`
3. Update `CBXShell.vcxproj` with library paths
4. Update this document
5. Update `.github/workflows/build.yml`

### Updating Documentation

1. Development docs → `docs/development/`
2. Release notes → `docs/release-notes/`
3. Build guides → `.github/docs/`
4. Keep this structure doc updated

### Archive Policy

- Move deprecated scripts to `archive/deprecated-scripts/`
- Archive old build logs to `archive/old-builds/logs/`
- Keep archives for at least 2 major versions
- Document archival reason in commit message

---

## Contact & Support

- **GitHub Issues**: Report bugs and feature requests
- **Documentation**: See `docs/` directory
- **Build Help**: See `.github/docs/WINDOWS_BUILD_TOOLS.md`
- **CI/CD**: See `.github/workflows/build.yml`

---

*This structure was reorganized on 2025-12-10 to reduce clutter and improve maintainability.*
