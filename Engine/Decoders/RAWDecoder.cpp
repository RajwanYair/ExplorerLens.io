// RAWDecoder.cpp - LibRaw Implementation
// DarkThumbs Engine v1.0.0

#include "RAWDecoder.h"
#include "../Core/Logger.h"

#ifdef HAS_LIBRAW

// Suppress LibRaw static library warnings about DLL interfaces
#pragma warning(push)
#pragma warning(disable: 4251)  // needs to have dll-interface
#include <libraw/libraw.h>
#pragma warning(pop)

#include <windows.h>
#include <objbase.h>  // For IStream, PROPID (required before gdiplus.h)
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
#include <algorithm>
#include <vector>

namespace DarkThumbs {
namespace Engine {

// Extension list covering major camera manufacturers
const wchar_t* RAWDecoder::m_extensions[] = {
    // Canon
    L".cr2", L".cr3", L".crw",
    // Nikon
    L".nef", L".nrw",
    // Sony
    L".arw", L".srf", L".sr2",
    // Olympus
    L".orf",
    // Panasonic
    L".rw2", L".raw",
    // Fujifilm
    L".raf",
    // Pentax
    L".pef", L".ptx",
    // Adobe DNG (universal RAW)
    L".dng",
    // Leica
    L".rwl",
    // Samsung
    L".srw",
    // Hasselblad
    L".3fr",
    // Phase One
    L".iiq",
    // Sigma
    L".x3f",
    // Kodak
    L".dcr", L".kdc",
    // Minolta
    L".mrw",
    // Epson
    L".erf",
    // Mamiya
    L".mef",
    // Red Digital Cinema
    L".r3d"
};

const uint32_t RAWDecoder::m_extensionCount = 
    static_cast<uint32_t>(sizeof(m_extensions) / sizeof(m_extensions[0]));

// Private implementation class (pImpl pattern)
class RAWDecoder::Impl {
public:
    Impl() {
        // LibRaw initialization happens on-demand per decode
    }
    
    ~Impl() {
        // Cleanup handled by LibRaw destructor
    }
    
