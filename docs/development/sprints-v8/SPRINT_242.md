# Sprint 242 — MetadataExtractor

**Date:** 2026-06-15
**Component:** `Engine/Core/MetadataExtractor.h`, `Engine/Core/MetadataExtractor.cpp`
**Theme:** EXIF/IPTC/XMP Metadata Extraction

## Summary
Implemented a metadata extraction engine supporting 5 standards (EXIF, IPTC, XMP, ICC, GPS) and 16 common fields (Title through ColorSpace). Provides per-standard filtering, field lookup, and formatting utilities for GPS coordinates and exposure times.

## Key Types
- `MetadataStandard` — EXIF, IPTC, XMP, ICC, GPS (5 standards)
- `MetadataField` — 16 fields (Title, Author, Copyright, dates, camera info, GPS, dimensions, color)
- `MetadataTag` — standard, field, name, value, raw tagId
- `ExtractionResult` — success, tagCount, filePath, error, tags vector

## Tests Added (5)
- `TestMetadata_Extract` — extraction returns success with tags
- `TestMetadata_FieldLookup` — field value lookup by MetadataField enum
- `TestMetadata_Standards` — standard name resolution and count
- `TestMetadata_FieldNames` — field name resolution and count
- `TestMetadata_FormatExposure` — exposure time formatting (1/250s)

## Files Modified
- `Engine/CMakeLists.txt` — registered header + source
- `Engine/Tests/EngineTests.cpp` — 5 tests + RUN_TEST calls
