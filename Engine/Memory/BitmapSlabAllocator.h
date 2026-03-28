// BitmapSlabAllocator.h — Slab Allocator for Common Bitmap Sizes
// Copyright (c) 2026 ExplorerLens Project
//
// Provides a slab allocator with pre-sized pools for common thumbnail dimensions
// (128x128, 256x256, 512x512 BGRA). Eliminates heap fragmentation for bitmap buffers.
//
#pragma once

#include <cstdint>
#include <vector>
#include <array>
#include <algorithm>
#include <cassert>

namespace ExplorerLens {
namespace Engine {

struct SlabConfig {
    // Pre-defined slab sizes for common thumbnail dimensions (BGRA = 4 bytes)
    static constexpr uint32_t SLAB_128 = 128 * 128 * 4;    // 65536 bytes
    static constexpr uint32_t SLAB_256 = 256 * 256 * 4;    // 262144 bytes
    static constexpr uint32_t SLAB_512 = 512 * 512 * 4;    // 1048576 bytes

    uint32_t  poolCapacity128 = 32;
    uint32_t  poolCapacity256 = 16;
    uint32_t  poolCapacity512 = 8;
    bool      growOnExhaust = true;
    uint32_t  growIncrement = 4;
};

struct BitmapSlabStats {
    uint64_t totalAllocations = 0;
    uint64_t totalFrees = 0;
    uint64_t poolHits = 0;
    uint64_t poolMisses = 0;
    uint64_t currentInUse = 0;
    uint64_t peakInUse = 0;
    uint64_t totalBytesInPool = 0;
    uint32_t pool128Free = 0;
    uint32_t pool256Free = 0;
    uint32_t pool512Free = 0;
};

class BitmapSlabAllocator {
public:
    static BitmapSlabAllocator& Instance() { static BitmapSlabAllocator s; return s; }

    bool Initialize(const SlabConfig& config = SlabConfig{}) {
        m_config = config;
        m_pool128.clear();
        m_pool256.clear();
        m_pool512.clear();
        m_pool128.reserve(config.poolCapacity128);
        m_pool256.reserve(config.poolCapacity256);
        m_pool512.reserve(config.poolCapacity512);

        // Pre-allocate slab blocks
        for (uint32_t i = 0; i < config.poolCapacity128; ++i)
            m_pool128.push_back(std::vector<uint8_t>(SlabConfig::SLAB_128));
        for (uint32_t i = 0; i < config.poolCapacity256; ++i)
            m_pool256.push_back(std::vector<uint8_t>(SlabConfig::SLAB_256));
        for (uint32_t i = 0; i < config.poolCapacity512; ++i)
            m_pool512.push_back(std::vector<uint8_t>(SlabConfig::SLAB_512));

        m_stats.totalBytesInPool =
            static_cast<uint64_t>(config.poolCapacity128) * SlabConfig::SLAB_128 +
            static_cast<uint64_t>(config.poolCapacity256) * SlabConfig::SLAB_256 +
            static_cast<uint64_t>(config.poolCapacity512) * SlabConfig::SLAB_512;

        UpdateFreeStats();
        m_initialized = true;
        return true;
    }

    uint8_t* Allocate(uint32_t sizeBytes) {
        m_stats.totalAllocations++;
        std::vector<std::vector<uint8_t>>* pool = SelectPool(sizeBytes);
        if (pool && !pool->empty()) {
            m_stats.poolHits++;
            // Move last element out
            m_active.push_back(std::move(pool->back()));
            pool->pop_back();
            UpdateFreeStats();
            TrackInUse(1);
            return m_active.back().data();
        }

        // Pool miss — grow or fallback
        m_stats.poolMisses++;
        if (pool && m_config.growOnExhaust) {
            uint32_t slabSize = GetSlabSize(sizeBytes);
            for (uint32_t i = 0; i < m_config.growIncrement; ++i)
                pool->push_back(std::vector<uint8_t>(slabSize));
            m_stats.totalBytesInPool += static_cast<uint64_t>(m_config.growIncrement) * slabSize;
            m_active.push_back(std::move(pool->back()));
            pool->pop_back();
            UpdateFreeStats();
            TrackInUse(1);
            return m_active.back().data();
        }

        // Fallback: direct allocation for non-standard sizes
        m_active.push_back(std::vector<uint8_t>(sizeBytes));
        TrackInUse(1);
        return m_active.back().data();
    }

