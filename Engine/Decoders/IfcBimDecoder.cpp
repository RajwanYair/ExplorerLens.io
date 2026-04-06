// IfcBimDecoder.cpp — IFC Building Information Model Decoder
// Copyright (c) 2026 ExplorerLens Project

#include "IfcBimDecoder.h"
#include <cwchar>

namespace ExplorerLens {
namespace Engine {

const wchar_t* IfcBimDecoder::m_extensions[] = {L".ifc", L".ifczip"};
const uint32_t IfcBimDecoder::m_extensionCount = 2;

IfcBimDecoder::IfcBimDecoder() {}
IfcBimDecoder::~IfcBimDecoder() {}

bool IfcBimDecoder::CanDecode(const wchar_t* filePath)
{
    if (!filePath || filePath[0] == L'\0')
        return false;
    const wchar_t* ext = wcsrchr(filePath, L'.');
    if (!ext)
        return false;
    for (uint32_t i = 0; i < m_extensionCount; ++i) {
        if (_wcsicmp(ext, m_extensions[i]) == 0)
            return true;
    }
    return false;
}

HRESULT IfcBimDecoder::Decode(const ThumbnailRequest& request, ThumbnailResult& result)
{
    result.hBitmap = nullptr;
    result.width   = 0;
    result.height  = 0;
    if (!request.filePath)
        return E_INVALIDARG;
    // Production: parse IFC geometry via IfcOpenShell, extract floor plan, GPU rasterize.
    return E_NOTIMPL;
}

DecoderInfo IfcBimDecoder::GetInfo() const
{
    DecoderInfo info;
    info.name                = L"IfcBimDecoder";
    info.version             = L"1.0.0";
    info.supportedExtensions = m_extensions;
    info.extensionCount      = m_extensionCount;
    info.supportsGPU         = true;
    info.isArchiveDecoder    = false;
    return info;
}

const wchar_t** IfcBimDecoder::GetSupportedExtensions() const
{
    return m_extensions;
}

}  // namespace Engine
}  // namespace ExplorerLens
