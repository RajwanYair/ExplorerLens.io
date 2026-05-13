// Engine/Cache/AsyncCacheWriter.h
// ExplorerLens — Bounded async queue for background cache writes (ROADMAP v8.0 Phase 2)
// Sprint S325.
//
// Purpose:
//   The current decode pipeline writes the thumbnail to the L2 disk cache
//   synchronously on the same thread that runs the decode.  Writing a
//   compressed PNG to %LOCALAPPDATA% takes 2–15 ms on spinning disk, directly
//   inflating the p95 IThumbnailProvider latency metric.
//
//   AsyncCacheWriter moves the write onto a dedicated background thread with a
//   bounded FIFO queue.  The decode thread enqueues a write request and returns
//   the HBITMAP to Explorer immediately.  The writer drains the queue at its own pace.
//
//   Bounded queue prevents unbounded RAM growth: if Explorer is generating
//   thumbnails faster than the disk can absorb writes (HDD scenario), new
//   requests are dropped with DropOldest policy — the cache miss is benign.
//
// Thread-safety:
//   Enqueue() is safe to call from any thread.
//   The writer thread owns the disk I/O; no concurrent writes to the same path.
//
#pragma once
#ifndef EXPLORERLENS_ENGINE_ASYNC_CACHE_WRITER_H
#define EXPLORERLENS_ENGINE_ASYNC_CACHE_WRITER_H

#include <cstdint>
#include <cstddef>
#include <atomic>
#include <string>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <stop_token>
#include <functional>

namespace ExplorerLens::Engine {

// ---------------------------------------------------------------------------
// CacheWriteStatus
// ---------------------------------------------------------------------------
enum class CacheWriteStatus : std::uint8_t {
    ENQUEUED        = 0,  ///< Request accepted into the queue
    QUEUE_FULL      = 1,  ///< Queue at capacity; request dropped (oldest evicted)
    WRITER_STOPPED  = 2,  ///< AsyncCacheWriter not running; request rejected
    INVALID_REQUEST = 3,  ///< path empty or pixels empty
};

// ---------------------------------------------------------------------------
// CacheWriteRequest
// ---------------------------------------------------------------------------
struct CacheWriteRequest final {
    std::wstring            cachePath;     ///< Absolute path of the output .png
    std::vector<std::byte>  pixelsBGRA;    ///< Raw BGRA-8888 pixel data
    std::uint32_t           width{};
    std::uint32_t           height{};
    std::uint32_t           qualityHint{}; ///< 0–100; 0 = lossless PNG
};

// ---------------------------------------------------------------------------
// AsyncCacheWriterConfig
// ---------------------------------------------------------------------------
struct AsyncCacheWriterConfig final {
    /// Maximum pending write requests in the queue.
    /// Above this, oldest entries are dropped (DropOldest policy).
    static constexpr std::uint32_t kDefaultQueueDepth = 64u;
    static constexpr std::uint32_t kMinQueueDepth     = 4u;
    static constexpr std::uint32_t kMaxQueueDepth     = 512u;

    std::uint32_t queueDepth = kDefaultQueueDepth;

    /// Priority hint for the writer thread (WIN32 THREAD_PRIORITY_*).
    /// Default: THREAD_PRIORITY_BELOW_NORMAL (-1) — yields to UI threads.
    int threadPriority = -1; // THREAD_PRIORITY_BELOW_NORMAL
};

// ---------------------------------------------------------------------------
// AsyncCacheWriter
// ---------------------------------------------------------------------------
class AsyncCacheWriter final {
public:
    explicit AsyncCacheWriter(AsyncCacheWriterConfig cfg = {}) noexcept
        : m_cfg(cfg)
        , m_enqueuedTotal(0u)
        , m_droppedTotal(0u)
        , m_writtenTotal(0u)
        , m_running(false)
    {}

    ~AsyncCacheWriter() noexcept { Stop(); }

    // Non-copyable, non-movable (owns a thread + mutex)
    AsyncCacheWriter(const AsyncCacheWriter&)            = delete;
    AsyncCacheWriter& operator=(const AsyncCacheWriter&) = delete;
    AsyncCacheWriter(AsyncCacheWriter&&)                 = delete;
    AsyncCacheWriter& operator=(AsyncCacheWriter&&)      = delete;

