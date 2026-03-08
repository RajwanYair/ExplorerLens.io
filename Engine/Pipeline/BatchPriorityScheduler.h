// BatchPriorityScheduler.h — Visible-first thumbnail scheduling
// Copyright (c) 2026 ExplorerLens Project
//
// Prioritizes decode of thumbnails currently visible in the Explorer viewport
// over off-screen items, ensuring fastest perceived performance.
//
#pragma once
#include <string>
#include <cstdint>
#include <vector>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

struct BatchPrioritySchedulerConfig {
    bool enabled = true;
    uint32_t maxQueueSize = 512;
    uint32_t visibleBoost = 100;
    std::string label = "BatchPriorityScheduler";
};

class BatchPriorityScheduler {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    BatchPrioritySchedulerConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    struct Request {
        uint32_t id = 0;
        std::string filePath;
        uint32_t priority = 0;
        bool isVisible = false;
    };

    bool Enqueue(Request req) {
        if (m_queue.size() >= m_config.maxQueueSize) return false;
        if (req.isVisible) req.priority += m_config.visibleBoost;
        m_queue.push_back(req);
        return true;
    }

    Request DequeueHighest() {
        if (m_queue.empty()) return {};
        auto it = std::max_element(m_queue.begin(), m_queue.end(),
            [](const Request& a, const Request& b) { return a.priority < b.priority; });
        Request r = *it;
        m_queue.erase(it);
        return r;
    }

    uint32_t GetQueueSize() const { return static_cast<uint32_t>(m_queue.size()); }
    bool IsEmpty() const { return m_queue.empty(); }

private:
    bool m_initialized = false;
    BatchPrioritySchedulerConfig m_config;
    std::vector<Request> m_queue;
};

}
} // namespace ExplorerLens::Engine
