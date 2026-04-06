// JupyterNotebookDecoder.cpp — Jupyter Notebook (.ipynb) Preview Decoder
// Copyright (c) 2026 ExplorerLens Project

#include "JupyterNotebookDecoder.h"
#include <cwchar>

namespace ExplorerLens {
namespace Engine {

const wchar_t* JupyterNotebookDecoder::m_extensions[] = {L".ipynb"};
const uint32_t JupyterNotebookDecoder::m_extensionCount = 1;

JupyterNotebookDecoder::JupyterNotebookDecoder() {}
JupyterNotebookDecoder::~JupyterNotebookDecoder() {}

bool JupyterNotebookDecoder::CanDecode(const wchar_t* filePath)
{
    if (!filePath || filePath[0] == L'\0')
        return false;
    const wchar_t* ext = wcsrchr(filePath, L'.');
    if (!ext)
        return false;
    return _wcsicmp(ext, L".ipynb") == 0;
}

HRESULT JupyterNotebookDecoder::Decode(const ThumbnailRequest& request, ThumbnailResult& result)
{
    result.hBitmap = nullptr;
    result.width   = 0;
    result.height  = 0;
    if (!request.filePath)
        return E_INVALIDARG;
    // Production: parse .ipynb JSON, render first code cell + any embedded plot output.
    return E_NOTIMPL;
}

DecoderInfo JupyterNotebookDecoder::GetInfo() const
{
    DecoderInfo info;
    info.name                = L"JupyterNotebookDecoder";
    info.version             = L"1.0.0";
    info.supportedExtensions = m_extensions;
    info.extensionCount      = m_extensionCount;
    info.supportsGPU         = false;
    info.isArchiveDecoder    = false;
    return info;
}

const wchar_t** JupyterNotebookDecoder::GetSupportedExtensions() const
{
    return m_extensions;
}

}  // namespace Engine
}  // namespace ExplorerLens
