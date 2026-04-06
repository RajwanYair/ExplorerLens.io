// LasPointCloudDecoder.cpp — LAS / LAZ LiDAR Point Cloud Decoder
// Copyright (c) 2026 ExplorerLens Project

#include "LasPointCloudDecoder.h"
#include <cwchar>

namespace ExplorerLens {
namespace Engine {

const wchar_t* LasPointCloudDecoder::m_extensions[] = {L".las", L".laz"};
const uint32_t LasPointCloudDecoder::m_extensionCount = 2;

LasPointCloudDecoder::LasPointCloudDecoder() {}
LasPointCloudDecoder::~LasPointCloudDecoder() {}

bool LasPointCloudDecoder::CanDecode(const wchar_t* filePath)
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

HRESULT LasPointCloudDecoder::Decode(const ThumbnailRequest& request, ThumbnailResult& result)
{
    result.hBitmap = nullptr;
    result.width   = 0;
    result.height  = 0;
    if (!request.filePath)
        return E_INVALIDARG;
    // Production: read LAS 1.x header, sample point cloud, render top-down density heatmap.
    return E_NOTIMPL;
}

DecoderInfo LasPointCloudDecoder::GetInfo() const
{
    DecoderInfo info;
    info.name                = L"LasPointCloudDecoder";
    info.version             = L"1.0.0";
    info.supportedExtensions = m_extensions;
    info.extensionCount      = m_extensionCount;
    info.supportsGPU         = true;
    info.isArchiveDecoder    = false;
    return info;
}

const wchar_t** LasPointCloudDecoder::GetSupportedExtensions() const
{
    return m_extensions;
}

}  // namespace Engine
}  // namespace ExplorerLens
