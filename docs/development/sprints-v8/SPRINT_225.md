# Sprint 225 — Watch Folder Engine

**Sprint Number:** 225  
**Version:** v10.2.0  
**Status:** ✅ Complete

## Objective
Filesystem watch engine using ReadDirectoryChangesW (native), polling, or hybrid mode for automatic thumbnail regeneration on file changes.

## Files Changed
- `Engine/Core/WatchFolderEngine.h` — FileChangeType, WatchMode enums, WatchFolder/FileChangeEvent structs
- `Engine/Core/WatchFolderEngine.cpp` — Add/remove folders, change simulation, callback dispatch
- `Engine/CMakeLists.txt` — Registered header and source
- `Engine/Tests/EngineTests.cpp` — 5 unit tests

## Tests Added (5)
1. `TestWatch_ChangeTypeNames` 2. `TestWatch_WatchModes` 3. `TestWatch_AddFolder` 4. `TestWatch_RemoveFolder` 5. `TestWatch_SimulateChange`
