// PresentationSlideStrip.h — Presentation Slide Strip Layout
// Copyright (c) 2026 ExplorerLens Project
//
// Computes horizontal slide strip layouts for presentation file
// thumbnails (.pptx, .odp, .key). Pure geometry — calculates
// slide rectangles, spacing, and visible counts.
//
#pragma once

#include <windows.h>
#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

// Rectangle describing a slide's position within the strip
struct SlideStripRect
{
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
};

// Result of a strip layout calculation
struct StripLayoutResult
{
    int visibleSlides = 0;
    int slideWidth = 0;
    int slideHeight = 0;
    int totalStripWidth = 0;
    int gapBetweenSlides = 0;
    bool truncated = false;  // true if some slides are not visible
};

class PresentationSlideStrip
{
  public:
    PresentationSlideStrip() = default;
    ~PresentationSlideStrip() = default;

    // Sets the total number of slides. Returns false if count <= 0 or > 10000.
    bool SetSlideCount(int count)
    {
        if (count <= 0 || count > 10000)
            return false;
        m_slideCount = count;
        m_layoutDirty = true;
        return true;
    }

    // Computes horizontal strip layout. maxSlides caps the visible count.
    // Returns the layout result. Fails (returns zero result) if invalid input.
    StripLayoutResult CalculateStripLayout(int canvasW, int canvasH, int maxSlides) const
    {
        StripLayoutResult result{};
        if (m_slideCount <= 0 || canvasW <= 0 || canvasH <= 0 || maxSlides <= 0)
            return result;

        const int visible = (std::min)(m_slideCount, maxSlides);
        result.visibleSlides = visible;
        result.truncated = (m_slideCount > maxSlides);

        // Slide height fills canvas vertically with margins
        const int marginY = (std::max)(2, canvasH / 20);
        result.slideHeight = (std::max)(1, canvasH - 2 * marginY);

        // Slide width from aspect ratio
        result.slideWidth =
            (std::max)(1, static_cast<int>(result.slideHeight * m_aspectW / static_cast<double>(m_aspectH)));

        // Gap between slides
        result.gapBetweenSlides = (std::max)(1, canvasW / 80);

        // Check if all slides fit; shrink if necessary
        int neededW = visible * result.slideWidth + (visible - 1) * result.gapBetweenSlides;
        if (neededW > canvasW && visible > 0) {
            // Shrink slide width proportionally
            const int totalGaps = (visible - 1) * result.gapBetweenSlides;
            result.slideWidth = (std::max)(1, (canvasW - totalGaps) / visible);
            result.slideHeight =
                (std::max)(1, static_cast<int>(result.slideWidth * m_aspectH / static_cast<double>(m_aspectW)));
            neededW = visible * result.slideWidth + (visible - 1) * result.gapBetweenSlides;
        }

        result.totalStripWidth = neededW;
        return result;
    }

    // Returns the bounding rect for slide at given index. Returns zero rect if invalid.
    SlideStripRect GetSlideRect(int index) const
    {
        if (index < 0 || index >= m_slideCount)
            return {};

        // Use a default canvas if not explicitly set
        auto layout = CalculateStripLayout(m_canvasW, m_canvasH, (std::min)(m_slideCount, m_maxVisibleSlides));
        if (layout.visibleSlides <= 0 || index >= layout.visibleSlides)
            return {};

        const int totalW = layout.totalStripWidth;
        const int startX = (m_canvasW - totalW) / 2;  // center horizontally
        const int startY = (m_canvasH - layout.slideHeight) / 2;

        SlideStripRect rect;
        rect.x = startX + index * (layout.slideWidth + layout.gapBetweenSlides);
        rect.y = startY;
        rect.width = layout.slideWidth;
        rect.height = layout.slideHeight;
        return rect;
    }

    // Returns how many slides are visible within the canvas.
    int GetVisibleSlideCount() const
    {
        auto layout = CalculateStripLayout(m_canvasW, m_canvasH, (std::min)(m_slideCount, m_maxVisibleSlides));
        return layout.visibleSlides;
    }

    // Returns the slide aspect ratio as width/height.
    double GetSlideAspectRatio() const
    {
        if (m_aspectH <= 0)
            return 0.0;
        return static_cast<double>(m_aspectW) / m_aspectH;
    }

    // Sets the slide aspect ratio (e.g., 16:9, 4:3). Returns false if invalid.
    bool SetSlideAspectRatio(int w, int h)
    {
        if (w <= 0 || h <= 0)
            return false;
        m_aspectW = w;
        m_aspectH = h;
        m_layoutDirty = true;
        return true;
    }

    // Sets the canvas dimensions. Returns false if invalid.
    bool SetCanvasSize(int w, int h)
    {
        if (w <= 0 || h <= 0)
            return false;
        m_canvasW = w;
        m_canvasH = h;
        m_layoutDirty = true;
        return true;
    }

    // Sets the maximum visible slides cap. Returns false if <= 0.
    bool SetMaxVisibleSlides(int max)
    {
        if (max <= 0)
            return false;
        m_maxVisibleSlides = max;
        return true;
    }

    // Returns total slide count.
    int GetSlideCount() const
    {
        return m_slideCount;
    }

  private:
    int m_slideCount = 0;
    int m_aspectW = 16;
    int m_aspectH = 9;
    int m_canvasW = 512;
    int m_canvasH = 128;
    int m_maxVisibleSlides = 8;
    bool m_layoutDirty = true;
};

}  // namespace Engine
}  // namespace ExplorerLens
