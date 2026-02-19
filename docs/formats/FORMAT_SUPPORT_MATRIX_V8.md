# Format Support Matrix - DarkThumbs v8.4.0

**Last Updated:** June 2025  
**Engine Version:** 8.4.0 (Sprint 178)  
**Total Formats:** 200+ file extensions across 25 decoders  
**Shell Registrations:** 93 extensions in CBXShell.rgs

## Executive Summary

DarkThumbs v8.4.0 includes **25 specialized decoders** supporting **200+ file extensions** across images, archives, video, audio, documents, fonts, and 3D models.

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

| Extension | Format Name | CBXTYPE | Decoder | Status |
|-----------|------------|---------|---------|--------|
| .jpg, .jpeg, .jpe, .jfif | JPEG | WIC | ImageDecoder | ✅ |
| .png | PNG | WIC | ImageDecoder | ✅ |
| .bmp, .dib | Windows Bitmap | CBXTYPE_BMP (76) | ImageDecoder | ✅ |
| .gif | GIF (first frame) | CBXTYPE_GIF (77) | ImageDecoder | ✅ |
| .tif, .tiff | TIFF | CBXTYPE_TIFF (45) | ImageDecoder | ✅ |

### 1.2 Modern Image Formats

| Extension | Format Name | CBXTYPE | Decoder | Library | Status |
|-----------|------------|---------|---------|---------|--------|
| .webp | WebP | CBXTYPE_WEBP (40) | WebPDecoder | libwebp 1.5.0 | ✅ |
| .avif | AV1 Image | CBXTYPE_AVIF (41) | AVIFDecoder | libavif 1.3.0 + dav1d 1.5.1 | ✅ |
| .avifs | AVIF Sequence | CBXTYPE_AVIF (41) | AVIFDecoder | libavif 1.3.0 | ✅ |
| .heic | HEIC (Apple) | CBXTYPE_HEIC (42) | HEIFDecoder | libheif 1.19.5 + libde265 1.0.15 | ✅ |
| .heif | HEIF Container | CBXTYPE_HEIF (43) | HEIFDecoder | libheif 1.19.5 | ✅ |
| .hif | HEIF (Sony/Hasselblad) | CBXTYPE_HEIC (42) | HEIFDecoder | libheif 1.19.5 | ✅ |
| .avci, .avcs | AVC Intra HEIF | CBXTYPE_HEIC/HEIF | HEIFDecoder | libheif 1.19.5 | ✅ |
| .jxl | JPEG XL | CBXTYPE_JXL (44) | JXLDecoder | libjxl 0.11.1 | ✅ |

### 1.3 Professional Image Formats

| Extension | Format Name | CBXTYPE | Decoder | Status |
|-----------|------------|---------|---------|--------|
| .psd, .psb | Adobe Photoshop | CBXTYPE_PSD (48) | PSDDecoder | ✅ |
| .dds | DirectX Surface | CBXTYPE_DDS (49) | DDSDecoder | ✅ |
| .hdr | Radiance RGBE | CBXTYPE_HDR (55) | HDRDecoder | ✅ |
| .exr | OpenEXR | CBXTYPE_EXR (56) | EXRDecoder | ✅ |
| .tga | Targa | CBXTYPE_TGA (75) | TGADecoder | ✅ |
| .ico | Windows Icon | CBXTYPE_ICO (58) | ICODecoder | ✅ |
| .cur | Windows Cursor | CBXTYPE_ICO (58) | ICODecoder | ✅ |
| .qoi | Quite OK Image | CBXTYPE_QOI (59) | QOIDecoder | ✅ |

### 1.4 Vector Graphics

| Extension | Format Name | CBXTYPE | Decoder | Status |
|-----------|------------|---------|---------|--------|
| .svg | SVG | CBXTYPE_SVG (46) | SVGDecoder | ✅ |
| .svgz | Compressed SVG | CBXTYPE_SVG (46) | SVGDecoder | ✅ |

### 1.5 Netpbm Formats

