# Sprint 217 — Portable Mode Manager

**Sprint Number:** 217  
**Version:** v10.1.0  
**Status:** ✅ Complete

## Objective
Enable registry-free portable mode for USB/removable drives with INI-based configuration storage, directory structure management, and drive detection.

## Files Changed
- `Engine/Utils/PortableModeManager.h` — PortableStatus, StorageLocation enums, PortableConfig struct
- `Engine/Utils/PortableModeManager.cpp` — Drive detection, INI config, directory creation
- `Engine/CMakeLists.txt` — Registered header and source
- `Engine/Tests/EngineTests.cpp` — 5 unit tests added

## Tests Added (5)
1. `TestPortable_StatusNames` — Status name resolution
2. `TestPortable_LocationNames` — Storage location naming
3. `TestPortable_LocationCount` — Location count validation
4. `TestPortable_Detection` — Runtime portable detection
5. `TestPortable_CacheSize` — Cache size get/set

## Key Features
- 4 portable statuses: Installed, Portable, Hybrid, Unknown
- 5 storage locations: Registry, INI File, JSON File, AppData, Exe Directory
- Drive type detection (removable vs fixed)
- Write access verification with temp file test
- INI config save/load with [DarkThumbs] section
