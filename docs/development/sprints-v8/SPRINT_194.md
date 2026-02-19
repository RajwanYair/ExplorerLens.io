# Sprint 194: High-DPI Support

**Status:** ✅ Complete  
**Date:** 2025-07-17  
**Version:** v9.2.0  

## Objective
Implement per-monitor DPI awareness for thumbnail generation, supporting scale factors from 100% to 400% with automatic logical-to-physical pixel conversion.

## Changes

### New Files
- `Engine/Core/HighDPIScaling.h` — DPI scale enum, awareness modes, config
- `Engine/Core/HighDPIScaling.cpp` — System DPI detection, pixel conversion, monitor enumeration

### Key Features
1. **Per-Monitor DPI** — Detects DPI for each monitor independently
2. **9 Scale Presets** — 100%, 125%, 150%, 175%, 200%, 250%, 300%, 350%, 400%
3. **DPI Awareness Modes** — Unaware, SystemAware, PerMonitorV1, PerMonitorV2
4. **Pixel Conversion** — LogicalToPhysical/PhysicalToLogical functions
5. **Scaled Thumbnail Requests** — Auto-scale thumbnail size for target DPI
6. **Monitor Enumeration** — List all monitors with DPI info
7. **Dynamic Scaling** — Support for WM_DPICHANGED responses

### Tests Added (10)
- TestDPI_SystemDPI, TestDPI_GetMonitorDPI, TestDPI_EnumerateMonitors
- TestDPI_LogicalPhysicalConversion, TestDPI_ScaleRequest
- TestDPI_NearestScale, TestDPI_DPIForScale, TestDPI_ScaleFactors
- TestDPI_ScaleNames, TestDPI_AwarenessNames

### Registration
- `Engine/CMakeLists.txt` — Added to ENGINE_HEADERS and ENGINE_SOURCES (Core section)
- `Engine/Tests/EngineTests.cpp` — Include + 10 tests + RUN_TEST calls
