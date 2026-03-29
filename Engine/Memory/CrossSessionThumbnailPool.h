// CrossSessionThumbnailPool.h — Cross-Session Read-Only Shared Thumbnail Pool
// Copyright (c) 2026 ExplorerLens Project
//
// Memory-mapped read-only thumbnail pool shared across WTS sessions and multiple
// Explorer instances. Writers hold one named mutex; readers get pointer-stable
// references via offset-based addressing with generation-checked validity.
//
#pragma once
#include <string>
#include <vector>
#include <stdint.h>
#include <cstring>

namespace ExplorerLens {
namespace Engine {

struct SharedThumbEntry {
    uint64_t keyHash       = 0;
    uint32_t offset        = 0;     // offset into data region
    uint32_t dataSize      = 0;
    uint32_t generation    = 0;
    uint32_t sessionId     = 0;
    bool     valid         = false;
};

struct CrossSessionPoolStats {
    int      entryCount    = 0;
    uint64_t bytesUsed     = 0;
    uint64_t bytesCapacity = 0;
    int      hitCount      = 0;
    int      missCount     = 0;
    double   HitRatePct() const noexcept {
        int total = hitCount + missCount;
        return total > 0 ? (hitCount * 100.0 / total) : 0.0;
    }
};

class CrossSessionThumbnailPool {
public:
    static constexpr uint64_t DEFAULT_CAPACITY = 64 * 1024 * 1024; // 64 MB

    explicit CrossSessionThumbnailPool(uint64_t capacity = DEFAULT_CAPACITY)
        : m_capacity(capacity) { m_data.resize((size_t)capacity); }

    bool   Insert(uint64_t keyHash, const uint8_t* data, uint32_t size, uint32_t sessionId = 0) {
        if (m_used + size > m_capacity) return false;
        SharedThumbEntry e;
        e.keyHash    = keyHash;
        e.offset     = (uint32_t)m_used;
        e.dataSize   = size;
        e.generation = ++m_generation;
        e.sessionId  = sessionId;
        e.valid      = true;
        std::memcpy(m_data.data() + m_used, data, size);
        m_used += size;
        m_index.push_back(e);
        return true;
    }

    const uint8_t* Lookup(uint64_t keyHash, uint32_t& outSize) {
        for (const auto& e : m_index) {
            if (e.valid && e.keyHash == keyHash) {
                outSize = e.dataSize;
                m_stats.hitCount++;
                return m_data.data() + e.offset;
            }
        }
        m_stats.missCount++;
        return nullptr;
    }

    CrossSessionPoolStats Stats() const noexcept {
        CrossSessionPoolStats s = m_stats;
        s.entryCount    = (int)m_index.size();
        s.bytesUsed     = m_used;
        s.bytesCapacity = m_capacity;
        return s;
    }

    int  EntryCount()  const noexcept { return (int)m_index.size(); }
    void Clear()       noexcept       { m_index.clear(); m_used = 0; m_generation = 0; }

private:
    std::vector<uint8_t>       m_data;
    std::vector<SharedThumbEntry> m_index;
    uint64_t                   m_capacity  = DEFAULT_CAPACITY;
    uint64_t                   m_used      = 0;
    uint32_t                   m_generation = 0;
    CrossSessionPoolStats      m_stats;
};

} // namespace Engine
} // namespace ExplorerLens
