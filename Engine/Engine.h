//==============================================================================
// DarkThumbs Engine - Public API
// Copyright (c) 2026 - DarkThumbs Project
// Version: 6.0.0
//==============================================================================

#pragma once

#include <windows.h>  // For wchar_t and __DATE__/__TIME__ macros

// Core types and interfaces
#include "Core/Types.h"
#include "Core/IThumbnailDecoder.h"
#include "Core/IFormatDetector.h"
#include "Core/IGPURenderer.h"
#include "Core/ICacheProvider.h"

namespace DarkThumbs {
namespace Engine {

//==============================================================================
/// DarkThumbs Engine Version Information
//==============================================================================

// Define version macros if not already defined by CMake
#ifndef DARKTHUMBS_ENGINE_VERSION_MAJOR
#define DARKTHUMBS_ENGINE_VERSION_MAJOR 6
#endif

#ifndef DARKTHUMBS_ENGINE_VERSION_MINOR
#define DARKTHUMBS_ENGINE_VERSION_MINOR 0
#endif

#ifndef DARKTHUMBS_ENGINE_VERSION_PATCH
#define DARKTHUMBS_ENGINE_VERSION_PATCH 0
#endif

#ifndef DARKTHUMBS_ENGINE_VERSION
#define DARKTHUMBS_ENGINE_VERSION \
    ((DARKTHUMBS_ENGINE_VERSION_MAJOR << 16) | \
     (DARKTHUMBS_ENGINE_VERSION_MINOR << 8) | \
     DARKTHUMBS_ENGINE_VERSION_PATCH)
#endif

/// Get engine version as string
inline const wchar_t* GetEngineVersion() {
    return L"6.0.0";
}

/// Get engine build date
inline const wchar_t* GetEngineBuildDate() {
    return L"" __DATE__ " " __TIME__;
}

//==============================================================================
/// Engine Information
//==============================================================================

/// Get engine name
inline const wchar_t* GetEngineName() {
    return L"DarkThumbs Thumbnail Engine";
}

/// Get engine copyright
inline const wchar_t* GetEngineCopyright() {
    return L"Copyright (c) 2026 - DarkThumbs Project";
}

} // namespace Engine
} // namespace DarkThumbs

//==============================================================================
// Usage Example:
//
// #include <Engine.h>
// using namespace DarkThumbs::Engine;
//
// // Create decoder
// IThumbnailDecoder* decoder = new MyCustomDecoder();
//
// // Setup request
// ThumbnailRequest request;
// request.filePath = L"C:\\image.jpg";
// request.width = 256;
// request.height = 256;
// request.flags = ThumbnailFlags::UseGPU | ThumbnailFlags::UseCache;
//
// // Generate thumbnail
// ThumbnailResult result;
// HRESULT hr = decoder->Decode(request, result);
// if (SUCCEEDED(hr)) {
//     // Use result.hBitmap
//     DeleteObject(result.hBitmap);
// }
//
//==============================================================================
