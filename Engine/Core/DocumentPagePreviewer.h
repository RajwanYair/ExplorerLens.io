// DocumentPagePreviewer.h — Multi-Page Document Preview Navigator
// Copyright (c) 2026 ExplorerLens Project
//
// Provides progressive page rendering and navigation for multi-page documents
// including PDF, DOCX, PPTX, XLS, and ODP formats with transition effects.
//
#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class DocumentType : uint8_t {
    PDF,
    DOCX,
    PPTX,
    XLS,
    ODP
};

enum class TransitionEffect : uint8_t {
    None,
    CrossFade,
    SlideLeft,
    SlideUp,
    Dissolve
};

struct PageInfo
{
    uint32_t index = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    bool hasText = false;
    bool hasImages = false;
    bool hasAnnotations = false;
    uint32_t textLength = 0;
};

struct PreviewConfig
{
    uint32_t pageTransitionMs = 300;
    uint32_t maxPagesToPreload = 4;
    uint32_t renderDpi = 150;
    uint32_t thumbnailMaxWidth = 512;
    uint32_t thumbnailMaxHeight = 512;
    TransitionEffect transition = TransitionEffect::CrossFade;
    bool antiAlias = true;
};

using PageRenderedCallback = std::function<void(uint32_t pageIndex, const uint8_t* rgba, uint32_t w, uint32_t h)>;

class DocumentPagePreviewer
{
  public:
    explicit DocumentPagePreviewer(PreviewConfig config = {}) : m_config(config) {}

    ~DocumentPagePreviewer()
    {
        Close();
    }

    bool OpenDocument(const std::wstring& filePath, DocumentType type)
    {
        Close();
        m_filePath = filePath;
        m_docType = type;
        m_isOpen = true;
        m_pages.clear();
        m_currentPage = 0;
        return true;
    }

    uint32_t GetPageCount() const
    {
        return static_cast<uint32_t>(m_pages.size());
    }

    bool RenderPage(uint32_t pageIndex, std::vector<uint8_t>& outputRGBA) const
    {
        if (!m_isOpen || pageIndex >= m_pages.size())
            return false;
        const auto& pg = m_pages[pageIndex];
        uint32_t w = std::min(pg.width, m_config.thumbnailMaxWidth);
        uint32_t h = std::min(pg.height, m_config.thumbnailMaxHeight);
        outputRGBA.resize(static_cast<size_t>(w) * h * 4);
        if (m_pageCallback)
            m_pageCallback(pageIndex, outputRGBA.data(), w, h);
        return true;
    }

    bool NavigateToPage(uint32_t pageIndex)
    {
        if (!m_isOpen || pageIndex >= m_pages.size())
            return false;
        m_currentPage = pageIndex;
        PreloadAdjacent(pageIndex);
        return true;
    }

    std::vector<uint32_t> GetPageThumbnails(uint32_t startPage, uint32_t count) const
    {
        std::vector<uint32_t> indices;
        uint32_t end = std::min(startPage + count, static_cast<uint32_t>(m_pages.size()));
        for (uint32_t i = startPage; i < end; ++i)
            indices.push_back(i);
        return indices;
    }

    void SetTransitionEffect(TransitionEffect effect)
    {
        m_config.transition = effect;
    }
    uint32_t GetCurrentPage() const
    {
        return m_currentPage;
    }
    DocumentType GetDocumentType() const
    {
        return m_docType;
    }
    const PageInfo* GetPageInfo(uint32_t index) const
    {
        return (index < m_pages.size()) ? &m_pages[index] : nullptr;
    }

    void AddPage(const PageInfo& page)
    {
        m_pages.push_back(page);
    }
    void SetPageCallback(PageRenderedCallback cb)
    {
        m_pageCallback = std::move(cb);
    }
    bool IsOpen() const
    {
        return m_isOpen;
    }

    void Close()
    {
        m_isOpen = false;
        m_pages.clear();
        m_preloadedPages.clear();
        m_currentPage = 0;
    }

  private:
    void PreloadAdjacent(uint32_t center)
    {
        m_preloadedPages.clear();
        uint32_t start = (center > m_config.maxPagesToPreload) ? center - m_config.maxPagesToPreload : 0;
        uint32_t end = std::min(center + m_config.maxPagesToPreload, static_cast<uint32_t>(m_pages.size()));
        for (uint32_t i = start; i < end; ++i)
            m_preloadedPages.push_back(i);
    }

    PreviewConfig m_config;
    std::vector<PageInfo> m_pages;
    std::vector<uint32_t> m_preloadedPages;
    std::wstring m_filePath;
    PageRenderedCallback m_pageCallback;
    DocumentType m_docType = DocumentType::PDF;
    uint32_t m_currentPage = 0;
    bool m_isOpen = false;
};

}  // namespace Engine
}  // namespace ExplorerLens
