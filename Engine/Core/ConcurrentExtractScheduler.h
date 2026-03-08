// ConcurrentExtractScheduler.h — Resource-aware parallel archive extraction
// Copyright (c) 2026 ExplorerLens Project
//
// Schedules parallel archive extraction jobs with configurable concurrency
// limits, memory budgets, and priority-based ordering.
//
#pragma once
#include <string>
#include <cstdint>
#include <vector>
#include <queue>

namespace ExplorerLens {
namespace Engine {

struct ConcurrentExtractSchedulerConfig {
    bool enabled = true;
    uint32_t maxConcurrent = 4;
    uint64_t memoryBudgetMB = 512;
    std::string label = "ConcurrentExtractScheduler";
};

class ConcurrentExtractScheduler {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    ConcurrentExtractSchedulerConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    struct Job {
        uint32_t id = 0;
        std::string archivePath;
        uint32_t priority = 0; // higher = more urgent
        uint64_t estimatedMemMB = 0;
    };

    bool Submit(const Job& job) {
        if (m_usedMemoryMB + job.estimatedMemMB > m_config.memoryBudgetMB)
            return false;
        m_pendingJobs.push_back(job);
        return true;
    }

    uint32_t GetPendingCount() const { return static_cast<uint32_t>(m_pendingJobs.size()); }
    uint32_t GetActiveCount() const { return m_activeCount; }

    bool CanScheduleMore() const {
        return m_activeCount < m_config.maxConcurrent;
    }

    void MarkActive(uint32_t jobId, uint64_t memMB) {
        (void)jobId;
        m_activeCount++;
        m_usedMemoryMB += memMB;
    }

    void MarkComplete(uint64_t memMB) {
        if (m_activeCount > 0) m_activeCount--;
        if (m_usedMemoryMB >= memMB) m_usedMemoryMB -= memMB;
    }

private:
    bool m_initialized = false;
    ConcurrentExtractSchedulerConfig m_config;
    std::vector<Job> m_pendingJobs;
    uint32_t m_activeCount = 0;
    uint64_t m_usedMemoryMB = 0;
};

}
} // namespace ExplorerLens::Engine
