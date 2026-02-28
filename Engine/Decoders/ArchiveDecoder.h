// ArchiveDecoder.h
// Archive (ZIP/CBZ/7Z/RAR/TAR) thumbnail decoder for ExplorerLens Engine
// Extracts and decodes the best cover image from various archive formats
// Primary support: ZIP/CBZ using minizip-ng
// Future support: 7Z, RAR, TAR, CAB, ISO (requires additional libraries)

#pragma once

#include "../Core/IThumbnailDecoder.h"
#include <string>
#include <vector>
#include <wincodec.h>

namespace ExplorerLens {
namespace Engine {

// Archive metadata structure
struct ArchiveMetadata {
 std::wstring format;
 uint32_t totalFiles = 0;
 uint32_t totalImages = 0;
 uint64_t uncompressedSize = 0;
 bool isEncrypted = false;
};

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

 // Archive metadata extraction
 bool GetArchiveMetadata(const wchar_t* filePath, ArchiveMetadata& metadata);

 // Archive-specific functionality
 static bool IsArchiveFormat(const void* pData, size_t dataSize);
 
private:
 // Format signatures
 static const unsigned char ZIP_SIGNATURE[4]; // PK\x03\x04
 static const unsigned char RAR_SIGNATURE[4]; // Rar!
 static const unsigned char SEVENZ_SIGNATURE[6]; // 7z\xBC\xAF\x27\x1C
 static const unsigned char TAR_SIGNATURE[5]; // ustar

 // Helper methods
 HRESULT ExtractFirstImage(const wchar_t* archivePath, std::vector<unsigned char>& imageData, 
 std::wstring& imageName);
 HRESULT ExtractBestCoverImage(const wchar_t* archivePath, std::vector<unsigned char>& imageData,
 std::wstring& imageName);
 HRESULT DecodeImageData(const std::vector<unsigned char>& imageData, const std::wstring& imageName,
 UINT targetWidth, UINT targetHeight, HBITMAP* phBitmap);
 
 bool IsImageFile(const std::wstring& filename);
 bool IsCoverImage(const std::wstring& filename);
 int GetImagePriority(const std::wstring& filename);
 
 // Format detection
 bool IsZipFormat(const wchar_t* filePath);
 bool IsRarFormat(const wchar_t* filePath);
 bool Is7zFormat(const wchar_t* filePath);
 bool IsTarFormat(const wchar_t* filePath);
 
 // WIC factory (shared with ImageDecoder)
 static IWICImagingFactory* GetWICFactory();
 
 // Extension list (must be static for lifetime guarantee)
 static const wchar_t* m_extensions[];
 static const uint32_t m_extensionCount;
};

} // namespace Engine
} // namespace ExplorerLens

