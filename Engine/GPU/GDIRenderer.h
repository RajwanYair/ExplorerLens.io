//==============================================================================
// GDIRenderer.h - CPU Fallback Renderer using GDI+
// Copyright (c) 2026 - ExplorerLens Project
//
// Purpose: Software-based thumbnail rendering when GPU is unavailable
// Features:
// - High-quality bicubic interpolation
// - No GPU required
// - Reliable fallback for all systems
// - Compatible with all Windows versions
//==============================================================================

#pragma once

#ifndef NOMINMAX
    #define NOMINMAX
#endif

#include <windows.h>
#include <objidl.h>  // COM types (IStream, MIDL_INTERFACE) required by GDI+
#include <gdiplus.h>
#include <mutex>
#include <string>
#include "../Core/IGPURenderer.h"

#pragma comment(lib, "gdiplus.lib")

namespace ExplorerLens {
namespace Engine {

/// CPU-based renderer using GDI+ as fallback when GPU unavailable
class GDIRenderer : public IGPURenderer
{
  public:
    GDIRenderer();
    ~GDIRenderer() override;

    // IGPURenderer interface
    HRESULT Initialize() override;
    void Shutdown() override;
    bool IsAvailable() const override
    {
        return m_initialized;
    }

    HRESULT RenderThumbnail(const uint8_t* imageData, uint32_t imageWidth, uint32_t imageHeight, uint32_t thumbWidth,
                            uint32_t thumbHeight, HBITMAP* outBitmap) override;

    HRESULT GetGPUInfo(wchar_t* outName, uint32_t nameSize, uint32_t* outMemoryMB) const override;

    const wchar_t* GetRendererType() const override
    {
        return L"GDI+ (CPU)";
    }

  private:
    // GDI+ initialization
    HRESULT InitializeGDIPlus();
    void ShutdownGDIPlus();

    // Rendering helpers
    HRESULT CreateBitmapFromRGBA(const uint8_t* imageData, uint32_t width, uint32_t height, Gdiplus::Bitmap** outBitmap);

    HRESULT ScaleBitmap(Gdiplus::Bitmap* source, uint32_t targetWidth, uint32_t targetHeight,
                        Gdiplus::Bitmap** outScaled);

    HRESULT BitmapToHBITMAP(Gdiplus::Bitmap* gdiplusBitmap, HBITMAP* outHBitmap);

    // GDI+ state
    ULONG_PTR m_gdiplusToken;
    bool m_initialized;

    // Statistics
    mutable std::mutex m_statsMutex;
    uint64_t m_totalOperations;
    double m_totalRenderTimeMs;
};

}  // namespace Engine
}  // namespace ExplorerLens
