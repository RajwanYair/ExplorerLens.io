# Tool Discovery Guide - ExplorerLens Project

**Last Updated:** January 8, 2026  
**Purpose:** Automated detection of required build tools  
**For:** AI assistants, developers, CI/CD systems

---

## 🔧 Required Build Tools

### Core Requirements

| Tool | Version | Purpose | Detection Command |
|------|---------|---------|-------------------|
| **MSBuild** | 17.0+ | C++ project compilation | See detection script below |
| **Visual C++ Compiler (cl.exe)** | VS 2022+ | C++ compilation | Part of MSVC toolset |
| **MIDL** | Windows SDK | COM interface compilation | Part of Windows SDK |
| **CMake** | 3.15+ | External library builds | `cmake --version` |
| **NMake** | VS 2022+ | Makefile builds | Part of MSVC toolset |
| **Git** | 2.0+ | Version control | `git --version` |

### Optional Tools

| Tool | Purpose | Detection |
|------|---------|-----------|
| **Ninja** | Fast CMake builds | `ninja --version` |
| **PowerShell 7+** | Build scripts | `$PSVersionTable.PSVersion` |

---

## 🎯 Automated Tool Detection Script

Save as `build-scripts/Find-BuildTools.ps1`:

```powershell
# Find-BuildTools.ps1
# Automatically locates all required build tools for ExplorerLens
# Usage: .\build-scripts\Find-BuildTools.ps1 -Verbose

param(
    [switch]$Verbose,
    [switch]$ExportEnv
)

$ErrorActionPreference = "Continue"
$tools = @{}

Write-Host "`n🔍 ExplorerLens Build Tool Discovery`n" -ForegroundColor Cyan

# 1. Find Visual Studio Installations
Write-Host "1️⃣  Visual Studio Instances:" -ForegroundColor Yellow
$vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"

if (Test-Path $vsWhere) {
    $vsInstances = & $vsWhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -format json | ConvertFrom-Json
    
    if ($vsInstances) {
        $vs = $vsInstances | Select-Object -First 1
        Write-Host "   ✓ Found: $($vs.displayName)" -ForegroundColor Green
        Write-Host "     Path: $($vs.installationPath)" -ForegroundColor Gray
        Write-Host "     Version: $($vs.installationVersion)" -ForegroundColor Gray
        
        $tools['VSPath'] = $vs.installationPath
        $tools['VSVersion'] = $vs.installationVersion
    }
}

# 2. Find MSBuild
Write-Host "`n2️⃣  MSBuild:" -ForegroundColor Yellow
$msbuildPaths = @(
    "${env:ProgramFiles}\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\amd64\MSBuild.exe",
    "${env:ProgramFiles}\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\amd64\MSBuild.exe",
    "${env:ProgramFiles}\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\amd64\MSBuild.exe",
    "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2019\Enterprise\MSBuild\Current\Bin\amd64\MSBuild.exe"
)

$msbuild = $msbuildPaths | Where-Object { Test-Path $_ } | Select-Object -First 1

if ($msbuild) {
    $version = & $msbuild /version /nologo | Select-Object -Last 1
    Write-Host "   ✓ Found: $msbuild" -ForegroundColor Green
    Write-Host "     Version: $version" -ForegroundColor Gray
    $tools['MSBuild'] = $msbuild
}
else {
    Write-Host "   ✗ MSBuild not found" -ForegroundColor Red
}

# 3. Find MSVC Compiler (cl.exe)
Write-Host "`n3️⃣  MSVC Compiler (64-bit):" -ForegroundColor Yellow
if ($tools['VSPath']) {
    $vcPath = Join-Path $tools['VSPath'] "VC\Tools\MSVC"
    if (Test-Path $vcPath) {
        $latestMSVC = Get-ChildItem $vcPath | Sort-Object Name -Descending | Select-Object -First 1
        $clPath = Join-Path $latestMSVC.FullName "bin\Hostx64\x64\cl.exe"
        
        if (Test-Path $clPath) {
            Write-Host "   ✓ Found: $clPath" -ForegroundColor Green
            Write-Host "     MSVC Version: $($latestMSVC.Name)" -ForegroundColor Gray
            $tools['CL'] = $clPath
            $tools['MSVCVersion'] = $latestMSVC.Name
        }
    }
}

# 4. Find Windows SDK
Write-Host "`n4️⃣  Windows SDK:" -ForegroundColor Yellow
$sdkPaths = @(
    "${env:ProgramFiles(x86)}\Windows Kits\10\Include",
    "${env:ProgramFiles}\Windows Kits\10\Include"
)

