# Sprint 288: STEP/IGES CAD Decoder

**Status:** ✅ Complete  
**Version:** v12.0  

## Objective
ISO 10303 STEP and IGES CAD file thumbnail generation.

## Deliverables
- `Engine/Decoders/CADFormatDecoder.h` — 4 CAD formats (STEP AP203/214/242, IGES), 7 entity types, 5 render modes
- STEP magic detection ("ISO-10303-21;")
- Wireframe/shaded/xray rendering modes with configurable camera
- 5 unit tests: format names, entity names, render modes, STEP magic, counts
