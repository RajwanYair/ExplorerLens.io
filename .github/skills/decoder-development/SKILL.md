# ExplorerLens — Decoder Development Skill

## Purpose

Use this skill when adding a new format decoder, fixing an existing one, or wiring a
decoder into the engine pipeline. Read fully before writing any `Engine/Decoders/` code.

---

## When to Use This Skill

- Adding support for a new image/archive/document/video format
- Implementing `IStreamingDecoder` for a new decoder class
- Wiring a decoder into `DecoderRegistry`
- Writing or fixing decoder unit tests against the test corpus
- Debugging incorrect thumbnail output from a decoder

---

## Step-by-Step: Create a New Decoder

### 1. Collision check (MANDATORY before writing anything)

```powershell
# Search ALL Engine headers for your new class/struct/enum names
grep_search "MyNewDecoderClass" "Engine/**/*.h"
# Zero matches required (except the file you are about to create)
```

### 2. Create the header in `Engine/Decoders/`

```cpp
// MyFormatDecoder.h — MyFormat thumbnail decoder
// Copyright (c) 2026 ExplorerLens Project
//
// Decodes .myfmt files using libmyfmt 1.0.0 via IStreamingDecoder.
//
#pragma once
#include "../Core/IStreamingDecoder.h"

namespace ExplorerLens { namespace Engine {

class MyFormatDecoder final : public IStreamingDecoder {
public:
    // Phase 1: probe magic bytes — returns true if this is .myfmt
    DecodeResult ProbeHeader(std::span<const uint8_t> header) override;

    // Phase 2: decode at thumbnail size, cancel-aware
    DecodeResult DecodeAtSize(IStream* stream, uint32_t targetSize,
                              std::stop_token cancel) override;

    bool SupportsPartialDecode() const override { return false; }

    static constexpr std::string_view FORMAT_NAME = "MyFormat";
    static constexpr std::array<uint8_t, 4> MAGIC = { 0xMY, 0xFM, 0xT, 0x00 };
};

}} // namespace ExplorerLens::Engine
```

### 3. Create the implementation in `Engine/Decoders/`

```cpp
// MyFormatDecoder.cpp — MyFormat decoder implementation
#include "pch.h"
#include "MyFormatDecoder.h"
// ... real libmyfmt include ...

namespace ExplorerLens { namespace Engine {

DecodeResult MyFormatDecoder::ProbeHeader(std::span<const uint8_t> header) {
    if (header.size() < MAGIC.size()) return DecodeResult::InsufficientData;
    return std::equal(MAGIC.begin(), MAGIC.end(), header.begin())
        ? DecodeResult::Supported : DecodeResult::Unsupported;
}

DecodeResult MyFormatDecoder::DecodeAtSize(IStream* stream, uint32_t targetSize,
                                            std::stop_token cancel) {
    // Real decode using libmyfmt
    // ...
    return DecodeResult::Success;
}

}} // namespace ExplorerLens::Engine
```

### 4. Register in `Engine/CMakeLists.txt`

Add to ENGINE_HEADERS (before `# Pipeline`):
```cmake
Engine/Decoders/MyFormatDecoder.h
```
Add to ENGINE_SOURCES:
```cmake
Engine/Decoders/MyFormatDecoder.cpp
```

### 5. Register in `DecoderRegistry`

Add the decoder mapping in `Engine/Core/DecoderRegistry.cpp`:
```cpp
RegisterDecoder(".myfmt", std::make_unique<MyFormatDecoder>());
RegisterDecoder(".myf",   std::make_unique<MyFormatDecoder>());
```

### 6. Add LENSTYPE enum value (if needed)

Check `LENSShell/LENSArchive.h` for existing values — **no collisions allowed**.

### 7. Write tests (required before committing)

See the **Test Corpus Skill** for corpus-based testing procedure.

#### Custom TEST() macro (legacy — existing decoders)

New decoder tests go in `Engine/Tests/EngineTests_Platform.cpp`.
Add `extern void` declarations in `EngineTestsExterns.h` and `RUN_TEST()` calls in `EngineTests.cpp`.

