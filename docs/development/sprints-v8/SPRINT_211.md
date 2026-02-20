# Sprint 211 — eBook Cover Extraction Decoder

**Sprint Number:** 211
**Version:** v10.1.0
**Status:** ✅ Complete

## Objective
Implement an eBook decoder supporting EPUB, MOBI, FB2, and AZW3 formats with cover image extraction for thumbnail generation.

## Files Changed
- `Engine/Decoders/EBookDecoder.h` — Header with EBookFormat enum, CoverResult struct, EBookDecoder class
- `Engine/Decoders/EBookDecoder.cpp` — Full implementation with format detection and cover extraction
- `Engine/CMakeLists.txt` — Registered header and source
- `Engine/Tests/EngineTests.cpp` — 5 unit tests added

## Tests Added (5)
1. `TestEBook_FormatNames` — Format name resolution
2. `TestEBook_DecoderCreation` — Decoder instantiation
3. `TestEBook_FormatDetection` — Extension-based format detection
4. `TestEBook_CoverExtraction` — Cover extraction failure handling
5. `TestEBook_FormatCount` — Format count validation

## Key Features
- EBookFormat enum: EPUB, MOBI, FB2, AZW3, CBZ_EPUB, PDF_EBOOK
- Cover extraction from EPUB (OPF meta-inf), MOBI (EXTH records), FB2 (coverpage element)
- Extension-based format detection (.epub, .mobi, .fb2, .azw3)
- Fallback cover generation for formats without embedded covers
