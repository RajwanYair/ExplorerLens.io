// ============================================================================
// MultiPageStripRenderer.h — Multi-Page Document Strip Thumbnails
// ExplorerLens Engine v15.0.0
// Copyright (c) 2026 ExplorerLens Project
//
// Generates composite strip thumbnails for multi-page documents (PDF, TIFF,
// DJVU, Office docs). Shows up to 4 pages side-by-side with page numbers,
// giving users instant visual context about document content.
// ============================================================================

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <mutex>
#include <cmath>

namespace ExplorerLens {
namespace Engine {

// ============================================================================
// Strip layout modes
// ============================================================================

enum class StripLayout : uint8_t {
    HorizontalStrip = 0,  // Pages side by side ▪▪▪▪
    VerticalStack = 1,  // Pages stacked vertically
    Grid2x2 = 2,  // 2x2 grid of pages
    CoverPlusPeek = 3,  // Large first page + small subsequent
    FanSpread = 4   // Fan-like spread of pages
};

inline const char* StripLayoutToString(StripLayout layout) {
    static const char* names[] = {
        "HorizontalStrip", "VerticalStack", "Grid2x2",
        "CoverPlusPeek", "FanSpread"
    };
    return names[static_cast<uint8_t>(layout)];
}

// ============================================================================
// Page thumbnail descriptor
// ============================================================================

struct PageThumbnail {
    uint32_t pageNumber = 0;   // 1-based page number
    uint32_t width = 0;        // Rendered width
    uint32_t height = 0;       // Rendered height
    bool     hasContent = false;
    bool     isLandscape = false;
    double   aspectRatio = 1.0;
    std::wstring label;         // "1", "2", etc.

    void UpdateFromDimensions(uint32_t w, uint32_t h) {
        width = w;
        height = h;
        isLandscape = (w > h);
        aspectRatio = (h > 0) ? static_cast<double>(w) / h : 1.0;
    }
};

// ============================================================================
// Strip render configuration
// ============================================================================

struct StripRenderConfig {
    StripLayout layout = StripLayout::CoverPlusPeek;
    uint32_t maxPages = 4;             // Max pages to render
    uint32_t outputWidth = 256;        // Total strip width
    uint32_t outputHeight = 256;       // Total strip height
    uint32_t pageGap = 4;             // Pixel gap between pages
    uint32_t borderWidth = 1;          // Page border width
    uint32_t borderColor = 0xFFCCCCCC; // Light grey border
    uint32_t backgroundColor = 0xFFF0F0F0;  // Light background
    bool     showPageNumbers = true;
    bool     dropShadow = true;
    float    shadowOpacity = 0.3f;
    uint32_t pageNumberFontSize = 9;

    /// Auto-select layout based on page count and aspect ratio
    static StripLayout AutoSelectLayout(uint32_t pageCount, double avgAspectRatio) {
        if (pageCount == 1) return StripLayout::CoverPlusPeek;
        if (pageCount == 2) return StripLayout::HorizontalStrip;
        if (pageCount <= 4) return StripLayout::Grid2x2;
        return StripLayout::CoverPlusPeek;
    }
};

// ============================================================================
// Page placement in the final composite
// ============================================================================

struct PagePlacement {
    uint32_t pageIndex = 0;  // Index into pages array
    float    x = 0.0f;       // Placement X (pixels)
    float    y = 0.0f;       // Placement Y (pixels)
    float    width = 0.0f;   // Rendered width
    float    height = 0.0f;  // Rendered height
    float    rotation = 0.0f; // Degrees (for fan spread)
    float    scale = 1.0f;
    bool     showLabel = true;
};

// ============================================================================
// Strip composition result
// ============================================================================

struct StripComposition {
    uint32_t totalWidth = 0;
    uint32_t totalHeight = 0;
    std::vector<PagePlacement> placements;
    uint32_t pagesRendered = 0;
    uint32_t totalPages = 0;
    double   compositionTimeMs = 0.0;
    bool     truncated = false;  // More pages than maxPages

