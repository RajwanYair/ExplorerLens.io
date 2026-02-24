// AVIFDecoder.h - WIC-based AVIF/HEIF Decoder
// ExplorerLens Engine v1.0.0
// Copyright (c) 2025 ExplorerLens Project
//
// Supports: AVIF, HEIF using Windows Imaging Component
// Features:
// - Native WIC codec support (Windows 10 1809+)
// - Requires AV1 Video Extension from Microsoft Store
// - 10-bit and 12-bit color depth (auto-converts to 8-bit)
// - HDR images (tone-maps to SDR for thumbnails)
// - Alpha channel preservation
// - Multi-frame sequences (first frame only)

#pragma once

#include "../Core/IThumbnailDecoder.h"
#include <wincodec.h>
#include <wrl/client.h>
#include <mutex>

namespace ExplorerLens {
namespace Engine {

class AVIFDecoder : public IThumbnailDecoder {
public:
    AVIFDecoder();
    ~AVIFDecoder() override = default;

    // IThumbnailDecoder interface
    bool CanDecode(const wchar_t* filePath) override;
    HRESULT Decode(const ThumbnailRequest& request, ThumbnailResult& result) override;
    DecoderInfo GetInfo() const override;
    const wchar_t* GetName() const override { return L"AVIFDecoder"; }
    const wchar_t** GetSupportedExtensions() const override { return m_extensions; }
    uint32_t GetExtensionCount() const override { return m_extensionCount; }
    bool SupportsGPU() const override { return true; } // WIC can use GPU
    bool IsArchiveDecoder() const override { return false; }

private:
    // WIC factory (thread-safe singleton)
    static Microsoft::WRL::ComPtr<IWICImagingFactory> GetWICFactory();
    
    // Decode helpers
    HRESULT DecodeFromFile(const wchar_t* path, UINT targetWidth,
                          UINT targetHeight, HBITMAP* phBitmap);
    HRESULT DecodeFromMemory(const BYTE* data, size_t size, UINT targetWidth,
                            UINT targetHeight, HBITMAP* phBitmap);
    
    // Check if data is AVIF format (ftyp box with avif/avis brand)
    bool IsAVIFFormat(const BYTE* data, size_t size);
    
    // Extension list (must be static for lifetime guarantee)
    static const wchar_t* m_extensions[];
    static const uint32_t m_extensionCount;
    
    static std::mutex s_factoryMutex;
    static Microsoft::WRL::ComPtr<IWICImagingFactory> s_wicFactory;
};

} // namespace Engine
} // namespace ExplorerLens

