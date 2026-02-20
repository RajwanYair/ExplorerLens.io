# Sprint 245 — ContentIndexer

**Date:** 2026-06-15
**Component:** `Engine/Core/ContentIndexer.h`, `Engine/Core/ContentIndexer.cpp`
**Theme:** File Content Indexing for Search

## Summary
Implemented a file content indexer supporting 8 content types (Image, Archive, Document, Video, Audio, 3D Model, Font, Unknown) with extension-based classification covering 40+ file extensions. Features include add/remove/update lifecycle, batch indexing, search by name/type/extension, stats with per-type counts, and purge of removed entries.

## Key Types
- `ContentType` — Image, Archive, Document, Video, Audio, Model3D, Font, Unknown (8 types)
- `IndexState` — Pending, Indexed, Failed, Stale, Removed (5 states)
- `IndexEntry` — id, filePath, fileName, extension, contentType, state, fileSize, timestamps, dimensions
- `IndexStats` — counts per state + per content type + total size

## Tests Added (5)
- `TestIndexer_AddFile` — add file creates entry with valid id
- `TestIndexer_ClassifyExtension` — jpg→Image, zip→Archive, pdf→Document, mp4→Video
- `TestIndexer_IndexAll` — batch index marks all pending as indexed
- `TestIndexer_ContentTypeNames` — all 8 type names resolve
- `TestIndexer_SearchByType` — search by type filters correctly

## Files Modified
- `Engine/CMakeLists.txt` — registered header + source
- `Engine/Tests/EngineTests.cpp` — 5 tests + RUN_TEST calls
