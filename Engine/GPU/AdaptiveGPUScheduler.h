// ============================================================================
// AdaptiveGPUScheduler.h — GPU/CPU Adaptive Workload Routing
// ExplorerLens Engine v15.0.0
// Copyright (c) 2026 ExplorerLens Project
//
// Dynamically routes thumbnail decode/resize work between GPU and CPU based
// on real-time load, power state, thermal throttling, and format suitability.
// Maximizes throughput while avoiding GPU starvation or CPU bottlenecks.
// ============================================================================

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <deque>
#include <mutex>
#include <atomic>
#include <chrono>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

// ============================================================================
// Processing backend selection
// ============================================================================

enum class ProcessingBackend : uint8_t {
    CPU = 0,  // Multi-threaded CPU decode
    GPU_D3D11 = 1,  // D3D11 compute shader
    GPU_D3D12 = 2,  // D3D12 compute (preferred)
    GPU_Vulkan = 3,  // Vulkan compute
    GPU_Vendor = 4,  // Vendor-specific (NVDEC/QuickSync/AMF)
    Auto = 5   // Scheduler decides
};

inline const char* ProcessingBackendToString(ProcessingBackend backend) {
    static const char* names[] = {
        "CPU", "D3D11", "D3D12", "Vulkan", "Vendor", "Auto"
    };
    return names[static_cast<uint8_t>(backend)];
}

// ============================================================================
// System load snapshot
// ============================================================================

struct SystemLoadSnapshot {
    float cpuUtilization = 0.0f;  // 0-100%
    float gpuUtilization = 0.0f;  // 0-100%
    float gpuMemoryUsedMB = 0.0f;
    float gpuMemoryTotalMB = 0.0f;
    float gpuTemperatureC = 0.0f;
    float cpuTemperatureC = 0.0f;
    bool  gpuThrottled = false;
    bool  onBatteryPower = false;
    uint32_t pendingGPUWork = 0;
    uint32_t pendingCPUWork = 0;
    uint64_t timestamp = 0;

    float GetGPUMemoryPercent() const {
        return (gpuMemoryTotalMB > 0)
            ? (gpuMemoryUsedMB / gpuMemoryTotalMB * 100.0f)
            : 0.0f;
    }

    bool IsGPUOverloaded() const {
        return gpuUtilization > 90.0f || gpuThrottled ||
            GetGPUMemoryPercent() > 85.0f;
    }

    bool IsCPUOverloaded() const {
        return cpuUtilization > 85.0f;
    }
};

// ============================================================================
// Work item for scheduling
// ============================================================================

struct ScheduledWorkItem {
    uint64_t            id = 0;
    std::wstring        filePath;
    std::wstring        formatType;
    uint64_t            fileSize = 0;
    uint32_t            targetWidth = 256;
    uint32_t            targetHeight = 256;
    ProcessingBackend   preferredBackend = ProcessingBackend::Auto;
    ProcessingBackend   assignedBackend = ProcessingBackend::Auto;
    bool                assigned = false;
    bool                completed = false;
    double              estimatedTimeMs = 0.0;
    double              actualTimeMs = 0.0;
    uint32_t            priority = 0;  // Higher = more urgent
};

// ============================================================================
// Scheduling statistics
// ============================================================================

struct SchedulerStats {
    uint64_t totalScheduled = 0;
    uint64_t cpuAssigned = 0;
    uint64_t gpuAssigned = 0;
    uint64_t routingDecisions = 0;
    uint64_t rebalanceEvents = 0;  // GPU↔CPU migrations
    double   avgCPUTimeMs = 0.0;
    double   avgGPUTimeMs = 0.0;
    double   throughputPerSec = 0.0;

    double GetGPURatio() const {
        return (totalScheduled > 0)
            ? (static_cast<double>(gpuAssigned) / totalScheduled * 100.0)
            : 0.0;
    }
};

// ============================================================================
// Format GPU suitability score
// ============================================================================

struct FormatGPUAffinity {
    std::wstring formatType;
    float        gpuAffinity = 0.5f;  // 0.0 = CPU-only, 1.0 = GPU-strongly-preferred
    float        minGPUSpeedup = 1.0f; // GPU must be this many times faster

    /// Pre-scored format affinities
    static float GetDefaultAffinity(const std::wstring& format) {
        // Formats that benefit most from GPU
        if (format == L"RAW" || format == L"EXR" || format == L"HDR") return 0.95f;
        if (format == L"HEIF" || format == L"AVIF") return 0.9f;
        if (format == L"JPEG" || format == L"WebP") return 0.8f;
        if (format == L"TIFF" || format == L"PSD") return 0.75f;
        if (format == L"Video") return 0.95f;  // Hardware decode
        // Formats better suited to CPU
        if (format == L"SVG" || format == L"Font") return 0.2f;
        if (format == L"PDF" || format == L"Document") return 0.3f;
        if (format == L"Archive") return 0.1f;  // I/O bound
        return 0.5f;  // Default balanced
    }
};

