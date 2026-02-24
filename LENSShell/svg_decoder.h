///////////////////////////////////////////////
// SVG Decoder using Windows Direct2D
// Sprint D Phase 2 - Vector Graphics Support
///////////////////////////////////////////////

#ifndef _SVG_DECODER_D2D_4F8B9A3C_1E5D_4A2B_9F7C_8D3E6A1B2C4D_
#define _SVG_DECODER_D2D_4F8B9A3C_1E5D_4A2B_9F7C_8D3E6A1B2C4D_

#include <windows.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <wincodec.h>
#include <atlbase.h>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "windowscodecs.lib")

namespace ExplorerLens {

class SVGDecoder
{
public:
    // Check if data is SVG format (simple heuristic)
    static bool IsSVGFormat(const BYTE* data, size_t size)
    {
        if (!data || size < 10) return false;
        
        // Check for SVG signatures
        // XML declaration or direct SVG tag
        if (size > 5 && memcmp(data, "<?xml", 5) == 0) return true;
        if (size > 4 && memcmp(data, "<svg", 4) == 0) return true;
        if (size > 5 && memcmp(data, "\xEF\xBB\xBF<?xml", 6) == 0) return true; // UTF-8 BOM + XML
        
        // Search first 256 bytes for <svg tag
        size_t searchLimit = min(size, (size_t)256);
        for (size_t i = 0; i < searchLimit - 4; i++)
        {
            if (data[i] == '<' && data[i+1] == 's' && data[i+2] == 'v' && data[i+3] == 'g')
                return true;
        }
        
        return false;
    }
    
    // Decode SVG to HBITMAP for thumbnail display
    static HRESULT DecodeToHBITMAP(
        const BYTE* svgData,
        size_t dataSize,
        HBITMAP* phBitmap,
        int maxWidth = 256,
        int maxHeight = 256)
    {
        if (!svgData || dataSize == 0 || !phBitmap) return E_INVALIDARG;
        
        *phBitmap = NULL;
        
        // Note: Full SVG rendering via Direct2D requires complex parsing
        // For now, use WIC to create a placeholder with SVG metadata
        // Future enhancement: Use third-party SVG library (e.g., NanoSVG, resvg)
        
        HRESULT hr = S_OK;
        
        // Create WIC factory
        CComPtr<IWICImagingFactory> pWICFactory;
        hr = CoCreateInstance(
            CLSID_WICImagingFactory,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&pWICFactory));
        
        if (FAILED(hr)) return hr;
        
        // For MVP: Create a simple placeholder bitmap with SVG icon
        // TODO Sprint D2.1: Integrate full SVG rendering library
        
        // Create WIC bitmap
        CComPtr<IWICBitmap> pWICBitmap;
        hr = pWICFactory->CreateBitmap(
            maxWidth,
            maxHeight,
            GUID_WICPixelFormat32bppPBGRA,
            WICBitmapCacheOnDemand,
            &pWICBitmap);
        
        if (FAILED(hr)) return hr;
        
        // Lock bitmap for writing
        WICRect rcLock = { 0, 0, maxWidth, maxHeight };
        CComPtr<IWICBitmapLock> pLock;
        hr = pWICBitmap->Lock(&rcLock, WICBitmapLockWrite, &pLock);
        if (FAILED(hr)) return hr;
        
        UINT cbBufferSize = 0;
        BYTE* pData = NULL;
        hr = pLock->GetDataPointer(&cbBufferSize, &pData);
        if (FAILED(hr)) return hr;
        
        // Fill with gradient background (placeholder)
        for (int y = 0; y < maxHeight; y++)
        {
            for (int x = 0; x < maxWidth; x++)
            {
                int offset = (y * maxWidth + x) * 4;
                BYTE gray = (BYTE)(((float)y / maxHeight) * 255);
                pData[offset + 0] = gray;     // B
                pData[offset + 1] = gray;     // G
                pData[offset + 2] = gray;     // R
                pData[offset + 3] = 255;      // A
            }
        }
        