    // ------------------------------------------------------------------
    // Start() — launch the writer thread.
    // ------------------------------------------------------------------
    bool Start() noexcept
    {
        if (m_running.exchange(true)) return false;  // already running

        m_stopSource = std::stop_source{};
        m_writerThread = std::jthread([this](std::stop_token st) {
            WriterLoop(st);
        });
        return true;
    }

    // ------------------------------------------------------------------
    // Stop() — drain remaining queue, then stop the thread.
    // ------------------------------------------------------------------
    void Stop() noexcept
    {
        if (!m_running.exchange(false)) return;
        m_stopSource.request_stop();
        m_cv.notify_all();
        if (m_writerThread.joinable()) m_writerThread.join();
    }

    // ------------------------------------------------------------------
    // Enqueue() — add a write request.  Thread-safe.
    // ------------------------------------------------------------------
    [[nodiscard]]
    CacheWriteStatus Enqueue(CacheWriteRequest req) noexcept
    {
        if (!m_running.load(std::memory_order_relaxed))
            return CacheWriteStatus::WRITER_STOPPED;
        if (req.cachePath.empty() || req.pixelsBGRA.empty())
            return CacheWriteStatus::INVALID_REQUEST;

        {
            std::unique_lock lock(m_mutex);
            if (m_queue.size() >= m_cfg.queueDepth) {
                m_queue.erase(m_queue.begin());  // drop oldest
                m_droppedTotal.fetch_add(1u, std::memory_order_relaxed);
                m_cv.notify_one();
                return CacheWriteStatus::QUEUE_FULL;
            }
            m_queue.push_back(std::move(req));
            m_enqueuedTotal.fetch_add(1u, std::memory_order_relaxed);
        }
        m_cv.notify_one();
        return CacheWriteStatus::ENQUEUED;
    }

    // ------------------------------------------------------------------
    // Diagnostics
    // ------------------------------------------------------------------
    [[nodiscard]] std::uint64_t EnqueuedTotal() const noexcept
    { return m_enqueuedTotal.load(std::memory_order_relaxed); }

    [[nodiscard]] std::uint64_t DroppedTotal()  const noexcept
    { return m_droppedTotal.load(std::memory_order_relaxed); }

    [[nodiscard]] std::uint64_t WrittenTotal()  const noexcept
    { return m_writtenTotal.load(std::memory_order_relaxed); }

    [[nodiscard]] bool IsRunning() const noexcept
    { return m_running.load(std::memory_order_relaxed); }

    [[nodiscard]] std::uint32_t QueueDepth() const noexcept
    { return m_cfg.queueDepth; }

    // ------------------------------------------------------------------
    // Constants
    // ------------------------------------------------------------------
    static constexpr std::uint32_t kDefaultQueueDepth = AsyncCacheWriterConfig::kDefaultQueueDepth;
    static constexpr std::uint32_t kMinQueueDepth     = AsyncCacheWriterConfig::kMinQueueDepth;
    static constexpr std::uint32_t kMaxQueueDepth     = AsyncCacheWriterConfig::kMaxQueueDepth;

private:
    void WriterLoop(std::stop_token st) noexcept
    {
        while (!st.stop_requested()) {
            CacheWriteRequest req;
            {
                std::unique_lock lock(m_mutex);
                m_cv.wait(lock, [&] {
                    return !m_queue.empty() || st.stop_requested();
                });
                if (m_queue.empty()) break;
                req = std::move(m_queue.front());
                m_queue.erase(m_queue.begin());
            }
            WriteOne(req);
            m_writtenTotal.fetch_add(1u, std::memory_order_relaxed);
        }
    }

    void WriteOne(const CacheWriteRequest& req) noexcept
    {
        // In Phase 2: stub writes are no-ops (no disk I/O in unit tests).
        // Phase 3 will wire this to the SQLite + PNG write path.
        // The infrastructure (queue, thread, stats) is fully functional.
        (void)req;
    }

    AsyncCacheWriterConfig           m_cfg;
    std::atomic<std::uint64_t>       m_enqueuedTotal;
    std::atomic<std::uint64_t>       m_droppedTotal;
    std::atomic<std::uint64_t>       m_writtenTotal;
    std::atomic<bool>                m_running;
    std::stop_source                 m_stopSource;
    std::jthread                     m_writerThread;
    std::mutex                       m_mutex;
    std::condition_variable          m_cv;
    std::vector<CacheWriteRequest>   m_queue;
};

} // namespace ExplorerLens::Engine

#endif // EXPLORERLENS_ENGINE_ASYNC_CACHE_WRITER_H
