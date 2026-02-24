///////////////////////////////////////////////
// SVG Decoder using Windows Direct2D SVG
// Renders SVG to HBITMAP via ID2D1DeviceContext5
// Falls back to placeholder on pre-1703 Windows
///////////////////////////////////////////////

#ifndef _SVG_DECODER_D2D_4F8B9A3C_1E5D_4A2B_9F7C_8D3E6A1B2C4D_
#define _SVG_DECODER_D2D_4F8B9A3C_1E5D_4A2B_9F7C_8D3E6A1B2C4D_

#include <algorithm>
#include <atlbase.h>
#include <cstring>
#include <d2d1.h>
#include <d2d1_1.h>
#include <d2d1_3.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <wincodec.h>
#include <windows.h>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "windowscodecs.lib")

namespace ExplorerLens {

class SVGDecoder {
public:
  // Check if data is SVG format (simple heuristic)
  static bool IsSVGFormat(const BYTE *data, size_t size) {
    if (!data || size < 10)
      return false;

    // Check for SVG signatures
    if (size > 5 && memcmp(data, "<?xml", 5) == 0)
      return true;
    if (size > 4 && memcmp(data, "<svg", 4) == 0)
      return true;
    if (size > 5 && memcmp(data, "\xEF\xBB\xBF<?", 4) == 0)
      return true; // UTF-8 BOM

    // Search first 512 bytes for <svg tag
    size_t searchLimit = (std::min)(size, (size_t)512);
    for (size_t i = 0; i + 3 < searchLimit; i++) {
      if (data[i] == '<' && data[i + 1] == 's' && data[i + 2] == 'v' &&
          data[i + 3] == 'g')
        return true;
    }

    return false;
  }

  // Decode SVG to HBITMAP for thumbnail display
  static HRESULT DecodeToHBITMAP(const BYTE *svgData, size_t dataSize,
                                 HBITMAP *phBitmap, int maxWidth = 256,
                                 int maxHeight = 256) {
    if (!svgData || dataSize == 0 || !phBitmap)
      return E_INVALIDARG;
    *phBitmap = NULL;

    // Try Direct2D SVG rendering first (Windows 10 1703+)
    HRESULT hr =
        RenderSVGDirect2D(svgData, dataSize, phBitmap, maxWidth, maxHeight);
    if (SUCCEEDED(hr) && *phBitmap)
      return S_OK;

    // Fallback: create a styled SVG placeholder
    return CreateSVGPlaceholderBitmap(phBitmap, maxWidth, maxHeight);
  }

private:
  // Render SVG using Direct2D SVG support (Windows 10 Creators Update 1703+)
  static HRESULT RenderSVGDirect2D(const BYTE *svgData, size_t dataSize,
                                   HBITMAP *phBitmap, int width, int height) {
    HRESULT hr = S_OK;

    // Create D3D11 device (needed for D2D1 device context)
    CComPtr<ID3D11Device> pD3DDevice;
    D3D_FEATURE_LEVEL featureLevel;
    UINT createFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
                           createFlags, nullptr, 0, D3D11_SDK_VERSION,
                           &pD3DDevice, &featureLevel, nullptr);
    if (FAILED(hr)) {
      // Try WARP (software) fallback
      hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_WARP, nullptr,
                             createFlags, nullptr, 0, D3D11_SDK_VERSION,
                             &pD3DDevice, &featureLevel, nullptr);
      if (FAILED(hr))
        return hr;
    }

    // Get DXGI device
    CComPtr<IDXGIDevice> pDxgiDevice;
    hr = pD3DDevice->QueryInterface(&pDxgiDevice);
    if (FAILED(hr))
      return hr;

