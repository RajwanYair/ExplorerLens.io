// ThumbnailHistogram.h — Pixel histogram computation for thumbnails
// Copyright (c) 2026 ExplorerLens Project
//
// Computes luminance and channel histograms for thumbnail quality analysis.
//
#pragma once
#include <cstdint>
#include <cstring>

namespace ExplorerLens {
namespace Engine {

enum class HistogramChannel : uint8_t { Red = 0, Green = 1, Blue = 2, Luminance = 3, Alpha = 4 };

inline const char* HistogramChannelName(HistogramChannel c) noexcept {
    switch (c) {
    case HistogramChannel::Red:       return "Red";
    case HistogramChannel::Green:     return "Green";
    case HistogramChannel::Blue:      return "Blue";
    case HistogramChannel::Luminance: return "Luminance";
    case HistogramChannel::Alpha:     return "Alpha";
    default:                          return "Unknown";
    }
}

enum class HistogramBinSize : uint8_t { Bin64 = 0, Bin256 = 1, Bin1024 = 2 };

inline const char* HistogramBinSizeName(HistogramBinSize s) noexcept {
    switch (s) {
    case HistogramBinSize::Bin64:   return "64";
    case HistogramBinSize::Bin256:  return "256";
    case HistogramBinSize::Bin1024: return "1024";
    default:                        return "Unknown";
    }
}

struct HistogramResult {
    size_t   totalPixels = 0;
    float    meanValue   = 0.0f;
    uint32_t bins[256]   = {};
};

class ThumbnailHistogram {
public:
    HistogramResult ComputeHistogram(const uint8_t* data, size_t count, HistogramChannel /*channel*/) {
        HistogramResult result;
        result.totalPixels = count;
        double sum = 0.0;
        for (size_t i = 0; i < count; ++i) {
            result.bins[data[i]]++;
            sum += data[i];
        }
        if (count > 0) {
            result.meanValue = static_cast<float>(sum / static_cast<double>(count));
        }
        return result;
    }

    uint32_t GetPeakBin(const HistogramResult& result) const noexcept {
        uint32_t peakBin = 0;
        uint32_t peakCount = 0;
        for (uint32_t i = 0; i < 256; ++i) {
            if (result.bins[i] > peakCount) {
                peakCount = result.bins[i];
                peakBin = i;
            }
        }
        return peakBin;
    }

    float GetMeanBrightness(const HistogramResult& result) const noexcept {
        return result.meanValue;
    }
};

} // namespace Engine
} // namespace ExplorerLens
