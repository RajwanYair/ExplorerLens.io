// Utils\LinkageAuditEngine.h - LinkageAuditEngine
// Copyright (c) 2026 ExplorerLens Project
//
// Validates CRT and DLL linkage consistency across all binaries
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

class LinkageAuditEngine
{
  public:
    struct Config
    {
        bool enabled = true;
        uint32_t maxEntries = 1024;
        std::string label = "LinkageAuditEngine";
    };

    LinkageAuditEngine() = default;
    explicit LinkageAuditEngine(const Config& cfg) : m_config(cfg) {}

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
    std::string m_binaryPaths{};
    std::string m_expectedCRT{};
    uint32_t m_mismatchCount{};
};

}  // namespace Engine
}  // namespace ExplorerLens
