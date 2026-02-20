# Sprint 235 — File Hash Engine

**Sprint Number:** 235  
**Version:** v10.3.0  
**Status:** ✅ Complete

## Objective
File hashing engine supporting CRC32, MD5, SHA-1, SHA-256, and SHA-512 for integrity verification of downloaded updates and archives.

## Files Changed
- `Engine/Utils/FileHashEngine.h` — HashAlgorithm enum, HashResult struct, CRC32 table
- `Engine/Utils/FileHashEngine.cpp` — CRC32 table init, hash computation, verification
- `Engine/CMakeLists.txt` — Registered header and source
- `Engine/Tests/EngineTests.cpp` — 5 unit tests

## Tests Added (5)
1. `TestHash_AlgorithmNames` 2. `TestHash_CRC32` 3. `TestHash_ComputeHash` 4. `TestHash_VerifyHash` 5. `TestHash_AlgorithmCount`
