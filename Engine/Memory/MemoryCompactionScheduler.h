// MemoryCompactionScheduler.h — Scheduled Memory Compaction
// Copyright (c) 2026 ExplorerLens Project
//
// Provides a scheduler that triggers memory compaction based on configurable
// triggers such as idle time, allocation pressure, or periodic intervals.
//
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <cstdint>
#include <vector>
#include <string>
#include <algorithm>
#include <chrono>
#include <mutex>

namespace ExplorerLens {
namespace Engine {

enum class CompactionTrigger : uint32_t {
    Idle = 0,
    Pressure = 1,
    Periodic = 2,
    Manual = 3,
    LowMemory = 4,
    AppMinimized = 5
};

struct CompactionStats {
    uint64_t totalCompactions = 0;
    uint64_t bytesRecovered = 0;
    uint64_t avgCompactionTimeUs = 0;
    uint64_t lastCompactionTimeUs = 0;
    uint64_t peakReclaimBytes = 0;
    CompactionTrigger lastTrigger = CompactionTrigger::Idle;
    double   successRate = 1.0;

    uint64_t AverageRecovery() const {
        return totalCompactions > 0 ? bytesRecovered / totalCompactions : 0;
    }
};

class MemoryCompactionScheduler {
public:
    static MemoryCompactionScheduler& Instance() {
        static MemoryCompactionScheduler s;
        return s;
    }

    void ScheduleCompaction(CompactionTrigger trigger, uint32_t delayMs = 0) {
        std::lock_guard<std::mutex> lock(m_mutex);
        ScheduledEntry entry{};
        entry.trigger = trigger;
        entry.scheduledTime = GetTickCount64() + delayMs;
        entry.executed = false;
        m_scheduled.push_back(entry);
    }

    uint64_t RunImmediately() {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto start = std::chrono::high_resolution_clock::now();

        HANDLE heap = GetProcessHeap();
        uint64_t recovered = 0;
        if (heap) {
            if (HeapCompact(heap, 0) > 0) {
                recovered = HeapCompact(heap, 0);
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        uint64_t elapsedUs = static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());

        m_stats.totalCompactions++;
        m_stats.bytesRecovered += recovered;
        m_stats.lastCompactionTimeUs = elapsedUs;
        m_stats.lastTrigger = CompactionTrigger::Manual;
        if (recovered > m_stats.peakReclaimBytes)
            m_stats.peakReclaimBytes = recovered;

        uint64_t totalTime = m_stats.avgCompactionTimeUs * (m_stats.totalCompactions - 1) + elapsedUs;
        m_stats.avgCompactionTimeUs = totalTime / m_stats.totalCompactions;

        return recovered;
    }

    void ProcessScheduled() {
        std::lock_guard<std::mutex> lock(m_mutex);
        uint64_t now = GetTickCount64();
        for (auto& entry : m_scheduled) {
            if (!entry.executed && now >= entry.scheduledTime) {
                entry.executed = true;
                m_stats.totalCompactions++;
                m_stats.lastTrigger = entry.trigger;
            }
        }
        m_scheduled.erase(
            std::remove_if(m_scheduled.begin(), m_scheduled.end(),
                [](const ScheduledEntry& e) { return e.executed; }),
            m_scheduled.end());
    }

    CompactionStats GetStats() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats;
    }

    size_t GetPendingCount() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_scheduled.size();
    }

    void Reset() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_scheduled.clear();
        m_stats = CompactionStats{};
    }

    bool Validate() const {
        auto stats = GetStats();
        if (stats.successRate < 0.0 || stats.successRate > 1.0) return false;
        if (stats.totalCompactions > 0 && stats.avgCompactionTimeUs == 0 &&
            stats.lastCompactionTimeUs == 0) return false;
        return true;
    }

private:
    MemoryCompactionScheduler() = default;
    ~MemoryCompactionScheduler() = default;
    MemoryCompactionScheduler(const MemoryCompactionScheduler&) = delete;
    MemoryCompactionScheduler& operator=(const MemoryCompactionScheduler&) = delete;

    struct ScheduledEntry {
        CompactionTrigger trigger = CompactionTrigger::Idle;
        uint64_t scheduledTime = 0;
        bool executed = false;
    };

    mutable std::mutex m_mutex;
    std::vector<ScheduledEntry> m_scheduled;
    CompactionStats m_stats;
};

}
} // namespace ExplorerLens::Engine
