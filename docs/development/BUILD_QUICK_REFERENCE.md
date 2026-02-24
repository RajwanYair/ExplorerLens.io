# ExplorerLens Build Quick Reference
**For AI Assistants & Developers**  
**Updated:** February 9, 2026

---

## 🚀 ONE-LINE SETUP

```powershell
# In ANY PowerShell session:
.\scripts\Setup-DevEnvironment.ps1

# Or add to $PROFILE for automatic loading in every session
```

**Done!** All tools (MSBuild, CMake, CL, NMake, Git) are now available.

---

## 📦 Quick Build Commands

After running setup script above:

```powershell
# Build full solution (Release, x64)
dtbuild Release

# Build Engine only (CMake project)
dtbuild Engine

# Build LENSShell only
dtbuild Shell

# Clean all build outputs
dtbuild Clean

# Full rebuild (Clean + Release)
dtbuild Rebuild

# Run tests
dttest

# Show environment info
Show-ExplorerLensInfo
```

---

## 🔧 Direct Tool Commands (Without Shortcuts)

### MSBuild - Full Solution

```powershell
msbuild LENSShell.sln /p:Configuration=Release /p:Platform=x64 /m /v:minimal
```

### MSBuild - Single Project

```powershell
msbuild LENSShell\LENSShell.vcxproj /p:Configuration=Release /p:Platform=x64 /m
```

### CMake - Engine

```powershell
cd Engine
cmake -B build -G "Visual Studio 18 2026" -A x64
cmake --build build --config Release -j 8
```

### CMake - With Tests

```powershell
cd Engine/build
ctest --output-on-failure
```

---

## 📋 Installed Tools & Versions (Updated Feb 9, 2026)

- **Visual Studio:** 2026 Build Tools (v18)
- **MSVC:** 14.44.35207
- **MSBuild:** 18.3.0
- **CMake:** 4.2.3 ✅ Latest
- **Windows SDK:** 10.0.26100.0
- **Git:** 2.53.0 ✅ Latest
- **Ninja:** 1.13.2 (via Scoop)

---

## 🛠️ Tool Paths (Hard-Coded - No Searching!)

```powershell
# Already configured in Setup-DevEnvironment.ps1:
$Global:ExplorerLensConfig = @{
    VSPath = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools"
    MSBuild = "$VSPath\MSBuild\Current\Bin\amd64\MSBuild.exe"
    CMake = "C:\Users\ryair\scoop\shims\cmake.exe"
    Git = "C:\Users\ryair\scoop\shims\git.exe"
    VCVarsAll = "$VSPath\VC\Auxiliary\Build\vcvarsall.bat"
}
```

---

## ✅ Verify Environment

```powershell
# Check all tools are accessible
Test-BuildTools

# Show full environment details
Show-ExplorerLensInfo

# Test specific tool
cl.exe              # MSVC compiler
nmake /?            # NMake make tool
msbuild -version    # MSBuild
cmake --version     # CMake
git --version       # Git
```

---

## 🏗️ Common Build Scenarios

### Scenario 1: Quick Test Build

```powershell
dtbuild Release        # Fast build with /m (parallel)
```

### Scenario 2: Clean Rebuild

```powershell
dtbuild Rebuild        # Cleans then builds
```

### Scenario 3: Engine Development

```powershell
dtbuild Engine         # Build Engine with CMake
dttest                 # Run Engine tests
```

### Scenario 4: Debug Build

```powershell
msbuild LENSShell.sln /p:Configuration=Debug /p:Platform=x64 /m
```

### Scenario 5: Check for Errors Only

```powershell
msbuild LENSShell.sln /p:Configuration=Release /p:Platform=x64 /m /v:quiet /clp:ErrorsOnly
```

---

## 🐛 Quick Troubleshooting

| Problem | Solution |
|---------|----------|
| "cl.exe not found" | Run `Load-MSVCEnvironment` or `Setup-ExplorerLensEnv -Force` |
| "CMake generator not found" | Ensure VS Build Tools are installed, run `Setup-ExplorerLensEnv` |
| "Build failed" | Check `build.log` or add `/v:detailed` to msbuild command |
| Tools reset after reboot | Add setup script to `$PROFILE` for persistence |

---

## 📁 Project Structure

```
ExplorerLens/
├── LENSShell.sln              # Main VS solution
├── LENSShell/                 # Shell extension DLL
│   └── LENSShell.vcxproj
├── LENSManager/               # Manager application
│   └── LENSManager.vcxproj
├── Engine/                   # CMake-based engine library
│   ├── CMakeLists.txt
│   └── Tests/                # Security tests, benchmarks
├── external/                 # Pre-built dependencies
├── scripts/                  # Build & utility scripts
│   └── Setup-DevEnvironment.ps1  # ⭐ Main setup script
└── .github/                  # Build documentation
    ├── TOOL_VERSIONS.md      # Detailed tool info (this file)
    └── BUILD_QUICK_REFERENCE.txt
```

---

## 🔄 Git Workflow

```powershell
# Check status
git status

# Stage changes
git add .

# Commit
git commit -m "feat: description"

# View recent commits
git log --oneline --graph -10

# View changes
git diff --stat
```

---

## 📊 Build Performance

Typical build times on this machine:

- **LENSShell.dll:** ~60 seconds (incremental), ~120 seconds (clean)
- **Engine library:** ~45 seconds (CMake + compile)
- **Full solution:** ~3 minutes (clean build)

Use `/m` flag for parallel builds to maximize CPU usage.

---

## ⚡ Pro Tips

1. **Add to Profile for Auto-Load:**
   ```powershell
   notepad $PROFILE
   # Add: . "C:\...\ExplorerLens\scripts\Setup-DevEnvironment.ps1"
   ```

2. **Use Aliases:**
   ```powershell
   dtbuild          # Instead of Invoke-ExplorerLensBuild
   dtclean          # Instead of cleaning manually
   dttest           # Instead of navigating to test folder
   ```

3. **Monitor Builds:**
   ```powershell
   # In separate terminal:
   Get-Content build.log -Wait
   ```

4. **Check Build Output:**
   ```powershell
   # List recent DLLs:
   Get-ChildItem -Recurse -Filter "*.dll" | 
       Where LastWriteTime -gt (Get-Date).AddMinutes(-5)
   ```

---

## 📚 Additional Resources

- **Detailed Tool Docs:** `.github/TOOL_VERSIONS.md`
- **Environment Script:** `scripts/Setup-DevEnvironment.ps1`
- **Build Scripts:** `build-scripts/`
- **Project Roadmap:** `MASTER_PLAN.md`
- **Sprint 14 Docs:** `docs/PluginSecurityGuide.md`

---

*Auto-updated: February 9, 2026 - All tools verified working*