```cpp
// EngineTests_Platform.cpp
TEST(TestMyFormatDecoder_ProbeHeader) {
    MyFormatDecoder dec;
    uint8_t magic[] = { 0x4D, 0x59 };  // "MY" magic
    ASSERT(dec.ProbeHeader(magic, sizeof(magic)) == true);
    uint8_t bad[] = { 0x00, 0x00 };
    ASSERT(dec.ProbeHeader(bad, sizeof(bad)) == false);
}
```

#### Catch2 v3 (preferred for new decoders)

Catch2 tests live in `Engine/Tests/catch2/`. Use `TEST_CASE` + `SECTION` for structured decode testing:

```cpp
// Engine/Tests/catch2/test_myformat_decoder.cpp
#include <catch2/catch_test_macros.hpp>
#include "Engine/Decoders/MyFormatDecoder.h"

TEST_CASE("MyFormatDecoder: probe accepts valid magic", "[decoder][myformat]") {
    MyFormatDecoder dec;
    const uint8_t magic[] = { 0x4D, 0x59 };
    REQUIRE(dec.ProbeHeader(magic, sizeof(magic)));
}

TEST_CASE("MyFormatDecoder: decode produces valid bitmap", "[decoder][myformat]") {
    MyFormatDecoder dec;
    // Load corpus file
    auto data = LoadCorpusFile("images/myformat/sample.myf");
    REQUIRE(!data.empty());

    SECTION("256x256 thumbnail") {
        auto result = dec.DecodeAtSize(data.data(), data.size(), 256, 256);
        REQUIRE(result.success);
        REQUIRE(result.width == 256);
        REQUIRE(result.height <= 256);
    }

    SECTION("512x512 thumbnail") {
        auto result = dec.DecodeAtSize(data.data(), data.size(), 512, 512);
        REQUIRE(result.success);
        REQUIRE(result.width == 512);
    }
}

TEST_CASE("MyFormatDecoder: rejects malformed input", "[decoder][myformat][security]") {
    MyFormatDecoder dec;
    SECTION("empty buffer") {
        auto result = dec.DecodeAtSize(nullptr, 0, 256, 256);
        REQUIRE_FALSE(result.success);
    }
    SECTION("truncated header") {
        const uint8_t truncated[] = { 0x4D };
        auto result = dec.DecodeAtSize(truncated, 1, 256, 256);
        REQUIRE_FALSE(result.success);
    }
}
```

**Catch2 tag conventions:**
- `[decoder]` — all decoder tests
- `[myformat]` — format-specific tag (e.g., `[jpeg]`, `[png]`, `[pdf]`)
- `[security]` — fuzzing/malformed input tests
- `[corpus]` — requires files from `data/corpus/`
- `[slow]` — tests > 1 second (excluded from quick runs: `--skip-benchmarks`)

Register Catch2 test files in `Engine/Tests/CMakeLists.txt` under the `Catch2Tests` target.

---

## Two-Phase Decode Architecture

All decoders MUST implement the two-phase pattern:

| Phase | Purpose | Data Access | Performance Target |
| ------- | --------- | ------------- | ------------------- |
| `ProbeHeader` | Magic bytes only — confirm format | First 16 KB | < 0.1 ms |
| `DecodeAtSize` | Minimal decode at requested size | Full stream | Format-specific |

**RAW photo fast path:** Use `LibRaw::unpack_thumb()` before attempting full decode.
This is 100× faster and is what users want for thumbnails.

**Archive cover image:** Scan central directory; pick first image alphabetically.
Do NOT decompress all entries.

**PDF first page:** Use `fz_new_pixmap_from_page()` at 72 DPI then resize — do not
render at full resolution.

---

## Format-Specific Rules

