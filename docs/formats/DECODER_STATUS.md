# DarkThumbs Decoder Status Matrix
**Version:** 7.0.0  
**Last Updated:** February 17, 2026 (Sprint 9)

---

## Quick Reference

| Status | Meaning |
|--------|---------|
| ✅ Functional | Fully working, producing real thumbnails |
| ⚠️ Conditional | Working only when compile flag is enabled |
| 🔶 Partial | Some paths work, others are stubs |
| ❌ Stub | Returns E_NOTIMPL or placeholder bitmap |

---

## Engine Decoders (Engine/Decoders/)

These implement `IThumbnailDecoder` and are registered with `ThumbnailPipeline`.

| Decoder | Extensions | Status | Compile Flag | Library | Tests |
|---------|-----------|--------|-------------|---------|-------|
| **ImageDecoder** | .jpg .jpeg .jpe .jfif .png .bmp .dib .gif .tif .tiff | ✅ Functional | — | WIC (built-in) | 8 tests |
| **WebPDecoder** | .webp | ✅ Functional | — | libwebp 1.5.0 | 5 tests |
| **AVIFDecoder** | .avif .heif .heic | ✅ Functional | — | WIC (Win10+ codec) | 5 tests |
| **HEIFDecoder** | .heif .heic .hif .heifs .heics .avci .avcs | ✅ Functional | `HAS_LIBHEIF` (ON) | libheif | 2 tests |
| **JXLDecoder** | .jxl | ⚠️ Conditional | `HAS_LIBJXL` (OFF) | libjxl 0.11.1 | 2 tests |
| **ArchiveDecoder** | .zip .cbz | ✅ Functional | — | minizip-ng 4.0.10 | 6 tests |
| **RAWDecoder** | .cr2 .cr3 .crw .nef .nrw .arw .srf .sr2 .orf .rw2 .raw .raf .pef .ptx .dng .rwl .srw .3fr .iiq .x3f | ✅ Functional | `HAS_LIBRAW` (ON) | LibRaw 0.21.2 | 0 tests |
| **TGADecoder** | .tga .tpic | ✅ Functional | — | Native (no deps) | 0 tests |
| **QOIDecoder** | .qoi | ✅ Functional | — | Native (no deps) | 0 tests |

**Total Engine tests:** 100 unit tests + 5 benchmark suites = 105 total

---

## CBXShell Legacy Decoders (CBXShell/)

These are static-method classes used by the COM shell extension directly. They do NOT implement `IThumbnailDecoder`.

| Decoder | Extensions | Status | Library | Notes |
|---------|-----------|--------|---------|-------|
| **AVIFDecoder** | .avif | ✅ Functional | WIC | Separate from Engine AVIFDecoder |
| **WebPDecoder** | .webp | ✅ Functional | libwebp | Separate from Engine WebPDecoder |
| **JXLDecoder** | .jxl | ✅ Functional | libjxl (required) | No #ifdef guard — requires libjxl always |
| **HEIFDecoderNative** | .heif .heic | ✅ Functional | WIC | Uses Windows HEIF codec, not libheif |
| **RAWDecoder** | .dng .cr2 .cr3 .crw .nef .nrw .arw .srf .sr2 .orf .rw2 .pef .raf .dcr .mrw .x3f | ✅ Functional | WIC | Relies on Camera Codec Pack |
| **PDFDecoder** | .pdf | ❌ Stub | — | Returns E_NOTIMPL |
| **SVGDecoder** | .svg | ❌ Stub | — | Returns gradient placeholder bitmap |
| **AudioThumbnail** | .mp3 .m4a .flac .ogg .wav .opus | 🔶 Partial | Native | Album art works; waveform is flat-line placeholder |
| **VideoThumbnail** | .mp4 .avi .mkv .wmv etc. | ✅ Functional | DirectShow | Inline ISampleGrabber (qedit.h removed from SDK) |
| **DocumentThumbnail** | .docx .doc .xlsx .xls .pptx .ppt .txt .rtf .xps .oxps | ✅ Functional | COM/WIC | Multiple fallback strategies |
| **FontPreview** | .ttf .otf .woff .woff2 | ✅ Functional | DirectWrite | GDI fallback available |

---

## Plugin Decoder (Engine/Plugin/)

| Component | Status | Notes |
|-----------|--------|-------|
| **PluginDecoder** | ⚠️ Built, not wired | Adapter wrapping plugin DLL as IThumbnailDecoder |
| **PluginManager** | ⚠️ Built, not wired | Discovery, loading, lifecycle management |
| **PluginHostClient** | ⚠️ Built, not wired | IPC to isolated PluginHost.exe |
| **IPC Protocol** | ⚠️ Built, not wired | Named pipe + shared memory |
| **Plugin SDK** | ✅ Complete | C ABI, stable API v1.0 |
| **Sample Plugin** | ✅ Example | SDK/examples/minimal-plugin/ |

**Plugin system activation:** Sprint 17 — uncomment `LoadPlugins()` in ThumbnailPipeline.cpp

---

## Format Coverage Summary

### Fully Functional (producing real thumbnails)
- **Images:** JPEG, PNG, BMP, GIF, TIFF, WebP, AVIF, TGA, QOI
- **Camera RAW:** CR2, CR3, CRW, NEF, NRW, ARW, SRF, SR2, ORF, RW2, RAF, PEF, PTX, DNG, RWL, SRW, 3FR, IIQ, X3F, DNG, DCR, MRW
- **Archives:** ZIP, CBZ (via minizip-ng)
- **Audio:** MP3, M4A, FLAC, OGG, WAV, OPUS (album art extraction)
- **Video:** MP4, AVI, MKV, WMV (DirectShow frame capture)
- **Documents:** DOCX, DOC, XLSX, XLS, PPTX, PPT, TXT, RTF, XPS, OXPS
- **Fonts:** TTF, OTF, WOFF, WOFF2

### Conditional (requires library build)
- **HEIF/HEIC:** Engine decoder needs `HAS_LIBHEIF=ON` + libheif built
- **JPEG XL:** Engine decoder needs `HAS_LIBJXL=ON` + libjxl built

### Stubs (awaiting implementation)
- **PDF:** Shell extension returns E_NOTIMPL
- **SVG:** Shell extension shows placeholder gradient

### Total Format Count
- **Working:** ~100+ file extensions across all decoders
- **Conditional:** 8 extensions (HEIF family + JXL)
- **Stub:** 2 extensions (PDF, SVG)

---

## Compile Flags Reference

Set in `Engine/CMakeLists.txt`:

```cmake
option(HAS_LIBJXL  "Enable JPEG XL support via libjxl"  OFF)
option(HAS_LIBHEIF "Enable HEIF/HEIC support via libheif" OFF)
option(HAS_LIBRAW  "Enable Camera RAW support via LibRaw" ON)
```

To enable a decoder, flip the flag and ensure the library is built:
```powershell
cmake -DHAS_LIBJXL=ON -DHAS_LIBHEIF=ON ..
```

---

## Test Coverage Gaps

Decoders without unit tests (candidates for Sprint 19):
1. **RAWDecoder** (Engine) — High priority, complex code
2. **TGADecoder** (Engine) — Medium priority, native code
3. **QOIDecoder** (Engine) — Low priority, simple reference impl
4. All CBXShell legacy decoders — Not Engine-based, harder to unit test
