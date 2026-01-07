# Build Tools Setup for DarkThumbs

This document lists all required tools and utilities for building DarkThumbs on Windows.

## Core Build Tools

### Visual Studio Build Tools

**Required Version:** Visual Studio 2022/2026 Build Tools with MSVC v19.50+

**Installation:**

```powershell
# Download from Microsoft
# https://visualstudio.microsoft.com/downloads/
# Select "Build Tools for Visual Studio"

# Required components:
# - MSVC v143 - VS 2022 C++ x64/x86 build tools (Latest)
# - Windows 11 SDK (10.0.26100.0 or later)
# - C++ CMake tools for Windows
# - C++ ATL for latest build tools (x86 & x64)
```

**Verification:**

```powershell
# Find Visual Studio installation
& "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath

# Check MSVC version
cl.exe # After running vcvars64.bat
```

### CMake

**Required Version:** 3.20 or higher

**Installation:**

```powershell
# Using winget
winget install Kitware.CMake

# Using chocolatey
choco install cmake

# Manual download
# https://cmake.org/download/
```

**Verification:**

```powershell
cmake --version
# Should show: cmake version 3.20 or higher
```

### PowerShell 7

**Required Version:** 7.0 or higher

**Installation:**

```powershell
# Using winget
winget install Microsoft.PowerShell

# Using MSI installer
# https://github.com/PowerShell/PowerShell/releases
```

**Verification:**

```powershell
pwsh --version
# Should show: PowerShell 7.x.x
```

### Git

**Required Version:** 2.30 or higher

**Installation:**

```powershell
# Using winget
winget install Git.Git

# Using chocolatey
choco install git

# Manual download
# https://git-scm.com/downloads
```

**Verification:**

```powershell
git --version
```

---

## VS Code Setup (Recommended)

### Installation

```powershell
# Using winget
winget install Microsoft.VisualStudioCode

# Manual download
# https://code.visualstudio.com/
```

### Required Extensions

Install these extensions for optimal development experience:

```powershell
# PowerShell extension
code --install-extension ms-vscode.powershell

# C/C++ extension
code --install-extension ms-vscode.cpptools

# CMake Tools
code --install-extension ms-vscode.cmake-tools

# Task Explorer
code --install-extension spmeesseman.vscode-taskexplorer
```

### Workspace Configuration

The workspace already includes optimized settings in `.vscode/settings.json` and `.vscode/tasks.json`.

**Key Features:**

- Barebone PowerShell terminal (no profile, no logo)
- Extended scrollback (50,000 lines)
- File-based build monitoring
- Auto-save for log updates
- Predefined build tasks

---

## Build Scripts and Utilities

All scripts are located in the `build-scripts/` directory.

### Library Builders

| Script | Purpose | Timeout |
|--------|---------|---------|
| `Build-Zlib.ps1` | Build zlib 1.3.1 | 5 min |
| `Build-LZ4.ps1` | Build LZ4 1.10.0 | 10 min |
| `Build-Zstd.ps1` | Build Zstandard 1.5.7 | 15 min |
| `build-lzma-simple.ps1` | Build liblzma (xz-5.6.3) | 20 min |
| `Build-LibWebP-NMake.ps1` | Build LibWebP 1.5.0 | 20 min |
| `Build-MinizipNG.ps1` | Build Minizip-NG 4.0.10 | 15 min |

### Monitoring and Status

| Script | Purpose |
|--------|---------|
| `Build-With-Monitoring.ps1` | Wrapper for file-based build monitoring |
| `Monitor-Build-Logs.ps1` | Real-time log file monitoring with color coding |
| `Check-Build-Status.ps1` | Quick status check of all libraries |
| `Find-MSBuild.ps1` | Locate MSBuild.exe |

### Production Builds

| Script | Purpose |
|--------|---------|
| `Build-Production-SlowMachine.ps1` | Full build optimized for slow machines |
| `Build-Production.ps1` | Standard production build |

---

## Environment Setup

### vcvars64.bat Setup

Many build scripts require the Visual Studio environment. The scripts automatically call:

```cmd
"C:\Program Files\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
```

**Manual setup:**

```powershell
# Open Developer Command Prompt for VS 2022
# Or run:
cmd /k "C:\Program Files\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
```

### Path Configuration

Ensure these are in your `PATH`:

```powershell
# Check PATH
$env:PATH -split ';'

# Should include:
# - C:\Program Files\CMake\bin
# - C:\Program Files\Git\cmd
# - C:\Program Files\PowerShell\7
```

---

## Building from VS Code

### Using Tasks (Recommended)

Press `Ctrl+Shift+P` → `Tasks: Run Task` → Select:

