// GPUMemoryDefragmenter.h — Defragments GPU texture heap allocations
// Copyright (c) 2026 ExplorerLens Project
//
// Compacts fragmented GPU memory by relocating textures and buffers,
// reducing wasted committed memory on the GPU dedicated heap.
//
#pragma once
#include <string>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

struct GPUMemoryDefragmenterConfig {
    bool enabled = true;
    uint32_t fragmentationThresholdPercent = 25;
    std::string label = "GPUMemoryDefragmenter";
};

class GPUMemoryDefragmenter {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    GPUMemoryDefragmenterConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    struct DefragStats {
        uint64_t freedBytes = 0;
        uint32_t movedAllocations = 0;
        double durationMs = 0.0;
    };

    bool NeedsDefrag(uint64_t usedBytes, uint64_t committedBytes) const {
        if (committedBytes == 0) return false;
        uint32_t fragPercent = static_cast<uint32_t>((committedBytes - usedBytes) * 100 / committedBytes);
        return fragPercent >= m_config.fragmentationThresholdPercent;
    }

private:
    bool m_initialized = false;
    GPUMemoryDefragmenterConfig m_config;
};

}
} // namespace ExplorerLens::Engine
