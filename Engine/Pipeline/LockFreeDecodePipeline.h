#pragma once
// ============================================================================
// LockFreeDecodePipeline.h — Lock-Free MPMC Decode Queue
// Copyright (c) 2026 ExplorerLens Project
//
// Suppress C4324 — intentional cache-line alignment padding
#pragma warning(push)
#pragma warning(disable: 4324)
//
// PURPOSE:
//   Lock-free multi-producer multi-consumer decode queue using Compare-And-Swap
//   (CAS) operations on a power-of-2 sized ring buffer. Provides zero-mutex
//   concurrent enqueue/dequeue for thumbnail decode tasks with configurable
//   worker threads that drain the queue and invoke a user-supplied decode
//   function.
//
// CLASSES:
//   - DecodeTask: Describes a single file decode request with path, target
//     dimensions, priority, and a completion callback.
//   - LockFreeDecodePipeline: CAS-based bounded MPMC queue (Vyukov algorithm)
//     with background worker threads for decode execution.
//
// INPUTS:
//   - DecodeTask objects pushed via TryEnqueue()
//   - A decode function set via SetDecodeFunction()
//
// OUTPUTS:
//   - Decoded results delivered via per-task callback
//   - QueueDepth() for monitoring backlog
//   - GetItemsProcessed() / GetItemsDropped() for stats
//
// THREAD SAFETY:
//   TryEnqueue/TryDequeue are fully lock-free using per-slot atomic sequence
//   counters with memory_order_acquire/release. Head and tail indices reside
//   on separate cache lines to eliminate false sharing.
// ============================================================================

#include <atomic>
#include <vector>
#include <thread>
#include <functional>
#include <string>
#include <cstdint>
#include <memory>

namespace ExplorerLens {
namespace Engine {

/// Pipeline stage state (atomic transitions)
enum class PipelineStageState : uint8_t {
    Idle = 0,
    Queued,
    Decoding,
    Scaling,
    Caching,
    Complete,
    Failed,
    COUNT
};

/// Queue overflow policy
enum class OverflowPolicy : uint8_t {
    Block = 0,
    DropOldest,
    DropNewest,
    ExpandBuffer,
    COUNT
};

struct LockFreePipelineConfig {
    uint32_t queueCapacity = 256;
    uint32_t workerThreads = 4;
    OverflowPolicy overflowPolicy = OverflowPolicy::DropOldest;
    bool     useAffinity = false;
    uint32_t batchSize = 8;
};

struct LockFreePipelineStats {
    uint64_t itemsProcessed = 0;
    uint64_t itemsDropped = 0;
    uint64_t contentionEvents = 0;
    double   avgLatencyUs = 0.0;
    double   p99LatencyUs = 0.0;
    double   throughputPerSec = 0.0;
};

/// Decode task submitted to the lock-free queue.
struct DecodeTask {
    std::wstring filePath;
    uint32_t     targetWidth = 256;
    uint32_t     targetHeight = 256;
    uint64_t     priority = 0;
    std::function<void(bool, std::vector<uint8_t>&)> callback;
};

/// Lock-free bounded MPMC queue with background decode workers.
///
/// Uses Dmitry Vyukov's bounded MPMC queue algorithm: each ring buffer slot
/// has an atomic sequence counter. Producers CAS the head index to claim a
/// slot, write the task, then advance the slot's sequence. Consumers CAS the
/// tail index, read the task, then advance the sequence by the buffer capacity
/// to mark it free for reuse. No mutexes are required.
class LockFreeDecodePipeline {
public:
    // ====================================================================
    // Backward-compatible static API (v14)
    // ====================================================================

    static constexpr size_t StageStateCount() {
        return static_cast<size_t>(PipelineStageState::COUNT);
    }

    static constexpr size_t PolicyCount() {
        return static_cast<size_t>(OverflowPolicy::COUNT);
    }

    static inline const wchar_t* StageStateName(PipelineStageState s) {
        switch (s) {
        case PipelineStageState::Idle:     return L"Idle";
        case PipelineStageState::Queued:   return L"Queued";
        case PipelineStageState::Decoding: return L"Decoding";
        case PipelineStageState::Scaling:  return L"Scaling";
        case PipelineStageState::Caching:  return L"Caching";
        case PipelineStageState::Complete: return L"Complete";
        case PipelineStageState::Failed:   return L"Failed";
        default:                           return L"Unknown";
        }
    }

