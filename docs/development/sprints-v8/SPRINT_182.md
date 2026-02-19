# Sprint 182: Enhanced Model Decoder — PLY, DAE, Flat-Shading Lighting

**Date:** 2026-02-26
**Version:** v8.4.0 → v9.0.0-dev
**Phase:** Phase 2 — Format Expansion (Sprint 3 of 7)

## Objective

Enhance the ModelDecoder with PLY (Stanford Polygon Format) and DAE (COLLADA) parsers,
expand extension support from 4 to 8 formats, and replace the basic wireframe renderer
with a flat-shaded renderer using directional lighting and depth-sorted painter's algorithm.

## Changes

### ModelDecoder.h
- Added `LoadPLY()`, `LoadPLYAscii()`, `LoadPLYBinary()` declarations
- Added `LoadDAE()` declaration
- Added `ComputeFaceNormals()` for automatic normal generation
- Added `LightParams` struct and lighting helper methods
- Extended `s_extensions[]` from 5 to 9 slots (8 formats + nullptr)
- Added `<string>` include

### ModelDecoder.cpp
- **PLY Parser:** Full ASCII and binary (little-endian) support
  - Parses PLY header for element/property declarations
  - Handles point clouds (no face element) and triangle meshes
  - Fan triangulation for polygons with >3 vertices
  - 2M vertex safety cap for thumbnail generation
- **DAE (COLLADA) Parser:** Lightweight XML text parser
  - Extracts position data from `<float_array>` elements
  - Identifies position arrays by source ID containing "position"
  - Parses `<triangles>` and `<polylist>` index elements
  - Handles variable index stride (vertex/normal/texcoord interleaving)
  - Falls back to sequential triangles when no indices present
- **Flat-Shading Renderer:**
  - Isometric projection (30° Y rotation, 20° X tilt)
  - Directional lighting from upper-right (Lambertian diffuse + ambient)
  - Two-sided lighting (absolute value of dot product)
  - Depth-sorted painter's algorithm for correct occlusion
  - Steel blue base color on dark charcoal background
  - Point cloud visualization for meshes without faces
- **Extended Extensions:** .obj, .stl, .gltf, .glb, .ply, .dae, .3ds, .fbx
- Version bumped to 2.0.0

### EngineTests.cpp
- Added 6 new tests:
  - `TestModelDecoder_PLYSupport` — PLY extension recognition
  - `TestModelDecoder_DAESupport` — DAE extension recognition
  - `TestModelDecoder_3DSSupport` — 3DS extension recognition
  - `TestModelDecoder_FBXSupport` — FBX extension recognition
  - `TestModelDecoder_ExpandedExtensions` — All 8 extensions + negatives
  - `TestModelDecoder_ExtensionCount` — Verifies count is 8

## Pre-Existing State (Unchanged)
- `.ply`, `.dae`, `.3ds`, `.fbx` already registered in CBXShell.rgs (Sprint 176)
- All 8 extensions already mapped to CBXTYPE_MODEL in cbxArchive.h (Sprint 175)
- No new shell registrations or CBXTYPE values needed

## Technical Notes
- PLY binary big-endian is detected but not byte-swapped (extremely rare format)
- DAE parser is a lightweight text search, not a full XML DOM — sufficient for thumbnail
- GLTF/GLB still returns false (needs tinygltf/cgltf library integration)
- FBX/3DS return false via CanDecode true path but no loader — recognized but not rendered

## Test Count
- Previous: ~443 tests
- Added: 6 tests
- New total: ~449 tests
