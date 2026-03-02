#pragma once
// ============================================================================
// AllocationTracker.h — Per-site memory attribution and leak detection wrapper
// ExplorerLens Engine v15.0.0 "Zenith"
// ============================================================================

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <atomic>
#include <mutex>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

// Allocation tag for call-site identification
struct AllocationTag {
    const char* file = nullptr;
    int         line = 0;
    const char* function = nullptr;
    const char* component = nullptr;  // e.g., "JPEG2000Decoder", "BitmapPool"
};

// Per-site allocation record
struct AllocationSiteRecord {
    AllocationTag tag;
    uint64_t currentBytes = 0;
    uint64_t peakBytes = 0;
    uint32_t activeAllocations = 0;
    uint32_t totalAllocations = 0;
    uint32_t totalFrees = 0;
};

// Individual allocation entry for tracking
struct TrackedAllocation {
    void* address = nullptr;
    uint64_t size = 0;
    uint32_t siteId = 0;
    uint64_t timestamp = 0;
};

// Summary by component
struct ComponentMemorySummary {
    std::string component;
    uint64_t    currentBytes = 0;
    uint64_t    peakBytes = 0;
    uint32_t    activeCount = 0;
};

// Global tracker stats
struct AllocationTrackerStats {
    uint64_t totalAllocated = 0;
    uint64_t totalFreed = 0;
    uint64_t currentOutstanding = 0;
    uint64_t peakOutstanding = 0;
    uint32_t activeSiteCount = 0;
    uint32_t leakSuspectCount = 0;
};

// ========================================================================
// AllocationTracker — Debug-mode memory attribution tracker
// ========================================================================
class AllocationTracker {
public:
    static AllocationTracker& Instance() {
        static AllocationTracker instance;
        return instance;
    }

    void Initialize(bool enableTracking = true) {
        m_enabled = enableTracking;
        m_stats = {};
        m_siteRecords.clear();
        m_allocations.clear();
        m_initialized = true;
    }

    bool IsInitialized() const { return m_initialized; }
    bool IsEnabled() const { return m_enabled; }

    // Register a call site (returns site ID)
    uint32_t RegisterSite(const AllocationTag& tag) {
        if (!m_enabled) return 0;
        std::lock_guard<std::mutex> lock(m_mutex);
        uint32_t id = static_cast<uint32_t>(m_siteRecords.size());
        AllocationSiteRecord record;
        record.tag = tag;
        m_siteRecords.push_back(record);
        return id;
    }

    // Track an allocation
    void TrackAlloc(void* address, uint64_t size, uint32_t siteId) {
        if (!m_enabled || !address) return;
        std::lock_guard<std::mutex> lock(m_mutex);

        TrackedAllocation alloc;
        alloc.address = address;
        alloc.size = size;
        alloc.siteId = siteId;
        alloc.timestamp = GetTickCount64();
        m_allocations[address] = alloc;

        if (siteId < m_siteRecords.size()) {
            auto& site = m_siteRecords[siteId];
            site.currentBytes += size;
            site.activeAllocations++;
            site.totalAllocations++;
            if (site.currentBytes > site.peakBytes)
                site.peakBytes = site.currentBytes;
        }

        m_stats.totalAllocated += size;
        m_stats.currentOutstanding += size;
        if (m_stats.currentOutstanding > m_stats.peakOutstanding)
            m_stats.peakOutstanding = m_stats.currentOutstanding;
    }

    // Track a deallocation
    void TrackFree(void* address) {
        if (!m_enabled || !address) return;
        std::lock_guard<std::mutex> lock(m_mutex);

        auto it = m_allocations.find(address);
        if (it != m_allocations.end()) {
            auto& alloc = it->second;
            if (alloc.siteId < m_siteRecords.size()) {
                auto& site = m_siteRecords[alloc.siteId];
                site.currentBytes = (alloc.size <= site.currentBytes) ? (site.currentBytes - alloc.size) : 0;
                site.activeAllocations = (site.activeAllocations > 0) ? (site.activeAllocations - 1) : 0;
                site.totalFrees++;
            }
            m_stats.totalFreed += alloc.size;
            m_stats.currentOutstanding = (alloc.size <= m_stats.currentOutstanding)
                ? (m_stats.currentOutstanding - alloc.size) : 0;
            m_allocations.erase(it);
        }
    }

    // Get top N memory consumers by component
    std::vector<ComponentMemorySummary> GetTopConsumers(uint32_t maxCount = 10) {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::unordered_map<std::string, ComponentMemorySummary> byComponent;

        for (auto& site : m_siteRecords) {
            std::string comp = site.tag.component ? site.tag.component : "Unknown";
            auto& summary = byComponent[comp];
            summary.component = comp;
            summary.currentBytes += site.currentBytes;
            summary.peakBytes += site.peakBytes;
            summary.activeCount += site.activeAllocations;
        }

        std::vector<ComponentMemorySummary> result;
        result.reserve(byComponent.size());
        for (auto& [key, val] : byComponent) {
            result.push_back(val);
        }

        std::sort(result.begin(), result.end(), [](const ComponentMemorySummary& a, const ComponentMemorySummary& b) {
            return a.currentBytes > b.currentBytes;
            });

        if (result.size() > maxCount) result.resize(maxCount);
        return result;
    }

    // Get stats
    AllocationTrackerStats GetStats() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stats.activeSiteCount = static_cast<uint32_t>(m_siteRecords.size());
        m_stats.leakSuspectCount = 0;
        for (auto& site : m_siteRecords) {
            if (site.totalFrees < site.totalAllocations && site.activeAllocations > 0) {
                m_stats.leakSuspectCount++;
            }
        }
        return m_stats;
    }

    // Get sites with potential leaks
    std::vector<AllocationSiteRecord> GetLeakSuspects() {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<AllocationSiteRecord> suspects;
        for (auto& site : m_siteRecords) {
            if (site.activeAllocations > 0 && site.totalFrees < site.totalAllocations) {
                suspects.push_back(site);
            }
        }
        return suspects;
    }

private:
    AllocationTracker() = default;

    bool                   m_enabled = false;
    bool                   m_initialized = false;
    std::mutex             m_mutex;
    AllocationTrackerStats m_stats;
    std::vector<AllocationSiteRecord> m_siteRecords;
    std::unordered_map<void*, TrackedAllocation> m_allocations;
};

// Macro for easy site registration
#define ALLOC_TRACK_SITE(component) \
        static uint32_t s_allocSiteId = AllocationTracker::Instance().RegisterSite({__FILE__, __LINE__, __FUNCTION__, component})

} // namespace Engine
} // namespace ExplorerLens
