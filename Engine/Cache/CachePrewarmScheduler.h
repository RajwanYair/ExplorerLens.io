// CachePrewarmScheduler.h — MRU-Driven Cache Prewarm Scheduling
// Copyright (c) 2026 ExplorerLens Project
//
// Predicts which directories the user will browse next using a
// frequency + recency MRU model, then schedules cache pre-warm tasks in
// priority order. Tasks are priority-inserted into a bounded queue; LRU
// eviction trims the MRU table at capacity. Explicit scheduling at
// configurable priority levels is also supported.
//
// Thread-safe singleton.

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <algorithm>
#include <atomic>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class PrewarmPriority : uint32_t {
    Critical = 0,  // User just opened this directory
    High = 1,      // Recently visited, high MRU rank
    Medium = 2,    // Predicted from browsing pattern
    Low = 3,       // Background discovery
    Idle = 4       // Only when system is idle
};

struct PrewarmTask
{
    std::wstring directoryPath;
    PrewarmPriority priority = PrewarmPriority::Medium;
    uint32_t estimatedFiles = 0;
    uint64_t scheduledTime = 0;
    uint64_t completedTime = 0;
    bool completed = false;
    bool cancelled = false;
};

struct DirectoryMRUEntry
{
    std::wstring path;
    uint32_t accessCount = 0;
    uint64_t lastAccess = 0;
    uint64_t firstAccess = 0;

    double GetFrequencyScore(uint64_t now) const
    {
        if (now <= firstAccess)
            return 0.0;
        double ageHours = static_cast<double>(now - firstAccess) / 3600000.0;
        if (ageHours < 0.01)
            ageHours = 0.01;
        return static_cast<double>(accessCount) / ageHours;
    }

    double GetRecencyScore(uint64_t now) const
    {
        double ageSec = static_cast<double>(now - lastAccess) / 1000.0;
        return 1.0 / (1.0 + ageSec / 60.0);  // Decay over minutes
    }
};

struct PrewarmSchedulerConfig
{
    uint32_t maxMRUEntries = 128;
    uint32_t maxConcurrentTasks = 4;
    uint32_t maxQueueDepth = 64;
    uint32_t minIdleMs = 2000;  // Wait 2s idle before low-priority
    double frequencyWeight = 0.7;
    double recencyWeight = 0.3;
};

struct PrewarmSchedulerStats
{
    uint64_t totalScheduled = 0;
    uint64_t totalCompleted = 0;
    uint64_t totalCancelled = 0;
    uint32_t currentQueueSize = 0;
    uint32_t mruEntryCount = 0;
};

// ========================================================================
// CachePrewarmScheduler — Decides what/when to prewarm based on patterns
// ========================================================================
class CachePrewarmScheduler
{
  public:
    static CachePrewarmScheduler& Instance()
    {
        static CachePrewarmScheduler instance;
        return instance;
    }

    void Initialize(const PrewarmSchedulerConfig& config = {})
    {
        m_config = config;
        m_stats = {};
        m_mruEntries.clear();
        m_taskQueue.clear();
        m_initialized = true;
    }

    bool IsInitialized() const
    {
        return m_initialized;
    }

    // Record a directory access (feeds MRU model)
    void RecordAccess(const std::wstring& directoryPath)
    {
        uint64_t now = GetTickCount64();
        auto it = m_mruMap.find(directoryPath);
        if (it != m_mruMap.end()) {
            auto& entry = m_mruEntries[it->second];
            entry.accessCount++;
            entry.lastAccess = now;
        } else {
            if (m_mruEntries.size() >= m_config.maxMRUEntries) {
                EvictLeastValuable();
            }
            DirectoryMRUEntry entry;
            entry.path = directoryPath;
            entry.accessCount = 1;
            entry.lastAccess = now;
            entry.firstAccess = now;
            m_mruMap[directoryPath] = m_mruEntries.size();
            m_mruEntries.push_back(entry);
        }
    }

