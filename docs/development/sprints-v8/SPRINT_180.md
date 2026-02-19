# Sprint 180 — Easy Format Wins (WMF/EMF, PCX, Farbfeld)

**Version:** v9.0.0-dev  
**Date:** June 2025  
**Phase:** Phase 2 — Format Expansion  
**Status:** ✅ Complete

---

## Objective

Add three new decoders for commonly requested legacy and niche image formats, all implementable without external library dependencies.

## New Decoders

### 1. WMFDecoder — Windows Metafile (.wmf, .emf)
- **Library:** GDI+ (built into Windows)
- **Approach:** Load via `Gdiplus::Metafile`, render with `DrawImage` to HBITMAP
- **Features:** Aspect-ratio preserving render, white background, high-quality bicubic scaling
- **CBXTYPE:** `CBXTYPE_WMF` (82), `CBXTYPE_EMF` (83)

### 2. PCXDecoder — ZSoft PCX (.pcx)
- **Library:** None (custom RLE decoder)
- **Approach:** Parse 128-byte PCX header, RLE-decode scanlines, handle planar color
- **Color depth support:** 1-bit mono, 8-bit indexed (with VGA palette), 24-bit RGB (3 planes)
- **CBXTYPE:** `CBXTYPE_PCX` (84)

### 3. FarbfeldDecoder — Farbfeld (.ff)
- **Library:** None (trivial format)
- **Approach:** Validate 8-byte magic, read BE32 dimensions, convert 16-bit RGBA to 8-bit BGRA
- **CBXTYPE:** `CBXTYPE_FARBFELD` (85)

## Files Created

| File | Purpose |
|------|---------|
| `Engine/Decoders/WMFDecoder.h` | WMF/EMF decoder header |
| `Engine/Decoders/WMFDecoder.cpp` | WMF/EMF decoder implementation |
| `Engine/Decoders/PCXDecoder.h` | PCX decoder header |
| `Engine/Decoders/PCXDecoder.cpp` | PCX decoder implementation |
| `Engine/Decoders/FarbfeldDecoder.h` | Farbfeld decoder header |
| `Engine/Decoders/FarbfeldDecoder.cpp` | Farbfeld decoder implementation |

## Files Modified

| File | Change |
|------|--------|
| `Engine/CMakeLists.txt` | 3 headers + 3 sources registered |
| `CBXShell/cbxArchive.h` | CBXTYPE_WMF (82), CBXTYPE_EMF (83), CBXTYPE_PCX (84), CBXTYPE_FARBFELD (85); 4 extension mappings |
| `CBXShell/CBXShell.rgs` | 4 new shell registrations (.wmf, .emf, .pcx, .ff) — total now 97 |

## CBXTYPE Values Added

| Value | Constant | Extension |
|-------|----------|-----------|
| 82 | CBXTYPE_WMF | .wmf |
| 83 | CBXTYPE_EMF | .emf |
| 84 | CBXTYPE_PCX | .pcx |
| 85 | CBXTYPE_FARBFELD | .ff |
