# Windows Build Tools - Installation & Configuration

**Last Updated:** January 8, 2026  
**Target OS:** Windows 10/11 x64  
**Installation Method:** winget (preferred) → choco → scoop

---

## Required Tools

### 1. Visual Studio 2022 Build Tools
**Version:** 17.8+ (v143 toolset, MSVC 19.38+)

**Installation via winget:**
```powershell
winget install --id Microsoft.VisualStudio.2022.BuildTools --override "--quiet --add Microsoft.VisualStudio.Workload.VCTools --add Microsoft.VisualStudio.Component.VC.Tools.x86.x64 --add Microsoft.VisualStudio.Component.Windows11SDK.22621"
```

**Installation via Visual Studio Installer:**
- Workload: "Desktop development with C++"
- Individual components:
  - MSVC v143 - VS 2022 C++ x64/x86 build tools
  - Windows 11 SDK (10.0.22621.0)
  - C++ CMake tools for Windows

**Verification:**
```powershell
& "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
cl.exe /? | Select-String "Version"
```
Expected: `Version 19.38` or higher

**Path (default):**
```
C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\
```

---

### 2. CMake
**Version:** 3.28+

**Installation via winget:**
```powershell
winget install Kitware.CMake
```

**Installation via installer:**
- Download from https://cmake.org/download/
- Select "Add CMake to system PATH for all users"

**Verification:**
```powershell
cmake --version
```
Expected: `cmake version 3.28.0` or higher

**Path (default):**
```
C:\Program Files\CMake\bin\cmake.exe
```

---

### 3. Ninja Build System
**Version:** 1.11+

**Installation via winget:**
```powershell
winget install Ninja-build.Ninja
```

**Installation via GitHub release:**
```powershell
# Download ninja-win.zip from https://github.com/ninja-build/ninja/releases
# Extract to C:\Tools\ninja\
# Add to PATH: $env:Path += ";C:\Tools\ninja"
```

**Verification:**
```powershell
ninja --version
```
Expected: `1.11.1` or higher

**Path (typical):**
```
C:\Program Files\Ninja\ninja.exe
or
C:\Tools\ninja\ninja.exe
```

---

### 4. Git
**Version:** 2.43+

**Installation via winget:**
```powershell
winget install Git.Git
```

**Configuration:**
```powershell
git config --global core.autocrlf true
git config --global user.name "Your Name"
git config --global user.email "your.email@example.com"
```

**Verification:**
```powershell
git --version
```
Expected: `git version 2.43.0` or higher

**Path (default):**
```
C:\Program Files\Git\bin\git.exe
```

---

### 5. Python (Optional, for build scripts)
**Version:** 3.12+

**Installation via winget:**
```powershell
winget install Python.Python.3.12
```

**Verification:**
```powershell
python --version
```
Expected: `Python 3.12.1` or higher

**Path (default):**
```
C:\Users\<USERNAME>\AppData\Local\Programs\Python\Python312\python.exe
```

---

### 6. 7-Zip (for archive extraction)
**Version:** 23.01+

**Installation via winget:**
```powershell
winget install 7zip.7zip
```

**Verification:**
```powershell
& "C:\Program Files\7-Zip\7z.exe" --help
```

**Path (default):**
```
C:\Program Files\7-Zip\7z.exe
```

---

### 7. PowerShell 7 (Recommended)
**Version:** 7.4+

**Installation via winget:**
```powershell
winget install Microsoft.PowerShell
```

**Verification:**
```powershell
pwsh --version
```
Expected: `PowerShell 7.4.1` or higher

**Path (default):**
```
C:\Program Files\PowerShell\7\pwsh.exe
```

---

## PATH Configuration

After installation, verify all tools are in PATH:

```powershell
# Check each tool
$tools = @("cmake", "ninja", "git", "python", "msbuild")
foreach ($tool in $tools) {
    $found = Get-Command $tool -ErrorAction SilentlyContinue
    if ($found) {
        Write-Host "✅ $tool found at: $($found.Source)" -ForegroundColor Green
    } else {
        Write-Host "❌ $tool NOT FOUND" -ForegroundColor Red
    }
}
```

**Manual PATH addition (if needed):**
```powershell
# Run as Administrator
[Environment]::SetEnvironmentVariable("Path", $env:Path + ";C:\Tools\ninja", [System.EnvironmentVariableTarget]::Machine)
```

---

## Tool Versions Lock File

Current verified versions (as of January 8, 2026):

| Tool | Version | Installation Date |
|------|---------|------------------|
| Visual Studio Build Tools | 17.8.3 | 2026-01-05 |
| MSVC Compiler | 19.38.33133 | 2026-01-05 |
| CMake | 3.28.1 | 2026-01-05 |
| Ninja | 1.11.1 | 2026-01-05 |
| Git | 2.43.0 | 2026-01-04 |
| Python | 3.12.1 | 2026-01-04 |
| 7-Zip | 23.01 | 2026-01-04 |
| PowerShell | 7.4.1 | 2026-01-04 |

---

## Removing Older Versions

### Uninstall via winget:
```powershell
winget uninstall <PACKAGE_ID>
```

### Uninstall via Control Panel:
- Apps & Features → Search for tool → Uninstall

### Clean up PATH manually:
```powershell
# View current PATH
$env:Path -split ';'

# Remove specific entry (example)
$newPath = ($env:Path -split ';' | Where-Object { $_ -notlike "*chocolatey*" }) -join ';'
[Environment]::SetEnvironmentVariable("Path", $newPath, [System.EnvironmentVariableTarget]::Machine)
```

---

## Verification Script

Save as `scripts/verify-tools.ps1`:

```powershell
$tools = @{
    "CMake" = "cmake --version"
    "Ninja" = "ninja --version"
    "Git" = "git --version"
    "MSBuild" = "msbuild /version"
    "Python" = "python --version"
    "7-Zip" = '"C:\Program Files\7-Zip\7z.exe" --help'
}

foreach ($tool in $tools.Keys) {
    try {
        $output = Invoke-Expression $tools[$tool] 2>&1
        Write-Host "✅ $tool is available" -ForegroundColor Green
    } catch {
        Write-Host "❌ $tool NOT FOUND" -ForegroundColor Red
    }
}
```

Run:
```powershell
.\scripts\verify-tools.ps1
```

---

## Troubleshooting

### "cl.exe not found"
- Run from Developer Command Prompt, or
- Call `vcvars64.bat` first:
  ```powershell
  & "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
  ```

### "ninja: command not found"
- Add Ninja to PATH (see above)
- Restart terminal after PATH changes

### CMake can't find MSVC
```powershell
cmake -G "Visual Studio 17 2022" -A x64 .
```
If fails, ensure VS Build Tools installed with C++ workload.

---

## Additional Resources

- [Visual Studio Build Tools](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022)
- [CMake Documentation](https://cmake.org/documentation/)
- [Ninja Build](https://ninja-build.org/)
- [winget Documentation](https://learn.microsoft.com/en-us/windows/package-manager/winget/)
