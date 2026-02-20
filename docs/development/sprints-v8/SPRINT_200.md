# Sprint 200 — Advanced 3D Format Decoder

**Date:** 2026-01-20  
**Version:** v10.0.0  
**Status:** ✅ Complete

## Objective
Add native detection and wireframe thumbnail generation for industry-standard 3D
model formats: FBX, USD/USDA/USDC/USDZ, 3MF, STEP, and IGES.

## Deliverables
| Artifact | Path |
|----------|------|
| Header | `Engine/Decoders/Advanced3DFormatDecoder.h` |
| Source | `Engine/Decoders/Advanced3DFormatDecoder.cpp` |
| Tests | 5 tests in `Engine/Tests/EngineTests.cpp` |
| CMake | Registered in `Engine/CMakeLists.txt` |

## Key Features
- `Format3D` enum: FBX, USDA, USDC, USDZ, 3MF, STEP, IGES
- FBX binary magic detection (`Kaydara FBX Binary  \0`)
- `BoundingBox3D` and `AutoCamera` computation for framing
- `MeshInfo3D` wireframe complexity estimation
- 10 file extensions supported

## Tests Added (5)
1. `TestAdvanced3D_FBXDetection` — binary magic recognition
2. `TestAdvanced3D_Extensions` — extension count and presence
3. `TestAdvanced3D_FormatNames` — format→name mapping
4. `TestAdvanced3D_BoundingBox` — auto-camera from bounding box
5. `TestAdvanced3D_WireframeRender` — wireframe complexity estimation

## Impact
- Decoder count: 27 → 28
- Shell registrations: +10 extensions (.fbx, .usd, .usda, .usdc, .usdz, .3mf, .stp, .step, .igs, .iges)
