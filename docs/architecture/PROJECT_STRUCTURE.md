# ExplorerLens Project Structure

**Last Updated:** July 2025
**Version:** 15.0.0 "Zenith"
**Organization Standard:** Industrial Open Source

This document describes the complete directory structure and organization of the ExplorerLens project.

---

## 📁 Root Directory Structure

```text
ExplorerLens/
├── .git/ # Git repository data
├── .github/ # GitHub configuration and workflows
├── .gitignore # Git exclusions
├── .gitattributes # Git file attributes
├── .vscode/ # VS Code workspace settings
├── build/ # CMake build outputs (not tracked)
├── build-logs/ # Build logs with timestamps (not tracked)
├── build-scripts/ # Build automation scripts
├── LENSManager/ # GUI management application
├── LENSShell/ # Shell extension core (COM DLL)
├── LENSShell.sln # Visual Studio solution
├── CMakeLists.txt # Root CMake configuration
├── ExplorerLensSetup_x64/ # Installer project (WiX) 
├── docs/ # Documentation
├── downloads/ # Downloaded library archives (tracked)
├── Engine/ # Thumbnail engine (C++20, unit-tested)
├── external/ # Third-party dependencies
├── install/ # CMake install output (not tracked)
├── LICENSE # MIT License
├── marketplace/ # Marketplace assets & submissions
├── packages/ # NuGet packages (WTL)
├── packaging/ # Package configuration (MSIX, MSI)
├── README.md # Project overview and quick start
├── release-scripts/ # Release automation
├── scripts/ # Utility scripts
├── SDK/ # Plugin SDK
├── src/ # Future modular C++ projects
├── test-archives/ # Test files for format support
├── tests/ # Unit and integration tests
├── tools/ # Development tools
├── x64/ # x64 build outputs (not tracked)
└── PROJECT_STRUCTURE.md # This file
```

---

## 📂 Detailed Directory Breakdown

### `.github/` - GitHub Configuration

**Purpose:** GitHub-specific configuration, templates, and CI/CD workflows.

```text
.github/
├── ISSUE_TEMPLATE/ # Issue templates (bug, feature, etc.)
├── workflows/ # GitHub Actions CI/CD
│ ├── build.yml # Standard build workflow
│ ├── build-and-test.yml # Build + tests
│ ├── code-quality.yml # Linters, static analysis
│ └── release.yml # Release packaging
├── standards/ # Coding standards and conventions
├── CONTRIBUTING.md # Contribution guidelines
├── PULL_REQUEST_TEMPLATE.md # PR template
└── SECURITY.md # Security policy
```

**See Also:** [GitHub Standards](.github/standards/)

---

### `build-scripts/` - Build Automation

**Purpose:** PowerShell scripts for building ExplorerLens and external dependencies.

```text
build-scripts/
├── external-libs/ # Individual library builders
│ ├── Build-Dav1d.ps1
│ ├── Build-LibAVIF.ps1
│ ├── Build-LibHEIF.ps1
│ ├── Build-LibJXL.ps1
│ ├── Build-LibRaw.ps1
│ ├── Build-LibWebP-NMake.ps1
│ ├── Build-LZ4.ps1
│ ├── Build-MinizipNG.ps1
│ ├── Build-Zlib.ps1
│ └── Build-Zstd.ps1
├── library-builders/ # Orchestrate multiple library builds
│ ├── Build-All-External-Libraries.ps1
│ ├── Build-Critical-Libraries.ps1
│ └── Download-And-Build-Libraries.ps1
├── production/ # Production build scripts
│ ├── Build-Production-SlowMachine.ps1
│ └── Rebuild-External-Libs-Correct-Runtime.ps1
├── utilities/ # Build utilities
│ ├── ExplorerLens.ps1
│ ├── Enable-ExplorerLensDiagnostics.ps1
│ └── Monitor-Build.ps1
├── validation/ # Build validation
│ ├── Check-Tools.ps1
│ ├── Simple-Validate.ps1
│ └── Validate-Build.ps1
├── archive/ # Archived/deprecated scripts
├── build.ps1 # Main build script
├── Find-MSBuild.ps1 # MSBuild locator
└── README.md # Build scripts documentation
```

**See Also:** [Build Scripts README](build-scripts/README.md)

---

### `LENSManager/` - Management GUI Application

**Purpose:** GUI application for configuring ExplorerLens settings and file associations.

```text
LENSManager/
├── MainDlg.cpp # Main dialog implementation
├── MainDlg.h # Main dialog header
├── LENSManager.cpp # Application entry point
├── LENSManager.vcxproj # Visual Studio project
├── DarkModeHelper.h # Dark mode support
├── RegManager.h # Registry management
├── About.h # About dialog
├── CMakeLists.txt # CMake configuration
└── x64/ # Build outputs (not tracked)
```

