// NetworkAwarePrefetcher.h — Rate-Limited Prefetcher for Cloud Paths
// Copyright (c) 2026 ExplorerLens Project
//
// Extends ThumbnailPrefetcher with network-awareness: throttles prefetch
// requests on metered connections, respects bandwidth budgets, and queues
// cloud file prefetches behind local file requests.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace ExplorerLens {
namespace Engine {

// ---- Network State ----------------------------------------------------------

enum class NetworkCostCategory : uint8_t {
    Unknown       = 0,
    Unrestricted  = 1,   // Ethernet / unmetered Wi-Fi
    Fixed         = 2,   // Wi-Fi with data cap
    Variable      = 3,   // Metered cellular / roaming
    OverDataLimit = 4,   // Cellular over plan limit
    Roaming       = 5,
};

struct NetworkConnectionInfo {
    NetworkCostCategory cost      = NetworkCostCategory::Unknown;
    bool                connected = false;
    uint32_t            latencyMs = 0;
    uint64_t            estimatedBandwidthBps = 0;
};

// ---- Prefetch Job -----------------------------------------------------------

struct CloudPrefetchJob {
    std::string  localPath;      // OneDrive placeholder or remote path
    bool         isCloud         = false;
    uint32_t     priority        = 5;   // 1 (highest) – 10 (lowest)
    uint32_t     estFileSizeKB   = 0;
    bool         cancelled       = false;
};

using PrefetchCompleteCallback = std::function<void(const std::string& path, bool success)>;

// ---- NetworkAwarePrefetcher -------------------------------------------------

class NetworkAwarePrefetcher {
public:
    NetworkAwarePrefetcher();
    ~NetworkAwarePrefetcher();

    // Enqueue a cloud path for prefetch (respects bandwidth policy).
    void Enqueue(CloudPrefetchJob job, PrefetchCompleteCallback cb = nullptr);

    // Cancel all jobs for a given path.
    void Cancel(const std::string& localPath);

    // Cancel all queued cloud jobs (e.g., when switching to airplane mode).
    void CancelAllCloud();

    void Start();
    void Stop();

    // Update network status — called by OS connectivity change notifications.
    void UpdateNetworkStatus(const NetworkConnectionInfo& status);
    NetworkConnectionInfo CurrentStatus() const;

    // Per-policy settings.
    bool  PrefetchOnMetered() const      { return m_prefetchOnMetered; }
    void  SetPrefetchOnMetered(bool v)   { m_prefetchOnMetered = v; }

    uint64_t MaxBandwidthBps() const     { return m_maxBandwidthBps; }
    void     SetMaxBandwidthBps(uint64_t v) { m_maxBandwidthBps = v; }

    static NetworkAwarePrefetcher& Instance();

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
    bool     m_prefetchOnMetered = false;
    uint64_t m_maxBandwidthBps   = 1 * 1024 * 1024; // 1 MB/s default cloud budget
};

} // namespace Engine
} // namespace ExplorerLens
