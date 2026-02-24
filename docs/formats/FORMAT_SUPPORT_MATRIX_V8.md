# Format Support Matrix - ExplorerLens v8.4.0

**Last Updated:** June 2025  
**Engine Version:** 8.4.0 (Sprint 178)  
**Total Formats:** 200+ file extensions across 25 decoders  
**Shell Registrations:** 93 extensions in LENSShell.rgs

## Executive Summary

ExplorerLens v8.4.0 includes **25 specialized decoders** supporting **200+ file extensions** across images, archives, video, audio, documents, fonts, and 3D models.

### Coverage by Category

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

| Extension | Format Name | LENSTYPE | Decoder | Status |
|-----------|------------|---------|---------|--------|
| .jpg, .jpeg, .jpe, .jfif | JPEG | WIC | ImageDecoder | ✅ |
| .png | PNG | WIC | ImageDecoder | ✅ |
| .bmp, .dib | Windows Bitmap | LENSTYPE_BMP (76) | ImageDecoder | ✅ |
| .gif | GIF (first frame) | LENSTYPE_GIF (77) | ImageDecoder | ✅ |
| .tif, .tiff | TIFF | LENSTYPE_TIFF (45) | ImageDecoder | ✅ |

### 1.2 Modern Image Formats

| Extension | Format Name | LENSTYPE | Decoder | Library | Status |
|-----------|------------|---------|---------|---------|--------|
| .webp | WebP | LENSTYPE_WEBP (40) | WebPDecoder | libwebp 1.5.0 | ✅ |
| .avif | AV1 Image | LENSTYPE_AVIF (41) | AVIFDecoder | libavif 1.3.0 + dav1d 1.5.1 | ✅ |
| .avifs | AVIF Sequence | LENSTYPE_AVIF (41) | AVIFDecoder | libavif 1.3.0 | ✅ |
| .heic | HEIC (Apple) | LENSTYPE_HEIC (42) | HEIFDecoder | libheif 1.19.5 + libde265 1.0.15 | ✅ |
| .heif | HEIF Container | LENSTYPE_HEIF (43) | HEIFDecoder | libheif 1.19.5 | ✅ |
| .hif | HEIF (Sony/Hasselblad) | LENSTYPE_HEIC (42) | HEIFDecoder | libheif 1.19.5 | ✅ |
| .avci, .avcs | AVC Intra HEIF | LENSTYPE_HEIC/HEIF | HEIFDecoder | libheif 1.19.5 | ✅ |
| .jxl | JPEG XL | LENSTYPE_JXL (44) | JXLDecoder | libjxl 0.11.1 | ✅ |

### 1.3 Professional Image Formats

| Extension | Format Name | LENSTYPE | Decoder | Status |
|-----------|------------|---------|---------|--------|
| .psd, .psb | Adobe Photoshop | LENSTYPE_PSD (48) | PSDDecoder | ✅ |
| .dds | DirectX Surface | LENSTYPE_DDS (49) | DDSDecoder | ✅ |
| .hdr | Radiance RGBE | LENSTYPE_HDR (55) | HDRDecoder | ✅ |
| .exr | OpenEXR | LENSTYPE_EXR (56) | EXRDecoder | ✅ |
| .tga | Targa | LENSTYPE_TGA (75) | TGADecoder | ✅ |
| .ico | Windows Icon | LENSTYPE_ICO (58) | ICODecoder | ✅ |
| .cur | Windows Cursor | LENSTYPE_ICO (58) | ICODecoder | ✅ |
| .qoi | Quite OK Image | LENSTYPE_QOI (59) | QOIDecoder | ✅ |

### 1.4 Vector Graphics

| Extension | Format Name | LENSTYPE | Decoder | Status |
|-----------|------------|---------|---------|--------|
| .svg | SVG | LENSTYPE_SVG (46) | SVGDecoder | ✅ |
| .svgz | Compressed SVG | LENSTYPE_SVG (46) | SVGDecoder | ✅ |

### 1.5 Netpbm Formats

| Extension | Format Name | LENSTYPE | Decoder | Status |
|-----------|------------|---------|---------|--------|
| .ppm | Portable Pixmap | LENSTYPE_PPM (57) | PPMDecoder | ✅ |
| .pgm | Portable Graymap | LENSTYPE_PPM (57) | PPMDecoder | ✅ |
| .pbm | Portable Bitmap | LENSTYPE_PPM (57) | PPMDecoder | ✅ |
| .pnm | Portable Anymap | LENSTYPE_PPM (57) | PPMDecoder | ✅ |
| .pam | Portable Arbitrary Map | LENSTYPE_PPM (57) | PPMDecoder | ✅ |
| .pfm | Portable Float Map | LENSTYPE_PPM (57) | PPMDecoder | ✅ |

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

## 4. Video Formats — VideoDecoder (Media Foundation)

| Extension | Format | Status | Note |
|-----------|--------|--------|------|
| .mp4 | MPEG-4 | ✅ | |
| .mkv | Matroska | ✅ | |
| .avi | AVI | ✅ | |
| .wmv | Windows Media | ✅ | |
| .mov | QuickTime | ✅ | |
| .flv | Flash Video | ✅ | |
| .webm | WebM | ✅ | |
| .m4v | iTunes Video | ✅ | |
| .mpg, .mpeg | MPEG-1/2 | ✅ | |
| .ts, .m2ts, .mts | MPEG Transport Stream | ✅ | |
| .3gp, .3g2 | 3GPP | ✅ | |
| .vob | DVD Video | ✅ | |
| .ogv | Ogg Video | ✅ | |
| .asf | Advanced Streaming | ✅ | |
| .divx | DivX | ✅ | |

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

**Note:** Audio extensions are NOT registered in LENSShell.rgs. Extracts album art or generates waveform visualization.

---

## 6. Document Formats — DocumentDecoder + PDFDecoder

### PDF

| Extension | Format | LENSTYPE | Status |
|-----------|--------|---------|--------|
| .pdf | PDF | LENSTYPE_PDF (19) | ✅ (via MuPDF) |

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

---

## 9. LENSTYPE Enum Reference

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

## 10. Shell Registration Status

**Registered (93):** All extensions that trigger ExplorerLens thumbnail generation in Windows Explorer.

**Not registered (by design):**
- Standard images (.jpg, .png, .bmp, .gif, .tiff) — Windows has native handlers
- Video formats — Potential conflict with Windows Media handlers
- Audio formats — Potential conflict with media player handlers

---

*This document supersedes FORMAT_SUPPORT_MATRIX_V7.md*  
*Auto-synchronized with lensArchive.h LENSTYPE enum and LENSShell.rgs*

