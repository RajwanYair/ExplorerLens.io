# ExplorerLens — Test Corpus Skill

## Purpose

Use this skill when working with the test corpus, writing corpus-based decoder tests,
or validating that decoders produce correct output from real files. The test corpus is
the single highest-priority gap identified in ROADMAP §7.3.

---

## When to Use This Skill

- Adding CC0/public-domain test files for a decoder
- Writing Catch2 tests that validate decoder output against corpus files
- Running the full corpus validation suite
- Checking SSIM similarity between expected and actual thumbnail output
- Updating `data/corpus/MANIFEST.json` after adding new files

---

## Corpus Directory Layout

```
data/corpus/
├── images/
│   ├── jpeg/       (5+ files: basic, EXIF-rotated, progressive, CMYK, large)
│   ├── png/        (5+ files: 8-bit, 16-bit, alpha, interlaced, animated APNG)
│   ├── webp/       (5+ files: lossy, lossless, alpha, animated)
│   ├── avif/       (3+ files: 8-bit, 10-bit HDR, animated)
│   ├── heic/       (3+ files: single, burst, live-photo key frame)
│   ├── jxl/        (3+ files: lossy, lossless, HDR)
│   ├── raw/        (5+ files: CR2, NEF, ARW, DNG, RAF)
│   └── ...         (EXR, HDR, PSD, TIFF, BMP, GIF, TGA, DDS)
├── archives/       (ZIP, RAR, 7Z, CBZ, CBR — each with ≥1 image inside)
├── documents/      (PDF, EPUB — real multi-page content)
├── fonts/          (TTF, OTF)
├── video/          (MP4 H.264, MKV H.265 — short clips for keyframe)
├── malformed/      (deliberately broken files for fuzz/crash testing)
└── MANIFEST.json   (checksums, expected thumbnail hashes, metadata)
```

---

## Step-by-Step: Add a New Corpus File

### 1. Obtain a CC0 or public-domain file

