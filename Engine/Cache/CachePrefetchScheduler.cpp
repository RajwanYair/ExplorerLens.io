// CachePrefetchScheduler.cpp — Topology-Aware Pre-Fetch Queue
// Copyright (c) 2026 ExplorerLens Project
//
#include "CachePrefetchScheduler.h"
#include <algorithm>

namespace ExplorerLens { namespace Engine {

void CachePrefetchScheduler::Enqueue(const PrefetchRequest& req)
{
    // Drop oldest low-priority item when at capacity
    if (m_queue.size() >= m_cfg.maxQueueDepth) {
        auto minIt = std::min_element(m_queue.begin(), m_queue.end(),
            [](const PrefetchRequest& a, const PrefetchRequest& b) {
                return a.priority < b.priority;
            });
        if (minIt != m_queue.end() && req.priority >= minIt->priority) {
            m_queue.erase(minIt);
            ++m_totalDropped;
        } else {
            ++m_totalDropped;
            return;
        }
    }
    m_queue.push_back(req);
    // Maintain max-heap order by priority
    std::push_heap(m_queue.begin(), m_queue.end(),
        [](const PrefetchRequest& a, const PrefetchRequest& b) {
            return a.priority < b.priority;
        });
    ++m_totalEnqueued;
}

bool CachePrefetchScheduler::Dequeue(uint64_t /*nowMs*/, PrefetchRequest& out)
{
    if (m_queue.empty()) return false;
    std::pop_heap(m_queue.begin(), m_queue.end(),
        [](const PrefetchRequest& a, const PrefetchRequest& b) {
            return a.priority < b.priority;
        });
    out = m_queue.back();
    m_queue.pop_back();
    ++m_totalDequeued;
    return true;
}

void CachePrefetchScheduler::Cancel(const std::wstring& path)
{
    m_queue.erase(std::remove_if(m_queue.begin(), m_queue.end(),
        [&path](const PrefetchRequest& r) { return r.path == path; }),
        m_queue.end());
    std::make_heap(m_queue.begin(), m_queue.end(),
        [](const PrefetchRequest& a, const PrefetchRequest& b) {
            return a.priority < b.priority;
        });
}

void CachePrefetchScheduler::Flush()
{
    m_queue.clear();
    m_totalEnqueued = 0;
    m_totalDequeued = 0;
    m_totalDropped  = 0;
}

}} // namespace ExplorerLens::Engine
