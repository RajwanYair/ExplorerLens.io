// ThumbnailDeltaEncoder.h — Thumbnail Delta Encoder
// Copyright (c) 2026 ExplorerLens Project
//
// Encodes only the changed pixel regions between two successive BGRA thumbnail
// surfaces.  The resulting delta payload is compressed with LZMA (or raw memcpy
// for small surfaces) and transmitted to collaborative sessions instead of a full
// thumbnail, reducing network traffic by up to 90% for incremental edits.
//
#pragma once
#include <cstdint>
#include <vector>
#include <string>

namespace ExplorerLens { namespace Engine {

/// Compression method used for delta payloads.
enum class DeltaCodec : uint8_t {
    NONE  = 0,   ///< Raw bytes; used when delta >= 80% of full size
    LZMA  = 1,
    ZSTD  = 2,
};

/// A compact delta payload describing changed pixel regions.
struct ThumbnailDelta {
    uint32_t             width      = 0;
    uint32_t             height     = 0;
    uint32_t             dirtyX     = 0;  ///< Bounding box of changed region
    uint32_t             dirtyY     = 0;
    uint32_t             dirtyW     = 0;
    uint32_t             dirtyH     = 0;
    DeltaCodec           codec      = DeltaCodec::NONE;
    std::vector<uint8_t> payload;         ///< Compressed delta bytes
    bool                 isEmpty    = false; ///< True if nothing changed
};

/// Encodes/decodes thumbnail deltas.
class ThumbnailDeltaEncoder {
public:
    struct Config {
        uint32_t minDirtyPixels = 4;      ///< Below this, emit isEmpty delta
        bool     preferLZMA     = true;   ///< Prefer LZMA over ZSTD
    };

    explicit ThumbnailDeltaEncoder(const Config& cfg = {});

    /// Compute delta between two BGRA surfaces of identical dimensions.
    /// Returns an isEmpty delta if fewer than minDirtyPixels differ.
    ThumbnailDelta Encode(
        const uint8_t* prevBGRA,
        const uint8_t* currBGRA,
        uint32_t width, uint32_t height) const;

    /// Apply a delta to a BGRA surface in place.
    bool Decode(
        uint8_t*            targetBGRA,
        uint32_t            width, uint32_t height,
        const ThumbnailDelta& delta) const;

    uint64_t TotalBytesEncoded() const;
    uint64_t TotalBytesSaved()   const;
    void     ResetStats();

    const Config& GetConfig() const;

private:
    Config   m_config;
    mutable uint64_t m_totalEncoded = 0;
    mutable uint64_t m_totalSaved   = 0;
};

}} // namespace ExplorerLens::Engine