**Technology:** ATL/WTL, Win32 GUI

---

### `LENSShell/` - Shell Extension (Core)

**Purpose:** COM DLL shell extension that provides thumbnail generation for Windows Explorer.

```text
LENSShell/
├── decoders/ # Format-specific decoders
│ ├── archive_decoder.cpp
│ ├── avif_decoder.cpp
│ ├── jxl_decoder.cpp
│ ├── raw_decoder.cpp
│ └── webp_decoder.cpp
├── LENSShell.cpp # DLL entry point
├── LENSShell.idl # COM interface definitions
├── LENSShellClass.cpp # Main COM class
├── LENSShell.vcxproj # Visual Studio project
├── EngineAdapter.h # Engine integration
└── x64/ # Build outputs (not tracked)
```

**Technology:** ATL COM, C++20

---

### `docs/` - Documentation

**Purpose:** Comprehensive project documentation organized by category.

```text
docs/
├── architecture/ # Architecture documentation
├── build/ # Build guides
│ └── BUILD_GUIDE.md
├── development/ # Development documentation
│ ├── BUILD_QUICK_REFERENCE.md
│ ├── THIRD_PARTY.md
│ ├── DEVELOPER_GUIDE.md
│ └── README.md
├── formats/ # Supported format documentation
├── getting-started/ # Getting started guides
│ ├── INSTALLATION_TESTING_GUIDE.md
│ └── QUICK_SETUP.md
├── gpu/ # GPU acceleration docs
├── planning/ # Project planning
│ ├── ENHANCEMENT_PLAN_SUMMARY.md
│ ├── LIBRARY_UPDATE_PLAN.md
│ └── OPENSOURCE_ENHANCEMENT_PLAN.md
├── plugins/ # Plugin system documentation
│ ├── PLUGIN_PACKAGE_FORMAT_V1.md
│ └── PLUGIN_SANDBOX_MODEL_V1.md
├── release-notes/ # Release notes history
├── archive/ # Archived development summaries
├── testing/ # Testing documentation
│ ├── PRIORITY1_BASELINE_VERIFICATION.md
│ ├── TESTING_GUIDE.md
│ └── TEST_VALIDATION_CHECKLIST.md
├── archive/ # Archived documentation
├── CHANGELOG.md # Project changelog
├── INDEX.md # Documentation index
├── mkdocs.yml # MkDocs configuration
└── SBOM.json # Software Bill of Materials
```

**See Also:** [Documentation Index](docs/INDEX.md)

---

### `downloads/` - Library Archives

**Purpose:** Downloaded source archives for external libraries (cached for builds).

```text
downloads/
├── libarchive-3.7.6.tar.gz # Archive library
├── minizip-ng-4.0.10.zip # ZIP library
├── wtl.10.0.10320.zip # Windows Template Library
├── zlib131.zip # zlib compression
└── README.md # Downloads documentation
```

**Note:** These files ARE tracked in git (exception to `*.tar.gz`, `*.zip` exclusion).

**See Also:** [Downloads README](downloads/README.md)

---

### `Engine/` - Thumbnail Engine

**Purpose:** Core thumbnail generation engine (standalone C++20 library with unit tests).

```text
Engine/
├── src/ # Engine source code
│ ├── ThumbnailEngine.cpp
│ ├── DecoderRegistry.cpp
│ ├── ThumbnailPipeline.cpp
│ └── ...
├── include/ # Public API headers
│ ├── Engine.h
│ ├── Types.h
│ └── ...
├── tests/ # Unit tests (Google Test)
│ ├── DecoderTests.cpp
│ ├── PipelineTests.cpp
│ └── ...
├── CMakeLists.txt # CMake configuration
├── README.md # Engine documentation
└── README_ARCHITECTURE.md # Architecture details
```

**Technology:** C++20, Google Test, CMake

**Test Status:** ✅ 38/38 tests passing

---

### `external/` - Third-Party Dependencies

**Purpose:** External library source code, builds, and installations. Organized by library category for maintainability.