$sdkPath = $sdkPaths | Where-Object { Test-Path $_ } | Select-Object -First 1

if ($sdkPath) {
    $sdkVersions = Get-ChildItem $sdkPath | Where-Object { $_.Name -match '^\d+\.\d+\.\d+\.\d+$' } | Sort-Object Name -Descending
    $latestSDK = $sdkVersions | Select-Object -First 1
    
    if ($latestSDK) {
        Write-Host "   ✓ Found: Windows SDK $($latestSDK.Name)" -ForegroundColor Green
        Write-Host "     Path: $($latestSDK.FullName)" -ForegroundColor Gray
        $tools['WindowsSDK'] = $latestSDK.Name
        $tools['WindowsSDKPath'] = $latestSDK.FullName
    }
}

# 5. Find CMake
Write-Host "`n5️⃣  CMake:" -ForegroundColor Yellow
$cmakePath = (Get-Command cmake -ErrorAction SilentlyContinue).Source

if ($cmakePath) {
    $cmakeVersion = & cmake --version | Select-Object -First 1
    Write-Host "   ✓ Found: $cmakePath" -ForegroundColor Green
    Write-Host "     $cmakeVersion" -ForegroundColor Gray
    $tools['CMake'] = $cmakePath
}
else {
    Write-Host "   ⚠ CMake not in PATH" -ForegroundColor Yellow
    Write-Host "     Tip: Install from https://cmake.org/ or use VS installer" -ForegroundColor Gray
}

# 6. Find Git
Write-Host "`n6️⃣  Git:" -ForegroundColor Yellow
$gitPath = (Get-Command git -ErrorAction SilentlyContinue).Source

if ($gitPath) {
    $gitVersion = & git --version
    Write-Host "   ✓ Found: $gitPath" -ForegroundColor Green
    Write-Host "     $gitVersion" -ForegroundColor Gray
    $tools['Git'] = $gitPath
}
else {
    Write-Host "   ⚠ Git not in PATH" -ForegroundColor Yellow
}

# 7. Find Developer Command Prompt
Write-Host "`n7️⃣  Developer Command Prompt:" -ForegroundColor Yellow
if ($tools['VSPath']) {
    $devCmd = Join-Path $tools['VSPath'] "Common7\Tools\VsDevCmd.bat"
    if (Test-Path $devCmd) {
        Write-Host "   ✓ Found: $devCmd" -ForegroundColor Green
        $tools['VsDevCmd'] = $devCmd
    }
}

# Summary
Write-Host "`n" + "="*70 -ForegroundColor Cyan
Write-Host "Summary: Found $($tools.Count) tools" -ForegroundColor Cyan
Write-Host "="*70 + "`n" -ForegroundColor Cyan

$required = @('MSBuild', 'CL', 'WindowsSDK')
$missing = $required | Where-Object { -not $tools.ContainsKey($_) }

if ($missing) {
    Write-Host "⚠ Missing required tools:" -ForegroundColor Yellow
    foreach ($tool in $missing) {
        Write-Host "   - $tool" -ForegroundColor Red
    }
    Write-Host "`nInstall Visual Studio 2022 with 'Desktop development with C++' workload`n" -ForegroundColor Yellow
}
else {
    Write-Host "✓ All required tools found!" -ForegroundColor Green
}

# Export to environment file
if ($ExportEnv) {
    $envFile = Join-Path $PSScriptRoot "..\build-env.json"
    $tools | ConvertTo-Json -Depth 10 | Set-Content $envFile
    Write-Host "`n📝 Exported tool paths to: $envFile" -ForegroundColor Cyan
}

# Verbose output
if ($Verbose) {
    Write-Host "`n📋 Detailed Tool Information:" -ForegroundColor Cyan
    $tools | Format-Table -AutoSize
}

return $tools
```

---

## 🚀 Quick Start Commands

### For Developers

```powershell
# Run tool discovery
.\build-scripts\Find-BuildTools.ps1 -Verbose

# Export to environment file
.\build-scripts\Find-BuildTools.ps1 -ExportEnv

# Open Developer Command Prompt (64-bit)
# Method 1: Through VS Start Menu
# "x64 Native Tools Command Prompt for VS 2022"

# Method 2: Programmatically
$vsPath = "C:\Program Files\Microsoft Visual Studio\2022\Professional"
& "$vsPath\Common7\Tools\VsDevCmd.bat" -arch=x64
```

### For CI/CD

```yaml
# GitHub Actions example
- name: Setup Build Tools
  run: |
    .\build-scripts\Find-BuildTools.ps1 -ExportEnv
    $tools = Get-Content build-env.json | ConvertFrom-Json
    echo "MSBuild: $($tools.MSBuild)"
