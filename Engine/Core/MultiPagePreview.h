// MultiPagePreview.h — Multi-Page Document Preview Generator
// Copyright (c) 2026 ExplorerLens Project
//
// Generates composite thumbnails for multi-page documents (PDF, TIFF,
// DOCX, PPTX) showing a grid or fan of pages instead of just page 1.
//
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class PreviewLayout : uint8_t {
    SinglePage,       // Just page 1
    Grid2x2,          // 2x2 grid of pages 1-4
    FanStack,         // Overlapping fan of first 3 pages
    StripHorizontal,  // Horizontal strip of pages
    StripVertical,    // Vertical strip of pages
    COUNT
};

struct MultiPageInfo
{
    uint32_t pageIndex = 0;
    uint32_t widthPx = 0;
    uint32_t heightPx = 0;
    bool hasText = false;
    bool hasImages = false;
};

struct MultiPageResult
{
    PreviewLayout layout = PreviewLayout::SinglePage;
    uint32_t pagesUsed = 1;
    uint32_t totalPages = 1;
    uint32_t outputWidth = 256;
    uint32_t outputHeight = 256;
    bool composite = false;
};

class MultiPagePreview
{
  public:
    void SetLayout(PreviewLayout layout)
    {
        m_layout = layout;
    }
    PreviewLayout GetLayout() const
    {
        return m_layout;
    }

    void SetMaxPages(uint32_t n)
    {
        m_maxPages = n;
    }
    uint32_t GetMaxPages() const
    {
        return m_maxPages;
    }

    void AddPage(const MultiPageInfo& page)
    {
        m_pages.push_back(page);
    }
    void ClearPages()
    {
        m_pages.clear();
    }
    size_t PageCount() const
    {
        return m_pages.size();
    }

    MultiPageResult Generate(uint32_t thumbW, uint32_t thumbH) const
    {
        MultiPageResult r;
        r.totalPages = static_cast<uint32_t>(m_pages.size());
        r.outputWidth = thumbW;
        r.outputHeight = thumbH;
        r.layout = m_layout;
        r.pagesUsed = (r.totalPages < m_maxPages) ? r.totalPages : m_maxPages;
        r.composite = (r.pagesUsed > 1 && m_layout != PreviewLayout::SinglePage);
        return r;
    }

    static PreviewLayout SuggestLayout(uint32_t totalPages)
    {
        if (totalPages <= 1)
            return PreviewLayout::SinglePage;
        if (totalPages <= 3)
            return PreviewLayout::FanStack;
        return PreviewLayout::Grid2x2;
    }

    static const wchar_t* LayoutName(PreviewLayout l)
    {
        switch (l) {
            case PreviewLayout::SinglePage:
                return L"SinglePage";
            case PreviewLayout::Grid2x2:
                return L"Grid2x2";
            case PreviewLayout::FanStack:
                return L"FanStack";
            case PreviewLayout::StripHorizontal:
                return L"StripHorizontal";
            case PreviewLayout::StripVertical:
                return L"StripVertical";
            default:
                return L"Unknown";
        }
    }
    static size_t LayoutCount()
    {
        return static_cast<size_t>(PreviewLayout::COUNT);
    }

  private:
    PreviewLayout m_layout = PreviewLayout::SinglePage;
    uint32_t m_maxPages = 4;
    std::vector<MultiPageInfo> m_pages;
};

}  // namespace Engine
}  // namespace ExplorerLens
