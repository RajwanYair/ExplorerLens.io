// PDFDecoder.h - PDF Document Thumbnail Decoder
// ExplorerLens Engine v6.2.0+
// Copyright (c) 2026 ExplorerLens Project
//
// Supports: PDF (.pdf)
// Features:
// - Uses Windows Shell IThumbnailProvider for PDF rendering
// - Falls back to IShellItemImageFactory
// - Generates placeholder with page count info when no renderer available
// - No external library dependency (relies on system PDF handlers like Edge/Acrobat)

#pragma once

#include "../Core/IThumbnailDecoder.h"
#include <cstdint>

namespace ExplorerLens {
namespace Engine {

class PDFDecoder : public IThumbnailDecoder {
public:
    PDFDecoder();
    ~PDFDecoder() override;

    // IThumbnailDecoder interface
    bool CanDecode(const wchar_t* filePath) override;
    HRESULT Decode(const ThumbnailRequest& request, ThumbnailResult& result) override;
    DecoderInfo GetInfo() const override;
    const wchar_t* GetName() const override { return L"PDFDecoder"; }
    const wchar_t** GetSupportedExtensions() const override;
    uint32_t GetExtensionCount() const override { return m_extensionCount; }
    bool SupportsGPU() const override { return false; }
    bool IsArchiveDecoder() const override { return false; }

private:
    // Shell-based thumbnail extraction
    HRESULT ExtractThumbnailShell(const wchar_t* filePath, uint32_t width,
                                   uint32_t height, HBITMAP* phBitmap);

    // Placeholder generation
    HBITMAP CreatePDFPlaceholder(uint32_t width, uint32_t height,
                                 const wchar_t* filePath);

    // Verify PDF signature (%PDF)
    bool IsPDFFormat(const wchar_t* path);

    static const wchar_t* m_extensions[];
    static const uint32_t m_extensionCount;
};

} // namespace Engine
} // namespace ExplorerLens

