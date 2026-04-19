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
New decoder tests go in `Engine/Tests/EngineTests_Late.cpp`.

---

## Two-Phase Decode Architecture

All decoders MUST implement the two-phase pattern:

| Phase | Purpose | Data Access | Performance Target |
|-------|---------|-------------|-------------------|
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
|--------|---------|---------------|
| JPEG | libjpeg-turbo | Always apply EXIF orientation (tag 0x0112) |
| PNG | WIC / libpng | Handle 16-bit; return first APNG frame |
| AVIF | libavif + dav1d | Use `avifDecoderSetIOMemory`; do not call `avifDecoderNextImage` in loop |
| HEIC | libheif + libde265 | Use `heif_context_get_primary_image_handle()` |
| JXL | libjxl | Set `JxlDecoderSubscribeEvents(JXL_DEC_FULL_IMAGE)` only |
| RAW | LibRaw | Call `unpack_thumb()` first; fall back to `dcraw_process()` |
| PDF | MuPDF | Use `fz_new_pixmap_from_page()` at `1.0f` scale then resize |
| ZIP/CBZ | minizip-ng | Walk central directory; pick first `.jpg`/`.png`/`.webp` |

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
