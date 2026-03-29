// EvictionAwareCachePrimer.h — Eviction-Aware Cache Primer
// Copyright (c) 2026 ExplorerLens Project
//
// Primes cache entries with awareness of eviction pressure to avoid wasted pre-generation.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>

namespace ExplorerLens { namespace Engine {

struct EACPConfig {
    uint32_t maxPrimeCount     = 50;
    float    pressureThreshold = 0.8f;  // 0-1 fraction of cache full
    bool     dryRunMode        = false;
};

struct EACPPrimeResult {
    bool     success       = false;
    uint32_t primedCount   = 0;
    uint32_t skippedCount  = 0;
    float    cachePressure = 0.0f;
};

class EvictionAwareCachePrimer {
public:
    explicit EvictionAwareCachePrimer(const EACPConfig& config) : m_config(config) {}

    EACPPrimeResult Prime(const std::vector<std::wstring>& candidates, float currentPressure) {
        EACPPrimeResult r;
        r.cachePressure = currentPressure;
        if (currentPressure >= m_config.pressureThreshold) {
            r.skippedCount = static_cast<uint32_t>(candidates.size());
            r.success      = true;
            return r;
        }
        uint32_t limit = std::min(m_config.maxPrimeCount, static_cast<uint32_t>(candidates.size()));
        if (!m_config.dryRunMode)
            for (uint32_t i = 0; i < limit; ++i) m_primed.push_back(candidates[i]);
        r.primedCount  = limit;
        r.skippedCount = static_cast<uint32_t>(candidates.size()) - limit;
        r.success      = true;
        return r;
    }
    uint32_t PrimedCount() const { return static_cast<uint32_t>(m_primed.size()); }

private:
    EACPConfig               m_config;
    std::vector<std::wstring> m_primed;
};

}} // namespace ExplorerLens::Engine
