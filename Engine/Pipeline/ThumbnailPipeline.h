//==============================================================================
// ThumbnailPipeline.h - Main Thumbnail Generation Orchestration
// Version: 1.0.0
//
// The ThumbnailPipeline is the primary entry point for thumbnail generation.
// It coordinates decoder selection, format detection, caching, and GPU 
// acceleration to produce high-quality thumbnails efficiently.
//
// THREAD SAFETY:
// - GenerateThumbnail(): Thread-safe, can be called concurrently
// - Initialize()/Shutdown(): NOT thread-safe, call from main thread only
// - IsFormatSupported(): Thread-safe after initialization
//
// PERFORMANCE:
// - First call: ~50-100ms (cold cache, decoder registration)
// - Cached: <1ms (cache hit)
// - Average: ~20-33ms per thumbnail (warm cache, GPU enabled)
//
// USAGE EXAMPLE:
// ThumbnailPipeline pipeline;
// PipelineConfig config;
// config.enableCache = true;
// config.enableGPU = true;
// if (pipeline.Initialize(config)) {
// ThumbnailRequest req;
// req.filePath = L"C:\\image.webp";
// req.width = 256;
// req.height = 256;
// ThumbnailResult result = pipeline.GenerateThumbnail(req);
// if (SUCCEEDED(result.status)) {
// // Use result.hBitmap
// }
// }
//==============================================================================

#pragma once

#include "../Core/Types.h"
#include "../Core/IThumbnailDecoder.h"
#include "../Core/IFormatDetector.h"
#include "../Core/IGPURenderer.h"
#include "../Core/ICacheProvider.h"
#include "DecoderRegistry.h"
#include <memory>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

/// Configuration for the thumbnail pipeline
/// 
/// All fields have sensible defaults. Adjust based on use case:
/// - For maximum speed: disable cache, use GPU
/// - For memory-con strained: enable cache, disable GPU
/// - For quality: increase timeout, enable GPU
struct PipelineConfig {
 bool enableCache = true; ///< Enable disk-backed LRU cache
 bool enableGPU = true; ///< Use GPU for scaling/rendering
 bool enablePlugins = true; ///< Enable third-party plugin loading
 bool preserveAspectRatio = true; ///< Maintain original aspect ratio
 bool enableParallelDecode = false; ///< Enable parallel batch decoding
 uint32_t defaultWidth = 256; ///< Default thumbnail width in pixels
 uint32_t defaultHeight = 256; ///< Default thumbnail height in pixels
 uint32_t maxFileSize = 100 * 1024 * 1024; ///< Max file size: 100MB
 uint32_t timeoutMs = 5000; ///< Decoding timeout: 5 seconds
 uint32_t threadPoolSize = 0; ///< Worker threads (0 = auto = hardware_concurrency/2)
 uint32_t maxConcurrentDecodes = 4; ///< Max simultaneous decode operations
};

/// Main orchestration class for thumbnail generation
/// 
/// Thread Safety: Read-only operations (IsFormatSupported, GetStatistics) and
/// GenerateThumbnail are thread-safe. Initialize/Shutdown must be called from
/// the main thread before/after concurrent usage.
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

 /// Generate thumbnails for multiple files in parallel
 /// @param requests Vector of thumbnail requests
 /// @return Vector of results (same order as requests)
 std::vector<ThumbnailResult> GenerateThumbnailsBatch(
 const std::vector<ThumbnailRequest>& requests);

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

 //==========================================================================
 // Decoder Status Reporting
 //==========================================================================

 /// Status information for a single registered decoder
 struct DecoderStatus {
 const wchar_t* name; ///< Decoder display name
 const wchar_t* version; ///< Decoder version string
 uint32_t extensionCount; ///< Number of supported extensions
 bool supportsGPU; ///< GPU acceleration available
 bool isArchiveDecoder; ///< Handles archive formats
 };

 /// Get status of all registered decoders
 /// @return Vector of decoder status entries
 std::vector<DecoderStatus> GetDecoderStatus() const;

 /// Get the count of registered decoders
 /// @return Number of decoders in the registry
 size_t GetDecoderCount() const;

 /// Get the total number of supported file extensions across all decoders
 /// @return Total extension count
 size_t GetTotalExtensionCount() const;

private:
 class Impl;
 std::unique_ptr<Impl> m_impl;
};

} // namespace Engine
} // namespace ExplorerLens

