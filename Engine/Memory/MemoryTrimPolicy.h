// MemoryTrimPolicy.h — Policy engine for memory trim decisions
// Copyright (c) 2026 ExplorerLens Project
//
// Defines when and how aggressively to trim working set — responds to
// system memory notifications, idle timeouts, and explicit pressure signals.
//
#pragma once
#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

struct MemoryTrimPolicyConfig
{
    bool enabled = true;
    uint32_t idleTimeoutSec = 60;
    uint32_t trimPercentOnPressure = 50;
    std::string label = "MemoryTrimPolicy";
};

class MemoryTrimPolicy
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
    MemoryTrimPolicyConfig GetConfig() const
    {
        return m_config;
    }
    std::string GetName() const
    {
        return m_config.label;
    }

    enum class TrimLevel : uint8_t {
        None,
        Gentle,
        Moderate,
        Aggressive
    };

    TrimLevel EvaluateTrimLevel(uint32_t pressureTier, bool isIdle) const
    {
        if (pressureTier >= 4)
            return TrimLevel::Aggressive;
        if (pressureTier >= 2)
            return TrimLevel::Moderate;
        if (isIdle)
            return TrimLevel::Gentle;
        return TrimLevel::None;
    }

    uint32_t GetTrimPercent(TrimLevel level) const
    {
        switch (level) {
            case TrimLevel::Gentle:
                return 10;
            case TrimLevel::Moderate:
                return 30;
            case TrimLevel::Aggressive:
                return m_config.trimPercentOnPressure;
            default:
                return 0;
        }
    }

  private:
    bool m_initialized = false;
    MemoryTrimPolicyConfig m_config;
};

}  // namespace Engine
}  // namespace ExplorerLens