// ============================================================================
// AdaptiveGPUScheduler
// ============================================================================

class AdaptiveGPUScheduler {
public:
    static constexpr uint32_t MAX_QUEUE_SIZE = 256;
    static constexpr float GPU_OVERLOAD_THRESHOLD = 90.0f;
    static constexpr float CPU_OVERLOAD_THRESHOLD = 85.0f;
    static constexpr float REBALANCE_HYSTERESIS = 10.0f; // % change needed
    static constexpr uint32_t LOAD_SAMPLE_INTERVAL_MS = 100;

    AdaptiveGPUScheduler() = default;
    ~AdaptiveGPUScheduler() = default;

    // Non-copyable
    AdaptiveGPUScheduler(const AdaptiveGPUScheduler&) = delete;
    AdaptiveGPUScheduler& operator=(const AdaptiveGPUScheduler&) = delete;

    // ========================================================================
    // Initialization
    // ========================================================================

    bool Initialize(bool gpuAvailable = true) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_gpuAvailable = gpuAvailable;
        m_initialized = true;
        return true;
    }

    bool IsInitialized() const { return m_initialized; }
    bool IsGPUAvailable() const { return m_gpuAvailable; }

    // ========================================================================
    // Load monitoring
    // ========================================================================

    void UpdateSystemLoad(const SystemLoadSnapshot& load) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_currentLoad = load;
        m_loadHistory.push_back(load);
        if (m_loadHistory.size() > 100) m_loadHistory.pop_front();
    }

    SystemLoadSnapshot GetCurrentLoad() const { return m_currentLoad; }

    // ========================================================================
    // Work scheduling
    // ========================================================================

    /// Route a work item to the optimal backend
    ProcessingBackend RouteWork(ScheduledWorkItem& item) {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_stats.totalScheduled++;
        m_stats.routingDecisions++;

        // Explicit backend preference
        if (item.preferredBackend != ProcessingBackend::Auto) {
            item.assignedBackend = item.preferredBackend;
            UpdateBackendStats(item.assignedBackend);
            return item.assignedBackend;
        }

        // Get format GPU affinity
        float affinity = FormatGPUAffinity::GetDefaultAffinity(item.formatType);

        // Decision factors
        bool gpuOverloaded = m_currentLoad.IsGPUOverloaded();
        bool cpuOverloaded = m_currentLoad.IsCPUOverloaded();
        bool batteryMode = m_currentLoad.onBatteryPower;

        ProcessingBackend chosen;

        if (!m_gpuAvailable || (batteryMode && affinity < 0.8f)) {
            chosen = ProcessingBackend::CPU;
        }
        else if (gpuOverloaded && !cpuOverloaded) {
            chosen = ProcessingBackend::CPU;
        }
        else if (!gpuOverloaded && cpuOverloaded) {
            chosen = ProcessingBackend::GPU_D3D12;
        }
        else if (affinity >= 0.7f && !gpuOverloaded) {
            chosen = ProcessingBackend::GPU_D3D12;
        }
        else if (affinity <= 0.3f) {
            chosen = ProcessingBackend::CPU;
        }
        else {
            // Balanced — prefer GPU if queue is shorter
            chosen = (m_currentLoad.pendingGPUWork < m_currentLoad.pendingCPUWork)
                ? ProcessingBackend::GPU_D3D12
                : ProcessingBackend::CPU;
        }

        item.assignedBackend = chosen;
        item.assigned = true;
        UpdateBackendStats(chosen);
        return chosen;
    }

    /// Report completion of a work item (for stats)
    void ReportCompletion(const ScheduledWorkItem& item) {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (item.assignedBackend == ProcessingBackend::CPU) {
            m_stats.avgCPUTimeMs =
                (m_stats.avgCPUTimeMs * (m_stats.cpuAssigned - 1) + item.actualTimeMs)
                / m_stats.cpuAssigned;
        }
        else {
            m_stats.avgGPUTimeMs =
                (m_stats.avgGPUTimeMs * (m_stats.gpuAssigned - 1) + item.actualTimeMs)
                / m_stats.gpuAssigned;
        }
    }

    // ========================================================================
    // Statistics
    // ========================================================================

    SchedulerStats GetStats() const { return m_stats; }

    /// Get format GPU affinity for a given type
    float GetFormatAffinity(const std::wstring& formatType) const {
        return FormatGPUAffinity::GetDefaultAffinity(formatType);
    }

private:
    void UpdateBackendStats(ProcessingBackend backend) {
        if (backend == ProcessingBackend::CPU) {
            m_stats.cpuAssigned++;
        }
        else {
            m_stats.gpuAssigned++;
        }
    }

    // State
    SystemLoadSnapshot m_currentLoad;
    std::deque<SystemLoadSnapshot> m_loadHistory;
    mutable std::mutex m_mutex;
    SchedulerStats m_stats{};
    bool m_gpuAvailable = false;
    bool m_initialized = false;
};

} // namespace Engine
} // namespace ExplorerLens
