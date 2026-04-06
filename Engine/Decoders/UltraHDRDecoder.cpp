// UltraHDRDecoder.cpp — Google Ultra HDR / Gainmap JPEG Decoder
// Copyright (c) 2026 ExplorerLens Project

#include "UltraHDRDecoder.h"
#include <cwchar>

namespace ExplorerLens {
namespace Engine {

const wchar_t* UltraHDRDecoder::m_extensions[] = {L".uhdr"};
const uint32_t UltraHDRDecoder::m_extensionCount = 1;

UltraHDRDecoder::UltraHDRDecoder() {}
UltraHDRDecoder::~UltraHDRDecoder() {}

bool UltraHDRDecoder::CanDecode(const wchar_t* filePath)
{
    if (!filePath || filePath[0] == L'\0')
        return false;
    const wchar_t* ext = wcsrchr(filePath, L'.');
    if (!ext)
        return false;
    return _wcsicmp(ext, L".uhdr") == 0;
}

HRESULT UltraHDRDecoder::Decode(const ThumbnailRequest& request, ThumbnailResult& result)
{
    result.hBitmap = nullptr;
    result.width   = 0;
    result.height  = 0;
    if (!request.filePath)
        return E_INVALIDARG;
    // Production: extract SDR base + gainmap via libultrahdr 1.3+, apply tone-mapping.
    return E_NOTIMPL;
}

DecoderInfo UltraHDRDecoder::GetInfo() const
{
    DecoderInfo info;
    info.name                = L"UltraHDRDecoder";
    info.version             = L"1.0.0";
    info.supportedExtensions = m_extensions;
    info.extensionCount      = m_extensionCount;
    info.supportsGPU         = true;
    info.isArchiveDecoder    = false;
    return info;
}

const wchar_t** UltraHDRDecoder::GetSupportedExtensions() const
{
    return m_extensions;
}

}  // namespace Engine
}  // namespace ExplorerLens
