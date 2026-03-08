// GPUTextureMipChain.h — Mipmap chain generation and caching
// Copyright (c) 2026 ExplorerLens Project
//
// Generates and caches GPU mipmap chains for high-quality thumbnail
// downscaling using pre-filtered mip levels.
//
#pragma once
#include <string>
#include <cstdint>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct GPUTextureMipChainConfig {
    bool enabled = true;
    uint32_t maxMipLevels = 12;
    uint32_t minMipSize = 4;
    std::string label = "GPUTextureMipChain";
};

class GPUTextureMipChain {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    GPUTextureMipChainConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    struct MipLevel {
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t level = 0;
        uint64_t sizeBytes = 0;
    };

    uint32_t CalculateMipCount(uint32_t width, uint32_t height) const {
        uint32_t count = 1;
        while ((width > m_config.minMipSize || height > m_config.minMipSize) &&
            count < m_config.maxMipLevels) {
            width = (width > 1) ? width / 2 : 1;
            height = (height > 1) ? height / 2 : 1;
            count++;
        }
        return count;
    }

    std::vector<MipLevel> BuildChain(uint32_t w, uint32_t h, uint32_t bpp = 4) const {
        std::vector<MipLevel> chain;
        uint32_t mips = CalculateMipCount(w, h);
        for (uint32_t i = 0; i < mips; ++i) {
            chain.push_back({ w, h, i, static_cast<uint64_t>(w) * h * bpp });
            w = (w > 1) ? w / 2 : 1;
            h = (h > 1) ? h / 2 : 1;
        }
        return chain;
    }

private:
    bool m_initialized = false;
    GPUTextureMipChainConfig m_config;
};

}
} // namespace ExplorerLens::Engine
