// LargePageOptimizer.h — Windows Large Page Optimization
// Copyright (c) 2026 ExplorerLens Project
//
// Manages Windows large page allocations for performance-critical buffers,
// handling privilege acquisition and fallback to standard pages.
//
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <cstdint>
#include <vector>
#include <mutex>
#include <string>

namespace ExplorerLens {
namespace Engine {

struct LargePageConfig {
    bool     enabled = false;
    size_t   minimumAllocationMB = 2;
    bool     fallbackToSmall = true;
    uint32_t maxLargePageAllocs = 64;
};

struct LargePageMetrics {
    uint64_t largePageAllocCount = 0;
    uint64_t fallbackAllocCount = 0;
    uint64_t totalLargePageBytes = 0;
    uint64_t totalFallbackBytes = 0;
    size_t   largePageSizeBytes = 0;
    bool     privilegeAcquired = false;

    double LargePageRatio() const {
        uint64_t total = largePageAllocCount + fallbackAllocCount;
        return total > 0 ? static_cast<double>(largePageAllocCount) / total : 0.0;
    }
};

class LargePageOptimizer {
public:
    static LargePageOptimizer& Instance() {
        static LargePageOptimizer s;
        return s;
    }

    bool EnableLargePages() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_metrics.largePageSizeBytes = GetLargePageMinimum();
        if (m_metrics.largePageSizeBytes == 0) {
            m_config.enabled = false;
            return false;
        }

        HANDLE token = nullptr;
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token)) {
            m_config.enabled = false;
            return false;
        }

        TOKEN_PRIVILEGES tp{};
        tp.PrivilegeCount = 1;
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

        if (LookupPrivilegeValueW(nullptr, L"SeLockMemoryPrivilege", &tp.Privileges[0].Luid)) {
            if (AdjustTokenPrivileges(token, FALSE, &tp, sizeof(tp), nullptr, nullptr)) {
                if (GetLastError() == ERROR_SUCCESS) {
                    m_metrics.privilegeAcquired = true;
                    m_config.enabled = true;
                }
            }
        }

        CloseHandle(token);
        return m_config.enabled;
    }

    void* AllocateLargePage(size_t requestedBytes) {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_config.enabled && m_metrics.largePageSizeBytes > 0) {
            size_t aligned = AlignToLargePage(requestedBytes);
            if (m_metrics.largePageAllocCount < m_config.maxLargePageAllocs) {
                void* ptr = VirtualAlloc(nullptr, aligned,
                    MEM_RESERVE | MEM_COMMIT | MEM_LARGE_PAGES, PAGE_READWRITE);
                if (ptr) {
                    m_metrics.largePageAllocCount++;
                    m_metrics.totalLargePageBytes += aligned;
                    m_allocations.push_back({ ptr, aligned, true });
                    return ptr;
                }
            }
        }

        if (m_config.fallbackToSmall) {
            void* ptr = VirtualAlloc(nullptr, requestedBytes,
                MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            if (ptr) {
                m_metrics.fallbackAllocCount++;
                m_metrics.totalFallbackBytes += requestedBytes;
                m_allocations.push_back({ ptr, requestedBytes, false });
            }
            return ptr;
        }

        return nullptr;
    }

    void FreePage(void* ptr) {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto it = m_allocations.begin(); it != m_allocations.end(); ++it) {
            if (it->address == ptr) {
                VirtualFree(ptr, 0, MEM_RELEASE);
                m_allocations.erase(it);
                return;
            }
        }
    }

    LargePageMetrics GetMetrics() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_metrics;
    }

    LargePageConfig GetConfig() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_config;
    }

    void SetConfig(const LargePageConfig& config) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_config = config;
    }

    bool Validate() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_metrics.largePageAllocCount > m_config.maxLargePageAllocs && m_config.enabled)
            return false;
        size_t tracked = 0;
        for (const auto& a : m_allocations) {
            if (a.address == nullptr) return false;
            tracked += a.size;
        }
        return true;
    }

private:
    LargePageOptimizer() = default;
    ~LargePageOptimizer() {
        for (auto& a : m_allocations) {
            if (a.address) VirtualFree(a.address, 0, MEM_RELEASE);
        }
    }
    LargePageOptimizer(const LargePageOptimizer&) = delete;
    LargePageOptimizer& operator=(const LargePageOptimizer&) = delete;

    size_t AlignToLargePage(size_t bytes) const {
        size_t lp = m_metrics.largePageSizeBytes;
        return lp > 0 ? ((bytes + lp - 1) / lp) * lp : bytes;
    }

    struct AllocationRecord {
        void* address = nullptr;
        size_t size = 0;
        bool   isLarge = false;
    };

    mutable std::mutex m_mutex;
    LargePageConfig m_config;
    LargePageMetrics m_metrics;
    std::vector<AllocationRecord> m_allocations;
};

}
} // namespace ExplorerLens::Engine
