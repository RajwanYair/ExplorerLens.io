// ThumbnailPrefetcher.h — Predictive Pre-Decode Using Access Patterns
// Copyright (c) 2026 ExplorerLens Project
//
// Monitors shell folder navigation events and prefetches thumbnails for
// files adjacent to the cursor using a sliding window prediction model.
//
#pragma once

#include <atomic>
#include <cstdint>
#include <deque>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct PrefetchEntry {
    std::wstring path;
    uint32_t     priority;      // higher = decode sooner
    uint32_t     requestedSize; // thumbnail pixel width
};

struct ThumbnailPrefetchConfig {
    uint32_t windowSize{8};          // number of adjacent files to prefetch
    uint32_t maxQueueDepth{32};      // max entries in prefetch queue
    uint32_t workerThreads{2};       // background decode threads
    uint32_t idleThresholdMs{500};   // queue when cursor idle > this
    bool     enableAccessLog{true};  // learn from navigation patterns
};

using PrefetchDecodeCallback = std::function<bool(const std::wstring& path, uint32_t size)>;

class ThumbnailPrefetcher {
public:
    explicit ThumbnailPrefetcher(ThumbnailPrefetchConfig cfg = {}) : m_cfg(cfg) {}

    ~ThumbnailPrefetcher() { Stop(); }

    ThumbnailPrefetcher(const ThumbnailPrefetcher&) = delete;
    ThumbnailPrefetcher& operator=(const ThumbnailPrefetcher&) = delete;

    void SetDecodeCallback(PrefetchDecodeCallback cb) { m_callback = std::move(cb); }

    // Called when the user navigates to a folder — seeds the prefetch window.
    void OnFolderOpened(std::vector<std::wstring> fileList, uint32_t focusIndex = 0) {
        std::lock_guard lock(m_mutex);
        m_fileList   = std::move(fileList);
        m_focusIndex = focusIndex;
        m_dirty      = true;
        m_cv.notify_one();
    }

    // Called when cursor hovers over a file — updates focus index.
    void OnFileFocused(uint32_t index) {
        std::lock_guard lock(m_mutex);
        m_focusIndex = index;
        m_dirty      = true;
        m_cv.notify_one();
    }

    void Start() {
        m_running = true;
        for (uint32_t i = 0; i < m_cfg.workerThreads; ++i) {
            m_workers.emplace_back([this] { WorkerLoop(); });
        }
        m_scheduler = std::thread([this] { SchedulerLoop(); });
    }

    void Stop() {
        m_running = false;
        m_cv.notify_all();
        if (m_scheduler.joinable()) m_scheduler.join();
        for (auto& w : m_workers) if (w.joinable()) w.join();
        m_workers.clear();
    }

    [[nodiscard]] uint64_t PrefetchHits()   const noexcept { return m_hits.load(); }
    [[nodiscard]] uint64_t PrefetchMisses() const noexcept { return m_misses.load(); }
    [[nodiscard]] size_t   QueueDepth()     const noexcept {
        std::lock_guard lock(m_mutex);
        return m_queue.size();
    }

private:
    void SchedulerLoop() {
        std::unique_lock lock(m_mutex);
        while (m_running) {
            m_cv.wait(lock, [this] { return m_dirty || !m_running; });
            if (!m_running) break;
            m_dirty = false;

            // Build sliding window around focus
            const uint32_t half = m_cfg.windowSize / 2;
            const size_t   sz   = m_fileList.size();
            uint32_t start = (m_focusIndex > half) ? m_focusIndex - half : 0;
            uint32_t end   = std::min(static_cast<uint32_t>(sz), start + m_cfg.windowSize);

            m_queue.clear();
            for (uint32_t i = start; i < end; ++i) {
                if (i == m_focusIndex) continue; // already decoded by shell
                uint32_t dist = (i < m_focusIndex) ? (m_focusIndex - i) : (i - m_focusIndex);
                uint32_t prio = m_cfg.windowSize - dist;
                if (m_queue.size() < m_cfg.maxQueueDepth) {
                    m_queue.push_back({ m_fileList[i], prio, 256 });
                }
            }
            m_workCv.notify_all();
        }
    }

    void WorkerLoop() {
        while (m_running) {
            PrefetchEntry entry{};
            {
                std::unique_lock lock(m_mutex);
                m_workCv.wait(lock, [this] { return !m_queue.empty() || !m_running; });
                if (!m_running) break;
                if (m_queue.empty()) continue;
                entry = m_queue.front();
                m_queue.pop_front();
            }
            if (m_callback) {
                bool hit = m_callback(entry.path, entry.requestedSize);
                if (hit) m_hits.fetch_add(1, std::memory_order_relaxed);
                else     m_misses.fetch_add(1, std::memory_order_relaxed);
            }
        }
    }

    ThumbnailPrefetchConfig              m_cfg;
    PrefetchDecodeCallback      m_callback;

    mutable std::mutex          m_mutex;
    std::condition_variable     m_cv;
    std::condition_variable     m_workCv;

    std::vector<std::wstring>   m_fileList;
    uint32_t                    m_focusIndex{0};
    bool                        m_dirty{false};
    std::deque<PrefetchEntry>   m_queue;

    std::atomic<bool>           m_running{false};
    std::thread                 m_scheduler;
    std::vector<std::thread>    m_workers;

    std::atomic<uint64_t>       m_hits{0};
    std::atomic<uint64_t>       m_misses{0};
};

} // namespace Engine
} // namespace ExplorerLens
