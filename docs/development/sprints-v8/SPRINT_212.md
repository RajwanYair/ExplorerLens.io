# Sprint 212 — Geospatial Format Decoder

**Sprint Number:** 212
**Version:** v10.1.0
**Status:** ✅ Complete

## Objective
Implement a geospatial data format decoder supporting GeoTIFF, Shapefile, KML, and GeoJSON with coordinate system handling and map projection utilities.

## Files Changed
- `Engine/Decoders/GeospatialDecoder.h` — Header with GeoFormat enum, GeoCoordinate struct, GeospatialDecoder class
- `Engine/Decoders/GeospatialDecoder.cpp` — Full implementation with format detection, Haversine distance, Mercator projection
- `Engine/CMakeLists.txt` — Registered header and source
- `Engine/Tests/EngineTests.cpp` — 5 unit tests added

## Tests Added (5)
1. `TestGeo_FormatNames` — Format name resolution
2. `TestGeo_DecoderCreation` — Decoder instantiation
3. `TestGeo_HaversineDistance` — Great-circle distance calculation (NY to London)
4. `TestGeo_MercatorProjection` — Mercator projection at origin
5. `TestGeo_FormatCount` — Format count validation

## Key Features
- GeoFormat enum: GeoTIFF, Shapefile, KML, KMZ, GeoJSON, GPX
- Haversine distance calculation (great-circle distance between coordinates)
- Mercator projection (lat/lon to screen coordinates)
- Format detection via magic bytes (TIFF, PK for KMZ, JSON for GeoJSON, SHP)
- Thumbnail rendering of geographic boundaries