```

---

## 📍 Tool Installation Paths

### Visual Studio 2022 (Default Locations)

```
VS Installation:
  C:\Program Files\Microsoft Visual Studio\2022\{Edition}\

MSBuild:
  C:\Program Files\Microsoft Visual Studio\2022\{Edition}\MSBuild\Current\Bin\amd64\MSBuild.exe

MSVC Compiler:
  C:\Program Files\Microsoft Visual Studio\2022\{Edition}\VC\Tools\MSVC\{Version}\bin\Hostx64\x64\cl.exe

Windows SDK:
  C:\Program Files (x86)\Windows Kits\10\Include\{Version}\

Developer Command Prompt:
  C:\Program Files\Microsoft Visual Studio\2022\{Edition}\Common7\Tools\VsDevCmd.bat
```

### External Tools

```
CMake:
  C:\Program Files\CMake\bin\cmake.exe
  (or via VS Installer: Individual Components → CMake tools)

Git:
  C:\Program Files\Git\cmd\git.exe
  (Download: https://git-scm.com/)
```

---

## 🔧 Environment Setup for AI Assistants

### Standard Detection Sequence

1. **Run Find-BuildTools.ps1** first to cache tool locations
2. **Use cached paths** for subsequent operations
3. **Re-run detection** if tools are updated or moved

### Example Usage in AI Workflows

```powershell
# Step 1: Detect tools (once per session)
$global:BuildTools = .\build-scripts\Find-BuildTools.ps1

# Step 2: Use detected tools
& $BuildTools.MSBuild LENSShell.vcxproj /p:Configuration=Release /p:Platform=x64

# Step 3: Verify tools are still valid
if (-not (Test-Path $BuildTools.MSBuild)) {
    Write-Warning "MSBuild path changed, re-running detection..."
    $global:BuildTools = .\build-scripts\Find-BuildTools.ps1
}
```

---

## ⚙️ Platform Configuration

### 64-bit Only (x64)

This project **ONLY** supports 64-bit builds. 32-bit (Win32/x86) is NOT supported.

**Configurations:**
- ✅ Debug|x64
- ✅ Release|x64
- ❌ Debug|Win32 (removed)
- ❌ Release|Win32 (removed)

**Build Commands:**
```powershell
# Always specify Platform=x64
msbuild LENSShell.sln /p:Platform=x64 /p:Configuration=Release

# Do NOT use Win32 or x86
# msbuild LENSShell.sln /p:Platform=Win32  # ❌ Will fail
```

---

## 📦 External Libraries

### Library Build Tools

Some external libraries require specific tools:

| Library | Tool | Notes |
|---------|------|-------|
| **zlib** | CMake + NMake | Use x64 Native Tools prompt |
| **zstd** | NMake | Makefile.vc in lib/ directory |
| **libwebp** | NMake | Makefile.vc in root |
| **minizip-ng** | CMake | Generate VS project files |
| **lzma/xz** | CMake | Generate NMake Makefiles |

### Build External Libraries

```powershell
# From x64 Native Tools Command Prompt
cd external\compression\zlib-1.3.1
cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release .
nmake

# Or use project build scripts
.\build-scripts\Build-AllComponents.ps1
```

---

## 🐛 Troubleshooting

### "MSBuild not found"

```powershell
# Find MSBuild manually
$msbuild = Get-ChildItem "C:\Program Files\Microsoft Visual Studio" -Recurse -Filter "MSBuild.exe" -ErrorAction SilentlyContinue | 
    Where-Object { $_.FullName -match "amd64" } | 
    Select-Object -First 1 -ExpandProperty FullName

# Add to PATH
$env:PATH = "$env:PATH;$(Split-Path $msbuild)"
```

### "cl.exe not found"

Open **x64 Native Tools Command Prompt for VS 2022** instead of regular PowerShell.

Or initialize environment:
```powershell
& "C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\Tools\VsDevCmd.bat" -arch=x64
```

### "CMake not found"

Install via:
1. Visual Studio Installer → Individual Components → "CMake tools for Windows"
2. Or download from: https://cmake.org/download/

---

## 📚 Related Documentation

- [Build Monitoring Guidelines](.github/BUILD_MONITORING_GUIDELINES.md)
- [AI Build Instructions](.github/AI_BUILD_INSTRUCTIONS.md)
- [Build Scripts](../build-scripts/)

---

**Last Updated:** January 8, 2026  
**Maintained By:** ExplorerLens Project Team