1. **Build All Libraries (Slow Machine)** - Full production build
2. **Build with VS Code Monitoring** - Opens logs in VS Code automatically
3. **Monitor Build Logs** - Watch real-time build progress
4. **Check Build Status** - Verify what's built
5. **Build [Library] (with monitoring)** - Build individual libraries

### Using Terminal

```powershell
# Open barebone PowerShell terminal (recommended)
# Terminal → New Terminal (will use workspace settings)

# Full production build
.\Build-Production-SlowMachine.ps1 -Clean

# Build with monitoring
.\Build-Production-SlowMachine.ps1 -Clean -MonitorInVSCode

# Monitor in separate terminal
.\build-scripts\Monitor-Build-Logs.ps1

# Check status
.\build-scripts\Check-Build-Status.ps1
```

---

## Troubleshooting

### MSBuild Not Found

```powershell
# Locate MSBuild
.\build-scripts\Find-MSBuild.ps1

# Or manually:
& "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" `
  -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe
```

### CMake Not in PATH

```powershell
# Add to current session
$env:PATH += ";C:\Program Files\CMake\bin"

# Add permanently (run as admin)
[Environment]::SetEnvironmentVariable(
    "Path",
    $env:Path + ";C:\Program Files\CMake\bin",
    "Machine"
)
```

### PowerShell Execution Policy

```powershell
# Check policy
Get-ExecutionPolicy

# Set to RemoteSigned (run as admin)
Set-ExecutionPolicy RemoteSigned -Scope CurrentUser
```

### Build Hangs on Slow Machine

Use the file-based monitoring approach:

```powershell
# Start build in background
Start-Job -ScriptBlock {
    Set-Location "C:\Path\To\DarkThumbs"
    & ".\build-scripts\Build-LibWebP-NMake.ps1" *>&1 | 
    Tee-Object -FilePath "build-logs\webp-$(Get-Date -Format 'yyyyMMdd-HHmmss').log"
}

# Monitor in VS Code
code "build-logs\webp-*.log"
```

---

## External Dependencies

These are downloaded automatically by the build scripts:

- **zlib** 1.3.1 - <https://zlib.net/>
- **LZ4** 1.10.0 - <https://github.com/lz4/lz4>
- **Zstandard** 1.5.7 - <https://github.com/facebook/zstd>
- **XZ Utils** 5.6.3 (liblzma) - <https://github.com/tukaani-project/xz>
- **LibWebP** 1.5.0 - <https://github.com/webmproject/libwebp>
- **Minizip-NG** 4.0.10 - <https://github.com/zlib-ng/minizip-ng>

Scripts will download to `external/` directory if not present.

---

## Quick Start

### First Time Setup

```powershell
# 1. Install tools
winget install Kitware.CMake
winget install Microsoft.PowerShell
winget install Microsoft.VisualStudioCode

# 2. Clone repository
git clone <repo-url> DarkThumbs
cd DarkThumbs

# 3. Open in VS Code
code .

# 4. Install VS Code extensions (from VS Code terminal)
code --install-extension ms-vscode.powershell
code --install-extension ms-vscode.cpptools
code --install-extension ms-vscode.cmake-tools

# 5. Run build
.\Build-Production-SlowMachine.ps1 -Clean -MonitorInVSCode
```

### Checking Everything Is Ready

```powershell
# Run environment check
.\build-scripts\Test-Build-Environment.ps1

# Or manual checks
cmake --version
pwsh --version
git --version
.\build-scripts\Find-MSBuild.ps1
```

---

## Performance Optimization

### For Slow Machines

1. **Use file-based monitoring** (not real-time terminal output)
2. **Disable antivirus** for workspace directory temporarily
3. **Close unnecessary applications** during builds
4. **Use SSD** if available
5. **Increase virtual memory** if RAM limited
6. **Build one library at a time** with `-SkipLibraries`

### VS Code Optimizations

The workspace settings already include:

- Disabled persistent sessions
- Disabled shell integration
- Reduced file watchers
- Disabled C++ IntelliSense (heavy)
- Disabled Git auto-refresh

---

## Additional Resources

- **[BUILD_INSTRUCTIONS.md](BUILD_INSTRUCTIONS.md)** - Detailed build guide for slow machines
- **[BUILD_FIXES_AND_LEARNINGS.md](BUILD_FIXES_AND_LEARNINGS.md)** - Troubleshooting guide
- **[BUILD_QUICKSTART.md](BUILD_QUICKSTART.md)** - Quick reference
- **[ROADMAP.md](ROADMAP.md)** - Project roadmap

---

**Last Updated:** January 7, 2026  
**For:** Windows 11 Build 26200.7462+ (x64)