    std::wstring GetSummaryText() const {
        std::wstring text = std::to_wstring(totalPages);
        text += (totalPages == 1) ? L" page" : L" pages";
        if (truncated) {
            text += L" (showing " + std::to_wstring(pagesRendered) + L")";
        }
        return text;
    }
};

// ============================================================================
// MultiPageStripRenderer
// ============================================================================

class MultiPageStripRenderer {
public:
    static constexpr uint32_t MAX_PAGES = 20;
    static constexpr uint32_t DEFAULT_PAGE_COUNT = 4;
    static constexpr uint32_t MIN_PAGE_SIZE = 32;

    MultiPageStripRenderer() = default;
    ~MultiPageStripRenderer() = default;

    // Non-copyable
    MultiPageStripRenderer(const MultiPageStripRenderer&) = delete;
    MultiPageStripRenderer& operator=(const MultiPageStripRenderer&) = delete;

    // ========================================================================
    // Configuration
    // ========================================================================

    void SetConfig(const StripRenderConfig& config) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_config = config;
    }

    StripRenderConfig GetConfig() const { return m_config; }

    // ========================================================================
    // Page registration
    // ========================================================================

    /// Set the pages available for rendering
    void SetPages(const std::vector<PageThumbnail>& pages) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_pages = pages;
        if (m_pages.size() > MAX_PAGES) {
            m_pages.resize(MAX_PAGES);
        }
    }

    /// Add a single page
    void AddPage(const PageThumbnail& page) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_pages.size() < MAX_PAGES) {
            m_pages.push_back(page);
        }
    }

    uint32_t GetPageCount() const {
        return static_cast<uint32_t>(m_pages.size());
    }

    void ClearPages() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_pages.clear();
    }

    // ========================================================================
    // Layout computation
    // ========================================================================

    /// Compute the strip composition layout
    StripComposition ComputeComposition() const {
        std::lock_guard<std::mutex> lock(m_mutex);

        StripComposition result;
        result.totalPages = static_cast<uint32_t>(m_pages.size());

        uint32_t pagesToRender = (std::min)(m_config.maxPages,
            static_cast<uint32_t>(m_pages.size()));
        result.truncated = (m_pages.size() > m_config.maxPages);

        switch (m_config.layout) {
        case StripLayout::HorizontalStrip:
            ComputeHorizontalLayout(result, pagesToRender);
            break;
        case StripLayout::VerticalStack:
            ComputeVerticalLayout(result, pagesToRender);
            break;
        case StripLayout::Grid2x2:
            ComputeGridLayout(result, pagesToRender);
            break;
        case StripLayout::CoverPlusPeek:
            ComputeCoverPeekLayout(result, pagesToRender);
            break;
        case StripLayout::FanSpread:
            ComputeFanLayout(result, pagesToRender);
            break;
        }

        result.pagesRendered = pagesToRender;
        return result;
    }

    /// Recommend optimal layout based on document type and page count
    static StripLayout RecommendLayout(const std::wstring& formatType,
        uint32_t pageCount) {
        if (pageCount == 1) return StripLayout::CoverPlusPeek;
        if (formatType == L"PDF" || formatType == L"DJVU") {
            return (pageCount <= 4) ? StripLayout::Grid2x2 : StripLayout::CoverPlusPeek;
        }
        if (formatType == L"TIFF") {
            return StripLayout::HorizontalStrip;  // Multi-frame
        }
        return StripLayout::CoverPlusPeek;
    }

