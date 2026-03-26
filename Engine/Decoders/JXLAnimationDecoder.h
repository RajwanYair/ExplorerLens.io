// JXLAnimationDecoder.h — JPEG XL Animation Frame Decoder
// Copyright (c) 2026 ExplorerLens Project
//
// Decodes animated JPEG XL sequences (.jxl) and extracts thumbnail-quality
// frames (cover frame or first keyframe) via libjxl with optional GPU HW path.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace ExplorerLens {
namespace Engine {

// ---- Metadata ---------------------------------------------------------------

struct JXLFrameInfo {
    uint32_t frameIndex     = 0;
    float    durationSecs   = 0.0f;   // Duration of this frame in seconds
    bool     isKeyframe     = false;
    bool     isCoverFrame   = false;  // Marked via JXL frame metadata
    uint32_t width          = 0;
    uint32_t height         = 0;
};

struct JXLSequenceInfo {
    uint32_t width         = 0;
    uint32_t height        = 0;
    uint32_t frameCount    = 0;
    float    totalDuration = 0.0f;    // Total animation duration in seconds
    bool     hasHDR        = false;   // Peak luminance > SDR (203 nits reference)
    float    peakLuminance = 203.0f;  // PQ peak in nits (HDR10 = 10000+)
    bool     hasAlpha      = false;
    uint32_t bitsPerSample = 8;       // 8, 10, 12, or 16
    std::string colorSpace;           // "sRGB", "Display-P3", "Rec2100-PQ", ...
    std::vector<JXLFrameInfo> frames;
};

// ---- Options & Result -------------------------------------------------------

struct JXLDecodeOptions {
    bool     preferGPU          = true;   // Attempt HW-accelerated decode
    bool     hdrTonemapToSDR    = true;   // Apply BT.2390 tonemap if HDR input
    float    targetPeakNits     = 203.0f; // SDR reference white target
    bool     decodeCoverOnly    = true;   // Only decode 1 frame for thumbnail
    uint32_t maxDimension       = 4096;   // Downsample if either axis > this
    bool     preserveAlpha      = false;  // Composite onto white if false
};

enum class JXLDecodeStatus {
    Success             = 0,
    InvalidData         = 1,   // Not a valid JXL bitstream
    NotAnimated         = 2,   // Single-frame JXL — use base JXL decoder
    FrameNotFound       = 3,   // Requested frame index out of range
    LibraryMissing      = 4,   // libjxl not available at runtime
    HardwareFallback    = 5,   // GPU path failed; CPU fallback used (non-fatal)
    OutOfMemory         = 6,
    InternalError       = 99,
};

struct JXLDecodeResult {
    JXLDecodeStatus status       = JXLDecodeStatus::InternalError;
    std::vector<uint8_t> pixels; // BGRA, row-major, stride = width * 4
    uint32_t    width            = 0;
    uint32_t    height           = 0;
    uint32_t    frameIndex       = 0;  // Which frame was decoded
    bool        usedHardware     = false;
    bool        tonemapped       = false;
    std::string decoderVariant;        // "libjxl-cpu", "libjxl-gpu-nvdec", ...
};

// ---- JXLAnimationDecoder ----------------------------------------------------

class JXLAnimationDecoder {
public:
    JXLAnimationDecoder();
    ~JXLAnimationDecoder();

    // Probe bitstream and extract metadata without decoding pixels.
    bool ParseInfo(const uint8_t* data, size_t size, JXLSequenceInfo& outInfo) const;

    // Decode the cover frame (or first keyframe) — primary thumbnail entry-point.
    JXLDecodeResult DecodeCoverFrame(
        const uint8_t*    data,
        size_t            size,
        const JXLDecodeOptions& opts = {}) const;

    // Decode an arbitrary frame by index.
    JXLDecodeResult DecodeFrame(
        const uint8_t*    data,
        size_t            size,
        uint32_t          frameIndex,
        const JXLDecodeOptions& opts = {}) const;

    // GPU HW decode availability check (resolved once at first call).
    bool SupportsHardwareDecode() const;

    // Magic bytes probe: first 2 bytes are 0xFF 0x0A (naked codestream)
    // or 12-byte ISO BMFF container with "JXL " ftyp brand.
    static bool LooksLikeJXL(const uint8_t* data, size_t size);

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;

    JXLDecodeResult DecodeInternal(
        const uint8_t*          data,
        size_t                  size,
        uint32_t                frameIndex,
        bool                    coverOnly,
        const JXLDecodeOptions& opts) const;

    void ApplyBT2390Tonemap(
        std::vector<uint8_t>& pixels,
        uint32_t width,
        uint32_t height,
        float sourcePeakNits,
        float targetPeakNits) const;
};

} // namespace Engine
} // namespace ExplorerLens
