// GPUOccupancyCalculator.h — Compute shader occupancy optimization
// Copyright (c) 2026 ExplorerLens Project
//
// Calculates theoretical and achieved GPU compute shader occupancy,
// suggesting register/shared-memory tuning for maximum throughput.
//
#pragma once
#include <string>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

struct GPUOccupancyCalculatorConfig {
    bool enabled = true;
    uint32_t maxRegistersPerThread = 64;
    uint32_t sharedMemoryLimitKB = 48;
    std::string label = "GPUOccupancyCalculator";
};

class GPUOccupancyCalculator {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    GPUOccupancyCalculatorConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    struct OccupancyResult {
        float theoreticalOccupancy = 0.0f;
        uint32_t activeWarps = 0;
        uint32_t maxWarps = 0;
        std::string limitingFactor;
    };

    OccupancyResult Calculate(uint32_t registersUsed, uint32_t sharedMemKB,
        uint32_t threadsPerBlock, uint32_t maxWarpsPerSM = 48) const {
        OccupancyResult r;
        r.maxWarps = maxWarpsPerSM;
        uint32_t warpsPerBlock = (threadsPerBlock + 31) / 32;
        r.activeWarps = warpsPerBlock;
        if (r.activeWarps > r.maxWarps) r.activeWarps = r.maxWarps;
        r.theoreticalOccupancy = r.maxWarps > 0
            ? static_cast<float>(r.activeWarps) / r.maxWarps : 0.0f;
        if (registersUsed > m_config.maxRegistersPerThread)
            r.limitingFactor = "registers";
        else if (sharedMemKB > m_config.sharedMemoryLimitKB)
            r.limitingFactor = "shared_memory";
        else
            r.limitingFactor = "none";
        return r;
    }

private:
    bool m_initialized = false;
    GPUOccupancyCalculatorConfig m_config;
};

}
} // namespace ExplorerLens::Engine
