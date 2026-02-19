# Sprint 171: Installer Lifecycle Automation

**Block:** v8.3.0 — Phase P5: v8.3.0 Release  
**Status:** ✅ Done  
**Sprint Count:** 171 / 174

---

## Overview

Implements the `InstallerLifecycleAutomation` component — a test harness that simulates fresh
install, upgrade, and uninstall scenarios for `CBXShell.dll`. Validates COM registration
(CLSID `{9E6ECB90-5A61-42BD-B851-D3297D9C7F39}`), registry keys, and thumbnail provider
activation across lifecycle transitions.

---

## Deliverables

| Artifact | Path | Notes |
|---|---|---|
| Header | `Engine/Utils/InstallerLifecycleAutomation.h` | `InstalledVersion`, `COMRegistrationRecord::Expected()`, `InstallerLifecycleAutomation` |
| GTest | `Engine/Tests/Sprint171_InstallerLifecycle.cpp` | 15 test cases |
| Sprint doc | `docs/development/sprints-v8/SPRINT_171.md` | This document |

---

## Tests (15)

- `InstallerLifecycle_COMClsidConstant` — `9E6ECB90-5A61-42BD-B851-D3297D9C7F39`
- `InstallerLifecycle_ExpectedRegistrationRecord` — registry key path correct
- `InstallerLifecycle_InstalledVersionFields` — major/minor/patch/build
- `InstallerLifecycle_VersionStringFormat` — `8.3.0.XXXX` format
- `InstallerLifecycle_FreshInstallSimulation` — SimulateFreshInstall result
- `InstallerLifecycle_UninstallSimulation` — SimulateUninstall result
- `InstallerLifecycle_UpgradeSimulation` — SimulateUpgrade old→new version
- `InstallerLifecycle_RegistrationStepsOrdered` — install steps ordered correctly
- `InstallerLifecycle_UnregistrationStepsOrdered`
- `InstallerLifecycle_DllPathValidation` — DLL path non-empty
- `InstallerLifecycle_RegistryKeyPresent` — expected key documented
- `InstallerLifecycle_RepairScenario` — repair re-registers COM
- `InstallerLifecycle_SilentInstallFlag` — silent mode flag respected
- `InstallerLifecycle_PerUserInstallScope`
- `InstallerLifecycle_PerMachineInstallScope`

---

## Acceptance Criteria

- [x] Header compiles with `/W4` zero warnings
- [x] COM CLSID constant `9E6ECB90-5A61-42BD-B851-D3297D9C7F39` verified
- [x] Fresh install, upgrade, uninstall simulation methods
- [x] All 15 GTest cases pass
- [x] Sprint doc created
