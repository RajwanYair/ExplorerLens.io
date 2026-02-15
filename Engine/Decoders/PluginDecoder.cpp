/******************************************************************************
 * DarkThumbs Engine - Plugin Decoder Wrapper Implementation
 * Copyright (c) 2026 - DarkThumbs Project
 *****************************************************************************/

#include "PluginDecoder.h"
#include <Windows.h>
#include <codecvt>
#include <locale>

namespace DarkThumbs {
namespace Engine {

//============================================================================
// PluginDecoder Implementation
//============================================================================

PluginDecoder::PluginDecoder(PluginHandle* plugin, const std::string& pluginName)
    : plugin_(plugin)
    , pluginName_(pluginName)
{
    if (!plugin_ || !plugin_->GetInfo()) {
        return;
    }
    
    const PluginInfo* info = plugin_->GetInfo();
    
    // Convert display name to wide string
    if (info->plugin_name) {
        displayName_ = Utf8ToWide(info->plugin_name);
    } else {
        displayName_ = Utf8ToWide(pluginName_.c_str());
    }
    
    // Cache extensions
    if (info->supported_extensions) {
        for (size_t i = 0; info->supported_extensions[i] != nullptr; ++i) {
            extensions_.push_back(Utf8ToWide(info->supported_extensions[i]));
        }
    }
    
    // Build extension pointer array
    extensionPtrs_.reserve(extensions_.size() + 1);
    for (const auto& ext : extensions_) {
        extensionPtrs_.push_back(ext.c_str());
    }
    extensionPtrs_.push_back(nullptr); // Null terminator
    
    // Build DecoderInfo
    info_.name = displayName_.c_str();
    info_.version = info->plugin_version ? Utf8ToWide(info->plugin_version).c_str() : L"1.0";
    info_.author = info->plugin_author ? Utf8ToWide(info->plugin_author).c_str() : L"Unknown";
    info_.description = info->plugin_description ? Utf8ToWide(info->plugin_description).c_str() : L"";
    info_.supportedFormats = static_cast<uint32_t>(extensions_.size());
    info_.capabilities = 0;
    
    // Map plugin capabilities to decoder capabilities
    if (info->capabilities & PLUGIN_CAP_STILL_IMAGE) {
        info_.capabilities |= DECODER_CAP_IMAGES;
    }
    if (info->capabilities & PLUGIN_CAP_ANIMATION) {
        info_.capabilities |= DECODER_CAP_ANIMATIONS;
    }
    if (info->capabilities & PLUGIN_CAP_MULTIPAGE) {
        info_.capabilities |= DECODER_CAP_MULTIPAGE;
    }
}

bool PluginDecoder::CanDecode(const wchar_t* filePath) {
    if (!plugin_ || !filePath) {
        return false;
    }
    
    std::string utf8Path = WideToUtf8(filePath);
    
    // Use plugin's can_decode method
    return plugin_->CanDecode(std::filesystem::path(filePath));
}

HRESULT PluginDecoder::Decode(const ThumbnailRequest& request, ThumbnailResult& result) {
    if (!plugin_) {
        return E_POINTER;
    }
    
    // Convert wide path to UTF-8
    std::string utf8Path = WideToUtf8(request.filePath);
    
    // Prepare plugin decode request
    DecodeRequest pluginRequest = {};
    pluginRequest.file_path = utf8Path.c_str();
    pluginRequest.data = nullptr;
    pluginRequest.data_size = 0;
    pluginRequest.target_width = request.requestedSize;
    pluginRequest.target_height = request.requestedSize;
    pluginRequest.output_format = PIXEL_FORMAT_BGRA32;
    pluginRequest.preserve_aspect_ratio = true;
    pluginRequest.high_quality = true;
    pluginRequest.frame_index = 0;
    pluginRequest.user_data = nullptr;
    
    DecodeResult pluginResult = {};
    
    // Call plugin decode
    auto startTime = std::chrono::high_resolution_clock::now();
    
    PluginErrorCode errorCode = plugin_->Decode(&pluginRequest, &pluginResult, nullptr);
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    
    if (errorCode != PLUGIN_SUCCESS) {
        // Map plugin error codes to HRESULT
        switch (errorCode) {
            case PLUGIN_ERROR_INVALID_PARAMETER:
                return E_INVALIDARG;
            case PLUGIN_ERROR_UNSUPPORTED_FORMAT:
                return E_NOTIMPL;
            case PLUGIN_ERROR_OUT_OF_MEMORY:
                return E_OUTOFMEMORY;
            case PLUGIN_ERROR_FILE_NOT_FOUND:
                return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
            case PLUGIN_ERROR_READ_ERROR:
                return HRESULT_FROM_WIN32(ERROR_READ_FAULT);
            case PLUGIN_ERROR_DECODE_ERROR:
            case PLUGIN_ERROR_CORRUPTED_DATA:
                return E_FAIL;
            default:
                return E_FAIL;
        }
    }
    
    // Validate result
    if (!pluginResult.pixels || pluginResult.buffer_size == 0) {
        plugin_->FreeResult(&pluginResult);
        return E_FAIL;
    }
    
    // Create HBITMAP from pixel data
    HBITMAP hBitmap = CreateBitmapFromPixels(
        pluginResult.pixels,
        pluginResult.width,
        pluginResult.height,
        pluginResult.stride
    );
    
    // Free plugin result
    plugin_->FreeResult(&pluginResult);
    
    if (!hBitmap) {
        return E_FAIL;
    }
    
    // Fill in result
    result.hBitmap = hBitmap;
    result.width = pluginResult.width;
    result.height = pluginResult.height;
    result.renderTimeUs = static_cast<uint64_t>(duration.count());
    result.status = S_OK;
    
    return S_OK;
}

DecoderInfo PluginDecoder::GetInfo() const {
    return info_;
}

const wchar_t* PluginDecoder::GetName() const {
    return displayName_.c_str();
}

const wchar_t** PluginDecoder::GetSupportedExtensions() const {
    return extensionPtrs_.data();
}

uint32_t PluginDecoder::GetExtensionCount() const {
    return static_cast<uint32_t>(extensions_.size());
}

bool PluginDecoder::SupportsGPU() const {
    // Plugins do not currently support GPU acceleration
    return false;
}

bool PluginDecoder::IsArchiveDecoder() const {
    // Check if plugin handles archive formats by examining extensions
    // Common archive extensions: .zip, .rar, .7z, .tar, .gz, etc.
    if (!plugin_ || !plugin_->GetInfo()) {
        return false;
    }
    
    const PluginInfo* info = plugin_->GetInfo();
    if (!info->supported_extensions) {
        return false;
    }
    
    // Check if any extension is an archive format
    for (size_t i = 0; info->supported_extensions[i] != nullptr; ++i) {
        const char* ext = info->supported_extensions[i];
        if (strcmp(ext, ".zip") == 0 || strcmp(ext, ".rar") == 0 ||
            strcmp(ext, ".7z") == 0 || strcmp(ext, ".tar") == 0 ||
            strcmp(ext, ".gz") == 0 || strcmp(ext, ".bz2") == 0 ||
            strcmp(ext, ".xz") == 0 || strcmp(ext, ".cbz") == 0 ||
            strcmp(ext, ".cbr") == 0 || strcmp(ext, ".cb7") == 0) {
            return true;
        }
    }
    
    return false;
}

//============================================================================
// Private Helper Methods
//============================================================================

HBITMAP PluginDecoder::CreateBitmapFromPixels(const uint8_t* pixels,
                                              uint32_t width,
                                              uint32_t height,
                                              uint32_t stride) {
    if (!pixels || width == 0 || height == 0) {
        return nullptr;
    }
    
    // Create DIB (Device-Independent Bitmap)
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -static_cast<LONG>(height); // Negative = top-down DIB
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32; // BGRA32
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = stride * height;
    
    void* bitmapBits = nullptr;
    HDC hdc = GetDC(nullptr);
    HBITMAP hBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &bitmapBits, nullptr, 0);
    ReleaseDC(nullptr, hdc);
    
    if (!hBitmap || !bitmapBits) {
        return nullptr;
    }
    
    // Copy pixel data
    // Plugin gives us BGRA32, which is what Windows expects
    const uint8_t* src = pixels;
    uint8_t* dst = static_cast<uint8_t*>(bitmapBits);
    
    for (uint32_t y = 0; y < height; ++y) {
        memcpy(dst, src, width * 4);
        src += stride;
        dst += width * 4;
    }
    
    return hBitmap;
}

std::string PluginDecoder::WideToUtf8(const wchar_t* wstr) {
    if (!wstr) {
        return "";
    }
    
    int size = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
    if (size <= 0) {
        return "";
    }
    
    std::string result(size - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &result[0], size, nullptr, nullptr);
    
    return result;
}

std::wstring PluginDecoder::Utf8ToWide(const char* str) {
    if (!str) {
        return L"";
    }
    
    int size = MultiByteToWideChar(CP_UTF8, 0, str, -1, nullptr, 0);
    if (size <= 0) {
        return L"";
    }
    
    std::wstring result(size - 1, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, str, -1, &result[0], size);
    
    return result;
}

} // namespace Engine
} // namespace DarkThumbs
