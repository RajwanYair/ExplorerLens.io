# Sprint 261: Release Gate V17

**Date:** 2026-02-20
**Version:** v11.0.0
**Phase:** Phase 2 — New Format Decoders

## Objective
Release Gate V17 validates v11.0.0 format decoder additions. 21 KPIs covering build quality, new decoder validation (DPX, APNG, TextPreview, DICOM V2, FITS V2, ModelFormat), documentation, and performance.

## Deliverables
- `Engine/Utils/ReleaseGateV17.h` — 21-KPI release gate evaluator
- GateV17KPI enum with decoder-specific KPIs (DPXDecoderValid, APNGHandlerValid, etc.)
- GateV17Verdict with approved/passed/failed counts
- 5 unit tests

## Test Results
- TestGateV17_KPINames ✅
- TestGateV17_KPICount ✅
- TestGateV17_Evaluate ✅
- TestGateV17_Approved ✅
- TestGateV17_Version ✅
