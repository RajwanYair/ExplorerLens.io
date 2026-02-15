# Development Documentation

This directory contains comprehensive documentation for developers working on DarkThumbs.

## Contents

### Build & Environment

- **[AI_BUILD_INSTRUCTIONS.md](AI_BUILD_INSTRUCTIONS.md)** - Instructions for AI assistants on building the project
- **[BUILD_QUICK_REFERENCE.md](BUILD_QUICK_REFERENCE.md)** - Quick reference for common build commands
- **[WINDOWS_BUILD_TOOLS.md](WINDOWS_BUILD_TOOLS.md)** - Windows-specific build tools setup
- **[TOOL_DISCOVERY.md](TOOL_DISCOVERY.md)** - How the build system discovers tools
- **[TOOL_VERSIONS.md](TOOL_VERSIONS.md)** - Required and tested tool versions

### Project Organization

- **[PROJECT_ORGANIZATION.md](PROJECT_ORGANIZATION.md)** - Project structure and conventions
- **[THIRD_PARTY.md](THIRD_PARTY.md)** - Third-party dependencies and licenses

## Quick Links

### Getting Started
- [Quick Setup](../getting-started/QUICK_SETUP.md)
- [Build Guide](../build/BUILD_GUIDE.md)
- [Installation Testing](../getting-started/INSTALLATION_TESTING_GUIDE.md)

### Architecture
- [Architecture Overview](../architecture/)
- [Engine Architecture](../../Engine/README_ARCHITECTURE.md)
- [Plugin System](../plugins/)

### Contributing
- [Contributing Guidelines](../../.github/CONTRIBUTING.md)
- [Security Policy](../../.github/SECURITY.md)
- [Pull Request Template](../../.github/PULL_REQUEST_TEMPLATE.md)

## Development Workflow

### 1. Environment Setup

```powershell
# Verify build environment
.\build-scripts\Test-Build-Environment.ps1

# Install missing tools if needed
```

### 2. Building

```powershell
# Quick incremental build
.\build-scripts\build-cbxshell-quick.ps1

# Full clean build
.\build-scripts\build.ps1 -Configuration Release -Clean

# Build external libraries
.\build-scripts\library-builders\Build-All-External-Libraries.ps1
```

### 3. Testing

```powershell
# Run unit tests
.\tests\run-tests.ps1

# Run integration tests
.\tests\run-integration-tests.ps1

# Manual testing
.\build-scripts\utilities\Enable-DarkThumbsDiagnostics.ps1
```

### 4. Debugging

Enable verbose logging and diagnostics:

```powershell
# Enable diagnostic logging
.\build-scripts\utilities\Enable-DarkThumbsDiagnostics.ps1

# View logs
.\build-scripts\utilities\View-DarkThumbsDiagnostics.ps1

# Monitor build process
.\build-scripts\utilities\Monitor-Build.ps1
```

## Common Tasks

### Adding a New Image Format

1. Add decoder to `CBXShell/decoders/`
2. Implement `IFormatDecoder` interface
3. Register in `DecoderRegistry`
4. Add tests to `tests/`
5. Update format count in UI
6. Document in `docs/formats/`

### Adding External Library

1. Download source to `downloads/`
2. Create build script in `build-scripts/external-libs/`
3. Extract/build to `external/[category]/[library]/`
4. Add to `external/LIBRARY_INVENTORY.md`
5. Document in `THIRD_PARTY.md`
6. Update build orchestrators

### Updating Documentation

1. Edit relevant markdown files
2. Update table of contents if needed
3. Cross-reference related docs
4. Test links with markdown linter
5. Update "Last Updated" timestamp

## Development Tools

### Required

- **Visual Studio 2022** (17.8+) - Primary IDE
- **Git** (2.40+) - Version control
- **PowerShell 7+** - Build automation
- **CMake 3.20+** - External library builds

### Recommended

- **Visual Studio Code** - Lightweight editing
- **WinDbg Preview** - Advanced debugging
- **Windows SDK 10.0.22621+** - Windows APIs
- **NASM** - Assembly for some libraries

### Optional

- **RenderDoc** - GPU debugging
- **ETW Viewer** - Telemetry analysis
- **Markdown Monster** - Documentation editing
- **Beyond Compare** - File comparison

## Code Style

### C++ Guidelines

- **Standard:** C++20
- **Naming:**
  - Classes: `PascalCase`
  - Functions: `PascalCase`
  - Variables: `camelCase`
  - Constants: `UPPER_CASE` or `kPascalCase`
- **Indentation:** 4 spaces (no tabs)
- **Line length:** 120 characters max
- **Headers:** `#pragma once`

