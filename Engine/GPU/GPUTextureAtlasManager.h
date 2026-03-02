// ============================================================================
// GPUTextureAtlasManager.h — Shelf-Packed GPU Texture Atlases
// ExplorerLens Engine v15.0.0  (Sprint 568)
// Copyright (c) 2026 ExplorerLens Project
//
// PURPOSE
//   Packs many small thumbnail images into large atlas textures using a
//   shelf (row) packing algorithm.  Each atlas is a CPU-side RGBA32 buffer
//   that can later be uploaded to a GPU texture in a single call, reducing
//   draw calls and texture-binding overhead when compositing thumbnails.
//
// CLASSES
//   GPUTextureAtlasManager — main entry point
//
// KEY API
//   Initialize(atlasW, atlasH, maxAtlases)
//       Configure atlas dimensions and max count.
//
//   Allocate(width, height) → AtlasAllocation
//       Sub-allocate a rectangle from a shelf.  If the current atlas is
//       full, a new one is created (up to maxAtlases).
//
//   Free(alloc)
//       Marks the allocation as dead.  Space is reclaimed on Compact().
//
//   Compact()
//       Re-packs all live allocations into the minimum number of atlases,
//       eliminating fragmentation.
//
//   Upload(alloc, rgbaData)
//       Copies pixel data into the atlas backing store at the allocated
//       rectangle.
//
//   GetAtlasData(atlasId) → const uint8_t*
//       Returns pointer to the raw RGBA32 atlas buffer.
//
//   GetStats() → AtlasStats
//       Atlases in use, allocations, occupancy per atlas, wasted bytes.
//
// THREAD SAFETY
//   All public methods guarded by SRWLOCK.
//
// DEPENDENCIES
//   Windows API + standard library only.
// ============================================================================
#pragma once

#include <windows.h>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <cstdint>
#include <algorithm>
#include <string>
#include <cstring>

namespace ExplorerLens {
namespace Engine {

// -----------------------------------------------------------------------
// AtlasAllocation — handle returned by Allocate()
// -----------------------------------------------------------------------
struct AtlasAllocation {
    uint32_t atlasId = UINT32_MAX;
    uint32_t x       = 0;
    uint32_t y       = 0;
    uint32_t width   = 0;
    uint32_t height  = 0;

    bool IsValid() const { return atlasId != UINT32_MAX; }
};

// -----------------------------------------------------------------------
// AtlasStats
// -----------------------------------------------------------------------
struct AtlasStats {
    uint32_t atlasesInUse      = 0;
    uint32_t totalAllocations  = 0;
    uint32_t liveAllocations   = 0;
    std::vector<float> occupancyPerAtlas;   // 0.0 – 1.0 per atlas
    uint64_t wastedSpaceBytes  = 0;
    uint64_t totalSpaceBytes   = 0;
};

// -----------------------------------------------------------------------
// GPUTextureAtlasManager
// -----------------------------------------------------------------------
class GPUTextureAtlasManager {
public:
    GPUTextureAtlasManager()  = default;
    ~GPUTextureAtlasManager() = default;

    GPUTextureAtlasManager(const GPUTextureAtlasManager&)            = delete;
    GPUTextureAtlasManager& operator=(const GPUTextureAtlasManager&) = delete;

    // ================================================================
    // Initialize
    // ================================================================
    inline bool Initialize(uint32_t atlasWidth  = 4096,
                           uint32_t atlasHeight = 4096,
                           uint32_t maxAtlases  = 16) {
        AcquireExclusive();
        m_atlasW     = atlasWidth;
        m_atlasH     = atlasHeight;
        m_maxAtlases = maxAtlases;
        m_atlases.clear();
        m_allocations.clear();
        m_nextAllocId = 0;
        m_ready = true;
        ReleaseExclusive();
        return true;
    }

    // ================================================================
    // Allocate — shelf-packing algorithm
    // ================================================================
    inline AtlasAllocation Allocate(uint32_t width, uint32_t height) {
        AcquireExclusive();
        AtlasAllocation result{};

        if (!m_ready || width == 0 || height == 0 ||
            width > m_atlasW || height > m_atlasH) {
            ReleaseExclusive();
            return result;
        }

        // Try to fit in an existing atlas
        for (size_t ai = 0; ai < m_atlases.size(); ++ai) {
            if (TryAllocateInAtlas(static_cast<uint32_t>(ai), width, height, result)) {
                RecordAllocation(result);
                ReleaseExclusive();
                return result;
            }
        }

        // Create a new atlas if allowed
        if (m_atlases.size() < m_maxAtlases) {
            uint32_t newId = static_cast<uint32_t>(m_atlases.size());
            m_atlases.push_back(AtlasData{});
            auto& ad = m_atlases.back();
            ad.pixels.resize(static_cast<size_t>(m_atlasW) * m_atlasH * 4, 0);
            ad.shelves.push_back(Shelf{ 0, 0, 0 });

            if (TryAllocateInAtlas(newId, width, height, result)) {
                RecordAllocation(result);
                ReleaseExclusive();
                return result;
            }
        }

        ReleaseExclusive();
        return result;  // invalid
    }

