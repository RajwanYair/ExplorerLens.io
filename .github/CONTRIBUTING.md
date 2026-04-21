# Contributing to ExplorerLens

Thank you for your interest in contributing to ExplorerLens!

## Getting Started

1. **Review the documentation:**
 - [README.md](../README.md) - Project overview
 - [CHANGELOG.md](../CHANGELOG.md) - Development history
- [docs/build/INSTALLATION_GUIDE.md](../docs/build/INSTALLATION_GUIDE.md) - Build & installation instructions

2. **Set up your development environment:**
 - Visual Studio 18 2026 BuildTools with MSVC v145 (cl.exe 19.50)
 - CMake 3.25+ and Ninja 1.13+ (install via Scoop: `scoop install cmake ninja`)
 - Windows SDK 10.0.26100.0
 - See [BUILD_QUICK_REFERENCE.md](../docs/development/BUILD_QUICK_REFERENCE.md) for detailed setup

## Development Workflow

### Building the Project

```powershell
# Preferred: repository build script (sources vcvars automatically)
.\build-scripts\Build-MSVC.ps1

# Clean build
.\build-scripts\Build-MSVC.ps1 -Clean

# Clean build + tests
.\build-scripts\Build-MSVC.ps1 -Clean -Test

# Shell + manager only (MSBuild)
msbuild LENSShell.sln /p:Configuration=Release /p:Platform=x64 /m /v:minimal
```

Use the shared MyScripts terminal bootstrap plus the workspace `.env.ps1` helpers when working in VS Code. The default ExplorerLens terminal profile is expected to load the common toolchain layer first, then project-local helpers.

### Testing Your Changes

```powershell
# Canonical CTest invocation
ctest --test-dir build -C Release --output-on-failure

# Direct engine test runner
& '.\build\bin\EngineTests.exe'

# Register and test shell extension manually when needed
regsvr32 /s x64\Release\LENSShell.dll
regsvr32 /u /s x64\Release\LENSShell.dll
```

## Code Standards

- **C++20** standard
- **RAII** for resource management
- **COM** for Windows integration
- **DirectX 11 + DirectX 12** for GPU acceleration
- **Zero warnings** policy — build must produce 0 warnings
- Follow `.clang-tidy` rules (see project root)
- See `.github/standards/coding-standards.md` for naming conventions
- All headers must use `#pragma once`

## Submitting Changes

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/your-feature`)
3. Make your changes
4. Run `cmake --build build --config Release -j 8` — must produce **0 errors, 0 warnings**
5. Run `ctest --test-dir build -C Release --output-on-failure` — all tests must pass
6. Commit with descriptive messages (see format below)
7. Push to your fork
8. Open a Pull Request

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
| **Main build** | `build-scripts/Build-MSVC.ps1` | Canonical Engine build entry point |
| **All-in-one packaging** | `build-scripts/Build-All-And-Package.ps1` | Production packaging entry point |
| **Version bump / release prep** | `build-scripts/Bump-Version.ps1` | Only supported version sync path |
| **Verification** | `scripts/verify-tools.ps1` | Tool detection |
| **Library builds** | `build-scripts/external-libs/*.ps1` | One script per external library |
| **Updates** | `build-scripts/Update-All-Libraries.ps1` | Dependency refresh workflow |

### AI-Assisted Development Assets

ExplorerLens keeps its AI-facing repository assets under `.github/`:

- `.github/copilot-instructions.md` — primary repository rules
- `.github/instructions/*.instructions.md` — scoped path-based instructions
- `.github/agents/*.agent.md` — custom agent definitions
- `.github/prompts/*.prompt.md` — reusable prompt templates
- `.github/skills/*/SKILL.md` — repository-local skills
- `.github/standards/ai-tooling-capabilities.md` — canonical AI tooling inventory

MCP servers are configured in `.vscode/mcp.json`. If you change the MCP server inventory or scope, update the capability reference above in the same pull request.

### Naming Conventions

**Scripts:**
- Use lowercase-with-hyphens: `build-library.ps1`, `verify-tools.ps1`
- Prefix by purpose: `build-*`, `install-*`, `verify-*`, `update-*`

**Documentation:**
- Major docs: UPPERCASE_WITH_UNDERSCORES: `BUILD_GUIDE.md`
- Subdirectory docs: lowercase-with-hyphens: `getting-started/installation.md`
- Session notes: `SESSION_SUMMARY_2026-01-08.md`
- Completion reports: `COMPLETION_REPORT_YYYY-MM-DD.md`

### Documentation Standards

**Avoid duplicate installation guides**:
- ✅ `INSTALLATION_READY.md` (root, user-facing)
- ✅ `docs/build/INSTALLATION_GUIDE.md` (detailed with troubleshooting)
- ❌ Don't create `QUICK_SETUP.md`, `INSTALL.md`, etc.

**Link to canonical docs**:
```markdown
For detailed troubleshooting, see [Installation Guide](docs/build/INSTALLATION_GUIDE.md)
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
- `docs/development/` - Development notes, changelogs

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

- Check [docs/build/INSTALLATION_GUIDE.md](../docs/build/INSTALLATION_GUIDE.md) for build issues
- Check [docs/development/BUILD_QUICK_REFERENCE.md](../docs/development/BUILD_QUICK_REFERENCE.md) for quick build commands
- Review [CHANGELOG.md](../CHANGELOG.md) for project direction
- Review [.github/standards/ai-tooling-capabilities.md](../.github/standards/ai-tooling-capabilities.md) for current instructions, agents, prompts, skills, MCP servers, and workflow coverage
- Open a GitHub Discussion for general questions

Thank you for contributing!
