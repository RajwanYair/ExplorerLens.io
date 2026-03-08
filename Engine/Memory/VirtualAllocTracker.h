// VirtualAllocTracker.h — Tracks VirtualAlloc/VirtualFree calls for leak detection
// Copyright (c) 2026 ExplorerLens Project
//
// Intercepts and tracks virtual memory allocations to detect leaks, measure
// commit charge growth, and report peak virtual memory usage.
//
#pragma once
#include <string>
#include <cstdint>
#include <unordered_map>
#include <mutex>

namespace ExplorerLens {
namespace Engine {

struct VirtualAllocTrackerConfig {
    bool enabled = true;
    uint32_t maxTrackedAllocs = 10000;
    std::string label = "VirtualAllocTracker";
};

class VirtualAllocTracker {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    VirtualAllocTrackerConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    bool TrackAlloc(uintptr_t addr, uint64_t size) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_allocs.size() >= m_config.maxTrackedAllocs) return false;
        m_allocs[addr] = size;
        m_totalAllocated += size;
        if (m_totalAllocated > m_peakAllocated) m_peakAllocated = m_totalAllocated;
        return true;
    }

    bool TrackFree(uintptr_t addr) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_allocs.find(addr);
        if (it == m_allocs.end()) return false;
        m_totalAllocated -= it->second;
        m_allocs.erase(it);
        return true;
    }

    uint64_t GetCurrentAllocated() const { return m_totalAllocated; }
    uint64_t GetPeakAllocated() const { return m_peakAllocated; }
    size_t GetLeakCount() const { return m_allocs.size(); }

private:
    bool m_initialized = false;
    VirtualAllocTrackerConfig m_config;
    mutable std::mutex m_mutex;
    std::unordered_map<uintptr_t, uint64_t> m_allocs;
    uint64_t m_totalAllocated = 0;
    uint64_t m_peakAllocated = 0;
};

}
} // namespace ExplorerLens::Engine
