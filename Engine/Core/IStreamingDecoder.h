// IStreamingDecoder.h — Streaming Decoder Interface
// Copyright (c) 2026 ExplorerLens Project
//
// Defines the IStreamingDecoder contract that all format decoders must implement
// as part of the Phase-1 architecture reset.  The interface enforces:
//
//   1. Two-phase decode: ProbeHeader (read first 16 KB only) +
//      DecodeAtSize (minimal decode at target thumbnail resolution).
//   2. Cancellation: every long-running decode accepts a std::stop_token so
//      Explorer can cancel requests when scrolling fast.
//   3. Partial-decode opt-in: decoders that support partial decode (e.g. RAW
//      embedded preview extraction) declare it via SupportsPartialDecode().
//
// Migration path from the legacy DecoderRegistry pattern:
//   Old: Extension → LENSTYPE → Decoder::Decode(stream) → HBITMAP
//   New: Extension → LENSTYPE → IStreamingDecoder → ProbeHeader +
//        DecodeAtSize(stream, targetSize, stopToken) → DecodedThumb
//
#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <stop_token>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// ---------------------------------------------------------------------------
// ProbeResult — fast header-only format inspection (< 1 ms target)
// ---------------------------------------------------------------------------

enum class ProbeStatus : uint8_t {
    OK,                 // header recognized; proceed to DecodeAtSize()
    UNSUPPORTED,        // this decoder does not handle this format
    TRUNCATED,          // header too short to determine format
    CORRUPT,            // header bytes indicate corrupt file
};

struct ImageMeta {
    uint32_t    width     = 0;     // 0 = unknown from header alone
    uint32_t    height    = 0;
    uint8_t     bitDepth  = 8;
    bool        hasAlpha  = false;
    bool        isHDR     = false;
    bool        isAnimated = false;
    uint32_t    frameCount = 1;
    std::string colorSpace;        // e.g. "sRGB", "AdobeRGB", "P3D65"
};

struct ProbeResult {
    ProbeStatus status   = ProbeStatus::UNSUPPORTED;
    std::string formatId;          // e.g. "JPEG", "WEBP", "AVIF"
    std::string mimeType;          // e.g. "image/jpeg"
    ImageMeta   meta;
    std::string errorMessage;      // non-empty when status != OK
};

// ---------------------------------------------------------------------------
// DecodedThumb — output of a successful DecodeAtSize() call
// ---------------------------------------------------------------------------

enum class DecodeStatus : uint8_t {
    OK,
    CANCELLED,          // stop_token was triggered during decode
    UNSUPPORTED,
    CORRUPT,
    IO_ERROR,
    OUT_OF_MEMORY,
    TIMEOUT,
};

struct DecodedThumb {
    DecodeStatus         status    = DecodeStatus::UNSUPPORTED;
    std::vector<uint8_t> pixels;   // BGRA32 row-major; stride = width * 4
    uint32_t             width     = 0;
    uint32_t             height    = 0;
    uint32_t             stride    = 0;  // bytes per row (always width*4 here)
    bool                 isPartial = false;  // embedded preview used, not full decode
    std::string          errorMessage;
};

// ---------------------------------------------------------------------------
// IStreamingDecoder — the contract every decoder must implement
// ---------------------------------------------------------------------------

class IStreamingDecoder {
public:
    virtual ~IStreamingDecoder() = default;

    // ---- Identity --------------------------------------------------------

    // Short lowercase identifier e.g. "jpeg", "webp", "avif".
    virtual const char* DecoderId() const noexcept = 0;

    // Human-readable name e.g. "JPEG (libjpeg-turbo)".
    virtual const char* DisplayName() const noexcept = 0;

    // ---- Phase 1: ProbeHeader -------------------------------------------
    //
    // Called with the first N bytes of the file (typically 16 KB, never less
    // than 32 bytes).  Must return in < 1 ms without blocking I/O.
    // The decoder MUST NOT read any further bytes from the stream here.
    //
    virtual ProbeResult ProbeHeader(std::span<const uint8_t> header) const = 0;

    // ---- Phase 2: DecodeAtSize -----------------------------------------
    //
    // Full decode of the stream to produce a BGRA32 thumbnail at targetSize×targetSize.
    // The implementation should:
    //   - Read only as much of the stream as needed (seek if possible).
    //   - Check cancel.stop_requested() at least every ~50 ms.
    //   - Return DecodeStatus::CANCELLED promptly if stop is requested.
    //   - For multi-frame/multi-page files, decode the first (or cover) frame.
    //
    // stream: seekable IStream* (cast internally via reinterpret_cast if needed)
    // targetSize: desired thumbnail edge length in pixels (e.g. 256)
    // cancel: cooperative cancellation token from Explorer's thread pool
    //
    virtual DecodedThumb DecodeAtSize(
        void*             stream,     // IStream* — void* to avoid Windows.h in header
        uint32_t          targetSize,
        std::stop_token   cancel) = 0;

    // ---- Capabilities ---------------------------------------------------

    // Returns true if this decoder can extract an embedded preview thumbnail
    // without full decode (e.g. RAW cameras with embedded JPEG, PDF preview).
    // When true, DecodeAtSize() should prefer the partial path for speed.
    virtual bool SupportsPartialDecode()   const noexcept { return false; }

    // Returns true if this decoder leverages the GPU.
    // Used by the pipeline to reserve a GPU command queue slot.
    virtual bool UsesGPU()                 const noexcept { return false; }

    // Returns the maximum file size (bytes) this decoder can handle.
    // Files larger than this are skipped.  0 = unlimited.
    virtual uint64_t MaxFileSizeBytes()    const noexcept { return 0; }

    // Returns the set of file extensions this decoder handles, lower-case.
    // e.g. { ".jpg", ".jpeg", ".jpe", ".jfif" }
    virtual std::vector<std::string> SupportedExtensions() const = 0;
};

// ---------------------------------------------------------------------------
// IStreamingDecoderFactory — create decoder instances per decode request
// ---------------------------------------------------------------------------

class IStreamingDecoderFactory {
public:
    virtual ~IStreamingDecoderFactory() = default;
    virtual std::unique_ptr<IStreamingDecoder> Create() const = 0;
    virtual const char* DecoderId() const noexcept = 0;
};

} // namespace Engine
} // namespace ExplorerLens
