# Sprint 133 — JPEG 2000 Decoder

## Objective
Implement JPEG 2000 decoder interface supporting JP2/J2K/JPX/JPH formats with wavelet resolution-level thumbnail extraction via OpenJPEG-compatible API.

## Scope
- **File**: `Engine/Decoders/JPEG2000Decoder.h`
- **Tests**: `tests/Sprint133_JPEG2000Decoder.cpp` (14 tests)

## Key Components
| Component | Purpose |
|-----------|---------|
| `JP2Format` | 4 sub-formats: JP2, J2K, JPX, JPH |
| `JP2Extensions` | 8 supported extensions with case-insensitive matching |
| `JP2ImageInfo` | Image metadata with reduction-level calculator |
| `JP2DecodeOptions` | Thumbnail vs FullResolution presets with memory limits |
| `JP2DecodeResult` | Status, pixel data, timing, reduction level used |
| `JPEG2000Decoder` | Main decoder with ReadInfo and DecodeThumbnail |

## Design Decisions
- Wavelet resolution levels enable efficient thumbnail extraction without full decode
- BestReductionLevel finds the smallest decode that exceeds target dimensions
- Memory limit enforcement before allocation (256 MB default, 64 MB for thumbnails)
- Extension-to-format classification for format-specific decode paths
- 8 error states covering all failure modes

## Supported Extensions
`.jp2`, `.j2k`, `.j2c`, `.jpf`, `.jpx`, `.jph`, `.jhc`, `.jpc`

## Test Coverage
- Extension support and case-insensitive matching
- Format classification from extension
- Image info validity and size estimation
- Resolution level optimization for thumbnails
- Decode options presets
- Thumbnail decode result validation

## Status: COMPLETE