        // Draw "SVG" text indicator (simple ASCII rendering)
        DrawSVGPlaceholder(pData, maxWidth, maxHeight);
        
        pLock.Release();
        
        // Convert WIC bitmap to HBITMAP
        hr = ConvertWICBitmapToHBITMAP(pWICFactory, pWICBitmap, phBitmap);
        
        return hr;
    }

private:
    // Simple text rendering for placeholder
    static void DrawSVGPlaceholder(BYTE* pData, int width, int height)
    {
        // Draw "SVG" text in center (simple 5x7 font)
        // This is a placeholder until full SVG rendering is implemented
        
        int centerX = width / 2;
        int centerY = height / 2;
        
        // Simple rectangle frame
        DWORD color = 0xFF4CAF50; // Material Green for SVG
        
        // Draw border
        for (int x = centerX - 60; x < centerX + 60; x++)
        {
            SetPixel(pData, width, height, x, centerY - 40, color);
            SetPixel(pData, width, height, x, centerY + 40, color);
        }
        for (int y = centerY - 40; y < centerY + 40; y++)
        {
            SetPixel(pData, width, height, centerX - 60, y, color);
            SetPixel(pData, width, height, centerX + 60, y, color);
        }
    }
    
    static void SetPixel(BYTE* pData, int width, int height, int x, int y, DWORD color)
    {
        if (x < 0 || x >= width || y < 0 || y >= height) return;
        
        int offset = (y * width + x) * 4;
        pData[offset + 0] = (color >> 0) & 0xFF;  // B
        pData[offset + 1] = (color >> 8) & 0xFF;  // G
        pData[offset + 2] = (color >> 16) & 0xFF; // R
        pData[offset + 3] = (color >> 24) & 0xFF; // A
    }
    
    // Convert WIC bitmap to GDI HBITMAP
    static HRESULT ConvertWICBitmapToHBITMAP(
        IWICImagingFactory* pFactory,
        IWICBitmap* pWICBitmap,
        HBITMAP* phBitmap)
    {
        if (!pFactory || !pWICBitmap || !phBitmap) return E_INVALIDARG;
        
        HRESULT hr = S_OK;
        
        UINT width = 0, height = 0;
        hr = pWICBitmap->GetSize(&width, &height);
        if (FAILED(hr)) return hr;
        
        // Create compatible DC and bitmap
        HDC hdcScreen = GetDC(NULL);
        HDC hdcMem = CreateCompatibleDC(hdcScreen);
        
        BITMAPINFO bmi = {};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = width;
        bmi.bmiHeader.biHeight = -(LONG)height; // Top-down
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;
        
        void* pBits = NULL;
        HBITMAP hBitmap = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
        
        if (hBitmap && pBits)
        {
            // Copy WIC bitmap data to DIB
            WICRect rcLock = { 0, 0, (INT)width, (INT)height };
            CComPtr<IWICBitmapLock> pLock;
            hr = pWICBitmap->Lock(&rcLock, WICBitmapLockRead, &pLock);
            
            if (SUCCEEDED(hr))
            {
                UINT cbBufferSize = 0;
                BYTE* pData = NULL;
                hr = pLock->GetDataPointer(&cbBufferSize, &pData);
                
                if (SUCCEEDED(hr))
                {
                    memcpy(pBits, pData, cbBufferSize);
                    *phBitmap = hBitmap;
                }
                else
                {
                    DeleteObject(hBitmap);
                }
            }
        }
        else
        {
            hr = E_FAIL;
        }
        
        DeleteDC(hdcMem);
        ReleaseDC(NULL, hdcScreen);
        
        return hr;
    }
};

} // namespace ExplorerLens

#endif // _SVG_DECODER_D2D_4F8B9A3C_1E5D_4A2B_9F7C_8D3E6A1B2C4D_

