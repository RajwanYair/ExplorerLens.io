// CacheMetricsCollector.h — Periodic cache metrics harvester
// Copyright (c) 2026 ExplorerLens Project
//
// Periodically calls TwoTierCacheManager::StatsJson() and forwards the
// snapshot to both ETW (TraceLogging) and an optional rolling JSON log on
// disk.  Default collection interval: 30 seconds.  The collector runs on a
// background std::thread and is lifetime-managed via Start/Stop.
//
#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <string>
#include <thread>

namespace ExplorerLens {
namespace Engine {

struct CacheSnapshot {
    double   l1HitRate     = 0.0;
    double   l2HitRate     = 0.0;
    uint64_t l1Inserts     = 0;
    uint64_t l2Inserts     = 0;
    uint64_t totalRequests = 0;
    uint64_t evictions     = 0;
    double   l1FillPercent = 0.0;
    std::string timestamp;          // ISO‑8601 UTC
    std::string rawJson;            // Original StatsJson() output
};

// Callback fired on each collection tick — can be used to forward to ETW/log/metrics
using MetricsCallback = std::function<void(const CacheSnapshot&)>;

class CacheMetricsCollector {
public:
    // Interval in seconds between collections (default: 30s)
    static constexpr uint32_t DEFAULT_INTERVAL_SECONDS = 30;

    explicit CacheMetricsCollector(uint32_t intervalSeconds = DEFAULT_INTERVAL_SECONDS);
    ~CacheMetricsCollector();

    // Start background collection thread.
    // statsProvider: callable returning a StatsJson() string from TwoTierCacheManager.
    // callback:      optional extra sink (ETW, file, metrics endpoint).
    void Start(std::function<std::string()> statsProvider,
               MetricsCallback callback = nullptr);

    void Stop();
    bool IsRunning() const { return m_running.load(); }

    // Enable rolling JSON log to the given file path (max ~1 MB, then rotated once)
    void SetLogFile(const std::string& path);
    void SetLogFile(const std::wstring& path);

    // Get the most recently collected snapshot (thread-safe copy)
    CacheSnapshot LastSnapshot() const;

    CacheMetricsCollector(const CacheMetricsCollector&) = delete;
    CacheMetricsCollector& operator=(const CacheMetricsCollector&) = delete;

private:
    void CollectorLoop();
    CacheSnapshot ParseStats(const std::string& json) const;
    void EmitETW(const CacheSnapshot& snap) const;
    void AppendToLog(const CacheSnapshot& snap) const;

    uint32_t                        m_intervalSeconds;
    std::atomic<bool>               m_running{ false };
    std::thread                     m_thread;
    std::function<std::string()>    m_statsProvider;
    MetricsCallback                 m_callback;
    std::string                     m_logPath;
    mutable CacheSnapshot           m_lastSnapshot;
    mutable std::mutex              m_snapshotMutex;
};

} // namespace Engine
} // namespace ExplorerLens
