# ExplorerLens v16.0.0 "Horizon" — Feature Freeze Document

**Status:** FROZEN as of sprint 71  
**Target release:** v16.0.0 "Horizon"  
**Branch policy:** Only bug-fix and documentation PRs accepted in the Horizon stabilisation window.

---

## Scope

### Features Included in v16.0.0

| Sprint | Feature | Status |
|--------|---------|--------|
| 41–52  | WinUI 3 LENSManager (9 pages) | ✅ Complete |
| 53     | TrayAgent (system tray icon) | ✅ Complete |
| 54     | ToastNotifier (WinRT bridge) | ✅ Complete |
| 55     | Onboarding Wizard | ✅ Complete |
| 58     | i18n EN/FR/DE scaffold | ✅ Complete |
| 59     | AnonymousTelemetry module | ✅ Complete |
| 61     | AccessibilityAudit (UIA/WCAG 2.1) | ✅ Complete |
| 63     | RegressionTestSuite (full matrix) | ✅ Complete |
| 64     | FeatureCompatMatrix (format × OS × GPU) | ✅ Complete |
| 65     | StoreReadinessChecker | ✅ Complete |
| 66     | WinUI 3 MSBuild project (.vcxproj) | ✅ Complete |
| 67     | MigrationGuide v15→v16 | ✅ Complete |
| 68     | Public API Surface freeze doc | ✅ Complete |

### Explicitly Excluded (deferred to v16.1.0+)

- Cloud provider integration (OneDrive, SharePoint, S3) — Sprint 73
- AI thumbnail upscaling (ONNX/DLSS/XeSS) — Sprint 82
- Intune / SCCM enterprise policy — Sprint 90
- Windows Store certification — Sprint 97

---

## Quality Gates for v16.0.0 Release

All of the following MUST pass before the v16.0.0 tag is created:

- [ ] 100% unit test pass rate (EngineTests.exe, all ~3200 tests)
- [ ] Zero new compiler warnings (MSVC cl.exe 19.50, `/W4`)
- [ ] AccessibilityAudit: zero Critical or Error findings
- [ ] RegressionTestSuite: zero regressions vs v15.8.0 baseline
- [ ] FeatureCompatMatrix: PASS on Win10 22H2 + Win11 23H2
- [ ] StoreReadinessChecker: all MSIX manifest rules passing
- [ ] SBOM generated and verified
- [ ] CHANGELOG.md updated with full [16.0.0] section
- [ ] SHA256SUMS.txt verified against build outputs
- [ ] GitHub Release draft approved

---

## Stabilisation Window Checklist

- [ ] All deferred features removed from active development branch
- [ ] Documentation updated to reflect final API surface
- [ ] MigrationGuide reviewed for accuracy
- [ ] Performance baselines re-measured and documented in PERFORMANCE.md
- [ ] Packaging (MSI + MSIX + ZIP) tested on clean VM
- [ ] Code signing: BOTH LENSShell.dll and LENSManager.exe signed

---

## Version Numbers at Freeze

| File | Value |
|------|-------|
| VERSION | 15.8.0 (next release: 16.0.0) |
| BuildValidation.h MajorVersion | 15 (next: 16) |
| Engine.h EXPLORERLENS_VERSION | "15.8.0" |
| CHANGELOG.md latest | [15.8.0] "Zenith-Y" |

---

*This document is maintained by the ExplorerLens project automation.*  
*Last updated by Bump-Version.ps1 during Sprint 71 execution.*
