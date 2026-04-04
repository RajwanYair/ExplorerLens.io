// EfficientResizeDecoder.h — Decode-Time Downscaling Optimizer
// Copyright (c) 2026 ExplorerLens Project
//
// Integrates with decoders to perform resolution reduction during decode
// (libjpeg scale_num/scale_denom, libwebp scaled decode) to avoid
// decoding full-resolution images when only thumbnails are needed.
//
#pragma once

#include <algorithm>
#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

struct DecodeScaleParams
{
    uint32_t sourceWidth = 0;
    uint32_t sourceHeight = 0;
    uint32_t targetWidth = 0;
    uint32_t targetHeight = 0;
    uint32_t scaleNumerator = 1;
    uint32_t scaleDenominator = 1;
    uint32_t decodedWidth = 0;
    uint32_t decodedHeight = 0;
    double memorySavingsPercent = 0.0;
};

struct ResizeDecoderConfig
{
    uint32_t thumbnailSize = 256;
    bool enableJPEGScaledDecode = true;
    bool enableLibWebPScaledDecode = true;
    bool enableLibRawHalfSize = true;
    double maxOversampleRatio = 2.0;  // Decode at most 2x target
};

struct ResizeDecoderStats
{
    uint64_t totalDecodes = 0;
    uint64_t scaledDecodes = 0;
    uint64_t fullDecodes = 0;
    double totalMemorySavedMB = 0.0;
    double avgSavingsPercent = 0.0;
};

class EfficientResizeDecoder
{
  public:
    void Configure(const ResizeDecoderConfig& config)
    {
        m_config = config;
    }

    DecodeScaleParams ComputeJPEGScale(uint32_t srcW, uint32_t srcH, uint32_t targetW, uint32_t targetH) const
    {
        DecodeScaleParams params;
        params.sourceWidth = srcW;
        params.sourceHeight = srcH;
        params.targetWidth = targetW;
        params.targetHeight = targetH;

        // JPEG supports 1/1, 1/2, 1/4, 1/8 scaling during decode
        uint32_t ratioW = srcW / std::max(targetW, 1u);
        uint32_t ratioH = srcH / std::max(targetH, 1u);
        uint32_t ratio = std::min(ratioW, ratioH);

        if (ratio >= 8) {
            params.scaleNumerator = 1;
            params.scaleDenominator = 8;
        } else if (ratio >= 4) {
            params.scaleNumerator = 1;
            params.scaleDenominator = 4;
        } else if (ratio >= 2) {
            params.scaleNumerator = 1;
            params.scaleDenominator = 2;
        } else {
            params.scaleNumerator = 1;
            params.scaleDenominator = 1;
        }

        params.decodedWidth = srcW * params.scaleNumerator / params.scaleDenominator;
        params.decodedHeight = srcH * params.scaleNumerator / params.scaleDenominator;

        uint64_t fullBytes = static_cast<uint64_t>(srcW) * srcH * 4;
        uint64_t scaledBytes = static_cast<uint64_t>(params.decodedWidth) * params.decodedHeight * 4;
        params.memorySavingsPercent =
            fullBytes > 0 ? 100.0 * (1.0 - static_cast<double>(scaledBytes) / fullBytes) : 0.0;

        return params;
    }

    bool ShouldScale(uint32_t srcW, uint32_t srcH) const
    {
        return srcW > m_config.thumbnailSize * m_config.maxOversampleRatio
               || srcH > m_config.thumbnailSize * m_config.maxOversampleRatio;
    }

    ResizeDecoderStats GetStats() const
    {
        return m_stats;
    }

  private:
    ResizeDecoderConfig m_config;
    ResizeDecoderStats m_stats;
};

}  // namespace Engine
}  // namespace ExplorerLens
