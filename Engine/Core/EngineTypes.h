// EngineTypes.h — Central type aliases for the ExplorerLens Engine
// Copyright (c) 2026 ExplorerLens Project
//
// This header provides the canonical type aliases and forward declarations
// for the ExplorerLens decode pipeline.  Include this instead of pulling in
// individual result-type headers from scattered locations.
//
// Design rules:
//   • COM boundary (LENSShell / LENSManager) uses HRESULT — always.
//   • Engine-internal paths use Result<T, EngineError> (via Expected.h).
//   • Decoder return type is DecodeResult<PixelBuffer> = Result<PixelBuffer, EngineError>.
//   • CLI / REST layers convert EngineError → std::string for logging.
//
#pragma once

#include "Expected.h"       // Result<T,E>, EngineError, DecodeErrorCategory

#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// ---------------------------------------------------------------------------
// PixelBuffer — BGRA32 pixel data returned by all decoders
// ---------------------------------------------------------------------------
struct PixelBuffer {
    std::vector<uint8_t> data;      // packed BGRA bytes (width * height * 4)
    uint32_t             width  = 0;
    uint32_t             height = 0;
    uint32_t             stride = 0; // bytes per row (>= width * 4)

    [[nodiscard]] uint32_t bytesPerPixel() const noexcept { return 4; }
    [[nodiscard]] size_t   totalBytes()    const noexcept { return static_cast<size_t>(stride) * height; }
    [[nodiscard]] bool     valid()         const noexcept { return width > 0 && height > 0 && !data.empty(); }

    // Zero-copy span view of pixel data
    [[nodiscard]] std::span<const uint8_t> view() const noexcept {
        return { data.data(), data.size() };
    }
};

// ---------------------------------------------------------------------------
// ThumbRequest — parameters passed from the shell extension to the engine
// ---------------------------------------------------------------------------
struct ThumbRequest {
    std::wstring    path;           // absolute file path (UTF-16)
    uint32_t        targetSize = 256; // desired thumbnail edge in pixels
    bool            gpuAllowed = true;
    bool            cancelRequested = false;
};

// ---------------------------------------------------------------------------
// ThumbResult — rich result type returned from the decode pipeline
// ---------------------------------------------------------------------------
struct ThumbResult {
    PixelBuffer     pixels;
    uint32_t        decodeTimeMs = 0;  // wall time for decode stage
    uint32_t        resizeTimeMs = 0;  // wall time for resize/post-process
    bool            fromCache    = false;
    bool            gpuAccelerated = false;
    std::string     decoderName;       // e.g. "JpegDecoder", "WebPDecoder"
};

// ---------------------------------------------------------------------------
// Canonical result type aliases
// ---------------------------------------------------------------------------

/// Result type for any decoder — returns PixelBuffer on success, EngineError on failure.
template<typename T>
using EngineResult = Result<T, EngineError>;

using DecodeOutcome  = EngineResult<PixelBuffer>;
using ThumbOutcome   = EngineResult<ThumbResult>;
using VoidOutcome    = EngineResult<void>;

// ---------------------------------------------------------------------------
// HRESULT bridge helpers
// ---------------------------------------------------------------------------

/// Convert an EngineError to a representative HRESULT for COM boundaries.
/// The original EngineError detail should be logged before calling this.
[[nodiscard]] inline HRESULT ToHRESULT(const EngineError& e) noexcept
{
    switch (e.category) {
    case DecodeErrorCategory::IO_FAILURE:       return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    case DecodeErrorCategory::FORMAT_MISMATCH:  return E_INVALIDARG;
    case DecodeErrorCategory::DECODE_OVERFLOW:  return E_OUTOFMEMORY;
    case DecodeErrorCategory::CANCELLED:        return HRESULT_FROM_WIN32(ERROR_CANCELLED);
    default:                                    return E_FAIL;
    }
}

/// Convert a failed EngineResult to an HRESULT, logging the message.
template<typename T>
[[nodiscard]] HRESULT ResultToHR(const EngineResult<T>& r) noexcept
{
    if (r.has_value()) return S_OK;
    return ToHRESULT(r.error());
}

} // namespace Engine
} // namespace ExplorerLens
