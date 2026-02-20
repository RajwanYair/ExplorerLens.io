# Sprint 222 — Format Converter Engine

**Sprint Number:** 222  
**Version:** v10.1.0  
**Status:** ✅ Complete

## Objective
Batch format conversion between PNG, JPEG, WebP, BMP, TIFF, AVIF, and JPEG XL with quality presets and compression ratio tracking.

## Files Changed
- `Engine/Core/FormatConverterEngine.h` — ConvertFormat, QualityPreset enums, ConversionJob struct
- `Engine/Core/FormatConverterEngine.cpp` — Format detection, single/batch conversion, quality mapping
- `Engine/CMakeLists.txt` — Registered header and source
- `Engine/Tests/EngineTests.cpp` — 5 unit tests

## Tests Added (5)
1. `TestConverter_FormatNames` 2. `TestConverter_FormatDetection` 3. `TestConverter_QualityPresets` 4. `TestConverter_FormatExtensions` 5. `TestConverter_FormatCount`