```text
external/
├── compression-libs/ # Compression libraries
│ ├── zlib-1.3.1/ # General-purpose compression
│ ├── zstd-1.5.7/ # Fast compression with high ratios
│ ├── lz4-1.10.0/ # Extremely fast compression
│ ├── lzma-sdk-24.08/ # LZMA compression
│ ├── brotli-1.1.0/ # Google's compression algorithm
│ ├── minizip-ng-4.0.10/ # ZIP archive handling
│ └── libarchive/ # Multi-format archive support
├── image-libs/ # Modern image format decoders
│ ├── libwebp-1.5.0-original/ # WebP format support
│ ├── libwebp-1.5.0-build/ # WebP build artifacts
│ ├── libjxl-0.11.1/ # JPEG XL next-gen format
│ ├── libavif-1.3.0/ # AVIF (AV1) image format
│ └── dav1d-1.5.1/ # Fast AV1 decoder
├── camera-libs/ # RAW photo processing
│ ├── libraw/ # RAW photo library source
│ └── libraw-install/ # LibRaw installation
├── archive-libs/ # Archive format support
│ └── unrar/ # RAR extraction support
├── ui-libs/ # UI frameworks
│ └── wtl/ # Windows Template Library
├── pdf-libs/ # PDF support (future)
│ └── mupdf/ # PDF renderer (planned)
├── CMakeLists.txt # CMake configuration
├── LIBRARY_INVENTORY.md # Complete inventory
└── README.md # External dependencies doc
```

**Organization:** Libraries are grouped by purpose (compression, image processing, camera, archives, UI, PDF) for easy management and selective builds.

**See Also:** [External Dependencies README](external/README.md)

---

### `marketplace/` - Marketplace Assets

**Purpose:** Assets for Microsoft Store and other marketplaces.

```text
marketplace/
├── screenshots/ # Store screenshots
├── icons/ # Application icons
├── descriptions/ # Store descriptions
└── README.md # Marketplace guidelines
```

---

### `packaging/` - Installation Packages

**Purpose:** Installers and package configurations.

```text
packaging/
├── msix/ # MSIX package (Microsoft Store)
└── ...
```

**Related:** `ExplorerLensSetup_x64/` (WiX installer project)

---

### `release-scripts/` - Release Automation

**Purpose:** Scripts for creating releases and publishing.

```text
release-scripts/
├── create-release.ps1 # Create GitHub release
├── package-installer.ps1 # Package installer
└── publish-store.ps1 # Publish to Microsoft Store
```

---

### `scripts/` - Utility Scripts

**Purpose:** General-purpose utility scripts (non-build).

```text
scripts/
├── reorganize-project.ps1 # Project structure reorganization
├── cleanup.ps1 # Clean build artifacts
└── README.md # Scripts documentation
```

---

### `SDK/` - Plugin SDK

**Purpose:** Plugin development kit for extending ExplorerLens.

```text
SDK/
├── include/ # SDK headers
│ └── plugin_api.h
├── examples/ # Example plugins
│ ├── minimal-plugin/
│ └── SamplePlugin/
├── docs/ # SDK documentation
│ └── PLUGIN_SDK.md
└── README.md # SDK documentation
```

---

### `tests/` - Testing

**Purpose:** Integration tests and test resources.

```text
tests/
├── integration/ # Integration tests
├── test-files/ # Test images/archives
├── run-tests.ps1 # Test runner
└── README.md # Testing documentation
```

**Note:** Engine unit tests are in `Engine/tests/`

---

### `tools/` - Development Tools

**Purpose:** Development utilities and helper tools.

```text
tools/
├── RegisterCOM.ps1 # COM registration utility
└── ...
```

---

## 🚫 Excluded from Git

These directories are generated during builds and **not tracked** in git:

- `build/` - CMake build outputs
- `build-logs/` - Build log files
- `x64/` - Visual Studio x64 outputs
- `LENSManager/x64/` - LENSManager builds
- `LENSShell/x64/` - LENSShell builds
- `Engine/build/` - Engine CMake builds
- `Engine/x64/` - Engine builds
- `install/` - CMake install output
- `external/**/build*/` - External lib builds
- `external/**/install/` - External lib installs

See [.gitignore](.gitignore) for complete exclusion list.

---

## 📝 Root-Level Files

### Documentation

- **README.md** - Project overview, features, quick start
- **LICENSE** - MIT License
- **PROJECT_STRUCTURE.md** - This file

### Build Configuration

- **LENSShell.sln** - Visual Studio solution (main)
- **CMakeLists.txt** - Root CMake configuration
- **.gitignore** - Git file exclusions
- **.gitattributes** - Git line endings and attributes

### IDE Configuration

