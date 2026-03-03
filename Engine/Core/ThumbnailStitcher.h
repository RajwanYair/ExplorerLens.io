// ThumbnailStitcher.h — Composite Thumbnail Layout Engine
// Copyright (c) 2026 ExplorerLens Project
//
// Computes layout geometry for stitching multiple thumbnail segments into
// panoramic or composite views. Pure layout/geometry calculations with no
// pixel manipulation — produces rectangles for upstream renderers.
//
#pragma once

#include <cstdint>
#include <vector>
#include <algorithm>

namespace ExplorerLens {
namespace Engine {

enum class StitchOrientation : uint8_t {
    Horizontal = 0,
    Vertical = 1,
    Grid = 2
};

struct StitchSegment {
    uint32_t width = 0;
    uint32_t height = 0;
};

struct StitchRect {
    int32_t  x = 0;
    int32_t  y = 0;
    uint32_t width = 0;
    uint32_t height = 0;
};

struct StitchedSize {
    uint32_t width = 0;
    uint32_t height = 0;
};

class ThumbnailStitcher {
public:
    ThumbnailStitcher() = default;

    // ---------------------------------------------------------------
    // Add a segment with the given dimensions
    // ---------------------------------------------------------------
    bool AddSegment(uint32_t width, uint32_t height) noexcept {
        if (width == 0 || height == 0) return false;
        m_segments.push_back({ width, height });
        return true;
    }

    // ---------------------------------------------------------------
    // Set layout orientation
    // ---------------------------------------------------------------
    void SetOrientation(StitchOrientation orientation) noexcept {
        m_orientation = orientation;
    }

    StitchOrientation GetOrientation() const noexcept {
        return m_orientation;
    }

    // ---------------------------------------------------------------
    // Number of segments
    // ---------------------------------------------------------------
    size_t GetSegmentCount() const noexcept {
        return m_segments.size();
    }

    // ---------------------------------------------------------------
    // Calculate the total stitched canvas size
    // ---------------------------------------------------------------
    StitchedSize CalculateStitchedSize() const noexcept {
        if (m_segments.empty()) return { 0, 0 };

        if (m_orientation == StitchOrientation::Horizontal) {
            uint32_t totalW = 0;
            uint32_t maxH = 0;
            for (const auto& seg : m_segments) {
                totalW += seg.width + m_spacing;
                maxH = (std::max)(maxH, seg.height);
            }
            // Remove trailing spacing
            if (!m_segments.empty() && totalW >= m_spacing) {
                totalW -= m_spacing;
            }
            return { totalW, maxH };
        }

        if (m_orientation == StitchOrientation::Vertical) {
            uint32_t maxW = 0;
            uint32_t totalH = 0;
            for (const auto& seg : m_segments) {
                maxW = (std::max)(maxW, seg.width);
                totalH += seg.height + m_spacing;
            }
            if (!m_segments.empty() && totalH >= m_spacing) {
                totalH -= m_spacing;
            }
            return { maxW, totalH };
        }

        // Grid: approximate square layout
        const uint32_t cols = ComputeGridColumns();
        if (cols == 0) return { 0, 0 };
        const uint32_t rows = (static_cast<uint32_t>(m_segments.size()) + cols - 1) / cols;

        uint32_t maxCellW = 0;
        uint32_t maxCellH = 0;
        for (const auto& seg : m_segments) {
            maxCellW = (std::max)(maxCellW, seg.width);
            maxCellH = (std::max)(maxCellH, seg.height);
        }

        const uint32_t totalW = cols * maxCellW + (cols > 0 ? (cols - 1) * m_spacing : 0);
        const uint32_t totalH = rows * maxCellH + (rows > 0 ? (rows - 1) * m_spacing : 0);
        return { totalW, totalH };
    }

    // ---------------------------------------------------------------
    // Get the placement rectangle for a specific segment
    // ---------------------------------------------------------------
    StitchRect GetSegmentRect(size_t index) const noexcept {
        if (index >= m_segments.size()) return {};

        if (m_orientation == StitchOrientation::Horizontal) {
            int32_t xOffset = 0;
            for (size_t i = 0; i < index; ++i) {
                xOffset += static_cast<int32_t>(m_segments[i].width + m_spacing);
            }
            return { xOffset, 0, m_segments[index].width, m_segments[index].height };
        }

        if (m_orientation == StitchOrientation::Vertical) {
            int32_t yOffset = 0;
            for (size_t i = 0; i < index; ++i) {
                yOffset += static_cast<int32_t>(m_segments[i].height + m_spacing);
            }
            return { 0, yOffset, m_segments[index].width, m_segments[index].height };
        }

        // Grid
        const uint32_t cols = ComputeGridColumns();
        if (cols == 0) return {};

        uint32_t maxCellW = 0;
        uint32_t maxCellH = 0;
        for (const auto& seg : m_segments) {
            maxCellW = (std::max)(maxCellW, seg.width);
            maxCellH = (std::max)(maxCellH, seg.height);
        }

        const uint32_t col = static_cast<uint32_t>(index) % cols;
        const uint32_t row = static_cast<uint32_t>(index) / cols;
        const int32_t x = static_cast<int32_t>(col * (maxCellW + m_spacing));
        const int32_t y = static_cast<int32_t>(row * (maxCellH + m_spacing));

        return { x, y, m_segments[index].width, m_segments[index].height };
    }

    // ---------------------------------------------------------------
    // Total pixel area across all segments
    // ---------------------------------------------------------------
    uint64_t GetTotalArea() const noexcept {
        uint64_t area = 0;
        for (const auto& seg : m_segments) {
            area += static_cast<uint64_t>(seg.width) * seg.height;
        }
        return area;
    }

    // ---------------------------------------------------------------
    // Spacing between segments (pixels)
    // ---------------------------------------------------------------
    void SetSpacing(uint32_t pixels) noexcept { m_spacing = pixels; }
    uint32_t GetSpacing() const noexcept { return m_spacing; }

    // ---------------------------------------------------------------
    // Clear all segments
    // ---------------------------------------------------------------
    void Clear() noexcept {
        m_segments.clear();
    }

private:
    uint32_t ComputeGridColumns() const noexcept {
        const auto n = static_cast<uint32_t>(m_segments.size());
        if (n == 0) return 0;
        // Integer square root approximation for near-square grid
        uint32_t cols = 1;
        while (cols * cols < n) ++cols;
        return cols;
    }

    std::vector<StitchSegment> m_segments;
    StitchOrientation          m_orientation = StitchOrientation::Horizontal;
    uint32_t                   m_spacing = 0;
};

} // namespace Engine
} // namespace ExplorerLens
