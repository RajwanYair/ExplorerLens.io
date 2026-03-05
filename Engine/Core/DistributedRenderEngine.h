// DistributedRenderEngine.h — Multi-Machine Distributed Thumbnail Generation
// Copyright (c) 2026 ExplorerLens Project
//
// Multi-machine distributed thumbnail generation. Manages work distribution
// across networked render nodes via named pipes.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <functional>
#include <chrono>
#include <atomic>
#include <unordered_map>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

enum class RenderNodeStatus : uint8_t {
    Idle,
    Busy,
    Offline,
    Error,
    Draining
};

struct RenderNode {
    std::string nodeId;
    std::string address;
    uint16_t port = 0;
    RenderNodeStatus status = RenderNodeStatus::Offline;
    uint32_t activeJobs = 0;
    uint32_t maxConcurrency = 4;
    double avgLatencyMs = 0.0;
    uint64_t completedJobs = 0;
};

struct RenderJob {
    uint64_t jobId = 0;
    std::string filePath;
    uint32_t targetWidth = 256;
    uint32_t targetHeight = 256;
    uint8_t priority = 5;
    std::chrono::steady_clock::time_point submittedAt;
    std::string assignedNode;
};

struct RenderResult {
    uint64_t jobId = 0;
    std::vector<uint8_t> thumbnailData;
    uint32_t width = 0;
    uint32_t height = 0;
    double renderTimeMs = 0.0;
    bool success = false;
    std::string errorMessage;
};

class DistributedRenderEngine {
public:
    static DistributedRenderEngine& Instance() {
        static DistributedRenderEngine instance;
        return instance;
    }

    inline bool RegisterNode(const std::string& nodeId, const std::string& address,
        uint16_t port, uint32_t maxConcurrency = 4) {
        std::lock_guard<std::mutex> lock(m_mutex);
        RenderNode node;
        node.nodeId = nodeId;
        node.address = address;
        node.port = port;
        node.maxConcurrency = maxConcurrency;
        node.status = RenderNodeStatus::Idle;
        m_nodes[nodeId] = node;
        return true;
    }

    inline bool UnregisterNode(const std::string& nodeId) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_nodes.find(nodeId);
        if (it == m_nodes.end()) return false;
        it->second.status = RenderNodeStatus::Draining;
        if (it->second.activeJobs == 0) {
            m_nodes.erase(it);
        }
        return true;
    }

    inline uint64_t SubmitJob(const std::string& filePath, uint32_t width, uint32_t height, uint8_t priority = 5) {
        std::lock_guard<std::mutex> lock(m_mutex);
        RenderJob job;
        job.jobId = m_nextJobId++;
        job.filePath = filePath;
        job.targetWidth = width;
        job.targetHeight = height;
        job.priority = priority;
        job.submittedAt = std::chrono::steady_clock::now();
        m_pendingJobs.push(job);
        return job.jobId;
    }

    inline std::string SelectBestNode() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::string bestNode;
        double bestScore = -1.0;

        for (const auto& [id, node] : m_nodes) {
            if (node.status != RenderNodeStatus::Idle && node.status != RenderNodeStatus::Busy) continue;
            if (node.activeJobs >= node.maxConcurrency) continue;

            double capacityRatio = 1.0 - static_cast<double>(node.activeJobs) / node.maxConcurrency;
            double latencyFactor = 1.0 / (1.0 + node.avgLatencyMs / 100.0);
            double score = capacityRatio * 0.7 + latencyFactor * 0.3;

            if (score > bestScore) {
                bestScore = score;
                bestNode = id;
            }
        }
        return bestNode;
    }

    inline size_t GetPendingJobCount() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_pendingJobs.size();
    }

    inline std::vector<RenderNode> GetNodeSnapshot() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<RenderNode> result;
        result.reserve(m_nodes.size());
        for (const auto& [id, node] : m_nodes) {
            result.push_back(node);
        }
        return result;
    }

    inline uint64_t GetTotalCompletedJobs() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        uint64_t total = 0;
        for (const auto& [id, node] : m_nodes) {
            total += node.completedJobs;
        }
        return total;
    }

private:
    DistributedRenderEngine() = default;

    struct JobComparator {
        bool operator()(const RenderJob& a, const RenderJob& b) const {
            return a.priority < b.priority;
        }
    };

    mutable std::mutex m_mutex;
    std::unordered_map<std::string, RenderNode> m_nodes;
    std::priority_queue<RenderJob, std::vector<RenderJob>, JobComparator> m_pendingJobs;
    uint64_t m_nextJobId = 1;
};

}
} // namespace ExplorerLens::Engine