- **.vscode/** - VS Code workspace settings, tasks, launch configs

---

## 🏗️ Build Artifacts Organization

### Build Outputs (x64/Release/)

```text
x64/Release/
├── LENSShell.dll # Shell extension DLL
├── LENSManager.exe # Management GUI
├── ExplorerLensEngine.lib # Engine static library
└── ...
```

### Build Logs (build-logs/)

```text
build-logs/
├── build-2026-02-11_14-30-25.log
├── build-progress.json
└── ...
```

Logs include timestamps and are auto-cleaned after 30 days.

---

## 📦 Package Management

### NuGet Packages (packages/)

```text
packages/
└── wtl.10.0.10320/ # Windows Template Library
```

Managed by NuGet Package Manager.

---

## 🔧 Development Workflow

### 1. Clone Repository

```powershell
git clone https://github.com/yourusername/ExplorerLens.git
cd ExplorerLens
```

### 2. Build External Libraries

```powershell
.\/build-scripts\/Build-All-ExplorerLens-V7.ps1 -Clean
```

### 3. Build ExplorerLens

```powershell
.\build-scripts\build.ps1 -Configuration Release
```

### 4. Run Tests

```powershell
.\tests\run-tests.ps1
```

### 5. Install Locally

```powershell
.\tools\RegisterCOM.ps1
```

---

## 📚 Documentation Standards

### Markdown Files

- **Location:** Organized by category in `docs/`
- **Naming:** `SHOUTING_CASE.md` for major docs, `PascalCase.md` for others
- **Cross-references:** Use relative paths
- **Headers:** ATX-style (`#`, `##`, `###`)

### Code Documentation

- **C++:** Doxygen-style comments
- **PowerShell:** Comment-based help (`.SYNOPSIS`, `.DESCRIPTION`, etc.)

### README Files

Every major directory should have a `README.md` explaining:
- Purpose of the directory
- Contents overview
- Usage instructions
- Related documentation links

---

## 🔐 .gitignore Strategy

### Tracked

- ✅ Source code (.cpp, .h, .ps1, etc.)
- ✅ Project files (.vcxproj, .sln, CMakeLists.txt)
- ✅ Documentation (.md files)
- ✅ Configuration files (.yml, .json, .xml)
- ✅ Downloaded archives in `downloads/`
- ✅ External library source in `external/`

### Not Tracked

- ❌ Build outputs (x64/, build/, `*.obj`, `*.lib`, `*.dll`, `*.exe`)
- ❌ Build logs (build-logs/, *.log)
- ❌ IDE files (.vs/, `*.user`, `*.suo`)
- ❌ Temporary files (`*.tmp`, `*~`, `*.bak`)
- ❌ External lib builds (external/**/build*/, external/**/install/)

---

## 🎯 Directory Purpose Summary

| Directory | Purpose | Tracked in Git |
| ----------- | --------- | ---------------- |
| `.github/` | GitHub config, workflows | ✅ Yes |
| `build/` | CMake build outputs | ❌ No |
| `build-logs/` | Build logs | ❌ No |
| `build-scripts/` | Build automation | ✅ Yes |
| `LENSManager/` | GUI application source | ✅ Yes |
| `LENSShell/` | Shell extension source | ✅ Yes |
| `docs/` | Documentation | ✅ Yes |
| `downloads/` | Library archives (cache) | ✅ Yes (exception) |
| `Engine/` | Engine source & tests | ✅ Yes |
| `external/` | Third-party dependencies | ✅ Source, ❌ Builds |
| `marketplace/` | Store assets | ✅ Yes |
| `packaging/` | Installer configs | ✅ Yes |
| `release-scripts/` | Release automation | ✅ Yes |
| `scripts/` | Utility scripts | ✅ Yes |
| `SDK/` | Plugin SDK | ✅ Yes |
| `tests/` | Integration tests | ✅ Yes |
| `tools/` | Development tools | ✅ Yes |
| `x64/` | x64 build outputs | ❌ No |

---

## 🆕 Recent Reorganization (Feb 11, 2026)

### Changes Made

1. **Moved `.github/docs/` → `docs/development/`**
- Consolidated GitHub-specific docs into main documentation
- Better organization for developers

1. **Archived development summaries**
- Consolidated into docs/archive/
- Cleaner root directory

1. **Organized `build-scripts/` by category:**
- Created `external-libs/` subdirectory
- Created `production/` subdirectory
- Better script organization

1. **Removed obsolete files:**
- Old `.bat` scripts
- Deprecated pending-features scripts
- Cleaned up `.github/docs/` after move

1. **Added README files to key directories:**
- `build-scripts/README.md`
- `docs/development/README.md`
- `downloads/README.md`
- `external/README.md`

---

## 📞 Support

For questions about project structure:
- **Read:** This document
- **Search:** [GitHub Issues](https://github.com/yourusername/ExplorerLens/issues)
- **Ask:** [GitHub Discussions](https://github.com/yourusername/ExplorerLens/discussions)

---

**Maintained by:** ExplorerLens Development Team
**Standard:** Industrial Open Source Project Organization
**Compliance:** GitHub best practices, Microsoft OSS guidelines
