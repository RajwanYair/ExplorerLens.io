// RAWDecoder.h - LibRaw Decoder for Camera RAW Formats
// Copyright (c) 2026 ExplorerLens Project
//
// Supports: Canon CR2/CR3, Nikon NEF, Sony ARW, Adobe DNG, and 20+ camera RAW formats
// Features:
// - Embedded thumbnail extraction (fast path < 10ms)
// - Full RAW decode with demosaicing (quality path < 200ms)
// - EXIF orientation handling
// - White balance and exposure adjustment
// - Wide color gamut support (converts to sRGB)
// - Supports 700+ camera models
// - RAW metadata extraction (camera model, ISO, exposure, aperture)

#pragma once

#include <memory>
#include <mutex>
#include <string>
#include "../Core/IThumbnailDecoder.h"
#include "../Utils/EXIFOrientation.h"

namespace ExplorerLens {
namespace Engine {

// RAW image metadata structure
struct RAWMetadata
{
    std::wstring cameraModel;
    std::wstring cameraMake;
    uint32_t isoSpeed = 0;
    float shutterSpeed = 0.0f;  // in seconds
    float aperture = 0.0f;      // f-stop value
    float focalLength = 0.0f;   // in mm
    uint32_t imageWidth = 0;
    uint32_t imageHeight = 0;
    std::wstring timestamp;
    bool hasEmbeddedThumbnail = false;
};

class RAWDecoder : public IThumbnailDecoder
{
  public:
    RAWDecoder();
    ~RAWDecoder() override;

    // IThumbnailDecoder interface
    bool CanDecode(const wchar_t* filePath) override;
    HRESULT Decode(const ThumbnailRequest& request, ThumbnailResult& result) override;
    DecoderInfo GetInfo() const override;
    const wchar_t* GetName() const override
    {
        return L"RAWDecoder";
    }
    const wchar_t** GetSupportedExtensions() const override
    {
        return m_extensions;
    }
    uint32_t GetExtensionCount() const override
    {
        return m_extensionCount;
    }
    bool SupportsGPU() const override
    {
        return false;
    }  // CPU-based LibRaw
    bool IsArchiveDecoder() const override
    {
        return false;
    }

    // RAW metadata extraction
    bool GetRAWMetadata(const wchar_t* filePath, RAWMetadata& metadata);

  private:
    // Implementation details hidden in pImpl
    class Impl;
    std::unique_ptr<Impl> m_pImpl;

    // Decode strategies
    HRESULT DecodeFromFile(const wchar_t* path, UINT targetWidth, UINT targetHeight, HBITMAP* phBitmap);

    // Try to extract embedded thumbnail (fast)
    HRESULT ExtractEmbeddedThumbnail(const wchar_t* path, UINT targetWidth, UINT targetHeight, HBITMAP* phBitmap);

    // Full RAW decode with demosaicing (slow but high quality)
    HRESULT DecodeFullRAW(const wchar_t* path, UINT targetWidth, UINT targetHeight, HBITMAP* phBitmap);

    // Check if file is a RAW format
    bool IsRAWFormat(const wchar_t* path);

    // Convert RGB to Windows BGRA DIB section with proper orientation
    HBITMAP CreateBitmapFromRGB(const BYTE* rgb, int width, int height, int orientation);

    // Decode JPEG data from memory to HBITMAP
    HBITMAP DecodeJPEGToHBITMAP(const BYTE* jpegData, size_t dataSize);

    // Scale bitmap to fit within target dimensions (preserves aspect ratio)
    HBITMAP ScaleToTarget(HBITMAP hSource, UINT targetWidth, UINT targetHeight);

    // Extension list (must be static for lifetime guarantee)
    // Canon: cr2, cr3, crw
    // Nikon: nef, nrw
    // Sony: arw, srf, sr2
    // Olympus: orf
    // Panasonic: rw2, raw
    // Fujifilm: raf
    // Pentax: pef, ptx, dng
    // Adobe: dng
    // Leica: rwl, dng
    // Samsung: srw
    // Hasselblad: 3fr
    // Phase One: iiq
    // Sigma: x3f
    // Kodak: dcr, kdc
    // Minolta: mrw
    // Epson: erf
    static const wchar_t* m_extensions[];
    static const uint32_t m_extensionCount;
};

}  // namespace Engine
}  // namespace ExplorerLens
