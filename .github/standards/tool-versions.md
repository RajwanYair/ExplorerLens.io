# Tool Versions & Upgrade Matrix

**Last Updated:** 24 May 2026 (v40.0.1 Procyon release)
**Version:** 40.0.1 "Procyon"

---

## Build Toolchain

| Tool | Current | Latest Stable | Status | Upgrade Path |
| ------ | --------- | --------------- | -------- | -------------- |
| **MSVC (cl.exe)** | 19.50.35720 (v145) | 19.50.35720 | ✅ Current | VS 18 2026 BuildTools |
| **MSBuild** | 18.3 | 18.3 | ✅ Current | Bundled with VS 18 |
| **CMake** | 4.4.3 | 4.4.3 | ✅ Current | `winget install --id Kitware.CMake --exact --scope machine` |
| **Ninja** | 1.13.2 | 1.13.2 | ✅ Current | `winget install --id Ninja-build.Ninja --exact --scope machine` |
| **Windows SDK** | 10.0.26100.0 | 10.0.26100.0 | ✅ Current | VS Installer |
| **vcpkg** | 2026-02-21 | 2026-02-21 | ✅ Current | Bundled with VS 18 |

## Developer Tools

| Tool | Current | Latest Stable | Status | Install |
| ------ | --------- | --------------- | -------- | --------- |
| **Git** | 2.53.0.2 | 2.53.0.2 | ✅ Current | `winget install --id Git.Git --exact --scope machine` |
| **LLVM / Clang** | 22.1.1 | 22.1.1 | ✅ Current | Machine-wide vendor installer |
| **MinGW** | 15.2.0-rt_v13-rev1 | 15.2.0-rt_v13-rev1 | ✅ Current | Machine-wide vendor installer |
| **NASM** | 3.02 | 3.02 | ✅ Current | Machine-wide vendor installer |
| **Meson** | 1.12.0 | 1.12.0 | ✅ Current | Machine-wide vendor installer |
| **NuGet** | 7.9.0 | 7.9.0 | ✅ Current | Machine-wide vendor installer |
| **7-Zip** | 26.00 | 26.00 | ✅ Current | Machine-wide vendor installer |
| **WiX** | 6.0.2 | 6.0.2 | ✅ Current | Machine-wide installer under `C:\Program Files` or `C:\ProgramData` |
| **cppcheck** | 2.20.0 | 2.20.0 | ✅ Current | Machine-wide vendor installer |
| **graphviz** | 14.1.4 | 14.1.4 | ✅ Current | Machine-wide vendor installer |
| **delta** | 0.19.2 | 0.19.2 | ✅ Current | Machine-wide vendor installer |
| **fd** | 10.4.2 | 10.4.2 | ✅ Current | Machine-wide vendor installer |
| **innounp** | 2.67.6 | 2.67.6 | ✅ Current | Machine-wide vendor installer |
| **PowerShell** | 7.6.0 | 7.6.0 | ✅ Current | `winget upgrade PowerShell` |
| **sccache** | 0.18.0 | 0.18.0 | ✅ Current | `winget install --id Mozilla.sccache --exact --scope machine` |

## AI Tooling & Automation Surface

| Surface | Current Standard | Status | Notes |
| -------- | ------------------ | -------- | ------- |
| GitHub Actions JS runtime | Node 24 opt-in (`FORCE_JAVASCRIPT_ACTIONS_TO_NODE24: true`) | ✅ Current | Avoids Node 20 deprecation noise in workflows |
| Copilot repository instructions | `.github/copilot-instructions.md` + `.github/instructions/*.instructions.md` | ✅ Current | Canonical rules plus scoped instruction files |
| Custom agent definitions | `.github/agents/*.agent.md` | ✅ Current | ExplorerLens repo-specific delegated agent |
| Prompt templates | `.github/prompts/*.prompt.md` | ✅ Current | Review, test, project creation, and quality prompts |
| Repository-local skills | `.github/skills/*/SKILL.md` | ✅ Current | Build/release and workflow/MCP capability playbooks |
| MCP server configuration | `.vscode/mcp.json` | ✅ Current | GitHub + filesystem + docs-scoped server inventory |
| AI tooling capability reference | `.github/standards/ai-tooling-capabilities.md` | ✅ Current | Canonical inventory and maintenance contract |

## External Libraries (Statically Linked)

### Compression Libraries

