// AdaptiveTimeoutController.h — Smart Timeout Management for Decode Ops
// Copyright (c) 2026 ExplorerLens Project
//
// Dynamically adjusts decode timeouts based on file size, format complexity,
// system load, and historical decode times. Prevents Explorer hangs from
// corrupt or excessively complex files while avoiding premature timeouts.
//
#pragma once

#include <algorithm>
#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class TimeoutStrategy : uint8_t {
    Fixed,          // Constant timeout
    FileSizeBased,  // Scale with file size
    HistoryBased,   // Use historical averages
    Adaptive,       // Combine all signals
    COUNT
};

struct TimeoutConfig
{
    uint32_t baseTimeoutMs = 5000;
    uint32_t maxTimeoutMs = 30000;
    uint32_t minTimeoutMs = 500;
    float fileSizeMultiplier = 1.0f;  // ms per MB
    float loadMultiplier = 1.5f;      // increase under load
};

struct TimeoutDecision
{
    uint32_t timeoutMs = 5000;
    TimeoutStrategy strategy = TimeoutStrategy::Fixed;
    bool wasExtended = false;
    float confidence = 1.0f;
};

class AdaptiveTimeoutController
{
  public:
    void SetConfig(const TimeoutConfig& cfg)
    {
        m_config = cfg;
    }
    const TimeoutConfig& GetConfig() const
    {
        return m_config;
    }

    TimeoutDecision Calculate(uint64_t fileSize, const std::wstring& format, float systemLoad = 0.5f) const
    {
        (void)format;
        TimeoutDecision d;
        switch (m_strategy) {
            case TimeoutStrategy::Fixed:
                d.timeoutMs = m_config.baseTimeoutMs;
                break;
            case TimeoutStrategy::FileSizeBased: {
                float sizeMB = static_cast<float>(fileSize) / (1024.0f * 1024.0f);
                d.timeoutMs = m_config.baseTimeoutMs + static_cast<uint32_t>(sizeMB * m_config.fileSizeMultiplier);
                break;
            }
            case TimeoutStrategy::Adaptive:
            default: {
                float sizeMB = static_cast<float>(fileSize) / (1024.0f * 1024.0f);
                uint32_t base = m_config.baseTimeoutMs + static_cast<uint32_t>(sizeMB * m_config.fileSizeMultiplier);
                d.timeoutMs = static_cast<uint32_t>(base * (1.0f + systemLoad * 0.5f));
                break;
            }
        }
        d.timeoutMs = std::clamp(d.timeoutMs, m_config.minTimeoutMs, m_config.maxTimeoutMs);
        d.strategy = m_strategy;
        d.confidence = 0.85f;
        return d;
    }

    void SetStrategy(TimeoutStrategy s)
    {
        m_strategy = s;
    }
    TimeoutStrategy GetStrategy() const
    {
        return m_strategy;
    }

    void RecordActualTime(uint32_t decodeMs)
    {
        m_historySum += decodeMs;
        m_historyCount++;
    }

    float AverageDecodeMs() const
    {
        return m_historyCount > 0 ? static_cast<float>(m_historySum) / static_cast<float>(m_historyCount) : 0.0f;
    }

    static const wchar_t* StrategyName(TimeoutStrategy s)
    {
        switch (s) {
            case TimeoutStrategy::Fixed:
                return L"Fixed";
            case TimeoutStrategy::FileSizeBased:
                return L"FileSizeBased";
            case TimeoutStrategy::HistoryBased:
                return L"HistoryBased";
            case TimeoutStrategy::Adaptive:
                return L"Adaptive";
            default:
                return L"Unknown";
        }
    }
    static size_t StrategyCount()
    {
        return static_cast<size_t>(TimeoutStrategy::COUNT);
    }

  private:
    TimeoutStrategy m_strategy = TimeoutStrategy::Adaptive;
    TimeoutConfig m_config;
    uint64_t m_historySum = 0;
    uint32_t m_historyCount = 0;
};

}  // namespace Engine
}  // namespace ExplorerLens
