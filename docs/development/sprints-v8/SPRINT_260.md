# Sprint 260: 3MF/USD Format Support

**Date:** 2026-02-20
**Version:** v11.0.0
**Phase:** Phase 2 — New Format Decoders

## Objective
3MF decoder (ZIP+XML+mesh) for 3D printing files and USD/USDZ format evaluation. Unified 3D model format detection for 9 extensions across 7 format types.

## Deliverables
- `Engine/Decoders/ModelFormatHandler.h` — 3MF/USD format handler
- Model3DFormat enum (7 types: ThreeMF, USD, USDA, USDC, USDZ, STEP, IGES)
- ThreeMFRelation enum for content type relationships
- Is3MFFile() ZIP magic detection for 3MF archives
- CanExtractThumbnail() for formats with embedded thumbnails (3MF, USDZ)
- 5 unit tests

## Test Results
- TestModelFmt_Detect3MF ✅
- TestModelFmt_FormatNames ✅
- TestModelFmt_Is3MF ✅
- TestModelFmt_Thumbnail ✅
- TestModelFmt_Counts ✅
