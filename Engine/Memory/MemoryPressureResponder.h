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

    enum class ResponderPressureLevel : uint8_t {
        None,
        Low,
        Medium,
        High,
        Critical
    };

    ResponderPressureLevel Evaluate(uint32_t memoryUsagePercent) const
    {
        if (memoryUsagePercent >= 95)
            return ResponderPressureLevel::Critical;
        if (memoryUsagePercent >= m_config.highThresholdPercent)
            return ResponderPressureLevel::High;
        if (memoryUsagePercent >= m_config.lowThresholdPercent)
            return ResponderPressureLevel::Medium;
        if (memoryUsagePercent >= 50)
            return ResponderPressureLevel::Low;
        return ResponderPressureLevel::None;
    }

    struct ResponseAction
    {
        bool evictCache = false;
        bool trimBuffers = false;
        bool reduceQuality = false;
        bool haltNewDecodes = false;
    };

    ResponseAction GetAction(ResponderPressureLevel level) const
    {
        switch (level) {
            case ResponderPressureLevel::Critical:
                return {true, true, true, true};
            case ResponderPressureLevel::High:
                return {true, true, true, false};
            case ResponderPressureLevel::Medium:
                return {true, false, false, false};
            case ResponderPressureLevel::Low:
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
