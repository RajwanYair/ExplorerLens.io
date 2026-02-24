# Test Archives - ExplorerLens Format Validation

This directory contains test samples for validating thumbnail generation across all supported formats.

## Directory Structure

```
test-archives/
├── images/           # Standard image formats
├── modern/           # Modern formats (WebP, AVIF, JXL, HEIF)
├── raw/              # Camera RAW formats
├── professional/     # PSD, EXR, HDR, DDS
├── vector/           # SVG, PDF
├── archives/         # ZIP, RAR, 7Z, TAR
├── video/            # MP4, AVI, MKV, WEBM
├── audio/            # MP3, FLAC, M4A (album art)
├── documents/        # PDF, DOCX, XLSX, EPUB
├── fonts/            # TTF, OTF
└── models/           # OBJ, STL, GLTF
```

## Format Coverage Matrix

### Image Formats
| Format | Extension | Decoder | Status | Test File |
|--------|-----------|---------|--------|-----------|
| **Standard** |
| JPEG | `.jpg`, `.jpeg` | ImageDecoder (WIC) | ✅ Ready | `images/sample.jpg` |
| PNG | `.png` | ImageDecoder (WIC) | ✅ Ready | `images/sample.png` |
| BMP | `.bmp` | ImageDecoder (WIC) | ✅ Ready | `images/sample.bmp` |
| GIF | `.gif` | ImageDecoder (WIC) | ✅ Ready | `images/sample.gif` |
| TIFF | `.tif`, `.tiff` | ImageDecoder (WIC) | ✅ Ready | `images/sample.tif` |
| **Modern** |
| WebP | `.webp` | WebPDecoder | ✅ Ready | `modern/sample.webp` |
| AVIF | `.avif` | AVIFDecoder | ✅ Ready | `modern/sample.avif` |
| JPEG XL | `.jxl` | JXLDecoder | ✅ Ready | `modern/sample.jxl` |
| HEIF/HEIC | `.heif`, `.heic` | HEIFDecoder | ⚠️ Needs libheif | `modern/sample.heic` |
| QOI | `.qoi` | QOIDecoder | ✅ Ready | `modern/sample.qoi` |
| **RAW** |
| Canon | `.cr2`, `.cr3` | RAWDecoder (LibRaw) | ✅ Ready | `raw/canon.cr2` |
| Nikon | `.nef`, `.nrw` | RAWDecoder (LibRaw) | ✅ Ready | `raw/nikon.nef` |
| Sony | `.arw`, `.srf`, `.sr2` | RAWDecoder (LibRaw) | ✅ Ready | `raw/sony.arw` |
| Adobe DNG | `.dng` | RAWDecoder (LibRaw) | ✅ Ready | `raw/sample.dng` |
| **Professional** |
| Photoshop | `.psd`, `.psb` | PSDDecoder | ✅ Ready | `professional/sample.psd` |
| OpenEXR | `.exr` | EXRDecoder (WIC) | ✅ Ready | `professional/sample.exr` |
| Radiance HDR | `.hdr` | HDRDecoder | ✅ Ready | `professional/sample.hdr` |
| DirectX DDS | `.dds` | DDSDecoder | ✅ Ready | `professional/texture.dds` |
| Targa | `.tga` | TGADecoder | ✅ Ready | `professional/sample.tga` |
| Icon | `.ico`, `.cur` | ICODecoder | ✅ Ready | `professional/icon.ico` |
| Netpbm | `.ppm`, `.pgm`, `.pbm` | PPMDecoder | ✅ Ready | `professional/sample.ppm` |

### Vector & Documents
| Format | Extension | Decoder | Status | Test File |
|--------|-----------|---------|--------|-----------|
| SVG | `.svg`, `.svgz` | SVGDecoder | ✅ Ready | `vector/logo.svg` |
| PDF | `.pdf` | PDFDecoder | ✅ Ready | `documents/manual.pdf` |

### Archives
| Format | Extension | Decoder | Status | Test File |
|--------|-----------|---------|--------|-----------|
| ZIP | `.zip` | ArchiveDecoder | ✅ Ready | `archives/photos.zip` |
| RAR | `.rar` | ArchiveDecoder | ✅ Ready | `archives/data.rar` |
| 7-Zip | `.7z` | ArchiveDecoder | ✅ Ready | `archives/backup.7z` |
| GZIP | `.gz`, `.gzip` | ArchiveDecoder | ✅ Ready | `archives/file.tar.gz` |
| TAR | `.tar` | ArchiveDecoder | ✅ Ready | `archives/archive.tar` |

