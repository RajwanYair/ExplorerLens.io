# DarkThumbs Build Scripts & Environment Setup

## 🚀 Quick Start

### One-Time Installation (Recommended)

Make all build tools available automatically in every PowerShell session:

```powershell
cd C:\Users\ryair\OneDrive - Intel Corporation\Documents\MyScripts\DarkThumbs\scripts
.\Install-DevEnvironment.ps1
```

**What this does:**
- ✅ Adds DarkThumbs environment to your PowerShell profile
- ✅ Loads MSVC tools (CL, NMake, Link, RC) automatically
- ✅ Verifies CMake, MSBuild, Git availability
- ✅ Creates convenient build aliases (dtbuild, dttest, dtclean)
- ✅ No more searching for tools every session!

### Manual Load (Without Installation)

If you prefer to load manually each session:

```powershell
.\Setup-DevEnvironment.ps1
```

---

## 📁 Scripts Overview

### Core Environment

| Script | Purpose |
|--------|---------|
| **Install-DevEnvironment.ps1** | Install to PowerShell profile (one-time) |
| **Setup-DevEnvironment.ps1** | Main environment setup script |

### Build Tasks (see `build-scripts/`)

- `Build-Production-SlowMachine.ps1` - Full production build
- `Monitor-Build-Logs.ps1` - Watch build progress
- `Check-Build-Status.ps1` - Verify build completion
- `Build-With-Monitoring.ps1` - Build with live monitoring
- And many more...

---

## ⚙️ Environment Configuration

After installation, these tools are immediately available:

- **MSVC Compiler** (CL.exe 19.44.35207)
- **MSBuild** (18.3.0)
- **NMake** (14.44.35207)
- **CMake** (4.2.1)
- **Git** (via Scoop)
- **Ninja** (via Scoop)
- **Link, RC, other VC tools**

### Hard-Coded Paths (No Searching!)

```powershell
$Global:DarkThumbsConfig = @{
    VSPath      = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools"
    MSVCVersion = "14.44.35207"
    WindowsSDK  = "10.0.26100.0"
    MSBuild     = "...\MSBuild.exe"
    CMake       = "C:\Users\ryair\scoop\shims\cmake.exe"
    Git         = "C:\Users\ryair\scoop\shims\git.exe"
    ProjectRoot = "C:\Users\ryair\OneDrive - Intel Corporation\Documents\MyScripts\DarkThumbs"
}
```

---

## 🛠️ Build Aliases

After environment is loaded:

```powershell
# Quick build commands
dtbuild          # Show available commands
dtbuild Release  # Build full solution (Release, x64)
dtbuild Engine   # Build Engine only (CMake)
dtbuild Shell    # Build CBXShell only
dtbuild Clean    # Clean all artifacts
dtbuild Rebuild  # Clean + Release build

# Testing
dttest           # Run Engine tests

# Information
Show-DarkThumbsInfo  # Display environment details
Test-BuildTools      # Verify all tools work
```

---

## 🧪 Verification

### Check Installation

```powershell
# After installing to profile, restart PowerShell and run:
Show-DarkThumbsInfo

# Expected output:
# ═══════════════════════════════════════════════════════
#  DarkThumbs Development Environment
# ═══════════════════════════════════════════════════════
# 
# Visual Studio:
#   Path:    C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools
#   MSVC:    14.44.35207
#   SDK:     10.0.26100.0
# 
# Build Tools:
#   MSBuild: 18.3.0
#   CMake:   4.2.1
#   Git:     2.x.x
```

### Test Build Tools

```powershell
Test-BuildTools

# Expected output:
#   ✅ MSBuild (18.3.0)
#   ✅ CMake (4.2.1)
#   ✅ Git (2.x.x)
#   ✅ MSVC (CL) (19.44.x)
#   ✅ NMake
#   ✅ Link
#   ✅ RC
#   ✅ Ninja (1.x.x)
```

---

## 🔄 Uninstall

To remove from PowerShell profile:

```powershell
.\Install-DevEnvironment.ps1 -Uninstall
```

Your profile will be backed up before any changes.

---

## 🐛 Troubleshooting

### "CL.exe not found"

```powershell
# Reload environment
Setup-DarkThumbsEnv -Force

# Or manually load MSVC
Load-MSVCEnvironment
```

### "Environment not loading automatically"

```powershell
# Check profile exists and has the entry
Get-Content $PROFILE | Select-String "DarkThumbs"

# If not found, reinstall:
.\Install-DevEnvironment.ps1 -Force
```

### "Tools work but aliases don't"

```powershell
# Reload full environment
Setup-DarkThumbsEnv -Force
```

---

## 📚 Additional Documentation

- **Tool Versions:** `.github/TOOL_VERSIONS.md`
- **Quick Reference:** `.github/BUILD_QUICK_REFERENCE.md`
- **AI Instructions:** `.github/AI_BUILD_INSTRUCTIONS.md`
- **Project Roadmap:** `../ROADMAP.md`

---

## ⚡ Performance

Environment setup is optimized for speed:

- **First load:** ~2-3 seconds (MSVC environment loading)
- **Subsequent calls:** Instant (environment cached)
- **No PATH searching:** All paths hard-coded
- **Lazy loading:** MSVC loaded only when needed

---

## 📝 Version History

| Date | Version | Changes |
|------|---------|---------|
| 2026-02-09 | 1.0 | Initial release - persistent environment setup |

---

*Last Updated: February 9, 2026*