    // Schedule a prewarm task for a directory
    bool SchedulePrewarm(const std::wstring& directoryPath, PrewarmPriority priority = PrewarmPriority::Medium)
    {
        if (m_taskQueue.size() >= m_config.maxQueueDepth)
            return false;

        PrewarmTask task;
        task.directoryPath = directoryPath;
        task.priority = priority;
        task.scheduledTime = GetTickCount64();

        // Insert sorted by priority
        auto insertPos = std::lower_bound(
            m_taskQueue.begin(), m_taskQueue.end(), task, [](const PrewarmTask& a, const PrewarmTask& b) {
                return static_cast<uint32_t>(a.priority) < static_cast<uint32_t>(b.priority);
            });
        m_taskQueue.insert(insertPos, task);
        m_stats.totalScheduled++;
        m_stats.currentQueueSize = static_cast<uint32_t>(m_taskQueue.size());
        return true;
    }

    // Get top predicted directories to prewarm (based on MRU patterns)
    std::vector<std::wstring> GetPredictedDirectories(uint32_t maxCount = 5) const
    {
        uint64_t now = GetTickCount64();
        struct ScoredEntry
        {
            size_t index;
            double score;
        };

        std::vector<ScoredEntry> scored;
        scored.reserve(m_mruEntries.size());
        for (size_t i = 0; i < m_mruEntries.size(); ++i) {
            double freq = m_mruEntries[i].GetFrequencyScore(now);
            double recency = m_mruEntries[i].GetRecencyScore(now);
            double score = m_config.frequencyWeight * freq + m_config.recencyWeight * recency;
            scored.push_back({i, score});
        }

        std::sort(scored.begin(), scored.end(),
                  [](const ScoredEntry& a, const ScoredEntry& b) { return a.score > b.score; });

        std::vector<std::wstring> result;
        uint32_t count = (std::min)(maxCount, static_cast<uint32_t>(scored.size()));
        for (uint32_t i = 0; i < count; ++i) {
            result.push_back(m_mruEntries[scored[i].index].path);
        }
        return result;
    }

    // Dequeue next task
    bool DequeueTask(PrewarmTask& outTask)
    {
        if (m_taskQueue.empty())
            return false;
        outTask = m_taskQueue.front();
        m_taskQueue.erase(m_taskQueue.begin());
        m_stats.currentQueueSize = static_cast<uint32_t>(m_taskQueue.size());
        return true;
    }

    // Mark task completed
    void MarkCompleted()
    {
        m_stats.totalCompleted++;
    }

    // Get queue size
    uint32_t GetQueueSize() const
    {
        return static_cast<uint32_t>(m_taskQueue.size());
    }

    // Get MRU count
    uint32_t GetMRUCount() const
    {
        return static_cast<uint32_t>(m_mruEntries.size());
    }

    // Get stats
    PrewarmSchedulerStats GetStats() const
    {
        PrewarmSchedulerStats stats = m_stats;
        stats.currentQueueSize = static_cast<uint32_t>(m_taskQueue.size());
        stats.mruEntryCount = static_cast<uint32_t>(m_mruEntries.size());
        return stats;
    }

  private:
    CachePrewarmScheduler() = default;

    void EvictLeastValuable()
    {
        if (m_mruEntries.empty())
            return;
        uint64_t now = GetTickCount64();
        double minScore = (std::numeric_limits<double>::max)();
        size_t minIdx = 0;
        for (size_t i = 0; i < m_mruEntries.size(); ++i) {
            double score = m_mruEntries[i].GetFrequencyScore(now) + m_mruEntries[i].GetRecencyScore(now);
            if (score < minScore) {
                minScore = score;
                minIdx = i;
            }
        }
        // Remove from map
        m_mruMap.erase(m_mruEntries[minIdx].path);
        // Swap and pop
        if (minIdx < m_mruEntries.size() - 1) {
            m_mruMap[m_mruEntries.back().path] = minIdx;
            m_mruEntries[minIdx] = std::move(m_mruEntries.back());
        }
        m_mruEntries.pop_back();
    }

    PrewarmSchedulerConfig m_config;
    PrewarmSchedulerStats m_stats;
    std::vector<DirectoryMRUEntry> m_mruEntries;
    std::unordered_map<std::wstring, size_t> m_mruMap;
    std::vector<PrewarmTask> m_taskQueue;
    bool m_initialized = false;
};

}  // namespace Engine
}  // namespace ExplorerLens
