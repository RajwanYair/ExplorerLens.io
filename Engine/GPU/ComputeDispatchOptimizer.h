// ComputeDispatchOptimizer.h — GPU dispatch dimension optimization
// Copyright (c) 2026 ExplorerLens Project
//
// Optimizes compute shader dispatch dimensions for target GPU architectures,
// aligning workgroup sizes to warp/wavefront boundaries.
//
#pragma once
#include <string>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

struct ComputeDispatchOptimizerConfig {
    bool enabled = true;
    uint32_t preferredWorkgroupSize = 256;
    uint32_t warpSize = 32;
    std::string label = "ComputeDispatchOptimizer";
};

class ComputeDispatchOptimizer {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    ComputeDispatchOptimizerConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    struct DispatchDims {
        uint32_t x = 1, y = 1, z = 1;
    };

    DispatchDims CalculateOptimal(uint32_t width, uint32_t height, uint32_t groupX = 16, uint32_t groupY = 16) const {
        DispatchDims d;
        d.x = (width + groupX - 1) / groupX;
        d.y = (height + groupY - 1) / groupY;
        d.z = 1;
        return d;
    }

    uint32_t AlignToWarp(uint32_t threadCount) const {
        return ((threadCount + m_config.warpSize - 1) / m_config.warpSize) * m_config.warpSize;
    }

    bool IsOptimalSize(uint32_t groupSize) const {
        return groupSize % m_config.warpSize == 0 && groupSize <= m_config.preferredWorkgroupSize;
    }

private:
    bool m_initialized = false;
    ComputeDispatchOptimizerConfig m_config;
};

}
} // namespace ExplorerLens::Engine