    // LibRaw processing options
    static constexpr int THUMBNAIL_QUALITY = 90; // JPEG quality for thumbnails
    static constexpr int HALF_SIZE_DECODE = 1;   // Use half-size for faster decode
    static constexpr int AUTO_WB = 1;            // Auto white balance
};

RAWDecoder::RAWDecoder() 
    : m_pImpl(std::make_unique<Impl>()) {
    LOG_INFO(L"RAWDecoder initialized (LibRaw 0.21)");
}

RAWDecoder::~RAWDecoder() {
    // pImpl destructor called automatically
}

bool RAWDecoder::CanDecode(const wchar_t* filePath) {
    if (!filePath || filePath[0] == L'\0') {
        return false;
    }
    
    // Check extension
    const wchar_t* ext = wcsrchr(filePath, L'.');
    if (!ext) {
        return false;
    }
    
    // Case-insensitive extension check
    for (uint32_t i = 0; i < m_extensionCount; ++i) {
        if (_wcsicmp(ext, m_extensions[i]) == 0) {
            return IsRAWFormat(filePath);
        }
    }
    
    return false;
}

HRESULT RAWDecoder::Decode(const ThumbnailRequest& request, ThumbnailResult& result) {
    if (!request.filePath || request.filePath[0] == L'\0') {
        LOG_ERROR(L"RAWDecoder: Invalid file path");
        return E_INVALIDARG;
    }
    
    LOG_INFO(L"RAWDecoder: Decoding %s (%dx%d)", 
             request.filePath, request.width, request.height);
    
    // Try embedded thumbnail first (fast path)
    HRESULT hr = ExtractEmbeddedThumbnail(request.filePath, 
                                         request.width,
                                         request.height,
                                         &result.hBitmap);
    
    if (SUCCEEDED(hr)) {
        LOG_INFO(L"RAWDecoder: Used embedded thumbnail (fast path)");
        result.width = request.width;
        result.height = request.height;
        return S_OK;
    }
    
    // Fall back to full RAW decode (slow but high quality)
    LOG_INFO(L"RAWDecoder: Embedded thumbnail not available, using full decode");
    hr = DecodeFullRAW(request.filePath,
                      request.width,
                      request.height,
                      &result.hBitmap);
    
    if (SUCCEEDED(hr)) {
        result.width = request.width;
        result.height = request.height;
    }
    
    return hr;
}

DecoderInfo RAWDecoder::GetInfo() const {
    DecoderInfo info;
    info.name = L"LibRaw RAW Decoder";
    info.version = L"0.21.2";
    return info;
}

HRESULT RAWDecoder::DecodeFromFile(const wchar_t* path, UINT targetWidth,
                                   UINT targetHeight, HBITMAP* phBitmap) {
    // Main entry point - try fast path first
    HRESULT hr = ExtractEmbeddedThumbnail(path, targetWidth, targetHeight, phBitmap);
    if (SUCCEEDED(hr)) {
        return hr;
    }
    
    // Fall back to full decode
    return DecodeFullRAW(path, targetWidth, targetHeight, phBitmap);
}

HRESULT RAWDecoder::ExtractEmbeddedThumbnail(const wchar_t* path, UINT targetWidth,
                                             UINT targetHeight, HBITMAP* phBitmap) {
    if (!phBitmap) {
        return E_POINTER;
    }
    
    *phBitmap = NULL;
    
    try {
        LibRaw rawProcessor;
        
        // Convert wchar_t path to char
        char pathMB[MAX_PATH];
        WideCharToMultiByte(CP_UTF8, 0, path, -1, pathMB, MAX_PATH, NULL, NULL);
        
        // Open file
        int ret = rawProcessor.open_file(pathMB);
        if (ret != LIBRAW_SUCCESS) {
            LOG_ERROR(L"RAWDecoder: Failed to open file: %d", ret);
            return E_FAIL;
        }
        
        // Unpack thumbnail
        ret = rawProcessor.unpack_thumb();
        if (ret != LIBRAW_SUCCESS || !rawProcessor.imgdata.thumbnail.thumb) {
            // No embedded thumbnail available
            return E_FAIL;
        }
        
        libraw_processed_image_t* thumbnail = rawProcessor.dcraw_make_mem_thumb(&ret);
        if (!thumbnail || ret != LIBRAW_SUCCESS) {
            return E_FAIL;
        }
        
        // Create HBITMAP from thumbnail data
        if (thumbnail->type == LIBRAW_IMAGE_JPEG) {
            // Decode embedded JPEG thumbnail (faster than full RAW decode)
            *phBitmap = DecodeJPEGToHBITMAP(thumbnail->data, thumbnail->data_size);
            LibRaw::dcraw_clear_mem(thumbnail);
            if (*phBitmap) {
                // Scale to target size if needed
                *phBitmap = ScaleToTarget(*phBitmap, targetWidth, targetHeight);
            }
            return (*phBitmap) ? S_OK : E_FAIL;
        } else if (thumbnail->type == LIBRAW_IMAGE_BITMAP) {
            // Create bitmap from RGB data
            *phBitmap = CreateBitmapFromRGB(thumbnail->data,
                                           thumbnail->width,
                                           thumbnail->height,
                                           rawProcessor.imgdata.sizes.flip);
            LibRaw::dcraw_clear_mem(thumbnail);
            if (*phBitmap) {
                // Scale to target size if needed
                *phBitmap = ScaleToTarget(*phBitmap, targetWidth, targetHeight);
            }
            return (*phBitmap) ? S_OK : E_FAIL;
        }
        
        LibRaw::dcraw_clear_mem(thumbnail);
        return E_FAIL;
        
    } catch (const std::exception& e) {
        LOG_ERROR(L"RAWDecoder: Exception in ExtractEmbeddedThumbnail: %S", e.what());
        return E_FAIL;
    }
}

HRESULT RAWDecoder::DecodeFullRAW(const wchar_t* path, UINT targetWidth,
                                  UINT targetHeight, HBITMAP* phBitmap) {
    if (!phBitmap) {
        return E_POINTER;
    }
    
    *phBitmap = NULL;
    
    try {
        LibRaw rawProcessor;
        
        // Set processing options for fast decode
        rawProcessor.imgdata.params.half_size = 1;           // Half-size decode (faster)
        rawProcessor.imgdata.params.use_auto_wb = 1;        // Auto white balance
        rawProcessor.imgdata.params.output_bps = 8;          // 8-bit output
        rawProcessor.imgdata.params.no_auto_bright = 0;      // Auto brightness
        rawProcessor.imgdata.params.use_camera_wb = 1;       // Use camera WB
        
        // Convert wchar_t path to char
        char pathMB[MAX_PATH];
        WideCharToMultiByte(CP_UTF8, 0, path, -1, pathMB, MAX_PATH, NULL, NULL);
        
        // Open and decode
        int ret = rawProcessor.open_file(pathMB);
        if (ret != LIBRAW_SUCCESS) {
            LOG_ERROR(L"RAWDecoder: Failed to open file: %d", ret);
            return E_FAIL;
        }
        
        // Unpack RAW data
        ret = rawProcessor.unpack();
        if (ret != LIBRAW_SUCCESS) {
            LOG_ERROR(L"RAWDecoder: Failed to unpack: %d", ret);
            return E_FAIL;
        }
        
        // Process to RGB
        ret = rawProcessor.dcraw_process();
        if (ret != LIBRAW_SUCCESS) {
            LOG_ERROR(L"RAWDecoder: Failed to process: %d", ret);
            return E_FAIL;
        }
        
        // Get processed image
        libraw_processed_image_t* image = rawProcessor.dcraw_make_mem_image(&ret);
        if (!image || ret != LIBRAW_SUCCESS) {
            LOG_ERROR(L"RAWDecoder: Failed to create image: %d", ret);
            return E_FAIL;
        }
        
        // Create bitmap from RGB data
        *phBitmap = CreateBitmapFromRGB(image->data,
                                       image->width,
                                       image->height,
                                       rawProcessor.imgdata.sizes.flip);
        
        LibRaw::dcraw_clear_mem(image);
        
        if (*phBitmap) {
            // Scale to target size if needed
            *phBitmap = ScaleToTarget(*phBitmap, targetWidth, targetHeight);
        }
        
        return (*phBitmap) ? S_OK : E_FAIL;
        
    } catch (const std::exception& e) {
        LOG_ERROR(L"RAWDecoder: Exception in DecodeFullRAW: %S", e.what());
        return E_FAIL;
    }
}

HBITMAP RAWDecoder::ScaleToTarget(HBITMAP hSource, UINT targetWidth, UINT targetHeight) {
    if (!hSource || targetWidth == 0 || targetHeight == 0) {
        return hSource;
    }
    
    BITMAP bm;
    if (!GetObject(hSource, sizeof(BITMAP), &bm)) {
        return hSource;
    }
    
    // Skip scaling if already within target size
    if (static_cast<UINT>(bm.bmWidth) <= targetWidth && 
        static_cast<UINT>(bm.bmHeight) <= targetHeight) {
        return hSource;
    }
    
    // Calculate scaled dimensions preserving aspect ratio
    float scaleW = static_cast<float>(targetWidth) / bm.bmWidth;
    float scaleH = static_cast<float>(targetHeight) / bm.bmHeight;
    float scale = (scaleW < scaleH) ? scaleW : scaleH;
    
    int newWidth = static_cast<int>(bm.bmWidth * scale);
    int newHeight = static_cast<int>(bm.bmHeight * scale);
    if (newWidth < 1) newWidth = 1;
    if (newHeight < 1) newHeight = 1;
    
    // Create scaled bitmap using GDI StretchBlt
    HDC hdc = GetDC(NULL);
    HDC hdcSrc = CreateCompatibleDC(hdc);
    HDC hdcDst = CreateCompatibleDC(hdc);
    
    HBITMAP hScaled = CreateCompatibleBitmap(hdc, newWidth, newHeight);
    HBITMAP hOldSrc = (HBITMAP)SelectObject(hdcSrc, hSource);
    HBITMAP hOldDst = (HBITMAP)SelectObject(hdcDst, hScaled);
    
    // Use HALFTONE stretch mode for quality
    SetStretchBltMode(hdcDst, HALFTONE);
    SetBrushOrgEx(hdcDst, 0, 0, NULL);
    
    StretchBlt(hdcDst, 0, 0, newWidth, newHeight,
               hdcSrc, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
    
    SelectObject(hdcDst, hOldDst);
    SelectObject(hdcSrc, hOldSrc);
    DeleteDC(hdcDst);
    DeleteDC(hdcSrc);
    ReleaseDC(NULL, hdc);
    
    // Delete original and return scaled version
    DeleteObject(hSource);
    return hScaled;
}

bool RAWDecoder::IsRAWFormat(const wchar_t* path) {
    // Quick check - just verify extension for now
    // Could add magic number check here if needed
    const wchar_t* ext = wcsrchr(path, L'.');
    if (!ext) {
        return false;
    }
    
    for (uint32_t i = 0; i < m_extensionCount; ++i) {
        if (_wcsicmp(ext, m_extensions[i]) == 0) {
            return true;
        }
    }
    
    return false;
}

HBITMAP RAWDecoder::CreateBitmapFromRGB(const BYTE* rgb, int width, int height, int orientation) {
    if (!rgb || width <= 0 || height <= 0) {
        return NULL;
    }
    
    // Create DIB section (Windows wants BGRA, but LibRaw gives RGB)
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height; // Top-down DIB
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    
    void* pBits = nullptr;
    HBITMAP hBitmap = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
    if (!hBitmap || !pBits) {
        LOG_ERROR(L"RAWDecoder: Failed to create DIB section");
        return NULL;
    }
    
    // Convert RGB to BGRA
    BYTE* dest = static_cast<BYTE*>(pBits);
    const BYTE* src = rgb;
    
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // LibRaw outputs RGB, Windows wants BGRA
            dest[0] = src[2]; // B
            dest[1] = src[1]; // G
            dest[2] = src[0]; // R
            dest[3] = 255;    // A (opaque)
            
            src += 3; // RGB
            dest += 4; // BGRA
        }
    }
    
