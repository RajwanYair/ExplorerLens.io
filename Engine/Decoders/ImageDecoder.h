// ImageDecoder.h - WIC-based Image Decoder for Core Formats
// Copyright (c) 2025 ExplorerLens Project
//
// Supports: JPEG, PNG, BMP, GIF, TIFF using Windows Imaging Component
// Features:
// - Hardware-accelerated decoding via WIC
// - EXIF orientation support
// - Color profile preservation
// - Progressive JPEG support
// - Animated GIF first frame extraction
// - Multi-page TIFF first page extraction

#pragma once

#include "../Core/IThumbnailDecoder.h"
#include <wincodec.h>
#include <wrl/client.h>
#include <mutex>

namespace ExplorerLens {
namespace Engine {

class ImageDecoder : public IThumbnailDecoder {
public:
    ImageDecoder();
    ~ImageDecoder() override = default;

    // IThumbnailDecoder interface
    bool CanDecode(const wchar_t* filePath) override;
    HRESULT Decode(const ThumbnailRequest& request, ThumbnailResult& result) override;
    DecoderInfo GetInfo() const override;
    const wchar_t* GetName() const override { return L"ImageDecoder"; }
    const wchar_t** GetSupportedExtensions() const override { return m_extensions; }
    uint32_t GetExtensionCount() const override { return m_extensionCount; }
    bool SupportsGPU() const override { return true; } // WIC is GPU-accelerated
    bool IsArchiveDecoder() const override { return false; }

private:
    // WIC factory (thread-safe singleton)
    static Microsoft::WRL::ComPtr<IWICImagingFactory> GetWICFactory();
    
    // Decode helpers
    HRESULT DecodeFromFile(const wchar_t* path, UINT targetWidth, 
                          UINT targetHeight, HBITMAP* phBitmap);
    HRESULT DecodeFromStream(IStream* pStream, UINT targetWidth,
                            UINT targetHeight, HBITMAP* phBitmap);
    
    // Convert WIC bitmap to HBITMAP
    HRESULT CreateHBITMAPFromWIC(IWICBitmapSource* pSource, HBITMAP* phBitmap);
    
    // Apply EXIF orientation
    HRESULT ApplyEXIFOrientation(IWICBitmapFrameDecode* pFrame,
                                 IWICBitmapSource** ppCorrected);

    // Extension list (must be static for lifetime guarantee)
    static const wchar_t* m_extensions[];
    static const uint32_t m_extensionCount;
    
    static std::mutex s_factoryMutex;
    static Microsoft::WRL::ComPtr<IWICImagingFactory> s_wicFactory;
};

} // namespace Engine
} // namespace ExplorerLens

