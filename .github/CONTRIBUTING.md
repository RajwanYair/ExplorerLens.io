# Contributing to DarkThumbs

Thank you for your interest in contributing to DarkThumbs!

## Getting Started

1. **Review the documentation:**
   - [README.md](../README.md) - Project overview
   - [ROADMAP.md](../ROADMAP.md) - Development roadmap
   - [docs/BUILD_GUIDE.md](../docs/BUILD_GUIDE.md) - Build instructions

2. **Set up your development environment:**
   - Visual Studio 2026 BuildTools with MSVC v19.50+
   - CMake 3.20+
   - See [WINDOWS_BUILD_TOOLS.md](WINDOWS_BUILD_TOOLS.md) for detailed setup

## Development Workflow

### Building the Project

```cmd
# Open "x64 Native Tools Command Prompt for VS 2026"
cd path\to\DarkThumbs

# Build external libraries first (see docs/BUILD_GUIDE.md)
# Then build the main project:
msbuild CBXShell.sln /t:Rebuild /p:Configuration=Release /p:Platform=x64
```

### Testing Your Changes

```cmd
# Register the DLL
regsvr32 /s x64\Release\CBXShell.dll

# Test in Windows Explorer
# Navigate to a folder with supported file types

# Unregister when done
regsvr32 /u /s x64\Release\CBXShell.dll
```

## Code Standards

- **C++20** standard
- **RAII** for resource management
- **COM** for Windows integration
- **DirectX 11** for GPU acceleration
- Follow existing code style and conventions

## Submitting Changes

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/your-feature`)
3. Make your changes
4. Test thoroughly
5. Commit with descriptive messages
6. Push to your fork
7. Open a Pull Request

## Pull Request Guidelines

- Describe what your changes do
- Reference any related issues
- Include test results
- Ensure build succeeds
- Update documentation if needed

## Project Organization

### Avoiding Duplicate Files

**Before creating new files**, search for existing ones:
```powershell
# Search for similar scripts
Get-ChildItem -Recurse -Filter "*install*.ps1"

# Search for similar docs
Get-ChildItem docs -Recurse -Filter "*.md" | Select-String "Installation"
```

### Canonical File Locations

| Purpose | Canonical Path | Notes |
|---------|---------------|-------|
| **Main build** | `scripts/build.ps1` | Use this, not build-scripts/build.ps1 |
| **Installation** | `scripts/install.ps1` | Consolidates all install logic |
| **Verification** | `scripts/verify-tools.ps1` | Tool detection |
| **Library builds** | `build-scripts/*.ps1` | Library-specific builds OK |
| **Updates** | `build-scripts/update-all-libraries.ps1` | Version updates |

### Naming Conventions

**Scripts:**
- Use lowercase-with-hyphens: `build-library.ps1`, `verify-tools.ps1`
- Prefix by purpose: `build-*`, `install-*`, `verify-*`, `update-*`

**Documentation:**
- Major docs: UPPERCASE_WITH_UNDERSCORES: `BUILD_GUIDE.md`
- Subdirectory docs: lowercase-with-hyphens: `getting-started/installation.md`
- Session notes: `SESSION_SUMMARY_2026-01-08.md`
- Sprint notes: `SPRINT12_COMPLETION_REPORT.md`

### Documentation Standards

**Avoid duplicate installation guides**:
- ✅ `INSTALLATION_READY.md` (root, user-facing)
- ✅ `docs/getting-started/installation.md` (detailed with troubleshooting)
- ❌ Don't create `QUICK_SETUP.md`, `INSTALL.md`, etc.

**Link to canonical docs**:
```markdown
For detailed troubleshooting, see [Installation Guide](docs/getting-started/installation.md)
```

### Commit Message Format

```
<type>(<scope>): <subject>

<body>
```

**Types**: `feat`, `fix`, `refactor`, `docs`, `build`, `chore`

**Examples**:
```bash
feat: add dry-run mode to installation script
fix: resolve COM registration timeout
refactor(scripts): consolidate duplicate installation scripts
docs: update build guide with VS 2026 support
```

### Directory Structure

**Keep:**
- `scripts/` - Canonical scripts at root
- `build-scripts/` - Library-specific builds
- `docs/` - All documentation
- `docs/archive/` - Historical summaries
- `docs/development/` - Sprint notes, changelogs

**Avoid:**
- Duplicate scripts in `scripts/install/*.ps1` (use `scripts/install.ps1`)
- Multiple guides for same topic (consolidate into one)

## Bug Reports

Use GitHub Issues with:
- Clear description of the problem
- Steps to reproduce
- Expected vs actual behavior
- System information (Windows version, GPU, etc.)
- Screenshots if applicable

## Feature Requests

Open an issue with:
- Clear use case description
- Why it would be valuable
- Any implementation ideas

## Questions?

- Check [docs/BUILD_GUIDE.md](../docs/BUILD_GUIDE.md) for build issues
- Review [ROADMAP.md](../ROADMAP.md) for project direction
- Open a GitHub Discussion for general questions

Thank you for contributing!
