//==============================================================================
// ExplorerLens Engine - Public API
// Copyright (c) 2026 - ExplorerLens Project
//==============================================================================

#pragma once

#include <windows.h>  // For wchar_t and __DATE__/__TIME__ macros

// Core types and interfaces
#include "Core/ICacheProvider.h"
#include "Core/IFormatDetector.h"
#include "Core/IGPURenderer.h"
#include "Core/IThumbnailDecoder.h"
#include "Core/Types.h"

namespace ExplorerLens {
namespace Engine {

//==============================================================================
/// ExplorerLens Engine Version Information
//==============================================================================

// Define version macros if not already defined by CMake
#ifndef EXPLORERLENS_ENGINE_VERSION_MAJOR
    #define EXPLORERLENS_ENGINE_VERSION_MAJOR 25
#endif

#ifndef EXPLORERLENS_ENGINE_VERSION_MINOR
    #define EXPLORERLENS_ENGINE_VERSION_MINOR 3
#endif

#ifndef EXPLORERLENS_ENGINE_VERSION_PATCH
    #define EXPLORERLENS_ENGINE_VERSION_PATCH 0
#endif

#ifndef EXPLORERLENS_ENGINE_VERSION
    #define EXPLORERLENS_ENGINE_VERSION                                                       \
        ((EXPLORERLENS_ENGINE_VERSION_MAJOR << 16) | (EXPLORERLENS_ENGINE_VERSION_MINOR << 8) \
         | EXPLORERLENS_ENGINE_VERSION_PATCH)
#endif

}  // namespace Engine
}  // namespace ExplorerLens

// Function declarations (implemented in EngineAPI.cpp) - included at global
// scope to avoid creating nested ExplorerLens::Engine::ExplorerLens namespace
#include "Core/EngineAPI.h"

namespace ExplorerLens {
namespace Engine {

//==============================================================================
/// Engine Information
//==============================================================================

/// Get engine name
inline const wchar_t* GetEngineName()
{
    return L"ExplorerLens Thumbnail Engine";
}

/// Get engine copyright
inline const wchar_t* GetEngineCopyright()
{
    return L"Copyright (c) 2026 - ExplorerLens Project";
}

}  // namespace Engine
}  // namespace ExplorerLens

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
// // Use result.hBitmap
// DeleteObject(result.hBitmap);
// }
//
//==============================================================================
