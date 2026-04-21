# ExplorerLens — Quick Start Guide

> **Version:** 37.2.0 "Antares"  
> **Target:** Windows 10/11 x64, Visual Studio 18 2026 (MSVC v145)

---

## 1. Prerequisites

| Tool | Version | Install |
|------|---------|---------|
| Visual Studio 18 2026 BuildTools | v145 (cl.exe 19.50) | [visualstudio.microsoft.com](https://visualstudio.microsoft.com/downloads/) |
| CMake | 4.3+ | `scoop install cmake` |
| Ninja | 1.13+ | `scoop install ninja` |
| Git | 2.40+ | `scoop install git` |
| Windows SDK | 10.0.26100+ | Installed with VS BuildTools |
| (optional) sccache | any | `scoop install sccache` |

---

## 2. Clone

```powershell
git clone https://github.com/RajwanYair/ExplorerLens.git
cd ExplorerLens
```

---

## 3. Build External Libraries (first time only)

External C libraries live in `external/` and must be built before the Engine:

```powershell
# Build all required libs (~10-20 min on first run)
.\build-scripts\Update-All-Libraries.ps1

# Or build individually:
.\build-scripts\external-libs\Build-Zlib.ps1      -Clean
.\build-scripts\external-libs\Build-LZ4.ps1       -Clean
.\build-scripts\external-libs\Build-Zstd.ps1      -Clean
.\build-scripts\external-libs\Build-LibWebP-NMake.ps1 -Clean
.\build-scripts\external-libs\Build-LibRaw.ps1    -Clean
.\build-scripts\external-libs\Build-Dav1d.ps1     -Clean
.\build-scripts\external-libs\Build-LibHEIF.ps1   -Clean
```

---

## 4. Build the Engine

```powershell
# Recommended: one-command build (handles vcvars + cmake + ninja)
.\build-scripts\Build-MSVC.ps1

# Clean rebuild:
.\build-scripts\Build-MSVC.ps1 -Clean

# Build + run tests:
.\build-scripts\Build-MSVC.ps1 -Clean -Test
```

Output: `build\lib\ExplorerLensEngine.lib`

---

## 5. Build the Shell Extension + Manager (MSBuild)

```powershell
msbuild LENSShell.sln /p:Configuration=Release /p:Platform=x64 /m /v:minimal
```

Output: `x64\Release\LENSShell.dll`, `x64\Release\LENSManager.exe`

---

## 6. Register the Shell Extension

```powershell
# Requires elevated prompt
regsvr32 "x64\Release\LENSShell.dll"

# To unregister:
regsvr32 /u "x64\Release\LENSShell.dll"
```

After registration, restart Windows Explorer (`taskkill /f /im explorer.exe && explorer.exe`) or simply browse to an image folder to verify thumbnails appear.

---

## 7. Run Tests

```powershell
# CTest (CTest-driven, all test suites)
ctest --test-dir build -C Release --output-on-failure -j 4

# Run EngineTests directly:
.\build\bin\EngineTests.exe

# Run Catch2 tests:
.\build\bin\EngineCatch2Tests.exe --reporter compact

# Validate test corpus:
.\tools\Validate-Corpus.ps1 -Verbose
```

Expected: `~4724 tests passed, 0 failed`.

---

## 8. CMake Presets

| Preset | Description |
|--------|-------------|
| `default-release` | Ninja + MSVC v145, local `external/` libs |
| `default-debug` | Debug build with /Z7 info |
| `vcpkg-release` | Use vcpkg for all dependencies |
| `vs2026` | Visual Studio 18 solution generator |

```powershell
# Manual preset workflow (vcvars must be sourced first):
cmd /c '"C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat" && powershell'
cmake --preset default-release
cmake --build --preset default-release -j 8
```

---

## 9. Useful Build Options

```powershell
# Enable sccache for faster rebuilds (auto-detects if installed)
cmake --preset default-release -DUSE_SCCACHE=ON

# Enable unity (jumbo) build for faster cold builds
cmake --preset default-release -DENABLE_UNITY_BUILD=ON

# Catch2 tests only (no custom test framework)
cmake --preset default-release -DBUILD_CATCH2_TESTS=ON -DBUILD_TESTS=OFF
```

---

## 10. Project Layout

```
ExplorerLens/
├── Engine/               Core decode + GPU pipeline (C++20 static lib)
│   ├── Core/             Decode pipeline, GPU renderer, resource management
│   ├── Decoders/         25+ format-specific decoders
│   ├── Cache/            Two-tier cache (L1 LRU + L2 disk)
│   ├── GPU/shaders/      HLSL compute shaders (resize, tonemap, demosaic)
│   ├── AI/               Scene understanding, smart crop, IQA
│   └── Tests/            Custom test harness + Catch2 suite
├── LENSShell/            COM IThumbnailProvider DLL
├── LENSManager/          WTL configuration GUI
├── data/corpus/          Test corpus (synthetic + real format samples)
├── tools/                Utility scripts (Validate-Corpus.ps1, etc.)
├── docs/                 Architecture, API, performance, release docs
├── packaging/            MSI (WiX), Scoop, Chocolatey, winget manifests
└── build-scripts/        PowerShell build automation
```

---

## 11. Common Issues

| Symptom | Fix |
|---------|-----|
| CMake picks Clang instead of MSVC | Source `vcvars64.bat` first, or use `Build-MSVC.ps1` |
| `LNK2019` unresolved external | Rebuild external libs with matching `/MD` CRT |
| Thumbnails don't appear after install | Run `regsvr32 LENSShell.dll` in elevated prompt |
| Build warnings about `/Zi` + sccache | Expected — `/Zi` is replaced with `/Z7` automatically |
| `EngineTests.exe` hangs at startup | Anti-virus scanning the new DLL; add exception for `build\bin\` |

---

## 12. Contributing

1. Fork → create a feature branch
2. Follow conventions in `.github/standards/coding-standards.md`
3. Run `.\build-scripts\Build-MSVC.ps1 -Clean -Test` — must pass with 0 errors, 0 warnings
4. Run `.\tools\Validate-Corpus.ps1` — all corpus entries must validate
5. Open a PR — binary-size gate + all CI checks must pass

See [ROADMAP.md](../ROADMAP.md) for planned features and open issues on GitHub for contribution opportunities.
