/******************************************************************************
 * DarkThumbs Plugin Decoder Adapter Implementation
 * Copyright (c) 2026 - DarkThumbs Project
 *****************************************************************************/

#include "PluginDecoder.h"
#include "IsolationModeSelector.h"
#include "PluginHostClient.h"
#include <chrono>
#include <codecvt>
#include <locale>

namespace DarkThumbs {
namespace Engine {

//============================================================================
// Helper Functions
//============================================================================

static std::string WideToUTF8(const std::wstring& wstr) {
    if (wstr.empty()) return {};
    
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), 
                                         (int)wstr.length(), nullptr, 0, 
                                         nullptr, nullptr);
    std::string result(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.length(),
                       &result[0], size_needed, nullptr, nullptr);
    return result;
}

static std::wstring UTF8ToWide(const std::string& str) {
    if (str.empty()) return {};
    
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(),
                                         (int)str.length(), nullptr, 0);
    std::wstring result(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(),
                       &result[0], size_needed);
    return result;
}

//============================================================================
// PluginDecoder Implementation
//============================================================================

PluginDecoder::PluginDecoder(PluginHandle* plugin_handle, 
                            const std::wstring& plugin_id)
    : plugin_handle_(plugin_handle)
    , plugin_id_(plugin_id)
    , isolation_mode_(IsolationMode::InWorker)
{
    // Get plugin info for decoder metadata
    if (plugin_handle_ && plugin_handle_->IsLoaded()) {
        const PluginInfo* pinfo = plugin_handle_->GetInfo();
        if (pinfo) {
            // Convert plugin info to DecoderInfo
            wcsncpy_s(info_.name, pinfo->name, _TRUNCATE);
            wcsncpy_s(info_.version, pinfo->version, _TRUNCATE);
            info_.supportedExtensions.clear();
            
            // Parse extensions
            const wchar_t* ext_start = pinfo->supported_extensions;
            while (*ext_start) {
                std::wstring ext;
                while (*ext_start && *ext_start != L';') {
                    ext += *ext_start++;
                }
                if (!ext.empty()) {
                    info_.supportedExtensions.push_back(ext);
                }
                if (*ext_start == L';') ext_start++;
            }
        }
    }
}

PluginDecoder::PluginDecoder(const std::filesystem::path& plugin_path,
                            const std::wstring& plugin_id)
    : plugin_path_(plugin_path)
    , plugin_id_(plugin_id)
    , isolation_mode_(IsolationMode::PluginHost)
{
    // Create PluginHostClient for isolated execution
    host_client_ = std::make_unique<PluginHostClient>(plugin_path);
    
    // Initialize decoder info (will be populated from PluginHost)
    wcscpy_s(info_.name, L"Plugin (Isolated)");
    wcscpy_s(info_.version, L"1.0");
}

PluginDecoder::~PluginDecoder() {
    // Cleanup handled by unique_ptr
}

bool PluginDecoder::CanDecode(const wchar_t* filePath) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (isolation_mode_ == IsolationMode::InWorker) {
        // Direct call for in-worker mode
        if (!plugin_handle_ || !plugin_handle_->IsLoaded()) {
            return false;
        }
        
        return plugin_handle_->CanDecode(filePath);
    }
    else {
        // PluginHost mode: check extension against supported list
        std::filesystem::path path(filePath);
        std::wstring ext = path.extension().wstring();
        
        for (const auto& supported : info_.supportedExtensions) {
            if (_wcsicmp(ext.c_str(), supported.c_str()) == 0) {
                return true;
            }
        }
        
        return false;
    }
}

HRESULT PluginDecoder::Decode(const ThumbnailRequest& request, 
                             ThumbnailResult& result) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto start_time = std::chrono::high_resolution_clock::now();
    stats_.total_decodes++;
    
    HRESULT hr;
    if (isolation_mode_ == IsolationMode::InWorker) {
        hr = DecodeInWorker(request, result);
    }
    else {
        hr = DecodeInPluginHost(request, result);
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        end_time - start_time);
    stats_.total_decode_time_us += duration.count();
    
    if (SUCCEEDED(hr)) {
        stats_.successful_decodes++;
    }
    else {
        stats_.failed_decodes++;
    }
    
    return hr;
}

