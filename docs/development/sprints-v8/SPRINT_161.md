# Sprint 161: CAD Format Plugin (DWG/DXF/STEP/IGES)

**Block:** v8.3.0 — Phase P3: Format Expansion  
**Status:** ✅ Done  
**Sprint Count:** 161 / 174

---

## Overview

Adds a plugin-based CAD format decoder that generates badge thumbnails for DWG, DXF, DWF,
DGN, STEP, and IGES files. Integrates with the DarkThumbs plugin architecture via
`ICADDecoderPlugin`.

---

## Deliverables

| Artifact | Path | Notes |
|---|---|---|
| Header | `Engine/Decoders/CADFormatPlugin.h` | `ICADDecoderPlugin`, `CADBadgeThumbnail`, format detection |
| GTest | `Engine/Tests/Sprint161_CADFormat.cpp` | 15 test cases |
| Sprint doc | `docs/development/sprints-v8/SPRINT_161.md` | This document |

---

## Supported Formats

| Format | Extension | Type |
|---|---|---|
| DWG | `.dwg` | AutoCAD native drawing |
| DXF | `.dxf` | AutoCAD exchange format |
| DWF | `.dwf` | Design Web Format |
| DGN | `.dgn` | Bentley MicroStation |
| STEP | `.stp`, `.step` | ISO 10303 3D exchange |
| IGES | `.igs`, `.iges` | Initial Graphics Exchange |

---

## Tests (15)

- `CADFormat_DetectDWG`
- `CADFormat_DetectDXF`
- `CADFormat_DetectSTEP`
- `CADFormat_DetectIGES`
- `CADFormat_DetectDGN`
- `CADFormat_DetectUnknown` — unknown extension returns Unknown
- `CADFormat_PluginInterface` — ICADDecoderPlugin interface complete
- `CADFormat_BadgeThumbnailType` — badge type enum values
- `CADFormat_BadgeThumbnailGeneration` — badge generation stub
- `CADFormat_PluginMetadata` — plugin name/version fields
- `CADFormat_ExtensionList` — 8+ extensions registered
- `CADFormat_FormatDisplayName`
- `CADFormat_PriorityValue`
- `CADFormat_ThumbnailDimensions`
- `CADFormat_CADFormatPluginFactory` — factory creates non-null plugin

---

## Acceptance Criteria

- [x] Header compiles with `/W4` zero warnings
- [x] 6 CAD formats with extensions mapped
- [x] Badge thumbnail generation stub
- [x] All 15 GTest cases pass
- [x] Sprint doc created
