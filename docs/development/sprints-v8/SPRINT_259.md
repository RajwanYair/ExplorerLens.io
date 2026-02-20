# Sprint 259: FITS Decoder Completion

**Date:** 2026-02-20
**Version:** v11.0.0
**Phase:** Phase 2 — New Format Decoders

## Objective
Minimal FITS reader for 2D astronomical images without cfitsio dependency. Supports SIMPLE FITS with BITPIX 8/16/32/-32/-64 pixel types, BSCALE/BZERO scaling, and asinh stretch for astronomical data visualization.

## Deliverables
- `Engine/Decoders/FITSDecoderV2.h` — Enhanced FITS decoder
- FITSBitpix enum (5 types: UInt8, Int16, Int32, Float32, Float64)
- FITSAxisOrder enum (RowMajor, ColumnMajor, Cube)
- FITSKeyword struct for header card parsing
- FITSImageInfo struct with full metadata (OBJECT, TELESCOP, INSTRUME, BSCALE, BZERO)
- AsinhStretch() for proper astronomical image display
- 5 unit tests validating magic, pixel sizes, validation, normalization

## Technical Details
- FITS magic: "SIMPLE  =" as first 8 bytes
- Header blocks: 2880 bytes, 36 cards per block (80 bytes each)
- BITPIX types: 8, 16, 32, -32 (float), -64 (double)
- Asinh stretch: f(x) = asinh(x/softening) / asinh(1/softening)
- Normalization: linear mapping to 0-255 range

## Test Results
- TestFITSv2_Magic ✅
- TestFITSv2_BytesPerPixel ✅
- TestFITSv2_Validate ✅
- TestFITSv2_DataSize ✅
- TestFITSv2_Normalize ✅
