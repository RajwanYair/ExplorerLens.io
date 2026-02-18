# Sprint 134 — KTX/KTX2 Texture Decoder

## Objective
Implement GPU-native texture container decoder for Khronos KTX and KTX2 formats, supporting block-compressed textures (BC1-BC7, ASTC, ETC2) with mipmap-level thumbnail extraction.

## Scope
- **File**: `Engine/Decoders/KTXTextureDecoder.h`
- **Tests**: `tests/Sprint134_KTXTextureDecoder.cpp` (13 tests)

## Key Components
| Component | Purpose |
|-----------|---------|
| `KTXVersion` | KTX1 (OpenGL) and KTX2 (Vulkan) differentiation |
| `TextureCompression` | 13 GPU block compression formats (BC1-BC7, ASTC, ETC1/2) |
| `KTXSupercompression` | KTX2 container-level compression: BasisLZ, Zstd, ZLIB |
| `KTXTextureInfo` | Full metadata: dimensions, mips, faces, compression type |
| `KTXTextureDecoder` | Main decoder with Info + Thumbnail extraction |

## Design Decisions
- Mipmap level selection for efficient thumbnail extraction without full decode
- Block-compressed size estimation for memory budgeting
- Cubemap and 3D texture awareness (decode first face/slice for thumbnail)
- sRGB flag tracking for correct color space handling

## Supported Extensions
`.ktx`, `.ktx2`

## Status: COMPLETE
