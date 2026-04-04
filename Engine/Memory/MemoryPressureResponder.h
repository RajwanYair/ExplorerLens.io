// MemoryPressureResponder.h — Windows memory pressure notification handler
// Copyright (c) 2026 ExplorerLens Project
//
// Responds to Windows memory pressure notifications by triggering cache
// eviction, buffer trimming, and quality reduction based on pressure level.
//
#pragma once
#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

struct MemoryPressureResponderConfig
{
    bool enabled = true;
    uint32_t lowThresholdPercent = 70;
    uint32_t highThresholdPercent = 90;
    std::string label = "MemoryPressureResponder";
};

class MemoryPressureResponder
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
    MemoryPressureResponderConfig GetConfig() const
    {
        return m_config;
    }
    std::string GetName() const
    {
        return m_config.label;
    }

    enum class PressureLevel : uint8_t {
        None,
        Low,
        Medium,
        High,
        Critical
    };

    PressureLevel Evaluate(uint32_t memoryUsagePercent) const
    {
        if (memoryUsagePercent >= 95)
            return PressureLevel::Critical;
        if (memoryUsagePercent >= m_config.highThresholdPercent)
            return PressureLevel::High;
        if (memoryUsagePercent >= m_config.lowThresholdPercent)
            return PressureLevel::Medium;
        if (memoryUsagePercent >= 50)
            return PressureLevel::Low;
        return PressureLevel::None;
    }

    struct ResponseAction
    {
        bool evictCache = false;
        bool trimBuffers = false;
        bool reduceQuality = false;
        bool haltNewDecodes = false;
    };

    ResponseAction GetAction(PressureLevel level) const
    {
        switch (level) {
            case PressureLevel::Critical:
                return {true, true, true, true};
            case PressureLevel::High:
                return {true, true, true, false};
            case PressureLevel::Medium:
                return {true, false, false, false};
            case PressureLevel::Low:
                return {false, false, false, false};
            default:
                return {};
        }
    }

  private:
    bool m_initialized = false;
    MemoryPressureResponderConfig m_config;
};

}  // namespace Engine
}  // namespace ExplorerLens
