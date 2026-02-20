# Sprint 278: USD/USDZ Support

**Status:** ✅ Complete  
**Version:** v11.2  

## Objective
Pixar Universal Scene Description decoder for 3D scene thumbnails.

## Deliverables
- `Engine/Decoders/USDDecoder.h` — 7 element types, 3 variants (USDA/USDC/USDZ)
- USDZ ZIP magic detection, variant detection from extension
- USDSceneInfo with mesh/material/camera/light counts
- 5 unit tests: element names, variant names, detect variant, USDZ magic, counts
