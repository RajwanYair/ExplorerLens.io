// DecodeWatermarkStamper.h — Stamps debug watermarks on decoded thumbnails
// Copyright (c) 2026 ExplorerLens Project
//
// Overlays diagnostic watermarks (format name, decode time, cache status)
// on thumbnails during debug builds for visual pipeline inspection.
//
#pragma once
#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

struct DecodeWatermarkStamperConfig
{
    bool enabled = false;
    uint32_t fontSize = 10;
    std::string label = "DecodeWatermarkStamper";
};

class DecodeWatermarkStamper
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
    DecodeWatermarkStamperConfig GetConfig() const
    {
        return m_config;
    }
    std::string GetName() const
    {
        return m_config.label;
    }

    std::string FormatWatermark(const std::string& format, double decodeMs, bool cached) const
    {
        return format + " | " + std::to_string(static_cast<int>(decodeMs)) + "ms" + (cached ? " | HIT" : " | MISS");
    }

  private:
    bool m_initialized = false;
    DecodeWatermarkStamperConfig m_config;
};

}  // namespace Engine
}  // namespace ExplorerLens
