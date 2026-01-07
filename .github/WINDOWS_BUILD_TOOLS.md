# Windows Build Tools Setup Guide

This document provides comprehensive instructions for finding and configuring all necessary Windows build tools for DarkThumbs development.

## Table of Contents

- [Visual Studio Build Tools](#visual-studio-build-tools)
- [CMake](#cmake)
- [Ninja Build System](#ninja-build-system)
- [vcpkg Package Manager](#vcpkg-package-manager)
- [PowerShell Scripts for Tool Detection](#powershell-scripts-for-tool-detection)
- [Automated Setup](#automated-setup)

---

## Visual Studio Build Tools

### Required Version

- **Visual Studio 2026 Build Tools** (or Visual Studio 2022/2026 with C++ workload)
- **Platform Toolset**: v143 or v180
- **Windows SDK**: 10.0.22621.0 or later

### Detection Script

```powershell
# Find-VSBuildTools.ps1
function Find-VSBuildTools {
    $possiblePaths = @(
        "C:\Program Files (x86)\Microsoft Visual Studio\2026\BuildTools",
        "C:\Program Files (x86)\Microsoft Visual Studio\2026\Community",
        "C:\Program Files (x86)\Microsoft Visual Studio\2026\Professional",
        "C:\Program Files (x86)\Microsoft Visual Studio\2026\Enterprise",
        "C:\Program Files (x86)\Microsoft Visual Studio\2025\BuildTools",
        "C:\Program Files (x86)\Microsoft Visual Studio\2025\Community",
        "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools",
        "C:\Program Files (x86)\Microsoft Visual Studio\2022\Community",
        "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools",
        "C:\Program Files (x86)\Microsoft Visual Studio\17\BuildTools"
    )
    
    foreach ($path in $possiblePaths) {
        if (Test-Path $path) {
            return $path
        }
    }
    
    return $null
}

# Find vcvarsall.bat
function Find-VCVarsAll {
    $vsPath = Find-VSBuildTools
    if ($vsPath) {
        $vcvarsall = Join-Path $vsPath "VC\Auxiliary\Build\vcvarsall.bat"
        if (Test-Path $vcvarsall) {
            return $vcvarsall
        }
    }
    return $null
}

# Find MSBuild.exe
function Find-MSBuild {
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    
    if (Test-Path $vswhere) {
        $msbuildPath = & $vswhere -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe | Select-Object -First 1
        if ($msbuildPath -and (Test-Path $msbuildPath)) {
            return $msbuildPath
        }
    }
    
    # Fallback to manual search
    $possiblePaths = @(
        "C:\Program Files (x86)\Microsoft Visual Studio\2026\BuildTools\MSBuild\Current\Bin\amd64\MSBuild.exe",
        "C:\Program Files (x86)\Microsoft Visual Studio\2026\Community\MSBuild\Current\Bin\amd64\MSBuild.exe",
        "C:\Program Files (x86)\Microsoft Visual Studio\2025\BuildTools\MSBuild\Current\Bin\amd64\MSBuild.exe",
        "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\amd64\MSBuild.exe",
        "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\MSBuild\Current\Bin\amd64\MSBuild.exe"
    )
    
    foreach ($path in $possiblePaths) {
        if (Test-Path $path) {
            return $path
        }
    }
    
    return $null
}
```

### Installation

If Visual Studio Build Tools are not found:

**Option 1: Via Winget**

```powershell
winget install Microsoft.VisualStudio.2022.BuildTools --silent --override "--wait --quiet --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended"
```

**Option 2: Via Chocolatey**

```powershell
choco install visualstudio2022buildtools --package-parameters "--add Microsoft.VisualStudio.Workload.VCTools --includeRecommended"
```

**Option 3: Manual Download**

- Download from: <https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022>
- Run installer and select "Desktop development with C++" workload
- Ensure these components are selected:
  - MSVC v143 - VS 2022 C++ x64/x86 build tools (Latest)
  - Windows 11 SDK (10.0.22621.0)
  - C++ CMake tools for Windows

---

## CMake

### Required Version

- **CMake 3.20 or later**

### Detection Script

```powershell
function Find-CMake {
    # Check if cmake is in PATH
    $cmake = Get-Command cmake -ErrorAction SilentlyContinue
    if ($cmake) {
        return $cmake.Source
    }
    
    # Check common installation paths
    $possiblePaths = @(
        "$env:ProgramFiles\CMake\bin\cmake.exe",
        "$env:ProgramFiles(x86)\CMake\bin\cmake.exe",
        "$env:LOCALAPPDATA\Programs\CMake\bin\cmake.exe",
        "C:\Program Files\CMake\bin\cmake.exe",
        "$env:USERPROFILE\scoop\shims\cmake.exe"
    )
    
    foreach ($path in $possiblePaths) {
        if (Test-Path $path) {
            return $path
        }
    }
    
    return $null
}
```

### Installation

**Option 1: Via Scoop (Recommended)**

```powershell
scoop install cmake
```

**Option 2: Via Chocolatey**

```powershell
choco install cmake --installargs 'ADD_CMAKE_TO_PATH=System'
```

**Option 3: Via Winget**

```powershell
winget install Kitware.CMake
```

**Option 4: Manual Download**

- Download from: <https://cmake.org/download/>
- Install and check "Add CMake to system PATH"

---

## Ninja Build System

### Required Version

- **Ninja 1.11 or later**

### Detection Script

```powershell
function Find-Ninja {
    # Check if ninja is in PATH
    $ninja = Get-Command ninja -ErrorAction SilentlyContinue
    if ($ninja) {
        return $ninja.Source
    }
    
    # Check common installation paths
    $possiblePaths = @(
        "$env:LOCALAPPDATA\Programs\ninja\ninja.exe",
        "C:\Tools\ninja\ninja.exe",
        "$env:USERPROFILE\scoop\shims\ninja.exe",
        "$env:ProgramFiles\Ninja\ninja.exe"
    )
    
    foreach ($path in $possiblePaths) {
        if (Test-Path $path) {
            return $path
        }
    }
    
    return $null
}
```

### Installation

**Option 1: Via Scoop (Recommended)**

```powershell
scoop install ninja
```

**Option 2: Via Chocolatey**

```powershell
choco install ninja
```

**Option 3: Manual Download**

```powershell
# Download latest Ninja binary
$ninjaUrl = "https://github.com/ninja-build/ninja/releases/download/v1.12.1/ninja-win.zip"
$ninjaZip = "$env:TEMP\ninja-win.zip"
$ninjaDir = "C:\Tools\ninja"

# Create directory
New-Item -ItemType Directory -Path $ninjaDir -Force | Out-Null

# Download and extract
Invoke-WebRequest -Uri $ninjaUrl -OutFile $ninjaZip -UseBasicParsing
Expand-Archive -Path $ninjaZip -DestinationPath $ninjaDir -Force
Remove-Item $ninjaZip -Force

# Add to PATH for current session
$env:Path = "$ninjaDir;$env:Path"

# Add to system PATH permanently (requires admin)
[Environment]::SetEnvironmentVariable("Path", "$ninjaDir;$([Environment]::GetEnvironmentVariable('Path', 'Machine'))", "Machine")
```

---

## vcpkg Package Manager

### Required Version

- **Latest version** (auto-updates)

### Detection Script

```powershell
function Find-Vcpkg {
    # Check if vcpkg is in PATH
    $vcpkg = Get-Command vcpkg -ErrorAction SilentlyContinue
    if ($vcpkg) {
        return $vcpkg.Source
    }
    
    # Check common installation paths
    $possiblePaths = @(
        "$env:USERPROFILE\vcpkg\vcpkg.exe",
        "$env:USERPROFILE\scoop\apps\vcpkg\current\vcpkg.exe",
        "C:\vcpkg\vcpkg.exe",
        "C:\Tools\vcpkg\vcpkg.exe"
    )
    
    foreach ($path in $possiblePaths) {
        if (Test-Path $path) {
            return $path
        }
    }
    
    return $null
}
```

### Installation

**Option 1: Via Scoop (Recommended for DarkThumbs)**

```powershell
scoop install vcpkg
```

**Option 2: Via Git Clone**

```powershell
# Clone repository
cd $env:USERPROFILE
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg

# Bootstrap
.\bootstrap-vcpkg.bat

# Integrate with system
.\vcpkg integrate install

# Add to PATH
$vcpkgPath = (Get-Location).Path
$env:Path = "$vcpkgPath;$env:Path"
[Environment]::SetEnvironmentVariable("Path", "$vcpkgPath;$([Environment]::GetEnvironmentVariable('Path', 'User'))", "User")
```

---

## PowerShell Scripts for Tool Detection

### Unified Tool Finder Script

Create `build-scripts\Find-All-Tools.ps1`:

```powershell
# Find-All-Tools.ps1
# Detects all required build tools for DarkThumbs

$ErrorActionPreference = "Continue"

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "DarkThumbs Build Tools Detector" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

$tools = @{
    "Visual Studio Path" = $null
    "vcvarsall.bat" = $null
    "MSBuild.exe" = $null
    "CMake" = $null
    "Ninja" = $null
    "vcpkg" = $null
}

# Find Visual Studio
$vsPaths = @(
    "C:\Program Files (x86)\Microsoft Visual Studio\2026\BuildTools",
    "C:\Program Files (x86)\Microsoft Visual Studio\2026\Community",
    "C:\Program Files (x86)\Microsoft Visual Studio\2025\BuildTools",
    "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools",
    "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools"
)

foreach ($path in $vsPaths) {
    if (Test-Path $path) {
        $tools["Visual Studio Path"] = $path
        break
    }
}

# Find vcvarsall.bat
if ($tools["Visual Studio Path"]) {
    $vcvarsall = Join-Path $tools["Visual Studio Path"] "VC\Auxiliary\Build\vcvarsall.bat"
    if (Test-Path $vcvarsall) {
        $tools["vcvarsall.bat"] = $vcvarsall
    }
}

# Find MSBuild
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $vswhere) {
    $msbuildPath = & $vswhere -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe | Select-Object -First 1
    if ($msbuildPath) {
        $tools["MSBuild.exe"] = $msbuildPath
    }
}

# Find CMake
$cmake = Get-Command cmake -ErrorAction SilentlyContinue
if ($cmake) {
    $tools["CMake"] = $cmake.Source
}

# Find Ninja
$ninja = Get-Command ninja -ErrorAction SilentlyContinue
if ($ninja) {
    $tools["Ninja"] = $ninja.Source
}

# Find vcpkg
$vcpkg = Get-Command vcpkg -ErrorAction SilentlyContinue
if ($vcpkg) {
    $tools["vcpkg"] = $vcpkg.Source
}

# Display results
$allFound = $true
foreach ($tool in $tools.Keys) {
    if ($tools[$tool]) {
        Write-Host "✓ $tool" -ForegroundColor Green
        Write-Host "  $($tools[$tool])" -ForegroundColor Gray
    } else {
        Write-Host "✗ $tool - NOT FOUND" -ForegroundColor Red
        $allFound = $false
    }
}

Write-Host ""

if ($allFound) {
    Write-Host "All build tools found!" -ForegroundColor Green
    Write-Host "You can now build DarkThumbs." -ForegroundColor Cyan
} else {
    Write-Host "Some tools are missing." -ForegroundColor Yellow
    Write-Host "Please refer to .github\WINDOWS_BUILD_TOOLS.md for installation instructions." -ForegroundColor Yellow
}

Write-Host ""

# Return tools object
return $tools
```

---

## Automated Setup

### Complete Setup Script

Create `Setup-Build-Environment.ps1` in project root:

```powershell
# Setup-Build-Environment.ps1
# Automatically installs all required build tools

$ErrorActionPreference = "Stop"

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "DarkThumbs Build Environment Setup" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Check if running as administrator
$isAdmin = ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)

if (-not $isAdmin) {
    Write-Host "⚠ Not running as administrator" -ForegroundColor Yellow
    Write-Host "Some installations may require elevation" -ForegroundColor Yellow
    Write-Host ""
}

# Check for Scoop
$scoop = Get-Command scoop -ErrorAction SilentlyContinue

if (-not $scoop) {
    Write-Host "Installing Scoop package manager..." -ForegroundColor Cyan
    Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser -Force
    Invoke-RestMethod -Uri https://get.scoop.sh | Invoke-Expression
    
    # Refresh PATH
    $env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")
}

Write-Host "✓ Scoop available" -ForegroundColor Green
Write-Host ""

# Install required tools via Scoop
$tools = @("cmake", "ninja", "vcpkg")

foreach ($tool in $tools) {
    $cmd = Get-Command $tool -ErrorAction SilentlyContinue
    
    if (-not $cmd) {
        Write-Host "Installing $tool..." -ForegroundColor Cyan
        scoop install $tool
    } else {
        Write-Host "✓ $tool already installed" -ForegroundColor Green
    }
}

Write-Host ""

# Check for Visual Studio Build Tools
$vsPath = & "$PSScriptRoot\build-scripts\Find-All-Tools.ps1"

if (-not $vsPath["Visual Studio Path"]) {
    Write-Host "Visual Studio Build Tools not found!" -ForegroundColor Red
    Write-Host ""
    Write-Host "Please install Visual Studio 2022 Build Tools manually:" -ForegroundColor Yellow
    Write-Host "1. Download from: https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022" -ForegroundColor White
    Write-Host "2. Run installer and select 'Desktop development with C++'" -ForegroundColor White
    Write-Host "3. Run this script again after installation" -ForegroundColor White
    Write-Host ""
    
    # Offer to download installer
    $download = Read-Host "Download installer now? (y/n)"
    if ($download -eq "y") {
        Start-Process "https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022"
    }
    
    exit 1
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "Build Environment Setup Complete!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Cyan
Write-Host "1. Install external libraries: .\Install-External-Libraries.ps1" -ForegroundColor White
Write-Host "2. Build the project: .\Build-Production.ps1" -ForegroundColor White
Write-Host ""
```

---

## Environment Variables

### Required Environment Variables

Set these in your PowerShell profile or system environment:

```powershell
# Visual Studio
$env:VSINSTALLDIR = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\"
$env:VCToolsInstallDir = "$env:VSINSTALLDIR\VC\Tools\MSVC\14.43.35720\"

# Windows SDK
$env:WindowsSdkDir = "C:\Program Files (x86)\Windows Kits\10\"
$env:WindowsSDKVersion = "10.0.22621.0\"

# Add to PATH
$env:Path = "$env:VCToolsInstallDir\bin\Hostx64\x64;$env:Path"
$env:Path = "$env:WindowsSdkDir\bin\$env:WindowsSDKVersion\x64;$env:Path"
```

---

## Quick Reference

### One-Line Tool Checks

```powershell
# Check all tools at once
@("cmake","ninja","vcpkg","msbuild") | ForEach-Object { $cmd = Get-Command $_ -EA SilentlyContinue; if ($cmd) { Write-Host "✓ $_" -ForegroundColor Green } else { Write-Host "✗ $_" -ForegroundColor Red } }
```

### Tool Versions

```powershell
cmake --version
ninja --version
vcpkg version
& "$(& build-scripts\Find-MSBuild.ps1)" /version
```

---

## Troubleshooting

### PATH Issues

If tools aren't found after installation:

```powershell
# Refresh PATH in current session
$env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")
```

### vcvarsall.bat Not Working

If builds fail with "compiler not found":

```powershell
# Manually initialize VS environment
& "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64
```

### CMake Can't Find Compiler

```powershell
# Set compiler explicitly
$env:CC = "cl.exe"
$env:CXX = "cl.exe"
```

---

## Additional Resources

- [Visual Studio Build Tools Documentation](https://learn.microsoft.com/en-us/visualstudio/install/use-command-line-parameters-to-install-visual-studio)
- [CMake Documentation](https://cmake.org/documentation/)
- [Ninja Build System](https://ninja-build.org/)
- [vcpkg Documentation](https://vcpkg.io/en/getting-started.html)
- [Scoop Package Manager](https://scoop.sh/)

---

## Maintenance

This document should be updated when:

- New Visual Studio versions are released
- Tool version requirements change
- New build dependencies are added
- Installation methods are deprecated

**Last Updated:** January 6, 2026
**Maintained By:** DarkThumbs Development Team
