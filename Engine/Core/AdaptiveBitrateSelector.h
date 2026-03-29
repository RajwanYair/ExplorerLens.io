// AdaptiveBitrateSelector.h — Adaptive Bitrate Selector
// Copyright (c) 2026 ExplorerLens Project
//
// Selects optimal HLS/DASH quality tier based on real-time bandwidth and buffer measurements.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>

namespace ExplorerLens { namespace Engine {

enum class ABRStrategy { BandwidthBased, BufferBased, Hybrid };

struct ABRProfile {
    uint32_t    bitrateKbps  = 0;
    uint32_t    width        = 0;
    uint32_t    height       = 0;
    std::string label;
};

struct ABRSelectResult {
    bool       success       = false;
    ABRProfile selectedProfile;
    float      estimatedBandwidthKbps = 0.0f;
};

class AdaptiveBitrateSelector {
public:
    explicit AdaptiveBitrateSelector(ABRStrategy strategy)
        : m_strategy(strategy), m_bandwidthKbps(5000.0f) {}

    void SetProfiles(const std::vector<ABRProfile>& profiles) { m_profiles = profiles; }

    ABRSelectResult Select(float measuredBandwidthKbps, uint32_t bufferMs) {
        ABRSelectResult r;
        if (m_profiles.empty()) return r;
        m_bandwidthKbps = measuredBandwidthKbps;
        float effectiveBw = m_strategy == ABRStrategy::BufferBased
            ? measuredBandwidthKbps * (static_cast<float>(bufferMs) / 5000.0f)
            : measuredBandwidthKbps * 0.8f;
        // Select highest profile that fits
        ABRProfile best = m_profiles.front();
        for (const auto& p : m_profiles)
            if (p.bitrateKbps <= static_cast<uint32_t>(effectiveBw)) best = p;
        r.selectedProfile           = best;
        r.estimatedBandwidthKbps    = measuredBandwidthKbps;
        r.success                   = true;
        return r;
    }
    float EstimatedBandwidth() const    { return m_bandwidthKbps; }

private:
    ABRStrategy              m_strategy;
    float                    m_bandwidthKbps;
    std::vector<ABRProfile>  m_profiles;
};

}} // namespace ExplorerLens::Engine
