# Sprint 145 — Version Drift CI Integration

**Status:** Complete  
**Phase:** N5 — CI & Release Hardening  
**Component:** `Engine/Core/VersionDriftDetector.h`

## Objective
Automated CI gate that scans docs, headers, build scripts, and configs for stale version strings and flags inconsistencies against the canonical version.

## Deliverables
- `Engine/Core/VersionDriftDetector.h` — Header-only version drift detector
- `tests/Sprint145_VersionDriftDetector.cpp` — 15 GTest cases

## Key Features
- **SemanticVersion** — Parse, compare, and format semver strings with pre-release and build metadata
- **DriftSeverity** — Four-tier classification: Info, Warning, Error, Critical
- **ArtifactKind** — Seven artifact types: Header, Documentation, BuildScript, Config, Installer, ReleaseNote, TestFixture
- **DriftScanPolicy** — Configurable canonical version, patch drift tolerance, include/exclude patterns
- **DriftScanResult** — Aggregated counts, drift score (0-100), clean/fail status
- **VersionDriftDetector** — Content-based regex scanning with severity classification and CI report formatting
- **FormatReport** — CI-friendly text report with file:line detail and pass/fail verdict

## Design Rationale
Addresses the recurring version-drift problem documented in MASTER_PLAN.md Section 2B where 12+ files had stale version references. Automated detection prevents regression.