| Extension | Format Name | CBXTYPE | Decoder | Status |
|-----------|------------|---------|---------|--------|
| .ppm | Portable Pixmap | CBXTYPE_PPM (57) | PPMDecoder | ✅ |
| .pgm | Portable Graymap | CBXTYPE_PPM (57) | PPMDecoder | ✅ |
| .pbm | Portable Bitmap | CBXTYPE_PPM (57) | PPMDecoder | ✅ |
| .pnm | Portable Anymap | CBXTYPE_PPM (57) | PPMDecoder | ✅ |
| .pam | Portable Arbitrary Map | CBXTYPE_PPM (57) | PPMDecoder | ✅ |
| .pfm | Portable Float Map | CBXTYPE_PPM (57) | PPMDecoder | ✅ |

---

## 2. Camera RAW Formats — RAWDecoder (LibRaw 0.21.3)

| Extension | Camera Brand | CBXTYPE | Status |
|-----------|-------------|---------|--------|
| .cr2 | Canon (older) | CBXTYPE_RAW (47) | ✅ |
| .cr3 | Canon (newer) | CBXTYPE_RAW (47) | ✅ |
| .crw | Canon (legacy) | CBXTYPE_RAW (47) | ✅ |
| .nef | Nikon | CBXTYPE_RAW (47) | ✅ |
| .nrw | Nikon (compact) | CBXTYPE_RAW (47) | ✅ |
| .arw | Sony Alpha | CBXTYPE_RAW (47) | ✅ |
| .srf | Sony (older) | CBXTYPE_RAW (47) | ✅ |
| .sr2 | Sony (v2) | CBXTYPE_RAW (47) | ✅ |
| .dng | Adobe DNG | CBXTYPE_RAW (47) | ✅ |
| .orf | Olympus | CBXTYPE_RAW (47) | ✅ |
| .rw2 | Panasonic | CBXTYPE_RAW (47) | ✅ |
| .raf | Fujifilm | CBXTYPE_RAW (47) | ✅ |
| .pef | Pentax | CBXTYPE_RAW (47) | ✅ |
| .dcr | Kodak | CBXTYPE_RAW (47) | ✅ |
| .mrw | Minolta | CBXTYPE_RAW (47) | ✅ |
| .x3f | Sigma (Foveon) | CBXTYPE_RAW (47) | ✅ |
| .srw | Samsung | CBXTYPE_RAW (47) | ✅ |
| .rwl | Leica | CBXTYPE_RAW (47) | ✅ |
| .3fr | Hasselblad | CBXTYPE_RAW (47) | ✅ |
| .iiq | Phase One | CBXTYPE_RAW (47) | ✅ |
| .erf | Epson | CBXTYPE_RAW (47) | ✅ |
| .kdc | Kodak DC | CBXTYPE_RAW (47) | ✅ |
| .mef | Mamiya | CBXTYPE_RAW (47) | ✅ |
| .gpr | GoPro | CBXTYPE_RAW (47) | ✅ |
| .raw | Generic RAW | CBXTYPE_RAW (47) | ✅ |
| .ptx | Pentax (older) | CBXTYPE_RAW (47) | ✅ |
| .r3d | RED Digital Cinema | CBXTYPE_RAW (47) | ✅ |

---

## 3. Archive & Compression Formats — ArchiveDecoder

| Extension | Format Name | CBXTYPE | Library | Status |
|-----------|------------|---------|---------|--------|
| .zip | ZIP Archive | CBXTYPE_ZIP (1) | minizip-ng 4.0.10 | ✅ |
| .cbz | Comic Book ZIP | CBXTYPE_CBZ (2) | minizip-ng | ✅ |
| .rar | RAR Archive | — | UnRAR 7.2.2 | ✅ |
| .cbr | Comic Book RAR | — | UnRAR | ✅ |
| .7z | 7-Zip Archive | CBXTYPE_7Z (6) | LZMA SDK 26.00 | ✅ |
| .cb7 | Comic Book 7z | CBXTYPE_CB7 (7) | LZMA SDK | ✅ |
| .tar | TAR Archive | CBXTYPE_TAR (8) | Built-in | ✅ |
| .cbt | Comic Book TAR | CBXTYPE_CBT (9) | Built-in | ✅ |
| .tgz, .tar.gz | Gzipped TAR | CBXTYPE_TAR_GZ (26) | zlib 1.3.1 | ✅ |
| .tar.bz2, .tbz | Bzipped TAR | CBXTYPE_TAR_BZ2 (27) | bzip2 | ✅ |
| .tar.xz, .txz | XZ TAR | CBXTYPE_TAR_XZ (28) | xz | ✅ |
| .tar.zst, .tzst | Zstd TAR | CBXTYPE_TAR_ZST (29) | zstd 1.5.7 | ✅ |
| .bz2 | Bzip2 | CBXTYPE_BZIP2 (15) | bzip2 | ✅ |
| .zst | Zstandard | CBXTYPE_ZSTD (16) | zstd 1.5.7 | ✅ |
| .xz | XZ | — | xz | ✅ |
| .iso | ISO Disc Image | CBXTYPE_ISO (50) | libarchive | ✅ |
| .cab | Windows Cabinet | CBXTYPE_CAB (54) | libarchive | ✅ |
| .cpio | CPIO Archive | CBXTYPE_CPIO (30) | libarchive | ✅ |
| .deb | Debian Package | CBXTYPE_DEB (53) | libarchive | ✅ |

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

