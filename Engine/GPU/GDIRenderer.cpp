//==============================================================================
// GDIRenderer.cpp - CPU Fallback Renderer Implementation
//==============================================================================

#include "GDIRenderer.h"
#include <windows.h>  // Must be included before GDI+
#include <chrono>

using namespace Gdiplus;

namespace ExplorerLens {
namespace Engine {

GDIRenderer::GDIRenderer() : m_gdiplusToken(0), m_initialized(false), m_totalOperations(0), m_totalRenderTimeMs(0.0) {}

GDIRenderer::~GDIRenderer()
{
    Shutdown();
}

//==============================================================================
// Initialization
//==============================================================================

HRESULT GDIRenderer::Initialize()
{
    if (m_initialized) {
        return S_OK;  // Already initialized
    }

    return InitializeGDIPlus();
}

HRESULT GDIRenderer::InitializeGDIPlus()
{
    // Initialize GDI+
    GdiplusStartupInput gdiplusStartupInput;
    Status status = GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, nullptr);

    if (status != Ok) {
        return E_FAIL;
    }

    m_initialized = true;
    return S_OK;
}

void GDIRenderer::Shutdown()
{
    if (!m_initialized) {
        return;
    }

    ShutdownGDIPlus();
}

void GDIRenderer::ShutdownGDIPlus()
{
    if (m_gdiplusToken != 0) {
        GdiplusShutdown(m_gdiplusToken);
        m_gdiplusToken = 0;
    }

    m_initialized = false;
}

//==============================================================================
// Rendering
//==============================================================================

HRESULT GDIRenderer::RenderThumbnail(const uint8_t* imageData, uint32_t imageWidth, uint32_t imageHeight,
                                     uint32_t thumbWidth, uint32_t thumbHeight, HBITMAP* outBitmap)
{
    if (!m_initialized) {
        return E_FAIL;
    }

    if (!imageData || !outBitmap) {
        return E_POINTER;
    }

    if (imageWidth == 0 || imageHeight == 0 || thumbWidth == 0 || thumbHeight == 0) {
        return E_INVALIDARG;
    }

    auto startTime = std::chrono::high_resolution_clock::now();

    HRESULT hr = S_OK;
    Bitmap* sourceBitmap = nullptr;
    Bitmap* scaledBitmap = nullptr;

    // Step 1: Create GDI+ bitmap from RGBA data
    hr = CreateBitmapFromRGBA(imageData, imageWidth, imageHeight, &sourceBitmap);
    if (FAILED(hr)) {
        return hr;
    }

    // Step 2: Scale the bitmap
    hr = ScaleBitmap(sourceBitmap, thumbWidth, thumbHeight, &scaledBitmap);
    if (FAILED(hr)) {
        delete sourceBitmap;
        return hr;
    }

    // Step 3: Convert to HBITMAP
    hr = BitmapToHBITMAP(scaledBitmap, outBitmap);

    // Cleanup
    delete scaledBitmap;
    delete sourceBitmap;

    // Update statistics
    if (SUCCEEDED(hr)) {
        auto endTime = std::chrono::high_resolution_clock::now();
        double durationMs = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() / 1000.0;

        std::lock_guard<std::mutex> lock(m_statsMutex);
        m_totalOperations++;
        m_totalRenderTimeMs += durationMs;
    }

    return hr;
}

//==============================================================================
// Helper Methods
//==============================================================================

HRESULT GDIRenderer::CreateBitmapFromRGBA(const uint8_t* imageData, uint32_t width, uint32_t height, Bitmap** outBitmap)
{
    if (!imageData || !outBitmap) {
        return E_POINTER;
    }

    // Create GDI+ bitmap
    Bitmap* bitmap = new Bitmap(width, height, PixelFormat32bppARGB);
    if (!bitmap) {
        return E_OUTOFMEMORY;
    }

    // Lock bitmap for direct pixel access
    BitmapData bitmapData;
    Rect rect(0, 0, width, height);
    Status status = bitmap->LockBits(&rect, ImageLockModeWrite, PixelFormat32bppARGB, &bitmapData);

    if (status != Ok) {
        delete bitmap;
        return E_FAIL;
    }

    // Copy RGBA data to bitmap
    // Input is RGBA, GDI+ expects BGRA (same layout, different channel order)
    uint8_t* destRow = static_cast<uint8_t*>(bitmapData.Scan0);
    const uint8_t* srcRow = imageData;

    for (uint32_t y = 0; y < height; ++y) {
        uint8_t* dest = destRow;
        const uint8_t* src = srcRow;

        for (uint32_t x = 0; x < width; ++x) {
            // Convert RGBA → BGRA
            dest[0] = src[2];  // B
            dest[1] = src[1];  // G
            dest[2] = src[0];  // R
            dest[3] = src[3];  // A

            dest += 4;
            src += 4;
        }

        destRow += bitmapData.Stride;
        srcRow += width * 4;
    }

    bitmap->UnlockBits(&bitmapData);

    *outBitmap = bitmap;
    return S_OK;
}

HRESULT GDIRenderer::ScaleBitmap(Bitmap* source, uint32_t targetWidth, uint32_t targetHeight, Bitmap** outScaled)
{
    if (!source || !outScaled) {
        return E_POINTER;
    }

    // Create target bitmap
    Bitmap* target = new Bitmap(targetWidth, targetHeight, PixelFormat32bppARGB);
    if (!target) {
        return E_OUTOFMEMORY;
    }

    // Create graphics object for high-quality rendering
    Graphics graphics(target);

    // Set high-quality rendering modes
    graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);
    graphics.SetSmoothingMode(SmoothingModeHighQuality);
    graphics.SetPixelOffsetMode(PixelOffsetModeHighQuality);
    graphics.SetCompositingQuality(CompositingQualityHighQuality);

    // Draw scaled image
    Status status = graphics.DrawImage(source, 0, 0,              // Destination X, Y
                                       targetWidth, targetHeight  // Destination width, height
    );

    if (status != Ok) {
        delete target;
        return E_FAIL;
    }

    *outScaled = target;
    return S_OK;
}

HRESULT GDIRenderer::BitmapToHBITMAP(Bitmap* gdiplusBitmap, HBITMAP* outHBitmap)
{
    if (!gdiplusBitmap || !outHBitmap) {
        return E_POINTER;
    }

    // Get HBITMAP from GDI+ bitmap
    Color transparent(0, 0, 0, 0);
    HBITMAP hBitmap = nullptr;

    Status status = gdiplusBitmap->GetHBITMAP(transparent, &hBitmap);
    if (status != Ok || !hBitmap) {
        return E_FAIL;
    }

    *outHBitmap = hBitmap;
    return S_OK;
}

//==============================================================================
// GPU Info (N/A for CPU renderer)
//==============================================================================

HRESULT GDIRenderer::GetGPUInfo(wchar_t* outName, uint32_t nameSize, uint32_t* outMemoryMB) const
{
    if (outName && nameSize > 0) {
        wcscpy_s(outName, nameSize, L"GDI+ CPU Renderer");
    }

    if (outMemoryMB) {
        *outMemoryMB = 0;  // N/A for CPU renderer
    }

    return S_OK;
}

}  // namespace Engine
}  // namespace ExplorerLens
