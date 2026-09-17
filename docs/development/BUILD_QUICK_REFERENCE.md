# ExplorerLens Build Quick Reference

## Prerequisites

Use Windows 11 x64, PowerShell 7, Visual Studio 2026 Build Tools with MSVC v145,
ATL, and Windows SDK 10.0.26100.0. Import [the component manifest](https://github.com/RajwanYair/ExplorerLens.io/blob/main/.vsconfig)
through Visual Studio Installer's **More > Import configuration** for the VS 2026
instance. Installing or repairing these machine-wide components requires elevation.

CMake 4.2 or newer is required for the Visual Studio 18 2026 generator. Prefer
current stable releases, not preview compilers. The project uses C++23 and `/MD`.

Install the auxiliary utilities machine-wide. The examples below require an
elevated PowerShell session and keep the executables available to every user:

```powershell
winget install --id Kitware.CMake --exact --scope machine
winget install --id Ninja-build.Ninja --exact --scope machine
winget install --id NASM.NASM --exact --scope machine
winget install --id Git.Git --exact --scope machine
winget install --id 7zip.7zip --exact --scope machine
```

Install Meson, NuGet, sccache, and WiX through machine-wide installers or
machine-wide tool directories under `C:\Program Files` or `C:\ProgramData`.
Do not use user-profile Scoop, `pip --user`, or the user-scoped .NET tool path.

WiX and .NET are needed for MSI packaging. Review the
[WiX licensing and upgrade notes](https://docs.firegiant.com/wix/whatsnew/releasenotes/)
before changing major versions; WiX 7 requires explicit OSMF EULA acceptance.

## Non-Executing Checks

From the repository root, run the VS Code **Verify Tools** task or:

```powershell
.\build-scripts\Test-Build-Environment.ps1
```

This checks prerequisite files and tool availability. It does not configure,
compile, run tests, register the COM extension, or launch ExplorerLens.
Passing does not certify compiled libraries, linking, or runtime correctness.

## Build Commands

These commands compile code. Do not run them during a collateral-only audit.

```powershell
.\build-scripts\Build-MSVC.ps1
.\build-scripts\Build-MSVC.ps1 -Preset default-debug
.\build-scripts\Build-MSVC.ps1 -Preset vs2026
```

The launcher discovers VS 2026 through `vswhere`, selects the newest installed
v145 patch, and initializes the x64 compiler environment before CMake. It resolves
CMake and Ninja from machine PATH, machine-wide roots, or the Visual Studio
installation. Use it rather than configuring
from an uninitialized terminal, where unrelated compilers can be found on PATH.

Local presets use `%TEMP%/ExplorerLens-build` and corresponding `-debug`, `-vs`,
or `-vcpkg` directories. CI presets use the repository's build directory.
Inspect [the presets](https://github.com/RajwanYair/ExplorerLens.io/blob/main/CMakePresets.json) for exact output locations.
The `-Test` switch runs project tests and is not part of a non-executing audit.

## Dependencies and Data

The vcpkg presets require `VCPKG_ROOT` to identify a complete installation and use
`x64-windows-static-md`: static dependency libraries with the dynamic MSVC CRT.
Do not select `x64-windows-static`, which uses a different CRT policy.

The default build still uses versioned, vendored native dependencies. Updating
vcpkg alone does not update these sources or produce their compiled libraries.
Native upgrades must update source provenance, build/link paths, and the SBOM
together, followed by a clean MSVC build and format regression validation.

The [corpus manifest](https://github.com/RajwanYair/ExplorerLens.io/blob/main/data/corpus/MANIFEST.json)
describes validation inputs,
not a guarantee that every sample is present. Missing samples limit format testing
but are not compiler prerequisites. Never replace missing real-format samples with
empty files or rename unrelated data to make an inventory pass.

## Local Audit: 2026-09-17

| Component | Observed state |
| --- | --- |
| CMake | Installed and version-checked: 4.4.3 |
| Ninja | Installed and version-checked: 1.13.2 |
| Meson | Installed and version-checked: 1.12.0 |
| NASM | Installed and version-checked: 3.02 |
| sccache | Installed and version-checked: 0.18.0 |
| NuGet | Installed and version-checked: 7.9.0 |
| VS Code / GitHub CLI | Already current: 1.138.0 / 2.101.0 |
| .NET SDK / VC++ x64 runtime | Present: 10.0.401 / 14.51.36247 |
| Windows SDK | Required headers, libraries, and resource tools present |
| MSVC v145 / ATL | Missing; VS installer metadata does not prove the payload exists |
| WiX | 6.0.2 retained pending WiX 7 license acceptance |
| Vendored compiled libraries | No `.lib` files found under `external/` |
| Corpus files | 16 of 106 manifest paths present; 90 missing |

**Not build-ready.** Compiler installation needs administrator action. Native
dependency updates and compiled artifacts remain outstanding. No project binaries,
tests, shell registration, or installers were executed during this audit.