    // Apply EXIF orientation if needed
    if (orientation != 0) {
        HBITMAP hRotated = Utils::ApplyEXIFOrientation(hBitmap, orientation);
        if (hRotated) {
            DeleteObject(hBitmap);
            return hRotated;
        }
    }
    
    return hBitmap;
}

HBITMAP RAWDecoder::DecodeJPEGToHBITMAP(const BYTE* jpegData, size_t dataSize) {
    if (!jpegData || dataSize == 0) {
        return NULL;
    }
    
    // Create stream from memory
    IStream* pStream = nullptr;
    HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, dataSize);
    if (!hGlobal) {
        return NULL;
    }
    
    void* pData = GlobalLock(hGlobal);
    if (!pData) {
        GlobalFree(hGlobal);
        return NULL;
    }
    
    memcpy(pData, jpegData, dataSize);
    GlobalUnlock(hGlobal);
    
    if (CreateStreamOnHGlobal(hGlobal, TRUE, &pStream) != S_OK) {
        GlobalFree(hGlobal);
        return NULL;
    }
    
    // Load JPEG image
    Gdiplus::Bitmap* pBitmap = Gdiplus::Bitmap::FromStream(pStream);
    pStream->Release();
    
    if (!pBitmap || pBitmap->GetLastStatus() != Gdiplus::Ok) {
        if (pBitmap) delete pBitmap;
        return NULL;
    }
    
