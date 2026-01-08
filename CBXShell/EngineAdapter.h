//==============================================================================
// EngineAdapter.h - Bridge between CBXShell COM and DarkThumbs Engine
// Copyright (c) 2026 - DarkThumbs Project
//==============================================================================

#pragma once

#include "../Engine/Engine.h"
#include "../Engine/Pipeline/ThumbnailPipeline.h"
#include "../Engine/Pipeline/DecoderRegistry.h"
#include <memory>
#include <windows.h>

namespace DarkThumbs {

/// Adapter class to bridge COM shell extension with Engine library
class EngineAdapter {
public:
    EngineAdapter();
    ~EngineAdapter();

    /// Initialize the engine with default configuration
    /// @return true if initialization succeeded
    bool Initialize();

    /// Shutdown the engine and release resources
    void Shutdown();

    /// Generate a thumbnail using the Engine pipeline
    /// @param filePath Path to the file
    /// @param width Requested thumbnail width
    /// @param height Requested thumbnail height
    /// @param useGPU Enable GPU acceleration
    /// @param phBitmap Output bitmap handle
    /// @return HRESULT (S_OK on success)
    HRESULT GenerateThumbnail(
        const wchar_t* filePath,
        uint32_t width,
        uint32_t height,
        bool useGPU,
        HBITMAP* phBitmap);

    /// Check if a file format is supported
    /// @param filePath Path to check
    /// @return true if Engine can generate thumbnails for this file
    bool IsFormatSupported(const wchar_t* filePath) const;

    /// Get Engine statistics
    /// @param totalRequests Output: total thumbnails generated
    /// @param cacheHits Output: cache hits
    /// @param averageTimeMs Output: average generation time
    void GetStatistics(
        uint64_t& totalRequests,
        uint64_t& cacheHits,
        double& averageTimeMs) const;

    /// Check if Engine is initialized and ready
    /// @return true if ready to generate thumbnails
    bool IsInitialized() const { return m_initialized; }

private:
    std::unique_ptr<Engine::ThumbnailPipeline> m_pipeline;
    bool m_initialized;

    /// Register all built-in decoders with the pipeline
    void RegisterDecoders();
};

} // namespace DarkThumbs
