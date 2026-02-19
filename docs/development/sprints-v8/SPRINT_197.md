# Sprint 197: Malformed Input Hardening

**Status:** ✅ Complete  
**Version:** v9.2.0  
**Phase:** Phase 4 — Platform & Polish (Sprint 5 of 6)

## Objective
Implement robust malformed/corrupt file handling with header validation, decompression bomb detection, magic byte verification, and structured error reporting.

## Deliverables

### Engine/Utils/MalformedInputHandler.h
- `CorruptionType` enum (13 types: TruncatedFile, InvalidMagic, DecompressionBomb, etc.)
- `ValidationSeverity` enum (4 levels: Info, Warning, Error, Critical)
- `ValidationIssue` — per-check result with file offset
- `FileValidationResult` — aggregate result with worst-issue tracking
- `BombLimits` — max decompressed size (256MB), ratio (100:1), pixel count (256M), nesting (3)
- `MalformedInputConfig` — timeout (30s), max file size (4GB), placeholder-on-error

### Engine/Utils/MalformedInputHandler.cpp
- Magic byte signatures: PNG (8B), JPEG (3B), GIF87/89 (6B), BMP (2B), ZIP (4B), PDF (4B), WebP (RIFF+WEBP)
- `ValidateFile()` — zero-byte check, max size check, header read (64B)
- `AreDimensionsSafe()` — width/height/pixel count validation
- `IsCompressionRatioSafe()` — ratio + absolute size limit
- `ClampDimensions()` — aspect-preserving clamping

## Test Coverage
10 tests: DefaultConfig, DimensionsSafe, DimensionsBomb, CompressionRatio, NestingDepth, MagicBytesPNG, MagicBytesJPEG, MagicBytesMultiple, ClampDimensions, CorruptionNames

## Files Changed
- `Engine/Utils/MalformedInputHandler.h` (new)
- `Engine/Utils/MalformedInputHandler.cpp` (new)
- `Engine/CMakeLists.txt` (registered)
- `Engine/Tests/EngineTests.cpp` (10 tests added)
- `docs/development/sprints-v8/SPRINT_197.md` (this file)
