// DecoderHealthMonitor.h — Runtime decoder health tracking singleton
// Copyright (c) 2026 ExplorerLens Project
//
// Thread-safe singleton that tracks per-decoder success/failure counts
// and determines decoder availability based on health thresholds.
//
#pragma once

#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>

namespace ExplorerLens {
namespace Engine {

struct HealthStats
{
    uint64_t successCount = 0;
    uint64_t failureCount = 0;

    bool IsHealthy() const noexcept
    {
        if (successCount + failureCount == 0)
            return true;
        return failureCount * 100 / (successCount + failureCount) < 50;
    }
};

class DecoderHealthMonitor
{
  public:
    static DecoderHealthMonitor& GetInstance() noexcept
    {
        static DecoderHealthMonitor instance;
        return instance;
    }

    void ResetAll()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stats.clear();
    }

    void RecordSuccess(const std::wstring& decoderName)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stats[decoderName].successCount++;
    }

    void RecordFailure(const std::wstring& decoderName)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stats[decoderName].failureCount++;
    }

    HealthStats GetStats(const std::wstring& decoderName) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_stats.find(decoderName);
        if (it != m_stats.end())
            return it->second;
        return {};
    }

    bool IsDecoderAvailable(const std::wstring& decoderName) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_stats.find(decoderName);
        if (it == m_stats.end())
            return false;
        return it->second.IsHealthy();
    }

  private:
    DecoderHealthMonitor() = default;
    mutable std::mutex m_mutex;
    std::unordered_map<std::wstring, HealthStats> m_stats;
};

}  // namespace Engine
}  // namespace ExplorerLens
