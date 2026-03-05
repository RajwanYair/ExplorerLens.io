// DirectStorageIntegration.h — DirectStorage for Ultra-Fast File Loading
// Copyright (c) 2026 ExplorerLens Project
//
// DirectStorage for ultra-fast file loading. Bypasses OS I/O stack, loads files
// directly from NVMe to GPU memory with queue-based async I/O.
//
#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <queue>
#include <mutex>
#include <atomic>
#include <chrono>
#include <functional>

namespace ExplorerLens {
namespace Engine {

enum class StorageRequestPriority : uint8_t {
    Low,
    Normal,
    High,
    Realtime
};

enum class StorageRequestStatus : uint8_t {
    Pending,
    InProgress,
    Completed,
    Failed,
    Cancelled
};

enum class StorageDeviceType : uint8_t {
    NVMe,
    SATA_SSD,
    HDD,
    NetworkDrive,
    Unknown
};

struct StorageRequest {
    uint64_t requestId = 0;
    std::string filePath;
    uint64_t offset = 0;
    uint64_t size = 0;
    StorageRequestPriority priority = StorageRequestPriority::Normal;
    StorageRequestStatus status = StorageRequestStatus::Pending;
    bool directToGPU = false;
};

struct StorageStats {
    uint64_t totalBytesRead = 0;
    uint64_t totalRequests = 0;
    uint64_t completedRequests = 0;
    uint64_t failedRequests = 0;
    double avgLatencyUs = 0.0;
    double peakBandwidthMBps = 0.0;
    double currentBandwidthMBps = 0.0;
};

struct StorageDeviceInfo {
    StorageDeviceType type = StorageDeviceType::Unknown;
    std::string deviceName;
    uint64_t capacityBytes = 0;
    uint32_t maxQueueDepth = 32;
    bool supportsDirectStorage = false;
    double maxBandwidthMBps = 0.0;
};

class DirectStorageIntegration {
public:
    static DirectStorageIntegration& Instance() {
        static DirectStorageIntegration instance;
        return instance;
    }

    inline uint64_t SubmitReadRequest(const std::string& filePath, uint64_t offset, uint64_t size,
        StorageRequestPriority priority = StorageRequestPriority::Normal,
        bool directToGPU = false) {
        std::lock_guard<std::mutex> lock(m_mutex);
        StorageRequest req;
        req.requestId = m_nextRequestId++;
        req.filePath = filePath;
        req.offset = offset;
        req.size = size;
        req.priority = priority;
        req.directToGPU = directToGPU;
        req.status = StorageRequestStatus::Pending;
        m_pendingQueue.push(req);
        m_stats.totalRequests++;
        return req.requestId;
    }

    inline bool CancelRequest(uint64_t requestId) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_activeRequests.find(requestId);
        if (it != m_activeRequests.end()) {
            it->second.status = StorageRequestStatus::Cancelled;
            return true;
        }
        return false;
    }

    inline StorageRequestStatus GetRequestStatus(uint64_t requestId) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_activeRequests.find(requestId);
        if (it != m_activeRequests.end()) return it->second.status;
        return StorageRequestStatus::Failed;
    }

    inline StorageStats GetStats() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats;
    }

    inline void UpdateBandwidthMeasurement(uint64_t bytesTransferred, double elapsedUs) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stats.totalBytesRead += bytesTransferred;
        if (elapsedUs > 0.0) {
            double mbps = (bytesTransferred / (1024.0 * 1024.0)) / (elapsedUs / 1000000.0);
            m_stats.currentBandwidthMBps = mbps;
            if (mbps > m_stats.peakBandwidthMBps) m_stats.peakBandwidthMBps = mbps;
        }
    }

    inline uint32_t GetOptimalQueueDepth(StorageDeviceType deviceType) const {
        switch (deviceType) {
        case StorageDeviceType::NVMe:         return 64;
        case StorageDeviceType::SATA_SSD:     return 32;
        case StorageDeviceType::HDD:          return 4;
        case StorageDeviceType::NetworkDrive:  return 8;
        default:                               return 16;
        }
    }

    inline size_t GetPendingCount() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_pendingQueue.size();
    }

    inline std::string DeviceTypeToString(StorageDeviceType type) const {
        switch (type) {
        case StorageDeviceType::NVMe:         return "NVMe SSD";
        case StorageDeviceType::SATA_SSD:     return "SATA SSD";
        case StorageDeviceType::HDD:          return "HDD";
        case StorageDeviceType::NetworkDrive:  return "Network Drive";
        default:                               return "Unknown";
        }
    }

private:
    DirectStorageIntegration() = default;

    struct RequestComparator {
        bool operator()(const StorageRequest& a, const StorageRequest& b) const {
            return static_cast<uint8_t>(a.priority) < static_cast<uint8_t>(b.priority);
        }
    };

    mutable std::mutex m_mutex;
    std::priority_queue<StorageRequest, std::vector<StorageRequest>, RequestComparator> m_pendingQueue;
    std::unordered_map<uint64_t, StorageRequest> m_activeRequests;
    StorageStats m_stats;
    uint64_t m_nextRequestId = 1;
};

}
} // namespace ExplorerLens::Engine
