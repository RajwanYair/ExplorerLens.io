---
mode: agent
description: "Scaffold a new format decoder for ExplorerLens. Creates the header, source, and test stub following the IStreamingDecoder pattern with ProbeHeader + DecodeAtSize."
---

# Decoder Scaffold — ExplorerLens

Create a new decoder for the **{{formatName}}** format (file extensions: {{extensions}}).

## Step 1: Collision Check

```powershell
# Verify class name is unique across all Engine headers
Select-String -Path Engine\**\*.h -Pattern "\b{{decoderClass}}\b" -Recurse
# Must return ZERO results
```

## Step 2: Create Header

Create `Engine/Decoders/{{decoderClass}}.h`:

```cpp
// {{decoderClass}}.h — {{formatName}} Thumbnail Decoder
// Copyright (c) 2026 ExplorerLens Project
//
// Decodes {{formatName}} files ({{extensions}}) to BGRA thumbnails.
// Uses {{libraryName}} for format parsing.
//
#pragma once
#include "Core/IStreamingDecoder.h"

namespace ExplorerLens {
namespace Engine {

class {{decoderClass}} final : public IStreamingDecoder {
public:
    DecodeResult ProbeHeader(std::span<const uint8_t> header) override;
    DecodeResult DecodeAtSize(IStream* stream,
                              uint32_t targetSize,
                              std::stop_token cancel) override;
    bool SupportsPartialDecode() const override { return {{supportsPartial}}; }
};

} // namespace Engine
} // namespace ExplorerLens
```

## Step 3: Create Source

Create `Engine/Decoders/{{decoderClass}}.cpp` with:

1. `ProbeHeader`: check magic bytes (`{{magicBytes}}` at offset {{magicOffset}})
2. `DecodeAtSize`: full decode using `{{libraryName}}`
3. EXIF orientation handling (for image decoders)
4. Size guard: reject if decoded dimensions > 4096×4096

## Step 4: Register in CMakeLists.txt

Add to `Engine/CMakeLists.txt`:

```cmake
# ENGINE_HEADERS — add:
Decoders/{{decoderClass}}.h

# ENGINE_SOURCES — add:
Decoders/{{decoderClass}}.cpp
```

## Step 5: Register Tests

1. Add `#include "Decoders/{{decoderClass}}.h"` to `EngineTestsIncludes.h`
2. Add `extern void {{decoderClass}}Tests();` to `EngineTestsExterns.h`
3. Add `RUN_TEST({{decoderClass}}Tests)` to `EngineTests.cpp`
4. Add test body to `EngineTests_Late.cpp`:

```cpp
TEST({{decoderClass}}Tests) {
    // Test 1: ProbeHeader with valid magic bytes
    {
        const uint8_t magic[] = { {{magicBytes}} };
        auto result = {{decoderClass}}{}.ProbeHeader(magic);
        ASSERT(result.success);
    }
    // Test 2: DecodeAtSize with real corpus file
    {
        // Requires data/corpus/{{corpusPath}} to exist
        // auto result = {{decoderClass}}{}.DecodeAtSize(..., 256, {});
        // ASSERT(result.success);
        // ASSERT(result.width == 256 || result.height == 256);
    }
    // Test 3: Malformed input returns failure (never crashes)
    {
        const uint8_t garbage[] = { 0xFF, 0x00, 0xAA, 0xBB };
        auto result = {{decoderClass}}{}.ProbeHeader(garbage);
        ASSERT(!result.success);
    }
}
```

## Step 6: Add Corpus Files

Add ≥ 3 CC0/public-domain files to `data/corpus/{{corpusDir}}/`:
- `{{formatLower}}-basic.{{ext}}` — standard file
- `{{formatLower}}-large.{{ext}}` — high-resolution file
- `{{formatLower}}-malformed.{{ext}}` — intentionally corrupt file

Update `data/corpus/MANIFEST.json` with each file's SHA-256 and source URL.

## Step 7: LENSTYPE Enum (if new format)

If {{formatName}} is not in `LENSTYPE` enum in `LENSShell/LENSArchive.h`:

```powershell
# Find last enum value to avoid collision
Select-String -Path LENSShell\LENSArchive.h -Pattern "^\s+[A-Z_]+\s*=" | Select-Object -Last 5
```

Add: `{{LENSTYPE_VALUE}} = <next_available_number>,`
