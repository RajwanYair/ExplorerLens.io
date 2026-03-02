#pragma once
/******************************************************************************
 * CacheWarmingService.h
 * Copyright (c) 2026 ExplorerLens Project
 *
 * PURPOSE:
 *   Pre-populates the thumbnail cache by monitoring directories for
 *   Explorer navigation via ReadDirectoryChangesW with IOCP, then
 *   enumerating supported image files and triggering warming callbacks.
 *
 * CLASSES:
 *   CacheWarmingService — Monitors up to 8 directories concurrently using
 *     IOCP-based ReadDirectoryChangesW. When changes detected, enumerates
 *     supported file types and fires warming callback with rate limiting
 *     (max 20 requests/second). Provides global start/stop and stats.
 *
 * INPUTS:
 *   AddWatchDirectory(path) — directories to monitor.
 *   SetWarmingCallback(fn)  — called per file to pre-decode.
 *
 * OUTPUTS:
 *   WarmingStats — directories watched, files warmed, queue depth.
 *   Backward-compatible static methods for enum name lookup.
 *
 * THREAD SAFETY:
 *   SRWLOCK protects directory list and stats. IOCP thread is internal.
 *****************************************************************************/

#include <windows.h>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <atomic>
#include <functional>
#include <queue>
#include <chrono>
#include <algorithm>
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

// ─── Backward-compatible enums ───────────────────────────────────────────────

enum class WarmingMode : uint8_t { Idle = 0, Proactive, OnDemand, COUNT };

enum class WarmingStrategy : uint8_t {
    MostRecent, MostFrequent, DirectoryWatch, Schedule, Predictive, COUNT
};

enum class WarmingPriority : uint8_t {
    Idle, Low, Normal, High, COUNT
};

enum class WarmingJobStatus : uint8_t {
    Queued, Running, Paused, Complete, Failed, Cancelled, COUNT
};

struct CacheWarmingStats {
    uint64_t filesWarmed = 0;
    uint64_t bytesProcessed = 0;
    uint64_t cacheHitsAvoided = 0;
    uint32_t errorsSkipped = 0;
    double   totalTimeSeconds = 0;
    double   avgTimePerFile = 0;
    uint32_t directoriesWatched = 0;
    uint32_t warmingQueueDepth = 0;
};

struct CacheWarmingConfig {
    WarmingStrategy strategy = WarmingStrategy::MostRecent;
    WarmingPriority priority = WarmingPriority::Idle;
    uint32_t        maxConcurrent = 2;
    uint32_t        maxFilesPerSession = 1000;
    uint64_t        maxFileSizeBytes = 100 * 1024 * 1024;
    bool            respectPowerMode = true;
    bool            pauseOnUserActivity = true;
};

// ─── Warming stats (extended) ────────────────────────────────────────────────

struct WarmingStats {
    uint32_t directoriesWatched = 0;
    uint64_t filesWarmed = 0;
    uint64_t cacheHitsAvoided = 0;
    uint32_t warmingQueueDepth = 0;
    uint32_t rateLimitDrops = 0;
};

// ─── Supported extensions ────────────────────────────────────────────────────

inline bool IsSupportedWarmExtension(const std::wstring& ext) {
    static const std::vector<std::wstring> supported = {
        L".jpg", L".jpeg", L".png", L".bmp", L".gif", L".webp",
        L".avif", L".jxl", L".heic", L".heif", L".tif", L".tiff",
        L".svg", L".psd", L".raw", L".cr2", L".nef", L".arw", L".dng"
    };
    for (const auto& s : supported) {
        if (_wcsicmp(ext.c_str(), s.c_str()) == 0) return true;
    }
    return false;
}

// ─── Directory watch context ─────────────────────────────────────────────────

struct WatchContext {
    HANDLE          hDir = INVALID_HANDLE_VALUE;
    OVERLAPPED      overlapped = {};
    alignas(8) BYTE buffer[8192] = {};
    std::wstring    dirPath;
    bool            active = false;
};

// ─── CacheWarmingService ─────────────────────────────────────────────────────

