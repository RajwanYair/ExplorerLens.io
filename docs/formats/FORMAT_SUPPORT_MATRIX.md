# Format Support Matrix — ExplorerLens v39.9.0

**Last Updated:** May 2026 
**Engine Version:** 39.9.0 "Betelgeuse" 
**Total Formats:** 200+ file extensions across 25 decoders 
**Shell Registrations:** 93 extensions in LENSShell.rgs 
**Decoder Compliance:** 25/25 (100%)

---

## Coverage Summary

| Category | Extensions | Decoders | Shell Registered | Status |
|----------|-----------|----------|-----------------|--------|
| **Standard Images** | 10 | ImageDecoder | No (Windows native) | ✅ Complete |
| **Modern Images** | 14 | WebP/AVIF/HEIF/JXL | 7 | ✅ Complete |
| **Professional Images** | 12 | PSD/DDS/HDR/EXR/TGA/ICO/QOI | 12 | ✅ Complete |
| **Vector Graphics** | 2 | SVGDecoder | 2 | ✅ Complete |
| **Netpbm** | 6 | PPMDecoder | 6 | ✅ Complete |
| **Camera RAW** | 27 | RAWDecoder | 25 | ✅ Complete |
| **Archives** | 22+ | ArchiveDecoder | 16 | ✅ Complete |
| **Video** | 22 | VideoDecoder | No (conflict risk) | ✅ Complete |
| **Audio** | 14 | AudioDecoder | No (conflict risk) | ✅ Complete |
| **Documents** | 19 | DocumentDecoder | 12 | ✅ Complete |
| **PDF** | 1 | PDFDecoder | 1 | ✅ Complete |
| **Fonts** | 7 | FontDecoder | 5 | ✅ Complete |
| **3D Models** | 8 | ModelDecoder | 8 | ✅ Complete |
| **eBooks** | 7 | DocumentDecoder | 7 | ✅ Complete |

---

## 1. Image Formats

### 1.1 Standard Images — ImageDecoder (WIC-based)

| Extension | Format Name | LENSTYPE | Performance | Status |
|-----------|------------|---------|-------------|--------|
| .jpg, .jpeg, .jpe, .jfif | JPEG | WIC | <5ms | ✅ |
| .png | PNG | WIC | <10ms | ✅ |
| .bmp, .dib | Windows Bitmap | LENSTYPE_BMP (76) | <5ms | ✅ |
| .gif | GIF (first frame) | LENSTYPE_GIF (77) | <8ms | ✅ |
| .tif, .tiff | TIFF | LENSTYPE_TIFF (45) | <15ms | ✅ |

### 1.2 Modern Image Formats

| Extension | Format Name | LENSTYPE | Decoder | Library | Performance | Status |
|-----------|------------|---------|---------|---------|-------------|--------|
| .webp | WebP | LENSTYPE_WEBP (40) | WebPDecoder | libwebp 1.5.0 | <20ms lossy, <30ms lossless | ✅ |
| .avif, .avifs | AV1 Image | LENSTYPE_AVIF (41) | AVIFDecoder | libavif 1.3.0 + dav1d 1.5.1 | <25ms HW, <80ms SW | ✅ |
| .heic | HEIC (Apple) | LENSTYPE_HEIC (42) | HEIFDecoder | libheif 1.19.5 + libde265 1.0.15 | <10ms embedded, <50ms full | ✅ |
| .heif | HEIF Container | LENSTYPE_HEIF (43) | HEIFDecoder | libheif 1.19.5 | <50ms | ✅ |
| .hif | HEIF (Sony/Hasselblad) | LENSTYPE_HEIC (42) | HEIFDecoder | libheif 1.19.5 | <50ms | ✅ |
| .avci, .avcs | AVC Intra HEIF | LENSTYPE_HEIC/HEIF | HEIFDecoder | libheif 1.19.5 | <50ms | ✅ |
| .jxl | JPEG XL | LENSTYPE_JXL (44) | JXLDecoder | libjxl 0.11.1 | <50ms simple, <100ms progressive | ✅ |

### 1.3 Professional Image Formats

