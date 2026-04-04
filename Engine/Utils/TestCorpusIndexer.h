// Utils\TestCorpusIndexer.h - TestCorpusIndexer
// Copyright (c) 2026 ExplorerLens Project
//
// Indexes test file corpus for format-specific integration testing
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

class TestCorpusIndexer
{
  public:
    struct Config
    {
        bool enabled = true;
        uint32_t maxEntries = 1024;
        std::string label = "TestCorpusIndexer";
    };

    TestCorpusIndexer() = default;
    explicit TestCorpusIndexer(const Config& cfg) : m_config(cfg) {}

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
    std::string m_corpusPath{};
    std::string m_indexEntries{};
    std::string m_formatCoverage{};
};

}  // namespace Engine
}  // namespace ExplorerLens
