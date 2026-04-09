// GainmapJPEGToneMapper.h — Google Ultra HDR Gainmap JPEG Tone Mapper
// Copyright (c) 2026 ExplorerLens Project
//
// Parses the XMP gainmap metadata embedded in Google Ultra HDR JPEG files
// (ISO 21496-1 draft) and applies gainmap-based SDR tone mapping that
// preserves local contrast for thumbnails displayed on sRGB monitors.
// Used by UltraHDRDecoder for thumbnail rendering. P50 target: < 1 ms.
//
#pragma once
#include <cstdint>
#include <memory>

namespace ExplorerLens { namespace Engine {

struct GainmapMetadata {
    float gainmapMin  = 0.0f;    // log2 gainmap minimum value
    float gainmapMax  = 1.0f;    // log2 gainmap maximum value
    float hdrCapacityMin = 0.0f; // min display capacity in log2 nits
    float hdrCapacityMax = 1.0f; // max display capacity in log2 nits
    float offsetSdr   = 1.0f / 64.0f;  // SDR weight factor
    float offsetHdr   = 1.0f / 64.0f;  // HDR weight factor
    bool  isValid     = false;
};

struct GainmapToneMapRequest {
    const uint8_t* baseBGRA     = nullptr; // SDR base JPEG decoded to BGRA32
    const uint8_t* gainmapGray  = nullptr; // Gainmap image decoded to 8-bit luma
    uint32_t       width        = 0;
    uint32_t       height       = 0;
    GainmapMetadata metadata;
    float           displayBoost = 1.0f;   // Target display SDR boost (1.0 = no boost)
};

struct GainmapToneMapResult {
    uint8_t* outputBGRA = nullptr;  // Caller-owned; width × height × 4 bytes
    bool     success    = false;
    float    processMs  = 0.0f;
};

class GainmapJPEGToneMapper {
public:
    GainmapJPEGToneMapper();
    ~GainmapJPEGToneMapper();

    // Parse gainmap XMP metadata from raw JPEG bytes.
    // Returns false if no gainmap metadata is present.
    bool ParseGainmapMetadata(const uint8_t* jpegData, size_t jpegSize,
                               GainmapMetadata& out) const noexcept;

    // Apply gainmap tone-mapping to produce an SDR thumbnail.
    GainmapToneMapResult ToneMap(const GainmapToneMapRequest& req) const noexcept;

    // Detect whether a JPEG file is an Ultra HDR gainmap JPEG.
    static bool IsUltraHDR(const uint8_t* jpegData, size_t jpegSize) noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

}} // namespace ExplorerLens::Engine
