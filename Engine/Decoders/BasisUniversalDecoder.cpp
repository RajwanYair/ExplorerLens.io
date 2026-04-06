// BasisUniversalDecoder.cpp — Basis Universal / KTX2 GPU Texture Decoder
// Copyright (c) 2026 ExplorerLens Project

#include "BasisUniversalDecoder.h"
#include <cwchar>

namespace ExplorerLens {
namespace Engine {

const wchar_t* BasisUniversalDecoder::m_extensions[] = {L".basis", L".ktx2"};
const uint32_t BasisUniversalDecoder::m_extensionCount = 2;

BasisUniversalDecoder::BasisUniversalDecoder() {}
BasisUniversalDecoder::~BasisUniversalDecoder() {}

bool BasisUniversalDecoder::CanDecode(const wchar_t* filePath)
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

HRESULT BasisUniversalDecoder::Decode(const ThumbnailRequest& request, ThumbnailResult& result)
{
    result.hBitmap = nullptr;
    result.width   = 0;
    result.height  = 0;
    if (!request.filePath)
        return E_INVALIDARG;
    // Production: transcode Basis/KTX2 → RGBA via basisu transcoder, then render to HBITMAP.
    return E_NOTIMPL;
}

DecoderInfo BasisUniversalDecoder::GetInfo() const
{
    DecoderInfo info;
    info.name                = L"BasisUniversalDecoder";
    info.version             = L"1.0.0";
    info.supportedExtensions = m_extensions;
    info.extensionCount      = m_extensionCount;
    info.supportsGPU         = true;
    info.isArchiveDecoder    = false;
    return info;
}

const wchar_t** BasisUniversalDecoder::GetSupportedExtensions() const
{
    return m_extensions;
}

}  // namespace Engine
}  // namespace ExplorerLens
