// CompressionTierOptimizer.h — Adaptive Compression Tier Selection
// Copyright (c) 2026 ExplorerLens Project
//
// Adaptive compression tier selection. Chooses LZ4/Zstd/None based on
// data compressibility analysis and access frequency patterns.
//
#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <mutex>
#include <cmath>
#include <numeric>

namespace ExplorerLens {
namespace Engine {

enum class CompressionTier : uint8_t {
    None,
    LZ4Fast,
    LZ4Default,
    ZstdFast,
    ZstdDefault,
    ZstdMax
};

struct CompressibilityProfile {
    double entropy = 0.0;
    double repetitionRatio = 0.0;
    double estimatedRatio = 1.0;
    size_t sampleSize = 0;
    bool isHighlyCompressible = false;
};

struct TierDecision {
    CompressionTier tier = CompressionTier::None;
    double expectedRatio = 1.0;
    double expectedSpeedMBps = 0.0;
    double confidenceScore = 0.0;
    std::string reason;
};

struct TierStats {
    uint64_t totalDecisions = 0;
    uint64_t tierCounts[6] = {};
    double avgCompressionRatio = 1.0;
    double totalBytesSaved = 0.0;
};

class CompressionTierOptimizer {
public:
    static CompressionTierOptimizer& Instance() {
        static CompressionTierOptimizer instance;
        return instance;
    }

    inline CompressibilityProfile AnalyzeCompressibility(const uint8_t* data, size_t size) const {
        CompressibilityProfile profile;
        if (!data || size == 0) return profile;

        size_t sampleCount = (std::min)(size, static_cast<size_t>(4096));
        profile.sampleSize = sampleCount;

        uint64_t freq[256] = {};
        for (size_t i = 0; i < sampleCount; ++i) {
            freq[data[i]]++;
        }

        double entropy = 0.0;
        for (int i = 0; i < 256; ++i) {
            if (freq[i] > 0) {
                double p = static_cast<double>(freq[i]) / sampleCount;
                entropy -= p * std::log2(p);
            }
        }
        profile.entropy = entropy;

        uint64_t repeats = 0;
        for (size_t i = 1; i < sampleCount; ++i) {
            if (data[i] == data[i - 1]) ++repeats;
        }
        profile.repetitionRatio = static_cast<double>(repeats) / (std::max)(static_cast<size_t>(1), sampleCount - 1);

        profile.estimatedRatio = entropy / 8.0;
        if (profile.estimatedRatio < 0.01) profile.estimatedRatio = 0.01;
        profile.isHighlyCompressible = (entropy < 4.0);

        return profile;
    }

    inline TierDecision SelectTier(const CompressibilityProfile& profile, uint32_t accessFrequency = 1,
        size_t dataSize = 0) const {
        TierDecision decision;

        if (profile.entropy > 7.5) {
            decision.tier = CompressionTier::None;
            decision.expectedRatio = 1.0;
            decision.expectedSpeedMBps = 10000.0;
            decision.reason = "Near-random data, compression would waste CPU";
            decision.confidenceScore = 0.9;
            return decision;
        }

        if (accessFrequency > 100) {
            if (profile.entropy < 5.0) {
                decision.tier = CompressionTier::LZ4Fast;
                decision.expectedSpeedMBps = 4000.0;
                decision.reason = "Hot data, LZ4 fast for low decompression latency";
            }
            else {
                decision.tier = CompressionTier::None;
                decision.expectedSpeedMBps = 10000.0;
                decision.reason = "Hot data with moderate entropy, skip compression";
            }
        }
        else if (profile.isHighlyCompressible) {
            if (dataSize > 1024 * 1024) {
                decision.tier = CompressionTier::ZstdDefault;
                decision.expectedSpeedMBps = 500.0;
                decision.reason = "Large highly compressible data, Zstd default";
            }
            else {
                decision.tier = CompressionTier::LZ4Default;
                decision.expectedSpeedMBps = 3000.0;
                decision.reason = "Small compressible data, LZ4 default";
            }
        }
        else {
            decision.tier = CompressionTier::LZ4Fast;
            decision.expectedSpeedMBps = 4000.0;
            decision.reason = "Moderate compressibility, LZ4 fast";
        }

        decision.expectedRatio = profile.estimatedRatio;
        decision.confidenceScore = 1.0 - (profile.entropy / 8.0) * 0.5;
        return decision;
    }

    inline std::string TierToString(CompressionTier tier) const {
        switch (tier) {
        case CompressionTier::None:       return "None";
        case CompressionTier::LZ4Fast:    return "LZ4 Fast";
        case CompressionTier::LZ4Default: return "LZ4 Default";
        case CompressionTier::ZstdFast:   return "Zstd Fast";
        case CompressionTier::ZstdDefault:return "Zstd Default";
        case CompressionTier::ZstdMax:    return "Zstd Max";
        default:                          return "Unknown";
        }
    }

private:
    CompressionTierOptimizer() = default;
};

}
} // namespace ExplorerLens::Engine