### PowerShell Guidelines

- **Naming:** `Verb-Noun.ps1` (approved verbs)
- **Parameters:** PascalCase with type hints
- **Comments:** Comment-based help (`.SYNOPSIS`, etc.)
- **Error handling:** `$ErrorActionPreference = "Stop"`

## Documentation Standards

### Markdown Files

- **Headings:** ATX-style (`#`, `##`, etc.)
- **Lists:** `-` for unordered, `1.` for ordered
- **Code blocks:** Fenced with language hint (` ```powershell `)
- **Links:** Relative paths from file location
- **Tables:** Use for structured data

### Code Documentation

- **Classes:** Doxygen-style comments
- **Functions:** Purpose, parameters, return value, exceptions
- **TODOs:** `// TODO(owner): Description`
- **NOTEs:** `// NOTE: Important context`

## Testing

See [Testing Guide](../testing/TESTING_GUIDE.md) for comprehensive testing documentation.

### Quick Test Commands

```powershell
# Run all tests
.\tests\run-tests.ps1

# Run specific test suite
.\tests\run-tests.ps1 -Suite Engine

# Run with coverage
.\tests\run-tests.ps1 -Coverage
```

## Build Troubleshooting

### Common Errors

**"Cannot find MSBuild.exe"**
```powershell
.\build-scripts\Find-MSBuild.ps1
# Ensure Visual Studio 2022 is installed
```

**"Library not found: *.lib"**
```powershell
# Rebuild external libraries
.\build-scripts\production\Rebuild-External-Libs-Correct-Runtime.ps1
```

**"LNK1104: cannot open file"**
```powershell
# Clean and rebuild
.\build-scripts\build.ps1 -Configuration Release -Clean
```

## Performance Profiling

### GPU Profiling

Use RenderDoc or PIX for GPU profiling:

```powershell
# Launch with RenderDoc
renderdoc --attach explorer.exe

# Or use Windows PIX
```

### CPU Profiling

Use Visual Studio Profiler or ETW:

```powershell
# Visual Studio: Debug → Performance Profiler
# Select: CPU Usage, Memory Usage
```

## Debugging Tips

### Shell Extension Debugging

Shell extensions run in `explorer.exe`:

1. **Attach debugger** to explorer.exe
2. **Set breakpoints** in DarkThumbs code
3. **Trigger** thumbnail request (navigate to folder with supported files)
4. **Debugger breaks** at your breakpoint

### Crash Debugging

```powershell
# Enable full crash dumps
.\build-scripts\utilities\Enable-DarkThumbsDiagnostics.ps1

# Crashes saved to: %LocalAppData%\DarkThumbs\CrashDumps\
# Open with WinDbg Preview
```

### Logging

```powershell
# View logs
Get-Content "$env:LOCALAPPDATA\DarkThumbs\Logs\*.log" -Tail 50

# Follow logs in real-time
Get-Content "$env:LOCALAPPDATA\DarkThumbs\Logs\latest.log" -Wait
```

## CI/CD

See `.github/workflows/` for GitHub Actions workflows:

- **build.yml** - Standard build on push/PR
- **build-and-test.yml** - Build + run tests
- **code-quality.yml** - Code analysis, linting
- **release.yml** - Release builds and packaging

## Contributing

See [CONTRIBUTING.md](../../.github/CONTRIBUTING.md) for:
- Code of conduct
- How to report bugs
- How to submit pull requests
- Coding conventions
- Review process

## Resources

### Microsoft Documentation
- [Windows Shell Extensions](https://docs.microsoft.com/en-us/windows/win32/shell/shell-exts)
- [DirectX 11](https://docs.microsoft.com/en-us/windows/win32/direct3d11/atoc-dx-graphics-direct3d-11)
- [COM Programming](https://docs.microsoft.com/en-us/windows/win32/com/)

### Third-Party Libraries
- [libwebp](https://developers.google.com/speed/webp)
- [libjxl](https://github.com/libjxl/libjxl)
- [libavif](https://github.com/AOMediaCodec/libavif)
- [LibRaw](https://www.libraw.org/)

### Community
- [GitHub Issues](https://github.com/yourusername/DarkThumbs/issues)
- [Discussions](https://github.com/yourusername/DarkThumbs/discussions)

## Support

For development questions:
1. **Check** this documentation
2. **Search** existing issues on GitHub
3. **Ask** in GitHub Discussions
4. **Open** new issue if unresolved

---

**Last Updated:** February 11, 2026  
**Maintained by:** DarkThumbs Development Team
