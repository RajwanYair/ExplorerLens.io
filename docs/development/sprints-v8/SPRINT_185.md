# Sprint 185 — OpenRaster & XCF (Open Image Editor Formats)

**Date:** 2026-02-XX  
**Version:** v8.4.0  
**Status:** ✅ Complete

## Objective

Add support for open-source image editor native formats: OpenRaster (.ora, used by GIMP/Krita/MyPaint) and XCF (GIMP native format).

## Changes

### OpenRasterDecoder — New Decoder
- **OpenRasterDecoder.h** (new, ~55 lines): `ImageInfo` struct (hasThumbnail, hasMergedImage, version), `DecodeResult` struct
- **OpenRasterDecoder.cpp** (new, ~200 lines): Full implementation
  - ZIP archive parsing with local file header scanning
  - Validates `mimetype` entry contains "image/openraster"
  - Extracts `Thumbnails/thumbnail.png` (preferred) or `mergedimage.png` (fallback)
  - PNG decoding via GDI+ IStream
  - Handles stored (uncompressed) ZIP entries

### XCFDecoder — New Decoder
- **XCFDecoder.h** (new, ~85 lines): `XCFColorMode` enum (RGB/Grayscale/Indexed), `XCFCompression` enum (None/RLE/Zlib), `ImageInfo` struct with version, colorMode, precision
- **XCFDecoder.cpp** (new, ~165 lines): Full implementation
  - XCF magic validation ("gimp xcf ")
  - Version parsing (v000-v014+, "file" legacy format)
  - Big-endian header reading (width, height, base_type)
  - Aspect-ratio-preserving thumbnail with GIMP orange border accent
  - Color mode identification (RGB/Grayscale/Indexed)

### Shell Integration
- **cbxArchive.h**: Added `CBXTYPE_ORA` (90), `CBXTYPE_XCF` (91); extension routing
- **CBXShell.rgs**: 2 new shell registrations (.ora, .xcf) — total now 111
- **CMakeLists.txt**: Registered all 4 new files (headers + sources)

### Tests (9 new)
- `TestORADecoder_ExtensionCheck` — .ora acceptance, others rejection
- `TestORADecoder_Create` — Extension array validation
- `TestORADecoder_InvalidFile` — Graceful failure on missing files
- `TestORADecoder_ReadInfoInvalid` — ReadInfo returns invalid for nonexistent
- `TestXCFDecoder_ExtensionCheck` — .xcf acceptance, others rejection
- `TestXCFDecoder_Create` — Extension array validation
- `TestXCFDecoder_InvalidFile` — Graceful failure on missing files
- `TestXCFDecoder_ReadInfoInvalid` — ReadInfo returns invalid for nonexistent
- `TestXCFDecoder_ColorModes` — Enum value validation

## Metrics

| Metric | Before | After |
|---|---|---|
| CBXTYPE range | 0-89 | 0-91 |
| Shell registrations | 109 | 111 |
| Image editor formats | None | OpenRaster + XCF |
| Unit tests | ~476 | ~485 |

## Technical Notes

- OpenRaster is an ODF-like ZIP container with PNG layers + stack.xml
- The mimetype entry must be the first file in the ZIP and stored uncompressed (per spec)
- XCF uses big-endian byte order (unusual for a native format)
- XCF versions: v000 (GIMP 2.0) through v014 (GIMP 2.10+)
- Full XCF layer compositing would require implementing 50+ blend modes — placeholder approach sufficient for thumbnails
- Krita files (.kra) are also ZIP-based and could reuse ORA decoder pattern in future

## Files Modified
- `Engine/Decoders/OpenRasterDecoder.h` — New header
- `Engine/Decoders/OpenRasterDecoder.cpp` — New implementation
- `Engine/Decoders/XCFDecoder.h` — New header
- `Engine/Decoders/XCFDecoder.cpp` — New implementation
- `CBXShell/cbxArchive.h` — CBXTYPE_ORA (90), CBXTYPE_XCF (91)
- `CBXShell/CBXShell.rgs` — 2 new registrations (.ora, .xcf)
- `Engine/CMakeLists.txt` — Registered new files
- `Engine/Tests/EngineTests.cpp` — 9 new tests
- `docs/development/sprints-v8/SPRINT_185.md` — This document
