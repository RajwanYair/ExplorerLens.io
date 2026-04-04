// DecoderCapabilityProbe.h — Probes decoder availability and capabilities
// Copyright (c) 2026 ExplorerLens Project
//
// Queries each decoder to determine its current capability set — supported
// color depths, max dimensions, feature flags, and library availability.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct DecoderCapabilityProbeConfig
{
    bool enabled = true;
    uint32_t maxDecoders = 64;
    std::string label = "DecoderCapabilityProbe";
};

class DecoderCapabilityProbe
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
    DecoderCapabilityProbeConfig GetConfig() const
    {
        return m_config;
    }
    std::string GetName() const
    {
        return m_config.label;
    }

    struct Capability
    {
        std::string decoderName;
        bool isAvailable = false;
        uint32_t maxWidth = 0;
        uint32_t maxHeight = 0;
        uint32_t supportedBPP = 32;
        bool supportsAlpha = false;
        bool supportsAnimation = false;
    };

    bool RegisterCapability(const Capability& cap)
    {
        if (m_capabilities.size() >= m_config.maxDecoders)
            return false;
        m_capabilities.push_back(cap);
        return true;
    }

    size_t GetAvailableDecoderCount() const
    {
        size_t count = 0;
        for (const auto& c : m_capabilities)
            if (c.isAvailable)
                count++;
        return count;
    }

  private:
    bool m_initialized = false;
    DecoderCapabilityProbeConfig m_config;
    std::vector<Capability> m_capabilities;
};

}  // namespace Engine
}  // namespace ExplorerLens