| Extension | Format Name | LENSTYPE | Decoder | Status |
|-----------|------------|---------|---------|--------|
| .psd, .psb | Adobe Photoshop | LENSTYPE_PSD (48) | PSDDecoder | ✅ |
| .dds | DirectX Surface | LENSTYPE_DDS (49) | DDSDecoder | ✅ |
| .hdr | Radiance RGBE | LENSTYPE_HDR (55) | HDRDecoder | ✅ |
| .exr | OpenEXR | LENSTYPE_EXR (56) | EXRDecoder | ✅ |
| .tga | Targa | LENSTYPE_TGA (75) | TGADecoder | ✅ |
| .ico, .cur | Windows Icon/Cursor | LENSTYPE_ICO (58) | ICODecoder | ✅ |
| .qoi | Quite OK Image | LENSTYPE_QOI (59) | QOIDecoder | ✅ |

### 1.4 Vector Graphics — SVGDecoder

| Extension | Format Name | LENSTYPE | Status |
|-----------|------------|---------|--------|
| .svg | SVG | LENSTYPE_SVG (46) | ✅ |
| .svgz | Compressed SVG | LENSTYPE_SVG (46) | ✅ |

### 1.5 Netpbm Formats — PPMDecoder

| Extension | Format Name | LENSTYPE | Status |
|-----------|------------|---------|--------|
| .ppm | Portable Pixmap | LENSTYPE_PPM (57) | ✅ |
| .pgm | Portable Graymap | LENSTYPE_PPM (57) | ✅ |
| .pbm | Portable Bitmap | LENSTYPE_PPM (57) | ✅ |
| .pnm | Portable Anymap | LENSTYPE_PPM (57) | ✅ |
| .pam | Portable Arbitrary Map | LENSTYPE_PPM (57) | ✅ |
| .pfm | Portable Float Map | LENSTYPE_PPM (57) | ✅ |

---

## 2. Camera RAW Formats — RAWDecoder (LibRaw 0.21.3)

| Extension | Camera Brand | LENSTYPE | Status |
|-----------|-------------|---------|--------|
| .cr2 | Canon (older) | LENSTYPE_RAW (47) | ✅ |
| .cr3 | Canon (newer) | LENSTYPE_RAW (47) | ✅ |
| .crw | Canon (legacy) | LENSTYPE_RAW (47) | ✅ |
| .nef | Nikon | LENSTYPE_RAW (47) | ✅ |
| .nrw | Nikon (compact) | LENSTYPE_RAW (47) | ✅ |
| .arw | Sony Alpha | LENSTYPE_RAW (47) | ✅ |
| .srf | Sony (older) | LENSTYPE_RAW (47) | ✅ |
| .sr2 | Sony (v2) | LENSTYPE_RAW (47) | ✅ |
| .dng | Adobe DNG | LENSTYPE_RAW (47) | ✅ |
| .orf | Olympus | LENSTYPE_RAW (47) | ✅ |
| .rw2 | Panasonic | LENSTYPE_RAW (47) | ✅ |
| .raf | Fujifilm | LENSTYPE_RAW (47) | ✅ |
| .pef | Pentax | LENSTYPE_RAW (47) | ✅ |
| .dcr | Kodak | LENSTYPE_RAW (47) | ✅ |
| .mrw | Minolta | LENSTYPE_RAW (47) | ✅ |
| .x3f | Sigma (Foveon) | LENSTYPE_RAW (47) | ✅ |
| .srw | Samsung | LENSTYPE_RAW (47) | ✅ |
| .rwl | Leica | LENSTYPE_RAW (47) | ✅ |
| .3fr | Hasselblad | LENSTYPE_RAW (47) | ✅ |
| .iiq | Phase One | LENSTYPE_RAW (47) | ✅ |
| .erf | Epson | LENSTYPE_RAW (47) | ✅ |
| .kdc | Kodak DC | LENSTYPE_RAW (47) | ✅ |
| .mef | Mamiya | LENSTYPE_RAW (47) | ✅ |
| .gpr | GoPro | LENSTYPE_RAW (47) | ✅ |
| .raw | Generic RAW | LENSTYPE_RAW (47) | ✅ |
| .ptx | Pentax (older) | LENSTYPE_RAW (47) | ✅ |
| .r3d | RED Digital Cinema | LENSTYPE_RAW (47) | ✅ |

---

## 3. Archive & Compression Formats — ArchiveDecoder

