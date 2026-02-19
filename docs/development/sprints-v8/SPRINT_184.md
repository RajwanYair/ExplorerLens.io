# Sprint 184 — Game Texture Formats (KTX/KTX2 + VTF)

**Date:** 2026-02-XX  
**Version:** v8.4.0  
**Status:** ✅ Complete

## Objective

Add game engine texture format support: Khronos KTX/KTX2 (GPU-native textures for OpenGL/Vulkan) and Valve Texture Format (VTF, Source Engine textures). Also enhanced DDS awareness for BC7/ASTC.

## Changes

### KTXTextureDecoder — Full Implementation
- **KTXTextureDecoder.cpp** (new, ~280 lines): Complete implementation from Sprint 134 header stub
  - KTX1 header parsing (12-byte magic `0xAB 'K' 'T' 'X' ' ' '1' '1'...`)
  - KTX2 header parsing (12-byte magic with `'2' '0'` version)
  - GL internal format → TextureCompression mapping (12 GL formats)
  - Vulkan VkFormat → TextureCompression mapping (14 VK formats including ASTC)
  - BC1 (DXT1) block decompression with full 4-color interpolation + 1-bit alpha
  - Mip level selection via `BestMipForThumbnail()`
  - KTX1 mip data navigation with alignment
  - Uncompressed fallback with gradient placeholder
  - Block-compressed fallback with checkerboard pattern

- **KTXTextureDecoder.h** updated: Replaced inline stub methods with proper declarations; added private helper signatures (`MapGLFormat`, `MapVkFormat`, `DecodeUncompressed`, `DecodeBlockCompressed`, `DecompressBC1Block`)

### VTFDecoder — New Decoder
- **VTFDecoder.h** (new, ~115 lines): Header with `VTFImageFormat` enum (27 formats), `VTFHeader` struct (VTF v7.x), `TextureInfo` and `DecodeResult` structs
- **VTFDecoder.cpp** (new, ~310 lines): Full implementation
  - VTF signature validation ("VTF\0", version 7.0-7.5)
  - Image size computation for all pixel formats (RGBA/BGR/RGB/DXT/luminance/HDR)
  - Mip level navigation (VTF stores smallest-first)
  - Direct pixel format decoding: BGRA8888, RGBA8888, BGR888, RGB888, I8
  - DXT1 decompression with BC1 block decoder
  - DXT5 decompression with 8-value alpha interpolation + 48-bit alpha index table
  - Low-res thumbnail skipping

### Shell Integration
- **cbxArchive.h**: Added `CBXTYPE_KTX` (88), `CBXTYPE_VTF` (89); extension routing `.ktx`→KTX, `.ktx2`→KTX, `.vtf`→VTF
- **CBXShell.rgs**: 3 new shell registrations (.ktx, .ktx2, .vtf) — total now 109
- **CMakeLists.txt**: Registered VTFDecoder.h in ENGINE_HEADERS, KTXTextureDecoder.cpp + VTFDecoder.cpp in ENGINE_SOURCES

### Tests (11 new)
- `TestKTXDecoder_Create` — Factory creation and availability
- `TestKTXDecoder_ExtensionCheck` — .ktx/.ktx2 acceptance, .png/.dds rejection
- `TestKTXDecoder_ExtensionVersion` — Extension-to-version mapping
- `TestKTXDecoder_CompressionNames` — Compression format name strings
- `TestKTXDecoder_TextureInfo` — Info validation, mip selection, size estimation
- `TestKTXDecoder_SupercompressionNames` — Supercompression name strings
- `TestKTXDecoder_InvalidFile` — Graceful failure on missing files
- `TestVTFDecoder_ExtensionCheck` — .vtf acceptance, other rejection
- `TestVTFDecoder_Create` — Extension array validation
- `TestVTFDecoder_InvalidFile` — Graceful failure on missing files
- `TestVTFDecoder_ImageSizeCompute` — Read invalid file returns invalid info

## Metrics

| Metric | Before | After |
|---|---|---|
| CBXTYPE range | 0-87 | 0-89 |
| Shell registrations | 106 | 109 |
| Game texture formats | DDS only (WIC) | DDS + KTX/KTX2 + VTF |
| BC1 decompression | WIC only | Native BC1 in KTX + VTF decoders |
| DXT5 decompression | WIC only | Native DXT5 in VTF decoder |
| Unit tests | ~465 | ~476 |

## Technical Notes

- KTX1 uses OpenGL internal format enums; KTX2 uses Vulkan VkFormat values
- BC1 decompression implements the full S3TC spec: 4-color mode (c0>c1) and 3-color+transparent mode (c0<=c1)
- DXT5 alpha uses 8-value interpolation (a0>a1) or 6-value+0+255 mode
- VTF stores mipmaps smallest-to-largest (reverse of DDS)
- VTF supports versions 7.0-7.5 (Half-Life 2 through CS:GO)
- ASTC/ETC2 formats are identified but not software-decompressed (placeholder pattern used)

## Files Modified
- `Engine/Decoders/KTXTextureDecoder.h` — Updated class declarations
- `Engine/Decoders/KTXTextureDecoder.cpp` — New implementation
- `Engine/Decoders/VTFDecoder.h` — New header
- `Engine/Decoders/VTFDecoder.cpp` — New implementation
- `CBXShell/cbxArchive.h` — CBXTYPE_KTX (88), CBXTYPE_VTF (89)
- `CBXShell/CBXShell.rgs` — 3 new registrations (.ktx, .ktx2, .vtf)
- `Engine/CMakeLists.txt` — Registered new files
- `Engine/Tests/EngineTests.cpp` — 11 new tests
- `docs/development/sprints-v8/SPRINT_184.md` — This document