class CacheWarmingService {
public:
    static constexpr uint32_t kMaxConcurrentWatches = 8;
    static constexpr uint32_t kMaxWarmRequestsPerSec = 20;

    CacheWarmingService() {
        ::InitializeSRWLock(&m_srwLock);
        m_iocp = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 1);
    }

    ~CacheWarmingService() {
        StopWarming();
        if (m_iocp != nullptr && m_iocp != INVALID_HANDLE_VALUE) {
            ::CloseHandle(m_iocp);
        }
    }

    CacheWarmingService(const CacheWarmingService&) = delete;
    CacheWarmingService& operator=(const CacheWarmingService&) = delete;

    // ── Directory watch management ──────────────────────────────────────────

    bool AddWatchDirectory(const std::wstring& dirPath) {
        ::AcquireSRWLockExclusive(&m_srwLock);
        if (m_watches.size() >= kMaxConcurrentWatches) {
            ::ReleaseSRWLockExclusive(&m_srwLock);
            return false;
        }
        if (m_watchedDirs.count(dirPath) > 0) {
            ::ReleaseSRWLockExclusive(&m_srwLock);
            return true;
        }

        HANDLE hDir = ::CreateFileW(
            dirPath.c_str(),
            FILE_LIST_DIRECTORY,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            nullptr,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
            nullptr
        );
        if (hDir == INVALID_HANDLE_VALUE) {
            ::ReleaseSRWLockExclusive(&m_srwLock);
            return false;
        }

        auto ctx = std::make_unique<WatchContext>();
        ctx->hDir = hDir;
        ctx->dirPath = dirPath;
        ctx->active = true;
        ::ZeroMemory(&ctx->overlapped, sizeof(OVERLAPPED));

        if (m_iocp != nullptr) {
            ::CreateIoCompletionPort(hDir, m_iocp,
                reinterpret_cast<ULONG_PTR>(ctx.get()), 0);
        }

        // Issue initial ReadDirectoryChangesW
        IssueReadRequest(ctx.get());

        m_watchedDirs.insert(dirPath);
        m_watches.push_back(std::move(ctx));
        ::ReleaseSRWLockExclusive(&m_srwLock);
        return true;
    }

    void RemoveWatchDirectory(const std::wstring& dirPath) {
        ::AcquireSRWLockExclusive(&m_srwLock);
        for (auto it = m_watches.begin(); it != m_watches.end(); ++it) {
            if ((*it)->dirPath == dirPath) {
                (*it)->active = false;
                ::CancelIo((*it)->hDir);
                ::CloseHandle((*it)->hDir);
                m_watches.erase(it);
                break;
            }
        }
        m_watchedDirs.erase(dirPath);
        ::ReleaseSRWLockExclusive(&m_srwLock);
    }

    // ── Warming callback ────────────────────────────────────────────────────

    void SetWarmingCallback(std::function<void(const std::wstring&)> fn) {
        ::AcquireSRWLockExclusive(&m_srwLock);
        m_warmingCallback = std::move(fn);
        ::ReleaseSRWLockExclusive(&m_srwLock);
    }

    // ── Global start/stop ───────────────────────────────────────────────────

    void StartWarming() {
        if (m_running.exchange(true)) return;
        m_iocpThread = std::thread([this]() { IOCPWorkerLoop(); });
        m_rateLimitThread = std::thread([this]() { RateLimitedDispatchLoop(); });
    }

    void StopWarming() {
        if (!m_running.exchange(false)) return;

        // Post quit completion to wake IOCP thread
        if (m_iocp != nullptr) {
            ::PostQueuedCompletionStatus(m_iocp, 0, 0, nullptr);
        }
        m_queueCV.notify_all();

        if (m_iocpThread.joinable()) m_iocpThread.join();
        if (m_rateLimitThread.joinable()) m_rateLimitThread.join();

        // Close all watch handles
        ::AcquireSRWLockExclusive(&m_srwLock);
        for (auto& ctx : m_watches) {
            if (ctx->hDir != INVALID_HANDLE_VALUE) {
                ::CancelIo(ctx->hDir);
                ::CloseHandle(ctx->hDir);
                ctx->hDir = INVALID_HANDLE_VALUE;
            }
        }
        m_watches.clear();
        m_watchedDirs.clear();
        ::ReleaseSRWLockExclusive(&m_srwLock);
    }

    // ── Statistics ──────────────────────────────────────────────────────────

    WarmingStats GetStats() const {
        ::AcquireSRWLockShared(const_cast<PSRWLOCK>(&m_srwLock));
        WarmingStats stats;
        stats.directoriesWatched = static_cast<uint32_t>(m_watchedDirs.size());
        stats.filesWarmed = m_filesWarmed.load();
        stats.cacheHitsAvoided = m_cacheHitsAvoided.load();
        {
            std::lock_guard<std::mutex> ql(const_cast<std::mutex&>(m_queueMutex));
            stats.warmingQueueDepth = static_cast<uint32_t>(
                const_cast<std::queue<std::wstring>&>(m_warmingQueue).size());
        }
        stats.rateLimitDrops = m_rateLimitDrops.load();
        ::ReleaseSRWLockShared(const_cast<PSRWLOCK>(&m_srwLock));
        return stats;
    }

    // ── Backward-compatible static methods ──────────────────────────────────

    static const wchar_t* StrategyName(WarmingStrategy s) {
        switch (s) {
        case WarmingStrategy::MostRecent:    return L"Most Recent";
        case WarmingStrategy::MostFrequent:  return L"Most Frequent";
        case WarmingStrategy::DirectoryWatch:return L"Directory Watch";
        case WarmingStrategy::Schedule:      return L"Scheduled";
        case WarmingStrategy::Predictive:    return L"Predictive";
        default:                             return L"Unknown";
        }
    }

    static const wchar_t* PriorityName(WarmingPriority p) {
        switch (p) {
        case WarmingPriority::Idle:   return L"Idle";
        case WarmingPriority::Low:    return L"Low";
        case WarmingPriority::Normal: return L"Normal";
        case WarmingPriority::High:   return L"High";
        default:                      return L"Unknown";
        }
    }

    static const wchar_t* JobStatusName(WarmingJobStatus s) {
        switch (s) {
        case WarmingJobStatus::Queued:    return L"Queued";
        case WarmingJobStatus::Running:   return L"Running";
        case WarmingJobStatus::Paused:    return L"Paused";
        case WarmingJobStatus::Complete:  return L"Complete";
        case WarmingJobStatus::Failed:    return L"Failed";
        case WarmingJobStatus::Cancelled: return L"Cancelled";
        default:                          return L"Unknown";
        }
    }

    static const wchar_t* ModeName(WarmingMode m) {
        switch (m) {
        case WarmingMode::Idle:      return L"Idle";
        case WarmingMode::Proactive: return L"Proactive";
        case WarmingMode::OnDemand:  return L"On Demand";
        default:                     return L"Unknown";
        }
    }

    static constexpr size_t StrategyCount() { return static_cast<size_t>(WarmingStrategy::COUNT); }
    static constexpr size_t PriorityCount() { return static_cast<size_t>(WarmingPriority::COUNT); }
    static constexpr size_t JobStatusCount() { return static_cast<size_t>(WarmingJobStatus::COUNT); }
    static constexpr size_t ModeCount() { return static_cast<size_t>(WarmingMode::COUNT); }

