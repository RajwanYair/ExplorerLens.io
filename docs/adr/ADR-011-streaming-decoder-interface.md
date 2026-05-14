# ADR-011: IStreamingDecoder Interface for Probe-then-Decode Pipeline

**Status:** Accepted
**Version:** v38.4.0+
**Date:** 2026-04-23
**ROADMAP Reference:** §7.4, Decision D38

## Context

ExplorerLens currently dispatches decodes through a monolithic `LENSArchive::Decode()` function that
reads the full file before format detection and routing. This has several problems:

1. **Full-read on probe** — detecting a format requires reading the entire file in some decoders
1. **No partial decode** — progressive JPEG, HEIC embedded previews, and RAW embedded JPEGs cannot
   return a preview without completing the full decode pipeline
1. **No cancellation** — long decodes inside `explorer.exe` block the UI thread with no escape hatch
1. **No streaming** — decoders that could work on the first 16 KB must wait for full IStream content
1. **No metadata extraction without pixel decode** — width/height/colorspace require running the
   whole decoder even when only metadata is needed

XnView MP (§3.1, H4) uses a "probe then decode" model: a `ProbeHeader()` pass reads only the first
N KB to extract metadata and determine routing, then `DecodeAtSize()` performs the full decode only
when the shell requests a specific thumbnail size.

Apple's Quick Look generator protocol and GNOME Tumbler's thumbnailer interface independently
converged on the same architecture (see competitor matrix §3.1), validating this pattern.

## Decision

Introduce a new **`IStreamingDecoder`** abstract interface that all new decoders must implement.
Existing decoders may implement it incrementally; a shim wraps legacy decoders to satisfy the
interface contract.

```cpp
// Engine/Core/IStreamingDecoder.h

#pragma once
#include <span>
#include <optional>
#include <expected>
#include <stop_token>
#include <chrono>

namespace ExplorerLens::Engine {

enum class PartialDecodeState : uint8_t {
    COMPLETE,        // Full-fidelity decode produced
    PARTIAL,         // Reduced-quality decode (e.g., truncated progressive JPEG)
    HEADER_ONLY,     // No pixels; only metadata populated
    FAILED           // Decode failed; hr contains the error code
};

struct Metadata {
    uint32_t width  = 0;
    uint32_t height = 0;
    uint8_t  bit_depth = 8;
    bool     has_alpha = false;
    bool     has_embedded_preview = false;  // RAW [H7], HEIC hnti box
    bool     is_hdr = false;
    std::optional<std::string> color_profile;  // ICC profile name or "sRGB"
};

struct DecodeResult {
    HRESULT             hr    = S_OK;
    PartialDecodeState  state = PartialDecodeState::FAILED;
    std::optional<Metadata> meta;
    std::chrono::microseconds elapsed{0};
    // Bitmap output is returned separately via IWICBitmap* to avoid
    // coupling this header to WIC types; callers cast from void*.
    void* bitmap_wic = nullptr;  // IWICBitmap* — AddRef'd; caller releases
};

struct IStreamingDecoder {
    virtual ~IStreamingDecoder() = default;

    // Probe the first 16 KB of the stream. Must NOT allocate pixel memory.
    // Returns HEADER_ONLY on success. Must complete in < 1 ms.
    virtual DecodeResult ProbeHeader(std::span<const uint8_t> first16KB) = 0;

    // Decode the stream at the requested output size. `target` is the longer
    // dimension; the decoder preserves aspect ratio.
    // Must respect `cancel` — check `cancel.stop_requested()` every 5 ms.
    virtual DecodeResult DecodeAtSize(IStream* stream,
                                      uint32_t target,
                                      std::stop_token cancel) = 0;

    // Whether this decoder can return a partial bitmap from a truncated stream.
    virtual bool SupportsPartialDecode()    const noexcept { return false; }

    // Whether this format embeds a pre-rendered preview (RAW, HEIC, EPS).
    // When true, ExtractEmbeddedPreview() is preferred over full decode [H7].
    virtual bool SupportsEmbeddedPreview() const noexcept { return false; }

    // Extract embedded preview without full decode. Only valid if
    // SupportsEmbeddedPreview() returns true.
    virtual DecodeResult ExtractEmbeddedPreview(IStream* stream,
                                                std::stop_token cancel) {
        (void)stream; (void)cancel;
        return { E_NOTIMPL, PartialDecodeState::FAILED, {}, {} };
    }
};

}  // namespace ExplorerLens::Engine
```

### Integration with DecoderRegistry

The `DecoderRegistry` stores `IStreamingDecoder*` per format. Legacy decoders receive a shim:

```cpp
// LegacyDecoderShim wraps old-style decoders
class LegacyDecoderShim final : public IStreamingDecoder {
    ILegacyDecoder* m_legacy;
public:
    DecodeResult ProbeHeader(std::span<const uint8_t> first16KB) override;
    DecodeResult DecodeAtSize(IStream* stream, uint32_t target,
                               std::stop_token cancel) override;
};
```

### Phase schedule

| Phase | Requirement |
| ------- | ------------- |
| 1 | Interface defined; all new decoders implement it; shim wraps legacy |
| 2 | P0 decoders (JPEG, PNG, WebP, AVIF, HEIC, JXL, RAW) fully native |
| 3 | All P1/P2 decoders native; legacy shim retired |

## Rationale

- **Incremental adoption** — shim bridges old code without a rewrite
- **Competitive parity** — matches XnView MP [H4], Quick Look, and GNOME Tumbler
- **Cancellation** — `std::stop_token` (C++20) is zero-overhead when unused
- **MSVC v145 compatible** — `std::expected` requires `/std:c++23preview`; guarded with
  `#if __cpp_lib_expected` (Decision D31)
- **No COM leakage** — `IStreamingDecoder` is a C++ pure-virtual, not a COM interface;
  the COM boundary stays at `LENSShell.dll`
- **RAW fast-path** — `SupportsEmbeddedPreview()` enables `LibRaw::unpack_thumb()` (100×
  faster per H7) without touching the full RAW decoder

## Consequences

### Positive

- ProbeHeader results can be cached; repeat thumbnails skip full decode
- `std::stop_token` allows `explorer.exe` to cancel stale thumbnail requests
- Embedded preview extraction turns 150ms RAW decode into < 2ms for small thumbnails
- Uniform interface enables the `lens benchmark` JSON report (§6.3, Phase 2)

### Negative

- Shim adds one vtable call overhead per decode (~0 ns in practice)
- `std::expected` requires `/std:c++23preview` guard until MSVC stabilises C++23 library
- Legacy decoders needing cancellation must be refactored to poll `stop_token`

## Alternatives Considered

| Alternative | Why rejected |
| ------------- | ------------- |
| `std::function` callbacks for progress | Heap allocation per decode; less composable |
| COM `IAsyncOperation` | Requires marshalling overhead; COM-leaks into Engine |
| Future/Promise model | Heavier machinery; `stop_token` is sufficient for this use case |
| Keep monolithic decoder dispatch | Blocks Phase 2 streaming I/O and SIMD patterns [H11] |
