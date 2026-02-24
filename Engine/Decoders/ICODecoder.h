// ICODecoder.h - Windows Icon (.ico, .cur) Decoder
// ExplorerLens Engine v5.3.0+
// Copyright (c) 2026 ExplorerLens Project
//
// Supports: Windows Icons (.ico), Cursors (.cur)
// Features:
// - Multi-resolution icon extraction
// - Automatic best-fit resolution selection
// - Supports both classic and PNG-compressed icons
// - Alpha channel preservation
// - Windows Vista+ icon support (256x256)

#pragma once

#include "../Core/IThumbnailDecoder.h"
#include <windows.h>
#include <wincodec.h>
#include <wrl/client.h>
#include <mutex>

namespace ExplorerLens {
namespace Engine {

class ICODecoder : public IThumbnailDecoder {
public:
    ICODecoder();
    ~ICODecoder() override;

    // IThumbnailDecoder interface
    bool CanDecode(const wchar_t* filePath) override;
    HRESULT Decode(const ThumbnailRequest& request, ThumbnailResult& result) override;
    DecoderInfo GetInfo() const override;
    const wchar_t* GetName() const override { return L"ICODecoder"; }
    const wchar_t** GetSupportedExtensions() const override;
    uint32_t GetExtensionCount() const override { return m_extensionCount; }
    bool SupportsGPU() const override { return true; } // WIC can use GPU
    bool IsArchiveDecoder() const override { return false; }

private:
    // WIC factory singleton (shared across all instances)
    static Microsoft::WRL::ComPtr<IWICImagingFactory> GetWICFactory();
    static std::mutex s_factoryMutex;
    
    // Decode from file
    HRESULT DecodeFromFile(const wchar_t* path, UINT targetWidth, UINT targetHeight, 
                           HBITMAP* phBitmap);
    
    // Select best frame from multi-resolution ICO
    HRESULT SelectBestFrame(IWICBitmapDecoder* decoder, UINT targetSize, 
                           IWICBitmapFrameDecode** ppFrame);
    
    // Check if file is ICO format
    bool IsICOFormat(const wchar_t* path);
    
    // Extension list
    static const wchar_t* m_extensions[];
    static const uint32_t m_extensionCount;
};

} // namespace Engine
} // namespace ExplorerLens

