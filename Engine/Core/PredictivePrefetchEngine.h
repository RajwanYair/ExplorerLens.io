// PredictivePrefetchEngine.h — Folder-navigation-aware thumbnail prefetcher
// Copyright (c) 2026 ExplorerLens Project
//
// Observes which file paths have been requested and predicts which files are
// likely to be requested next.  Predictions are based on lexicographic
// adjacency within the same directory: when file N is decoded, files N±1
// through N±PrefetchRadius are scheduled for background decode so they are
// in the L1 cache before Explorer requests them.
//
// Thread safety: all public methods are thread-safe.
//
#pragma once

#include <atomic>
#include <condition_variable>
#include <deque>
#include <filesystem>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct PrefetchRequest {
    std::filesystem::path path;
    uint32_t              sizeHint = 256;    // Target thumbnail side length
    int                   priority = 0;      // Lower = higher priority
};

// Callback executed on the prefetch worker thread; should call through to
// ThumbnailPipeline::ProcessAsync() or equivalent decode path.
using PrefetchDecodeCallback = std::function<void(const PrefetchRequest&)>;

class PredictivePrefetchEngine {
public:
    static constexpr uint32_t DEFAULT_RADIUS    = 4;    // Files ahead/behind to prefetch
    static constexpr uint32_t DEFAULT_MAX_QUEUE = 32;   // Max pending requests
    static constexpr uint32_t DIR_CACHE_TTL_SEC = 60;   // Directory listing TTL

    explicit PredictivePrefetchEngine(uint32_t radius   = DEFAULT_RADIUS,
                                      uint32_t maxQueue = DEFAULT_MAX_QUEUE);
    ~PredictivePrefetchEngine();

    // Start/stop background prefetch worker
    void Start(PrefetchDecodeCallback callback);
    void Stop();
    bool IsRunning() const { return m_running.load(); }

    // Called by the decode pipeline after each cache miss to trigger predictions
    void RecordAccess(const std::filesystem::path& path, uint32_t sizeHint = 256);

    // Manual enqueue of a specific path (e.g. from a hover-preview hint)
    void EnqueuePath(const std::filesystem::path& path, uint32_t sizeHint = 256, int priority = 10);

    // Cancel all pending requests for files in a directory (e.g. on folder navigate-away)
    void CancelDirectory(const std::filesystem::path& dir);

    // Drain queue immediately (e.g. at shutdown)
    void Flush();

    // Metrics
    uint64_t TotalPrefetched()  const { return m_totalPrefetched.load(); }
    uint64_t TotalCacheHits()   const { return m_totalCacheHits.load(); }  // Filled by caller via RecordHit()
    void     RecordHit()              { ++m_totalCacheHits; }

    // Tune radius at runtime (e.g. based on observed hit rate)
    void SetRadius(uint32_t radius);
    uint32_t GetRadius() const { return m_radius.load(); }

    PredictivePrefetchEngine(const PredictivePrefetchEngine&) = delete;
    PredictivePrefetchEngine& operator=(const PredictivePrefetchEngine&) = delete;

private:
    void WorkerLoop();
    std::vector<std::filesystem::path> GetDirListing(const std::filesystem::path& dir);
    void ScheduleNeighbours(const std::filesystem::path& path, uint32_t sizeHint);

    std::atomic<bool>     m_running{ false };
    std::atomic<uint32_t> m_radius;
    uint32_t              m_maxQueue;
    std::thread           m_worker;

    std::deque<PrefetchRequest> m_queue;
    std::mutex                  m_queueMutex;
    std::condition_variable     m_queueCV;

    PrefetchDecodeCallback m_decodeCallback;

    // Directory listing cache (path → {sorted files, expiry_time_point})
    struct DirEntry {
        std::vector<std::filesystem::path>          files;
        std::chrono::steady_clock::time_point       expiry;
    };
    std::unordered_map<std::string, DirEntry>       m_dirCache;
    std::mutex                                      m_dirCacheMutex;

    std::atomic<uint64_t> m_totalPrefetched{ 0 };
    std::atomic<uint64_t> m_totalCacheHits{ 0 };
};

} // namespace Engine
} // namespace ExplorerLens
