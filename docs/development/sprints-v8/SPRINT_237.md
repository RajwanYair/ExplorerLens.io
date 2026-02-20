# Sprint 237 — Error Recovery Engine

**Sprint Number:** 237  
**Version:** v10.3.0  
**Status:** ✅ Complete

## Objective
Crash recovery engine with 5 strategies (Retry/Checkpoint/Rollback/SafeMode/FullReset), checkpoint save/restore, and state machine for recovery lifecycle.

## Files Changed
- `Engine/Core/ErrorRecoveryEngine.h` — RecoveryStrategy, RecoveryState enums, Checkpoint struct
- `Engine/Core/ErrorRecoveryEngine.cpp` — Checkpoint management, recovery simulation
- `Engine/CMakeLists.txt` — Registered header and source
- `Engine/Tests/EngineTests.cpp` — 5 unit tests

## Tests Added (5)
1. `TestRecovery_StrategyNames` 2. `TestRecovery_CreateCheckpoint` 3. `TestRecovery_RestoreCheckpoint` 4. `TestRecovery_CrashRecovery` 5. `TestRecovery_StrategyCount`
