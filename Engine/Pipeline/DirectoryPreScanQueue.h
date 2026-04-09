// DirectoryPreScanQueue.h — Background Directory Pre-Scan Queue
// Copyright (c) 2026 ExplorerLens Project
//
// Enumerates all files in a folder on first Explorer open and builds a
// priority-ordered background pre-generation queue. Visible files get
// immediate priority; adjacent files fill next; deep scan fills last.
// Throttles automatically on battery or thermal pressure.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <functional>

namespace ExplorerLens { namespace Engine {

enum class ScanPriority : uint8_t {
    Immediate  = 0,   // Visible viewport — thumbnail request in flight
    Adjacent   = 1,   // Files adjacent to visible range (±2 scroll pages)
    Background = 2,   // All remaining files in directory
    Deferred   = 3,   // Deep subdirectory scan
};

struct PreScanEntry {
    std::wstring filePath;
    ScanPriority priority = ScanPriority::Background;
    uint64_t     fileSize = 0;
    bool         isQueued = false;
};

struct DirectoryPreScanConfig {
    uint32_t maxQueueDepth     = 4096;    // Max pending entries across all priorities
    uint32_t workerThreadCount = 4;       // Background pre-gen thread pool size
    uint32_t batteryThreshold  = 20;      // % battery below which pre-gen is disabled
    bool     skipNetworkDrives = true;    // Skip UNC / SMB paths
    bool     skipRemovable     = false;   // Optionally skip removable media
};

class DirectoryPreScanQueue {
public:
    using PreGenCallback = std::function<void(const std::wstring& path)>;

    explicit DirectoryPreScanQueue(const DirectoryPreScanConfig& config = {});
    ~DirectoryPreScanQueue();

    // Enqueue all files in `dirPath` for background pre-generation.
    // Returns number of files queued (0 if network/battery throttled).
    uint32_t EnqueueDirectory(const std::wstring& dirPath) noexcept;

    // Promote a specific file to Immediate priority (user scrolled over it).
    void Promote(const std::wstring& filePath) noexcept;

    // Set the callback invoked by worker threads when a file should be pre-generated.
    void SetPreGenCallback(PreGenCallback cb) noexcept;

    // Start/stop background worker threads.
    void Start() noexcept;
    void Stop()  noexcept;

    uint32_t QueueDepth() const noexcept;
    bool     IsRunning()  const noexcept;

    // Check if a path is a network drive (UNC or mapped network drive).
    static bool IsNetworkPath(const std::wstring& path) noexcept;

private:
    struct Impl;
    Impl* m_impl = nullptr;
    DirectoryPreScanConfig m_config;
    PreGenCallback m_callback;
    bool m_running = false;
};

}} // namespace ExplorerLens::Engine
