// DirectoryPreScanQueue.cpp — Background Directory Pre-Scan Queue
// Copyright (c) 2026 ExplorerLens Project
//
#include "Pipeline/DirectoryPreScanQueue.h"
#include <algorithm>
#include <deque>
#include <mutex>
#include <thread>
#include <vector>
#include <atomic>

#if defined(_WIN32)
#   include <windows.h>
#endif

namespace ExplorerLens { namespace Engine {

bool DirectoryPreScanQueue::IsNetworkPath(const std::wstring& path) noexcept
{
    if (path.size() >= 2) {
        // UNC path: \\server\share
        if (path[0] == L'\\' && path[1] == L'\\') return true;
    }
#if defined(_WIN32)
    if (!path.empty()) {
        wchar_t rootBuf[8] = {};
        if (path.size() >= 3) {
            rootBuf[0] = path[0];
            rootBuf[1] = path[1];
            rootBuf[2] = path[2];
            rootBuf[3] = L'\0';
        }
        if (GetDriveTypeW(rootBuf) == DRIVE_REMOTE) return true;
    }
#endif
    return false;
}

struct DirectoryPreScanQueue::Impl {
    std::deque<PreScanEntry>  queues[4]; // one per ScanPriority
    std::mutex                queueMutex;
    std::vector<std::thread>  workers;
    std::atomic<bool>         stopFlag    { false };
    std::atomic<uint32_t>     queueDepth  { 0 };
};

DirectoryPreScanQueue::DirectoryPreScanQueue(const DirectoryPreScanConfig& config)
    : m_config(config)
{
    m_impl = new Impl{};
}

DirectoryPreScanQueue::~DirectoryPreScanQueue()
{
    Stop();
    delete m_impl;
}

void DirectoryPreScanQueue::SetPreGenCallback(PreGenCallback cb) noexcept
{
    m_callback = std::move(cb);
}

uint32_t DirectoryPreScanQueue::EnqueueDirectory(const std::wstring& dirPath) noexcept
{
    if (m_config.skipNetworkDrives && IsNetworkPath(dirPath)) return 0;

    uint32_t count = 0;
#if defined(_WIN32)
    std::wstring pattern = dirPath;
    if (!pattern.empty() && pattern.back() != L'\\') pattern += L'\\';
    pattern += L"*.*";

    WIN32_FIND_DATAW fd{};
    HANDLE h = FindFirstFileW(pattern.c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return 0;

    std::lock_guard<std::mutex> lk(m_impl->queueMutex);
    const std::wstring base = dirPath + (dirPath.back() == L'\\' ? L"" : L"\\");

    do {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
        if (m_impl->queueDepth.load() >= m_config.maxQueueDepth)  break;

        PreScanEntry e{};
        e.filePath = base + fd.cFileName;
        e.priority = ScanPriority::Background;
        e.fileSize = (static_cast<uint64_t>(fd.nFileSizeHigh) << 32u) | fd.nFileSizeLow;
        m_impl->queues[static_cast<int>(ScanPriority::Background)].push_back(std::move(e));
        ++count;
        m_impl->queueDepth.fetch_add(1);
    } while (FindNextFileW(h, &fd));

    FindClose(h);
#endif
    return count;
}

void DirectoryPreScanQueue::Promote(const std::wstring& filePath) noexcept
{
    std::lock_guard<std::mutex> lk(m_impl->queueMutex);
    for (int p = 1; p <= 3; ++p) {
        auto& q = m_impl->queues[p];
        for (auto it = q.begin(); it != q.end(); ++it) {
            if (it->filePath == filePath) {
                PreScanEntry e = std::move(*it);
                e.priority = ScanPriority::Immediate;
                q.erase(it);
                m_impl->queues[0].push_front(std::move(e));
                return;
            }
        }
    }
}

void DirectoryPreScanQueue::Start() noexcept
{
    if (m_running) return;
    m_running = true;
    m_impl->stopFlag.store(false);

    for (uint32_t t = 0; t < m_config.workerThreadCount; ++t) {
        m_impl->workers.emplace_back([this]() {
#if defined(_WIN32)
            SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_LOWEST);
#endif
            while (!m_impl->stopFlag.load()) {
                std::wstring path;
                {
                    std::lock_guard<std::mutex> lk(m_impl->queueMutex);
                    for (int p = 0; p <= 3; ++p) {
                        if (!m_impl->queues[p].empty()) {
                            path = m_impl->queues[p].front().filePath;
                            m_impl->queues[p].pop_front();
                            m_impl->queueDepth.fetch_sub(1);
                            break;
                        }
                    }
                }
                if (!path.empty() && m_callback) m_callback(path);
                else                             std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
        });
    }
}

void DirectoryPreScanQueue::Stop() noexcept
{
    if (!m_running) return;
    m_impl->stopFlag.store(true);
    for (auto& t : m_impl->workers) if (t.joinable()) t.join();
    m_impl->workers.clear();
    m_running = false;
}

uint32_t DirectoryPreScanQueue::QueueDepth() const noexcept
{
    return m_impl->queueDepth.load();
}

bool DirectoryPreScanQueue::IsRunning() const noexcept { return m_running; }

}} // namespace ExplorerLens::Engine
