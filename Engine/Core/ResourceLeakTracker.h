// ResourceLeakTracker.h — Runtime Resource Leak Detection
// Copyright (c) 2026 ExplorerLens Project
//
// Tracks allocation/deallocation of GDI handles, COM objects, GPU
// resources, file handles, and heap memory. Reports leaks at
// checkpoint boundaries and provides call-stack hints for debugging.
//
#pragma once

#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

enum class TrackedResourceType : uint8_t {
    GDIObject,
    COMRef,
    GPUTexture,
    GPUBuffer,
    FileHandle,
    HeapAlloc,
    Event,
    Mutex,
    COUNT
};

struct ResourceRecord
{
    TrackedResourceType type = TrackedResourceType::HeapAlloc;
    uint64_t handle = 0;
    size_t sizeBytes = 0;
    uint32_t threadId = 0;
    uint64_t allocTimeUs = 0;
    bool freed = false;
};

struct LeakReport
{
    uint32_t totalAllocations = 0;
    uint32_t totalFrees = 0;
    uint32_t leakedCount = 0;
    size_t leakedBytes = 0;
    uint32_t leaksByType[static_cast<size_t>(TrackedResourceType::COUNT)] = {};
};

class ResourceLeakTracker
{
  public:
    void Enable(bool enable)
    {
        m_enabled = enable;
    }
    bool IsEnabled() const
    {
        return m_enabled;
    }

    void RecordAlloc(TrackedResourceType type, uint64_t handle, size_t size = 0)
    {
        if (!m_enabled)
            return;
        if (m_recordCount < MAX_RECORDS) {
            auto& r = m_records[m_recordCount++];
            r.type = type;
            r.handle = handle;
            r.sizeBytes = size;
            r.freed = false;
            m_totalAllocs++;
        }
    }

    void RecordFree(TrackedResourceType type, uint64_t handle)
    {
        if (!m_enabled)
            return;
        for (uint32_t i = 0; i < m_recordCount; ++i) {
            if (m_records[i].handle == handle && m_records[i].type == type && !m_records[i].freed) {
                m_records[i].freed = true;
                m_totalFrees++;
                return;
            }
        }
    }

    LeakReport GenerateReport() const
    {
        LeakReport report;
        report.totalAllocations = m_totalAllocs;
        report.totalFrees = m_totalFrees;
        for (uint32_t i = 0; i < m_recordCount; ++i) {
            if (!m_records[i].freed) {
                report.leakedCount++;
                report.leakedBytes += m_records[i].sizeBytes;
                report.leaksByType[static_cast<size_t>(m_records[i].type)]++;
            }
        }
        return report;
    }

    bool HasLeaks() const
    {
        for (uint32_t i = 0; i < m_recordCount; ++i) {
            if (!m_records[i].freed)
                return true;
        }
        return false;
    }

    void Reset()
    {
        m_recordCount = 0;
        m_totalAllocs = 0;
        m_totalFrees = 0;
    }

    static size_t TypeCount()
    {
        return static_cast<size_t>(TrackedResourceType::COUNT);
    }

  private:
    static constexpr uint32_t MAX_RECORDS = 4096;
    ResourceRecord m_records[MAX_RECORDS] = {};
    uint32_t m_recordCount = 0;
    uint32_t m_totalAllocs = 0;
    uint32_t m_totalFrees = 0;
    bool m_enabled = false;
};

}  // namespace Engine
}  // namespace ExplorerLens
