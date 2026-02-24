/**
 * SamplePlugin.cpp - Reference Plugin Implementation
 * Demonstrates ExplorerLens Plugin SDK usage
 * 
 * This is a minimal example plugin that decodes a hypothetical ".xyz" format.
 * Use this as a template for creating your own plugins.
 */

#include "ExplorerLensPlugin.h"
#include <windows.h>
#include <gdiplus.h>
#include <string>
#include <fstream>
#include <vector>

#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

// Plugin state
static bool g_initialized = false;
static ULONG_PTR g_gdiplusToken = 0;
static DT_PluginStatistics g_stats = {sizeof(DT_PluginStatistics)};

// Plugin metadata
static const DT_PluginInfo g_pluginInfo = {
    DT_PLUGIN_ABI_VERSION,                      // abiVersion
    L"explorerlens.plugin.sample",                // id
    L"Sample XYZ Format Decoder",               // name
    L"1.0.0",                                   // version
    L"ExplorerLens Project",                      // vendor
    L"Reference plugin demonstrating SDK usage", // description
    DT_CAP_READ_FILE | DT_CAP_DECODE,           // capabilities
    DT_ENGINE_VERSION_5_4_0,                    // minEngineVersion
    0                                           // maxEngineVersion (no limit)
};

// Supported formats
static const DT_FormatInfo g_formats[] = {
    {
        L".xyz",                                // extension
        L"image/x-xyz",                         // mimeType
        L"XYZ Sample Format",                   // description
        50                                      // priority
    }
};

/**
 * Helper: Check if file is valid XYZ format
 */
static bool IsValidXYZ(const wchar_t* filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) return false;
    
    // Read magic bytes (example: "XYZ1")
    char magic[4];
    file.read(magic, 4);
    
    return (magic[0] == 'X' && magic[1] == 'Y' && 
            magic[2] == 'Z' && magic[3] == '1');
}

/**
 * Helper: Decode XYZ file to HBITMAP
 */
static HBITMAP DecodeXYZFile(const wchar_t* filePath, uint32_t sizePx) {
    // Example: Load file data
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file) return nullptr;
    
    size_t fileSize = static_cast<size_t>(file.tellg());
    file.seekg(0);
    
    std::vector<uint8_t> data(fileSize);
    file.read(reinterpret_cast<char*>(data.data()), fileSize);
    
    // Example XYZ format structure (fictitious):
    // Header: 4 bytes magic "XYZ1"
    // uint32_t width
    // uint32_t height
    // uint8_t pixels[width * height * 4] (BGRA)
    
    if (fileSize < 12) return nullptr;
    
    uint32_t width = *reinterpret_cast<uint32_t*>(&data[4]);
    uint32_t height = *reinterpret_cast<uint32_t*>(&data[8]);
    
    if (width == 0 || height == 0 || width > 10000 || height > 10000) {
        return nullptr;
    }
    
    size_t expectedSize = 12 + (width * height * 4);
    if (fileSize < expectedSize) return nullptr;
    
    // Create GDI+ Bitmap
    Bitmap* srcBitmap = new Bitmap(width, height, PixelFormat32bppARGB);
    if (!srcBitmap) return nullptr;
    
    // Copy pixel data
    BitmapData bitmapData;
    Rect rect(0, 0, width, height);
    if (srcBitmap->LockBits(&rect, ImageLockModeWrite, PixelFormat32bppARGB, &bitmapData) == Ok) {
        memcpy(bitmapData.Scan0, &data[12], width * height * 4);
        srcBitmap->UnlockBits(&bitmapData);
    }
    
    // Calculate thumbnail size (maintain aspect ratio)
    uint32_t thumbWidth, thumbHeight;
    if (width > height) {
        thumbWidth = sizePx;
        thumbHeight = (height * sizePx) / width;
    } else {
        thumbHeight = sizePx;
        thumbWidth = (width * sizePx) / height;
    }
    
    // Create thumbnail bitmap
    Bitmap* thumbBitmap = new Bitmap(thumbWidth, thumbHeight, PixelFormat32bppARGB);
    if (!thumbBitmap) {
        delete srcBitmap;
        return nullptr;
    }
    
    // Resize using high-quality interpolation
    Graphics* graphics = Graphics::FromImage(thumbBitmap);
    if (graphics) {
        graphics->SetInterpolationMode(InterpolationModeHighQualityBicubic);
        graphics->SetPixelOffsetMode(PixelOffsetModeHighQuality);
        graphics->SetSmoothingMode(SmoothingModeHighQuality);
        
        graphics->DrawImage(srcBitmap, 0, 0, thumbWidth, thumbHeight);
        delete graphics;
    }
    
    // Convert to HBITMAP
    HBITMAP hBitmap = nullptr;
    thumbBitmap->GetHBITMAP(Color(0, 0, 0, 0), &hBitmap);
    
    delete srcBitmap;
    delete thumbBitmap;
    
    return hBitmap;
}

