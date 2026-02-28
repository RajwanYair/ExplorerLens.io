/******************************************************************************
 * ExplorerLens Plugin Decoder Adapter Implementation
 * Copyright (c) 2026 - ExplorerLens Project
 *****************************************************************************/

#include "PluginDecoder.h"
#include "../Core/PluginTypes.h"
#include "IsolationModeSelector.h"
#include "PluginHostClient.h"
#include <chrono>
#include <codecvt>
#include <locale>

namespace ExplorerLens {
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

[[maybe_unused]] static std::wstring UTF8ToWide(const std::string& str) {
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
 // Initialize DecoderInfo with defaults
 info_.name = nullptr;
 info_.version = nullptr;
 info_.supportedExtensions = nullptr;
 info_.extensionCount = 0;
 info_.supportsGPU = false;
 info_.isArchiveDecoder = false;
 
 // Get plugin info for decoder metadata
 if (plugin_handle_ && plugin_handle_->IsLoaded()) {
 const PluginInfo* pinfo = plugin_handle_->GetInfo();
 if (pinfo) {
 // Convert UTF-8 plugin info to wide strings
 if (pinfo->plugin_name) {
 int len = MultiByteToWideChar(CP_UTF8, 0, pinfo->plugin_name, -1, nullptr, 0);
 if (len > 0) {
 plugin_name_.resize(len - 1);
 MultiByteToWideChar(CP_UTF8, 0, pinfo->plugin_name, -1, &plugin_name_[0], len);
 }
 }
 
 if (pinfo->plugin_version) {
 int len = MultiByteToWideChar(CP_UTF8, 0, pinfo->plugin_version, -1, nullptr, 0);
 if (len > 0) {
 plugin_version_.resize(len - 1);
 MultiByteToWideChar(CP_UTF8, 0, pinfo->plugin_version, -1, &plugin_version_[0], len);
 }
 }
 
 // Parse extensions from UTF-8
 if (pinfo->supported_extensions) {
 for (size_t i = 0; pinfo->supported_extensions[i] != nullptr; ++i) {
 int len = MultiByteToWideChar(CP_UTF8, 0, pinfo->supported_extensions[i], -1, nullptr, 0);
 if (len > 0) {
 std::wstring ext;
 ext.resize(len - 1);
 MultiByteToWideChar(CP_UTF8, 0, pinfo->supported_extensions[i], -1, &ext[0], len);
 extension_strings_.push_back(ext);
 }
 }
 
 // Build pointer array for DecoderInfo
 for (auto& ext : extension_strings_) {
 extension_ptrs_.push_back(ext.c_str());
 }
 extension_ptrs_.push_back(nullptr); // Null terminator
 }
 
 // Set DecoderInfo pointers
 info_.name = plugin_name_.empty() ? L"Unknown Plugin" : plugin_name_.c_str();
 info_.version = plugin_version_.empty() ? L"1.0.0" : plugin_version_.c_str();
 info_.supportedExtensions = extension_ptrs_.empty() ? nullptr : extension_ptrs_.data();
 info_.extensionCount = static_cast<uint32_t>(extension_strings_.size());
 info_.supportsGPU = (pinfo->capabilities & PLUGIN_CAP_GPU_DECODE) != 0;
 info_.isArchiveDecoder = false;
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
 host_client_ = std::make_unique<PluginHostClient>();
 
 // Initialize decoder info (will be populated from PluginHost)
 plugin_name_ = L"Plugin (Isolated)";
 plugin_version_ = L"1.0";
 info_.name = plugin_name_.c_str();
 info_.version = plugin_version_.c_str();
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
 
 for (size_t i = 0; i < info_.extensionCount && info_.supportedExtensions[i]; ++i) {
 if (_wcsicmp(ext.c_str(), info_.supportedExtensions[i]) == 0) {
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
 PluginTypeConvert::ToPluginRequest(request, plugin_req);
 
 // Decode via plugin
 DecodeResult plugin_result = {};
 PluginErrorCode error = plugin_handle_->Decode(&plugin_req, &plugin_result);
 
 if (error != PLUGIN_SUCCESS) {
 return PluginTypeConvert::TranslateErrorCode(error);
 }
 
 // Convert result to ThumbnailResult (creates HBITMAP)
 HRESULT hr = PluginTypeConvert::ToEngineResult(plugin_result, result);
 if (FAILED(hr)) {
 plugin_handle_->FreeResult(&plugin_result);
 return hr;
 }
 
 // Create HBITMAP from pixel data
 result.hBitmap = CreateHBITMAPFromPixels(
 plugin_result.pixels,
 plugin_result.width,
 plugin_result.height,
 plugin_result.pixel_format);
 
 // Free plugin result
 plugin_handle_->FreeResult(&plugin_result);
 
 if (!result.hBitmap) {
 return E_OUTOFMEMORY;
 }
 
 result.status = S_OK;
 return S_OK;
}

HRESULT PluginDecoder::DecodeInPluginHost(const ThumbnailRequest& request, 
 ThumbnailResult& result) {
 if (!host_client_) {
 return E_POINTER;
 }
 
 // Start PluginHost if not already running
 if (!host_client_->IsAlive()) {
 bool started = host_client_->StartPluginHost(plugin_path_.wstring());
 if (!started) {
 return E_FAIL;
 }
 }
 
 // Convert path to UTF-8
 std::string file_path_utf8 = WideToUTF8(request.filePath);
 
 // Prepare DecodeRequest
 DecodeRequest plugin_request = {};
 PluginTypeConvert::ToPluginRequest(request, plugin_request);
 
 // Send decode request via IPC 
 DecodeResult plugin_result = {};
 IPC::IPCErrorCode ipc_error = host_client_->RequestThumbnail(
 plugin_request,
 plugin_result);
 
 if (ipc_error != IPC::IPCErrorCode::SUCCESS) {
 return PluginTypeConvert::TranslateErrorCode(plugin_result.error_code);
 }
 
 // Convert result to ThumbnailResult (creates HBITMAP)
 HRESULT hr = PluginTypeConvert::ToEngineResult(plugin_result, result);
 if (FAILED(hr)) {
 if (plugin_result.pixels) {
 free(const_cast<uint8_t*>(plugin_result.pixels));
 }
 return hr;
 }
 
 // Create HBITMAP from pixel data
 result.hBitmap = CreateHBITMAPFromPixels(
 plugin_result.pixels,
 plugin_result.width,
 plugin_result.height,
 plugin_result.pixel_format);
 
 // Free plugin result memory
 if (plugin_result.pixels) {
 free(const_cast<uint8_t*>(plugin_result.pixels));
 }
 
 if (!result.hBitmap) {
 return E_OUTOFMEMORY;
 }
 
 result.status = S_OK;
 return hr;
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
 else if (format == PIXEL_FORMAT_RGB24) {
 // Convert RGB24 to BGRA32 (swap R/B, add alpha=255)
 for (uint32_t y = 0; y < height; y++) {
 for (uint32_t x = 0; x < width; x++) {
 size_t src_idx = y * width * 3 + x * 3;
 size_t dest_idx = y * dest_stride + x * 4;
 dest[dest_idx + 0] = pixels[src_idx + 2]; // B
 dest[dest_idx + 1] = pixels[src_idx + 1]; // G
 dest[dest_idx + 2] = pixels[src_idx + 0]; // R
 dest[dest_idx + 3] = 255; // A
 }
 }
 }
 else if (format == PIXEL_FORMAT_BGR24) {
 // Convert BGR24 to BGRA32 (add alpha=255)
 for (uint32_t y = 0; y < height; y++) {
 for (uint32_t x = 0; x < width; x++) {
 size_t src_idx = y * width * 3 + x * 3;
 size_t dest_idx = y * dest_stride + x * 4;
 dest[dest_idx + 0] = pixels[src_idx + 0]; // B
 dest[dest_idx + 1] = pixels[src_idx + 1]; // G
 dest[dest_idx + 2] = pixels[src_idx + 2]; // R
 dest[dest_idx + 3] = 255; // A
 }
 }
 }
 else if (format == PIXEL_FORMAT_GRAY8) {
 // Convert Grayscale 8-bit to BGRA32
 for (uint32_t y = 0; y < height; y++) {
 for (uint32_t x = 0; x < width; x++) {
 uint8_t gray = pixels[y * width + x];
 size_t dest_idx = y * dest_stride + x * 4;
 dest[dest_idx + 0] = gray; // B
 dest[dest_idx + 1] = gray; // G
 dest[dest_idx + 2] = gray; // R
 dest[dest_idx + 3] = 255; // A
 }
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
} // namespace ExplorerLens

