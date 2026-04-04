// Utils\DocCoverageAnalyzer.h - DocCoverageAnalyzer
// Copyright (c) 2026 ExplorerLens Project
//
// Analyzes documentation coverage gaps across public APIs
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

class DocCoverageAnalyzer
{
  public:
    struct Config
    {
        bool enabled = true;
        uint32_t maxEntries = 1024;
        std::string label = "DocCoverageAnalyzer";
    };

    DocCoverageAnalyzer() = default;
    explicit DocCoverageAnalyzer(const Config& cfg) : m_config(cfg) {}

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
    std::string m_apiSymbols{};
    uint32_t m_documentedCount{};
    uint32_t m_coveragePercent{};
};

}  // namespace Engine
}  // namespace ExplorerLens
