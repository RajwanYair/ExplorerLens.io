// GPUResourceLeakDetector.h — GPU Resource Leak Detection
// Copyright (c) 2026 ExplorerLens Project
//
// Tracks GPU resource allocations (textures, buffers, shaders) and
// detects leaks through reference counting and lifetime analysis.
//
#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>

namespace ExplorerLens {
namespace Engine {

enum class LeakDetectorResourceType : uint8_t {
    Texture2D,
    Buffer,
    ShaderModule,
    RenderTarget,
    DepthStencil,
    ConstantBuffer,
    CommandList,
    PipelineState
};

struct TrackedGPUResource {
    uint64_t resourceId = 0;
    LeakDetectorResourceType type = LeakDetectorResourceType::Texture2D;
    uint64_t sizeBytes = 0;
    std::string allocationSite;
    uint64_t allocatedTimestamp = 0;
    bool released = false;
};

struct GPULeakReport {
    uint64_t totalAllocations = 0;
    uint64_t totalReleases = 0;
    uint64_t leakedResources = 0;
    uint64_t leakedBytes = 0;
    std::vector<TrackedGPUResource> leakedItems;
};

class GPUResourceLeakDetector {
public:
    GPUResourceLeakDetector() = default;

    uint64_t TrackAllocation(LeakDetectorResourceType type, uint64_t sizeBytes,
        const std::string& site = "") {
        std::lock_guard<std::mutex> lock(m_mutex);
        uint64_t id = ++m_nextId;
        TrackedGPUResource res;
        res.resourceId = id;
        res.type = type;
        res.sizeBytes = sizeBytes;
        res.allocationSite = site;
        m_resources[id] = res;
        m_totalAllocations++;
        m_totalBytesAllocated += sizeBytes;
        return id;
    }

    bool TrackRelease(uint64_t resourceId) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_resources.find(resourceId);
        if (it == m_resources.end()) return false;
        it->second.released = true;
        m_totalReleases++;
        m_totalBytesReleased += it->second.sizeBytes;
        return true;
    }

    GPULeakReport GenerateReport() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        GPULeakReport report;
        report.totalAllocations = m_totalAllocations;
        report.totalReleases = m_totalReleases;
        for (const auto& [id, res] : m_resources) {
            if (!res.released) {
                report.leakedResources++;
                report.leakedBytes += res.sizeBytes;
                report.leakedItems.push_back(res);
            }
        }
        return report;
    }

    bool HasLeaks() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (const auto& [id, res] : m_resources) {
            if (!res.released) return true;
        }
        return false;
    }

    uint64_t GetActiveResourceCount() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        uint64_t count = 0;
        for (const auto& [id, res] : m_resources) {
            if (!res.released) count++;
        }
        return count;
    }

    uint64_t GetTotalBytesInUse() const {
        return m_totalBytesAllocated - m_totalBytesReleased;
    }

private:
    mutable std::mutex m_mutex;
    std::unordered_map<uint64_t, TrackedGPUResource> m_resources;
    uint64_t m_nextId = 0;
    uint64_t m_totalAllocations = 0;
    uint64_t m_totalReleases = 0;
    uint64_t m_totalBytesAllocated = 0;
    uint64_t m_totalBytesReleased = 0;
};

} // namespace Engine
} // namespace ExplorerLens
