//==============================================================================
// ExplorerLens Engine - GPU Renderer Interface
// Copyright (c) 2026 - ExplorerLens Project
//==============================================================================

#pragma once

#include "../Types.h"

namespace ExplorerLens {
namespace Engine {

//==============================================================================
/// Interface for GPU-accelerated rendering
/// 
/// Abstracts GPU rendering operations for high-performance thumbnail scaling.
/// Implementations may use DirectX 11, DirectX 12, or CPU fallback.
//==============================================================================
class IGPURenderer
{
public:
 virtual ~IGPURenderer() = default;
 
 //==========================================================================
 /// Initialize the GPU renderer
 /// 
 /// @return S_OK on success, error HRESULT on failure
 /// 
 /// @note Should detect available GPUs and create rendering context
 /// @note May fallback to CPU rendering if no GPU available
 //==========================================================================
 virtual HRESULT Initialize() = 0;
 
 //==========================================================================
 /// Check if GPU acceleration is available
 /// 
 /// @return true if GPU is available and initialized
 //==========================================================================
 virtual bool IsAvailable() const = 0;
 
 //==========================================================================
 /// Render a thumbnail from raw image data
 /// 
 /// @param imageData Raw image data (RGBA, 32-bit per pixel)
 /// @param imageWidth Source image width
 /// @param imageHeight Source image height
 /// @param thumbWidth Desired thumbnail width
 /// @param thumbHeight Desired thumbnail height
 /// @param outBitmap Output HBITMAP (caller must delete with DeleteObject)
 /// @return S_OK on success, error HRESULT on failure
 /// 
 /// @note imageData must be in RGBA format (4 bytes per pixel)
 /// @note outBitmap will be created as a device-compatible bitmap
 //==========================================================================
 virtual HRESULT RenderThumbnail(
 const uint8_t* imageData, 
 uint32_t imageWidth, 
 uint32_t imageHeight,
 uint32_t thumbWidth, 
 uint32_t thumbHeight,
 HBITMAP* outBitmap) = 0;
 
 //==========================================================================
 /// Shutdown the GPU renderer and release resources
 //==========================================================================
 virtual void Shutdown() = 0;
 
 //==========================================================================
 /// Get GPU information
 /// 
 /// @param outName Buffer to receive GPU name (can be nullptr)
 /// @param nameSize Size of outName buffer in characters
 /// @param outMemoryMB Output: GPU memory in MB (can be nullptr)
 /// @return S_OK on success, error HRESULT on failure
 //==========================================================================
 virtual HRESULT GetGPUInfo(
 wchar_t* outName, 
 uint32_t nameSize, 
 uint32_t* outMemoryMB) const = 0;
 
 //==========================================================================
 /// Get renderer type
 /// 
 /// @return Renderer name (e.g., "DirectX 11", "DirectX 12", "CPU Fallback")
 //==========================================================================
 virtual const wchar_t* GetRendererType() const = 0;
};

} // namespace Engine
} // namespace ExplorerLens

