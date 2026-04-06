// JupyterNotebookDecoder.h — Jupyter Notebook (.ipynb) Preview Decoder
// Copyright (c) 2026 ExplorerLens Project
//
// Renders a syntax-highlighted thumbnail showing the first cell and any inline
// plot/image output from a Jupyter notebook file (.ipynb).
//
#pragma once

#include <windows.h>
#include <cstdint>
#include "../Core/IThumbnailDecoder.h"

namespace ExplorerLens {
namespace Engine {

class JupyterNotebookDecoder : public IThumbnailDecoder
{
  public:
    JupyterNotebookDecoder();
    ~JupyterNotebookDecoder() override;

    bool       CanDecode(const wchar_t* filePath) override;
    HRESULT    Decode(const ThumbnailRequest& request, ThumbnailResult& result) override;
    DecoderInfo GetInfo() const override;

    const wchar_t* GetName() const override { return L"JupyterNotebookDecoder"; }
    const wchar_t** GetSupportedExtensions() const override;
    uint32_t   GetExtensionCount() const override { return m_extensionCount; }
    bool       SupportsGPU() const override { return false; }
    bool       IsArchiveDecoder() const override { return false; }

  private:
    static const wchar_t* m_extensions[];
    static const uint32_t m_extensionCount;
};

}  // namespace Engine
}  // namespace ExplorerLens