| Extension | Format Name | LENSTYPE | Library | Status |
|-----------|------------|---------|---------|--------|
| .zip | ZIP Archive | LENSTYPE_ZIP (1) | minizip-ng 4.0.10 | ✅ |
| .cbz | Comic Book ZIP | LENSTYPE_CBZ (2) | minizip-ng | ✅ |
| .rar | RAR Archive | — | UnRAR 7.2.2 | ✅ |
| .cbr | Comic Book RAR | — | UnRAR | ✅ |
| .7z | 7-Zip Archive | LENSTYPE_7Z (6) | LZMA SDK 26.00 | ✅ |
| .cb7 | Comic Book 7z | LENSTYPE_CB7 (7) | LZMA SDK | ✅ |
| .tar | TAR Archive | LENSTYPE_TAR (8) | Built-in | ✅ |
| .cbt | Comic Book TAR | LENSTYPE_CBT (9) | Built-in | ✅ |
| .tgz, .tar.gz | Gzipped TAR | LENSTYPE_TAR_GZ (26) | zlib 1.3.1 | ✅ |
| .tar.bz2, .tbz | Bzipped TAR | LENSTYPE_TAR_BZ2 (27) | bzip2 | ✅ |
| .tar.xz, .txz | XZ TAR | LENSTYPE_TAR_XZ (28) | xz | ✅ |
| .tar.zst, .tzst | Zstd TAR | LENSTYPE_TAR_ZST (29) | zstd 1.5.7 | ✅ |
| .bz2 | Bzip2 | LENSTYPE_BZIP2 (15) | bzip2 | ✅ |
| .zst | Zstandard | LENSTYPE_ZSTD (16) | zstd 1.5.7 | ✅ |
| .xz | XZ | — | xz | ✅ |
| .iso | ISO Disc Image | LENSTYPE_ISO (50) | libarchive | ✅ |
| .cab | Windows Cabinet | LENSTYPE_CAB (54) | libarchive | ✅ |
| .cpio | CPIO Archive | LENSTYPE_CPIO (30) | libarchive | ✅ |
| .deb | Debian Package | LENSTYPE_DEB (53) | libarchive | ✅ |

---

## 4. Video Formats — VideoDecoder (Media Foundation + DirectShow Fallback)

Backend: Windows Media Foundation (primary), DirectShow fallback, DXVA2 hardware acceleration.

| Extension | Format | Status |
|-----------|--------|--------|
| .mp4 | MPEG-4 | ✅ |
| .mkv | Matroska | ✅ |
| .avi | AVI | ✅ |
| .wmv | Windows Media | ✅ |
| .mov | QuickTime | ✅ |
| .flv | Flash Video | ✅ |
| .webm | WebM | ✅ |
| .m4v | iTunes Video | ✅ |
| .mpg, .mpeg | MPEG-1/2 | ✅ |
| .ts, .m2ts, .mts | MPEG Transport Stream | ✅ |
| .3gp, .3g2 | 3GPP | ✅ |
| .vob | DVD Video | ✅ |
| .ogv | Ogg Video | ✅ |
| .asf | Advanced Streaming | ✅ |
| .divx | DivX | ✅ |
| .xvid | XviD | ✅ |
| .rm, .rmvb | RealMedia | ✅ |

**Note:** Video extensions are NOT registered in LENSShell.rgs to avoid conflicts with Windows native video thumbnail handlers.

---

## 5. Audio Formats — AudioDecoder (Media Foundation)

| Extension | Format | Status |
|-----------|--------|--------|
| .mp3 | MPEG Audio Layer 3 | ✅ |
| .flac | Free Lossless Audio | ✅ |
| .m4a | AAC Audio | ✅ |
| .ogg | Ogg Vorbis | ✅ |
| .wma | Windows Media Audio | ✅ |
| .wav | Waveform Audio | ✅ |
| .opus | Opus Audio | ✅ |
| .aiff, .aif | AIFF | ✅ |
| .ape | Monkey's Audio | ✅ |
| .wv | WavPack | ✅ |
| .mpc | Musepack | ✅ |
| .aac | Raw AAC | ✅ |
| .alac | Apple Lossless | ✅ |

Extracts embedded album art or generates waveform visualization.

**Note:** Audio extensions are NOT registered in LENSShell.rgs to avoid conflicts with media player handlers.

---

## 6. Document & eBook Formats — DocumentDecoder + PDFDecoder + EBookDecoder

### PDF

