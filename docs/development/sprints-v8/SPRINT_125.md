# Sprint 125 — Automated Version Drift Gate

- **Phase:** N1 (Carry-Over Resolution & Truth Alignment)
- **Date:** 2026-02-18
- **Status:** Implemented

## Objective
Create automated version drift detection and enforcement infrastructure to eliminate stale version references in docs/code and provide CI-enforceable gate policies.

## Scope Areas
- `Engine/Core/VersionDriftGate.h` — Semantic version parsing, drift classification, gate policies
- `tests/Sprint125_VersionDriftGate.cpp` — 25 GTest cases

## Deliverables
1. `SemanticVersion` struct with parse, compare, format, pre-release/build-meta support.
2. `DriftSeverity` classification: None/Minor/Moderate/Major/Critical.
3. `VersionDriftGate` engine: source registration, reference tracking, drift validation.
4. `GatePolicy` enforcement: max severity threshold, compliance %, exempt file list.
5. `GatePolicies` presets: Strict (100% compliance), CI (95%, no major), Permissive (80%).
6. `DriftReport` output: compliance %, severity text, violation list, timestamp.

## Acceptance Criteria
- Semantic version parsing handles major.minor.patch, pre-release, and build metadata.
- Drift classification correctly identifies None/Minor/Moderate/Major/Critical severity.
- Gate check passes when all sources match canonical version.
- Gate check fails when major drift detected under CI policy.
- Compliance percentage computed correctly with edge cases (zero refs).
- 25 test cases passing.

## Key Design Decisions
- Version comparison treats pre-release as less than release (semver spec).
- Stale patterns auto-initialized for v5.x (critical), v6.x (major), v7.0.x (moderate).
- Gate policies are composable — exempt files allow version-archaeology files to pass.

## Files Created/Modified
- `Engine/Core/VersionDriftGate.h` (NEW)
- `tests/Sprint125_VersionDriftGate.cpp` (NEW)
- `Engine/CMakeLists.txt` (MODIFIED — header registration)
