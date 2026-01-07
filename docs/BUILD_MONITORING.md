# Build Monitoring and Performance Optimization Guide

## For Slow Machines

This guide provides tools and configurations optimized for machines with slow performance, high latency, or resource constraints.

---

## Quick Start

### 1. Install Barebone PowerShell Profile

```powershell
# Copy the barebone profile to your PowerShell profile location
Copy-Item scripts\setup\barebone-profile.ps1 $PROFILE -Force

# Or manually install
.\scripts\setup\barebone-profile.ps1
```

### 2. Use File-Based Build Monitoring

Instead of watching build output in the terminal (which can be slow), use file-based monitoring:

```powershell
# Start a monitored build
.\build-scripts\utilities\Monitor-Build.ps1 -BuildScript "Build-Production.ps1" -Arguments "-Clean"

# Monitor an existing build
Build-Monitor

# Check build status
Build-Status

# Live monitoring (refreshes every 10 seconds)
Start-BuildMonitor
```

---

## Tools and Utilities

### Monitor-Build.ps1

**Purpose**: Runs build scripts with output redirected to log files, monitors progress via file system instead of shell buffering.

**Usage**:

```powershell
# Basic usage
.\build-scripts\utilities\Monitor-Build.ps1

# With custom script
.\build-scripts\utilities\Monitor-Build.ps1 -BuildScript "Build-Production.ps1" -Arguments "-Clean"

# Faster monitoring (check every 3 seconds)
.\build-scripts\utilities\Monitor-Build.ps1 -MonitorInterval 3

# Silent mode (no progress updates)
.\build-scripts\utilities\Monitor-Build.ps1 -ShowProgress:$false
```

**Benefits**:

- No shell buffering issues
- Works reliably on slow machines
- Persistent logs for later analysis
- Progress tracking without blocking

---

## PowerShell Profile Options

### Barebone Profile (Recommended for Slow Machines)

Location: `scripts\setup\barebone-profile.ps1`

**Features**:

- Minimal overhead
- No fancy prompts
- No module loading
- Fast startup time
- Essential aliases only

**Available Commands**:

- `dt` - Navigate to DarkThumbs directory
- `bs` - Navigate to build-scripts
- `src` - Navigate to CBXShell source
- `Build-Quick [-Clean]` - Quick build without libraries
- `Build-Monitor` - Watch latest build log
- `Build-Status` - Show recent build results
- `Start-BuildMonitor` - Live file-based monitoring

### Installation

```powershell
# Option 1: Direct copy
Copy-Item scripts\setup\barebone-profile.ps1 $PROFILE -Force

# Option 2: Backup and install
$backup = "$PROFILE.backup-$(Get-Date -Format 'yyyyMMdd-HHmmss')"
if (Test-Path $PROFILE) { Copy-Item $PROFILE $backup }
Copy-Item scripts\setup\barebone-profile.ps1 $PROFILE -Force

# Option 3: Use -NoProfile flag (no installation)
pwsh -NoProfile
```

---

## VSCode Settings for Slow Machines

The `.vscode\settings.json` file includes optimizations:

```json
{
    // Disable terminal persistence (faster)
    "terminal.integrated.persistentSessionReviveProcess": "never",
    "terminal.integrated.enablePersistentSessions": false,
    
    // Reduce scrollback buffer
    "terminal.integrated.scrollback": 5000,
    
    // Exclude build directories from file watching
    "files.watcherExclude": {
        "**/build/**": true,
        "**/x64/**": true,
        "**/external/**": true
    },
    
    // Disable IntelliSense for better performance
    "C_Cpp.intelliSenseEngine": "disabled",
    
    // Disable auto-refresh/fetch
    "git.autorefresh": false,
    "git.autofetch": false,
    
    // Use PowerShell with -NoProfile
    "terminal.integrated.defaultProfile.windows": "PowerShell",
    "terminal.integrated.profiles.windows": {
        "PowerShell": {
            "args": ["-NoProfile", "-NoLogo"]
        }
    }
}
```

---

## GitHub Actions Configuration

The `.github\workflows\build.yml` has been updated with:

### Extended Timeouts

- **Job timeout**: 90 minutes (was 30)
- **NuGet restore**: 15 minutes (was 5)
- **Build step**: 60 minutes (was 20)

### File-Based Logging

All build output is redirected to timestamped log files in `build-logs/`:

```
build-logs/
  github-build-2026-01-06_153045.log
  github-build-2026-01-06_153045.log.stdout
  github-build-2026-01-06_153045.log.stderr
```