| Library | Current | Latest Stable | Status | Notes |
| --------- | --------- | --------------- | -------- | ------- |
| **zlib** | 1.3.1 | 1.3.1 | ✅ Current | Stable, no recent releases |
| **LZ4** | 1.10.0 | 1.10.0 | ✅ Current | |
| **zstd** | 1.5.7 | 1.5.7 | ✅ Current | Facebook/Meta maintained |
| **LZMA SDK** | 26.00 | 26.00 | ✅ Current | Igor Pavlov, stable |
| **minizip-ng** | 4.0.10 | 4.0.10 | ✅ Current | |
| **UnRAR** | 7.2.2 | 7.2.2 | ✅ Current | |
| **bzip2** | 1.0.8 | 1.0.8 | ✅ Current | Very stable |
| **libarchive** | 3.7.6 | 3.7.6 | ✅ Current | |

### Image Libraries

| Library | Current | Latest Stable | Status | Notes |
| --------- | --------- | --------------- | -------- | ------- |
| **libwebp** | 1.5.0 | 1.5.0 | ✅ Current | Google maintained |
| **libavif** | 1.3.0 | 1.3.0 | ✅ Current | AOM/Google maintained |
| **dav1d** | 1.5.1 | 1.5.1 | ✅ Current | VideoLAN AV1 decoder |
| **libjxl** | 0.11.1 | 0.11.1 | ✅ Current | JPEG XL reference impl |
| **libheif** | 1.19.5 | 1.19.5 | ✅ Current | HEIF/HEIC container |
| **libde265** | 1.0.15 | 1.0.15 | ✅ Current | H.265/HEVC decoder |
| **LibRaw** | 0.21.3 | 0.21.3 | ✅ Current | RAW camera formats |

### Document/PDF Libraries

| Library | Current | Latest Stable | Status | Notes |
| --------- | --------- | --------------- | -------- | ------- |
| **MuPDF** | 1.24.11 | 1.24.11 | ✅ Current | Verified ABI compatible |
| **FreeType** | bundled | 2.13.3 | ✅ Current | Via MuPDF |
| **OpenJPEG** | bundled | 2.5.3 | ✅ Current | Via MuPDF |

## CRT Linkage Policy

| Target | CRT Mode | Notes |
| -------- | ---------- | ------- |
| **ExplorerLensEngine** | `/MD` (MultiThreadedDLL) | Standard |
| **LENSShell.dll** | `/MD` (MultiThreadedDLL) | COM DLL |
| **LENSManager.exe** | `/MD` (MultiThreadedDLL) | GUI app |
| **EngineTests.exe** | `/MD` (MultiThreadedDLL) | Test runner |
| **libwebp** | `/MD` (MultiThreadedDLL) | Rebuilt with `/MD` — no workaround needed |

> All external libraries including libwebp have been rebuilt with `/MD`. No `/NODEFAULTLIB:LIBCMT` workarounds are needed.

## Upgrade Procedure

### For machine-wide tools

> `packaging/scoop/scoopfile.json` is retained as legacy distribution metadata and is
> not a supported local setup path. Install or update local tools at machine scope.

```powershell
# Install or update core tools at machine scope
winget install --id Kitware.CMake --exact --scope machine
winget install --id Ninja-build.Ninja --exact --scope machine
winget install --id Git.Git --exact --scope machine
winget install --id Mozilla.sccache --exact --scope machine

# Use the vendor installer for tools without a verified winget package ID.
# Run elevated when the installer writes under C:\Program Files or C:\ProgramData.
```

### For external libraries

1. Download new version source
2. Update version in build script filename (e.g., `Build-Zlib.ps1`)
3. Update version in `Build-Library-Core.ps1` if needed
4. Clean build: `.\build-scripts\external-libs\Build-<Library>.ps1 -Clean`
5. Update this file with new version
6. Update `copilot-instructions.md` version table
7. Run full build + test: `.\build-scripts\Build-MSVC.ps1 -Clean -Test`

### For MSVC toolchain

1. Run VS Installer → Update VS 18 BuildTools
2. Update `copilot-instructions.md` with new cl.exe version
3. Update `build-method.md` tool paths
4. Update `c_cpp_properties.json` compiler path
5. Clean build and verify zero warnings

## Version Checking Script

```powershell
# Quick version check for all tools
.\build-scripts\Find-All-Tools.ps1

# Or manually:
cmake --version
ninja --version
cl.exe 2>&1 | Select-String "Version"
git --version
```

---

## Compatibility Notes

- **Windows SDK 10.0.26100.0** — Targets Windows 10 1809+ and Windows 11
- **C++20** — Requires MSVC 19.29+ (v142+), we use v145
- **AVX2** — Required for SIMD acceleration; runtime fallback exists for SSE4.2
- **DirectX 11** — Primary GPU renderer, available on all supported Windows versions
- **DirectX 12** — Used for compute pipeline, requires Windows 10 1607+
- **Vulkan** — Optional compute pipeline, requires Vulkan-capable GPU + drivers
