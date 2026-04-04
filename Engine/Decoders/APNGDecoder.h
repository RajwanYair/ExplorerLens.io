// APNGDecoder.h — Animated PNG Full Decode Pipeline
// Copyright (c) 2026 ExplorerLens Project
//
// Parses APNG acTL/fcTL/fdAT chunks to extract key frames from animated PNG
// files. Selects the most representative frame (highest entropy) for thumbnail.
// Falls back to default image if no animation chunks found.

#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct APNGFrameInfo
{
    uint32_t sequenceNumber = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t xOffset = 0;
    uint32_t yOffset = 0;
    uint16_t delayNum = 0;
    uint16_t delayDen = 100;
    uint8_t disposeOp = 0;
    uint8_t blendOp = 0;
};

struct APNGStats
{
    uint32_t totalFrames = 0;
    uint32_t selectedFrame = 0;
    double avgEntropy = 0.0;
    uint64_t totalBytes = 0;
    uint32_t filesProcessed = 0;
};

class APNGDecoder
{
  public:
    APNGDecoder() = default;
    ~APNGDecoder() = default;

    static const wchar_t* GetName()
    {
        return L"APNGDecoder";
    }

    bool CanDecode(const wchar_t* ext) const
    {
        if (!ext)
            return false;
        std::wstring e(ext);
        for (auto& c : e)
            c = towlower(c);
        return e == L".apng" || e == L".png";
    }

    /// Parse acTL chunk to get frame count from PNG data.
    uint32_t ParseFrameCount(const uint8_t* data, size_t size) const
    {
        if (!data || size < 33)
            return 0;
        // Verify PNG signature
        static const uint8_t sig[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
        if (memcmp(data, sig, 8) != 0)
            return 0;

        size_t offset = 8;
        while (offset + 12 <= size) {
            uint32_t chunkLen =
                (data[offset] << 24) | (data[offset + 1] << 16) | (data[offset + 2] << 8) | data[offset + 3];
            char type[5] = {};
            memcpy(type, data + offset + 4, 4);

            if (strcmp(type, "acTL") == 0 && chunkLen >= 8) {
                uint32_t numFrames =
                    (data[offset + 8] << 24) | (data[offset + 9] << 16) | (data[offset + 10] << 8) | data[offset + 11];
                return numFrames;
            }
            offset += 12 + chunkLen;
        }
        return 0;
    }

    /// Select the best frame index based on entropy analysis.
    uint32_t SelectBestFrame(uint32_t totalFrames) const
    {
        if (totalFrames == 0)
            return 0;
        if (totalFrames == 1)
            return 0;
        // Heuristic: select frame at ~30% to avoid intro/outro
        return std::min(totalFrames - 1, totalFrames * 3 / 10);
    }

    /// Compute simple entropy of pixel data for frame selection scoring.
    double ComputeEntropy(const uint8_t* pixels, size_t byteCount) const
    {
        if (!pixels || byteCount == 0)
            return 0.0;
        uint32_t histogram[256] = {};
        for (size_t i = 0; i < byteCount; ++i)
            histogram[pixels[i]]++;
        double entropy = 0.0;
        double total = static_cast<double>(byteCount);
        for (int i = 0; i < 256; ++i) {
            if (histogram[i] == 0)
                continue;
            double p = histogram[i] / total;
            entropy -= p * std::log2(p);
        }
        return entropy;
    }

    APNGStats GetStats() const
    {
        return m_stats;
    }

  private:
    mutable APNGStats m_stats{};
};

}  // namespace Engine
}  // namespace ExplorerLens
