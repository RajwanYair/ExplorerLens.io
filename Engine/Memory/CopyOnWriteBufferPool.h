// CopyOnWriteBufferPool.h — COW Buffer Semantics for Decode Pipelines
// Copyright (c) 2026 ExplorerLens Project
//
// Provides copy-on-write buffer semantics for decode output shared
// across multiple pipeline consumers.  Multiple readers share a single
// allocation; a private copy is only made when a writer needs to mutate.
//
#pragma once

#include <atomic>
#include <cstdint>
#include <cstring>
#include <memory>
#include <mutex>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Statistics for COW buffer pool usage
struct COWBufferStats {
    uint64_t buffersCreated = 0;
    uint64_t sharedReads = 0;   // Reads that shared existing data
    uint64_t cowCopies = 0;   // Copies triggered by write
    uint64_t bytesSaved = 0;   // Bytes saved by sharing
    uint64_t totalBytesManaged = 0;
};

/// A single reference-counted buffer with COW semantics
class COWBuffer {
public:
    /// Create a buffer with the given size, zero-filled
    explicit COWBuffer(size_t size)
        : m_data(std::make_shared<std::vector<uint8_t>>(size, static_cast<uint8_t>(0)))
        , m_size(size) {
    }

    /// Create a buffer by copying existing data
    COWBuffer(const uint8_t* src, size_t size)
        : m_data(std::make_shared<std::vector<uint8_t>>(src, src + size))
        , m_size(size) {
    }

    /// Read-only access (no copy — just returns shared pointer)
    const uint8_t* ReadPtr() const {
        return m_data ? m_data->data() : nullptr;
    }

    /// Writable access (triggers COW copy if shared)
    uint8_t* WritePtr() {
        EnsureUnique();
        return m_data ? m_data->data() : nullptr;
    }

    /// Copy another COWBuffer (increments refcount, no data copy)
    COWBuffer Share() const {
        COWBuffer copy;
        copy.m_data = m_data;
        copy.m_size = m_size;
        return copy;
    }

    /// Check if this buffer is shared with another
    bool IsShared() const {
        return m_data && m_data.use_count() > 1;
    }

    size_t Size() const { return m_size; }
    long UseCount() const { return m_data ? m_data.use_count() : 0; }

private:
    COWBuffer() = default;

    void EnsureUnique() {
        if (m_data && m_data.use_count() > 1) {
            // Deep copy — COW triggered
            m_data = std::make_shared<std::vector<uint8_t>>(*m_data);
        }
    }

    std::shared_ptr<std::vector<uint8_t>> m_data;
    size_t m_size = 0;
};

/// Pool that manages COW buffers with size-class bucketing
class CopyOnWriteBufferPool {
public:
    static CopyOnWriteBufferPool& Instance() {
        static CopyOnWriteBufferPool inst;
        return inst;
    }

    /// Acquire a buffer of at least `size` bytes
    COWBuffer Acquire(size_t size) {
        std::lock_guard<std::mutex> lock(m_mutex);
        size_t bucket = SizeToBucket(size);

        // Check free pool for this bucket
        auto& freeList = m_freeLists[bucket];
        if (!freeList.empty()) {
            COWBuffer buf = std::move(freeList.back());
            freeList.pop_back();
            m_stats.buffersCreated++;
            return buf;
        }

        // Allocate new
        m_stats.buffersCreated++;
        m_stats.totalBytesManaged += size;
        return COWBuffer(BucketToSize(bucket));
    }

    /// Return a buffer to the pool for reuse
    void Release(COWBuffer buf) {
        if (buf.IsShared()) return;  // Still referenced elsewhere
        std::lock_guard<std::mutex> lock(m_mutex);
        size_t bucket = SizeToBucket(buf.Size());
        auto& freeList = m_freeLists[bucket];
        if (freeList.size() < MAX_FREE_PER_BUCKET) {
            freeList.push_back(std::move(buf));
        }
    }

    /// Record a shared read (for stats)
    void RecordSharedRead(size_t bytes) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stats.sharedReads++;
        m_stats.bytesSaved += bytes;
    }

    /// Record a COW copy (for stats)
    void RecordCOWCopy() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stats.cowCopies++;
    }

    COWBufferStats GetStats() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats;
    }

    /// Flush all free buffers
    void Flush() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_freeLists.clear();
    }

private:
    CopyOnWriteBufferPool() = default;

    static constexpr size_t MAX_FREE_PER_BUCKET = 16;
    static constexpr size_t BUCKET_COUNT = 16;

    /// Map size to bucket index (power-of-2 rounding)
    static size_t SizeToBucket(size_t size) {
        if (size == 0) return 0;
        size_t bucket = 0;
        size_t s = size - 1;
        while (s >= 4096 && bucket < BUCKET_COUNT - 1) { s >>= 1; ++bucket; }
        return bucket;
    }

    /// Map bucket index back to allocation size
    static size_t BucketToSize(size_t bucket) {
        return 4096ULL << bucket;
    }

    mutable std::mutex m_mutex;
    std::unordered_map<size_t, std::vector<COWBuffer>> m_freeLists;
    COWBufferStats m_stats;
};

} // namespace Engine
} // namespace ExplorerLens
