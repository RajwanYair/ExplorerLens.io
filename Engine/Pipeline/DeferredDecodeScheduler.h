// DeferredDecodeScheduler.h — Deferred Rendering Pipeline for Thumbnails
// Copyright (c) 2026 ExplorerLens Project
//
// Deferred rendering pipeline for thumbnails. Queues decode requests, batches
// by format, processes in priority order for optimal throughput.
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
#include <unordered_map>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

enum class DecodeTaskState : uint8_t {
    Queued,
    Batched,
    Decoding,
    Rendering,
    Completed,
    Failed
};

enum class DeferredFormatGroup : uint8_t {
    RasterImage,
    VectorGraphic,
    Document,
    Archive,
    ThreeDModel,
    Video,
    Audio,
    Unknown
};

struct DeferredDecodeTask {
    uint64_t taskId = 0;
    std::string filePath;
    DeferredFormatGroup formatGroup = DeferredFormatGroup::Unknown;
    uint8_t priority = 5;
    uint32_t targetWidth = 256;
    uint32_t targetHeight = 256;
    DecodeTaskState state = DecodeTaskState::Queued;
    std::chrono::steady_clock::time_point queuedAt;
    double decodeTimeMs = 0.0;
};

struct BatchInfo {
    DeferredFormatGroup group = DeferredFormatGroup::Unknown;
    std::vector<uint64_t> taskIds;
    uint32_t batchSize = 0;
    double estimatedTimeMs = 0.0;
};

struct DeferredSchedulerStats {
    uint64_t totalQueued = 0;
    uint64_t totalCompleted = 0;
    uint64_t totalFailed = 0;
    double avgDecodeTimeMs = 0.0;
    double avgQueueWaitMs = 0.0;
    uint32_t currentBatchSize = 0;
    uint32_t pendingTasks = 0;
};

class DeferredDecodeScheduler {
public:
    static DeferredDecodeScheduler& Instance() {
        static DeferredDecodeScheduler instance;
        return instance;
    }

    inline uint64_t SubmitTask(const std::string& filePath, DeferredFormatGroup group,
        uint8_t priority = 5, uint32_t width = 256, uint32_t height = 256) {
        std::lock_guard<std::mutex> lock(m_mutex);
        DeferredDecodeTask task;
        task.taskId = m_nextTaskId++;
        task.filePath = filePath;
        task.formatGroup = group;
        task.priority = priority;
        task.targetWidth = width;
        task.targetHeight = height;
        task.state = DecodeTaskState::Queued;
        task.queuedAt = std::chrono::steady_clock::now();

        m_taskQueue.push(task);
        m_taskMap[task.taskId] = task;
        m_stats.totalQueued++;
        m_stats.pendingTasks++;
        return task.taskId;
    }

    inline std::vector<BatchInfo> FormBatches(uint32_t maxBatchSize = 16) {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::unordered_map<uint8_t, std::vector<uint64_t>> groupedTasks;

        std::priority_queue<DeferredDecodeTask, std::vector<DeferredDecodeTask>, TaskComparator> tempQueue;
        while (!m_taskQueue.empty()) {
            auto task = m_taskQueue.top();
            m_taskQueue.pop();
            groupedTasks[static_cast<uint8_t>(task.formatGroup)].push_back(task.taskId);
            tempQueue.push(task);
        }
        m_taskQueue = std::move(tempQueue);

        std::vector<BatchInfo> batches;
        for (auto& [groupId, ids] : groupedTasks) {
            for (size_t i = 0; i < ids.size(); i += maxBatchSize) {
                BatchInfo batch;
                batch.group = static_cast<DeferredFormatGroup>(groupId);
                size_t end = (std::min)(i + maxBatchSize, ids.size());
                batch.taskIds.assign(ids.begin() + i, ids.begin() + end);
                batch.batchSize = static_cast<uint32_t>(batch.taskIds.size());
                batch.estimatedTimeMs = EstimateBatchTime(batch.group, batch.batchSize);
                batches.push_back(batch);
            }
        }
        return batches;
    }

    inline DecodeTaskState GetTaskState(uint64_t taskId) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_taskMap.find(taskId);
        return it != m_taskMap.end() ? it->second.state : DecodeTaskState::Failed;
    }

    inline void MarkComplete(uint64_t taskId, double decodeTimeMs = 0.0) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_taskMap.find(taskId);
        if (it != m_taskMap.end()) {
            it->second.state = DecodeTaskState::Completed;
            it->second.decodeTimeMs = decodeTimeMs;
            m_stats.totalCompleted++;
            m_stats.pendingTasks--;
            m_totalDecodeTime += decodeTimeMs;
            m_stats.avgDecodeTimeMs = m_totalDecodeTime / m_stats.totalCompleted;
        }
    }

    inline DeferredSchedulerStats GetStats() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats;
    }

    inline std::string FormatGroupToString(DeferredFormatGroup group) const {
        switch (group) {
        case DeferredFormatGroup::RasterImage:   return "Raster Image";
        case DeferredFormatGroup::VectorGraphic: return "Vector Graphic";
        case DeferredFormatGroup::Document:      return "Document";
        case DeferredFormatGroup::Archive:       return "Archive";
        case DeferredFormatGroup::ThreeDModel:   return "3D Model";
        case DeferredFormatGroup::Video:         return "Video";
        case DeferredFormatGroup::Audio:         return "Audio";
        default:                         return "Unknown";
        }
    }

private:
    DeferredDecodeScheduler() = default;

    struct TaskComparator {
        bool operator()(const DeferredDecodeTask& a, const DeferredDecodeTask& b) const {
            return a.priority < b.priority;
        }
    };

    inline double EstimateBatchTime(DeferredFormatGroup group, uint32_t count) const {
        double perItem = 5.0;
        switch (group) {
        case DeferredFormatGroup::RasterImage:   perItem = 3.0; break;
        case DeferredFormatGroup::Document:      perItem = 15.0; break;
        case DeferredFormatGroup::ThreeDModel:   perItem = 25.0; break;
        case DeferredFormatGroup::Archive:       perItem = 8.0; break;
        default:                         perItem = 5.0; break;
        }
        return perItem * count;
    }

    mutable std::mutex m_mutex;
    std::priority_queue<DeferredDecodeTask, std::vector<DeferredDecodeTask>, TaskComparator> m_taskQueue;
    std::unordered_map<uint64_t, DeferredDecodeTask> m_taskMap;
    DeferredSchedulerStats m_stats;
    uint64_t m_nextTaskId = 1;
    double m_totalDecodeTime = 0.0;
};

}
} // namespace ExplorerLens::Engine
