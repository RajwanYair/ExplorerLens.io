# Sprint 183: Vector Format Expansion — EPS Decoder, AI Routing

**Date:** 2026-02-26
**Version:** v9.0.0-dev
**Phase:** Phase 2 — Format Expansion (Sprint 4 of 7)

## Objective

Add Encapsulated PostScript (EPS/EPSF/PS) decoder with embedded preview extraction,
and route Adobe Illustrator (.ai) files through the existing PDFDecoder since most
modern AI files use PDF internally.

## Changes

### New Files
- **EPSDecoder.h** — Header with EPS binary header struct, TIFF/WMF preview extraction
- **EPSDecoder.cpp** — Full implementation:
  - DOS EPS binary header detection (magic 0xC5D0D3C6)
  - Embedded TIFF preview extraction via GDI+
  - Embedded WMF preview extraction via GDI+ Metafile
  - %%BoundingBox / %%HiResBoundingBox parsing
  - Informative placeholder with page dimensions
  - Supports .eps, .epsf, .ps

### cbxArchive.h
- Added `CBXTYPE_EPS` (87) for PostScript/EPS files
- Added extension mappings: .eps, .epsf, .ps → CBXTYPE_EPS
- Added .ai → CBXTYPE_PDF routing (Adobe Illustrator files are PDF-based)

### CBXShell.rgs
- Added 4 shell registrations: .eps, .epsf, .ps, .ai
- Total shell registrations: 102 → 106

### Engine/CMakeLists.txt
- Registered EPSDecoder.h in ENGINE_HEADERS
- Registered EPSDecoder.cpp in ENGINE_SOURCES

### EngineTests.cpp
- Added `#include "../Decoders/EPSDecoder.h"`
- Added 5 tests:
  - TestEPSDecoder_Create
  - TestEPSDecoder_CanDecode
  - TestEPSDecoder_NoDecodeNonEPS
  - TestEPSDecoder_GetInfo
  - TestPDFDecoder_AIRouting

## Technical Notes
- EPS files often contain embedded TIFF or WMF preview images in a DOS binary header
- The decoder tries TIFF first (higher quality), then WMF, then generates a placeholder
- ASCII EPS/PS files get a placeholder showing BoundingBox dimensions
- .ai files route to CBXTYPE_PDF because modern AI files (CS+) are PDF-based
- PostScript rendering would require Ghostscript; the embedded preview approach avoids that dependency

## Test Count
- Previous: ~449 tests
- Added: 5 tests
- New total: ~454 tests

## CBXTYPE Summary
- CBXTYPE_EPS = 87 (new)
- Next available: 88