HRESULT PluginDecoder::DecodeInWorker(const ThumbnailRequest& request, 
                                     ThumbnailResult& result) {
    if (!plugin_handle_ || !plugin_handle_->IsLoaded()) {
        return E_POINTER;
    }
    
    // Convert ThumbnailRequest to plugin DecodeRequest
    DecodeRequest plugin_req = {};
    ConvertToPluginRequest(request, plugin_req);
    
    // Decode via plugin
    DecodeResult plugin_result = {};
    PluginErrorCode error = plugin_handle_->Decode(&plugin_req, &plugin_result);
    
    if (error != PLUGIN_SUCCESS) {
        return TranslatePluginError(error);
    }
    
    // Convert result to ThumbnailResult
    HRESULT hr = ConvertPluginResult(plugin_result, result);
    
    // Free plugin result
    plugin_handle_->FreeResult(&plugin_result);
    
    return hr;
}

HRESULT PluginDecoder::DecodeInPluginHost(const ThumbnailRequest& request, 
                                         ThumbnailResult& result) {
    if (!host_client_) {
        return E_POINTER;
    }
    
    // Start PluginHost if not already running
    if (!host_client_->IsRunning()) {
        HRESULT hr = host_client_->Start();
        if (FAILED(hr)) {
            return hr;
        }
    }
    
    // Convert path to UTF-8
    std::string file_path_utf8 = WideToUTF8(request.filePath);
    
    // Send decode request via IPC
    DecodeResult plugin_result = {};
    HRESULT hr = host_client_->DecodeImage(
        file_path_utf8.c_str(),
        request.targetWidth,
        request.targetHeight,
        &plugin_result);
    
    if (FAILED(hr)) {
        if (hr == HRESULT_FROM_WIN32(ERROR_TIMEOUT)) {
            stats_.timeouts++;
        }
        return hr;
    }
    
    // Convert result to ThumbnailResult
    hr = ConvertPluginResult(plugin_result, result);
    
    // Free plugin result memory
    if (plugin_result.pixels) {
        free(const_cast<uint8_t*>(plugin_result.pixels));
    }
    
    return hr;
}

void PluginDecoder::ConvertToPluginRequest(const ThumbnailRequest& request,
                                          DecodeRequest& plugin_request) {
    // Convert file path to UTF-8
    std::string file_path_utf8 = WideToUTF8(request.filePath);
    strncpy_s(plugin_request.file_path, file_path_utf8.c_str(), _TRUNCATE);
    
    plugin_request.target_width = request.targetWidth;
    plugin_request.target_height = request.targetHeight;
    plugin_request.flags = 0;
    
    // Set flags based on request
    if (request.flags & ThumbnailFlags::PreserveAspectRatio) {
        plugin_request.flags |= DECODE_FLAG_PRESERVE_ASPECT;
    }
    if (request.flags & ThumbnailFlags::HighQuality) {
        plugin_request.flags |= DECODE_FLAG_HIGH_QUALITY;
    }
    
    plugin_request.progress_callback = nullptr;
    plugin_request.user_data = nullptr;
}

HRESULT PluginDecoder::ConvertPluginResult(const DecodeResult& plugin_result,
                                          ThumbnailResult& result) {
    if (!plugin_result.pixels || plugin_result.width == 0 || 
        plugin_result.height == 0) {
        return E_FAIL;
    }
    
    // Create HBITMAP from pixel data
    result.hBitmap = CreateHBITMAPFromPixels(
        plugin_result.pixels,
        plugin_result.width,
        plugin_result.height,
        plugin_result.pixel_format);
    
    if (!result.hBitmap) {
        return E_OUTOFMEMORY;
    }
    
    result.width = plugin_result.width;
    result.height = plugin_result.height;
    result.status = S_OK;
    
    return S_OK;
}