### Progress Monitoring

The workflow monitors build progress by checking log file size every 15 seconds instead of relying on shell output.

---

## Best Practices for Slow Machines

### 1. Use File-Based Monitoring

**Don't do this**:

```powershell
.\Build-Production.ps1  # Output goes to terminal, can buffer/hang
```

**Do this**:

```powershell
.\build-scripts\utilities\Monitor-Build.ps1
```

### 2. Disable IntelliSense

For large C++ projects, IntelliSense can be very slow:

```json
"C_Cpp.intelliSenseEngine": "disabled"
```

### 3. Start PowerShell with -NoProfile

```powershell
pwsh -NoProfile
```

Or configure VSCode to always use `-NoProfile`:

```json
"terminal.integrated.profiles.windows": {
    "PowerShell": {
        "args": ["-NoProfile", "-NoLogo"]
    }
}
```

### 4. Exclude Build Directories

Prevent file watchers from scanning large build directories:

```json
"files.watcherExclude": {
    "**/build/**": true,
    "**/x64/**": true,
    "**/external/**": true,
    "**/packages/**": true
}
```

### 5. Build in Stages

Don't rebuild everything at once:

```powershell
# Build libraries first
.\build-scripts\Build-All-Libraries-v2.ps1

# Then build main project (skip libraries)
.\Build-Production.ps1 -SkipLibraries
```

### 6. Monitor Log Files Directly

Instead of watching terminal output, tail log files:

```powershell
Get-Content .\build-logs\production-build-*.log -Wait -Tail 20
```

---

## Troubleshooting

### Build seems stuck

**Symptoms**: No output for several minutes, terminal frozen

**Solution**:

1. Use file-based monitoring
2. Check log file directly:

   ```powershell
   Get-ChildItem build-logs\*.log -File | Sort-Object LastWriteTime -Descending | Select-Object -First 1 | Get-Content -Tail 30
   ```

### Terminal is very slow

**Solution**:

1. Use `-NoProfile` flag
2. Disable terminal persistence in VSCode
3. Reduce scrollback buffer

### IntelliSense causes freezing

**Solution**:

```json
"C_Cpp.intelliSenseEngine": "disabled"
```

### File watcher uses too much CPU

**Solution**:
Add exclusions in `.vscode\settings.json`:

```json
"files.watcherExclude": {
    "**/node_modules/**": true,
    "**/build/**": true,
    "**/x64/**": true
}
```

---

## Command Reference

### Build Commands

```powershell
# Full build with monitoring
.\build-scripts\utilities\Monitor-Build.ps1

# Clean build
.\build-scripts\utilities\Monitor-Build.ps1 -Arguments "-Clean"

# Build without libraries
.\Build-Production.ps1 -SkipLibraries

# Build specific library
.\build-scripts\Build-Zlib.ps1
```

### Monitoring Commands

```powershell
# Watch latest log (from profile)
Build-Monitor

# Check recent builds (from profile)
Build-Status

# Live monitoring with auto-refresh (from profile)
Start-BuildMonitor

# Manual log check
Get-Content .\build-logs\production-build-*.log -Tail 50
```

### Profile Commands

```powershell
# Navigate
dt      # Go to DarkThumbs root
bs      # Go to build-scripts
src     # Go to CBXShell source

# Build
Build-Quick           # Quick build
Build-Quick -Clean    # Clean build

# Monitor
Build-Monitor         # Tail latest log
Build-Status          # Show recent builds
Start-BuildMonitor    # Live monitoring
```

---

## Performance Tips

1. **Close unnecessary applications** during builds
2. **Disable antivirus scanning** for build directories temporarily
3. **Use SSD** instead of HDD if possible
4. **Increase virtual memory** (pagefile) if RAM is limited
5. **Build with `/m:1`** to reduce parallel overhead on slow CPUs
6. **Use ccache or similar** for incremental builds

---

## Files Modified/Created

### Created

- `build-scripts\utilities\Monitor-Build.ps1` - File-based build monitor
- `scripts\setup\barebone-profile.ps1` - Minimal PowerShell profile
- This README

### Modified

- `.github\workflows\build.yml` - Extended timeouts, file-based logging
- `.vscode\settings.json` - Performance optimizations

---

## Support

For issues or questions, check the logs in:

- `build-logs\` - Build logs
- `.vscode\settings.json` - VSCode configuration
- `scripts\setup\` - Profile and setup scripts

---

**Last Updated**: 2026-01-06  
**Version**: 1.0