Sources (in order of preference):
- [Unsplash Source](https://source.unsplash.com) — CC0 photos
- [Wikimedia Commons](https://commons.wikimedia.org) — CC0 files
- [TestImages.net](https://testimages.org) — format-specific test images
- [libavif test corpus](https://github.com/AOMediaCodec/av1-avif) — AVIF samples
- Synthetic generation via ImageMagick/FFmpeg (see below)

### 2. Generate synthetic test files with ImageMagick

```powershell
# JPEG with EXIF rotation tag
magick -size 640x480 gradient:red-blue -set exif:Orientation 6 test-jpeg-rotated.jpg

# PNG 16-bit with alpha
magick -size 256x256 gradient:cyan-magenta -depth 16 -alpha on test-png-16bit-alpha.png

# Animated WebP (3 frames)
magick -delay 30 -size 128x128 xc:red xc:green xc:blue test-animated.webp

# Generate a malformed JPEG (truncated)
$bytes = [IO.File]::ReadAllBytes("valid.jpg"); $bytes[0..1023] | Set-Content malformed-truncated.jpg -Encoding Byte
```

### 3. Add to MANIFEST.json

```json
{
  "files": [
    {
      "path": "images/jpeg/test-basic.jpg",
      "format": "JPEG",
      "sha256": "abc123...",
      "expected_thumbnail_hash": "def456...",
      "notes": "Standard 640x480 sRGB JPEG",
      "license": "CC0",
      "source": "synthetic/ImageMagick"
    }
  ]
}
```

### 4. Generate expected thumbnail hash

```powershell
# After corpus file is added and decoder works correctly:
$thumb = .\build\bin\lens.exe generate data/corpus/images/jpeg/test-basic.jpg -s 256
(Get-FileHash $thumb -Algorithm SHA256).Hash
# Paste into MANIFEST.json expected_thumbnail_hash
```

---

## Step-by-Step: Write a Corpus-Based Test (Catch2)

```cpp
// In Engine/Tests/EngineTests_Late.cpp
#include <catch2/catch_test_macros.hpp>
#include "../Decoders/JpegDecoder.h"
#include "CorpusHelper.h"  // GetCorpusPath(), LoadFileToSpan()

namespace ExplorerLens { namespace Engine {

TEST_CASE("JpegDecoder: basic 640x480 sRGB", "[decoder][jpeg][corpus]") {
    auto path = GetCorpusPath("images/jpeg/test-basic.jpg");
    REQUIRE(std::filesystem::exists(path));

    auto data = LoadFileToSpan(path);
    JpegDecoder decoder;

    // Phase 1: probe
    auto probeResult = decoder.ProbeHeader(std::span(data).first(64));
    REQUIRE(probeResult == DecodeResult::Supported);

    // Phase 2: decode at 256px
    auto stream = MakeIStreamFromSpan(data);
    auto decodeResult = decoder.DecodeAtSize(stream.Get(), 256, std::stop_token{});
    REQUIRE(decodeResult == DecodeResult::Success);
    REQUIRE(decodeResult.width <= 256);
    REQUIRE(decodeResult.height <= 256);
}

TEST_CASE("JpegDecoder: EXIF rotation applied", "[decoder][jpeg][corpus]") {
    auto path = GetCorpusPath("images/jpeg/test-jpeg-rotated.jpg");
    // ... verify output width > height when source is portrait with rot=6 tag ...
}

}} // namespace
```

---

## Running Corpus Tests

```powershell
# Run all corpus-tagged tests
ctest --test-dir build -C Release -L corpus --output-on-failure

# Run specific format corpus tests
ctest --test-dir build -C Release -L jpeg --output-on-failure

# Run with verbose output
.\build\bin\EngineTests.exe "[corpus]" --reporter console
```

---

## SSIM Comparison (Phase 2+)

When validating GPU vs CPU output or cross-platform SSIM:

```powershell
# Using ImageMagick compare
magick compare -metric SSIM expected.png actual.png diff.png
# Target: SSIM >= 0.99 for all formats

# Programmatic in C++
#include "Engine/Core/SSIMValidator.h"
auto ssim = SSIMValidator::Compare(expectedBGRA, actualBGRA, width, height);
REQUIRE(ssim >= 0.99f);
```

---

## Malformed File Testing

Every decoder MUST handle malformed input without crashing:

```cpp
TEST_CASE("JpegDecoder: truncated file does not crash", "[decoder][jpeg][fuzz]") {
    auto path = GetCorpusPath("malformed/jpeg-truncated.jpg");
    JpegDecoder decoder;
    auto data = LoadFileToSpan(path);
    auto stream = MakeIStreamFromSpan(data);
    // Must return an error, not crash or hang
    auto result = decoder.DecodeAtSize(stream.Get(), 256, std::stop_token{});
    REQUIRE(result == DecodeResult::Error);
}
```

---

## Required Constraints

1. **Only CC0 or public-domain files** in `data/corpus/` — no copyrighted images.
2. **All files must be in MANIFEST.json** with SHA256 checksums.
3. **Synthetic files preferred** for edge cases (rotation, alpha, HDR, malformed).
4. **Malformed files required** for every decoder in `data/corpus/malformed/`.
5. **Tests must be tagged** with `[corpus]` + `[format-name]` Catch2 tags.
6. **SSIM check required** for any decode path that involves color transformation.
7. **corpus/ is gitignored for large files** — use Git LFS for files > 500 KB.

---

## Current Corpus Status (v36.3.0)

| Format | Files in corpus | Target | Status |
|--------|----------------|--------|--------|
| WebP | 1 (webp-basic) | 5+ | Needs expansion |
| QOI | 1 (qoi-gradient) | 3+ | Needs expansion |
| PPM | 1 (ppm-red-gradient) | 3+ | Needs expansion |
| BMP | 1 (bmp-solid-blue) | 3+ | Needs expansion |
| ICO | 1 (ico-multisize) | 3+ | Needs expansion |
| TGA | 1 (tga-uncompressed) | 3+ | Needs expansion |
| JPEG | 0 | 5+ | Missing |
| PNG | 0 | 5+ | Missing |
| HEIC | 0 | 3+ | Missing |
| AVIF | 0 | 3+ | Missing |
| JXL | 0 | 3+ | Missing |
| RAW (CR2/NEF/ARW/DNG) | 0 | 5+ | Missing |
| PDF | 0 | 3+ | Missing |
| CBZ/CBR | 0 | 3+ | Missing |

**Phase 1 target:** 100+ total corpus files. Current: ~21 (synthetic).

---

## CI Integration

The `corpus-validation.yml` workflow runs corpus tests on push/PR:

```yaml
# Key steps:
# 1. Checkout with data/corpus/ included
# 2. Build Engine with Catch2 tests
# 3. Run: ctest --test-dir build -C Release -L corpus --output-on-failure
# 4. Upload test results as artifact
```

To add corpus files to CI cache (avoiding re-download):
```yaml
- uses: actions/cache@v4
  with:
    path: data/corpus
    key: corpus-${{ hashFiles('data/corpus/MANIFEST.json') }}
```

---

## Validation Checklist

- [ ] All new corpus files have a MANIFEST.json entry with SHA256
- [ ] All corpus files are CC0 or public-domain with source documented
- [ ] Malformed variant exists for each new format
- [ ] Catch2 tests cover: basic decode, ProbeHeader, malformed input
- [ ] Tests tagged `[corpus][format-name]`
- [ ] SSIM validation added for formats with color management
- [ ] Corpus CI job runs on every PR (`corpus-validate.yml`)
