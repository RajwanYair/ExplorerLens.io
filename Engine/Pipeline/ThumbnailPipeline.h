#pragma once

#include "../Core/Types.h"
#include "../Core/IThumbnailDecoder.h"
#include "../Core/IFormatDetector.h"
#include "../Core/IGPURenderer.h"
#include "../Core/ICacheProvider.h"
#include "DecoderRegistry.h"
#include <memory>
#include <string>

namespace DarkThumbs {
namespace Engine {

/// Configuration for the thumbnail pipeline
struct PipelineConfig {
    bool enableCache = true;
    bool enableGPU = true;
    bool preserveAspectRatio = true;
    uint32_t defaultWidth = 256;
    uint32_t defaultHeight = 256;
    uint32_t maxFileSize = 100 * 1024 * 1024;  // 100MB
    uint32_t timeoutMs = 5000;  // 5 seconds
};

/// Main orchestration class for thumbnail generation
class ThumbnailPipeline {
public:
    /// Constructor
    ThumbnailPipeline();

    /// Destructor
    ~ThumbnailPipeline();

    /// Initialize the pipeline with configuration
    /// @param config Pipeline configuration
    /// @return true if initialization succeeded
    bool Initialize(const PipelineConfig& config = PipelineConfig());

    /// Shutdown and clean up resources
    void Shutdown();

    /// Generate a thumbnail for a file
    /// @param request Thumbnail generation request
    /// @return Result containing pixel data or error
    ThumbnailResult GenerateThumbnail(const ThumbnailRequest& request);

    /// Check if a file format is supported
    /// @param filePath Path to check
    /// @return true if thumbnails can be generated
    bool IsFormatSupported(const std::wstring& filePath) const;

    /// Get decoder registry for registering custom decoders
    /// @return Reference to decoder registry
    DecoderRegistry& GetDecoderRegistry();

    /// Set format detector implementation
    /// @param detector Format detector instance
    void SetFormatDetector(std::unique_ptr<IFormatDetector> detector);

    /// Set GPU renderer implementation
    /// @param renderer GPU renderer instance
    void SetGPURenderer(std::unique_ptr<IGPURenderer> renderer);

    /// Set cache provider implementation
    /// @param cache Cache provider instance
    void SetCacheProvider(std::unique_ptr<ICacheProvider> cache);

    /// Get pipeline statistics
    /// @param totalRequests Output: total thumbnails requested
    /// @param cacheHits Output: number of cache hits
    /// @param cacheMisses Output: number of cache misses
    /// @param averageTimeMs Output: average generation time
    void GetStatistics(
        uint64_t& totalRequests,
        uint64_t& cacheHits,
        uint64_t& cacheMisses,
        double& averageTimeMs) const;

    /// Reset statistics
    void ResetStatistics();

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace Engine
} // namespace DarkThumbs
