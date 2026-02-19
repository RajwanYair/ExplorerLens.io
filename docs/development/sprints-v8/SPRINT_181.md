# Sprint 181 — JPEG 2000 Implementation

**Version:** v9.0.0-dev  
**Date:** June 2025  
**Phase:** Phase 2 — Format Expansion  
**Status:** ✅ Complete

---

## Objective

Implement the JPEG 2000 decoder (Sprint 133/160 header) with OpenJPEG integration, supporting JP2/J2K/JPX/JPH formats with wavelet reduction-level thumbnail extraction.

## Changes

### 1. JPEG2000Decoder.cpp (New)
- Full implementation of `DecodeJPEG2000Thumbnail()` for shell extension integration
- OpenJPEG backend (`#ifdef HAS_OPENJPEG`) with reduction-level decoding for efficient thumbnails
- Fallback backend for builds without OpenJPEG (placeholder gradient + J2K SIZ marker parsing)
- JP2 magic byte detection (JP2 signature box + raw J2K SOC marker)
- Extension-based format classification fallback
- BGRA pixel conversion for both RGB and grayscale JP2 images

### 2. CBXTYPE_JP2 (86) Added
- New type constant for JPEG 2000 family
- Extension routing for `.jp2`, `.j2k`, `.j2c`, `.jpx`, `.jpf`, `.jph`

### 3. Shell Registrations
- 5 new extensions: `.jp2`, `.j2k`, `.j2c`, `.jpx`, `.jph`
- Total shell registrations: 102

## Supported Extensions

| Extension | Format | Spec |
|-----------|--------|------|
| .jp2 | JPEG 2000 Part 1 | ISO 15444-1 |
| .j2k, .j2c | J2K Raw Codestream | ISO 15444-1 |
| .jpx, .jpf | JPEG 2000 Part 2 Extended | ISO 15444-2 |
| .jph | JPEG 2000 High-Throughput | ISO 15444-15 |

## Files Changed

| File | Action |
|------|--------|
| `Engine/Decoders/JPEG2000Decoder.cpp` | Created — full implementation |
| `Engine/CMakeLists.txt` | Source registered |
| `CBXShell/cbxArchive.h` | CBXTYPE_JP2 (86) + 6 extension mappings |
| `CBXShell/CBXShell.rgs` | 5 shell registrations |

## Build Notes

- Without OpenJPEG: builds and runs with fallback placeholder thumbnails
- With OpenJPEG: `cmake -DHAS_OPENJPEG=ON` enables full wavelet decoding
- Future: add OpenJPEG to `external/image-libs/` build scripts
