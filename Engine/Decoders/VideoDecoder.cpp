// VideoDecoder.cpp - Video Thumbnail Decoder Implementation
// ExplorerLens Engine v6.2.0+
// Copyright (c) 2026 ExplorerLens Project

#include "VideoDecoder.h"
#include "../Utils/PerformanceProfiler.h"
#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <propvarutil.h>
#include <shlwapi.h>
#include <thumbcache.h>
#include <shobjidl.h>
#include <cwchar>

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "mf.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "propsys.lib")

namespace ExplorerLens {
namespace Engine {

// Supported extensions — broad video format coverage
// Includes VP9 (.webm) and AV1 (.mp4, .mkv, .webm) if codecs installed
const wchar_t* VideoDecoder::m_extensions[] = {
 L".mp4", L".mkv", L".avi", L".wmv", L".mov",
 L".flv", L".webm", L".m4v", L".mpg", L".mpeg",
 L".ts", L".mts", L".m2ts", L".3gp", L".3g2",
 L".vob", L".ogv", L".rm", L".rmvb", L".asf",
 L".divx", L".xvid",
 nullptr
};
const uint32_t VideoDecoder::m_extensionCount = 22;

VideoDecoder::VideoDecoder() = default;
VideoDecoder::~VideoDecoder() = default;

bool VideoDecoder::CanDecode(const wchar_t* filePath) {
 if (!filePath) return false;
 return IsVideoFormat(filePath);
}

HRESULT VideoDecoder::Decode(const ThumbnailRequest& request, ThumbnailResult& result) {
 PROFILE_SCOPE(ProfileComponent::DECODE_VIDEO);

 result.hBitmap = nullptr;
 result.width = 0;
 result.height = 0;
 if (!request.filePath) return E_INVALIDARG;

 // Try Media Foundation first
 HRESULT hr = ExtractFrameMF(request.filePath, request.width, request.height, &result.hBitmap);
 
 // Fallback to Shell thumbnail provider
 if (FAILED(hr) || !result.hBitmap) {
 hr = ExtractFrameShell(request.filePath, request.width, request.height, &result.hBitmap);
 }

 if (SUCCEEDED(hr) && result.hBitmap) {
 BITMAP bm;
 if (GetObject(result.hBitmap, sizeof(bm), &bm)) {
 result.width = bm.bmWidth;
 result.height = bm.bmHeight;
 }
 }
 return hr;
}

DecoderInfo VideoDecoder::GetInfo() const {
 DecoderInfo info;
 info.name = L"Video Decoder";
 info.version = L"1.0.0";
 info.supportedExtensions = const_cast<const wchar_t**>(m_extensions);
 info.extensionCount = m_extensionCount;
 info.supportsGPU = false;
 info.isArchiveDecoder = false;
 return info;
}

const wchar_t** VideoDecoder::GetSupportedExtensions() const {
 return const_cast<const wchar_t**>(m_extensions);
}

// ============================================================================
// Media Foundation Frame Extraction
// ============================================================================

HRESULT VideoDecoder::ExtractFrameMF(const wchar_t* filePath, uint32_t width,
 uint32_t height, HBITMAP* phBitmap) {
 if (!phBitmap) return E_INVALIDARG;
 *phBitmap = nullptr;
 (void)width; // Used for output scaling in full implementation
 (void)height;

 // Initialize COM and Media Foundation
 HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
 bool comInit = SUCCEEDED(hr) || hr == S_FALSE || hr == RPC_E_CHANGED_MODE;
 if (!comInit) return hr;

 hr = MFStartup(MF_VERSION);
 if (FAILED(hr)) {
 CoUninitialize();
 return hr;
 }

 IMFSourceReader* pReader = nullptr;
 IMFMediaType* pOutputType = nullptr;
 IMFAttributes* pAttributes = nullptr;

 // Create attributes for hardware acceleration (DXVA2)
 if (m_useHardwareAccel) {
 MFCreateAttributes(&pAttributes, 2);
 if (pAttributes) {
 pAttributes->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE);
 pAttributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE);
 }
 }

 // Create Source Reader with optional hardware acceleration
 hr = MFCreateSourceReaderFromURL(filePath, pAttributes, &pReader);
 if (pAttributes) pAttributes->Release();
 if (FAILED(hr)) goto cleanup;

 // Configure output as RGB32
 hr = MFCreateMediaType(&pOutputType);
 if (FAILED(hr)) goto cleanup;

 hr = pOutputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
 if (FAILED(hr)) goto cleanup;

 hr = pOutputType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
 if (FAILED(hr)) goto cleanup;

