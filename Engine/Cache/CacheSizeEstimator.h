// CacheSizeEstimator.h — Estimates memory cost of cache entries before insertion
// Copyright (c) 2026 ExplorerLens Project
//
// Calculates the memory footprint of a decoded thumbnail before caching,
// factoring in pixel format, metadata, alignment, and overhead.
//
#pragma once
#include <string>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

struct CacheSizeEstimatorConfig {
    bool enabled = true;
    uint32_t alignmentBytes = 64;
    std::string label = "CacheSizeEstimator";
};

class CacheSizeEstimator {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    CacheSizeEstimatorConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    uint64_t EstimatePixelBytes(uint32_t width, uint32_t height, uint32_t bpp) const {
        uint64_t rowBytes = (static_cast<uint64_t>(width) * bpp + 7) / 8;
        uint64_t alignedRow = (rowBytes + m_config.alignmentBytes - 1) & ~(static_cast<uint64_t>(m_config.alignmentBytes) - 1);
        return alignedRow * height;
    }

    uint64_t EstimateTotalBytes(uint32_t width, uint32_t height, uint32_t bpp, uint32_t metadataBytes) const {
        return EstimatePixelBytes(width, height, bpp) + metadataBytes + 128; // 128 = header overhead
    }

private:
    bool m_initialized = false;
    CacheSizeEstimatorConfig m_config;
};

}
} // namespace ExplorerLens::Engine