| Format | Library | Key Constraint |
| -------- | --------- | --------------- |
| JPEG | libjpeg-turbo | Always apply EXIF orientation (tag 0x0112) |
| PNG | WIC / libpng | Handle 16-bit; return first APNG frame |
| AVIF | libavif + dav1d | Use `avifDecoderSetIOMemory`; do not call `avifDecoderNextImage` in loop |
| HEIC | libheif + libde265 | Use `heif_context_get_primary_image_handle()` |
| JXL | libjxl | Set `JxlDecoderSubscribeEvents(JXL_DEC_FULL_IMAGE)` only |
| RAW | LibRaw | Call `unpack_thumb()` first; fall back to `dcraw_process()` |
| PDF | MuPDF | Use `fz_new_pixmap_from_page()` at `1.0f` scale then resize |
| ZIP/CBZ | minizip-ng | Walk central directory; pick first `.jpg`/`.png`/`.webp` |

---

## Streaming and Partial Decode Patterns

Some decoders can produce a valid thumbnail before the full file is read. Implement
`SupportsPartialDecode()` returning `true` and follow this pattern:

### Progressive JPEG Example

```cpp
DecodeResult JpegDecoder::DecodeAtSize(IStream* stream, uint32_t targetSize,
                                        std::stop_token cancel) {
    // Read first 64 KB — often enough for thumbnail
    constexpr size_t INITIAL_READ = 64 * 1024;
    std::vector<uint8_t> buffer(INITIAL_READ);
    ULONG bytesRead = 0;
    stream->Read(buffer.data(), INITIAL_READ, &bytesRead);
    buffer.resize(bytesRead);

    // Attempt progressive decode with partial data
    auto result = TryProgressiveDecode(buffer, targetSize);
    if (result == DecodeResult::Success) return result;

    // If not enough data, read the rest (cancel-aware)
    while (bytesRead > 0 && !cancel.stop_requested()) {
        std::array<uint8_t, 32768> chunk{};
        stream->Read(chunk.data(), chunk.size(), &bytesRead);
        if (bytesRead == 0) break;
        buffer.insert(buffer.end(), chunk.data(), chunk.data() + bytesRead);
    }
    if (cancel.stop_requested()) return DecodeResult::Cancelled;
    return FullDecode(buffer, targetSize);
}
```

### IStream Position Management

```cpp
// Reset stream to beginning before decode
LARGE_INTEGER zero{};
stream->Seek(zero, STREAM_SEEK_SET, nullptr);

// Get total stream size (for progress estimation)
STATSTG stat{};
stream->Stat(&stat, STATFLAG_NONAME);
uint64_t totalBytes = stat.cbSize.QuadPart;
```

### Cancel Token Best Practice

Check `stop_token` at these points:
1. Before starting decode (early exit if Explorer cancelled)
2. Inside per-scanline/per-tile loops
3. After large I/O reads

---

## Required Constraints

1. **No header without .cpp** — implement before declaring (ROADMAP §4.4).
2. **ProbeHeader must be fast** — only read magic bytes, no I/O seeking.
3. **DecodeAtSize must respect `stop_token`** — check cancellation in decode loops.
4. **Never seek backwards** on a forward-only IStream.
5. **All output must be BGRA32** — the engine assumes this pixel format.
6. **Test against corpus** — every new decoder needs ≥ 3 real test files.
7. **LENSTYPE enum values must not collide** — always grep before adding.
8. **Register in CMakeLists.txt** — both ENGINE_HEADERS and ENGINE_SOURCES.

---

## Validation Checklist

- [ ] Collision check passed: no duplicate class/enum names in Engine headers
- [ ] Header has copyright banner, `#pragma once`, namespace `ExplorerLens::Engine`
- [ ] `.cpp` registered in `Engine/CMakeLists.txt` ENGINE_SOURCES
- [ ] Decoder registered in `DecoderRegistry` for all relevant extensions
- [ ] `ProbeHeader` tested with correct and incorrect magic bytes
- [ ] `DecodeAtSize` tested against 3+ real corpus files
- [ ] Output is BGRA32 (not RGB, not RGBA with premult)
- [ ] Build: 0 errors, 0 warnings with MSVC v145