    static inline const wchar_t* PolicyName(OverflowPolicy p) {
        switch (p) {
        case OverflowPolicy::Block:        return L"Block";
        case OverflowPolicy::DropOldest:   return L"Drop Oldest";
        case OverflowPolicy::DropNewest:   return L"Drop Newest";
        case OverflowPolicy::ExpandBuffer: return L"Expand Buffer";
        default:                           return L"Unknown";
        }
    }

    /// Round up to next power of 2 (for ring buffer sizing).
    static inline uint32_t NextPowerOf2(uint32_t v) {
        v--;
        v |= v >> 1;
        v |= v >> 2;
        v |= v >> 4;
        v |= v >> 8;
        v |= v >> 16;
        v++;
        return v;
    }

    /// Check if value is a power of 2.
    static inline bool IsPowerOf2(uint32_t v) {
        return v > 0 && (v & (v - 1)) == 0;
    }

    /// Construct with ring buffer capacity (rounded to next power-of-2).
    /// Default capacity is 1024 slots.
    explicit LockFreeDecodePipeline(uint32_t capacity = 1024)
        : m_capacity(NextPowerOf2(capacity))
        , m_mask(m_capacity - 1)
        , m_head(0)
        , m_tail(0)
        , m_stopFlag(false)
        , m_itemsProcessed(0)
        , m_itemsDropped(0) {
        m_slots = std::make_unique<Slot[]>(m_capacity);
        for (uint32_t i = 0; i < m_capacity; ++i) {
            m_slots[i].sequence.store(static_cast<uint64_t>(i),
                std::memory_order_relaxed);
        }
    }

    ~LockFreeDecodePipeline() {
        StopWorkers();
    }

    LockFreeDecodePipeline(const LockFreeDecodePipeline&) = delete;
    LockFreeDecodePipeline& operator=(const LockFreeDecodePipeline&) = delete;

    /// CAS-based lock-free enqueue. Returns false if the queue is full.
    /// The task is moved into the ring buffer slot on success.
    inline bool TryEnqueue(DecodeTask&& task) {
        uint64_t pos = m_head.load(std::memory_order_relaxed);
        for (;;) {
            Slot& slot = m_slots[static_cast<size_t>(pos & m_mask)];
            uint64_t seq = slot.sequence.load(std::memory_order_acquire);
            auto diff = static_cast<int64_t>(seq) - static_cast<int64_t>(pos);

            if (diff == 0) {
                // Slot is free at this position — attempt to claim it
                if (m_head.compare_exchange_weak(pos, pos + 1,
                    std::memory_order_relaxed)) {
                    slot.task = std::move(task);
                    slot.sequence.store(pos + 1, std::memory_order_release);
                    return true;
                }
                // CAS failed — another producer won; pos is reloaded by CAS
            }
            else if (diff < 0) {
                // Queue is full — the slot hasn't been consumed yet
                m_itemsDropped.fetch_add(1, std::memory_order_relaxed);
                return false;
            }
            else {
                // Slot sequence is ahead of pos — reload head
                pos = m_head.load(std::memory_order_relaxed);
            }
        }
    }

    /// CAS-based lock-free dequeue. Returns false if the queue is empty.
    /// The task is moved out of the ring buffer slot on success.
    inline bool TryDequeue(DecodeTask& outTask) {
        uint64_t pos = m_tail.load(std::memory_order_relaxed);
        for (;;) {
            Slot& slot = m_slots[static_cast<size_t>(pos & m_mask)];
            uint64_t seq = slot.sequence.load(std::memory_order_acquire);
            auto diff = static_cast<int64_t>(seq) - static_cast<int64_t>(pos + 1);

            if (diff == 0) {
                // Slot has data ready — attempt to claim it
                if (m_tail.compare_exchange_weak(pos, pos + 1,
                    std::memory_order_relaxed)) {
                    outTask = std::move(slot.task);
                    slot.sequence.store(pos + m_capacity,
                        std::memory_order_release);
                    return true;
                }
            }
            else if (diff < 0) {
                // Queue is empty
                return false;
            }
            else {
                pos = m_tail.load(std::memory_order_relaxed);
            }
        }
    }

