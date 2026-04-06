// IfcBimDecoder.h — IFC Building Information Model Decoder
// Copyright (c) 2026 ExplorerLens Project
//
// Renders floor-plan thumbnail previews for IFC 2x3/IFC4 BIM files (.ifc, .ifczip).
// Uses IfcOpenShell geometry extraction + GPU rasterization pipeline.
//
#pragma once

#include <windows.h>
#include <cstdint>
#include "../Core/IThumbnailDecoder.h"

namespace ExplorerLens {
namespace Engine {

class IfcBimDecoder : public IThumbnailDecoder
{
  public:
    IfcBimDecoder();
    ~IfcBimDecoder() override;

    bool       CanDecode(const wchar_t* filePath) override;
    HRESULT    Decode(const ThumbnailRequest& request, ThumbnailResult& result) override;
    DecoderInfo GetInfo() const override;

    const wchar_t* GetName() const override { return L"IfcBimDecoder"; }
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
