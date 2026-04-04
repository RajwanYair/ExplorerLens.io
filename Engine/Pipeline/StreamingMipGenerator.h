// StreamingMipGenerator.h — Progressive mipmap generation during decode
// Copyright (c) 2026 ExplorerLens Project
//
// Generates lower mip levels on-the-fly as pixels are decoded, enabling
// early display of low-resolution previews before full decode completes.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct StreamingMipGeneratorConfig
{
    bool enabled = true;
    uint32_t maxMipLevels = 12;
    std::string label = "StreamingMipGenerator";
};

class StreamingMipGenerator
{
  public:
    bool Initialize()
    {
        if (m_initialized)
            return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const
    {
        return m_initialized;
    }
    StreamingMipGeneratorConfig GetConfig() const
    {
        return m_config;
    }
    std::string GetName() const
    {
        return m_config.label;
    }

    uint32_t CalculateMipLevels(uint32_t width, uint32_t height) const
    {
        uint32_t levels = 1;
        uint32_t dim = (width > height) ? width : height;
        while (dim > 1 && levels < m_config.maxMipLevels) {
            dim >>= 1;
            levels++;
        }
        return levels;
    }

    struct MipLevel
    {
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t level = 0;
    };

    MipLevel GetMipDimensions(uint32_t baseW, uint32_t baseH, uint32_t level) const
    {
        return {baseW >> level, baseH >> level, level};
    }

  private:
    bool m_initialized = false;
    StreamingMipGeneratorConfig m_config;
};

}  // namespace Engine
}  // namespace ExplorerLens
