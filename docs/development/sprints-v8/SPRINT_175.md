# Sprint 175: Critical Bug Fixes

**Status:** ✅ COMPLETE  
**Date:** 2026-02-19  
**Version:** v8.4.0  
**Phase:** Phase 1 — Foundation Fix (Sprints 175–179)

## Objective

Fix critical routing bugs, decoder overlap, and missing format routing discovered during the v8.3.0 comprehensive audit.

## Changes Made

### 1. Fixed `.djvu`/`.djv` Routing Bug (🔴 Critical)
- **File:** `CBXShell/cbxArchive.h` — `GetCBXType()`
- **Before:** `.djvu` and `.djv` incorrectly returned `CBXTYPE_EPUB`
- **After:** Now correctly return `CBXTYPE_DJVU` (value 22)
- **Impact:** DjVu files now get proper decoder routing

### 2. Added 3D Model Format Routing (🟡 High)
- **File:** `CBXShell/cbxArchive.h` — `GetCBXType()`
- `CBXTYPE_MODEL` (80) was defined but had zero extension mappings
- Added: `.obj`, `.stl`, `.gltf`, `.glb`, `.fbx`, `.3ds`, `.dae`, `.ply`
- **Impact:** ModelDecoder is now reachable from shell extension

### 3. Fixed AVIF/HEIF Decoder Extension Overlap (🟡 High)
- **File:** `Engine/Decoders/AVIFDecoder.cpp`
  - Removed `.heif` and `.heic` from AVIFDecoder
  - Added `.avifs` to AVIFDecoder
  - AVIFDecoder now handles ONLY: `.avif`, `.avifs`
- **File:** `Engine/Decoders/HEIFDecoder.h`
  - Removed `.avif` from HEIFDecoder
  - HEIFDecoder now handles: `.heif`, `.heic`, `.hif`, `.heifs`, `.heics`, `.avci`, `.avcs`, `.heif-sequence`, `.heic-sequence`
- **Impact:** No more ambiguous decoder selection for HEIF/AVIF formats

### 4. Added Missing HEIF Extensions to GetCBXType (🟡 High)
- **File:** `CBXShell/cbxArchive.h` — `GetCBXType()`
- Added: `.hif` → `CBXTYPE_HEIC`, `.avci` → `CBXTYPE_HEIC`, `.avcs` → `CBXTYPE_HEIF`
- **Impact:** Full HEIF format coverage in shell extension routing

### 5. Updated Header Version Comment
- **File:** `CBXShell/cbxArchive.h` line 2
- Changed from `v4.6` to `v8.4.0 (Sprint 175+)`

### 6. Updated CBXTYPE Comment
- Changed "Future format support (not yet implemented)" to "Extended format support (v8.4+)"
- `CBXTYPE_DJVU` is now actively used

## Files Modified
- `CBXShell/cbxArchive.h`
- `Engine/Decoders/AVIFDecoder.cpp`
- `Engine/Decoders/HEIFDecoder.h`

## Test Impact
- Existing tests should pass (routing changes are additive)
- New test cases recommended for DJVU routing and Model routing
