#pragma once
// ============================================================================
// ThumbnailHistogram.h — Histogram computation and analysis for thumbnail
//                        images
//
// Purpose:   Histogram computation and analysis for thumbnail images
// Provides:  HistogramChannel, HistogramType enums, HistogramData struct,
//            ThumbnailHistogram class
// Used by:   Quality assessment and auto-exposure
// ============================================================================

#include <cstdint>
#include <string>
#include <array>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

/// Histogram channel selection
enum class HistogramChannel : uint8_t {
    Red = 0,  // Red channel histogram
    Green = 1,  // Green channel histogram
    Blue = 2,  // Blue channel histogram
    Luminance = 3,  // Luminance (weighted R+G+B)
    Alpha = 4   // Alpha channel histogram
};

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

/// Number of bins in the histogram
enum class HistogramBinSize : uint16_t {
    Bin64 = 64,
    Bin128 = 128,
    Bin256 = 256,
    Bin512 = 512,
    Bin1024 = 1024
};

inline const char* HistogramBinSizeName(HistogramBinSize b) noexcept {
    switch (b) {
    case HistogramBinSize::Bin64:   return "64";
    case HistogramBinSize::Bin128:  return "128";
    case HistogramBinSize::Bin256:  return "256";
    case HistogramBinSize::Bin512:  return "512";
    case HistogramBinSize::Bin1024: return "1024";
    default:                        return "Unknown";
    }
}

/// Result of a histogram computation
struct HistogramResult {
    static constexpr uint32_t BINS_DEFAULT = 256;

    HistogramChannel channel = HistogramChannel::Luminance;
    uint32_t         binCount = BINS_DEFAULT;
    uint32_t         peakBin = 0;
    uint32_t         peakValue = 0;
    float            meanValue = 0.0f;
    uint64_t         totalPixels = 0;
    std::array<uint32_t, 256> bins = {};
};

/// Configuration for histogram generation
struct HistogramConfig {
    HistogramBinSize binSize = HistogramBinSize::Bin256;
    bool             normalize = true;     // Normalize to [0..1] range
    bool             skipAlpha = true;     // Skip fully transparent pixels
    uint32_t         sampleRate = 1;        // Process every Nth pixel (1 = all)
};

/// Computes per-channel histograms for thumbnail images,
/// providing quality metrics such as peak brightness,
/// mean luminance, and contrast range analysis.
class ThumbnailHistogram {
public:
    ThumbnailHistogram() = default;
    ~ThumbnailHistogram() = default;

    ThumbnailHistogram(const ThumbnailHistogram&) = delete;
    ThumbnailHistogram& operator=(const ThumbnailHistogram&) = delete;
    ThumbnailHistogram(ThumbnailHistogram&&) noexcept = default;
    ThumbnailHistogram& operator=(ThumbnailHistogram&&) noexcept = default;

    /// Compute histogram for a 8-bit single-channel buffer
    HistogramResult ComputeHistogram(const uint8_t* data, uint32_t pixelCount,
        HistogramChannel channel) {
        HistogramResult result;
        result.channel = channel;
        result.binCount = static_cast<uint32_t>(m_config.binSize);
        result.totalPixels = pixelCount;

        if (!data || pixelCount == 0) return result;

        result.bins.fill(0);
        uint64_t sum = 0;
        for (uint32_t i = 0; i < pixelCount; i += m_config.sampleRate) {
            uint8_t v = data[i];
            result.bins[v]++;
            sum += v;
        }
        uint32_t sampled = (pixelCount + m_config.sampleRate - 1) / m_config.sampleRate;
        result.meanValue = sampled > 0 ? static_cast<float>(sum) / sampled : 0.0f;

        // Find peak
        for (uint32_t i = 0; i < 256; i++) {
            if (result.bins[i] > result.peakValue) {
                result.peakValue = result.bins[i];
                result.peakBin = i;
            }
        }
        m_computeCount++;
        return result;
    }

    /// Get the peak bin from last computation
    uint32_t GetPeakBin(const HistogramResult& r) const noexcept {
        return r.peakBin;
    }

    /// Get mean brightness from histogram result
    float GetMeanBrightness(const HistogramResult& r) const noexcept {
        return r.meanValue;
    }

    /// Apply configuration
    void SetConfig(const HistogramConfig& cfg) noexcept { m_config = cfg; }

    /// Get computation count
    uint64_t GetComputeCount() const noexcept { return m_computeCount; }

    /// Get current config
    const HistogramConfig& GetConfig() const noexcept { return m_config; }

private:
    HistogramConfig m_config;
    uint64_t        m_computeCount = 0;
};

} // namespace Engine
} // namespace ExplorerLens