    // Create D2D1 factory
    CComPtr<ID2D1Factory3> pD2DFactory;
    D2D1_FACTORY_OPTIONS factoryOptions = {};
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,
                           __uuidof(ID2D1Factory3), &factoryOptions,
                           reinterpret_cast<void **>(&pD2DFactory));
    if (FAILED(hr))
      return hr;

    // Create D2D1 device
    CComPtr<ID2D1Device2> pD2DDevice;
    hr = pD2DFactory->CreateDevice(pDxgiDevice, &pD2DDevice);
    if (FAILED(hr))
      return hr;

    // Create device context
    CComPtr<ID2D1DeviceContext2> pDC2;
    hr = pD2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
                                         &pDC2);
    if (FAILED(hr))
      return hr;

    // QI for ID2D1DeviceContext5 (SVG support)
    CComPtr<ID2D1DeviceContext5> pDC5;
    hr = pDC2->QueryInterface(&pDC5);
    if (FAILED(hr))
      return hr; // Pre-1703 Windows, fall back

    // Create WIC bitmap as render target
    CComPtr<IWICImagingFactory> pWICFactory;
    hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr,
                          CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pWICFactory));
    if (FAILED(hr))
      return hr;

    CComPtr<IWICBitmap> pWICBitmap;
    hr = pWICFactory->CreateBitmap(width, height, GUID_WICPixelFormat32bppPBGRA,
                                   WICBitmapCacheOnDemand, &pWICBitmap);
    if (FAILED(hr))
      return hr;

    // Create image render target from WIC bitmap
    D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties(
        D2D1_RENDER_TARGET_TYPE_SOFTWARE,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM,
                          D2D1_ALPHA_MODE_PREMULTIPLIED),
        96.0f, 96.0f);

    CComPtr<ID2D1RenderTarget> pRT;
    hr = pD2DFactory->CreateWicBitmapRenderTarget(pWICBitmap, rtProps, &pRT);
    if (FAILED(hr))
      return hr;

    CComPtr<ID2D1DeviceContext5> pSvgDC;
    hr = pRT->QueryInterface(&pSvgDC);
    if (FAILED(hr))
      return hr;

    // Create IStream from SVG data
    CComPtr<IStream> pStream;
    pStream.Attach(SHCreateMemStream(svgData, static_cast<UINT>(dataSize)));
    if (!pStream)
      return E_OUTOFMEMORY;

    // Create SVG document
    CComPtr<ID2D1SvgDocument> pSvgDoc;
    D2D1_SIZE_F viewportSize =
        D2D1::SizeF(static_cast<float>(width), static_cast<float>(height));
    hr = pSvgDC->CreateSvgDocument(pStream, viewportSize, &pSvgDoc);
    if (FAILED(hr))
      return hr;

    // Render SVG
    pSvgDC->BeginDraw();
    pSvgDC->Clear(D2D1::ColorF(D2D1::ColorF::White));
    pSvgDC->DrawSvgDocument(pSvgDoc);
    hr = pSvgDC->EndDraw();
    if (FAILED(hr))
      return hr;

    // Convert WIC bitmap to HBITMAP
    hr = ConvertWICBitmapToHBITMAP(pWICFactory, pWICBitmap, phBitmap);
    return hr;
  }

  // Fallback placeholder for systems without D2D SVG support
  static HRESULT CreateSVGPlaceholderBitmap(HBITMAP *phBitmap, int width,
                                            int height) {
    CComPtr<IWICImagingFactory> pWICFactory;
    HRESULT hr =
        CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                         IID_PPV_ARGS(&pWICFactory));
    if (FAILED(hr))
      return hr;

    CComPtr<IWICBitmap> pWICBitmap;
    hr = pWICFactory->CreateBitmap(width, height, GUID_WICPixelFormat32bppPBGRA,
                                   WICBitmapCacheOnDemand, &pWICBitmap);
    if (FAILED(hr))
      return hr;

    WICRect rcLock = {0, 0, width, height};
    CComPtr<IWICBitmapLock> pLock;
    hr = pWICBitmap->Lock(&rcLock, WICBitmapLockWrite, &pLock);
    if (FAILED(hr))
      return hr;

    UINT cbBufferSize = 0;
    BYTE *pData = nullptr;
    hr = pLock->GetDataPointer(&cbBufferSize, &pData);
    if (FAILED(hr))
      return hr;

    // Light gray background with SVG icon border
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        int offset = (y * width + x) * 4;
        pData[offset + 0] = 240; // B
        pData[offset + 1] = 240; // G
        pData[offset + 2] = 240; // R
        pData[offset + 3] = 255; // A
      }
    }
    DrawSVGPlaceholder(pData, width, height);
    pLock.Release();

    hr = ConvertWICBitmapToHBITMAP(pWICFactory, pWICBitmap, phBitmap);
    return hr;
  }

  // Simple text rendering for placeholder
  static void DrawSVGPlaceholder(BYTE *pData, int width, int height) {
    int centerX = width / 2;
    int centerY = height / 2;
    DWORD color = 0xFF4CAF50; // Material Green

    int bx = (std::min)(60, width / 4);
    int by = (std::min)(40, height / 4);
    for (int x = centerX - bx; x < centerX + bx; x++) {
      SetPixel(pData, width, height, x, centerY - by, color);
      SetPixel(pData, width, height, x, centerY + by, color);
    }
    for (int y = centerY - by; y < centerY + by; y++) {
      SetPixel(pData, width, height, centerX - bx, y, color);
      SetPixel(pData, width, height, centerX + bx, y, color);
    }
  }

  static void SetPixel(BYTE *pData, int width, int height, int x, int y,
                       DWORD color) {
    if (x < 0 || x >= width || y < 0 || y >= height)
      return;
    int offset = (y * width + x) * 4;
    pData[offset + 0] = (color >> 0) & 0xFF;
    pData[offset + 1] = (color >> 8) & 0xFF;
    pData[offset + 2] = (color >> 16) & 0xFF;
    pData[offset + 3] = (color >> 24) & 0xFF;
  }

  // Convert WIC bitmap to GDI HBITMAP
  static HRESULT ConvertWICBitmapToHBITMAP(IWICImagingFactory *pFactory,
                                           IWICBitmap *pWICBitmap,
                                           HBITMAP *phBitmap) {
    if (!pFactory || !pWICBitmap || !phBitmap)
      return E_INVALIDARG;

    UINT width = 0, height = 0;
    HRESULT hr = pWICBitmap->GetSize(&width, &height);
    if (FAILED(hr))
      return hr;

    HDC hdcScreen = GetDC(NULL);

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -(LONG)height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void *pBits = NULL;
    HBITMAP hBitmap =
        CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);

    if (hBitmap && pBits) {
      WICRect rcLock = {0, 0, (INT)width, (INT)height};
      CComPtr<IWICBitmapLock> pLock;
      hr = pWICBitmap->Lock(&rcLock, WICBitmapLockRead, &pLock);
      if (SUCCEEDED(hr)) {
        UINT cbBufferSize = 0;
        BYTE *pData = NULL;
        hr = pLock->GetDataPointer(&cbBufferSize, &pData);
        if (SUCCEEDED(hr)) {
          memcpy(pBits, pData, cbBufferSize);
          *phBitmap = hBitmap;
        } else {
          DeleteObject(hBitmap);
        }
      }
    } else {
      hr = E_FAIL;
    }

    ReleaseDC(NULL, hdcScreen);
    return hr;
  }
};

} // namespace ExplorerLens

#endif // _SVG_DECODER_D2D_4F8B9A3C_1E5D_4A2B_9F7C_8D3E6A1B2C4D_