### Multimedia
| Format | Extension | Decoder | Status | Test File |
|--------|-----------|---------|--------|-----------|
| **Video** |
| MP4 | `.mp4`, `.m4v` | VideoDecoder (MF) | ✅ Ready | `video/sample.mp4` |
| MKV | `.mkv` | VideoDecoder (MF) | ✅ Ready | `video/movie.mkv` |
| AVI | `.avi` | VideoDecoder (MF) | ✅ Ready | `video/clip.avi` |
| WebM | `.webm` | VideoDecoder (MF) | ✅ Ready | `video/web.webm` |
| MOV | `.mov` | VideoDecoder (MF) | ✅ Ready | `video/quicktime.mov` |
| **Audio** |
| MP3 | `.mp3` | AudioDecoder | ✅ Ready | `audio/song.mp3` |
| FLAC | `.flac` | AudioDecoder | ✅ Ready | `audio/lossless.flac` |
| M4A | `.m4a` | AudioDecoder | ✅ Ready | `audio/apple.m4a` |
| OGG | `.ogg` | AudioDecoder | ✅ Ready | `audio/vorbis.ogg` |

### 3D Models
| Format | Extension | Decoder | Status | Test File |
|--------|-----------|---------|--------|-----------|
| Wavefront | `.obj` | ModelDecoder | ✅ Ready | `models/asset.obj` |
| STL | `.stl` | ModelDecoder | ✅ Ready | `models/print.stl` |
| GLTF | `.gltf`, `.glb` | ModelDecoder | ✅ Ready | `models/scene.gltf` |

### Fonts
| Format | Extension | Decoder | Status | Test File |
|--------|-----------|---------|--------|-----------|
| TrueType | `.ttf` | FontDecoder | ✅ Ready | `fonts/arial.ttf` |
| OpenType | `.otf` | FontDecoder | ✅ Ready | `fonts/font.otf` |

## Test Sample Requirements

### Image Files
- **Minimum:** At least one sample per decoder category
- **Sizes:** Small (< 1MB), Medium (1-10MB), Large (> 10MB)
- **Color modes:** RGB, RGBA, Grayscale, CMYK (where applicable)
- **Bit depths:** 8-bit, 16-bit, 32-bit floating point

### Archive Files
- **Contents:** Mix of images (JPEG, PNG, WebP, AVIF)
- **Sizes:** Small (< 100 files), Medium (100-1000 files), Large (> 1000 files)
- **Nesting:** Test nested archives (ZIP within ZIP)
- **Encryption:** Password-protected archives (for security testing)

### Video Files
- **Codecs:** H.264, H.265/HEVC, VP9, AV1
- **Resolutions:** 480p, 720p, 1080p, 4K
- **Duration:** Short (< 1 min), Medium (1-10 min), Long (> 10 min)

### Audio Files
- **With album art:** MP3 with embedded JPEG, FLAC with cover art
- **Without art:** Generate waveform visualization

## Automated Testing

### Run All Tests
```powershell
.\scripts\test\Test-ExplorerLens.ps1 -TestDirectory "test-archives" -Recursive
```

### Test Specific Format
```powershell
.\scripts\test\Test-ExplorerLens.ps1 -Format "webp" -InputPath "test-archives\modern"
```

### Benchmark Performance
```powershell
.\build\bin\Release\EngineBenchmark.exe --input "test-archives\images" --iterations 100
```

## Adding New Test Files

1. **Place file in appropriate category folder**
2. **Run verification:** `.\scripts\Verify-Decoders.ps1`
3. **Generate thumbnail:** `.\scripts\test\Test-Single-File.ps1 -Path "path\to\file"`
4. **Compare output:** Check `test-archives\output\` for results
5. **Document:** Add entry to this README

## Known Issues

### HEIF/HEIC
- ⚠️ Requires libheif library (not yet built)
- Workaround: Use fallback PNG decoder for iPhone photos converted via iCloud

### OpenEXR
- ⚠️ Requires WIC codec (install via Windows Store: Raw Image Extension)
- Fallback: Returns placeholder if codec not available

### 3D Models
- ⚠️ Complex scenes may timeout beyond 30 seconds
- Recommendation: Use low-poly models for thumbnails

## Version History

- **v7.0.0** (2026-02-16): Added Video, PDF, SVG, QOI, Model, Font decoders
- **v6.2.0** (2026-01): Added HEIF, EXR, DDS, TGA decoders
- **v6.0.0** (2025-10): Refactored to Engine-based architecture
- **v5.3.0** (2025-06): Added JPEG XL, AVIF support
- **v1.0.0** (2024-01): Initial release with JPEG, PNG, WebP, RAW

