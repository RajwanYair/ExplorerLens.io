// RealTimePreviewPipeline.cpp — Real-Time Preview Update Pipeline
// Copyright (c) 2026 ExplorerLens Project
//
#include "RealTimePreviewPipeline.h"
#include <chrono>
#include <algorithm>

namespace ExplorerLens { namespace Engine {

uint64_t RealTimePreviewPipeline::NowMs()
{
    using namespace std::chrono;
    return static_cast<uint64_t>(
        duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
}

RealTimePreviewPipeline::RealTimePreviewPipeline(const Config& cfg)
    : m_config(cfg)
{}

void RealTimePreviewPipeline::Subscribe(PreviewSubscriber sub)
{
    m_subscriber = std::move(sub);
}

void RealTimePreviewPipeline::Notify(const std::wstring& filePath)
{
    ++m_totalEnqueued;

    // Check if this path is already pending → coalesce.
    auto it = m_queueIndex.find(filePath);
    if (it != m_queueIndex.end()) {
        uint32_t idx = it->second;
        if (idx < m_queue.size()) {
            m_queue[idx].lastSeenMs = NowMs();
            m_queue[idx].coalesceCount++;
            return;
        }
    }

    // Back-pressure: drop oldest if queue is at capacity.
    if (m_queue.size() >= m_config.maxQueueDepth) {
        const auto& dropped = m_queue.front();
        m_queueIndex.erase(dropped.filePath);
        m_queue.pop_front();
        ++m_totalDropped;

        // Rebuild index (simple O(n) rebuild; queue is small).
        m_queueIndex.clear();
        for (uint32_t i = 0; i < static_cast<uint32_t>(m_queue.size()); ++i)
            m_queueIndex[m_queue[i].filePath] = i;
    }

    PendingEvent ev;
    ev.filePath     = filePath;
    ev.firstSeenMs  = NowMs();
    ev.lastSeenMs   = ev.firstSeenMs;
    ev.coalesceCount = 1;
    m_queueIndex[filePath] = static_cast<uint32_t>(m_queue.size());
    m_queue.push_back(std::move(ev));
}

uint32_t RealTimePreviewPipeline::Drain(uint64_t currentMs)
{
    uint32_t fired = 0;

    auto it = m_queue.begin();
    while (it != m_queue.end()) {
        const bool debounceElapsed = (currentMs - it->lastSeenMs) >= m_config.debounceMs;
        const bool maxTimeElapsed  = (currentMs - it->firstSeenMs) >= m_config.maxCoalesceMs;

        if (debounceElapsed || maxTimeElapsed) {
            if (m_subscriber) {
                PreviewUpdateEvent ev;
                ev.filePath       = it->filePath;
                ev.enqueuedAtMs   = it->firstSeenMs;
                ev.coalesceCount  = it->coalesceCount;
                m_subscriber(ev);
            }
            m_queueIndex.erase(it->filePath);
            it = m_queue.erase(it);
            ++m_totalFired;
            ++fired;
        } else {
            ++it;
        }
    }

    // Rebuild index only if we fired anything.
    if (fired > 0) {
        m_queueIndex.clear();
        for (uint32_t i = 0; i < static_cast<uint32_t>(m_queue.size()); ++i)
            m_queueIndex[m_queue[i].filePath] = i;
    }

    return fired;
}

uint32_t RealTimePreviewPipeline::PendingEventCount() const
{
    return static_cast<uint32_t>(m_queue.size());
}

uint64_t RealTimePreviewPipeline::TotalEventsEnqueued()  const { return m_totalEnqueued; }
uint64_t RealTimePreviewPipeline::TotalEventsFired()     const { return m_totalFired;    }
uint64_t RealTimePreviewPipeline::TotalEventsDropped()   const { return m_totalDropped;  }
const RealTimePreviewPipeline::Config& RealTimePreviewPipeline::GetConfig() const { return m_config; }

}} // namespace ExplorerLens::Engine