    // ================================================================
    // Free — mark allocation as dead (lazy; reclaimed on Compact())
    // ================================================================
    inline void Free(const AtlasAllocation& alloc) {
        AcquireExclusive();
        uint64_t key = MakeKey(alloc);
        auto it = m_allocations.find(key);
        if (it != m_allocations.end()) {
            it->second.alive = false;
        }
        ReleaseExclusive();
    }

    // ================================================================
    // Compact — re-pack live allocations
    // ================================================================
    inline void Compact() {
        AcquireExclusive();

        // Gather live allocations with their pixel data
        struct LiveRect {
            uint32_t w, h;
            std::vector<uint8_t> pixels;
        };
        std::vector<LiveRect> live;

        for (auto& [key, rec] : m_allocations) {
            if (!rec.alive) continue;
            if (rec.alloc.atlasId >= m_atlases.size()) continue;

            LiveRect lr;
            lr.w = rec.alloc.width;
            lr.h = rec.alloc.height;
            lr.pixels.resize(static_cast<size_t>(lr.w) * lr.h * 4);

            const auto& src = m_atlases[rec.alloc.atlasId].pixels;
            for (uint32_t row = 0; row < lr.h; ++row) {
                size_t srcOff = (static_cast<size_t>(rec.alloc.y + row) * m_atlasW
                                 + rec.alloc.x) * 4;
                size_t dstOff = static_cast<size_t>(row) * lr.w * 4;
                std::memcpy(lr.pixels.data() + dstOff,
                            src.data() + srcOff,
                            static_cast<size_t>(lr.w) * 4);
            }
            live.push_back(std::move(lr));
        }

        // Sort by height descending for better shelf packing
        std::sort(live.begin(), live.end(),
                  [](const LiveRect& a, const LiveRect& b) { return a.h > b.h; });

        // Reset all atlases
        m_allocations.clear();
        for (auto& ad : m_atlases) {
            std::memset(ad.pixels.data(), 0, ad.pixels.size());
            ad.shelves.clear();
            ad.shelves.push_back(Shelf{ 0, 0, 0 });
            ad.usedPixels = 0;
        }

        // Re-insert live allocations
        for (auto& lr : live) {
            AtlasAllocation alloc = AllocateInternal(lr.w, lr.h);
            if (alloc.IsValid()) {
                UploadInternal(alloc, lr.pixels.data());
                RecordAllocation(alloc);
            }
        }

        // Trim empty trailing atlases
        while (!m_atlases.empty() && m_atlases.back().usedPixels == 0) {
            m_atlases.pop_back();
        }

        ReleaseExclusive();
    }

    // ================================================================
    // Upload — copy pixel data into atlas at the allocated rect
    // ================================================================
    inline void Upload(const AtlasAllocation& alloc, const uint8_t* rgbaData) {
        AcquireExclusive();
        UploadInternal(alloc, rgbaData);
        ReleaseExclusive();
    }

    // ================================================================
    // GetAtlasData
    // ================================================================
    inline const uint8_t* GetAtlasData(uint32_t atlasId) {
        AcquireShared();
        const uint8_t* ptr = nullptr;
        if (atlasId < m_atlases.size()) {
            ptr = m_atlases[atlasId].pixels.data();
        }
        ReleaseShared();
        return ptr;
    }

    // ================================================================
    // GetStats
    // ================================================================
    inline AtlasStats GetStats() {
        AcquireShared();
        AtlasStats s;
        s.atlasesInUse  = static_cast<uint32_t>(m_atlases.size());
        s.totalAllocations = static_cast<uint32_t>(m_allocations.size());

        uint64_t totalAtlasPixels = 0;
        uint64_t usedPixels       = 0;
        uint32_t liveCount        = 0;

        for (size_t ai = 0; ai < m_atlases.size(); ++ai) {
            uint64_t atlasTotal = static_cast<uint64_t>(m_atlasW) * m_atlasH;
            totalAtlasPixels += atlasTotal;
            uint64_t used = m_atlases[ai].usedPixels;
            usedPixels += used;
            float occ = (atlasTotal > 0)
                ? static_cast<float>(used) / static_cast<float>(atlasTotal)
                : 0.0f;
            s.occupancyPerAtlas.push_back(occ);
        }

        for (auto& [key, rec] : m_allocations) {
            if (rec.alive) liveCount++;
        }
        s.liveAllocations  = liveCount;
        s.totalSpaceBytes  = totalAtlasPixels * 4;
        s.wastedSpaceBytes = (totalAtlasPixels - usedPixels) * 4;

        ReleaseShared();
        return s;
    }

private:
    // ---- shelf structure ----
    struct Shelf {
        uint32_t y       = 0;   // top row of this shelf
        uint32_t height  = 0;   // tallest item in this shelf
        uint32_t cursorX = 0;   // next free X on this shelf
    };