| Extension | Format | LENSTYPE | Status |
|-----------|--------|---------|--------|
| .pdf | PDF | LENSTYPE_PDF (19) | ✅ (MuPDF) |

### Office Documents

| Extension | Format | LENSTYPE | Status |
|-----------|--------|---------|--------|
| .docx | Word 2007+ | LENSTYPE_DOCX (60) | ✅ |
| .doc | Word Legacy | LENSTYPE_DOC (63) | ✅ |
| .pptx | PowerPoint 2007+ | LENSTYPE_PPTX (61) | ✅ |
| .ppt | PowerPoint Legacy | LENSTYPE_PPT (64) | ✅ |
| .xlsx | Excel 2007+ | LENSTYPE_XLSX (62) | ✅ |
| .xls | Excel Legacy | LENSTYPE_XLS (65) | ✅ |
| .rtf | Rich Text Format | — | ✅ |

### OpenDocument Formats

| Extension | Format | LENSTYPE | Status |
|-----------|--------|---------|--------|
| .odt | OpenDocument Text | LENSTYPE_ODT (24) | ✅ |
| .odp | OpenDocument Presentation | LENSTYPE_ODP (25) | ✅ |
| .ods | OpenDocument Spreadsheet | — | ✅ |
| .xps | XML Paper Specification | — | ✅ |

### eBook Formats

| Extension | Format | LENSTYPE | Status |
|-----------|--------|---------|--------|
| .epub | EPUB | LENSTYPE_EPUB (5) | ✅ |
| .mobi | Mobipocket | LENSTYPE_MOBI (10) | ✅ |
| .azw | Kindle | LENSTYPE_AZW (12) | ✅ |
| .azw3 | Kindle Format 8 | LENSTYPE_AZW3 (13) | ✅ |
| .fb2 | FictionBook 2 | LENSTYPE_FB2 (11) | ✅ |
| .djvu, .djv | DjVu | LENSTYPE_DJVU (22) | ✅ |
| .phz | PHZ eBook | LENSTYPE_PHZ (14) | ✅ |

---

## 7. Font Formats — FontDecoder

| Extension | Format | LENSTYPE | Status |
|-----------|--------|---------|--------|
| .ttf | TrueType Font | LENSTYPE_FONT (70) | ✅ |
| .otf | OpenType Font | LENSTYPE_FONT (70) | ✅ |
| .woff | Web Open Font Format | LENSTYPE_FONT (70) | ✅ |
| .woff2 | WOFF2 | LENSTYPE_FONT (70) | ✅ |
| .ttc | TrueType Collection | LENSTYPE_FONT (70) | ✅ |

---

## 8. 3D Model Formats — ModelDecoder

| Extension | Format | LENSTYPE | Status |
|-----------|--------|---------|--------|
| .obj | Wavefront OBJ | LENSTYPE_MODEL (80) | ✅ |
| .stl | Stereolithography | LENSTYPE_MODEL (80) | ✅ |
| .gltf | GL Transmission Format | LENSTYPE_MODEL (80) | ✅ |
| .glb | GL Binary | LENSTYPE_MODEL (80) | ✅ |
| .fbx | Autodesk FBX | LENSTYPE_MODEL (80) | ✅ |
| .3ds | 3D Studio | LENSTYPE_MODEL (80) | ✅ |
| .dae | COLLADA | LENSTYPE_MODEL (80) | ✅ |
| .ply | Stanford PLY | LENSTYPE_MODEL (80) | ✅ |

Rendered via DirectX 11 viewport with wireframe fallback.

---

## 9. Decoder Compliance Audit

All 25 decoders implement the `IThumbnailDecoder` interface. Each must provide:

- `CanDecode()` — Format detection
- `Decode()` — Decoding logic 
- `GetInfo()` — Capability reporting
- `GetName()` — Decoder identification
- `GetSupportedExtensions()` / `GetExtensionCount()` — Extension enumeration
- `SupportsGPU()` — GPU capability flag
- `IsArchiveDecoder()` — Archive type flag

### Compliance Matrix