private:
    // ========================================================================
    // Layout algorithms
    // ========================================================================

    void ComputeHorizontalLayout(StripComposition& result, uint32_t count) const {
        float totalGaps = (count > 1) ? (count - 1) * static_cast<float>(m_config.pageGap) : 0;
        float availWidth = m_config.outputWidth - totalGaps;
        float pageWidth = availWidth / count;
        float pageHeight = static_cast<float>(m_config.outputHeight);

        result.totalWidth = m_config.outputWidth;
        result.totalHeight = m_config.outputHeight;

        for (uint32_t i = 0; i < count; i++) {
            PagePlacement p;
            p.pageIndex = i;
            p.x = i * (pageWidth + m_config.pageGap);
            p.y = 0;
            p.width = pageWidth;
            p.height = pageHeight;
            p.showLabel = m_config.showPageNumbers;
            result.placements.push_back(p);
        }
    }

    void ComputeVerticalLayout(StripComposition& result, uint32_t count) const {
        float totalGaps = (count > 1) ? (count - 1) * static_cast<float>(m_config.pageGap) : 0;
        float availHeight = m_config.outputHeight - totalGaps;
        float pageHeight = availHeight / count;
        float pageWidth = static_cast<float>(m_config.outputWidth);

        result.totalWidth = m_config.outputWidth;
        result.totalHeight = m_config.outputHeight;

        for (uint32_t i = 0; i < count; i++) {
            PagePlacement p;
            p.pageIndex = i;
            p.x = 0;
            p.y = i * (pageHeight + m_config.pageGap);
            p.width = pageWidth;
            p.height = pageHeight;
            p.showLabel = m_config.showPageNumbers;
            result.placements.push_back(p);
        }
    }

    void ComputeGridLayout(StripComposition& result, uint32_t count) const {
        uint32_t cols = (count <= 2) ? count : 2;
        uint32_t rows = (count + cols - 1) / cols;

        float cellW = (m_config.outputWidth - (cols - 1) * m_config.pageGap) / static_cast<float>(cols);
        float cellH = (m_config.outputHeight - (rows - 1) * m_config.pageGap) / static_cast<float>(rows);

        result.totalWidth = m_config.outputWidth;
        result.totalHeight = m_config.outputHeight;

        for (uint32_t i = 0; i < count; i++) {
            uint32_t col = i % cols;
            uint32_t row = i / cols;

            PagePlacement p;
            p.pageIndex = i;
            p.x = col * (cellW + m_config.pageGap);
            p.y = row * (cellH + m_config.pageGap);
            p.width = cellW;
            p.height = cellH;
            p.showLabel = m_config.showPageNumbers;
            result.placements.push_back(p);
        }
    }

    void ComputeCoverPeekLayout(StripComposition& result, uint32_t count) const {
        result.totalWidth = m_config.outputWidth;
        result.totalHeight = m_config.outputHeight;

        // Cover page takes 65% of width
        float coverWidth = m_config.outputWidth * 0.65f;
        float peekWidth = m_config.outputWidth - coverWidth - m_config.pageGap;

        // Cover
        PagePlacement cover;
        cover.pageIndex = 0;
        cover.x = 0;
        cover.y = 0;
        cover.width = coverWidth;
        cover.height = static_cast<float>(m_config.outputHeight);
        cover.showLabel = false;
        result.placements.push_back(cover);

        // Peek pages (stacked on the right)
        uint32_t peekCount = (std::min)(count - 1, static_cast<uint32_t>(3));
        if (peekCount > 0) {
            float peekHeight = (m_config.outputHeight - (peekCount - 1) * m_config.pageGap) /
                static_cast<float>(peekCount);

            for (uint32_t i = 0; i < peekCount; i++) {
                PagePlacement p;
                p.pageIndex = i + 1;
                p.x = coverWidth + m_config.pageGap;
                p.y = i * (peekHeight + m_config.pageGap);
                p.width = peekWidth;
                p.height = peekHeight;
                p.scale = 0.8f;
                p.showLabel = m_config.showPageNumbers;
                result.placements.push_back(p);
            }
        }
    }

    void ComputeFanLayout(StripComposition& result, uint32_t count) const {
        result.totalWidth = m_config.outputWidth;
        result.totalHeight = m_config.outputHeight;

        float centerX = m_config.outputWidth / 2.0f;
        float centerY = m_config.outputHeight * 0.6f;
        float pageSize = m_config.outputHeight * 0.7f;
        float angleStep = 15.0f;  // Degrees between pages
        float startAngle = -static_cast<float>(count - 1) * angleStep / 2.0f;

        for (uint32_t i = 0; i < count; i++) {
            PagePlacement p;
            p.pageIndex = i;
            p.rotation = startAngle + i * angleStep;
            p.width = pageSize * 0.7f;
            p.height = pageSize;
            p.x = centerX - p.width / 2;
            p.y = centerY - p.height;
            p.scale = 0.85f + (i == count / 2 ? 0.15f : 0.0f);
            p.showLabel = (i == count / 2);
            result.placements.push_back(p);
        }
    }

    // State
    std::vector<PageThumbnail> m_pages;
    StripRenderConfig m_config;
    mutable std::mutex m_mutex;
};

} // namespace Engine
} // namespace ExplorerLens
