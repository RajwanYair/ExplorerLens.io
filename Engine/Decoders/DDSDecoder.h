// DDSDecoder.h - DirectDraw Surface Texture Decoder
// ExplorerLens Engine v6.1.0+
// Copyright (c) 2026 ExplorerLens Project
//
// Supports: DirectDraw Surface (.dds) - game/3D textures
// Features:
// - DXT1/DXT3/DXT5 (BC1/BC2/BC3) decompression
// - Uncompressed RGBA/BGRA textures
// - DX10 extended header support
// - Mipmap level 0 extraction (largest resolution)
// - No external dependencies (WIC has DDS codec on Windows 10+)

#pragma once

#include <wincodec.h>
#include <windows.h>
#include <mutex>
#include "../Core/IThumbnailDecoder.h"
#include <wrl/client.h>

namespace ExplorerLens {
namespace Engine {

class DDSDecoder : public IThumbnailDecoder
{
  public:
    DDSDecoder();
    ~DDSDecoder() override;

    bool CanDecode(const wchar_t* filePath) override;
    HRESULT Decode(const ThumbnailRequest& request, ThumbnailResult& result) override;
    DecoderInfo GetInfo() const override;
    const wchar_t* GetName() const override
    {
        return L"DDSDecoder";
    }
    const wchar_t** GetSupportedExtensions() const override;
    uint32_t GetExtensionCount() const override
    {
        return m_extensionCount;
    }
    bool SupportsGPU() const override
    {
        return true;
    }  // WIC + D3D11
    bool IsArchiveDecoder() const override
    {
        return false;
    }

  private:
    static Microsoft::WRL::ComPtr<IWICImagingFactory> GetWICFactory();
    static std::mutex s_factoryMutex;

    HRESULT DecodeFromFile(const wchar_t* path, UINT targetWidth, UINT targetHeight, HBITMAP* phBitmap);
    bool IsDDSFormat(const wchar_t* path);

    static const wchar_t* m_extensions[];
    static const uint32_t m_extensionCount;
};

}  // namespace Engine
}  // namespace ExplorerLens
