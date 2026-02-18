# DarkThumbs v7.1.0 Release Notes

**Release Date:** February 18, 2026  
**Codename:** Production Hardening  
**Sprints:** 50-74 (25 post-production sprints)  
**Previous Version:** v7.0.0

---

## Overview

DarkThumbs v7.1.0 is a **production hardening** release focused on fixing known gaps,
improving documentation accuracy, strengthening CI/CD pipelines, and preparing the
project for public distribution. No new feature headers were added — instead, this
release ensures that everything built in v7.0.0 (Sprints 1-49) is properly wired,
documented, tested, and ready for deployment.

---

## Highlights

### Known Gaps Fixed (Sprint 50)
- **MSIX CLSID:** The `YOUR-CLSID-HERE` placeholder in `AppxManifest.xml` has been replaced with the actual COM class GUID (`9E6ECB90-5A61-42BD-B851-D3297D9C7F39`)
- **CBXTYPE Enum:** Added missing type defines for ICO, QOI, TGA, BMP, GIF, MODEL, and DOCUMENT formats
- **MSIX File Types:** Expanded from 8 to 17 registered file type associations

### Build System Integration (Sprint 51)
- 40+ header files from Sprints 6-49 are now registered in the CMake `ENGINE_HEADERS` target
- Covers AI, Cloud, Cache, Codec, Memory, Shell, and Release subsystems
- Ensures all headers appear in IDE project views and compile-time validation

### Observability Pipeline (Sprint 52)
- `ObservabilityIntegration.h` — Singleton connecting ETW and structured logger to decode pipeline
- `IObservabilitySink` — Extensible sink interface for custom telemetry backends
- `PipelineEvent` — Structured event model with request ID, timing, cache status, decoder name
- Privacy modes: Hashed (default), Full (verbose), Redacted (enterprise)
- Request lifecycle counters for diagnostics dashboard

### Documentation Accuracy (Sprints 54-61)
- Fixed stale version references across 6+ documentation files (v5.x/v6.x → v7.0.0/v7.1.0)
- Updated CHANGELOG, README, DEVELOPER_GUIDE, USER_GUIDE, KNOWN_ISSUES, QUICK_BUILD_REFERENCE
- Release notes published for v7.1.0

### CI/CD & Quality (Sprints 62-63)
- Enhanced GitHub Actions workflows with dependency caching and artifact publishing
- Added static analysis and code quality gates
- Matrix builds for x64 and ARM64 configurations

### Community & Governance (Sprints 64-65, 70-71)
- Updated CONTRIBUTING.md with current development workflow
- Enhanced PR template with checklist and review requirements
- Updated SECURITY.md with vulnerability disclosure process
- AI assistant guide (COPILOT_INSTRUCTIONS.md) for consistent development
- Modernized issue templates (bug report, feature request, format request)

### Documentation Infrastructure (Sprints 67-69, 73)
- Plugin SDK docs updated to v7.1
- Build script deprecation enforced with migration paths
- Comprehensive project standards documentation
- docs/INDEX.md rebuilt with cross-reference validation

---

## Build Metrics

| Metric | v7.0.0 | v7.1.0 |
|--------|--------|--------|
| CBXShell.dll | 2,940 KB | 2,940 KB |
| CBXManager.exe | 400 KB | 400 KB |
| DarkThumbsEngine.lib | 133 MB | 133 MB |
| Compiler warnings | 0 | 0 |
| Unit tests | 100 | 100+ |
| Benchmarks | 5 | 5 |
| Total sprints | 49 | 74 |
| Documentation files updated | — | 20+ |
| Stale version refs fixed | — | 12 |

---

## Breaking Changes

None. v7.1.0 is fully backward-compatible with v7.0.0.

---

## Upgrade Path

### From v7.0.0
No action required — v7.1.0 is a drop-in replacement.

### From v6.x
Follow the standard upgrade path documented in the [Upgrade Testing Guide](docs/testing/UPGRADE_TESTING_GUIDE_V7.md).

---

## Known Issues

See [KNOWN_ISSUES.md](KNOWN_ISSUES.md) for the current list.

---

## Contributors

DarkThumbs Project Team

---

## Full Sprint List (v7.1.0)

| Sprint | Name | Status |
|--------|------|--------|
| 50 | Fix Known Gaps (CLSID, CBXTYPE, MSIX) | ✅ |
| 51 | CMake Header Registration | ✅ |
| 52 | Observability Pipeline Wiring | ✅ |
| 53 | Build Validation & Test Expansion | ✅ |
| 54 | Documentation Version Audit | ✅ |
| 55 | CHANGELOG v7.1 | ✅ |
| 56 | Release Notes v7.1.0 | ✅ |
| 57 | README.md Update | ✅ |
| 58 | DEVELOPER_GUIDE.md Update | ✅ |
| 59 | USER_GUIDE.md Update | ✅ |
| 60 | KNOWN_ISSUES.md Audit | ✅ |
| 61 | QUICK_BUILD_REFERENCE.md Update | ✅ |
| 62 | CI/CD Workflow Hardening | ✅ |
| 63 | Code Quality Workflow | ✅ |
| 64 | CONTRIBUTING.md & PR Template | ✅ |
| 65 | Security Policy Update | ✅ |
| 66 | Performance Report Update | ✅ |
| 67 | Plugin SDK Documentation | ✅ |
| 68 | Build Script Cleanup | ✅ |
| 69 | .github/standards Update | ✅ |
| 70 | COPILOT_INSTRUCTIONS.md | ✅ |
| 71 | Issue Templates Modernization | ✅ |
| 72 | Integration Test Matrix | ✅ |
| 73 | docs/INDEX.md Rebuild | ✅ |
| 74 | v7.1 Release Preparation | ✅ |
