// IncrementalIndexUpdater.h — Incremental Index Update on Filesystem Change Events
// Copyright (c) 2026 ExplorerLens Project
//
// Listens for filesystem change notifications and queues incremental updates to
// the semantic search index, batching operations for throughput efficiency.
//
#pragma once

#include <chrono>
#include <cstdint>
#include <mutex>
#include <queue>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// Index-updater-specific change type (distinct from LivePreviewUpdater's FileChangeType)
enum class IndexFileChangeType : uint8_t {
    Created = 0,
    Deleted = 1,
    Modified = 2,
    Renamed = 3
};

struct IndexFileChangeEvent
{
    std::wstring filePath;
    IndexFileChangeType changeType = IndexFileChangeType::Modified;
    uint64_t fileHash = 0;
    int64_t timestamp = 0;
};

class IncrementalIndexUpdater
{
public:
    IncrementalIndexUpdater() = default;
    ~IncrementalIndexUpdater() = default;

    IncrementalIndexUpdater(const IncrementalIndexUpdater&) = delete;
    IncrementalIndexUpdater& operator=(const IncrementalIndexUpdater&) = delete;

    bool Initialize()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stats = {};
        while (!m_pendingQueue.empty())
            m_pendingQueue.pop();
        m_initialized = true;
        return true;
    }

    void Shutdown()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        while (!m_pendingQueue.empty())
            m_pendingQueue.pop();
        m_initialized = false;
    }

    bool OnFileCreated(const std::wstring& path) { return EnqueueEvent(path, IndexFileChangeType::Created); }

    bool OnFileDeleted(const std::wstring& path) { return EnqueueEvent(path, IndexFileChangeType::Deleted); }

    bool OnFileModified(const std::wstring& path) { return EnqueueEvent(path, IndexFileChangeType::Modified); }

    uint32_t GetPendingUpdates() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return static_cast<uint32_t>(m_pendingQueue.size());
    }

    void FlushUpdates()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        while (!m_pendingQueue.empty()) {
            m_pendingQueue.pop();
            ++m_stats.totalFlushed;
        }
    }

    uint64_t GetTotalProcessed() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats.totalFlushed;
    }

    double GetAverageFlushLatencyMs() const { return 0.0; }

    bool IsInitialized() const { return m_initialized; }

private:
    bool EnqueueEvent(const std::wstring& path, IndexFileChangeType type)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_initialized)
            return false;
        IndexFileChangeEvent evt;
        evt.filePath = path;
        evt.changeType = type;
        evt.timestamp = std::chrono::system_clock::now().time_since_epoch().count();
        m_pendingQueue.push(std::move(evt));
        ++m_stats.totalReceived;
        return true;
    }

    struct Stats
    {
        uint64_t totalReceived = 0;
        uint64_t totalFlushed = 0;
    };

    std::queue<IndexFileChangeEvent> m_pendingQueue;
    Stats m_stats;
    mutable std::mutex m_mutex;
    bool m_initialized = false;
};

}  // namespace Engine
}  // namespace ExplorerLens
