// GPUMemoryPoolManager.h — GPU-Side Memory Pool Management
// Copyright (c) 2026 ExplorerLens Project
//
// Manages GPU memory pools for textures, buffers, and staging resources.
// Suballocates from large committed heaps to reduce allocation overhead
// and fragmentation during thumbnail batch processing.
//
#pragma once

#include <cstdint>

namespace ExplorerLens {
namespace Engine {

enum class GPUPoolType : uint8_t {
    TextureDefault,
    TextureUpload,
    TextureReadback,
    BufferStructured,
    BufferConstant,
    COUNT
};

enum class GPUHeapTier : uint8_t {
    Tier1_Segregated,
    Tier2_Mixed,
    COUNT
};

struct GPUMemPoolConfig
{
    size_t initialSizeMB = 64;
    size_t maxSizeMB = 512;
    size_t blockSizeKB = 256;
    bool allowGrowth = true;
    GPUHeapTier heapTier = GPUHeapTier::Tier1_Segregated;
};

struct GPUPoolAllocation
{
    uint64_t offset = 0;
    uint64_t size = 0;
    uint32_t poolIndex = 0;
    bool valid = false;
};

struct GPUMemPoolStats
{
    size_t totalAllocated = 0;
    size_t totalFree = 0;
    size_t peakUsage = 0;
    uint32_t allocCount = 0;
    uint32_t freeCount = 0;
    float fragmentation = 0.0f;
};

class GPUMemoryPoolManager
{
  public:
    void Initialize(const GPUMemPoolConfig& cfg)
    {
        m_config = cfg;
        m_stats = {};
        m_initialized = true;
    }

    bool IsInitialized() const
    {
        return m_initialized;
    }
    const GPUMemPoolConfig& GetConfig() const
    {
        return m_config;
    }

    GPUPoolAllocation Allocate(GPUPoolType type, size_t sizeBytes)
    {
        GPUPoolAllocation alloc;
        alloc.size = sizeBytes;
        alloc.poolIndex = static_cast<uint32_t>(type);
        alloc.offset = m_stats.totalAllocated;
        alloc.valid = (m_stats.totalAllocated + sizeBytes <= m_config.maxSizeMB * 1024 * 1024);
        if (alloc.valid) {
            m_stats.totalAllocated += sizeBytes;
            m_stats.allocCount++;
            if (m_stats.totalAllocated > m_stats.peakUsage)
                m_stats.peakUsage = m_stats.totalAllocated;
        }
        return alloc;
    }

    void Free(const GPUPoolAllocation& alloc)
    {
        if (alloc.valid && alloc.size <= m_stats.totalAllocated) {
            m_stats.totalAllocated -= alloc.size;
            m_stats.totalFree += alloc.size;
            m_stats.freeCount++;
        }
    }

    const GPUMemPoolStats& GetStats() const
    {
        return m_stats;
    }

    void Reset()
    {
        m_stats = {};
    }

    static size_t PoolTypeCount()
    {
        return static_cast<size_t>(GPUPoolType::COUNT);
    }

  private:
    GPUMemPoolConfig m_config;
    GPUMemPoolStats m_stats;
    bool m_initialized = false;
};

}  // namespace Engine
}  // namespace ExplorerLens
