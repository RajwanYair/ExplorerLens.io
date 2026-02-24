//==============================================================================
// EPUBDecoder.h
// EPUB (Electronic Publication) thumbnail decoder
// Extracts cover image from EPUB container
// Date: February 17, 2026
//==============================================================================

#pragma once

#include "../Core/IThumbnailDecoder.h"
#include <vector>
#include <string>
#include <windows.h>

namespace ExplorerLens {
namespace Engine {

/**
 * @brief EPUB (.epub) eBook thumbnail decoder
 * 
 * EPUB files are ZIP archives containing:
 * - META-INF/container.xml (points to content.opf)
 * - OEBPS/content.opf (OPF package document)
 * - Images/cover.jpg (cover image)
 * 
 * Strategy:
 * 1. Open EPUB as ZIP archive
 * 2. Parse container.xml to find OPF location
 * 3. Parse OPF to find cover image reference
 * 4. Extract cover image from archive
 * 5. Decode cover image with WIC
 * 
 * Fallback: Extract first image file from archive
 */
class EPUBDecoder : public IThumbnailDecoder {
public:
    EPUBDecoder();
    virtual ~EPUBDecoder();

    // IThumbnailDecoder interface
    bool CanDecode(const wchar_t* filePath) override;
    HRESULT Decode(const ThumbnailRequest& request, ThumbnailResult& result) override;
    DecoderInfo GetInfo() const override;
    const wchar_t* GetName() const override { return L"EPUBDecoder"; }
    const wchar_t** GetSupportedExtensions() const override { return m_extensions; }
    uint32_t GetExtensionCount() const override { return m_extensionCount; }
    bool SupportsGPU() const override { return false; }

private:
    // EPUB structure constants
    static constexpr const char* CONTAINER_XML_PATH = "META-INF/container.xml";
    static constexpr const char* OPF_NAMESPACE = "urn:oasis:names:tc:opendocument:xmlns:container";

    // Cover detection strategies
    struct CoverInfo {
        std::wstring path;
        std::wstring mimeType;
        bool foundInMetadata = false;
    };

    // Helper methods
    HRESULT ExtractCoverImage(const wchar_t* epubPath, std::vector<uint8_t>& imageData);
    bool FindOPFPath(void* zipHandle, std::wstring& opfPath);
    bool FindCoverInOPF(void* zipHandle, const std::wstring& opfPath, CoverInfo& cover);
    bool ExtractFileFromZip(void* zipHandle, const std::wstring& filePath, std::vector<uint8_t>& data);
    HRESULT DecodeImageData(const std::vector<uint8_t>& imageData, 
                            UINT targetWidth, UINT targetHeight, HBITMAP* phBitmap);

    // XML parsing helpers
    bool ParseContainerXML(const std::string& xml, std::wstring& opfPath);
    bool ParseOPFForCover(const std::string& opf, CoverInfo& cover);
    std::string ExtractXMLAttribute(const std::string& xml, const std::string& tag, const std::string& attr);
    std::string ExtractXMLContent(const std::string& xml, const std::string& tag);

    // Cover detection heuristics
    bool IsLikelyCoverImage(const std::wstring& filename);
    int GetCoverPriority(const std::wstring& filename);

    // Extension list
    static const wchar_t* m_extensions[];
    static const uint32_t m_extensionCount;
};

} // namespace Engine
} // namespace ExplorerLens