**Note:** Video extensions are NOT registered in CBXShell.rgs to avoid conflicts with Windows native video thumbnail handlers.

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

**Note:** Audio extensions are NOT registered in CBXShell.rgs. Extracts album art or generates waveform visualization.

---

## 6. Document Formats — DocumentDecoder + PDFDecoder

### PDF

| Extension | Format | CBXTYPE | Status |
|-----------|--------|---------|--------|
| .pdf | PDF | CBXTYPE_PDF (19) | ✅ (via MuPDF) |

### Office Documents

| Extension | Format | CBXTYPE | Status |
|-----------|--------|---------|--------|
| .docx | Word 2007+ | CBXTYPE_DOCX (60) | ✅ |
| .doc | Word Legacy | CBXTYPE_DOC (63) | ✅ |
| .pptx | PowerPoint 2007+ | CBXTYPE_PPTX (61) | ✅ |
| .ppt | PowerPoint Legacy | CBXTYPE_PPT (64) | ✅ |
| .xlsx | Excel 2007+ | CBXTYPE_XLSX (62) | ✅ |
| .xls | Excel Legacy | CBXTYPE_XLS (65) | ✅ |
| .rtf | Rich Text Format | — | ✅ |

### OpenDocument Formats

| Extension | Format | CBXTYPE | Status |
|-----------|--------|---------|--------|
| .odt | OpenDocument Text | CBXTYPE_ODT (24) | ✅ |
| .odp | OpenDocument Presentation | CBXTYPE_ODP (25) | ✅ |
| .ods | OpenDocument Spreadsheet | — | ✅ |
| .xps | XML Paper Specification | — | ✅ |

### eBook Formats

| Extension | Format | CBXTYPE | Status |
|-----------|--------|---------|--------|
| .epub | EPUB | CBXTYPE_EPUB (5) | ✅ |
| .mobi | Mobipocket | CBXTYPE_MOBI (10) | ✅ |
| .azw | Kindle | CBXTYPE_AZW (12) | ✅ |
| .azw3 | Kindle Format 8 | CBXTYPE_AZW3 (13) | ✅ |
| .fb2 | FictionBook 2 | CBXTYPE_FB2 (11) | ✅ |
| .djvu, .djv | DjVu | CBXTYPE_DJVU (22) | ✅ |
| .phz | PHZ eBook | CBXTYPE_PHZ (14) | ✅ |

---

## 7. Font Formats — FontDecoder

| Extension | Format | CBXTYPE | Status |
|-----------|--------|---------|--------|
| .ttf | TrueType Font | CBXTYPE_FONT (70) | ✅ |
| .otf | OpenType Font | CBXTYPE_FONT (70) | ✅ |
| .woff | Web Open Font Format | CBXTYPE_FONT (70) | ✅ |
| .woff2 | WOFF2 | CBXTYPE_FONT (70) | ✅ |
| .ttc | TrueType Collection | CBXTYPE_FONT (70) | ✅ |

---

## 8. 3D Model Formats — ModelDecoder

| Extension | Format | CBXTYPE | Status |
|-----------|--------|---------|--------|
| .obj | Wavefront OBJ | CBXTYPE_MODEL (80) | ✅ |
| .stl | Stereolithography | CBXTYPE_MODEL (80) | ✅ |
| .gltf | GL Transmission Format | CBXTYPE_MODEL (80) | ✅ |
| .glb | GL Binary | CBXTYPE_MODEL (80) | ✅ |
| .fbx | Autodesk FBX | CBXTYPE_MODEL (80) | ✅ |
| .3ds | 3D Studio | CBXTYPE_MODEL (80) | ✅ |
| .dae | COLLADA | CBXTYPE_MODEL (80) | ✅ |
| .ply | Stanford PLY | CBXTYPE_MODEL (80) | ✅ |

