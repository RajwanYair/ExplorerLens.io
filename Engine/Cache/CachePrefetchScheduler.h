// CachePrefetchScheduler.h — Topology-Aware Pre-Fetch Queue
// Copyright (c) 2026 ExplorerLens Project
//
// Maintains a priority-ordered queue of thumbnail pre-fetch requests.
// Pre-fetch depth and bandwidth cap are sourced from StreamingCacheTierPolicy
// so the scheduler automatically backs off on metered/slow connections.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens { namespace Engine {

struct PrefetchRequest {
    std::wstring path;
    uint32_t     priority   = 0;   // Higher = fetch first
    uint64_t     enqueuedMs = 0;
};

class CachePrefetchScheduler {
public:
    struct Config {
        uint32_t maxQueueDepth   = 64;
        uint32_t maxActiveJobs   = 4;
        uint32_t maxDepth        = 4;    // From StreamingCacheTierPolicy::prefetchDepth
        uint32_t maxBandwidthKbps = 0;   // 0 = unlimited
    };

    explicit CachePrefetchScheduler(const Config& cfg = {}) : m_cfg(cfg) {}

    void     Enqueue(const PrefetchRequest& req);
    bool     Dequeue(uint64_t nowMs, PrefetchRequest& out);  // Returns next item to fetch
    void     Cancel(const std::wstring& path);
    void     Flush();

    void     SetMaxDepth(uint32_t depth)        { m_cfg.maxDepth = depth; }
    void     SetMaxBandwidthKbps(uint32_t kbps) { m_cfg.maxBandwidthKbps = kbps; }

    uint32_t QueueDepth()        const { return static_cast<uint32_t>(m_queue.size()); }
    uint32_t TotalEnqueued()     const { return m_totalEnqueued; }
    uint32_t TotalDequeued()     const { return m_totalDequeued; }
    uint32_t TotalDropped()      const { return m_totalDropped; }

    const Config& GetConfig()    const { return m_cfg; }

private:
    Config                     m_cfg;
    std::vector<PrefetchRequest> m_queue;  // max-heap by priority
    uint32_t m_totalEnqueued = 0;
    uint32_t m_totalDequeued = 0;
    uint32_t m_totalDropped  = 0;
};

}} // namespace ExplorerLens::Engine
