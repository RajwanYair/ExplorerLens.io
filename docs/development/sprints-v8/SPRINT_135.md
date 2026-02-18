# Sprint 135 — JPEG XR / WDP / HDP WIC Decoder

## Objective
Implement JPEG XR decoder via Windows Imaging Component (WIC), supporting .wdp, .hdp, .jxr formats with HDR and wide pixel format handling.

## Scope
- **File**: `Engine/Decoders/JXRWICDecoder.h`
- **Tests**: `tests/Sprint135_JXRWICDecoder.cpp` (13 tests)

## Key Components
| Component | Purpose |
|-----------|---------|
| `JXRFormat` | 3 sub-formats: WDP, HDP, JXR |
| `JXRPixelFormat` | 8 pixel formats including HDR float and CMYK |
| `JXRImageInfo` | Image metadata with DPI, alpha, HDR flags |
| `JXRDecodeOptions` | Thumbnail/FullRes presets with embedded thumbnail preference |
| `JXRWICDecoder` | WIC-based decoder with aspect-ratio scaling |

## Design Decisions
- Leverages built-in Windows WIC codec (no external library needed)
- BGRA32 conversion for DirectX compatibility
- Embedded thumbnail preference for fast path
- Aspect ratio preservation during thumbnail scaling
- HDR pixel format awareness for proper tone mapping

## Supported Extensions
`.wdp`, `.hdp`, `.jxr`

## Status: COMPLETE
