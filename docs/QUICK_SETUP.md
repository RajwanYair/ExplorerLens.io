# Quick Setup Guide - DarkThumbs

**For slow machines with file-based monitoring**

## Prerequisites (5 minutes)

```powershell
# Install tools
winget install Kitware.CMake
winget install Microsoft.PowerShell
winget install Microsoft.VisualStudioCode

# Install VS extensions
code --install-extension ms-vscode.powershell
code --install-extension ms-vscode.cpptools
```

## Clone & Open (2 minutes)

```powershell
git clone <repo-url> DarkThumbs
cd DarkThumbs
code .
```

## Build (30-60 minutes on slow machines)

### Option 1: Full Auto Build with VS Code Monitoring

```powershell
.\Build-Production-SlowMachine.ps1 -Clean -MonitorInVSCode
```

This will:

- Clean previous builds
- Build all 6 libraries
- Open log files in VS Code
- Show progress in `build-progress.json`

### Option 2: Manual Monitoring

```powershell
# Terminal 1: Start build
.\Build-Production-SlowMachine.ps1 -Clean

# Terminal 2: Monitor
.\build-scripts\Monitor-Build-Logs.ps1
```

### Option 3: VS Code Tasks

Press `Ctrl+Shift+B` (default build task)

Or press `Ctrl+Shift+P` → `Tasks: Run Task` → `Build with VS Code Monitoring`

## Check Status

```powershell
.\build-scripts\Check-Build-Status.ps1
```

Shows ✅ for built libraries, ❌ for missing.

## Troubleshooting

### Build hangs?

- Check `build-logs/build-progress.json` for current step
- Open latest log in `build-logs/` directory
- Expected time: 5-20 minutes per library on slow machines

### Script errors?

```powershell
# Set execution policy
Set-ExecutionPolicy RemoteSigned -Scope CurrentUser

# Find MSBuild
.\build-scripts\Find-MSBuild.ps1
```

### Need to rebuild one library?

```powershell
# Example: Rebuild LZMA with monitoring
.\build-scripts\Build-With-Monitoring.ps1 -ScriptPath ".\build-scripts\build-lzma-simple.ps1" -Watch
```

## VS Code Tips

- Log files auto-update in editor (no reload needed)
- Open `build-logs/build-progress.json` to see current task
- Use `Ctrl+P` → `build-logs/` to quickly open logs
- Terminal uses barebone PowerShell (fast, no profile)

## Full Documentation

- [BUILD_INSTRUCTIONS.md](BUILD_INSTRUCTIONS.md) - Complete guide
- [TOOLS_SETUP.md](TOOLS_SETUP.md) - All tools and utilities
- [BUILD_FIXES_AND_LEARNINGS.md](BUILD_FIXES_AND_LEARNINGS.md) - Troubleshooting

---

**Estimated Total Time:** 45-90 minutes (setup + build on slow machine)
