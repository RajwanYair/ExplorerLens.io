// LasPointCloudDecoder.h — LAS / LAZ LiDAR Point Cloud Decoder
// Copyright (c) 2026 ExplorerLens Project
//
// Renders a top-down point density map thumbnail for LAS/LAZ aerial scan files.
// Supports LAS 1.0–1.4 and LAZ (LASzip compressed) via the PDAL pipeline.
//
#pragma once

#include <windows.h>
#include <cstdint>
#include "../Core/IThumbnailDecoder.h"

namespace ExplorerLens {
namespace Engine {

class LasPointCloudDecoder : public IThumbnailDecoder
{
  public:
    LasPointCloudDecoder();
    ~LasPointCloudDecoder() override;

    bool       CanDecode(const wchar_t* filePath) override;
    HRESULT    Decode(const ThumbnailRequest& request, ThumbnailResult& result) override;
    DecoderInfo GetInfo() const override;

    const wchar_t* GetName() const override { return L"LasPointCloudDecoder"; }
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
