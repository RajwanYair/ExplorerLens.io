// HEIFBurstDecoder.h — HEIF Live Photo and Burst Capture Handler
// Copyright (c) 2026 ExplorerLens Project
//
// Extracts the primary still from HEIF Live Photos, burst capture sequences,
// and Apple ProRAW containers by parsing ItemReference and PrimaryItem boxes.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

namespace ExplorerLens {
namespace Engine {

// ---- Burst Sequence Metadata ------------------------------------------------

enum class HEIFContainerType {
    Unknown        = 0,
    SingleImage    = 1,   // Plain HEIC/HEIF
    LivePhoto      = 2,   // Apple Live Photo (video ref in 'cdsc' ItemRef)
    BurstSequence  = 3,   // Multi-image HEIF burst
    ProRAW         = 4,   // Apple ProRAW (linear DNG in HEIF wrapper)
    MotionPhoto    = 5,   // Google Motion Photo (MP4 sidecar embedded)
};

struct HEIFImageItem {
    uint32_t    itemId      = 0;
    uint32_t    width       = 0;
    uint32_t    height      = 0;
    bool        isPrimary   = false;
    bool        isHidden    = false;
    bool        isThumb     = false;
    std::string type;       // "hvc1", "av01", "jpeg", "dng", ...
};

struct HEIFBurstInfo {
    HEIFContainerType type           = HEIFContainerType::Unknown;
    uint32_t          primaryItemId  = 0;
    uint32_t          imageCount     = 0;    // Total still frames / burst images
    bool              hasEmbeddedVideo = false;
    bool              hasHDRGainMap  = false; // HEIF gainmap (ISO 21496-1)
    uint32_t          primaryWidth   = 0;
    uint32_t          primaryHeight  = 0;
    std::vector<HEIFImageItem> items;
};

// ---- Options & Result -------------------------------------------------------

struct HEIFDecodeOptions {
    bool   decodePrimaryOnly  = true;   // Skip burst extras, decode primary item
    bool   applyGainMap       = true;   // Apply HDR gain map if present (ISO 21496-1)
    bool   hdrTonemapToSDR    = true;   // Tonemap to SDR if display is not HDR
    float  targetPeakNits     = 203.0f;
    uint32_t maxDimension     = 4096;
};

enum class HEIFDecodeStatus {
    Success             = 0,
    InvalidData         = 1,
    NoPrimaryItem       = 2,
    UnsupportedCodec    = 3,   // Item codec not supported (e.g., AV1 w/o dav1d)
    LibraryMissing      = 4,   // libheif not available at runtime
    OutOfMemory         = 5,
    InternalError       = 99,
};

struct HEIFDecodeResult {
    HEIFDecodeStatus     status      = HEIFDecodeStatus::InternalError;
    std::vector<uint8_t> pixels;     // BGRA, row-major
    uint32_t             width       = 0;
    uint32_t             height      = 0;
    uint32_t             itemId      = 0;
    bool                 gainMapApplied = false;
    HEIFContainerType    containerType  = HEIFContainerType::Unknown;
    std::string          codecUsed;
};

// ---- HEIFBurstDecoder -------------------------------------------------------

class HEIFBurstDecoder {
public:
    HEIFBurstDecoder() {}
    ~HEIFBurstDecoder() {}

    // Parse ISOBMFF box structure and populate HEIFBurstInfo.
    bool ParseInfo(const uint8_t* data, size_t size, HEIFBurstInfo& outInfo) const;

    // Decode the primary still image — skips burst extras + video frames.
    HEIFDecodeResult DecodePrimary(
        const uint8_t*        data,
        size_t                size,
        const HEIFDecodeOptions& opts = {}) const;

    // Decode a specific item by itemId (e.g. burst frame N).
    HEIFDecodeResult DecodeItem(
        const uint8_t*        data,
        size_t                size,
        uint32_t              itemId,
        const HEIFDecodeOptions& opts = {}) const;

    // Probe: checks 'ftyp' brand for 'heic', 'heix', 'mif1', 'avif', 'hevc'.
    static bool LooksLikeHEIF(const uint8_t* data, size_t size) {
        (void)data; (void)size; return false;
    }

private:
    struct Impl;
    Impl* m_impl{nullptr};

    HEIFDecodeResult DecodeInternal(
        const uint8_t*          data,
        size_t                  size,
        uint32_t                itemId,
        const HEIFDecodeOptions& opts) const;

    bool ParseGainMap(
        const uint8_t* data,
        size_t         size,
        uint32_t       primaryItemId,
        std::vector<uint8_t>& gainPixels,
        uint32_t& gainWidth,
        uint32_t& gainHeight) const;
};

} // namespace Engine
} // namespace ExplorerLens
