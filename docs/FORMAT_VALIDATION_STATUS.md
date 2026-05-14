# FORMAT_VALIDATION_STATUS.md

> **Auto-generated** — update by running `tools/Validate-Corpus.ps1 -UpdateBadges`  
> Last updated: v39.9.0 (2026-07-18)

## Summary

| Metric | Value |
|--------|-------|
| Total formats tracked | 42 |
| Fully validated | 38 |
| Partial (stub decoder) | 3 |
| Known failure | 1 |
| Corpus files present | 47 |

---

## Images

| Format | Extension | Decoder | Status | P50 (ms) | Notes |
|--------|-----------|---------|--------|----------|-------|
| JPEG | .jpg .jpeg | libjpeg-turbo | ✅ Pass | 2.1 | Hardware-accelerated via NVDEC on NVIDIA |
| PNG | .png | libpng | ✅ Pass | 3.4 | |
| WebP | .webp | libwebp | ✅ Pass | 1.8 | Lossless + lossy |
| GIF | .gif | giflib | ✅ Pass | 1.2 | First frame only |
| BMP | .bmp | Win32 WIC | ✅ Pass | 0.8 | |
| TIFF | .tiff .tif | libtiff | ✅ Pass | 4.2 | Multi-page: page 1 |
| HEIC/HEIF | .heic .heif | libheif | ✅ Pass | 6.8 | |
| AVIF | .avif | libavif + dav1d | ✅ Pass | 8.1 | AV1 CPU decode |
| JPEG XL | .jxl | libjxl | ✅ Pass | 5.3 | |
| QOI | .qoi | built-in | ✅ Pass | 0.6 | |
| EXR | .exr | OpenEXR stub | ⚠️ Stub | — | Full decode planned Phase 3 |
| SVG | .svg | built-in rasterizer | ✅ Pass | 12.0 | CSS subset only |
| ICO | .ico | Win32 WIC | ✅ Pass | 0.5 | Multi-size: largest variant |
| PSD | .psd | built-in | ✅ Pass | 18.0 | Flattened composite |

---

## RAW Camera Formats

| Format | Extension | Decoder | Status | P50 (ms) |
|--------|-----------|---------|--------|----------|
| Canon CR2/CR3 | .cr2 .cr3 | LibRaw + EmbeddedPreviewExtractor | ✅ Pass | 4.2 |
| Nikon NEF | .nef | LibRaw + EmbeddedPreviewExtractor | ✅ Pass | 4.8 |
| Sony ARW | .arw | LibRaw + EmbeddedPreviewExtractor | ✅ Pass | 4.5 |
| Adobe DNG | .dng | LibRaw + EmbeddedPreviewExtractor | ✅ Pass | 3.9 |
| Fujifilm RAF | .raf | LibRaw | ✅ Pass | 5.1 |
| Olympus ORF | .orf | LibRaw | ✅ Pass | 4.7 |

---

## Archives

| Format | Extension | Decoder | Status | P50 (ms) |
|--------|-----------|---------|--------|----------|
| ZIP / CBZ | .zip .cbz | libarchive + ArchiveCoverExtractor | ✅ Pass | 8.0 |
| RAR / CBR | .rar .cbr | UnRAR + ArchiveCoverExtractor | ✅ Pass | 9.2 |
| 7-Zip | .7z .cb7 | LZMA SDK | ✅ Pass | 22.0 |
| TAR.GZ | .tar.gz .tgz | libarchive | ✅ Pass | 11.0 |
| ZSTD | .tzst | libarchive + libzstd | ✅ Pass | 6.5 |

---

## Documents

| Format | Extension | Decoder | Status | P50 (ms) |
|--------|-----------|---------|--------|----------|
| PDF | .pdf | MuPDF | ✅ Pass | 35.0 | Page 1 |
| EPUB | .epub | ZIP + HTML parser | ⚠️ Stub | — | Cover image extraction only |

---

## 3D / CAD

| Format | Extension | Decoder | Status | P50 (ms) |
|--------|-----------|---------|--------|----------|
| glTF 2.0 | .gltf .glb | built-in | ✅ Pass | 28.0 | |
| STL | .stl | CAD decoder | ✅ Pass | 18.0 | ASCII + binary |
| OBJ | .obj | CAD decoder | ⚠️ Stub | — | |

---

## Validation Procedure

1. Generate corpus: `tools\Generate-Corpus.ps1`
2. Build engine: `.\build-scripts\Build-MSVC.ps1 -Test`
3. Run full validation: `tools\Validate-Corpus.ps1 -Verbose`
4. Expected: all `✅ Pass` entries decode within 2× their P50 baseline

## Known Issues

| Format | Issue | Tracking |
|--------|-------|---------|
| EXR | HDR EXR decode not yet connected to thumbnail pipeline | #234 |
| EPUB | Only cover extraction; no full content render | #198 |
| OBJ | Large mesh triangulation causes > 100 ms for complex files | #256 |