    // ---- atlas backing store ----
    struct AtlasData {
        std::vector<uint8_t> pixels;
        std::vector<Shelf>   shelves;
        uint64_t             usedPixels = 0;
    };

    // ---- allocation record ----
    struct AllocRecord {
        AtlasAllocation alloc;
        bool            alive = true;
    };

    // ---- SRWLOCK ----
    SRWLOCK m_srw = SRWLOCK_INIT;
    inline void AcquireExclusive() { ::AcquireSRWLockExclusive(&m_srw); }
    inline void ReleaseExclusive() { ::ReleaseSRWLockExclusive(&m_srw); }
    inline void AcquireShared()    { ::AcquireSRWLockShared(&m_srw);    }
    inline void ReleaseShared()    { ::ReleaseSRWLockShared(&m_srw);    }

    // ---- state ----
    bool     m_ready       = false;
    uint32_t m_atlasW      = 4096;
    uint32_t m_atlasH      = 4096;
    uint32_t m_maxAtlases  = 16;
    uint64_t m_nextAllocId = 0;
    std::vector<AtlasData>                       m_atlases;
    std::unordered_map<uint64_t, AllocRecord>    m_allocations;

    // ---- key from allocation ----
    static inline uint64_t MakeKey(const AtlasAllocation& a) {
        return (static_cast<uint64_t>(a.atlasId) << 32)
             | (static_cast<uint64_t>(a.y) << 16)
             | static_cast<uint64_t>(a.x);
    }

    // ---- try to fit rect into shelf in atlas ----
    inline bool TryAllocateInAtlas(uint32_t atlasId, uint32_t w, uint32_t h,
                                   AtlasAllocation& out) {
        auto& ad = m_atlases[atlasId];

        // Try existing shelves
        for (auto& shelf : ad.shelves) {
            if (shelf.cursorX + w <= m_atlasW && shelf.y + (std::max)(shelf.height, h) <= m_atlasH) {
                // Check the item fits vertically in this shelf
                if (h <= (std::max)(shelf.height, h)) {
                    out.atlasId = atlasId;
                    out.x       = shelf.cursorX;
                    out.y       = shelf.y;
                    out.width   = w;
                    out.height  = h;

                    shelf.cursorX += w;
                    if (h > shelf.height) shelf.height = h;
                    ad.usedPixels += static_cast<uint64_t>(w) * h;
                    return true;
                }
            }
        }

        // New shelf
        uint32_t newY = 0;
        if (!ad.shelves.empty()) {
            auto& last = ad.shelves.back();
            newY = last.y + last.height;
        }
        if (newY + h > m_atlasH || w > m_atlasW) return false;

        Shelf ns;
        ns.y       = newY;
        ns.height  = h;
        ns.cursorX = w;
        ad.shelves.push_back(ns);

        out.atlasId = atlasId;
        out.x       = 0;
        out.y       = newY;
        out.width   = w;
        out.height  = h;
        ad.usedPixels += static_cast<uint64_t>(w) * h;
        return true;
    }

    // ---- internal allocate (no lock) ----
    inline AtlasAllocation AllocateInternal(uint32_t w, uint32_t h) {
        AtlasAllocation result{};
        for (size_t ai = 0; ai < m_atlases.size(); ++ai) {
            if (TryAllocateInAtlas(static_cast<uint32_t>(ai), w, h, result))
                return result;
        }
        if (m_atlases.size() < m_maxAtlases) {
            uint32_t newId = static_cast<uint32_t>(m_atlases.size());
            m_atlases.push_back(AtlasData{});
            auto& ad = m_atlases.back();
            ad.pixels.resize(static_cast<size_t>(m_atlasW) * m_atlasH * 4, 0);
            ad.shelves.push_back(Shelf{ 0, 0, 0 });
            if (TryAllocateInAtlas(newId, w, h, result))
                return result;
        }
        return result;
    }

    // ---- record allocation ----
    inline void RecordAllocation(const AtlasAllocation& alloc) {
        uint64_t key = MakeKey(alloc);
        m_allocations[key] = AllocRecord{ alloc, true };
    }

    // ---- upload pixels (no lock) ----
    inline void UploadInternal(const AtlasAllocation& alloc, const uint8_t* rgbaData) {
        if (!alloc.IsValid() || !rgbaData) return;
        if (alloc.atlasId >= m_atlases.size()) return;

        auto& dst = m_atlases[alloc.atlasId].pixels;
        for (uint32_t row = 0; row < alloc.height; ++row) {
            size_t dstOff = (static_cast<size_t>(alloc.y + row) * m_atlasW + alloc.x) * 4;
            size_t srcOff = static_cast<size_t>(row) * alloc.width * 4;
            std::memcpy(dst.data() + dstOff, rgbaData + srcOff,
                        static_cast<size_t>(alloc.width) * 4);
        }
    }
};

} // namespace Engine
} // namespace ExplorerLens