// ============================================================================
// Plugin Exports (Required)
// ============================================================================

DT_PLUGIN_API const DT_PluginInfo* DT_GetPluginInfo(void) {
    return &g_pluginInfo;
}

DT_PLUGIN_API const DT_FormatInfo* DT_GetSupportedFormats(uint32_t* outCount) {
    if (outCount) {
        *outCount = sizeof(g_formats) / sizeof(g_formats[0]);
    }
    return g_formats;
}

DT_PLUGIN_API DT_Status DT_Initialize(void) {
    if (g_initialized) {
        return DT_ERROR_ALREADY_INITIALIZED;
    }
    
    // Initialize GDI+
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartupOutput gdiplusStartupOutput;
    Status status = GdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, &gdiplusStartupOutput);
    
    if (status != Ok) {
        return DT_ERROR_GENERIC;
    }
    
    // Initialize statistics
    g_stats = {sizeof(DT_PluginStatistics)};
    
    g_initialized = true;
    return DT_SUCCESS;
}

DT_PLUGIN_API DT_Status DT_Shutdown(void) {
    if (!g_initialized) {
        return DT_ERROR_NOT_INITIALIZED;
    }
    
    // Shutdown GDI+
    if (g_gdiplusToken) {
        GdiplusShutdown(g_gdiplusToken);
        g_gdiplusToken = 0;
    }
    
    g_initialized = false;
    return DT_SUCCESS;
}

DT_PLUGIN_API DT_Status DT_GenerateThumbnail(
    const DT_ThumbnailRequest* request,
    DT_ThumbnailResult* result
) {
    if (!g_initialized) {
        return DT_ERROR_NOT_INITIALIZED;
    }
    
    // Validate input
    if (!request || request->structSize != sizeof(DT_ThumbnailRequest)) {
        return DT_ERROR_INVALID_ARGUMENT;
    }
    if (!result || result->structSize != sizeof(DT_ThumbnailResult)) {
        return DT_ERROR_INVALID_ARGUMENT;
    }
    if (!request->filePath) {
        return DT_ERROR_INVALID_ARGUMENT;
    }
    if (request->sizePx == 0 || request->sizePx > 4096) {
        return DT_ERROR_INVALID_ARGUMENT;
    }
    
    g_stats.totalRequests++;
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Check if file exists and is valid
    if (!IsValidXYZ(request->filePath)) {
        g_stats.failedRequests++;
        return DT_ERROR_NOT_SUPPORTED;
    }
    
    // Decode and generate thumbnail
    HBITMAP hBitmap = DecodeXYZFile(request->filePath, request->sizePx);
    if (!hBitmap) {
        g_stats.failedRequests++;
        return DT_ERROR_CORRUPT_DATA;
    }
    
    // Get bitmap dimensions
    BITMAP bm;
    GetObject(hBitmap, sizeof(BITMAP), &bm);
    
    // Fill result structure
    result->hBitmap = hBitmap;
    result->width = bm.bmWidth;
    result->height = bm.bmHeight;
    result->pixelFormat = 0; // DXGI_FORMAT_B8G8R8A8_UNORM
    result->stageMask = (1 << 0) | (1 << 2) | (1 << 3); // Detection | Decode | Transform
    
    auto endTime = std::chrono::high_resolution_clock::now();
    result->elapsedUs = std::chrono::duration_cast<std::chrono::microseconds>(
        endTime - startTime
    ).count();
    
    // Update statistics
    g_stats.successfulRequests++;
    g_stats.totalElapsedUs += result->elapsedUs;
    
    return DT_SUCCESS;
}

// ============================================================================
// Plugin Exports (Optional)
// ============================================================================

DT_PLUGIN_API DT_Status DT_CanHandle(
    const wchar_t* filePath,
    IStream* stream,
    uint32_t* outConfidence
) {
    if (!filePath || !outConfidence) {
        return DT_ERROR_INVALID_ARGUMENT;
    }
    
    // Quick check: file extension
    std::wstring path(filePath);
    if (path.length() < 4 || 
        _wcsicmp(path.substr(path.length() - 4).c_str(), L".xyz") != 0) {
        *outConfidence = 0;
        return DT_ERROR_NOT_SUPPORTED;
    }
    
    // Check magic bytes
    if (IsValidXYZ(filePath)) {
        *outConfidence = 100; // Definitely can handle
        return DT_SUCCESS;
    }
    
    *outConfidence = 0;
    return DT_ERROR_NOT_SUPPORTED;
}

DT_PLUGIN_API DT_Status DT_GetStatistics(DT_PluginStatistics* stats) {
    if (!stats || stats->structSize != sizeof(DT_PluginStatistics)) {
        return DT_ERROR_INVALID_ARGUMENT;
    }
    
    *stats = g_stats;
    return DT_SUCCESS;
}

// ============================================================================
// DLL Entry Point
// ============================================================================

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

