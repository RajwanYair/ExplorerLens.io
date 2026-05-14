# Decoder Implementation Status

**Last Verified:** v39.9.0 "Betelgeuse"
**Auto-audit:** Run `.\build-scripts\Audit-Headers.ps1` for header-level inventory

## Top 20 Priority Formats — All Implemented

| # | Format | Decoder File | Library | Status | Notes |
|---|--------|-------------|---------|--------|-------|
| 1 | JPEG | ImageDecoder.cpp | WIC | **REAL** | EXIF rotation, progressive, CMYK |
| 2 | PNG | ImageDecoder.cpp | WIC | **REAL** | 8/16-bit, alpha, interlaced |
| 3 | WebP | WebPDecoder.cpp | libwebp 1.5+ | **REAL** | Lossy, lossless, alpha, animated |
| 4 | AVIF | AVIFDecoder.cpp | WIC + libavif/dav1d | **REAL** | 8/10-bit, HDR, `HAS_LIBAVIF` |
| 5 | HEIC | HEIFDecoder.cpp | WIC + libheif/libde265 | **REAL** | `HAS_LIBHEIF` flag |
| 6 | JXL | JXLDecoder.cpp | libjxl 0.11.1 | **REAL** | `HAS_LIBJXL` flag |
| 7 | PDF | PDFDecoder.cpp | MuPDF 1.24.11 | **REAL** | First-page render; `HAS_MUPDF` (AGPL) |
| 8 | RAW | RAWDecoder.cpp | LibRaw 0.21+ | **REAL** | 100+ cameras; embedded preview fast path |
| 9 | ZIP/CBZ | ArchiveDecoder.cpp | minizip-ng 4.0+ | **REAL** | First image extraction |
| 10 | RAR/CBR | ArchiveDecoder.cpp | minizip-ng / UnRAR | **REAL** | First image extraction |
| 11 | 7Z/CB7 | ArchiveDecoder.cpp | LZMA SDK 26.00 | **REAL** | First image extraction |
| 12 | EPUB | EBookDecoder.cpp | minizip-ng | **REAL** | Cover image extraction |
| 13 | GIF | GIFAnimationDecoder.cpp | WIC + custom parser | **REAL** | First frame; frame counting |
| 14 | BMP | ImageDecoder.cpp | WIC | **REAL** | Standard WIC path |
| 15 | TIFF | ImageDecoder.cpp | WIC | **REAL** | Multi-page (first page) |
| 16 | SVG | SVGDecoder.cpp | GDI+ / Direct2D | **REAL** | SVGZ support; viewBox parsing |
| 17 | ICO | ICODecoder.cpp | WIC | **REAL** | .ico and .cur |
| 18 | PSD | PSDDecoder.cpp | Custom parser | **REAL** | Adobe spec composite extraction |
| 19 | EXR | EXRDecoder.cpp | WIC + optional tinyexr | **REAL** | Tone-mapping to sRGB |
| 20 | STL/OBJ/glTF | ModelDecoder.cpp | Custom + wireframe | **REAL** | 8 3D formats; orthographic projection |

## Additional Implemented Decoders

| Format | Decoder File | Library | Status |
|--------|-------------|---------|--------|
| DDS | DDSDecoder.cpp | DirectXTex | **REAL** |
| DICOM | DICOMDecoder.cpp | Custom + GDCM | **REAL** |
| FITS | FITSDecoder.cpp | Custom parser | **REAL** |
| Font (TTF/OTF) | FontDecoder.cpp | FreeType 2.13+ | **REAL** |
| HDR (Radiance) | HDRDecoder.cpp | Custom parser | **REAL** |
| JPEG 2000 | JPEG2000Decoder.cpp | OpenJPEG 2.5+ | **REAL** |
| KTX | KTXTextureDecoder.cpp | Custom parser | **REAL** |
| OpenRaster | OpenRasterDecoder.cpp | minizip-ng | **REAL** |
| PCX | PCXDecoder.cpp | Custom parser | **REAL** |
| PPM/PGM/PBM | PPMDecoder.cpp | Custom parser | **REAL** |
| Farbfeld | FarbfeldDecoder.cpp | Custom parser | **REAL** |
| EPS | EPSDecoder.cpp | Custom + GDI+ | **REAL** |
| Geospatial | GeospatialDecoder.cpp | Custom parser | **REAL** |
| Video | VideoDecoder.cpp | Media Foundation | **REAL** |
| Audio | AudioDecoder.cpp | Media Foundation + ID3 | **REAL** |

## Architecture Notes

- **WIC Foundation**: JPEG, PNG, BMP, GIF, TIFF, ICO use Windows Imaging Component (fast, built-in)
- **Conditional Compilation**: `HAS_LIBJXL`, `HAS_LIBHEIF`, `HAS_LIBAVIF`, `HAS_MUPDF`, `HAS_LIBRAW` gates
- **Archive Pattern**: minizip-ng reads archives, then re-invokes image decoders on extracted frames
- **Model Pattern**: ModelDecoder.cpp handles OBJ, STL, glTF, GLB, PLY, DAE with wireframe rendering

## Header-Only Stubs (No .cpp — Future Work)

These headers exist in `Engine/Decoders/` but have no corresponding .cpp implementation.
They will be implemented as demand arises or deleted during header audit.

- `ALembicDecoder.h` — Alembic 3D animation format
- `COLLADADecoder.h` — COLLADA 3D interchange
- `EPUBDecoder.h` — Superseded by EBookDecoder.cpp
- `GLTFModelDecoder.h` — Superseded by ModelDecoder.cpp
- `NerfDecoder.h` — Neural Radiance Field (aspirational)
- `OBJMeshDecoder.h` — Superseded by ModelDecoder.cpp
- `PLYPointCloudDecoder.h` — Superseded by ModelDecoder.cpp
- `STLMeshDecoder.h` — Superseded by ModelDecoder.cpp (has STL parser)
- `TIFFMultiPageDecoder.h` — Superseded by ImageDecoder.cpp (WIC handles TIFF)
- `USDDecoder.h` — Pixar USD (aspirational)
