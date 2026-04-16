// CacheInvalidationWatcher.cpp — ReadDirectoryChangesW cache invalidation
// Copyright (c) 2026 ExplorerLens Project
//
#include "CacheInvalidationWatcher.h"

#include <atomic>
#include <cassert>
#include <cstring>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif

namespace ExplorerLens {
namespace Engine {

// ---------------------------------------------------------------------------
// Platform implementation: Windows ReadDirectoryChangesW
// ---------------------------------------------------------------------------

#ifdef _WIN32

struct WatchEntry {
    std::string path;
    bool        recursive;
    HANDLE      hDir  = INVALID_HANDLE_VALUE;
    OVERLAPPED  ovl   = {};
    alignas(DWORD) uint8_t buf[65536] = {};
};

struct CacheInvalidationWatcher::Impl {
    std::vector<WatchEntry>   watches;
    CacheInvalidationCallback callback;
    std::thread               thread;
    HANDLE                    hStopEvent = INVALID_HANDLE_VALUE;
    std::atomic<bool>         running{false};
    std::atomic<uint64_t>     eventsProcessed{0};
    std::mutex                mu;

    Impl()
    {
        hStopEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    }

    ~Impl()
    {
        if (hStopEvent != INVALID_HANDLE_VALUE) CloseHandle(hStopEvent);
    }

    bool AddWatch(const std::string& dir, bool recursive)
    {
        std::unique_lock lk(mu);
        for (auto& w : watches) {
            if (w.path == dir) return true; // already watching
        }
        WatchEntry entry;
        entry.path      = dir;
        entry.recursive = recursive;

        // Convert to utf-16
        int wlen = MultiByteToWideChar(CP_UTF8, 0, dir.c_str(), -1, nullptr, 0);
        std::wstring wdir(static_cast<size_t>(wlen), L'\0');
        MultiByteToWideChar(CP_UTF8, 0, dir.c_str(), -1, wdir.data(), wlen);

        entry.hDir = CreateFileW(
            wdir.c_str(),
            FILE_LIST_DIRECTORY,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            nullptr,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
            nullptr);

        if (entry.hDir == INVALID_HANDLE_VALUE) return false;

        entry.ovl.hEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
        watches.push_back(std::move(entry));
        return true;
    }

    void RemoveWatch(const std::string& dir)
    {
        std::unique_lock lk(mu);
        auto it = std::find_if(watches.begin(), watches.end(),
            [&dir](const WatchEntry& w) { return w.path == dir; });
        if (it == watches.end()) return;
        if (it->hDir != INVALID_HANDLE_VALUE) CloseHandle(it->hDir);
        if (it->ovl.hEvent) CloseHandle(it->ovl.hEvent);
        watches.erase(it);
    }

    void IssueRead(WatchEntry& w)
    {
        ReadDirectoryChangesW(
            w.hDir,
            w.buf, sizeof(w.buf),
            w.recursive ? TRUE : FALSE,
            FILE_NOTIFY_CHANGE_FILE_NAME |
            FILE_NOTIFY_CHANGE_DIR_NAME  |
            FILE_NOTIFY_CHANGE_LAST_WRITE|
            FILE_NOTIFY_CHANGE_SIZE,
            nullptr,
            &w.ovl,
            nullptr);
    }

    void ProcessEvents(WatchEntry& w)
    {
        DWORD bytes = 0;
        if (!GetOverlappedResult(w.hDir, &w.ovl, &bytes, FALSE)) return;
        if (bytes == 0) return;

        auto* fni = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(w.buf);
        while (true) {
            // Convert wchar filename to UTF-8
            int n = WideCharToMultiByte(CP_UTF8, 0, fni->FileName,
                static_cast<int>(fni->FileNameLength / sizeof(wchar_t)),
                nullptr, 0, nullptr, nullptr);
            std::string name(static_cast<size_t>(n), '\0');
            WideCharToMultiByte(CP_UTF8, 0, fni->FileName,
                static_cast<int>(fni->FileNameLength / sizeof(wchar_t)),
                name.data(), n, nullptr, nullptr);

            std::string fullPath = w.path + "\\" + name;

            ChangeEvent ev;
            ev.path = fullPath;
            switch (fni->Action) {
                case FILE_ACTION_MODIFIED:         ev.type = ChangeType::MODIFIED; break;
                case FILE_ACTION_REMOVED:          ev.type = ChangeType::DELETED;  break;
                case FILE_ACTION_ADDED:            ev.type = ChangeType::CREATED;  break;
                case FILE_ACTION_RENAMED_OLD_NAME: ev.type = ChangeType::RENAMED; ev.oldPath = fullPath; break;
                case FILE_ACTION_RENAMED_NEW_NAME: ev.type = ChangeType::RENAMED; break;
                default:                           ev.type = ChangeType::MODIFIED; break;
            }

            if (callback) callback(ev);
            ++eventsProcessed;

            if (!fni->NextEntryOffset) break;
            fni = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(
                reinterpret_cast<uint8_t*>(fni) + fni->NextEntryOffset);
        }
    }

