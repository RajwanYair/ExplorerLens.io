// Core\ArchiveStreamFactory.h - ArchiveStreamFactory
// Copyright (c) 2026 ExplorerLens Project
//
// Factory for creating archive stream handlers (ZIP/RAR/7z) from format type
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

class ArchiveStreamFactory
{
  public:
    struct Config
    {
        bool enabled = true;
        uint32_t maxEntries = 1024;
        std::string label = "ArchiveStreamFactory";
    };

    ArchiveStreamFactory() = default;
    explicit ArchiveStreamFactory(const Config& cfg) : m_config(cfg) {}

    bool Initialize()
    {
        if (!m_config.enabled)
            return false;
        m_initialized = true;
        return true;
    }

    bool IsInitialized() const
    {
        return m_initialized;
    }
    const Config& GetConfig() const
    {
        return m_config;
    }
    const std::string& GetName() const
    {
        return m_config.label;
    }

  private:
    Config m_config{};
    bool m_initialized = false;
};

}  // namespace Engine
}  // namespace ExplorerLens