    bool Free(uint8_t* ptr) {
        if (!ptr) return false;
        for (auto it = m_active.begin(); it != m_active.end(); ++it) {
            if (it->data() == ptr) {
                uint32_t sz = static_cast<uint32_t>(it->size());
                std::vector<std::vector<uint8_t>>* pool = SelectPool(sz);
                if (pool) {
                    pool->push_back(std::move(*it));
                } // else: non-standard size, just drop it
                m_active.erase(it);
                m_stats.totalFrees++;
                m_stats.currentInUse--;
                UpdateFreeStats();
                return true;
            }
        }
        return false;
    }

    BitmapSlabStats GetStats() const { return m_stats; }
    bool IsInitialized() const { return m_initialized; }

    uint32_t GetBestSlabSize(uint32_t width, uint32_t height, uint32_t channels = 4) const {
        uint32_t needed = width * height * channels;
        if (needed <= SlabConfig::SLAB_128) return SlabConfig::SLAB_128;
        if (needed <= SlabConfig::SLAB_256) return SlabConfig::SLAB_256;
        if (needed <= SlabConfig::SLAB_512) return SlabConfig::SLAB_512;
        return needed; // oversized, direct alloc
    }

    bool Validate() const {
        if (!m_initialized) return true;
        if (m_stats.totalAllocations < m_stats.totalFrees) return false;
        if (m_stats.currentInUse != m_stats.totalAllocations - m_stats.totalFrees) return false;
        if (m_stats.poolHits + m_stats.poolMisses != m_stats.totalAllocations) return false;
        return true;
    }

private:
    BitmapSlabAllocator() = default;
    ~BitmapSlabAllocator() = default;
    BitmapSlabAllocator(const BitmapSlabAllocator&) = delete;
    BitmapSlabAllocator& operator=(const BitmapSlabAllocator&) = delete;

    std::vector<std::vector<uint8_t>>* SelectPool(uint32_t size) {
        if (size <= SlabConfig::SLAB_128) return &m_pool128;
        if (size <= SlabConfig::SLAB_256) return &m_pool256;
        if (size <= SlabConfig::SLAB_512) return &m_pool512;
        return nullptr;
    }

    static uint32_t GetSlabSize(uint32_t requested) {
        if (requested <= SlabConfig::SLAB_128) return SlabConfig::SLAB_128;
        if (requested <= SlabConfig::SLAB_256) return SlabConfig::SLAB_256;
        if (requested <= SlabConfig::SLAB_512) return SlabConfig::SLAB_512;
        return requested;
    }

    void UpdateFreeStats() {
        m_stats.pool128Free = static_cast<uint32_t>(m_pool128.size());
        m_stats.pool256Free = static_cast<uint32_t>(m_pool256.size());
        m_stats.pool512Free = static_cast<uint32_t>(m_pool512.size());
    }

    void TrackInUse(int32_t delta) {
        m_stats.currentInUse += delta;
        if (m_stats.currentInUse > m_stats.peakInUse)
            m_stats.peakInUse = m_stats.currentInUse;
    }

    SlabConfig                              m_config{};
    BitmapSlabStats                               m_stats{};
    std::vector<std::vector<uint8_t>>       m_pool128;
    std::vector<std::vector<uint8_t>>       m_pool256;
    std::vector<std::vector<uint8_t>>       m_pool512;
    std::vector<std::vector<uint8_t>>       m_active;
    bool                                    m_initialized = false;
};

} // namespace Engine
} // namespace ExplorerLens
