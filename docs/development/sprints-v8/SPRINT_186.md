# Sprint 186 — SGI/RGB & Legacy (XPM) Image Formats

**Date:** 2026-02-XX  
**Version:** v8.4.0  
**Status:** ✅ Complete

## Objective

Add support for SGI (Silicon Graphics Image) and XPM (X PixMap) legacy image formats, completing Phase 2 format expansion.

## Changes

### SGIDecoder — New Decoder
- **SGIDecoder.h** (new, ~70 lines): `SGIStorageType` enum (Verbatim/RLE), `ImageInfo` struct with channels/bytesPerChannel/imageName
- **SGIDecoder.cpp** (new, ~220 lines): Full implementation
  - Big-endian header parsing (512-byte fixed header, magic 0x01DA)
  - Verbatim (uncompressed) 8-bit decode with planar channel layout
  - RLE decompression with offset/length tables
  - Automatic bottom-up to top-down flip (SGI convention)
  - 1-4 channel support: grayscale, gray+alpha, RGB, RGBA
  - 80-character image name extraction

### XPMDecoder — New Decoder
- **XPMDecoder.h** (new, ~60 lines): `ImageInfo` struct with numColors/charsPerPixel
- **XPMDecoder.cpp** (new, ~220 lines): Full implementation
  - XPM3 format parsing (C source code with quoted strings)
  - Color table parsing: #RRGGBB, #RGB, #RRRRGGGGBBBB (X11)
  - Named color support: black/white/red/green/blue/yellow/cyan/magenta/gray
  - Transparency via "None" color value
  - Variable chars-per-pixel (1-4) lookup
  - Pixel data row parsing from quoted strings

### Shell Integration
- **cbxArchive.h**: Added `CBXTYPE_SGI` (92), `CBXTYPE_XPM` (93); extension routing for .sgi/.rgb/.rgba/.bw/.xpm
- **CBXShell.rgs**: 3 new shell registrations (.sgi, .bw, .xpm) — total now 114
- **CMakeLists.txt**: Registered all 4 new files

### Tests (9 new)
- SGI: ExtensionCheck, Create (6 extensions), InvalidFile, ReadInfoInvalid, StorageTypes
- XPM: ExtensionCheck, Create, InvalidFile, ReadInfoInvalid

## Metrics

| Metric | Before | After |
|---|---|---|
| CBXTYPE range | 0-91 | 0-93 |
| Shell registrations | 111 | 114 |
| Legacy formats | WMF/EMF/PCX/Farbfeld | + SGI/RGB/BW + XPM |
| Unit tests | ~485 | ~494 |

## Technical Notes

- SGI format uses big-endian byte order (inherited from MIPS/IRIX)
- SGI stores channels in separate planes (not interleaved), which is optimal for GPU upload
- RLE in SGI uses a count byte where bit 7 indicates literal (1) vs repeat (0)
- XPM is unique as an ASCII source code format (can be #included in C programs)
- XPM color names come from the X11 color database — we support the 9 most common
- .rgb/.rgba are NOT registered in shell to avoid conflicts with generic color profile extensions

## Files Modified
- `Engine/Decoders/SGIDecoder.h` — New header
- `Engine/Decoders/SGIDecoder.cpp` — New implementation
- `Engine/Decoders/XPMDecoder.h` — New header
- `Engine/Decoders/XPMDecoder.cpp` — New implementation
- `CBXShell/cbxArchive.h` — CBXTYPE_SGI (92), CBXTYPE_XPM (93)
- `CBXShell/CBXShell.rgs` — 3 new registrations (.sgi, .bw, .xpm)
- `Engine/CMakeLists.txt` — Registered new files
- `Engine/Tests/EngineTests.cpp` — 9 new tests
- `docs/development/sprints-v8/SPRINT_186.md` — This document
