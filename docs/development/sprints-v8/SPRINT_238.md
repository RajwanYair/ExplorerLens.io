# Sprint 238 — Log Rotation Engine

**Sprint Number:** 238  
**Version:** v10.3.0  
**Status:** ✅ Complete

## Objective
Log file rotation with 4 policies (Size/Time/Count/Hybrid), compression support (GZip/Zstd/LZ4), and automatic cleanup of excess rotated files.

## Files Changed
- `Engine/Utils/LogRotationEngine.h` — RotationPolicy, LogCompression enums, RotationConfig struct
- `Engine/Utils/LogRotationEngine.cpp` — Rotation trigger check, file tracking, cleanup
- `Engine/CMakeLists.txt` — Registered header and source
- `Engine/Tests/EngineTests.cpp` — 5 unit tests

## Tests Added (5)
1. `TestLogRot_PolicyNames` 2. `TestLogRot_CompressionNames` 3. `TestLogRot_NeedsRotation` 4. `TestLogRot_Cleanup` 5. `TestLogRot_PolicyCount`
