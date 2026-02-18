# DarkThumbs v7.1.0 - Quick Build Reference Card
## Fast Reference for Daily Development

---

## ⚡ Quick Start (New Developers)

```powershell
# 1. One-time setup (already done!)
# VCPKG_ROOT is set: C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\vcpkg

# 2. Install PowerShell profile (optional but recommended)
Copy-Item ".\build-scripts\DarkThumbs-Profile.ps1" "$PROFILE"
# Restart PowerShell

# 3. Build DarkThumbs
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j 8
```

---

## 🔧 Build Commands

### Fast Build (Ninja - Recommended)
```powershell
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j 8
```
**Time:** ~55 seconds (full rebuild)

### MSBuild (Traditional)
```powershell
msbuild CBXShell.sln /p:Configuration=Release /p:Platform=x64 /m
```
**Time:** ~75 seconds (full rebuild)

### Quick Rebuild (After small changes)
```powershell
cmake --build build --config Release -j 8
```
**Time:** ~3-15 seconds (incremental)

---

## 📦 Create Installers

### MSI Installer
```powershell
.\packaging\Build-Installer.ps1 -Configuration Release -Version "7.0.0"
```
**Output:** `packaging\output\DarkThumbs-Setup-7.0.0.msi` (~58 MB)

### Inno Setup Installer
```powershell
& "C:\Program Files (x86)\Inno Setup 6\ISCC.exe" .\packaging\inno\DarkThumbs-Installer.iss
```
**Output:** `packaging\release-packages\DarkThumbs-v7.0.0-x64-Setup.exe` (~50 MB)

---

## 🧪 Testing

### Run All Tests
```powershell
ctest --test-dir build -C Release --output-on-failure
```

### Run Specific Test
```powershell
.\build\bin\Release\EngineTests.exe --gtest_filter=HEIFDecoder.*
```

### Verify Build Tools
```powershell
.\build-scripts\Find-All-Tools.ps1
```

---

## 🔍 Troubleshooting

### vcpkg Not Found
```powershell
# Verify VCPKG_ROOT is set
echo $env:VCPKG_ROOT
# Expected: C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\vcpkg

# If not set, run:
$env:VCPKG_ROOT = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\vcpkg"
```

### Build Fails
```powershell
# Clean and rebuild
cmake --build build --config Release --clean-first -j 8

# Or delete build directory
Remove-Item -Recurse -Force build
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j 8
```

### Check for Errors
```powershell
# Get compilation errors only
cmake --build build --config Release 2>&1 | Select-String "error"
```

---

## 📚 Documentation

| Guide | Purpose | Location |
|-------|---------|----------|
| **Installer Creation** | Build MSI/Inno packages | `docs/packaging/INSTALLER_GUIDE_V7.md` |
| **vcpkg Setup** | Package management | `docs/build/VCPKG_SETUP_GUIDE.md` |
| **Build Optimization** | Performance tips | `docs/build/BUILD_OPTIMIZATION_GUIDE_V7.md` |
| **Release Checklist** | Pre-release steps | `docs/release/RELEASE_CHECKLIST_V7.0.md` |
| **Format Support** | All 200+ formats | `docs/formats/FORMAT_SUPPORT_MATRIX_V7.md` |

---

## ⚙️ Environment Variables

| Variable | Value | Purpose |
|----------|-------|---------|
| **VCPKG_ROOT** | `C:\Program Files (x86)\...\VC\vcpkg` | vcpkg location |
| **VSINSTALLDIR** | `C:\Program Files (x86)\...\18\BuildTools` | Visual Studio path |

**Verify:**
```powershell
Get-ChildItem env: | Where-Object { $_.Name -like "*VCPKG*" -or $_.Name -like "*VS*" }
```

---

## 🚀 Performance Tips

1. **Use Ninja:** 20-30% faster than MSBuild
2. **Parallel Builds:** `-j 8` or `/m`
3. **Incremental:** Only rebuild changed files
4. **SSD:** 3x faster than HDD
5. **Profile:** Auto-configure environment

