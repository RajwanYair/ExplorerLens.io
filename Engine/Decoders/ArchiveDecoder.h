// ArchiveDecoder.h
// Archive (ZIP/CBZ) thumbnail decoder for DarkThumbs Engine
// Extracts and decodes the first image file from ZIP archives using minizip-ng and WIC

#pragma once

#include "../Core/IThumbnailDecoder.h"
#include <string>
#include <vector>
#include <wincodec.h>

namespace DarkThumbs {
namespace Engine {

class ArchiveDecoder : public IThumbnailDecoder {
public:
    ArchiveDecoder();
    virtual ~ArchiveDecoder();

    // IThumbnailDecoder interface
    bool CanDecode(const wchar_t* filePath) override;
    HRESULT Decode(const ThumbnailRequest& request, ThumbnailResult& result) override;
    DecoderInfo GetInfo() const override;
    const wchar_t* GetName() const override { return L"ArchiveDecoder"; }
    const wchar_t** GetSupportedExtensions() const override { return m_extensions; }
    uint32_t GetExtensionCount() const override { return m_extensionCount; }
    bool SupportsGPU() const override { return false; } // Archive extraction is CPU-bound
    bool IsArchiveDecoder() const override { return true; }

    // Archive-specific functionality
    static bool IsArchiveFormat(const void* pData, size_t dataSize);
    
private:
    // ZIP signature detection
    static const unsigned char ZIP_SIGNATURE[4]; // PK\x03\x04

    // Helper methods
    HRESULT ExtractFirstImage(const wchar_t* archivePath, std::vector<unsigned char>& imageData, 
                          std::wstring& imageName);
    HRESULT DecodeImageData(const std::vector<unsigned char>& imageData, const std::wstring& imageName,
                        UINT targetWidth, UINT targetHeight, HBITMAP* phBitmap);
    
    bool IsImageFile(const std::wstring& filename);
    
    // WIC factory (shared with ImageDecoder)
    static IWICImagingFactory* GetWICFactory();
    
    // Extension list (must be static for lifetime guarantee)
    static const wchar_t* m_extensions[];
    static const uint32_t m_extensionCount;
};

} // namespace Engine
} // namespace DarkThumbs