private:
    // ── IOCP directory monitoring ───────────────────────────────────────────

    void IssueReadRequest(WatchContext* ctx) {
        if (!ctx->active || ctx->hDir == INVALID_HANDLE_VALUE) return;
        DWORD bytesReturned = 0;
        ::ReadDirectoryChangesW(
            ctx->hDir,
            ctx->buffer,
            sizeof(ctx->buffer),
            FALSE,
            FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE
            | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_CREATION,
            &bytesReturned,
            &ctx->overlapped,
            nullptr
        );
    }

    void IOCPWorkerLoop() {
        while (m_running.load()) {
            DWORD bytesTransferred = 0;
            ULONG_PTR key = 0;
            LPOVERLAPPED pOvlp = nullptr;

            BOOL ok = ::GetQueuedCompletionStatus(
                m_iocp, &bytesTransferred, &key, &pOvlp, 1000);

            if (!m_running.load()) break;
            if (!ok || key == 0) continue;

            auto* ctx = reinterpret_cast<WatchContext*>(key);
            if (!ctx->active) continue;

            if (bytesTransferred > 0) {
                ProcessDirectoryChanges(ctx);
            }

            // Re-issue read
            IssueReadRequest(ctx);
        }
    }

    void ProcessDirectoryChanges(WatchContext* ctx) {
        auto* info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(ctx->buffer);
        for (;;) {
            std::wstring fileName(info->FileName, info->FileNameLength / sizeof(WCHAR));

            if (info->Action == FILE_ACTION_ADDED ||
                info->Action == FILE_ACTION_MODIFIED ||
                info->Action == FILE_ACTION_RENAMED_NEW_NAME) {
                // Check extension
                auto dotPos = fileName.find_last_of(L'.');
                if (dotPos != std::wstring::npos) {
                    std::wstring ext = fileName.substr(dotPos);
                    if (IsSupportedWarmExtension(ext)) {
                        std::wstring fullPath = ctx->dirPath;
                        if (!fullPath.empty() && fullPath.back() != L'\\') {
                            fullPath += L'\\';
                        }
                        fullPath += fileName;
                        EnqueueWarmRequest(fullPath);
                    }
                }
            }

            if (info->NextEntryOffset == 0) break;
            info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(
                reinterpret_cast<BYTE*>(info) + info->NextEntryOffset);
        }
    }

    void EnqueueWarmRequest(const std::wstring& path) {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        if (m_warmingQueue.size() < 10000) {
            m_warmingQueue.push(path);
            m_queueCV.notify_one();
        }
        else {
            m_rateLimitDrops++;
        }
    }

    // ── Rate-limited dispatch ───────────────────────────────────────────────

    void RateLimitedDispatchLoop() {
        using clock = std::chrono::steady_clock;
        const auto interval = std::chrono::milliseconds(1000 / kMaxWarmRequestsPerSec);

        while (m_running.load()) {
            std::wstring path;
            {
                std::unique_lock<std::mutex> lock(m_queueMutex);
                m_queueCV.wait_for(lock, std::chrono::milliseconds(200),
                    [this]() { return !m_warmingQueue.empty() || !m_running.load(); });
                if (!m_running.load()) break;
                if (m_warmingQueue.empty()) continue;
                path = m_warmingQueue.front();
                m_warmingQueue.pop();
            }

            // Dispatch callback
            std::function<void(const std::wstring&)> cb;
            {
                ::AcquireSRWLockShared(&m_srwLock);
                cb = m_warmingCallback;
                ::ReleaseSRWLockShared(&m_srwLock);
            }
            if (cb) {
                cb(path);
                m_filesWarmed++;
            }

            // Rate limit
            std::this_thread::sleep_for(interval);
        }
    }

    // ── Members ─────────────────────────────────────────────────────────────

    SRWLOCK m_srwLock{};
    HANDLE m_iocp = INVALID_HANDLE_VALUE;

    std::vector<std::unique_ptr<WatchContext>> m_watches;
    std::unordered_set<std::wstring> m_watchedDirs;

    std::function<void(const std::wstring&)> m_warmingCallback;
    std::atomic<bool> m_running{ false };
    std::thread m_iocpThread;
    std::thread m_rateLimitThread;

    std::mutex m_queueMutex;
    std::condition_variable m_queueCV;
    std::queue<std::wstring> m_warmingQueue;

    std::atomic<uint64_t> m_filesWarmed{ 0 };
    std::atomic<uint64_t> m_cacheHitsAvoided{ 0 };
    std::atomic<uint32_t> m_rateLimitDrops{ 0 };
};

} // namespace Engine
} // namespace ExplorerLens
