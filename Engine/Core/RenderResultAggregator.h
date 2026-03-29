// RenderResultAggregator.h — Render Result Aggregator
// Copyright (c) 2026 ExplorerLens Project
//
// Combines partial tile renders from multiple cluster nodes into a single thumbnail.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

namespace ExplorerLens { namespace Engine {

struct RRATile {
    uint32_t             tileX  = 0;
    uint32_t             tileY  = 0;
    uint32_t             width  = 0;
    uint32_t             height = 0;
    std::vector<uint8_t> rgbaData;
};

struct RRAAggregateResult {
    bool                 success      = false;
    std::vector<uint8_t> rgbaData;
    uint32_t             width        = 0;
    uint32_t             height       = 0;
    uint32_t             tilesComposed = 0;
    std::string          errorMsg;
};

class RenderResultAggregator {
public:
    void AddTile(const RRATile& tile) {
        m_tiles[tile.tileX * 1000u + tile.tileY] = tile;
    }

    RRAAggregateResult Compose(uint32_t totalWidth, uint32_t totalHeight) {
        RRAAggregateResult r;
        if (m_tiles.empty()) { r.errorMsg = "No tiles"; return r; }
        r.width    = totalWidth;
        r.height   = totalHeight;
        r.rgbaData.assign(static_cast<size_t>(totalWidth) * totalHeight * 4, 0x80u);
        for (const auto& [key, tile] : m_tiles) {
            for (uint32_t row = 0; row < tile.height && (tile.tileY + row) < totalHeight; ++row) {
                for (uint32_t col = 0; col < tile.width && (tile.tileX + col) < totalWidth; ++col) {
                    size_t dst = ((tile.tileY + row) * totalWidth + (tile.tileX + col)) * 4;
                    size_t src = (static_cast<size_t>(row) * tile.width + col) * 4;
                    if ((dst + 3) < r.rgbaData.size() && (src + 3) < tile.rgbaData.size())
                        for (int c = 0; c < 4; ++c)
                            r.rgbaData[dst + c] = tile.rgbaData[src + c];
                }
            }
            ++r.tilesComposed;
        }
        r.success = true;
        return r;
    }

    uint32_t TileCount() const { return static_cast<uint32_t>(m_tiles.size()); }
    void Clear() { m_tiles.clear(); }

private:
    std::unordered_map<uint32_t, RRATile> m_tiles;
};

}} // namespace ExplorerLens::Engine