    /// Approximate pending count (lock-free snapshot of head and tail).
    inline uint32_t QueueDepth() const {
        uint64_t h = m_head.load(std::memory_order_acquire);
        uint64_t t = m_tail.load(std::memory_order_acquire);
        return static_cast<uint32_t>(h >= t ? h - t : 0u);
    }

    /// Set the decode function that worker threads invoke for each task.
    /// If not set, workers invoke the task's callback with (false, empty).
    inline void SetDecodeFunction(std::function<void(DecodeTask&)> fn) {
        m_decodeFn = std::move(fn);
    }

    /// Spawn the specified number of worker threads that drain the queue.
    inline void StartWorkers(uint32_t count) {
        m_stopFlag.store(false, std::memory_order_release);
        m_workers.reserve(count);
        for (uint32_t i = 0; i < count; ++i) {
            m_workers.emplace_back([this]() { WorkerLoop(); });
        }
    }

    /// Signal all workers to stop and join their threads.
    inline void StopWorkers() {
        m_stopFlag.store(true, std::memory_order_release);
        for (auto& w : m_workers) {
            if (w.joinable()) w.join();
        }
        m_workers.clear();
    }

    /// Total tasks successfully dequeued and processed by workers.
    inline uint64_t GetItemsProcessed() const {
        return m_itemsProcessed.load(std::memory_order_acquire);
    }

    /// Total tasks dropped because the queue was full.
    inline uint64_t GetItemsDropped() const {
        return m_itemsDropped.load(std::memory_order_acquire);
    }

    /// Ring buffer capacity (always a power of 2).
    inline uint32_t Capacity() const { return m_capacity; }

    /// Check whether worker threads are running.
    inline bool IsRunning() const {
        return !m_stopFlag.load(std::memory_order_acquire) && !m_workers.empty();
    }

private:
    /// Cache-line-aligned slot with per-slot sequence counter.
    /// The sequence counter coordinates producers and consumers without
    /// a global lock. Each slot cycles through:
    ///   writable(pos) → readable(pos+1) → writable(pos+capacity) → ...
    struct alignas(64) Slot {
        std::atomic<uint64_t> sequence{ 0 };
        DecodeTask            task;
    };

    /// Worker thread main loop: dequeue tasks and invoke the decode function
    /// or the per-task callback. Yields on empty queue to avoid busy-spinning.
    inline void WorkerLoop() {
        DecodeTask task;
        while (!m_stopFlag.load(std::memory_order_acquire)) {
            if (TryDequeue(task)) {
                if (m_decodeFn) {
                    m_decodeFn(task);
                }
                else if (task.callback) {
                    std::vector<uint8_t> emptyResult;
                    task.callback(false, emptyResult);
                }
                m_itemsProcessed.fetch_add(1, std::memory_order_relaxed);
            }
            else {
                // No work available — yield to avoid burning CPU
                std::this_thread::yield();
            }
        }
        // Drain remaining items on shutdown so callbacks are not lost
        while (TryDequeue(task)) {
            if (m_decodeFn) {
                m_decodeFn(task);
            }
            else if (task.callback) {
                std::vector<uint8_t> emptyResult;
                task.callback(false, emptyResult);
            }
            m_itemsProcessed.fetch_add(1, std::memory_order_relaxed);
        }
    }

    // Ring buffer storage
    uint32_t                          m_capacity;
    uint32_t                          m_mask;
    std::unique_ptr<Slot[]>           m_slots;

    // Head and tail on separate cache lines to prevent false sharing
    alignas(64) std::atomic<uint64_t> m_head;
    alignas(64) std::atomic<uint64_t> m_tail;

    // Control and stats
    std::atomic<bool>                 m_stopFlag;
    std::atomic<uint64_t>             m_itemsProcessed;
    std::atomic<uint64_t>             m_itemsDropped;
    std::function<void(DecodeTask&)>  m_decodeFn;
    std::vector<std::thread>          m_workers;
};

} // namespace Engine
} // namespace ExplorerLens

#pragma warning(pop)