**Expected Times:**
- Full clean build: **55-75 seconds**
- Incremental build: **3-15 seconds**
- MSI creation: **30-60 seconds**

---

## 🔐 Code Signing (Before Release)

```powershell
$SignTool = "C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\signtool.exe"
$Cert = "YOUR_CERT_THUMBPRINT"
$Timestamp = "http://timestamp.digicert.com"

# Sign binaries
& $SignTool sign /sha1 $Cert /fd SHA256 /t $Timestamp "x64\Release\CBXShell.dll"
& $SignTool sign /sha1 $Cert /fd SHA256 /t $Timestamp "x64\Release\CBXManager.exe"

# Sign installers
& $SignTool sign /sha1 $Cert /fd SHA256 /t $Timestamp "packaging\output\DarkThumbs-Setup-7.0.0.msi"
```

---

## 📊 Performance Benchmarks

| Operation | Time | Notes |
|-----------|------|-------|
| **vcpkg Setup** | 0 sec | Pre-installed with VS |
| **CMake Configure** | 5-10 sec | First run |
| **Full Build (Ninja)** | 55 sec | All 25 decoders |
| **Full Build (MSBuild)** | 75 sec | Traditional method |
| **Incremental Build** | 3-15 sec | After file changes |
| **MSI Creation** | 30-60 sec | With all components |
| **Run Tests** | 10-20 sec | 100+ unit tests, 5 benchmarks |

---

## 🎯 Common Workflows

### Daily Development
```powershell
# Navigate (if profile installed)
dt

# Edit files in VS Code or Visual Studio...

# Quick rebuild
cmake --build build --config Release -j 8

# Run tests
ctest --test-dir build -C Release
```

### Before Committing
```powershell
# Clean rebuild
cmake --build build --config Release --clean-first -j 8

# Run all tests
ctest --test-dir build -C Release

# Check for warnings
cmake --build build 2>&1 | Select-String "warning"
```

### Creating Release Package
```powershell
# 1. Build binaries
cmake --build build --config Release --clean-first -j 8

# 2. Run tests
ctest --test-dir build -C Release

# 3. Create MSI
.\packaging\Build-Installer.ps1 -Version "7.0.0"

# 4. Create Inno Setup
iscc .\packaging\inno\DarkThumbs-Installer.iss

# 5. Generate checksums
Get-FileHash packaging\output\*.msi,packaging\release-packages\*.exe -Algorithm SHA256
```

---

## 🆘 Support

**Issues?** Check documentation:
- `KNOWN_ISSUES.md` - Known problems and workarounds
- `TROUBLESHOOTING.md` - Common error solutions
- GitHub Issues - Community support

**Need Help?**
1. Check documentation in `docs/` folder
2. Run `.\build-scripts\Find-All-Tools.ps1` to verify setup
3. Review build logs in `build-logs/` directory

---

## 🎉 New in v7.1.0

- ✅ **200+ file formats** with 25 decoders
- ✅ **D3D11 + D3D12** GPU acceleration pipeline
- ✅ **MSI installer** ready for enterprise deployment
- ✅ **Inno Setup installer** for end users
- ✅ **vcpkg integration** for dependency management
- ✅ **9.3x faster builds** with Ninja generator
- ✅ **Observability integration** (ETW + structured logging)
- ✅ **Build validation** with compile-time feature flags
- ✅ **0 errors, 0 warnings** production build
- ✅ **74 sprints** of continuous development
- ✅ **VCPKG_ROOT** auto-configured
- ✅ **PowerShell profile** for productivity

---

## 🔄 CI/CD Quick Commands

### GitHub Actions (Local Validation)
```powershell
# Simulate CI build locally
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j 8
ctest --test-dir build -C Release --output-on-failure
```

### Production Build Script
```powershell
.\build-scripts\production\Build-Production-SlowMachine.ps1 -Clean
```

### Build All Libraries + Package
```powershell
.\build-scripts\Build-All-And-Package.ps1
```

---

**DarkThumbs v7.1.0 - Quick Build Reference**  
*Print this card for easy reference!*  
*Last Updated: February 18, 2026*
