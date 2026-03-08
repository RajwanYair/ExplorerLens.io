// MemoryAllocationTracer.h — Allocation tracing for leak detection
// Copyright (c) 2026 ExplorerLens Project
//
// Traces all memory allocations with source location and size, enabling
// leak detection and allocation profiling in debug builds.
//
#pragma once
#include <string>
#include <cstdint>
#include <vector>

namespace ExplorerLens {
namespace Engine {

struct MemoryAllocationTracerConfig {
    bool enabled = true;
    uint32_t maxTraces = 8192;
    bool captureCallstack = false;
    std::string label = "MemoryAllocationTracer";
};

class MemoryAllocationTracer {
public:
    bool Initialize() {
        if (m_initialized) return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const { return m_initialized; }
    MemoryAllocationTracerConfig GetConfig() const { return m_config; }
    std::string GetName() const { return m_config.label; }

    struct AllocationTrace {
        uint64_t address = 0;
        uint64_t size = 0;
        std::string sourceLoc;
        bool freed = false;
    };

    bool RecordAlloc(uint64_t addr, uint64_t size, const std::string& loc = "") {
        if (m_traces.size() >= m_config.maxTraces) return false;
        m_traces.push_back({ addr, size, loc, false });
        m_totalAllocated += size;
        return true;
    }

    bool RecordFree(uint64_t addr) {
        for (auto& t : m_traces) {
            if (t.address == addr && !t.freed) {
                t.freed = true;
                m_totalFreed += t.size;
                return true;
            }
        }
        return false;
    }

    uint64_t GetLeakedBytes() const { return m_totalAllocated - m_totalFreed; }
    uint32_t GetLeakedCount() const {
        uint32_t count = 0;
        for (const auto& t : m_traces)
            if (!t.freed) count++;
        return count;
    }

private:
    bool m_initialized = false;
    MemoryAllocationTracerConfig m_config;
    std::vector<AllocationTrace> m_traces;
    uint64_t m_totalAllocated = 0;
    uint64_t m_totalFreed = 0;
};

}
} // namespace ExplorerLens::Engine
