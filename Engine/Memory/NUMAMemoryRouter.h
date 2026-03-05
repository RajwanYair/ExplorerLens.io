// NUMAMemoryRouter.h — NUMA-Aware Memory Allocation Router
// Copyright (c) 2026 ExplorerLens Project
//
// Routes memory allocations to the NUMA node closest to the executing
// processor, reducing cross-node memory traffic for decode operations.
// Falls back to standard VirtualAlloc on non-NUMA systems.
//
#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class NUMAStrategy : uint8_t {
    LocalNode,       // Allocate on local NUMA node
    Interleaved,     // Round-robin across nodes
    PreferLocal,     // Try local, fall back to any
    Disabled,        // Standard allocation
    COUNT
};

struct NUMANodeInfo {
    uint32_t nodeId = 0;
    uint64_t totalMemory = 0;
    uint64_t freeMemory = 0;
    uint32_t processorCount = 0;
    bool isLocal = false;
};

struct NUMARouteStats {
    uint64_t localAllocs = 0;
    uint64_t remoteAllocs = 0;
    uint64_t totalBytes = 0;
    uint32_t nodeCount = 0;
    float localityRatio = 1.0f;
};

class NUMAMemoryRouter {
public:
    void SetStrategy(NUMAStrategy s) { m_strategy = s; }
    NUMAStrategy GetStrategy() const { return m_strategy; }

    void DetectTopology() {
        m_nodes.clear();
        ULONG highestNode = 0;
        if (!::GetNumaHighestNodeNumber(&highestNode))
            highestNode = 0;

        // Determine the current processor's preferred node
        PROCESSOR_NUMBER procNum;
        ::GetCurrentProcessorNumberEx(&procNum);
        USHORT localNode = 0;
        ::GetNumaProcessorNodeEx(&procNum, &localNode);

        for (ULONG node = 0; node <= highestNode; ++node) {
            NUMANodeInfo info;
            info.nodeId = node;
            info.isLocal = (node == static_cast<ULONG>(localNode));

            // Query available memory on this node
            ULONGLONG avail = 0;
            if (::GetNumaAvailableMemoryNode(static_cast<UCHAR>(node), &avail))
                info.freeMemory = avail;

            // Count processors on this node via affinity mask
            GROUP_AFFINITY affinity = {};
            if (::GetNumaNodeProcessorMaskEx(static_cast<USHORT>(node), &affinity)) {
                ULONG_PTR mask = affinity.Mask;
                DWORD count = 0;
                while (mask) { count += (mask & 1); mask >>= 1; }
                info.processorCount = count;
            }

            // Estimate total from free (actual total requires WMI; free is sufficient)
            info.totalMemory = info.freeMemory;
            m_nodes.push_back(info);
        }

        m_stats.nodeCount = static_cast<uint32_t>(m_nodes.size());
        m_roundRobin = 0;
    }

    size_t NodeCount() const { return m_nodes.size(); }
    const std::vector<NUMANodeInfo>& Nodes() const { return m_nodes; }
    const NUMARouteStats& Stats() const { return m_stats; }

    void* AllocateOnNode(size_t size, uint32_t nodeId) {
        if (size == 0) return nullptr;

        void* ptr = ::VirtualAllocExNuma(
            ::GetCurrentProcess(),
            nullptr, size,
            MEM_COMMIT | MEM_RESERVE,
            PAGE_READWRITE,
            nodeId);

        // Fallback to standard VirtualAlloc if NUMA alloc fails
        if (!ptr) {
            ptr = ::VirtualAlloc(nullptr, size,
                MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        }

        if (ptr) {
            bool isLocal = IsLocalNode(nodeId);
            if (isLocal) m_stats.localAllocs++;
            else m_stats.remoteAllocs++;
            m_stats.totalBytes += size;
            UpdateLocalityRatio();
        }
        return ptr;
    }

    void* AllocateRouted(size_t size) {
        uint32_t node = ResolveNode();
        return AllocateOnNode(size, node);
    }

    void Free(void* ptr) {
        if (ptr) ::VirtualFree(ptr, 0, MEM_RELEASE);
    }

    static const wchar_t* StrategyName(NUMAStrategy s) {
        switch (s) {
        case NUMAStrategy::LocalNode:    return L"LocalNode";
        case NUMAStrategy::Interleaved:  return L"Interleaved";
        case NUMAStrategy::PreferLocal:  return L"PreferLocal";
        case NUMAStrategy::Disabled:     return L"Disabled";
        default: return L"Unknown";
        }
    }
    static size_t StrategyCount() { return static_cast<size_t>(NUMAStrategy::COUNT); }

private:
    bool IsLocalNode(uint32_t nodeId) const {
        for (const auto& n : m_nodes)
            if (n.nodeId == nodeId) return n.isLocal;
        return (nodeId == 0);
    }

    uint32_t ResolveNode() const {
        if (m_strategy == NUMAStrategy::Disabled || m_nodes.empty())
            return 0;
        if (m_strategy == NUMAStrategy::Interleaved && m_nodes.size() > 1) {
            uint32_t node = m_nodes[m_roundRobin % m_nodes.size()].nodeId;
            const_cast<NUMAMemoryRouter*>(this)->m_roundRobin++;
            return node;
        }
        // LocalNode / PreferLocal: find the local node
        for (const auto& n : m_nodes)
            if (n.isLocal) return n.nodeId;
        return 0;
    }

    void UpdateLocalityRatio() {
        uint64_t total = m_stats.localAllocs + m_stats.remoteAllocs;
        m_stats.localityRatio = (total > 0)
            ? static_cast<float>(m_stats.localAllocs) / static_cast<float>(total)
            : 1.0f;
    }

    NUMAStrategy m_strategy = NUMAStrategy::PreferLocal;
    std::vector<NUMANodeInfo> m_nodes;
    NUMARouteStats m_stats;
    uint32_t m_roundRobin = 0;
};

} // namespace Engine
} // namespace ExplorerLens
