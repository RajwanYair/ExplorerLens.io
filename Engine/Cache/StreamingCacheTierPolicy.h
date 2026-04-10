// StreamingCacheTierPolicy.h — Network-Adaptive Cache Tier Policy
// Copyright (c) 2026 ExplorerLens Project
//
// Maps the current NetworkTopology to concrete cache tier parameters:
// max budget, eviction aggressiveness, pre-fetch depth, and bandwidth cap.
//
#pragma once
#include <cstdint>
#include "NetworkTopologyProbe.h"

namespace ExplorerLens { namespace Engine {

enum class CacheEvictionAggressiveness : uint8_t {
    NONE       = 0,
    LAZY       = 1,  // Only evict when budget exceeded
    MODERATE   = 2,  // Proactive LRU trimming
    AGGRESSIVE = 3,  // Immediately trim on any pressure signal
};

struct CacheTierParameters {
    uint32_t                   maxBudgetMb          = 256;
    uint32_t                   prefetchDepth         = 4;    // # items to pre-fetch ahead
    uint32_t                   maxBandwidthKbps      = 0;    // 0 = unlimited
    CacheEvictionAggressiveness eviction              = CacheEvictionAggressiveness::LAZY;
    bool                       allowRemoteFetch      = true;
    bool                       usePlaceholders       = false;
};

class StreamingCacheTierPolicy {
public:
    CacheTierParameters Derive(NetworkTopology topology) const;
    CacheTierParameters DeriveFromProbe(const NetworkProbeResult& result) const;

    void OverrideParams(NetworkTopology topology, const CacheTierParameters& params);
    void ResetOverrides();

    uint32_t OverrideCount() const { return m_overrideCount; }

private:
    struct Override {
        bool               active = false;
        CacheTierParameters params;
    };
    Override m_overrides[6] = {}; // indexed by NetworkTopology enum
    uint32_t m_overrideCount = 0;
};

}} // namespace ExplorerLens::Engine
