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

static constexpr uint32_t INDEX_UPDATE_BATCH_SIZE    = 64;
static constexpr uint32_t INDEX_UPDATE_FLUSH_MS      = 500;
static constexpr uint32_t INDEX_UPDATE_MAX_PENDING   = 10000;

enum class FileChangeType : uint8_t {
    Created  = 0,
    Deleted  = 1,
    Modified = 2,
    Renamed  = 3
};

struct FileChangeEvent {
    std::wstring   filePath;
    FileChangeType changeType;
    uint64_t       fileHash;
    int64_t        timestamp;
};

struct UpdateBatch {
    std::vector<FileChangeEvent> events;
    int64_t                      createdAt;
    uint32_t                     retryCount = 0;
};

struct UpdaterStats {
    uint64_t totalEventsReceived  = 0;
    uint64_t totalEventsFlushed   = 0;
    uint64_t totalBatchesFlushed  = 0;
    uint64_t pendingEvents        = 0;
    float    avgFlushLatencyMs    = 0.0f;
};

struct IndexUpdaterConfig {
    uint32_t batchSize       = INDEX_UPDATE_BATCH_SIZE;
    uint32_t flushIntervalMs = INDEX_UPDATE_FLUSH_MS;
    uint32_t maxPending      = INDEX_UPDATE_MAX_PENDING;
    uint32_t maxRetries      = 3;
    bool     autoFlush       = true;
};

class IncrementalIndexUpdater {
public:
    inline bool Initialize(const IndexUpdaterConfig& config) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_config = config;
        m_stats = {};
        while (!m_pendingQueue.empty()) m_pendingQueue.pop();
        m_initialized = true;
        return true;
    }

    inline bool OnFileCreated(const std::wstring& path, uint64_t fileHash) {
        return EnqueueEvent(path, FileChangeType::Created, fileHash);
    }

    inline bool OnFileDeleted(const std::wstring& path, uint64_t fileHash) {
        return EnqueueEvent(path, FileChangeType::Deleted, fileHash);
    }

    inline bool OnFileModified(const std::wstring& path, uint64_t fileHash) {
        return EnqueueEvent(path, FileChangeType::Modified, fileHash);
    }

    inline uint32_t GetPendingUpdates() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return static_cast<uint32_t>(m_pendingQueue.size());
    }

    inline UpdateBatch FlushUpdates() {
        std::lock_guard<std::mutex> lock(m_mutex);
        UpdateBatch batch;
        batch.createdAt = std::chrono::system_clock::now().time_since_epoch().count();

        uint32_t count = 0;
        while (!m_pendingQueue.empty() && count < m_config.batchSize) {
            batch.events.push_back(std::move(m_pendingQueue.front()));
            m_pendingQueue.pop();
            ++count;
        }

        m_stats.totalEventsFlushed += count;
        ++m_stats.totalBatchesFlushed;
        m_stats.pendingEvents = m_pendingQueue.size();
        return batch;
    }

    inline UpdaterStats GetStats() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats;
    }

    inline bool IsInitialized() const { return m_initialized; }

private:
    inline bool EnqueueEvent(const std::wstring& path, FileChangeType type, uint64_t hash) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_initialized) return false;
        if (m_pendingQueue.size() >= m_config.maxPending) return false;

        FileChangeEvent evt;
        evt.filePath   = path;
        evt.changeType = type;
        evt.fileHash   = hash;
        evt.timestamp  = std::chrono::system_clock::now().time_since_epoch().count();

        m_pendingQueue.push(std::move(evt));
        ++m_stats.totalEventsReceived;
        m_stats.pendingEvents = m_pendingQueue.size();

        if (m_config.autoFlush && m_pendingQueue.size() >= m_config.batchSize) {
            // Caller should poll FlushUpdates() — auto-flush is a hint
        }
        return true;
    }

    IndexUpdaterConfig             m_config;
    std::queue<FileChangeEvent>    m_pendingQueue;
    UpdaterStats                   m_stats;
    mutable std::mutex             m_mutex;
    bool                           m_initialized = false;
};

} // namespace Engine
} // namespace ExplorerLens
