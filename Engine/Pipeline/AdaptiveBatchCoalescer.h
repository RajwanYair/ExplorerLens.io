// AdaptiveBatchCoalescer.h — Dynamic Batch Size Optimization
// Copyright (c) 2026 ExplorerLens Project
//
// Dynamic batch size optimization. Monitors system load, adjusts batch sizes
// for optimal throughput based on real-time performance metrics.
//
#pragma once

#include <cstdint>
#include <vector>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <mutex>
#include <numeric>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class SystemLoadLevel : uint8_t {
    Idle,
    Light,
    Moderate,
    Heavy,
    Saturated
};

struct BatchMetrics {
    uint32_t batchSize = 0;
    double processingTimeMs = 0.0;
    double throughputItemsPerSec = 0.0;
    double cpuUtilization = 0.0;
    double memoryUsageMB = 0.0;
};

struct CoalescerConfig {
    uint32_t minBatchSize = 1;
    uint32_t maxBatchSize = 128;
    uint32_t initialBatchSize = 16;
    double targetCpuUtilization = 0.75;
    double targetLatencyMs = 100.0;
    double learningRate = 0.1;
    uint32_t historyWindow = 20;
};

struct CoalescerStats {
    uint32_t currentBatchSize = 16;
    double currentThroughput = 0.0;
    double peakThroughput = 0.0;
    uint64_t totalBatchesProcessed = 0;
    uint64_t totalItemsProcessed = 0;
    double avgProcessingTimeMs = 0.0;
    SystemLoadLevel currentLoad = SystemLoadLevel::Idle;
};

class AdaptiveBatchCoalescer {
public:
    static AdaptiveBatchCoalescer& Instance() {
        static AdaptiveBatchCoalescer instance;
        return instance;
    }

    inline void Configure(const CoalescerConfig& config) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_config = config;
        m_currentBatchSize = config.initialBatchSize;
    }

    inline void RecordBatchResult(const BatchMetrics& metrics) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_history.push_back(metrics);
        if (m_history.size() > m_config.historyWindow) {
            m_history.erase(m_history.begin());
        }

        m_stats.totalBatchesProcessed++;
        m_stats.totalItemsProcessed += metrics.batchSize;
        m_stats.currentThroughput = metrics.throughputItemsPerSec;
        if (metrics.throughputItemsPerSec > m_stats.peakThroughput) {
            m_stats.peakThroughput = metrics.throughputItemsPerSec;
        }

        double totalTime = 0.0;
        for (const auto& m : m_history) totalTime += m.processingTimeMs;
        m_stats.avgProcessingTimeMs = totalTime / m_history.size();

        AdaptBatchSize(metrics);
    }

    inline uint32_t GetOptimalBatchSize() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_currentBatchSize;
    }

    inline SystemLoadLevel ClassifyLoad(double cpuUtil, double memUsageMB) const {
        (void)memUsageMB;
        if (cpuUtil < 0.2) return SystemLoadLevel::Idle;
        if (cpuUtil < 0.4) return SystemLoadLevel::Light;
        if (cpuUtil < 0.65) return SystemLoadLevel::Moderate;
        if (cpuUtil < 0.85) return SystemLoadLevel::Heavy;
        return SystemLoadLevel::Saturated;
    }

    inline CoalescerStats GetStats() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats;
    }

    inline std::string LoadLevelToString(SystemLoadLevel level) const {
        switch (level) {
        case SystemLoadLevel::Idle:      return "Idle";
        case SystemLoadLevel::Light:     return "Light";
        case SystemLoadLevel::Moderate:  return "Moderate";
        case SystemLoadLevel::Heavy:     return "Heavy";
        case SystemLoadLevel::Saturated: return "Saturated";
        default:                         return "Unknown";
        }
    }

private:
    AdaptiveBatchCoalescer() = default;

    inline void AdaptBatchSize(const BatchMetrics& latest) {
        double cpuGap = m_config.targetCpuUtilization - latest.cpuUtilization;
        double latencyFactor = latest.processingTimeMs / m_config.targetLatencyMs;

        int32_t adjustment = 0;
        if (cpuGap > 0.15 && latencyFactor < 0.8) {
            adjustment = static_cast<int32_t>(m_currentBatchSize * m_config.learningRate);
            adjustment = (std::max)(1, adjustment);
        }
        else if (cpuGap < -0.1 || latencyFactor > 1.2) {
            adjustment = -static_cast<int32_t>(m_currentBatchSize * m_config.learningRate);
            adjustment = (std::min)(-1, adjustment);
        }

        int32_t newSize = static_cast<int32_t>(m_currentBatchSize) + adjustment;
        m_currentBatchSize = static_cast<uint32_t>(
            (std::max)(static_cast<int32_t>(m_config.minBatchSize),
                (std::min)(static_cast<int32_t>(m_config.maxBatchSize), newSize)));

        m_stats.currentBatchSize = m_currentBatchSize;
        m_stats.currentLoad = ClassifyLoad(latest.cpuUtilization, latest.memoryUsageMB);
    }

    mutable std::mutex m_mutex;
    CoalescerConfig m_config;
    uint32_t m_currentBatchSize = 16;
    std::vector<BatchMetrics> m_history;
    CoalescerStats m_stats;
};

}
} // namespace ExplorerLens::Engine