| # | Decoder | Extensions | GPU | Archive | Status |
|---|---------|-----------|-----|---------|--------|
| 1 | ImageDecoder (WIC) | 10 | ✅ | — | ✅ |
| 2 | WebPDecoder | 1 | ✅ | — | ✅ |
| 3 | AVIFDecoder | 2 | ✅ | — | ✅ |
| 4 | HEIFDecoder | 10 | ✅ | — | ✅ |
| 5 | JXLDecoder | 1 | ✅ | — | ✅ |
| 6 | PSDDecoder | 2 | ✅ | — | ✅ |
| 7 | DDSDecoder | 1 | ✅ | — | ✅ |
| 8 | HDRDecoder | 1 | ✅ | — | ✅ |
| 9 | EXRDecoder | 1 | ✅ | — | ✅ |
| 10 | TGADecoder | 1 | ✅ | — | ✅ |
| 11 | ICODecoder | 2 | ✅ | — | ✅ |
| 12 | QOIDecoder | 1 | ✅ | — | ✅ |
| 13 | SVGDecoder | 2 | — | — | ✅ |
| 14 | PPMDecoder | 6 | — | — | ✅ |
| 15 | RAWDecoder | 27 | ✅ | — | ✅ |
| 16 | ArchiveDecoder | 22+ | — | ✅ | ✅ |
| 17 | VideoDecoder | 22 | ✅ | — | ✅ |
| 18 | AudioDecoder | 14 | — | — | ✅ |
| 19 | DocumentDecoder | 19 | — | — | ✅ |
| 20 | PDFDecoder | 1 | ✅ | — | ✅ |
| 21 | EBookDecoder | 7 | — | — | ✅ |
| 22 | FontDecoder | 5 | — | — | ✅ |
| 23 | ModelDecoder | 8 | ✅ | — | ✅ |
| 24 | CADDecoder | plugin | ✅ | — | ✅ |
| 25 | ScientificDecoder | plugin | — | — | ✅ |

**GPU-Capable:** 16 decoders | **Plugin-Based:** 2 decoders

---

## 10. LENSShell Legacy Decoders

The Shell extension DLL includes dedicated decoders that do NOT use the `IThumbnailDecoder` interface:

