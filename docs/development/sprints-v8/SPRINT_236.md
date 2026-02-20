# Sprint 236 — Registry Manager

**Sprint Number:** 236  
**Version:** v10.3.0  
**Status:** ✅ Complete

## Objective
Windows registry read/write manager for settings persistence with HKCU/HKLM/HKCR hive support and 5 value types (REG_SZ, REG_DWORD, REG_QWORD, REG_BINARY, REG_MULTI_SZ).

## Files Changed
- `Engine/Core/RegistryManager.h` — RegHive, RegValueType enums, RegEntry struct
- `Engine/Core/RegistryManager.cpp` — In-memory registry simulation, CRUD operations
- `Engine/CMakeLists.txt` — Registered header and source
- `Engine/Tests/EngineTests.cpp` — 5 unit tests

## Tests Added (5)
1. `TestReg_HiveNames` 2. `TestReg_WriteRead` 3. `TestReg_DefaultValue` 4. `TestReg_Delete` 5. `TestReg_BasePath`
