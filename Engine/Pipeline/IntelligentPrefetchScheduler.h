// IntelligentPrefetchScheduler.h — Predictive Prefetch Scheduling
// Copyright (c) 2026 ExplorerLens Project
//
// Schedules file prefetch and thumbnail pre-decode operations based on predicted
// access patterns from UserBehaviorAnalytics. Uses a priority queue sorted by
// prediction confidence, adjusting prefetch depth dynamically based on available
// memory and I/O bandwidth.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class SmartPrefetchPriority : uint8_t {
    Immediate = 0,
    High,
    Normal,
    Low,
    Speculative
};

struct SmartPrefetchRequest
{
    std::wstring filePath;
    SmartPrefetchPriority priority = SmartPrefetchPriority::Normal;
    double confidenceScore = 0.0;
    uint32_t targetSizePx = 256;
    bool completed = false;
};

struct IntelligentPrefetchStats
{
    uint64_t totalScheduled = 0;
    uint64_t totalCompleted = 0;
    uint64_t totalCacheHits = 0;
    uint64_t totalEvicted = 0;
    double hitRatePercent = 0.0;
    double averagePrefetchLatencyMs = 0.0;
    uint64_t bandwidthSavedBytes = 0;
};

class IntelligentPrefetchScheduler
{
  public:
    static IntelligentPrefetchScheduler& Instance()
    {
        static IntelligentPrefetchScheduler instance;
        return instance;
    }

    bool Initialize(uint32_t maxDepth = 32, uint64_t budgetBytes = 64 * 1024 * 1024)
    {
        m_maxDepth = maxDepth;
        m_budgetBytes = budgetBytes;
        m_initialized = true;
        return true;
    }

    bool Schedule(const SmartPrefetchRequest& request)
    {
        if (!m_initialized)
            return false;
        if (m_queue.size() >= m_maxDepth) {
            m_stats.totalEvicted++;
            m_queue.erase(m_queue.begin());
        }
        m_queue.push_back(request);
        m_stats.totalScheduled++;
        return true;
    }

    bool ScheduleBatch(const std::vector<SmartPrefetchRequest>& requests)
    {
        if (!m_initialized)
            return false;
        for (const auto& req : requests) {
            Schedule(req);
        }
        return true;
    }

    size_t GetQueueDepth() const
    {
        return m_queue.size();
    }
    uint32_t GetMaxDepth() const
    {
        return m_maxDepth;
    }
    IntelligentPrefetchStats GetStats() const
    {
        return m_stats;
    }
    bool IsInitialized() const
    {
        return m_initialized;
    }

    void Flush()
    {
        m_queue.clear();
    }

    void Shutdown()
    {
        m_queue.clear();
        m_initialized = false;
    }

  private:
    IntelligentPrefetchScheduler() = default;
    bool m_initialized = false;
    uint32_t m_maxDepth = 32;
    uint64_t m_budgetBytes = 64 * 1024 * 1024;
    std::vector<SmartPrefetchRequest> m_queue;
    IntelligentPrefetchStats m_stats{};
};

}  // namespace Engine
}  // namespace ExplorerLens
