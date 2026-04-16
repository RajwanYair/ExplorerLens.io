// PredictivePrefetchEngine.cpp — Folder-navigation-aware thumbnail prefetcher
// Copyright (c) 2026 ExplorerLens Project
//
#include "PredictivePrefetchEngine.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

namespace ExplorerLens {
namespace Engine {

PredictivePrefetchEngine::PredictivePrefetchEngine(uint32_t radius, uint32_t maxQueue)
    : m_radius(radius), m_maxQueue(maxQueue) {}

PredictivePrefetchEngine::~PredictivePrefetchEngine() {
    Stop();
}

void PredictivePrefetchEngine::Start(PrefetchDecodeCallback callback) {
    if (m_running.exchange(true)) return;
    m_decodeCallback = std::move(callback);
    m_worker = std::thread([this] { WorkerLoop(); });
}

void PredictivePrefetchEngine::Stop() {
    m_running.store(false);
    m_queueCV.notify_all();
    if (m_worker.joinable()) m_worker.join();
}

void PredictivePrefetchEngine::RecordAccess(const fs::path& path, uint32_t sizeHint) {
    if (!m_running.load()) return;
    ScheduleNeighbours(path, sizeHint);
}

void PredictivePrefetchEngine::EnqueuePath(const fs::path& path, uint32_t sizeHint, int priority) {
    if (!m_running.load()) return;
    {
        std::lock_guard<std::mutex> lk(m_queueMutex);
        if (m_queue.size() >= m_maxQueue) return;  // Drop if backpressure
        PrefetchRequest req;
        req.path     = path;
        req.sizeHint = sizeHint;
        req.priority = priority;
        m_queue.push_back(std::move(req));
    }
    m_queueCV.notify_one();
}

void PredictivePrefetchEngine::CancelDirectory(const fs::path& dir) {
    std::lock_guard<std::mutex> lk(m_queueMutex);
    m_queue.erase(
        std::remove_if(m_queue.begin(), m_queue.end(),
            [&dir](const PrefetchRequest& r) {
                return r.path.parent_path() == dir;
            }),
        m_queue.end());
}

void PredictivePrefetchEngine::Flush() {
    {
        std::lock_guard<std::mutex> lk(m_queueMutex);
        m_queue.clear();
    }
    m_queueCV.notify_all();
}

void PredictivePrefetchEngine::SetRadius(uint32_t radius) {
    m_radius.store(radius);
}

void PredictivePrefetchEngine::WorkerLoop() {
    while (m_running.load()) {
        PrefetchRequest req;
        {
            std::unique_lock<std::mutex> lk(m_queueMutex);
            m_queueCV.wait_for(lk, std::chrono::milliseconds(100),
                [this] { return !m_queue.empty() || !m_running.load(); });

            if (m_queue.empty()) continue;

            // Pop highest-priority (lowest numeric priority value) item
            auto best = std::min_element(m_queue.begin(), m_queue.end(),
                [](const PrefetchRequest& a, const PrefetchRequest& b) {
                    return a.priority < b.priority;
                });
            req = std::move(*best);
            m_queue.erase(best);
        }

        if (m_decodeCallback) {
            m_decodeCallback(req);
            ++m_totalPrefetched;
        }
    }
}

std::vector<fs::path> PredictivePrefetchEngine::GetDirListing(const fs::path& dir) {
    auto now = std::chrono::steady_clock::now();
    auto ttl = std::chrono::seconds(DIR_CACHE_TTL_SEC);
    auto key = dir.string();

    {
        std::lock_guard<std::mutex> lk(m_dirCacheMutex);
        auto it = m_dirCache.find(key);
        if (it != m_dirCache.end() && it->second.expiry > now) {
            return it->second.files;
        }
    }

    // Build sorted listing of image-like files
    static const std::vector<std::string> imageExts = {
        ".jpg", ".jpeg", ".png", ".webp", ".avif", ".heic", ".heif",
        ".jxl", ".bmp", ".tiff", ".tif", ".gif", ".cr2", ".cr3",
        ".nef", ".arw", ".dng", ".raw", ".raf", ".svg"
    };

    std::vector<fs::path> files;
    std::error_code ec;
    for (const auto& entry : fs::directory_iterator(dir, ec)) {
        if (!entry.is_regular_file(ec)) continue;
        auto ext = entry.path().extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        if (std::find(imageExts.begin(), imageExts.end(), ext) != imageExts.end()) {
            files.push_back(entry.path());
        }
    }
    std::sort(files.begin(), files.end());

    {
        std::lock_guard<std::mutex> lk(m_dirCacheMutex);
        m_dirCache[key] = { files, now + ttl };
    }
    return files;
}

void PredictivePrefetchEngine::ScheduleNeighbours(const fs::path& path, uint32_t sizeHint) {
    auto dir = path.parent_path();
    auto files = GetDirListing(dir);
    if (files.empty()) return;

    auto it = std::find(files.begin(), files.end(), path);
    if (it == files.end()) return;

    auto idx    = static_cast<int>(std::distance(files.begin(), it));
    auto radius = static_cast<int>(m_radius.load());
    int  prio   = 0;

    for (int delta = 1; delta <= radius; ++delta) {
        // Prefer forward neighbours (user scrolls forward more often)
        if (idx + delta < static_cast<int>(files.size())) {
            EnqueuePath(files[static_cast<size_t>(idx + delta)], sizeHint, prio++);
        }
        if (idx - delta >= 0) {
            EnqueuePath(files[static_cast<size_t>(idx - delta)], sizeHint, prio++);
        }
    }
}

} // namespace Engine
} // namespace ExplorerLens