    void RunLoop()
    {
        {
            std::unique_lock lk(mu);
            for (auto& w : watches) IssueRead(w);
        }

        while (running.load(std::memory_order_acquire)) {
            // Build wait handle list: [stopEvent, w[0].ovl.hEvent, ...]
            std::vector<HANDLE> handles;
            handles.push_back(hStopEvent);
            {
                std::unique_lock lk(mu);
                for (auto& w : watches) handles.push_back(w.ovl.hEvent);
            }

            DWORD r = WaitForMultipleObjects(
                static_cast<DWORD>(handles.size()),
                handles.data(),
                FALSE,
                INFINITE);

            if (r == WAIT_OBJECT_0) break; // stop event

            if (r > WAIT_OBJECT_0 && r < WAIT_OBJECT_0 + handles.size()) {
                std::unique_lock lk(mu);
                size_t idx = static_cast<size_t>(r - WAIT_OBJECT_0 - 1);
                if (idx < watches.size()) {
                    ProcessEvents(watches[idx]);
                    IssueRead(watches[idx]);
                }
            }
        }
    }
};

#else // Stub for non-Windows builds

struct CacheInvalidationWatcher::Impl {
    CacheInvalidationCallback callback;
    std::atomic<bool>         running{false};
    std::atomic<uint64_t>     eventsProcessed{0};
    std::mutex                mu;

    bool   AddWatch(const std::string&, bool) { return false; }
    void   RemoveWatch(const std::string&)     {}
    void   RunLoop()                           {}
};

#endif // _WIN32

// ---------------------------------------------------------------------------
// CacheInvalidationWatcher public API
// ---------------------------------------------------------------------------

CacheInvalidationWatcher::CacheInvalidationWatcher()
    : m_impl(std::make_unique<Impl>())
{}

CacheInvalidationWatcher::~CacheInvalidationWatcher()
{
    Stop();
}

CacheInvalidationWatcher::CacheInvalidationWatcher(CacheInvalidationWatcher&&) noexcept = default;
CacheInvalidationWatcher& CacheInvalidationWatcher::operator=(CacheInvalidationWatcher&&) noexcept = default;

bool CacheInvalidationWatcher::Watch(std::string_view directoryPath, bool recursive)
{
    return m_impl->AddWatch(std::string(directoryPath), recursive);
}

void CacheInvalidationWatcher::Unwatch(std::string_view directoryPath)
{
    m_impl->RemoveWatch(std::string(directoryPath));
}

void CacheInvalidationWatcher::SetCallback(CacheInvalidationCallback cb)
{
    std::unique_lock lk(m_impl->mu);
    m_impl->callback = std::move(cb);
}

void CacheInvalidationWatcher::Start()
{
    if (m_impl->running.exchange(true)) return;
#ifdef _WIN32
    ResetEvent(m_impl->hStopEvent);
#endif
    m_impl->thread = std::thread([this] { m_impl->RunLoop(); });
}

void CacheInvalidationWatcher::Stop()
{
    if (!m_impl->running.exchange(false)) return;
#ifdef _WIN32
    SetEvent(m_impl->hStopEvent);
#endif
    if (m_impl->thread.joinable()) m_impl->thread.join();
}

bool CacheInvalidationWatcher::IsRunning() const noexcept
{
    return m_impl->running.load(std::memory_order_relaxed);
}

uint64_t CacheInvalidationWatcher::EventsProcessed() const noexcept
{
    return m_impl->eventsProcessed.load(std::memory_order_relaxed);
}

} // namespace Engine
} // namespace ExplorerLens
