// StreamingCacheTierPolicy.cpp — Network-Adaptive Cache Tier Policy
// Copyright (c) 2026 ExplorerLens Project
//
#include "StreamingCacheTierPolicy.h"

namespace ExplorerLens { namespace Engine {

CacheTierParameters StreamingCacheTierPolicy::Derive(NetworkTopology topology) const
{
    const auto idx = static_cast<uint8_t>(topology);
    if (idx < 6 && m_overrides[idx].active)
        return m_overrides[idx].params;

    switch (topology) {
    case NetworkTopology::OFFLINE:
        return { 64, 0, 0, CacheEvictionAggressiveness::LAZY, false, true };

    case NetworkTopology::CELL:
        return { 96, 1, 512, CacheEvictionAggressiveness::AGGRESSIVE, true, true };

    case NetworkTopology::WIFI:
        return { 192, 3, 4096, CacheEvictionAggressiveness::MODERATE, true, false };

    case NetworkTopology::VPN:
        return { 192, 2, 2048, CacheEvictionAggressiveness::MODERATE, true, false };

    case NetworkTopology::LAN:
        return { 512, 8, 0, CacheEvictionAggressiveness::LAZY, true, false };

    case NetworkTopology::UNKNOWN:
    default:
        return { 128, 2, 1024, CacheEvictionAggressiveness::LAZY, true, false };
    }
}

CacheTierParameters StreamingCacheTierPolicy::DeriveFromProbe(const NetworkProbeResult& result) const
{
    auto params = Derive(result.topology);
    if (result.isMetered && params.maxBandwidthKbps == 0)
        params.maxBandwidthKbps = 512; // Enforce cap on metered connections
    return params;
}

void StreamingCacheTierPolicy::OverrideParams(NetworkTopology topology, const CacheTierParameters& params)
{
    const auto idx = static_cast<uint8_t>(topology);
    if (idx < 6) {
        if (!m_overrides[idx].active) ++m_overrideCount;
        m_overrides[idx] = { true, params };
    }
}

void StreamingCacheTierPolicy::ResetOverrides()
{
    for (auto& o : m_overrides) o = {};
    m_overrideCount = 0;
}

}} // namespace ExplorerLens::Engine
