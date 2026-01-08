//==============================================================================
// DarkThumbs Engine - Cache Provider Interface
// Copyright (c) 2026 - DarkThumbs Project
//==============================================================================

#pragma once

#include "Types.h"

namespace DarkThumbs {
namespace Engine {

//==============================================================================
/// Interface for thumbnail caching
/// 
/// Abstracts thumbnail cache operations for performance optimization.
/// Implementations may use Windows Thumbnail Cache, custom disk cache, or memory cache.
//==============================================================================
class ICacheProvider
{
public:
    virtual ~ICacheProvider() = default;
    
    //==========================================================================
    /// Check if a thumbnail exists in cache
    /// 
    /// @param filePath  Full path to the source file
    /// @param width     Thumbnail width
    /// @param height    Thumbnail height
    /// @return true if thumbnail exists in cache
    //==========================================================================
    virtual bool Exists(
        const wchar_t* filePath, 
        uint32_t width, 
        uint32_t height) const = 0;
    
    //==========================================================================
    /// Retrieve a thumbnail from cache
    /// 
    /// @param filePath  Full path to the source file
    /// @param width     Thumbnail width
    /// @param height    Thumbnail height
    /// @param outBitmap Output HBITMAP (caller must delete with DeleteObject)
    /// @return S_OK on success, error HRESULT on failure
    //==========================================================================
    virtual HRESULT Get(
        const wchar_t* filePath, 
        uint32_t width, 
        uint32_t height,
        HBITMAP* outBitmap) = 0;
    
    //==========================================================================
    /// Store a thumbnail in cache
    /// 
    /// @param filePath Full path to the source file
    /// @param width    Thumbnail width
    /// @param height   Thumbnail height
    /// @param hBitmap  Thumbnail bitmap to cache
    /// @return S_OK on success, error HRESULT on failure
    //==========================================================================
    virtual HRESULT Put(
        const wchar_t* filePath, 
        uint32_t width, 
        uint32_t height,
        HBITMAP hBitmap) = 0;
    
    //==========================================================================
    /// Remove a thumbnail from cache
    /// 
    /// @param filePath Full path to the source file
    /// @return S_OK on success, error HRESULT on failure
    //==========================================================================
    virtual HRESULT Remove(const wchar_t* filePath) = 0;
    
    //==========================================================================
    /// Clear all cached thumbnails
    /// 
    /// @return S_OK on success, error HRESULT on failure
    //==========================================================================
    virtual HRESULT Clear() = 0;
    
    //==========================================================================
    /// Get cache statistics
    /// 
    /// @param outEntryCount Output: Number of cached entries (can be nullptr)
    /// @param outTotalSizeMB Output: Total cache size in MB (can be nullptr)
    /// @return S_OK on success, error HRESULT on failure
    //==========================================================================
    virtual HRESULT GetStats(
        uint32_t* outEntryCount, 
        uint32_t* outTotalSizeMB) const = 0;
};

} // namespace Engine
} // namespace DarkThumbs
