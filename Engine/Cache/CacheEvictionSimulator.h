// CacheEvictionSimulator.h — Cache eviction strategy simulation
// Copyright (c) 2026 ExplorerLens Project
//
// Simulates different eviction strategies (LRU, LFU, ARC) against recorded
// access patterns to identify optimal cache configuration.
//
#pragma once
#include <string>
#include <cstdint>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct CacheEvictionSimulatorConfig {
    bool enabled = true;
    uint32_t maxCacheSize = 1000;
    std::string label = "CacheEvictionSimulator";
};

class CacheEvictionSimulator {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    CacheEvictionSimulatorConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    enum class Strategy : uint8_t { LRU, LFU, FIFO, ARC, Random };

    struct SimulationResult {
        Strategy strategy;
        uint64_t hits = 0;
        uint64_t misses = 0;
        double hitRate = 0.0;
    };

    SimulationResult SimulateLRU(const std::vector<uint32_t>& accesses) const {
        SimulationResult r{ Strategy::LRU, 0, 0, 0.0 };
        std::vector<uint32_t> cache;
        for (uint32_t key : accesses) {
            auto it = std::find(cache.begin(), cache.end(), key);
            if (it != cache.end()) {
                r.hits++;
                cache.erase(it);
                cache.push_back(key);
            }
            else {
                r.misses++;
                if (cache.size() >= m_config.maxCacheSize)
                    cache.erase(cache.begin());
                cache.push_back(key);
            }
        }
        uint64_t total = r.hits + r.misses;
        r.hitRate = total > 0 ? static_cast<double>(r.hits) / total : 0.0;
        return r;
    }

private:
    bool m_initialized = false;
    CacheEvictionSimulatorConfig m_config;
};

}
} // namespace ExplorerLens::Engine
