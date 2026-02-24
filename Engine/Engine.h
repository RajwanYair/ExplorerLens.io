//==============================================================================
// ExplorerLens Engine - Public API
// Copyright (c) 2026 - ExplorerLens Project
// Version: 14.0.0
//==============================================================================

#pragma once

#include <windows.h>  // For wchar_t and __DATE__/__TIME__ macros

// Core types and interfaces
#include "Core/Types.h"
#include "Core/IThumbnailDecoder.h"
#include "Core/IFormatDetector.h"
#include "Core/IGPURenderer.h"
#include "Core/ICacheProvider.h"

namespace ExplorerLens {
namespace Engine {

//==============================================================================
/// ExplorerLens Engine Version Information
//==============================================================================

// Define version macros if not already defined by CMake
#ifndef EXPLORERLENS_ENGINE_VERSION_MAJOR
#define EXPLORERLENS_ENGINE_VERSION_MAJOR 14
#endif

#ifndef EXPLORERLENS_ENGINE_VERSION_MINOR
#define EXPLORERLENS_ENGINE_VERSION_MINOR 0
#endif

#ifndef EXPLORERLENS_ENGINE_VERSION_PATCH
#define EXPLORERLENS_ENGINE_VERSION_PATCH 0
#endif

#ifndef EXPLORERLENS_ENGINE_VERSION
#define EXPLORERLENS_ENGINE_VERSION \
    ((EXPLORERLENS_ENGINE_VERSION_MAJOR << 16) | \
     (EXPLORERLENS_ENGINE_VERSION_MINOR << 8) | \
     EXPLORERLENS_ENGINE_VERSION_PATCH)
#endif

// Function declarations (implemented in EngineAPI.cpp)
#include "Core/EngineAPI.h"

//==============================================================================
/// Engine Information
//==============================================================================

/// Get engine name
inline const wchar_t* GetEngineName() {
    return L"ExplorerLens Thumbnail Engine";
}

/// Get engine copyright
inline const wchar_t* GetEngineCopyright() {
    return L"Copyright (c) 2026 - ExplorerLens Project";
}

} // namespace Engine
} // namespace ExplorerLens

//==============================================================================
// Usage Example:
//
// #include <Engine.h>
// using namespace ExplorerLens::Engine;
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