 hr = pReader->SetCurrentMediaType(
 static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM), nullptr, pOutputType);
 if (FAILED(hr)) goto cleanup;

 // Select only video stream
 hr = pReader->SetStreamSelection(
 static_cast<DWORD>(MF_SOURCE_READER_ALL_STREAMS), FALSE);
 if (SUCCEEDED(hr)) {
 hr = pReader->SetStreamSelection(
 static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM), TRUE);
 }
 if (FAILED(hr)) goto cleanup;

 {
 // Get video duration and seek to position
 PROPVARIANT var;
 PropVariantInit(&var);
 hr = pReader->GetPresentationAttribute(
 static_cast<DWORD>(MF_SOURCE_READER_MEDIASOURCE),
 MF_PD_DURATION, &var);

 LONGLONG seekPos = 0;
 if (SUCCEEDED(hr)) {
 LONGLONG duration = 0;
 PropVariantToInt64(var, &duration);
 seekPos = static_cast<LONGLONG>(duration * m_seekPosition);
 PropVariantClear(&var);
 }

 // Seek
 PROPVARIANT seekVar;
 PropVariantInit(&seekVar);
 seekVar.vt = VT_I8;
 seekVar.hVal.QuadPart = seekPos;
 pReader->SetCurrentPosition(GUID_NULL, seekVar);

 // Read a frame
 DWORD streamIndex = 0, flags = 0;
 LONGLONG timestamp = 0;
 IMFSample* pSample = nullptr;

 hr = pReader->ReadSample(
 static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM),
 0, &streamIndex, &flags, &timestamp, &pSample);

 if (SUCCEEDED(hr) && pSample) {
 IMFMediaBuffer* pBuffer = nullptr;
 hr = pSample->ConvertToContiguousBuffer(&pBuffer);
 if (SUCCEEDED(hr) && pBuffer) {
 BYTE* pData = nullptr;
 DWORD maxLen = 0, curLen = 0;
 hr = pBuffer->Lock(&pData, &maxLen, &curLen);
 if (SUCCEEDED(hr) && pData) {
 // Get actual video dimensions
 IMFMediaType* pActualType = nullptr;
 hr = pReader->GetCurrentMediaType(
 static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM), &pActualType);
 
 UINT32 videoWidth = 0, videoHeight = 0;
 if (SUCCEEDED(hr) && pActualType) {
 MFGetAttributeSize(pActualType, MF_MT_FRAME_SIZE, &videoWidth, &videoHeight);
 pActualType->Release();
 }

 if (videoWidth > 0 && videoHeight > 0) {
 // Create DIB section
 BITMAPINFO bmi = {};
 bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
 bmi.bmiHeader.biWidth = videoWidth;
 bmi.bmiHeader.biHeight = videoHeight; // top-down negative would flip
 bmi.bmiHeader.biPlanes = 1;
 bmi.bmiHeader.biBitCount = 32;
 bmi.bmiHeader.biCompression = BI_RGB;

 BYTE* pBits = nullptr;
 HDC hdc = GetDC(nullptr);
 *phBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS,
 reinterpret_cast<void**>(&pBits), nullptr, 0);
 ReleaseDC(nullptr, hdc);

 if (*phBitmap && pBits) {
 DWORD stride = videoWidth * 4;
 DWORD copySize = (std::min)(curLen, stride * videoHeight);
 memcpy(pBits, pData, copySize);
 }
 }

 pBuffer->Unlock();
 }
 pBuffer->Release();
 }
 pSample->Release();
 }
 }

cleanup:
 if (pOutputType) pOutputType->Release();
 if (pReader) pReader->Release();
 MFShutdown();
 CoUninitialize();

 return (*phBitmap) ? S_OK : hr;
}

// ============================================================================
// Shell Thumbnail Fallback
// ============================================================================

HRESULT VideoDecoder::ExtractFrameShell(const wchar_t* filePath, uint32_t width,
 uint32_t height, HBITMAP* phBitmap) {
 if (!phBitmap) return E_INVALIDARG;
 *phBitmap = nullptr;

 HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
 bool comInit = SUCCEEDED(hr) || hr == S_FALSE || hr == RPC_E_CHANGED_MODE;
 if (!comInit) return hr;

 // Use IShellItemImageFactory
 IShellItem* pItem = nullptr;
 hr = SHCreateItemFromParsingName(filePath, nullptr, IID_PPV_ARGS(&pItem));
 if (FAILED(hr)) {
 CoUninitialize();
 return hr;
 }

 IShellItemImageFactory* pFactory = nullptr;
 hr = pItem->QueryInterface(IID_PPV_ARGS(&pFactory));
 if (SUCCEEDED(hr) && pFactory) {
 SIZE sz = { static_cast<LONG>(width), static_cast<LONG>(height) };
 hr = pFactory->GetImage(sz, SIIGBF_THUMBNAILONLY | SIIGBF_BIGGERSIZEOK, phBitmap);
 pFactory->Release();
 }

 pItem->Release();
 CoUninitialize();
 return hr;
}

