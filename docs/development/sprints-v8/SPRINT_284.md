# Sprint 284: FLIF/BPG Legacy Image Decoder

**Status:** ✅ Complete  
**Version:** v12.0  

## Objective
Free Lossless Image Format (FLIF) and Better Portable Graphics (BPG) decoders.

## Deliverables
- `Engine/Decoders/LegacyImageDecoder.h` — 6 legacy formats (FLIF/BPG/JPEG2000/PCX/TGA/SGI), 6 color spaces
- FLIF magic ("FLIF") and BPG magic (0x42504746FB) detection
- LegacyImageInfo with dimensions, BPP, frame count, alpha, interlace
- 5 unit tests: format names, color spaces, FLIF magic, BPG magic, counts
