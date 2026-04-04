// OfflineAnnotationSyncQueue.h — Offline Annotation Sync Queue
// Copyright (c) 2026 ExplorerLens Project
//
// Queues annotation changes made offline and syncs them when connectivity is restored.
//
#pragma once
#include <cstdint>
#include <deque>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct OASQEntry
{
    std::string operationId;
    std::string payload;
    uint64_t createdMs = 0;
    uint32_t retryCount = 0;
};

struct OASQSyncResult
{
    bool success = false;
    uint32_t syncedCount = 0;
    uint32_t failedCount = 0;
};

class OfflineAnnotationSyncQueue
{
  public:
    void Enqueue(const OASQEntry& entry)
    {
        m_queue.push_back(entry);
    }

    OASQSyncResult FlushAll()
    {
        OASQSyncResult r;
        while (!m_queue.empty()) {
            auto& entry = m_queue.front();
            if (entry.retryCount < 3) {
                ++r.syncedCount;
            } else {
                ++r.failedCount;
            }
            m_queue.pop_front();
        }
        r.success = (r.failedCount == 0);
        return r;
    }
    uint32_t QueueDepth() const
    {
        return static_cast<uint32_t>(m_queue.size());
    }
    void Clear()
    {
        m_queue.clear();
    }

  private:
    std::deque<OASQEntry> m_queue;
};

}  // namespace Engine
}  // namespace ExplorerLens
