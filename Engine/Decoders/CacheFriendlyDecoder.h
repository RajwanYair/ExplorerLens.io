// CacheFriendlyDecoder.h — Memory-Access-Optimized Decode Pipeline
// Copyright (c) 2026 ExplorerLens Project
//
// Optimizes decode memory access patterns for CPU cache efficiency using
// tile-based processing, prefetching hints, and cache-line alignment.
//
#pragma once

#include <cstdint>
#include <vector>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

struct TileDescriptor {
    uint32_t tileX = 0;
    uint32_t tileY = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t offsetBytes = 0;
    uint32_t sizeBytes = 0;
};

struct CacheFriendlyConfig {
    uint32_t tileSize = 64;        // 64x64 pixel tiles
    uint32_t cacheLineBytes = 64;  // CPU cache line size
    bool enablePrefetch = true;
    uint32_t prefetchAheadTiles = 2;
    uint32_t alignment = 64;       // Memory alignment for allocations
};

struct CacheFriendlyStats {
    uint64_t tilesProcessed = 0;
    uint64_t prefetchHints = 0;
    double avgTileProcessMs = 0.0;
    double estimatedCacheMissRate = 0.0;
};

class CacheFriendlyDecoder {
public:
    void Configure(const CacheFriendlyConfig& config) { m_config = config; }

    std::vector<TileDescriptor> GenerateTiles(uint32_t imageWidth, uint32_t imageHeight,
        uint32_t bytesPerPixel) const {
        std::vector<TileDescriptor> tiles;
        uint32_t ts = m_config.tileSize;
        uint32_t tilesX = (imageWidth + ts - 1) / ts;
        uint32_t tilesY = (imageHeight + ts - 1) / ts;
        tiles.reserve(tilesX * tilesY);

        for (uint32_t ty = 0; ty < tilesY; ++ty) {
            for (uint32_t tx = 0; tx < tilesX; ++tx) {
                TileDescriptor t;
                t.tileX = tx;
                t.tileY = ty;
                t.width = std::min(ts, imageWidth - tx * ts);
                t.height = std::min(ts, imageHeight - ty * ts);
                t.offsetBytes = (ty * ts * imageWidth + tx * ts) * bytesPerPixel;
                t.sizeBytes = t.width * t.height * bytesPerPixel;
                tiles.push_back(t);
            }
        }
        return tiles;
    }

    size_t AlignedSize(size_t bytes) const {
        return (bytes + m_config.alignment - 1) & ~static_cast<size_t>(m_config.alignment - 1);
    }

    uint32_t TileCount(uint32_t imageWidth, uint32_t imageHeight) const {
        uint32_t ts = m_config.tileSize;
        return ((imageWidth + ts - 1) / ts) * ((imageHeight + ts - 1) / ts);
    }

    bool ShouldTile(uint32_t imageWidth, uint32_t imageHeight) const {
        // Only tile if image is significantly larger than a single tile
        return imageWidth > m_config.tileSize * 2 && imageHeight > m_config.tileSize * 2;
    }

    CacheFriendlyStats GetStats() const { return m_stats; }

private:
    CacheFriendlyConfig m_config;
    CacheFriendlyStats m_stats;
};

} // namespace Engine
} // namespace ExplorerLens
