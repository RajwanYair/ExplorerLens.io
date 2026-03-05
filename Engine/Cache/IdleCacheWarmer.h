// IdleCacheWarmer.h — Cache Warming During CPU Idle
// Copyright (c) 2026 ExplorerLens Project
//
// Populates cache entries during detected CPU idle periods using a
// priority queue of warming targets ordered by predicted access likelihood.
//
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <cstdint>
#include <vector>
#include <string>
#include <mutex>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

enum class IdleWarmPriority : uint32_t {
    Critical = 0,
    High = 1,
    Normal = 2,
    Low = 3,
    Background = 4
};

struct WarmTarget {
    std::wstring filePath;
    uint64_t     expectedSizeBytes = 0;
    IdleWarmPriority priority = IdleWarmPriority::Normal;
    bool         warmed = false;
    uint64_t     warmTimeUs = 0;
    uint32_t     accessCount = 0;

    bool operator<(const WarmTarget& other) const {
        return static_cast<uint32_t>(priority) < static_cast<uint32_t>(other.priority);
    }
};

class IdleCacheWarmer {
public:
    static IdleCacheWarmer& Instance() {
        static IdleCacheWarmer s;
        return s;
    }

    void SetTargets(const std::vector<WarmTarget>& targets) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_targets = targets;
        std::sort(m_targets.begin(), m_targets.end());
        m_warmedCount = 0;
        m_totalBytesWarmed = 0;
    }

    void AddTarget(const WarmTarget& target) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_targets.push_back(target);
        std::sort(m_targets.begin(), m_targets.end());
    }

    size_t OnIdle(uint32_t maxWarmMs = 50) {
        std::lock_guard<std::mutex> lock(m_mutex);
        DWORD startTick = GetTickCount();
        size_t warmedThisCycle = 0;

        for (auto& target : m_targets) {
            if (target.warmed) continue;
            if (GetTickCount() - startTick >= maxWarmMs) break;

            LARGE_INTEGER freq, t0, t1;
            QueryPerformanceFrequency(&freq);
            QueryPerformanceCounter(&t0);

            // Attempt file touch to bring into filesystem cache
            HANDLE hFile = CreateFileW(target.filePath.c_str(), GENERIC_READ,
                FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
                FILE_FLAG_SEQUENTIAL_SCAN, nullptr);

            if (hFile != INVALID_HANDLE_VALUE) {
                // Read first 64KB to warm filesystem cache
                uint8_t buffer[65536];
                DWORD bytesRead = 0;
                ReadFile(hFile, buffer, sizeof(buffer), &bytesRead, nullptr);
                CloseHandle(hFile);

                target.warmed = true;
                target.accessCount++;
                m_warmedCount++;
                m_totalBytesWarmed += bytesRead;
                warmedThisCycle++;
            }

            QueryPerformanceCounter(&t1);
            target.warmTimeUs = static_cast<uint64_t>(
                (t1.QuadPart - t0.QuadPart) * 1000000 / freq.QuadPart);
        }

        return warmedThisCycle;
    }

    double GetWarmingProgress() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_targets.empty()) return 1.0;
        return static_cast<double>(m_warmedCount) / m_targets.size();
    }

    size_t GetTargetCount() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_targets.size();
    }

    uint64_t GetTotalBytesWarmed() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_totalBytesWarmed;
    }

    void Reset() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_targets.clear();
        m_warmedCount = 0;
        m_totalBytesWarmed = 0;
    }

    bool Validate() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        size_t actualWarmed = 0;
        for (const auto& t : m_targets) {
            if (t.warmed) actualWarmed++;
            if (static_cast<uint32_t>(t.priority) > 4) return false;
        }
        if (actualWarmed != m_warmedCount) return false;
        double progress = m_targets.empty() ? 1.0 :
            static_cast<double>(m_warmedCount) / m_targets.size();
        return progress >= 0.0 && progress <= 1.0;
    }

private:
    IdleCacheWarmer() = default;
    ~IdleCacheWarmer() = default;
    IdleCacheWarmer(const IdleCacheWarmer&) = delete;
    IdleCacheWarmer& operator=(const IdleCacheWarmer&) = delete;

    mutable std::mutex m_mutex;
    std::vector<WarmTarget> m_targets;
    size_t   m_warmedCount = 0;
    uint64_t m_totalBytesWarmed = 0;
};

}
} // namespace ExplorerLens::Engine