---

## 9. CBXTYPE Enum Reference

| Value | Constant | Category |
|-------|----------|----------|
| 0 | CBXTYPE_NONE | None |
| 1 | CBXTYPE_ZIP | Archive |
| 2 | CBXTYPE_CBZ | Archive |
| 5 | CBXTYPE_EPUB | eBook |
| 6 | CBXTYPE_7Z | Archive |
| 7 | CBXTYPE_CB7 | Archive |
| 8 | CBXTYPE_TAR | Archive |
| 9 | CBXTYPE_CBT | Archive |
| 10 | CBXTYPE_MOBI | eBook |
| 11 | CBXTYPE_FB2 | eBook |
| 12 | CBXTYPE_AZW | eBook |
| 13 | CBXTYPE_AZW3 | eBook |
| 14 | CBXTYPE_PHZ | eBook |
| 15 | CBXTYPE_BZIP2 | Compression |
| 16 | CBXTYPE_ZSTD | Compression |
| 17 | CBXTYPE_LZMA | Compression |
| 18 | CBXTYPE_VIDEO | Video |
| 19 | CBXTYPE_PDF | Document |
| 20 | CBXTYPE_AUDIO | Audio |
| 21 | CBXTYPE_LZ4 | Compression |
| 22 | CBXTYPE_DJVU | eBook |
| 23 | CBXTYPE_CHM | Document |
| 24 | CBXTYPE_ODT | Document |
| 25 | CBXTYPE_ODP | Document |
| 26 | CBXTYPE_TAR_GZ | Archive |
| 27 | CBXTYPE_TAR_BZ2 | Archive |
| 28 | CBXTYPE_TAR_XZ | Archive |
| 29 | CBXTYPE_TAR_ZST | Archive |
| 30 | CBXTYPE_CPIO | Archive |
| 40 | CBXTYPE_WEBP | Image |
| 41 | CBXTYPE_AVIF | Image |
| 42 | CBXTYPE_HEIC | Image |
| 43 | CBXTYPE_HEIF | Image |
| 44 | CBXTYPE_JXL | Image |
| 45 | CBXTYPE_TIFF | Image |
| 46 | CBXTYPE_SVG | Image |
| 47 | CBXTYPE_RAW | Image |
| 48 | CBXTYPE_PSD | Image |
| 49 | CBXTYPE_DDS | Image |
| 50 | CBXTYPE_ISO | Archive |
| 51 | CBXTYPE_XAR | Archive |
| 52 | CBXTYPE_AR | Archive |
| 53 | CBXTYPE_DEB | Archive |
| 54 | CBXTYPE_CAB | Archive |
| 55 | CBXTYPE_HDR | Image |
| 56 | CBXTYPE_EXR | Image |
| 57 | CBXTYPE_PPM | Image |
| 58 | CBXTYPE_ICO | Image |
| 59 | CBXTYPE_QOI | Image |
| 60 | CBXTYPE_DOCX | Document |
| 61 | CBXTYPE_PPTX | Document |
| 62 | CBXTYPE_XLSX | Document |
| 63 | CBXTYPE_DOC | Document |
| 64 | CBXTYPE_PPT | Document |
| 65 | CBXTYPE_XLS | Document |
| 70 | CBXTYPE_FONT | Font |
| 75 | CBXTYPE_TGA | Image |
| 76 | CBXTYPE_BMP | Image |
| 77 | CBXTYPE_GIF | Image |
| 80 | CBXTYPE_MODEL | 3D Model |
| 81 | CBXTYPE_DOCUMENT | Document |

---

## 10. Shell Registration Status

**Registered (93):** All extensions that trigger DarkThumbs thumbnail generation in Windows Explorer.

**Not registered (by design):**
- Standard images (.jpg, .png, .bmp, .gif, .tiff) — Windows has native handlers
- Video formats — Potential conflict with Windows Media handlers
- Audio formats — Potential conflict with media player handlers

---

*This document supersedes FORMAT_SUPPORT_MATRIX_V7.md*  
*Auto-synchronized with cbxArchive.h CBXTYPE enum and CBXShell.rgs*