HBITMAP PluginDecoder::CreateHBITMAPFromPixels(const uint8_t* pixels,
                                               uint32_t width, uint32_t height,
                                               PixelFormat format) {
    // Create compatible DC
    HDC hdc = GetDC(nullptr);
    HDC memdc = CreateCompatibleDC(hdc);
    
    // Setup BITMAPINFO
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -(LONG)height; // Top-down DIB
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    
    // Create DIB section
    void* bits = nullptr;
    HBITMAP hBitmap = CreateDIBSection(memdc, &bmi, DIB_RGB_COLORS, &bits, 
                                       nullptr, 0);
    
    if (hBitmap && bits) {
        uint8_t* dest = static_cast<uint8_t*>(bits);
        size_t dest_stride = ((width * 32 + 31) / 32) * 4; // DWORD aligned
        
        // Convert pixel format if needed
        if (format == PIXEL_FORMAT_RGBA32) {
            // Convert RGBA to BGRA
            for (uint32_t y = 0; y < height; y++) {
                for (uint32_t x = 0; x < width; x++) {
                    size_t src_idx = (y * width + x) * 4;
                    size_t dest_idx = y * dest_stride + x * 4;
                    
                    dest[dest_idx + 0] = pixels[src_idx + 2]; // B
                    dest[dest_idx + 1] = pixels[src_idx + 1]; // G
                    dest[dest_idx + 2] = pixels[src_idx + 0]; // R
                    dest[dest_idx + 3] = pixels[src_idx + 3]; // A
                }
            }
        }
        else if (format == PIXEL_FORMAT_BGRA32) {
            // Direct copy
            for (uint32_t y = 0; y < height; y++) {
                memcpy(dest + y * dest_stride, 
                       pixels + y * width * 4, 
                       width * 4);
            }
        }
        else {
            // Unsupported format
            DeleteObject(hBitmap);
            hBitmap = nullptr;
        }
    }
    
    DeleteDC(memdc);
    ReleaseDC(nullptr, hdc);
    
    return hBitmap;
}

HRESULT PluginDecoder::TranslatePluginError(PluginErrorCode error) {
    switch (error) {
        case PLUGIN_SUCCESS:
            return S_OK;
        case PLUGIN_ERROR_NOT_SUPPORTED:
            return E_NOTIMPL;
        case PLUGIN_ERROR_INVALID_INPUT:
            return E_INVALIDARG;
        case PLUGIN_ERROR_OUT_OF_MEMORY:
            return E_OUTOFMEMORY;
        case PLUGIN_ERROR_FILE_NOT_FOUND:
            return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        case PLUGIN_ERROR_DECODE_FAILED:
            return E_FAIL;
        case PLUGIN_ERROR_TIMEOUT:
            return HRESULT_FROM_WIN32(ERROR_TIMEOUT);
        default:
            return E_FAIL;
    }
}

DecoderInfo PluginDecoder::GetInfo() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return info_;
}

PluginDecoder::Statistics PluginDecoder::GetStatistics() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return stats_;
}

void PluginDecoder::ResetStatistics() {
    std::lock_guard<std::mutex> lock(mutex_);
    stats_ = Statistics();
}

//============================================================================
// PluginDecoderFactory Implementation
//============================================================================

std::unique_ptr<PluginDecoder> PluginDecoderFactory::CreateDecoder(
    PluginHandle* plugin_handle,
    const std::wstring& plugin_id,
    const std::filesystem::path& plugin_path)
{
    // Use IsolationModeSelector to determine mode
    IsolationMode mode = IsolationModeSelector::Instance().DetermineMode(
        plugin_id, plugin_path);
    
    return CreateDecoderWithMode(plugin_handle, plugin_id, plugin_path, mode);
}

std::unique_ptr<PluginDecoder> PluginDecoderFactory::CreateDecoderWithMode(
    PluginHandle* plugin_handle,
    const std::wstring& plugin_id,
    const std::filesystem::path& plugin_path,
    IsolationMode mode)
{
    if (mode == IsolationMode::InWorker) {
        // In-Worker: direct plugin handle
        if (!plugin_handle) {
            return nullptr;
        }
        return std::make_unique<PluginDecoder>(plugin_handle, plugin_id);
    }
    else {
        // PluginHost: isolated execution
        return std::make_unique<PluginDecoder>(plugin_path, plugin_id);
    }
}

} // namespace Engine
} // namespace DarkThumbs
