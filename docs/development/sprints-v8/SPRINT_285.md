# Sprint 285: CDR/Visio Vector Decoder

**Status:** ✅ Complete  
**Version:** v12.0  

## Objective
CorelDRAW (.cdr) and Visio (.vsd/.vsdx) vector format thumbnail generation.

## Deliverables
- `Engine/Decoders/VectorFormatDecoder.h` — 7 vector formats (CDR/CMX/VSD/VSDX/AI/WMF/EMF), 7 element types
- Extension-based format detection, anti-aliased rendering config
- 5 unit tests: format names, element names, detection, counts, config defaults
