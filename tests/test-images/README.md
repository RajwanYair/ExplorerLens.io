# ExplorerLens Test Images

This directory contains test images for validating decoder functionality and performance testing.

## Directory Structure

### `/standard/`
Standard image formats for baseline testing:
- JPEG (.jpg, .jpeg)
- PNG (.png)
- BMP (.bmp)
- GIF (.gif)
- TIFF (.tif, .tiff)

### `/webp/`
WebP format test images:
- Lossless WebP
- Lossy WebP
- Animated WebP
- Various sizes and compression levels

### `/avif/`
AVIF format test images:
- Single-frame AVIF
- Various quality levels
- Different chroma subsampling modes

### `/raw/`
Camera RAW format test images:
- Canon (.CR2, .CR3)
- Nikon (.NEF, .NRW)
- Sony (.ARW)
- Various camera models and settings

### `/jxl/`
JPEG XL format test images:
- Lossless JXL
- Lossy JXL
- Animated JXL
- Various encoding options

### `/heif/`
HEIF/HEIC format test images:
- HEIF images
- HEIC (Apple) images
- Various quality levels

### `/ico/`
Icon format test images:
- Single-size icons
- Multi-size icons (.ico)
- Compressed icons

### `/tga/`
TGA (Targa) format test images:
- Uncompressed TGA
- RLE-compressed TGA
- Various bit depths

### `/qoi/`
QOI (Quite OK Image) format test images:
- Various image types
- Different color depths

### `/archives/`
Archive file test samples:
- ZIP archives with images
- RAR archives with images
- 7Z archives with images

## Usage

Tests expect to find sample images in these directories. The tests will:
1. Verify each decoder can detect the correct format
2. Decode the images and verify basic properties (dimensions, format)
3. Measure decode performance
4. Test edge cases (corrupted files, malformed headers, etc.)

## Adding Test Images

When adding new test images:
1. Place them in the appropriate format subdirectory
2. Use descriptive filenames (e.g., `lossless-1920x1080.jxl`, `canon-eos-r5-sample.CR3`)
3. Include a variety of sizes, quality levels, and encoding options
4. Consider copyright - use only public domain or properly licensed images
5. Keep file sizes reasonable (prefer < 5 MB per file)

## Performance Test Images

For performance testing, include images of varying resolutions:
- Small: ~640x480 or smaller
- Medium: ~1920x1080 (Full HD)
- Large: ~3840x2160 (4K)
- Very Large: ~7680x4320 (8K)

## Test Image Sources

Recommended sources for test images:
- https://samples.ffmpeg.org/image-samples/ (Various formats)
- https://raw.pixls.us/ (Camera RAW samples)
- https://github.com/libjxl/conformance (JPEG XL test suite)
- Create your own using format-specific encoders

## Notes

- Test images are NOT included in the repository by default
- Developers should populate this directory with their own test images
- CI/CD pipelines should download or generate test images as needed
- Total test image collection should be kept under 100 MB for practical reasons

