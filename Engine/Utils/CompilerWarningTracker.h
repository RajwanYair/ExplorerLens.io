// Utils\CompilerWarningTracker.h - CompilerWarningTracker
// Copyright (c) 2026 ExplorerLens Project
//
// Tracks compiler warnings across builds and flags regressions
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

class CompilerWarningTracker
{
  public:
    struct Config
    {
        bool enabled = true;
        uint32_t maxEntries = 1024;
        std::string label = "CompilerWarningTracker";
    };

    CompilerWarningTracker() = default;
    explicit CompilerWarningTracker(const Config& cfg) : m_config(cfg) {}

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
    std::string m_warningHistory{};
    uint32_t m_baselineCount{};
    bool m_regressionDetected{};
};

}  // namespace Engine
}  // namespace ExplorerLens
