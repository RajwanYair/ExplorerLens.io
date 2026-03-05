// NUMAAffinityAllocator.h — NUMA-Aware Memory Allocation
// Copyright (c) 2026 ExplorerLens Project
//
// Provides NUMA-aware allocation for multi-socket systems, placing memory
// close to the processing cores for optimal memory bandwidth.
//
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

struct NUMATopology {
    uint32_t  nodeCount = 1;
    uint32_t  totalLogicalCores = 0;
    struct NodeInfo {
        uint32_t nodeId = 0;
        uint32_t logicalCoreCount = 0;
        uint64_t availableMemoryMB = 0;
        uint64_t totalMemoryMB = 0;
    };
    std::vector<NodeInfo> nodes;
};

struct NUMAAllocationStats {
    uint64_t totalAllocations = 0;
    uint64_t totalFrees = 0;
    uint64_t totalBytesAllocated = 0;
    uint64_t totalBytesFreed = 0;
    uint64_t currentBytes = 0;
    uint64_t peakBytes = 0;
    std::vector<uint64_t> perNodeBytes;
};

struct NUMABlock {
    void* ptr = nullptr;
    size_t    size = 0;
    uint32_t  nodeId = 0;
};

class NUMAAffinityAllocator {
public:
    static NUMAAffinityAllocator& Instance() { static NUMAAffinityAllocator s; return s; }

    NUMATopology GetTopology() {
        if (m_topology.nodeCount > 0 && !m_topology.nodes.empty())
            return m_topology;

        DetectTopology();
        return m_topology;
    }

    void* AllocOnNode(size_t size, uint32_t nodeId = 0) {
        if (size == 0) return nullptr;
        nodeId = (std::min)(nodeId, m_topology.nodeCount > 0 ? m_topology.nodeCount - 1 : 0u);

        void* ptr = ::VirtualAllocExNuma(
            ::GetCurrentProcess(), nullptr, size,
            MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE, nodeId);

        if (!ptr) {
            // Fallback to regular allocation
            ptr = ::VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
            if (!ptr) return nullptr;
        }

        NUMABlock block;
        block.ptr = ptr;
        block.size = size;
        block.nodeId = nodeId;
        m_blocks[reinterpret_cast<uintptr_t>(ptr)] = block;

        m_stats.totalAllocations++;
        m_stats.totalBytesAllocated += size;
        m_stats.currentBytes += size;
        if (m_stats.currentBytes > m_stats.peakBytes)
            m_stats.peakBytes = m_stats.currentBytes;

        if (nodeId < m_stats.perNodeBytes.size())
            m_stats.perNodeBytes[nodeId] += size;

        return ptr;
    }

    bool Free(void* ptr) {
        if (!ptr) return false;
        uintptr_t key = reinterpret_cast<uintptr_t>(ptr);
        auto it = m_blocks.find(key);
        if (it == m_blocks.end()) return false;

        uint32_t nodeId = it->second.nodeId;
        size_t sz = it->second.size;
        ::VirtualFree(ptr, 0, MEM_RELEASE);
        m_blocks.erase(it);

        m_stats.totalFrees++;
        m_stats.totalBytesFreed += sz;
        m_stats.currentBytes -= sz;
        if (nodeId < m_stats.perNodeBytes.size())
            m_stats.perNodeBytes[nodeId] -= sz;

        return true;
    }

    uint32_t GetCurrentNode() const {
        PROCESSOR_NUMBER procNum{};
        ::GetCurrentProcessorNumberEx(&procNum);
        USHORT nodeNumber = 0;
        ::GetNumaProcessorNodeEx(&procNum, &nodeNumber);
        return static_cast<uint32_t>(nodeNumber);
    }

    void* AllocLocal(size_t size) {
        return AllocOnNode(size, GetCurrentNode());
    }

    bool RebalanceAllocations() {
        if (m_topology.nodeCount <= 1) return true; // nothing to rebalance
        // In a real implementation, this would migrate pages between NUMA nodes
        // Here we just verify the distribution
        uint64_t total = m_stats.currentBytes;
        if (total == 0) return true;

        uint64_t idealPerNode = total / m_topology.nodeCount;
        bool balanced = true;
        for (size_t i = 0; i < m_stats.perNodeBytes.size(); ++i) {
            uint64_t nodeBytes = m_stats.perNodeBytes[i];
            if (idealPerNode > 0) {
                float ratio = static_cast<float>(nodeBytes) / static_cast<float>(idealPerNode);
                if (ratio > 2.0f || (nodeBytes > 0 && ratio < 0.1f))
                    balanced = false;
            }
        }
        return balanced;
    }

    const NUMAAllocationStats& GetStats() const { return m_stats; }
    size_t ActiveBlockCount() const { return m_blocks.size(); }

    bool Validate() const {
        if (m_stats.totalAllocations < m_stats.totalFrees) return false;
        uint64_t expectedCurrent = m_stats.totalBytesAllocated - m_stats.totalBytesFreed;
        if (m_stats.currentBytes != expectedCurrent) return false;
        if (m_blocks.size() != m_stats.totalAllocations - m_stats.totalFrees) return false;
        return true;
    }

private:
    NUMAAffinityAllocator() { DetectTopology(); }
    ~NUMAAffinityAllocator() {
        for (auto& [key, block] : m_blocks) {
            if (block.ptr) ::VirtualFree(block.ptr, 0, MEM_RELEASE);
        }
        m_blocks.clear();
    }
    NUMAAffinityAllocator(const NUMAAffinityAllocator&) = delete;
    NUMAAffinityAllocator& operator=(const NUMAAffinityAllocator&) = delete;

    void DetectTopology() {
        ULONG highestNode = 0;
        ::GetNumaHighestNodeNumber(&highestNode);
        m_topology.nodeCount = highestNode + 1;
        m_topology.nodes.clear();

        SYSTEM_INFO sysInfo{};
        ::GetSystemInfo(&sysInfo);
        m_topology.totalLogicalCores = sysInfo.dwNumberOfProcessors;

        for (uint32_t i = 0; i <= highestNode; ++i) {
            NUMATopology::NodeInfo ni;
            ni.nodeId = i;
            GROUP_AFFINITY affinity{};
            if (::GetNumaNodeProcessorMaskEx(static_cast<USHORT>(i), &affinity)) {
                // Count set bits in mask
                uint64_t mask = affinity.Mask;
                uint32_t count = 0;
                while (mask) { count += (mask & 1); mask >>= 1; }
                ni.logicalCoreCount = count;
            }
            ULONGLONG avail = 0;
            if (::GetNumaAvailableMemoryNodeEx(static_cast<USHORT>(i), &avail)) {
                ni.availableMemoryMB = avail / (1024 * 1024);
            }
            m_topology.nodes.push_back(ni);
        }

        m_stats.perNodeBytes.resize(m_topology.nodeCount, 0);
    }

    NUMATopology                                m_topology{};
    NUMAAllocationStats                         m_stats{};
    std::unordered_map<uintptr_t, NUMABlock>    m_blocks;
};

} // namespace Engine
} // namespace ExplorerLens
