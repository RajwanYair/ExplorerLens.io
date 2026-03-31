// ThumbnailMetrics.h — Unified Pipeline Profiling, Sizing & Monitoring
// Copyright (c) 2026 ExplorerLens Project
//
// Unified header consolidating: ThumbnailPipelineProfiler.h,
// ThumbnailSizeNegotiator.h, ThumbnailCacheWarmer.h,
// ThumbnailHistogram.h, ThumbnailDensitySelector.h.
// Part of v31.2.0 consolidation pass.
//
#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// -- Pipeline profile stages (from PipelineProfiler) --------------------------

enum class ProfileStage : uint8_t {
    Total, CacheLookup, FileOpen, FileRead, FormatDetect,
    Decode, GPUResize, PostProcess, CacheStore, COUNT
};

// -- Histogram channels (from Histogram) --------------------------------------

enum class HistogramChannel : uint8_t { Red, Green, Blue, Luminance, Alpha };

// -- Structs ------------------------------------------------------------------

struct StageTiming {
    ProfileStage stage  = ProfileStage::Total;
    double       startUs = 0.0;
    double       endUs   = 0.0;
    double       DurationUs() const { return endUs - startUs; }
};

struct PipelineProfile {
    std::array<double, static_cast<size_t>(ProfileStage::COUNT)> stageUs{};
    double TotalUs() const { return stageUs[0]; }
    double DecodeUs() const { return stageUs[static_cast<size_t>(ProfileStage::Decode)]; }
};

struct SizeNegotiationResult {
    uint32_t decodeWidth   = 0;
    uint32_t decodeHeight  = 0;
    uint32_t outputWidth   = 0;
    uint32_t outputHeight  = 0;
    float    scaleFactor   = 1.0f;
    bool     needsResize   = false;
};

struct DensityResult {
    uint32_t physicalWidth  = 256;
    uint32_t physicalHeight = 256;
    float    effectiveScale = 1.0f;
    std::string cacheKeySuffix;
};

struct HistogramData {
    static constexpr uint32_t BINS = 256;
    std::array<uint32_t, BINS> counts{};
    uint32_t totalPixels = 0;
    float mean     = 0.0f;
    float stddev   = 0.0f;
    uint8_t minVal = 255;
    uint8_t maxVal = 0;
};

struct CacheWarmerStats {
    uint32_t directoriesTracked = 0;
    uint32_t thumbnailsWarmed   = 0;
    uint32_t warmHits           = 0;
};

// -- Unified metrics engine ---------------------------------------------------

class ThumbnailMetricsEngine {
public:
    SizeNegotiationResult NegotiateSize(uint32_t srcW, uint32_t srcH,
                                        uint32_t requested, float dpiScale = 1.0f) const {
        SizeNegotiationResult r;
        uint32_t target = static_cast<uint32_t>(requested * dpiScale);
        if (srcW == 0 || srcH == 0) return r;
        float scale = static_cast<float>(target) / static_cast<float>((std::max)(srcW, srcH));
        if (scale >= 1.0f) { r.decodeWidth = srcW; r.decodeHeight = srcH; r.needsResize = false; }
        else {
            r.decodeWidth  = static_cast<uint32_t>(srcW * scale);
            r.decodeHeight = static_cast<uint32_t>(srcH * scale);
            r.needsResize  = true;
        }
        r.outputWidth  = (std::min)(r.decodeWidth, target);
        r.outputHeight = (std::min)(r.decodeHeight, target);
        r.scaleFactor  = scale;
        return r;
    }

    DensityResult SelectDensity(uint32_t logicalSize, uint32_t monitorDPI) const {
        DensityResult r;
        float scale = static_cast<float>(monitorDPI) / 96.0f;
        r.physicalWidth  = static_cast<uint32_t>(logicalSize * scale);
        r.physicalHeight = r.physicalWidth;
        r.effectiveScale = scale;
        r.cacheKeySuffix = "@" + std::to_string(monitorDPI) + "dpi";
        return r;
    }

    HistogramData ComputeHistogram(const uint8_t* channel, uint32_t pixelCount) const {
        HistogramData h;
        if (!channel || pixelCount == 0) return h;
        h.totalPixels = pixelCount;
        for (uint32_t i = 0; i < pixelCount; ++i) {
            uint8_t v = channel[i];
            ++h.counts[v];
            if (v < h.minVal) h.minVal = v;
            if (v > h.maxVal) h.maxVal = v;
        }
        double sum = 0.0;
        for (uint32_t i = 0; i < 256; ++i) sum += static_cast<double>(i) * h.counts[i];
        h.mean = static_cast<float>(sum / pixelCount);
        double varSum = 0.0;
        for (uint32_t i = 0; i < 256; ++i) {
            double d = static_cast<double>(i) - h.mean;
            varSum += d * d * h.counts[i];
        }
        h.stddev = static_cast<float>(std::sqrt(varSum / pixelCount));
        return h;
    }

    void RecordStage(ProfileStage stage, double durationUs) {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_currentProfile.stageUs[static_cast<size_t>(stage)] += durationUs;
    }

    PipelineProfile GetProfile() const {
        std::lock_guard<std::mutex> lk(m_mutex);
        return m_currentProfile;
    }

    void ResetProfile() {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_currentProfile = {};
    }

    void RecordDirectoryAccess(const std::wstring& dir) {
        std::lock_guard<std::mutex> lk(m_mutex);
        ++m_dirAccess[dir];
    }

    CacheWarmerStats GetWarmerStats() const {
        std::lock_guard<std::mutex> lk(m_mutex);
        CacheWarmerStats s;
        s.directoriesTracked = static_cast<uint32_t>(m_dirAccess.size());
        return s;
    }

private:
    mutable std::mutex m_mutex;
    PipelineProfile m_currentProfile;
    std::unordered_map<std::wstring, uint32_t> m_dirAccess;
};

} // namespace Engine
} // namespace ExplorerLens