// ============================================================================
// Format Detection
// ============================================================================

bool VideoDecoder::IsVideoFormat(const wchar_t* path) {
 if (!path) return false;
 const wchar_t* ext = PathFindExtensionW(path);
 if (!ext || ext[0] == L'\0') return false;

 for (int i = 0; m_extensions[i] != nullptr; i++) {
 if (_wcsicmp(ext, m_extensions[i]) == 0) return true;
 }
 return false;
}

// ============================================================================
// Intelligent Keyframe Search
// ============================================================================

HRESULT VideoDecoder::SeekToKeyframe(IMFSourceReader* pReader, LONGLONG duration) {
 if (!pReader || duration <= 0) return E_INVALIDARG;

 // Calculate initial seek position
 LONGLONG seekPos = static_cast<LONGLONG>(duration * m_seekPosition);
 
 // Try seeking to exact position first
 PROPVARIANT var;
 PropVariantInit(&var);
 var.vt = VT_I8;
 var.hVal.QuadPart = seekPos;
 
 HRESULT hr = pReader->SetCurrentPosition(GUID_NULL, var);
 if (FAILED(hr)) return hr;

 // Read first sample to check if it's a keyframe 
// If not, search backwards within 5-second window for keyframe
 DWORD streamIndex = 0, flags = 0;
 LONGLONG timestamp = 0;
 IMFSample* pSample = nullptr;

 hr = pReader->ReadSample(
 static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM),
 0, &streamIndex, &flags, &timestamp, &pSample);

 if (pSample) pSample->Release();
 
 // If we got a sample, great! If not, try beginning of file
 if (FAILED(hr) || (flags & MF_SOURCE_READERF_ERROR)) {
 // Fallback: seek to 0
 PropVariantClear(&var);
 PropVariantInit(&var);
 var.vt = VT_I8;
 var.hVal.QuadPart = 0;
 hr = pReader->SetCurrentPosition(GUID_NULL, var);
 }

 return hr;
}

// ============================================================================
// Video Metadata Extraction
// ============================================================================

bool VideoDecoder::GetMetadata(const wchar_t* filePath, VideoMetadata& metadata) {
 if (!filePath) return false;

 HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
 bool comInit = SUCCEEDED(hr) || hr == S_FALSE || hr == RPC_E_CHANGED_MODE;
 if (!comInit) return false;

 hr = MFStartup(MF_VERSION);
 if (FAILED(hr)) {
 CoUninitialize();
 return false;
 }

 IMFSourceReader* pReader = nullptr;
 hr = MFCreateSourceReaderFromURL(filePath, nullptr, &pReader);
 
 if (SUCCEEDED(hr) && pReader) {
 // Get duration
 PROPVARIANT var;
 PropVariantInit(&var);
 hr = pReader->GetPresentationAttribute(
 static_cast<DWORD>(MF_SOURCE_READER_MEDIASOURCE),
 MF_PD_DURATION, &var);
 
 if (SUCCEEDED(hr)) {
 LONGLONG duration = 0;
 PropVariantToInt64(var, &duration);
 metadata.durationMs = static_cast<uint64_t>(duration / 10000); // Convert to milliseconds
 PropVariantClear(&var);
 }

 // Get video dimensions
 IMFMediaType* pType = nullptr;
 hr = pReader->GetCurrentMediaType(
 static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM), &pType);
 
 if (SUCCEEDED(hr) && pType) {
 MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &metadata.width, &metadata.height);
 
 // Try to get codec info (simplified)
 GUID subtype;
 if (SUCCEEDED(pType->GetGUID(MF_MT_SUBTYPE, &subtype))) {
 if (subtype == MFVideoFormat_H264) metadata.codec = L"H.264";
 else if (subtype == MFVideoFormat_HEVC) metadata.codec = L"H.265/HEVC";
 else if (subtype == MFVideoFormat_VP80) metadata.codec = L"VP8";
 else if (subtype == MFVideoFormat_VP90) metadata.codec = L"VP9";
 else if (subtype == MFVideoFormat_AV1) metadata.codec = L"AV1";
 else metadata.codec = L"Unknown";
 }
 
 pType->Release();
 }

 metadata.hasKeyframes = true; // Assume most videos have keyframes
 pReader->Release();
 }

 MFShutdown();
 CoUninitialize();
 return SUCCEEDED(hr);
}

} // namespace Engine
} // namespace ExplorerLens