    // Convert to HBITMAP
    HBITMAP hBitmap = NULL;
    pBitmap->GetHBITMAP(Gdiplus::Color(255, 255, 255), &hBitmap);
    delete pBitmap;
    
    return hBitmap;
}

// ============================================================================
// RAW Metadata Extraction
// ============================================================================

bool RAWDecoder::GetRAWMetadata(const wchar_t* filePath, RAWMetadata& metadata) {
    if (!filePath) return false;
    
    try {
        LibRaw rawProcessor;
        
        // Convert wchar_t path to char
        char pathMB[MAX_PATH];
        WideCharToMultiByte(CP_UTF8, 0, filePath, -1, pathMB, MAX_PATH, NULL, NULL);
        
        // Open file
        int ret = rawProcessor.open_file(pathMB);
        if (ret != LIBRAW_SUCCESS) {
            return false;
        }
        
        // Extract metadata from RAW file
        libraw_imgother_t& other = rawProcessor.imgdata.other;
        libraw_iparams_t& idata = rawProcessor.imgdata.idata;
        libraw_image_sizes_t& sizes = rawProcessor.imgdata.sizes;
        
        // Camera info
        if (idata.make[0] != '\0') {
            char makeMB[128];
            strncpy_s(makeMB, idata.make, sizeof(makeMB) - 1);
            int wideSize = MultiByteToWideChar(CP_UTF8, 0, makeMB, -1, nullptr, 0);
            if (wideSize > 0) {
                std::vector<wchar_t> wideStr(wideSize);
                MultiByteToWideChar(CP_UTF8, 0, makeMB, -1, wideStr.data(), wideSize);
                metadata.cameraMake = wideStr.data();
            }
        }
        
        if (idata.model[0] != '\0') {
            char modelMB[128];
            strncpy_s(modelMB, idata.model, sizeof(modelMB) - 1);
            int wideSize = MultiByteToWideChar(CP_UTF8, 0, modelMB, -1, nullptr, 0);
            if (wideSize > 0) {
                std::vector<wchar_t> wideStr(wideSize);
                MultiByteToWideChar(CP_UTF8, 0, modelMB, -1, wideStr.data(), wideSize);
                metadata.cameraModel = wideStr.data();
            }
        }
        
        // Exposure settings
        metadata.isoSpeed = static_cast<uint32_t>(other.iso_speed);
        metadata.shutterSpeed = other.shutter;
        metadata.aperture = other.aperture;
        metadata.focalLength = other.focal_len;
        
        // Image dimensions
        metadata.imageWidth = sizes.width;
        metadata.imageHeight = sizes.height;
        
        // Check for embedded thumbnail
        ret = rawProcessor.unpack_thumb();
        metadata.hasEmbeddedThumbnail = (ret == LIBRAW_SUCCESS && rawProcessor.imgdata.thumbnail.thumb != nullptr);
        
        // Timestamp
        if (other.timestamp > 0) {
            time_t timestamp = other.timestamp;
            struct tm timeinfo;
            if (localtime_s(&timeinfo, &timestamp) == 0) {
                wchar_t timeStr[64];
                wcsftime(timeStr, sizeof(timeStr) / sizeof(wchar_t), L"%Y-%m-%d %H:%M:%S", &timeinfo);
                metadata.timestamp = timeStr;
            }
        }
        
        return true;
        
    } catch (const std::exception& e) {
        LOG_ERROR(L"RAWDecoder: Exception in GetRAWMetadata: %S", e.what());
        return false;
    }
}

} // namespace Engine
} // namespace DarkThumbs

#else // !HAS_LIBRAW — Stub implementation when LibRaw is unavailable

#include <windows.h>

namespace DarkThumbs {
namespace Engine {

// Provide complete type for pImpl so unique_ptr destructor works
class RAWDecoder::Impl {};

const wchar_t* RAWDecoder::m_extensions[] = {
    L".cr2", L".cr3", L".nef", L".arw", L".dng", L".orf", L".rw2", L".raf"
};
const uint32_t RAWDecoder::m_extensionCount = 8;

RAWDecoder::RAWDecoder() {}
RAWDecoder::~RAWDecoder() {}
bool RAWDecoder::CanDecode(const wchar_t*) { return false; }
HRESULT RAWDecoder::Decode(const ThumbnailRequest&, ThumbnailResult&) { return E_NOTIMPL; }
bool RAWDecoder::GetRAWMetadata(const wchar_t*, RAWMetadata&) { return false; }
DecoderInfo RAWDecoder::GetInfo() const {
    DecoderInfo info{};
    info.version = 0;
    return info;
}

} // namespace Engine
} // namespace DarkThumbs

#endif // HAS_LIBRAW