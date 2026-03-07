// ThumbnailMergeEngine.h — Multi-Source Thumbnail Compositor
// Copyright (c) 2026 ExplorerLens Project
//
// Merges thumbnails from multiple sources (e.g., archive contents, multi-page
// documents) into a single composite thumbnail with layout strategies.
//
#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

enum class MergeLayout : uint8_t {
    Grid = 0,       // NxN grid
    Stacked = 1,    // Overlapping stack
    Strip = 2,      // Horizontal filmstrip
    Single = 3      // Best single thumbnail
};

struct MergeSource {
    uint32_t width = 0;
    uint32_t height = 0;
    const uint8_t* pixels = nullptr;
    float priority = 1.0f;
    std::string label;
};

struct MergeConfig {
    uint32_t outputWidth = 256;
    uint32_t outputHeight = 256;
    MergeLayout layout = MergeLayout::Grid;
    uint32_t maxSources = 4;
    uint32_t paddingPx = 2;
    uint32_t borderPx = 1;
    uint32_t bgColor = 0xFF2D2D30; // ARGB dark gray
};

struct MergeResult {
    bool success = false;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t sourcesUsed = 0;
    std::vector<uint8_t> pixels;
};

class ThumbnailMergeEngine {
public:
    void Configure(const MergeConfig& config) { m_config = config; }

    uint32_t GridDimension(uint32_t sourceCount) const {
        if (sourceCount <= 1) return 1;
        if (sourceCount <= 4) return 2;
        if (sourceCount <= 9) return 3;
        return 4;
    }

    uint32_t CellSize(uint32_t gridDim) const {
        if (gridDim == 0) return m_config.outputWidth;
        return (m_config.outputWidth - (gridDim + 1) * m_config.paddingPx) / gridDim;
    }

    uint32_t SelectSourceCount(uint32_t available) const {
        uint32_t maxGrid = GridDimension(m_config.maxSources);
        uint32_t maxSlots = maxGrid * maxGrid;
        return std::min(available, std::min(m_config.maxSources, maxSlots));
    }

    MergeResult CreateEmpty() const {
        MergeResult r;
        r.width = m_config.outputWidth;
        r.height = m_config.outputHeight;
        r.pixels.resize(static_cast<size_t>(r.width) * r.height * 4, 0);
        // Fill with background color
        for (size_t i = 0; i < r.pixels.size(); i += 4) {
            r.pixels[i + 0] = (m_config.bgColor >> 16) & 0xFF; // R
            r.pixels[i + 1] = (m_config.bgColor >> 8) & 0xFF;  // G
            r.pixels[i + 2] = m_config.bgColor & 0xFF;          // B
            r.pixels[i + 3] = (m_config.bgColor >> 24) & 0xFF;  // A
        }
        r.success = true;
        return r;
    }

private:
    MergeConfig m_config;
};

} // namespace Engine
} // namespace ExplorerLens
