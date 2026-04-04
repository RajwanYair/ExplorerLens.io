// MultipageDocumentDecoder.h — Multi-page document thumbnail handler
// Copyright (c) 2026 ExplorerLens Project
//
// Handles multi-page documents (TIFF, PDF) by selecting the most
// representative page for thumbnail generation with page count overlay.
//
#pragma once
#include <cstdint>
#include <string>

namespace ExplorerLens {
namespace Engine {

struct MultipageDocumentDecoderConfig
{
    bool enabled = true;
    uint32_t defaultPage = 0;  // first page
    bool showPageCount = true;
    std::string label = "MultipageDocumentDecoder";
};

class MultipageDocumentDecoder
{
  public:
    bool Initialize()
    {
        if (m_initialized)
            return true;
        m_initialized = true;
        return true;
    }
    bool IsInitialized() const
    {
        return m_initialized;
    }
    MultipageDocumentDecoderConfig GetConfig() const
    {
        return m_config;
    }
    std::string GetName() const
    {
        return m_config.label;
    }

    uint32_t SelectPage(uint32_t totalPages) const
    {
        if (totalPages == 0)
            return 0;
        return (m_config.defaultPage < totalPages) ? m_config.defaultPage : 0;
    }

    bool IsMultipage(uint32_t pageCount) const
    {
        return pageCount > 1;
    }

    std::string FormatPageLabel(uint32_t current, uint32_t total) const
    {
        if (!m_config.showPageCount || total <= 1)
            return "";
        return std::to_string(current + 1) + "/" + std::to_string(total);
    }

  private:
    bool m_initialized = false;
    MultipageDocumentDecoderConfig m_config;
};

}  // namespace Engine
}  // namespace ExplorerLens
