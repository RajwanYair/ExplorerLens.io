# Sprint 191: Memory Safety

**Date:** 2026-03-15  
**Version:** v9.1.0  
**Phase:** 3 — Performance & Quality  
**Status:** ✅ Complete

## Objective

Integrate AddressSanitizer (ASAN) build support and memory-mapped I/O for efficient large file handling.

## Deliverables

### New Files
- `Engine/Utils/MemorySafetyIntegration.h` — ASAN + memory-mapped I/O API
- `Engine/Utils/MemorySafetyIntegration.cpp` — Implementation with Windows API

### Key Features
1. **ASAN Detection** — Runtime check for `/fsanitize=address` builds
2. **Build Config Generation** — MSVC `/fsanitize=address` + CMake options
3. **Memory-Mapped File I/O** — CreateFileMapping/MapViewOfFile for large files
4. **SafeBuffer** — Bounds-checked buffer with Read/Seek/Available
5. **Access Validation** — Offset+length bounds checking
6. **Max Mappable Size** — 2GB (x64) / 256MB (x86) safety limits
7. **Access Patterns** — Sequential/Random/HeaderOnly/Streaming hints

## Test Summary (10 tests)
- ASANDetection, RecommendedConfig, CompilerFlags, CMakeOptions
- SafeBuffer, SafeBufferBounds, ValidateAccess
- SanitizerNames, AccessPatterns, MaxMappableSize
