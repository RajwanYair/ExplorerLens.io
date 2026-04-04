// DocumentDecoder.h - Document Thumbnail Decoder
// ExplorerLens Engine v6.2.0+
// Copyright (c) 2026 ExplorerLens Project
//
// Supports: EPUB, MOBI, AZW/AZW3, FB2, DOCX, XLSX, PPTX, XPS, OXPS, DJVU
// Features:
// - Extracts embedded cover images from EPUB (ZIP with cover.*)
// - Extracts OLE thumbnail from Office documents
// - Shell fallback for system-registered document types
// - Placeholder generation with document type indicator

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include "../Core/IThumbnailDecoder.h"

namespace ExplorerLens {
namespace Engine {

// Document metadata structure
struct DocumentMetadata
{
    std::wstring title;
    std::wstring author;
    std::wstring subject;
    uint32_t pageCount = 0;
    bool hasCoverImage = false;
};

class DocumentDecoder : public IThumbnailDecoder
{
  public:
    DocumentDecoder();
    ~DocumentDecoder() override;

    // IThumbnailDecoder interface
    bool CanDecode(const wchar_t* filePath) override;
    HRESULT Decode(const ThumbnailRequest& request, ThumbnailResult& result) override;
    DecoderInfo GetInfo() const override;
    const wchar_t* GetName() const override
    {
        return L"DocumentDecoder";
    }
    const wchar_t** GetSupportedExtensions() const override;
    uint32_t GetExtensionCount() const override
    {
        return m_extensionCount;
    }
    bool SupportsGPU() const override
    {
        return false;
    }
    bool IsArchiveDecoder() const override
    {
        return false;
    }

    // Document metadata extraction
    bool GetDocumentMetadata(const wchar_t* filePath, DocumentMetadata& metadata);

  private:
    // Shell-based thumbnail extraction
    HRESULT ExtractThumbnailShell(const wchar_t* filePath, uint32_t width, uint32_t height, HBITMAP* phBitmap);

    // Format-specific cover extraction
    HRESULT ExtractEPUBCover(const wchar_t* filePath, uint32_t width, uint32_t height, HBITMAP* phBitmap);
    HRESULT ExtractMOBICover(const wchar_t* filePath, uint32_t width, uint32_t height, HBITMAP* phBitmap);

    // Placeholder generation
    HBITMAP CreateDocumentPlaceholder(uint32_t width, uint32_t height, const wchar_t* ext);

    // Helpers
    bool IsDocumentFormat(const wchar_t* path);
    bool IsEpubFormat(const wchar_t* path);
    bool IsMobiFormat(const wchar_t* path);

    static const wchar_t* m_extensions[];
    static const uint32_t m_extensionCount;
};

}  // namespace Engine
}  // namespace ExplorerLens
