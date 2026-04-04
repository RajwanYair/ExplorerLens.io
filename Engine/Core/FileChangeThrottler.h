// FileChangeThrottler.h — Filesystem Change Event Throttling
// Copyright (c) 2026 ExplorerLens Project
//
// Debounces rapid filesystem change notifications to prevent
// excessive thumbnail regeneration during bulk file operations.
//
#pragma once

#include <chrono>
#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>

namespace ExplorerLens {
namespace Engine {

struct ThrottleConfig
{
    uint32_t debounceMs = 200;
    uint32_t maxBatchSize = 100;
    uint32_t cooldownMs = 1000;
    bool coalesceByFolder = true;
};

struct ThrottleStats
{
    uint64_t eventsReceived = 0;
    uint64_t eventsDelivered = 0;
    uint64_t eventsCoalesced = 0;

    double coalesceRatio() const
    {
        return eventsReceived > 0 ? static_cast<double>(eventsCoalesced) / eventsReceived : 0.0;
    }
};

class FileChangeThrottler
{
  public:
    explicit FileChangeThrottler(ThrottleConfig cfg = {}) : m_config(cfg) {}

    bool ShouldProcess(const std::wstring& path)
    {
        std::lock_guard lock(m_mtx);
        m_stats.eventsReceived++;
        auto now = std::chrono::steady_clock::now();

        auto it = m_lastEvent.find(path);
        if (it != m_lastEvent.end()) {
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - it->second).count();
            if (elapsed < m_config.debounceMs) {
                m_stats.eventsCoalesced++;
                return false;
            }
        }
        m_lastEvent[path] = now;
        m_stats.eventsDelivered++;
        return true;
    }

    void Reset()
    {
        std::lock_guard lock(m_mtx);
        m_lastEvent.clear();
    }

    ThrottleStats GetStats() const
    {
        return m_stats;
    }
    const ThrottleConfig& Config() const
    {
        return m_config;
    }

  private:
    ThrottleConfig m_config;
    ThrottleStats m_stats;
    std::mutex m_mtx;
    std::unordered_map<std::wstring, std::chrono::steady_clock::time_point> m_lastEvent;
};

}  // namespace Engine
}  // namespace ExplorerLens
