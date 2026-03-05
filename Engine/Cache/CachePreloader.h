// CachePreloader.h — Speculative Cache Pre-Population Engine
// Copyright (c) 2026 ExplorerLens Project
//
// Pre-populates cache entries for files that are likely to be viewed next,
// using directory listing heuristics and shell navigation patterns.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class CachePreloadStrategy : uint8_t {
    None,              // No preloading
    AdjacentFiles,     // Files near currently viewed file
    DirectoryAhead,    // Next N files in directory listing
    SiblingFolders,    // Files in sibling directories
    Smart,             // ML-predicted navigation pattern
    COUNT
};

struct PreloadTask {
    std::wstring filePath;
    uint32_t thumbnailSize = 256;
    float probability = 0.0f;
    bool completed = false;
};

struct PreloadStats {
    uint32_t tasksQueued = 0;
    uint32_t tasksCompleted = 0;
    uint32_t cacheHits = 0;  // Preloaded entry was actually accessed
    uint32_t cacheMisses = 0;  // Preloaded entry was never accessed
    float hitRate = 0.0f;
    double totalDecodeMs = 0.0;
};

class CachePreloader {
public:
    void SetStrategy(CachePreloadStrategy s) { m_strategy = s; }
    CachePreloadStrategy GetStrategy() const { return m_strategy; }

    void SetMaxPending(uint32_t n) { m_maxPending = n; }
    uint32_t MaxPending() const { return m_maxPending; }

    void QueuePreload(const PreloadTask& task) {
        if (m_tasks.size() < m_maxPending) {
            m_tasks.push_back(task);
            m_stats.tasksQueued++;
        }
    }

    void MarkCompleted(size_t index) {
        if (index < m_tasks.size()) {
            m_tasks[index].completed = true;
            m_stats.tasksCompleted++;
        }
    }

    void RecordAccess(bool wasPreloaded) {
        if (wasPreloaded) m_stats.cacheHits++;
        else m_stats.cacheMisses++;
        uint32_t total = m_stats.cacheHits + m_stats.cacheMisses;
        m_stats.hitRate = total > 0
            ? static_cast<float>(m_stats.cacheHits) / static_cast<float>(total)
            : 0.0f;
    }

    size_t PendingCount() const { return m_tasks.size(); }
    const PreloadStats& Stats() const { return m_stats; }

    static const wchar_t* StrategyName(CachePreloadStrategy s) {
        switch (s) {
        case CachePreloadStrategy::None:           return L"None";
        case CachePreloadStrategy::AdjacentFiles:  return L"AdjacentFiles";
        case CachePreloadStrategy::DirectoryAhead: return L"DirectoryAhead";
        case CachePreloadStrategy::SiblingFolders: return L"SiblingFolders";
        case CachePreloadStrategy::Smart:          return L"Smart";
        default: return L"Unknown";
        }
    }
    static size_t StrategyCount() { return static_cast<size_t>(CachePreloadStrategy::COUNT); }

private:
    CachePreloadStrategy m_strategy = CachePreloadStrategy::DirectoryAhead;
    uint32_t m_maxPending = 32;
    std::vector<PreloadTask> m_tasks;
    PreloadStats m_stats;
};

} // namespace Engine
} // namespace ExplorerLens
