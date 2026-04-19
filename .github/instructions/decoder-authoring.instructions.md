---
applyTo: "**/Engine/Decoders/**"
---

# Decoder Authoring Rules — ExplorerLens

## One Decoder Per Format Family

Each format family gets ONE decoder class. Do not create separate decoders for closely
related variants (e.g., JPEG/JFIF/EXIF → one `JpegDecoder`).

## Required Interface

Every decoder must implement `IStreamingDecoder`:

```cpp
class IStreamingDecoder {
public:
    // Phase 1: Read ≤ 16 KB header to confirm format + extract metadata.
    // Called BEFORE any full decode. Must not fail on valid files.
    virtual DecodeResult ProbeHeader(std::span<const uint8_t> header) = 0;

    // Phase 2: Decode at the requested thumbnail size.
    // cancel token MUST be respected — check it at I/O boundaries.
    virtual DecodeResult DecodeAtSize(IStream* stream,
                                      uint32_t targetSize,
                                      std::stop_token cancel) = 0;

    // Return true if the decoder can decode just a portion of the file.
    virtual bool SupportsPartialDecode() const { return false; }

    virtual ~IStreamingDecoder() = default;
};
```

## RAW Photo Fast Path (MANDATORY)

For LibRaw-based decoders, ALWAYS try the embedded JPEG preview first:

```cpp
// ✅ Fast path: embedded thumbnail (100× faster)
if (raw.imgdata.thumbnail.tformat != LIBRAW_THUMBNAIL_UNKNOWN) {
    return DecodeEmbeddedThumb(raw);
}
// Slow path: full RAW decode (only if no embedded preview)
return DecodeFullRaw(raw);
```

## EXIF Orientation (MANDATORY for Image Decoders)

Every image decoder must apply EXIF orientation before returning the bitmap:

```cpp
// Apply EXIF rotation so user never sees sideways photos
ApplyExifOrientation(bitmap, exifOrientation);
```

## Error Handling

Return `DecodeResult` — never throw across the decoder boundary:

```cpp
struct DecodeResult {
    bool success = false;
    std::wstring errorMessage;
    uint32_t width = 0;
    uint32_t height = 0;
    std::vector<uint8_t> bgraPixels; // BGRA 32-bit, row-major
};
```

## Registration in CMakeLists.txt

New decoder `.cpp` files must be added to `ENGINE_SOURCES` in `Engine/CMakeLists.txt`.
New decoder `.h` files must be added to `ENGINE_HEADERS`.

## Test Requirements

Every decoder must have:
1. A `ProbeHeader` test with valid magic bytes
2. A `DecodeAtSize` test using a real file from `data/corpus/`
3. A malformed-input test (decoder must return `success=false`, never crash)

## Naming Convention

| Class | Example |
|-------|---------|
| Decoder class | `JpegDecoder`, `WebPDecoder`, `AvifDecoder` |
| Source file | `JpegDecoder.cpp`, `WebPDecoder.cpp` |
| Header file | `JpegDecoder.h`, `WebPDecoder.h` |
| Test function | `TEST(JpegDecoderTests)`, `TEST(WebPDecoderTests)` |

## Pre-Authoring Checklist

- [ ] `grep_search` for the decoder class name across `Engine/**/*.h` — zero collision
- [ ] Check `LENSTYPE` enum in `LENSArchive.h` — add new enum value if needed, no collision
- [ ] Register in `Engine/CMakeLists.txt` ENGINE_HEADERS + ENGINE_SOURCES
- [ ] Add `#include` to `EngineTestsIncludes.h`
- [ ] Add `extern void Runner()` to `EngineTestsExterns.h`
- [ ] Add `RUN_TEST()` call to `EngineTests.cpp`
- [ ] Add TEST body to `EngineTests_Late.cpp`
- [ ] Add at least 3 corpus files to `data/corpus/<format>/`
