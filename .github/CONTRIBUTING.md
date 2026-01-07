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
