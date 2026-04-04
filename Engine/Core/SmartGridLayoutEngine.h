// SmartGridLayoutEngine.h — Intelligent Grid Layout for Multi-Image Thumbnails
// Copyright (c) 2026 ExplorerLens Project
//
// Packs a set of variably-sized items into an optimal grid that fits a given
// canvas while preserving aspect ratios. Used for multi-image archive
// thumbnails and collage-style previews.
//
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <numeric>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// Input item dimensions
struct GridItem
{
    double originalWidth = 0.0;
    double originalHeight = 0.0;
};

// Computed layout rectangle for a single item
struct GridItemRect
{
    double x = 0.0;
    double y = 0.0;
    double width = 0.0;
    double height = 0.0;
};

// Layout result for the entire grid
struct GridLayout
{
    std::vector<GridItemRect> rects;
    uint32_t columns = 0;
    uint32_t rows = 0;
    double canvasW = 0.0;
    double canvasH = 0.0;
    bool valid = false;
};

class SmartGridLayoutEngine
{
  public:
    static constexpr uint32_t kMaxItems = 1024;
    static constexpr double kPadding = 2.0;

    // Add an item with its natural dimensions.
    bool AddItem(double width, double height) noexcept
    {
        if (m_items.size() >= kMaxItems)
            return false;
        if (width <= 0.0 || height <= 0.0)
            return false;
        m_items.push_back({width, height});
        m_layoutDirty = true;
        return true;
    }

    void Clear() noexcept
    {
        m_items.clear();
        m_layout = {};
        m_layoutDirty = true;
    }

    uint32_t GetItemCount() const noexcept
    {
        return static_cast<uint32_t>(m_items.size());
    }

    // Compute the optimal grid layout for the given canvas size.
    // Chooses the column count that maximises coverage while
    // keeping aspect-ratio distortion low.
    GridLayout CalculateLayout(double canvasW, double canvasH) noexcept
    {
        m_layout = {};
        m_layout.canvasW = canvasW;
        m_layout.canvasH = canvasH;

        const uint32_t n = GetItemCount();
        if (n == 0 || canvasW <= 0.0 || canvasH <= 0.0)
            return m_layout;

        // Try every feasible column count and pick the best
        double bestScore = -1.0;
        uint32_t bestCols = 1;

        const uint32_t maxCols =
            (std::min)(n, static_cast<uint32_t>(std::ceil(std::sqrt(static_cast<double>(n) * (canvasW / canvasH)))));

        for (uint32_t cols = 1; cols <= (std::max)(1u, maxCols + 1); ++cols) {
            const uint32_t rowsNeeded = (n + cols - 1) / cols;
            const double cellW = (canvasW - kPadding * (cols + 1)) / static_cast<double>(cols);
            const double cellH = (canvasH - kPadding * (rowsNeeded + 1)) / static_cast<double>(rowsNeeded);

            if (cellW <= 0.0 || cellH <= 0.0)
                continue;

            double coverageSum = 0.0;
            double arFidelitySum = 0.0;

            for (uint32_t i = 0; i < n; ++i) {
                const auto& item = m_items[i];
                const double ar = item.originalWidth / item.originalHeight;
                double fitW = cellW;
                double fitH = cellW / ar;
                if (fitH > cellH) {
                    fitH = cellH;
                    fitW = cellH * ar;
                }
                coverageSum += fitW * fitH;
                const double cellAR = cellW / cellH;
                arFidelitySum += 1.0 - std::abs(ar - cellAR) / (std::max)(ar, cellAR);
            }

            const double coverage = coverageSum / (canvasW * canvasH);
            const double arFidelity = arFidelitySum / static_cast<double>(n);
            const double score = 0.6 * coverage + 0.4 * arFidelity;

            if (score > bestScore) {
                bestScore = score;
                bestCols = cols;
            }
        }

        // Build final layout with bestCols
        const uint32_t cols = bestCols;
        const uint32_t rows = (n + cols - 1) / cols;
        const double cellW = (canvasW - kPadding * (cols + 1)) / static_cast<double>(cols);
        const double cellH = (canvasH - kPadding * (rows + 1)) / static_cast<double>(rows);

        m_layout.columns = cols;
        m_layout.rows = rows;
        m_layout.rects.resize(n);

        for (uint32_t i = 0; i < n; ++i) {
            const uint32_t col = i % cols;
            const uint32_t row = i / cols;

            const double cellX = kPadding + col * (cellW + kPadding);
            const double cellY = kPadding + row * (cellH + kPadding);

            const auto& item = m_items[i];
            const double ar = item.originalWidth / item.originalHeight;
            double fitW = cellW;
            double fitH = cellW / ar;
            if (fitH > cellH) {
                fitH = cellH;
                fitW = cellH * ar;
            }

            // Centre within the cell
            m_layout.rects[i].x = cellX + (cellW - fitW) * 0.5;
            m_layout.rects[i].y = cellY + (cellH - fitH) * 0.5;
            m_layout.rects[i].width = fitW;
            m_layout.rects[i].height = fitH;
        }

        m_layout.valid = true;
        m_layoutDirty = false;
        return m_layout;
    }

    // Get the computed rectangle for item at index. Must call
    // CalculateLayout() first.
    GridItemRect GetItemRect(uint32_t index) const noexcept
    {
        if (index < static_cast<uint32_t>(m_layout.rects.size()))
            return m_layout.rects[index];
        return {};
    }

    // Fraction of the canvas area covered by items [0, 1].
    double GetCoverage() const noexcept
    {
        if (!m_layout.valid || m_layout.canvasW <= 0.0 || m_layout.canvasH <= 0.0)
            return 0.0;

        double total = 0.0;
        for (const auto& r : m_layout.rects)
            total += r.width * r.height;

        return (std::min)(1.0, total / (m_layout.canvasW * m_layout.canvasH));
    }

    // Mean aspect-ratio fidelity across items [0, 1].
    // 1.0 = all items perfectly match their original aspect ratio.
    double GetAspectRatioFidelity() const noexcept
    {
        const uint32_t n = GetItemCount();
        if (n == 0 || !m_layout.valid)
            return 0.0;

        double sum = 0.0;
        for (uint32_t i = 0; i < n; ++i) {
            const auto& item = m_items[i];
            const auto& rect = m_layout.rects[i];
            if (rect.width <= 0.0 || rect.height <= 0.0 || item.originalHeight <= 0.0)
                continue;

            const double origAR = item.originalWidth / item.originalHeight;
            const double fitAR = rect.width / rect.height;
            sum += 1.0 - std::abs(origAR - fitAR) / (std::max)(origAR, fitAR);
        }
        return sum / static_cast<double>(n);
    }

  private:
    std::vector<GridItem> m_items;
    GridLayout m_layout;
    bool m_layoutDirty = true;
};

}  // namespace Engine
}  // namespace ExplorerLens
