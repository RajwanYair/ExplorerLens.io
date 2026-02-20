# Sprint 286: HDF5/NetCDF Scientific Decoder

**Status:** ✅ Complete  
**Version:** v12.0  

## Objective
Hierarchical Data Format 5 and NetCDF scientific data visualization thumbnails.

## Deliverables
- `Engine/Decoders/ScientificDataDecoder.h` — 4 formats (HDF5/NetCDF3/NetCDF4/GRIB), 8 data types, 5 vis modes
- HDF5 magic byte detection (\x89HDF\r\n\x1a\n)
- Heatmap/contour/slice/histogram/time-series visualization
- 5 unit tests: format names, data types, vis modes, HDF5 magic, counts
