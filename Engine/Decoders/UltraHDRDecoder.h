// UltraHDRDecoder.h — Google Ultra HDR / Gainmap JPEG Decoder
// Copyright (c) 2026 ExplorerLens Project
//
// Decodes Ultra HDR JPEG files (.uhdr) that embed a gainmap alongside the SDR base image.
// Produces an SDR thumbnail with preserved local contrast via tone-mapping.
//
#pragma once

#include <windows.h>
#include <cstdint>
#include "../Core/IThumbnailDecoder.h"

namespace ExplorerLens {
namespace Engine {

class UltraHDRDecoder : public IThumbnailDecoder
{
  public:
    UltraHDRDecoder();
    ~UltraHDRDecoder() override;

    bool       CanDecode(const wchar_t* filePath) override;
    HRESULT    Decode(const ThumbnailRequest& request, ThumbnailResult& result) override;
    DecoderInfo GetInfo() const override;

    const wchar_t* GetName() const override { return L"UltraHDRDecoder"; }
    const wchar_t** GetSupportedExtensions() const override;
    uint32_t   GetExtensionCount() const override { return m_extensionCount; }
    bool       SupportsGPU() const override { return true; }
    bool       IsArchiveDecoder() const override { return false; }

  private:
    static const wchar_t* m_extensions[];
    static const uint32_t m_extensionCount;
};

}  // namespace Engine
}  // namespace ExplorerLens
