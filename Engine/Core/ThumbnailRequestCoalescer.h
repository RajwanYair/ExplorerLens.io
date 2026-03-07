// ThumbnailRequestCoalescer.h — Duplicate Request Elimination
// Copyright (c) 2026 ExplorerLens Project
//
// Merges identical or overlapping thumbnail requests to prevent
// redundant decode operations when Explorer rapidly requests
// thumbnails for the same file at different sizes.
//
#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <mutex>
#include <chrono>

namespace ExplorerLens {
namespace Engine {

struct RequestCoalescerConfig {
    uint32_t windowMs = 50;
    bool mergeSubsizes = true;
    uint32_t maxPendingPerFile = 4;
};

struct CoalescedRequest {
    std::wstring filePath;
    uint32_t maxRequestedSize = 0;
    uint32_t requestCount = 0;
    std::chrono::steady_clock::time_point firstRequest;
};

struct RequestCoalescerStats {
    uint64_t totalRequests = 0;
    uint64_t coalescedRequests = 0;
    uint64_t deliveredRequests = 0;

    double savingsRatio() const {
        return totalRequests > 0
            ? static_cast<double>(coalescedRequests) / totalRequests
            : 0.0;
    }
};

class ThumbnailRequestCoalescer {
public:
    explicit ThumbnailRequestCoalescer(RequestCoalescerConfig cfg = {})
        : m_config(cfg) {
    }

    bool Submit(const std::wstring& path, uint32_t size) {
        std::lock_guard lock(m_mtx);
        m_stats.totalRequests++;

        auto it = m_pending.find(path);
        if (it != m_pending.end()) {
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - it->second.firstRequest).count();
            if (elapsed < m_config.windowMs) {
                it->second.requestCount++;
                if (size > it->second.maxRequestedSize)
                    it->second.maxRequestedSize = size;
                m_stats.coalescedRequests++;
                return false; // coalesced
            }
        }

        m_pending[path] = CoalescedRequest{
            path, size, 1, std::chrono::steady_clock::now()
        };
        m_stats.deliveredRequests++;
        return true; // new request
    }

    void Clear() {
        std::lock_guard lock(m_mtx);
        m_pending.clear();
    }

    size_t PendingCount() const { return m_pending.size(); }
    RequestCoalescerStats Stats() const { return m_stats; }

private:
    RequestCoalescerConfig m_config;
    RequestCoalescerStats m_stats;
    std::mutex m_mtx;
    std::unordered_map<std::wstring, CoalescedRequest> m_pending;
};

} // namespace Engine
} // namespace ExplorerLens
