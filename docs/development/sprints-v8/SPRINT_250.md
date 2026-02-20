# Sprint 250: Format Registry Refactor

**Status:** ✅ Complete  
**Version:** 10.6.0  
**Date:** 2026-02-20  
**Phase:** Phase 1 — Architecture & Quality Foundation

## Objective
Replace `#define CBXTYPE int` with `enum class FormatType : uint16_t`. Create `FormatRegistry` singleton mapping extension → type → decoder → shell registration. Provides type safety and centralized format management.

## Deliverables
- `Engine/Core/FormatRegistry.h` — FormatRegistry singleton with Register/Lookup/Validate
- `FormatType` enum class with 50+ format identifiers (values 0-255)
- `FormatCategory` enum for format grouping (19 categories)
- `FormatEntry` struct linking type/category/extension/decoder
- Validation to detect orphaned extensions or missing primary ext
- 5 unit tests

## Files
- `Engine/Core/FormatRegistry.h` (NEW)
- `Engine/CMakeLists.txt` (ENGINE_HEADERS registration)
- `Engine/Tests/EngineTests.cpp` (5 tests)
