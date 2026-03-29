// AVIFSequenceDecoder.h — Animated AVIF Multi-Frame Extractor
// Copyright (c) 2026 ExplorerLens Project
//
// Decodes animated AVIF image sequences (AV1 video track) and extracts a
// representative frame for thumbnail generation. Uses dav1d for AV1 decode
// with optional NVDEC/QuickSync hardware acceleration.
//
#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <optional>

namespace ExplorerLens {
namespace Engine {

struct AVIFSequenceInfo {
    uint32_t frameCount{0};
    uint32_t width{0};
    uint32_t height{0};
    double   durationMs{0.0};
    double   frameRateFps{0.0};
    bool     hasAlpha{false};
    bool     isHDR{false};
    uint8_t  bitDepth{8};
};

struct AVIFDecodeOptions {
    uint32_t frameIndex{0};         // 0 = first frame, UINT32_MAX = cover frame
    uint32_t maxWidth{256};
    uint32_t maxHeight{256};
    bool     useHardwareDecode{true};
    bool     tonemapHDR{true};       // map HDR10 → sRGB for display
    float    tonemapNits{203.0f};    // SDR white level for tonemapping
};

struct AVIFDecodeResult {
    std::vector<uint8_t> bgra;
    uint32_t             width{0};
    uint32_t             height{0};
    uint32_t             stride{0};
    uint32_t             frameIndex{0};
    bool                 isHDR{false};
};

class AVIFSequenceDecoder {
public:
    AVIFSequenceDecoder() {}
    ~AVIFSequenceDecoder() {}

    AVIFSequenceDecoder(const AVIFSequenceDecoder&) = delete;
    AVIFSequenceDecoder& operator=(const AVIFSequenceDecoder&) = delete;

    // Parse sequence metadata without decoding pixels.
    [[nodiscard]] std::optional<AVIFSequenceInfo> ParseInfo(
        const void* data, size_t size) const noexcept;

    // Decode a single frame.
    [[nodiscard]] std::optional<AVIFDecodeResult> DecodeFrame(
        const void* data, size_t size,
        const AVIFDecodeOptions& opts = {}) const;

    // Convenience: decode the best representative frame for preview.
    [[nodiscard]] std::optional<AVIFDecodeResult> DecodeCoverFrame(
        const void* data, size_t size, uint32_t maxDim = 256) const;

    static bool SupportsHardwareDecode() noexcept { return false; }

private:
    struct Impl;
    Impl* m_impl{nullptr};
};

} // namespace Engine
} // namespace ExplorerLens