| Decoder | Extensions | Library | Notes |
|---------|-----------|---------|-------|
| AVIFDecoder | .avif | WIC | Separate from Engine AVIFDecoder |
| WebPDecoder | .webp | libwebp | Separate from Engine WebPDecoder |
| JXLDecoder | .jxl | libjxl | Requires libjxl always (no #ifdef) |
| HEIFDecoderNative | .heif .heic | WIC | Uses Windows HEIF codec |
| RAWDecoder | .dng .cr2 .cr3 .crw .nef .nrw .arw + more | WIC | Relies on Camera Codec Pack |
| VideoThumbnail | .mp4 .avi .mkv .wmv etc. | DirectShow | ISampleGrabber frame capture |
| AudioThumbnail | .mp3 .m4a .flac .ogg .wav .opus | Native | Album art + waveform |
| DocumentThumbnail | .docx .doc .xlsx .xls .pptx .ppt .txt .rtf .xps | COM/WIC | Multiple fallback strategies |
| FontPreview | .ttf .otf .woff .woff2 | DirectWrite | GDI fallback available |
| SVGDecoder | .svg | — | Shell stub, Engine has full decoder |
| PDFDecoder | .pdf | — | Shell stub, Engine uses MuPDF |

---

## 11. LENSTYPE Enum Reference

| Value | Constant | Category |
|-------|----------|----------|
| 0 | LENSTYPE_NONE | None |
| 1 | LENSTYPE_ZIP | Archive |
| 2 | LENSTYPE_CBZ | Archive |
| 5 | LENSTYPE_EPUB | eBook |
| 6 | LENSTYPE_7Z | Archive |
| 7 | LENSTYPE_CB7 | Archive |
| 8 | LENSTYPE_TAR | Archive |
| 9 | LENSTYPE_CBT | Archive |
| 10 | LENSTYPE_MOBI | eBook |
| 11 | LENSTYPE_FB2 | eBook |
| 12 | LENSTYPE_AZW | eBook |
| 13 | LENSTYPE_AZW3 | eBook |
| 14 | LENSTYPE_PHZ | eBook |
| 15 | LENSTYPE_BZIP2 | Compression |
| 16 | LENSTYPE_ZSTD | Compression |
| 17 | LENSTYPE_LZMA | Compression |
| 18 | LENSTYPE_VIDEO | Video |
| 19 | LENSTYPE_PDF | Document |
| 20 | LENSTYPE_AUDIO | Audio |
| 21 | LENSTYPE_LZ4 | Compression |
| 22 | LENSTYPE_DJVU | eBook |
| 23 | LENSTYPE_CHM | Document |
| 24 | LENSTYPE_ODT | Document |
| 25 | LENSTYPE_ODP | Document |
| 26 | LENSTYPE_TAR_GZ | Archive |
| 27 | LENSTYPE_TAR_BZ2 | Archive |
| 28 | LENSTYPE_TAR_XZ | Archive |
| 29 | LENSTYPE_TAR_ZST | Archive |
| 30 | LENSTYPE_CPIO | Archive |
| 40 | LENSTYPE_WEBP | Image |
| 41 | LENSTYPE_AVIF | Image |
| 42 | LENSTYPE_HEIC | Image |
| 43 | LENSTYPE_HEIF | Image |
| 44 | LENSTYPE_JXL | Image |
| 45 | LENSTYPE_TIFF | Image |
| 46 | LENSTYPE_SVG | Image |
| 47 | LENSTYPE_RAW | Image |
| 48 | LENSTYPE_PSD | Image |
| 49 | LENSTYPE_DDS | Image |
| 50 | LENSTYPE_ISO | Archive |
| 51 | LENSTYPE_XAR | Archive |
| 52 | LENSTYPE_AR | Archive |
| 53 | LENSTYPE_DEB | Archive |
| 54 | LENSTYPE_CAB | Archive |
| 55 | LENSTYPE_HDR | Image |
| 56 | LENSTYPE_EXR | Image |
| 57 | LENSTYPE_PPM | Image |
| 58 | LENSTYPE_ICO | Image |
| 59 | LENSTYPE_QOI | Image |
| 60 | LENSTYPE_DOCX | Document |
| 61 | LENSTYPE_PPTX | Document |
| 62 | LENSTYPE_XLSX | Document |
| 63 | LENSTYPE_DOC | Document |
| 64 | LENSTYPE_PPT | Document |
| 65 | LENSTYPE_XLS | Document |
| 70 | LENSTYPE_FONT | Font |
| 75 | LENSTYPE_TGA | Image |
| 76 | LENSTYPE_BMP | Image |
| 77 | LENSTYPE_GIF | Image |
| 80 | LENSTYPE_MODEL | 3D Model |
| 81 | LENSTYPE_DOCUMENT | Document |

---

## 12. Shell Registration

**Registered (93 extensions):** All extensions that trigger ExplorerLens thumbnail generation in Windows Explorer via `regsvr32`.

**Not registered (by design):**
- Standard images (.jpg, .png, .bmp, .gif, .tiff) — Windows has native handlers
- Video formats — Potential conflict with Windows Media handlers
- Audio formats — Potential conflict with media player handlers

---

## 13. Compile Flags

Set in `Engine/CMakeLists.txt`:

```cmake
option(HAS_LIBJXL "Enable JPEG XL support via libjxl" ON)
option(HAS_LIBHEIF "Enable HEIF/HEIC support via libheif" ON)
option(HAS_LIBRAW "Enable Camera RAW support via LibRaw" ON)
option(HAS_LIBAVIF "Enable AVIF support via libavif" ON)
option(ENABLE_VIDEO_DECODER "Enable Video thumbnail support" ON)
option(ENABLE_AUDIO_DECODER "Enable Audio thumbnail support" ON)
```

---

## 14. Performance Summary

### Decode Time Ranges (256×256 thumbnail)

| Tier | Time | Formats |
|------|------|---------|
| Ultra Fast | <10ms | JPEG, BMP, PNG (cached), HEIF (embedded thumbnail) |
| Fast | 10–30ms | PNG, GIF, WebP (lossy), AVIF (HW), Fonts |
| Good | 30–80ms | WebP (lossless), JXL, Video (HW), Audio, Text |
| Moderate | 80–200ms | 7Z, RAR, PDF, Video (SW), AVIF (SW) |

### Performance Targets

- Single thumbnail: 17ms 
- Batch throughput: 235 img/sec 
- Cache hit: <5ms

---

*This document consolidates FORMAT_SUPPORT_MATRIX_V8.md, DECODER_STATUS.md, DECODER_AUDIT_REPORT.md, and CAPABILITY_AUDIT.md.* 
*Synchronized with LENSArchive.h LENSTYPE enum and LENSShell.rgs.*
